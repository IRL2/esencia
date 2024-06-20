#include "Camera.h"
#include "../ofApp.h"

/// <summary>
/// assign params pointer and listeners to value changes
/// </summary>
/// <param name="params">pointer from the gui structure</param>
void Camera::setup(Gui::CameraParameters* params) {

    // link parameters and listeners
    parameters = params;

    // for when the 'bg reference' button is pressed
    parameters->startBackgroundReference.addListener(this, &Camera::onGUIStartBackgroundReference);

    // for changes in the video source
    parameters->_sourceOrbbec.addListener(this, &Camera::onGUIChangeSource);
    parameters->_sourceVideofile.addListener(this, &Camera::onGUIChangeSource);
    parameters->_sourceWebcam.addListener(this, &Camera::onGUIChangeSource);

    // to-do: extract the available resolutions
    auto deviceInfo = ofxOrbbecCamera::getDeviceList();
    ofLogNotice("Camera::setup()") << "Found " << deviceInfo.size() << " Orbbec devices";
    for (auto& device: deviceInfo)
    {
        ofLogNotice("Camera::setup()") << "Device: " << device.get()->name();
    }

    // Try to initialize the orbbec camera as first option, else use a video file
    if (deviceInfo.size() != 0) {
        changeSource(VideoSources::VIDEOSOURCE_ORBBEC);
    }
    else {
        changeSource(VideoSources::VIDEOSOURCE_VIDEOFILE);
    }

    restoreBackgroundReference(backgroundReference);
}

/// <summary>
/// Listener for the GUI buttons to change the source
/// </summary>
/// <param name="_"></param>
void Camera::onGUIChangeSource(bool& _) {
    ofLogWarning("Camera::onGUIChangeSource()") << "GUI triggered a source change w/" << _;

    if (parameters->_sourceOrbbec) {
        changeSource(VideoSources::VIDEOSOURCE_ORBBEC);
    }
    else if (parameters->_sourceVideofile) {
        changeSource(VideoSources::VIDEOSOURCE_VIDEOFILE);
    }
    else if (parameters->_sourceWebcam) {
        changeSource(VideoSources::VIDEOSOURCE_WEBCAM);
    } else {
        stopCurrentSource();
    }
}


/// <summary>
/// Use this to change the current video source
/// It handles everything: stops previous source and reset frames sizes
/// </summary>
/// <param name="newSource">From the list of VideoSources</param>
void Camera::changeSource(VideoSources newSource) {
    stopCurrentSource();

    currentVideosource = VideoSources::VIDEOSOURCE_NONE; // in the new setup fail, stay at None

    if (newSource == VideoSources::VIDEOSOURCE_ORBBEC) {
        setupOrbbecCamera();
    }
    else if (newSource == VideoSources::VIDEOSOURCE_VIDEOFILE) {
        loadVideoFile();
    }
    else if (newSource == VideoSources::VIDEOSOURCE_WEBCAM) {
        setupWebcam();
    }

    // to-do: need to call setFrameSize with the updated frame size from the new source
    setFrameSize(cameraResolutions[selectedOrbbecResolution].x, cameraResolutions[selectedOrbbecResolution].y);

    // Now handling the GUI button state
    // to-do: this belongs to the gui class, move it!
    // the .disable/enableEvents are here to avoid triggering the listener and enter into an (semi)infinite loop
    parameters->_sourceOrbbec.disableEvents();
    parameters->_sourceVideofile.disableEvents();
    parameters->_sourceWebcam.disableEvents();

    parameters->_sourceOrbbec = (currentVideosource == VideoSources::VIDEOSOURCE_ORBBEC);
    parameters->_sourceVideofile = (currentVideosource == VideoSources::VIDEOSOURCE_VIDEOFILE);
    parameters->_sourceWebcam = (currentVideosource == VideoSources::VIDEOSOURCE_WEBCAM);

    parameters->_sourceOrbbec.enableEvents();
    parameters->_sourceVideofile.enableEvents();
    parameters->_sourceWebcam.enableEvents();
}

/// <summary>
/// Allocates the opencv internal images for the processing according to the source resolution
/// And sets IMG_X shortuts (mostly for calcs and positioning)
/// </summary>
/// <param name="width"></param>
/// <param name="height"></param>
void Camera::setFrameSize(int width, int height) {
    IMG_WIDTH = width;
    IMG_HEIGHT = height;

    IMG_WIDTH2 = IMG_WIDTH * 2;
    IMG_HEIGHT2 = IMG_HEIGHT * 2;
    IMG_WIDTH_2 = IMG_WIDTH / 2;
    IMG_HEIGHT_2 = IMG_HEIGHT / 2;
    IMG_HEIGHT_4 = IMG_HEIGHT / 4;
    IMG_WIDTH_4 = IMG_WIDTH / 4;

    source.allocate(IMG_WIDTH, IMG_HEIGHT);
    processedImage.allocate(IMG_WIDTH, IMG_HEIGHT);
    backgroundNewFrame.allocate(IMG_WIDTH, IMG_HEIGHT);
    backgroundReference.allocate(IMG_WIDTH, IMG_HEIGHT);
    maskImage.allocate(IMG_WIDTH, IMG_HEIGHT);
    segment.allocate(IMG_WIDTH, IMG_HEIGHT);
    fillMaskforHoles.allocate(IMG_WIDTH+2, IMG_HEIGHT+2);
    colorFrame.allocate(IMG_WIDTH, IMG_HEIGHT);
    
    parameters->previewSource.allocate(IMG_WIDTH, IMG_HEIGHT, OF_IMAGE_GRAYSCALE);
    parameters->previewSegment.allocate(IMG_WIDTH, IMG_HEIGHT, OF_IMAGE_GRAYSCALE);
    parameters->previewBackground.allocate(IMG_WIDTH, IMG_HEIGHT, OF_IMAGE_GRAYSCALE);

    // TO-DO: if the prev/current background reference image has different resolution from the new video source: kaput.
}


/// <summary>
/// Initialize the acquisition of frames for the background reference
/// </summary>
/// <param name="value"></param>
void Camera::onGUIStartBackgroundReference(bool &value) {
    if (value == true) {
        // to-do: add a callback for when the process ends, so i can restart the button state
        //parameters->startBackgroundReference = false;
        startBackgroundReferenceSampling(BG_SAMPLE_FRAMES);
    }
}

/// <summary>
/// Loads a video file to use it as the video source
/// </summary>
void Camera::loadVideoFile() {
    ofLogNotice("Camera::loadVideoFile()") << "Switching source to video file";

    // to-do: let the user select the file (ofFileDialogResult from ofSystemLoadDialog)
    ofLogNotice("Camera::loadVideoFile()") << "Attempting to load a video file";
    prerecordedVideo.load("video_mocks/movement_nfov_h264.mp4");
    // to-do: throw error when file does not exist, or cant be loaded

    prerecordedVideo.setLoopState(OF_LOOP_NORMAL);
    prerecordedVideo.play();

    currentVideosource = VideoSources::VIDEOSOURCE_VIDEOFILE;
    ofLogNotice("Camera::loadVideoFile()") << "Camera::loadVideoFile Video file loaded";
}

/// <summary>
/// Initializes the orbbec camera as the video source
/// </summary>
void Camera::setupOrbbecCamera() {
    ofLogNotice("Camera::setupOrbbecCamera()") << "Switching source to Orbbec camera";
    
    if (ofxOrbbecCamera::getDeviceList().size() == 0) {
        ofLogNotice("Camera::setupOrbbecCamera()") << "No Orbbec camera detected";
        return;
    }

    int orbbecRequestedWidth = cameraResolutions[selectedOrbbecResolution].x;

    orbbecSettings.bColor = false;
    orbbecSettings.bDepth = true;
    orbbecSettings.bPointCloud = false;
    orbbecSettings.bPointCloudRGB = false;
    orbbecSettings.depthFrameSize.requestWidth = orbbecRequestedWidth;

    orbbecCam.open(orbbecSettings);
    currentVideosource = VideoSources::VIDEOSOURCE_ORBBEC;
}

/// <summary>
/// Initializes the webcam as the video source (not yet implemented)
/// </summary>
void Camera::setupWebcam() {
    ofLogNotice("Camera::setupWebcam()") << "Switching source to webcam";
    currentVideosource = VideoSources::VIDEOSOURCE_WEBCAM;
}

/// <summary>
/// Class exit.
/// Calls a funtion to stop the current video source
/// </summary>
void Camera::exit() {
    stopCurrentSource();
}

/// <summary>
/// Calls the corresponding stop methods according to the current video soure
/// </summary>
void Camera::stopCurrentSource() {
    if (currentVideosource == VideoSources::VIDEOSOURCE_ORBBEC) {
        ofLogNotice("Camera::stopCurrentSource()") << "Closing the orbbec camera";
        orbbecCam.close();
    }
    else if (currentVideosource == VideoSources::VIDEOSOURCE_VIDEOFILE) {
        ofLogNotice("Camera::stopCurrentSource()") << "Stopping the video file playback";
        prerecordedVideo.stop();
    }
    // webcam.stop
}

//--------------------------------------------------------------
void Camera::draw() {
    ofBackground(60, 60, 60);

    ofSetHexColor(0xffffff);
    
    //prerecordedVideo.draw(1, 1);
    source.draw(1, 1, IMG_WIDTH_2-1, IMG_HEIGHT_2-1);
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

    // acquire frame
    if (currentVideosource == VideoSources::VIDEOSOURCE_VIDEOFILE) {
        prerecordedVideo.update();

        if (prerecordedVideo.isFrameNew()) {
            colorFrame.setFromPixels(prerecordedVideo.getPixels());
            source = colorFrame;
        }
    }

    if (currentVideosource == VideoSources::VIDEOSOURCE_ORBBEC) {
        orbbecCam.update();

        if (orbbecCam.isFrameNewDepth()) {
            source.setFromPixels(orbbecCam.getDepthPixels());
        }
    }

    // to-do: make backgroundref an object with state?
    if (isTakingBackgroundReference) {
        addSampleToBackgroundReference(source, backgroundReference, BG_SAMPLE_FRAMES);
        return;
    }

    // all the processing from source to extract the final segment
    processCameraFrame(source, backgroundReference);


    // update the preview images on the shared parameters data structure for the GUI
    parameters->previewSource.setFromPixels(source.getPixels());
    parameters->previewSegment.setFromPixels(segment.getPixels());
    parameters->previewBackground.setFromPixels(backgroundReference.getPixels()); // TO-DO: move where bgref is assigned, so is running only when need it
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
        saveDebugImage(processedImage, "processedImage", "low threshold");
        // to-do: near detection is needed to stop all the processing/visualization (near clipping is not needed)
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
    processedImage.erode();
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

    output = source; // good enough for 1 frame!!

    // ---- failed strategies:

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
        ofLogNotice("Camera::addSampleToBackgroundReference()") << "Finishing adquiring background references";
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
    ofLogNotice("Camera::startBackgroundReferenceSampling()") << "Starting background";
    isTakingBackgroundReference = true;
    backgroundReferenceTaken = false;
    clearBackgroundReference();
    backgroundReference = source; // Important step to take an initial sample, so any strategy of adding/weighting/... dont start from a black image
    backgroundReferenceLeftFrames = samples;
}

void Camera::clearBackgroundReference() {
    ofLogNotice("Camera::clearBackgroundReference()") << "Clearing background";
    backgroundReference.set(0);
}


/// <summary>
/// Writes a file with the reference image for later use
/// </summary>
/// <param name="image">the current background grayscale image</param>
void Camera::saveBackgroundReference(ofxCvGrayscaleImage image) {
    ofLogNotice("Camera::saveBackgroundReference()") << "Saving background reference on data/" + BG_REFERENCE_FILENAME;
    if (ofFile::doesFileExist(BG_REFERENCE_FILENAME)) {
        ofLogNotice("Camera::saveBackgroundReference()") << "File already exist, will be overwriten";
    }
    ofSaveImage(image.getPixels(), BG_REFERENCE_FILENAME);
}


/// <summary>
/// Reloads a previous reference from the disk
/// </summary>
/// <param name="imageObject"></param>
/// <returns>True if loads the reference successfully</returns>
bool Camera::restoreBackgroundReference(ofxCvGrayscaleImage & outputImage) {
    ofLogNotice("Camera::restoreBackgroundReference") << "Attempting to load a background reference at data/" + BG_REFERENCE_FILENAME;
    ofPixels pixels;
    bool loaded = ofLoadImage(pixels, BG_REFERENCE_FILENAME);
    if (loaded) {
        // to-do: validate bg ref and frame have the same size resolution
        outputImage.allocate(pixels.getWidth(), pixels.getHeight());
        outputImage.setFromPixels(pixels);
        backgroundReferenceTaken = true;
        ofLogNotice("Camera::restoreBackgroundReference") << "Load successfull";
    }
    else {
        backgroundReferenceTaken = false;
        ofLogNotice("Camera::restoreBackgroundReference") << "Could not load the background reference image";
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
        ofLogError("Camera::saveMeshFrame()") << "Unable to open file for writing: " << filename;
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

    ofLogNotice("Camera::saveMeshFrame()") << "Mesh saved to " << filename;
}


#pragma endregion


