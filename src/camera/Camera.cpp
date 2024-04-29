#include "Camera.h"
#include "../ofApp.h"

void Camera::setup() {

    //For Femto: 512 is WFOV binned, 640 is NFOV, 320 is NFOV binned
    cameraResolutions[0] = ofPoint(640, 576);
    cameraResolutions[1] = ofPoint(320, 288);
    cameraResolutions[2] = ofPoint(512, 512);
    int _selectedResolution = 0;

    IMG_WIDTH  = cameraResolutions[_selectedResolution].x;
    IMG_HEIGHT = cameraResolutions[_selectedResolution].y;
    IMG_WIDTH2  = IMG_WIDTH * 2;
    IMG_HEIGHT2 = IMG_HEIGHT * 2;
    IMG_WIDTH_2  = IMG_WIDTH / 2;
    IMG_HEIGHT_2 = IMG_HEIGHT / 2;
    IMG_HEIGHT_4 = IMG_HEIGHT / 4;
    IMG_WIDTH_4 = IMG_WIDTH / 4;

    ofSetLogLevel(OF_LOG_NOTICE);
    auto deviceInfo = ofxOrbbecCamera::getDeviceList();

    settings.bColor = false;
    settings.bDepth = true;
    settings.bPointCloud = false;
    settings.bPointCloudRGB = false;
    settings.depthFrameSize.requestWidth = cameraResolutions[_selectedResolution].x;
    //settings.depthFrameSize.format = OB_FORMAT_Y16;
    //settings.colorFrameSize.format = OB_FORMAT_MJPG;
    //settings.colorFrameSize.requestWidth = 1280;
    //settings.bPointCloudRGB = true; 
    //settings.ip = "192.168.50.70";

    orbbecCam.open(settings);

    cameraImage.allocate(IMG_WIDTH, IMG_HEIGHT);
    processedImage.allocate(IMG_WIDTH, IMG_HEIGHT);
    backgroundNewFrame.allocate(IMG_WIDTH, IMG_HEIGHT);
    backgroundReference.allocate(IMG_WIDTH, IMG_HEIGHT);
    maskImage.allocate(IMG_WIDTH, IMG_HEIGHT);
    segment.allocate(IMG_WIDTH, IMG_HEIGHT);
    fillMaskforHoles.allocate(IMG_WIDTH+2, IMG_HEIGHT+2);
    
    restoreBackgroundReference(backgroundReference);
}

/// <summary>
/// assign params pointer and listeners to value changes
/// </summary>
/// <param name="params">pointer from the gui structure</param>
void Camera::linkGuiParams(Gui::CameraParameters *params) {
    parameters = params;
    parameters->startBackgroundReference.addListener(this, &Camera::onGUIStartBackgroundReference);
}

void Camera::onGUIStartBackgroundReference(bool &value) {
    if (value == true) {
        // to-do: add a callback for when the process ends, so i can restart the button state
        startBackgroundReferenceSampling(BG_SAMPLE_FRAMES);
        //parameters->startBackgroundReference = false;
    }
}


//--------------------------------------------------------------
void Camera::exit() {
    orbbecCam.close();
}

//--------------------------------------------------------------
void Camera::draw() {
    ofBackground(60, 60, 60);


    int verts   = mPointCloudMesh.getNumTexCoords();
    int indices = mPointCloudMesh.getNumIndices();
    int polys   = polygons.size();
    int polyLines = _polyLineCount;

    std::stringstream meshInfo;
    meshInfo << "Verts:   " << verts << "\nIndices: " << indices << "\nPolygons: " << polys << "\nLines: " << _polyLineCount;

    //ofSetColor(255);
    //ofEnableDepthTest();
    //mCam.begin();
    //ofPushMatrix();
    //ofTranslate(0, -300, 1000);
    //mPointCloudMesh.draw();
    //ofPopMatrix();
    //mCam.end();
    //ofDisableDepthTest();

    ofSetHexColor(0xffffff);

    cameraImage.draw(1, 1, IMG_WIDTH_2-1, IMG_HEIGHT_2-1);
    ofDrawBitmapStringHighlight("Raw camera", 11, 20, ofColor(30,30,30), ofColor(104,140,247));

    backgroundNewFrame.draw(1, IMG_HEIGHT_2+1, IMG_WIDTH_2-1, IMG_HEIGHT_2-1);
    ofDrawBitmapStringHighlight("Background reference", 11, IMG_HEIGHT_2+20, ofColor(30,30,30), ofColor(104,140,247));

    if (isTakingBackgroundReference) {
        ofDrawBitmapStringHighlight("registering background reference: -" + ofToString(backgroundReferenceLeftFrames), 10, IMG_HEIGHT_2 * 1.5, ofColor(230, 30,40), ofColor(250,250,250));
        return;
    }
    if (parameters->recordTestingVideo) {
        ofDrawBitmapStringHighlight("recording testing video " + ofToString(recordTestingFramesCounter), 30, IMG_HEIGHT_2/2, ofColor(230, 30, 40), ofColor(250, 250, 250));
        return;
    }


    // draw the processed image
    segment.draw(1, IMG_HEIGHT+2, IMG_WIDTH_2-1, IMG_HEIGHT_2-1);
    ofDrawBitmapStringHighlight("Final extraction", 11, IMG_HEIGHT+20, ofColor(30,30,30), ofColor(104,140,247));

    // draw found polygons
    if (parameters->showPolygons) {
        ofSetHexColor(0xff0000);
        ofSetLineWidth(5);
        ofFill();
        ofPushMatrix();
        ofTranslate(1, IMG_HEIGHT);
        ofScale(0.5);
        for (const auto& polyline : polygons) {
            polyline.draw();
            ofDrawBox(polyline.getCentroid2D().x, polyline.getCentroid2D().y, 10, 10);
        }
        ofPopMatrix();
    }

    ofSetHexColor(0xffffff);
    ofDrawBitmapString(ofGetFrameRate(), ofGetWidth()-40, 20);
}


//--------------------------------------------------------------
void Camera::update() {

    orbbecCam.update();

    if (orbbecCam.isFrameNewDepth()) {
        mDepthPixels = orbbecCam.getDepthPixels();
        cameraImage.setFromPixels(mDepthPixels);
        //outputTexDepth.loadData(mDepthPixels);
        //mPointCloudMesh = orbbecCam.getPointCloudMesh();

        // to-do: make backgroundref an object with state?
        if (isTakingBackgroundReference) {
            addSampleToBackgroundReference(cameraImage, backgroundReference, BG_SAMPLE_FRAMES);
            return;
        }

        processCameraFrame(cameraImage, backgroundReference);
    }
}


#pragma region Frame processing


/// <summary>
/// Process each camera frame to extract interested objects
/// </summary>
void Camera::processCameraFrame(ofxCvGrayscaleImage frame, ofxCvGrayscaleImage backgroundReference) {
    // The processing assumes a depth grayscalled image, a background reference if available
    // 
    // 1. removes far and near elements by thresholding
    // 2. gets the different pixels from frame and the background to get a roi
    // 3. reduce the noise
    // 4. (optional) uses the previous all-white image as a mask to extract the actual depth values from the frame
    // 5. (optional) obtains a series of polygons from the images
    // // additional noise reduction
    // 6. apply gaussian blur to the roi 

    processedImage = frame;
    backgroundNewFrame = backgroundReference;
    saveDebugImage(processedImage, "cameraFrame", "initial");
    if (parameters->recordTestingVideo) {
        recordTestingFrames(processedImage);
        return;
    }

    // 1. Depth threshold 
    // ---------
    // remove far and near by thresholding, from the background and from the 
    if (parameters->enableClipping) {
        // clipping the camera
        cvThreshold(processedImage.getCvImage(), processedImage.getCvImage(), parameters->clipFar, 0, CV_THRESH_TOZERO_INV);
        //saveDebugImage(processedImage, "processedImage", "low threshold");
        // to-do: near clipping is not needed
        //        near detection is needed to stop all the processing/visualization
        cvThreshold(processedImage.getCvImage(), processedImage.getCvImage(), parameters->clipNear, 0, CV_THRESH_TOZERO);
        saveDebugImage(processedImage, "processedImage", "high threshold");
    }

    // clipping the background reference after subtraction
    if (parameters->enableClipping) {
        // to-do: get reference background from the disk if available
        if (backgroundReferenceTaken) {
            cvThreshold(backgroundReference.getCvImage(), backgroundNewFrame.getCvImage(), parameters->clipFar, 0, CV_THRESH_TOZERO_INV);
            cvThreshold(backgroundNewFrame.getCvImage(), backgroundNewFrame.getCvImage(), parameters->clipNear, 0, CV_THRESH_TOZERO);
            saveDebugImage(backgroundNewFrame, "backgroundNewFrame", "thresholded");
        }
    }

    // 2. Subtract BG from frame
    // ---------
    // remove the background from the frame (save the output in maskImage object)
    if (backgroundReferenceTaken) {
        cvAbsDiff(processedImage.getCvImage(), backgroundNewFrame.getCvImage(), segment.getCvImage()); // this works great for a single background frame of reference!

        saveDebugImage(processedImage, "processedImage", "removed background absdiff");
    }
    else {
        segment = processedImage;
        // when no background ref taken to be subtracted, just use the frame
        //processedImage = segment;
        //maskImage = processedImage;
        //segment = proce
    }

    // 3. Flat the segment image
    // ---------
    // make it black and white
    segment.threshold(5);
    processedImage = segment;
    saveDebugImage(segment, "segment", "threshold 5");


    // 4. Remove holes
    // ---------
    if (parameters->floodfillHoles) {
        fillMaskforHoles.allocate(IMG_WIDTH + 2, IMG_HEIGHT + 2);
        fillMaskforHoles.clear();
        cvFloodFill(processedImage.getCvImage(), cvPoint(0, 0), cvScalar(255), cvScalar(0), cvScalarAll(0), comp CV_DEFAULT(NULL), CV_FLOODFILL_FIXED_RANGE, fillMaskforHoles.getCvImage());
        processedImage.invert();
        saveDebugImage(processedImage, "processedImage", "extracted holes");
        cvOr(segment.getCvImage(), processedImage.getCvImage(), processedImage.getCvImage());
        saveDebugImage(processedImage, "processedImage", "floodfill");
    }
    processedImage.erode();


    // 5. Mask image
    // ---------
    //// use the mask against the original frame, to get the original depth values from the roi
    ////maskImage = segment; // or use a previous state of segment before holes were filled
    //maskImage = segment;
    if (parameters->useMask) {
        maskImage = processedImage;
        segment *= maskImage; // this is for masking the original frame against the masked!
        processedImage = segment;
        saveDebugImage(processedImage, "processedImage", "masked");
    }

    // remove noise from the extracted image
    // to-do: check if a single erode pass is usually enough to remove the noise
    //processedImage.erode();
    //processedImage.erode();
    //saveDebugImage(processedImage, "processedImage", "eroded");

    // calculate the polygons from the roi
    if (parameters->showPolygons) {
        getContourPolygonsFromImage(processedImage, &polygons);
    }

    // add gaussian blur to the silouetes
    // (this step was originaly performed by the simulation on the sorounds of each particle, its here now to test if the performance is better)
    processedImage.blurGaussian(parameters->gaussianBlur);

    segment = processedImage;
}

void Camera::getContourPolygonsFromImage(ofxCvGrayscaleImage image, vector<ofPolyline> *output) {
//    ofxCvContourFinder contourFinder;
    vector<ofPolyline> _polys;

    // regular find contours with defined parameters
    contourFinder.findContours(image, parameters->blobMinArea * image.getPixels().size(), parameters->blobMaxArea * image.getPixels().size(), parameters->nConsidered, parameters->fillHolesOnPolygons, true);
    //ofLog() << "Camera::getContourPolygonsFromImage Blobs size after findContours: " << contourFinder.blobs.size();

    // generate (simplified) polygons from blobs
    _polys.clear();
    _polyLineCount = 0;
    for (const auto& blob : contourFinder.blobs) {
        ofPolyline polyline;
        for (const auto& point : blob.pts) {
            polyline.addVertex(point.x, point.y);
        }
        polyline.setClosed(true);
        //polyline.addVertex(polyline.getVertices()[0].x, polyline.getVertices()[0].y);
        _polyLineCount++;
        // simplify the shape to have less edges
        polyline.simplify(parameters->polygonTolerance);

        _polys.push_back(polyline);
        //_polyLineCount += polyline.size();
    }
    // to-do: convert poly into image

    *output = _polys;
}

#pragma endregion


#pragma region Background subtraction


/// <summary>
/// Updates the background reference for later removal
/// </summary>
void Camera::addSampleToBackgroundReference(ofxCvGrayscaleImage newFrame, ofxCvGrayscaleImage &output, int samples) {
    int currSample = samples - backgroundReferenceLeftFrames;

    // original artwork strategy: sampleFrame = (sampleFrame + newFrame) * currentsamplef * (1 / (currentsamplef + 1)) //wtf?

    output = cameraImage; // good enough for 1 frame!!

    // ---- other failed strategies!

    //output *= newFrame ; // dark image, the more samples the more  it goes

    // the most closer to the original strategy of sampling
    //cvDiv(0, output.getCvImage(), output.getCvImage(), 1/1-(1/samples));
    //cvDiv(0, newFrame.getCvImage(), newFrame.getCvImage(), 1/1-(1/samples));
    //cvAdd(output.getCvImage(), newFrame.getCvImage(), output.getCvImage());

    // try to replicate original strategy by 
    //cvAdd(output.getCvImage(), cameraImage.getCvImage(), output.getCvImage()); // too bright
      //cvScaleAdd(cameraImage.getCvImage(), cvScalar(currSample / (currSample + 1)), output.getCvImage(), output.getCvImage()); // does nothing
      //cvScaleAdd(cameraImage.getCvImage(), cvScalar((1/ (samples))), output.getCvImage() , output.getCvImage()); // too bright
      //cvMul(output.getCvImage(), newFrame.getCvImage(), output.getCvImage(), currSample * (1/(currSample+1)) ); // black

    // stack samples on the same image
    // this one is good way to get a noiseless image using ~20 frames, but i would need change the subtraction for a loose-filtering instead of a subtraction
    // however the current orbbec camera is pretty noiseless on the usable range and the noise is made of individual pixels instead of big chunks as in the original kinect
    //cvAddWeighted(
    //    output.getCvImage(),
    //    1.0 - (1.0 / samples),
    //    newFrame.getCvImage(),
    //    1.0 / samples,
    //    0.0,
    //    output.getCvImage());

    output.flagImageChanged();

    backgroundReferenceLeftFrames--;

    if (backgroundReferenceLeftFrames == 0) {
        backgroundReference = output;
        ofLog() << "Camera::addSampleToBackgroundReference: finishing adquiring background references";
        saveDebugImage(backgroundReference, "backgroundNewFrame", "initial");
        saveBackgroundReference(backgroundReference);

        // to-do: use new bgref class state
        backgroundReferenceTaken = true;
        isTakingBackgroundReference = false;

        // to-do: decouple this (gui call) with a callback or something
        parameters->startBackgroundReference = false;
    }
}

/// <summary>
/// Initiate the adquisition of background references for later background subtraction w/noise removal
/// </summary>
void Camera::startBackgroundReferenceSampling() {
    startBackgroundReferenceSampling(BG_SAMPLE_FRAMES);
}

void Camera::startBackgroundReferenceSampling(int samples) {
    ofLog() << "Camera::startBackgroundReferenceSampling: starting background";
    isTakingBackgroundReference = true;
    backgroundReferenceTaken = false;
    clearBackgroundReference();
    backgroundReference = cameraImage; // Important step to take an initial sample, so any strategy of adding/weighting/... dont start from a black image
    backgroundReferenceLeftFrames = samples;
}

void Camera::clearBackgroundReference() {
    ofLog() << "Camera::clearBackgroundReference: clearing background";
    backgroundReference.set(0);
}


/// <summary>
/// Writes a file with the reference image for later use
/// </summary>
/// <param name="image">the current background grayscale image</param>
void Camera::saveBackgroundReference(ofxCvGrayscaleImage image) {
    ofLog() << "Camera::saveBackgroundReference >> Saving background reference on data/" + BG_REFERENCE_FILENAME;
    if (ofFile::doesFileExist(BG_REFERENCE_FILENAME)) {
        ofLog() << "Camera::saveBackgroundReference >> File already exist, will be overwriten";
    }
    ofSaveImage(image.getPixels(), BG_REFERENCE_FILENAME);
}


/// <summary>
/// Reloads a previous reference from the disk
/// </summary>
/// <param name="imageObject"></param>
/// <returns>True if loads the reference successfully</returns>
bool Camera::restoreBackgroundReference(ofxCvGrayscaleImage & outputImage) {
    ofLog() << "Camera::restoreBackgroundReference >> Attempting to load a background reference at data/" + BG_REFERENCE_FILENAME;
    ofPixels pixels;
    bool loaded = ofLoadImage(pixels, BG_REFERENCE_FILENAME);
    if (loaded) {
        outputImage.allocate(pixels.getWidth(), pixels.getHeight());
        outputImage.setFromPixels(pixels);
        backgroundReferenceTaken = true;
        ofLog() << "Camera::restoreBackgroundReference >> Load successfull";
    }
    else {
        backgroundReferenceTaken = false;
        ofLog() << "Camera::restoreBackgroundReference >> Could not load the background reference image";
    }
    return loaded;
}

#pragma endregion


#pragma region utils

/// <summary>
/// save an image, used for debugging and documentation
/// </summary>
/// <param name="img">image</param>
/// <param name="name">object name id (background, camera, cleared..)</param>
/// <param name="step">additional id, i.e. sequence step</param>
void Camera::saveDebugImage(ofxCvGrayscaleImage img, string name, string step) {
#ifdef DEBUG_IMAGES
    if (parameters->saveDebugImages) {
        const string& filename = ofGetTimestampString() + "_" + name + "_" + step + ".png";
        ofSaveImage(img.getPixels(), filename);
    }
#endif
}

void Camera::recordTestingFrames(ofxCvGrayscaleImage img) {
    recordTestingFramesCounter++;
    const string& filename = "raw_recording\\" + ofToString(recordTestingFramesCounter) + ".png";
    ofSaveImage(img.getPixels(), filename);
}




void Camera::saveMeshFrame() {

    const string& filename = "mesh_" + ofGetTimestampString() + ".obj";

    if (mPointCloudMesh.getNumVertices() < 10) {
        return;
    }

    ofMesh & mesh = mPointCloudMesh;

    // Open the file for writing
    ofFile file(filename, ofFile::WriteOnly);

    if (!file.exists()) {
        ofLogError("saveMeshToFile") << "Unable to open file for writing: " << filename;
        return;
    }

    // Write vertices
    for (int i = 0; i < mesh.getNumVertices(); ++i) {
        ofVec3f vertex = mesh.getVertex(i);
        file << "v " << vertex.x << " " << vertex.y << " " << vertex.z << endl;
    }

    //// Write texture coordinates if present
    //if (mesh.getNumTexCoords() > 0) {
    //    for (int i = 0; i < mesh.getNumTexCoords(); ++i) {
    //        ofVec2f texCoord = mesh.getTexCoord(i);
    //        file << "vt " << texCoord.x << " " << texCoord.y << endl;
    //    }
    //}

    //// Write normals if present
    //if (mesh.getNumNormals() > 0) {
    //    for (int i = 0; i < mesh.getNumNormals(); ++i) {
    //        ofVec3f normal = mesh.getNormal(i);
    //        file << "vn " << normal.x << " " << normal.y << " " << normal.z << endl;
    //    }
    //}

    // Write faces
    for (int i = 0; i < mesh.getNumIndices(); i += 3) {
        file << "f ";
        for (int j = 0; j < 3; ++j) {
            file << mesh.getIndex(i + j) + 1 << "/" << mesh.getIndex(i + j) + 1 << "/" << mesh.getIndex(i + j) + 1 << " ";
        }
        file << endl;
    }

    ofLogNotice("saveMeshToFile") << "Mesh saved to " << filename;
}


#pragma endregion


