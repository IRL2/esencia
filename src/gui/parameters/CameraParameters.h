#pragma once
#include "ParametersBase.h"

class CameraParameters : public ParametersBase {
public:
    ofParameter<bool> enableClipping = true;
    ofParameter<int> clipFar = 170; // Threshold low
    ofParameter<int> clipNear = 20; // Threshold hi

    ofParameter<float> blobMinArea = 0.05f;
    ofParameter<float> blobMaxArea = 0.8f;
    ofParameter<int> gaussianBlur = 0;
    ofParameter<int> nConsidered = 0;
    ofParameter<bool> fillHolesOnPolygons = false;
    ofParameter<bool> floodfillHoles = true;

    ofParameter<float> polygonTolerance = 2.0f;
    ofParameter<bool> showPolygons = false;

    ofParameter<bool> startBackgroundReference = true;
    ofParameter<bool> saveDebugImages = false;
    ofParameter<bool> recordTestingVideo = false;
    ofParameter<bool> useMask = false;

    ofParameter<bool> _sourceOrbbec = false;
    ofParameter<bool> _sourceVideofile = false;
    ofParameter<bool> _sourceWebcam = false;

    ofImage previewSegment;
    ofImage previewSource;
    ofImage previewBackground;

    ofParameterGroup parameters;

    CameraParameters() {
        parameters.setName("Camera Parameters");
        parameters.add(enableClipping.set("Enable Clipping", true));
        parameters.add(clipFar.set("Clip Far", 170));
        parameters.add(clipNear.set("Clip Near", 20));
        parameters.add(blobMinArea.set("Blob Min Area", 0.05f));
        parameters.add(blobMaxArea.set("Blob Max Area", 0.8f));
        parameters.add(gaussianBlur.set("Gaussian Blur", 0));
        parameters.add(nConsidered.set("Number Considered", 0));
        parameters.add(fillHolesOnPolygons.set("Fill Holes on Polygons", false));
        parameters.add(floodfillHoles.set("Floodfill Holes", true));
        parameters.add(polygonTolerance.set("Polygon Tolerance", 2.0f));
        parameters.add(showPolygons.set("Show Polygons", false));
        parameters.add(startBackgroundReference.set("Start Background Reference", true));
        parameters.add(saveDebugImages.set("Save Debug Images", false));
        parameters.add(recordTestingVideo.set("Record Testing Video", false));
        parameters.add(useMask.set("Use Mask", false));
        parameters.add(_sourceOrbbec.set("Source: Orbbec", false));
        parameters.add(_sourceVideofile.set("Source: Video File", false));
        parameters.add(_sourceWebcam.set("Source: Webcam", false));
    }

    ofParameterGroup& getParameters() override {
        return parameters;
    }

    void loadPreviewImages(const std::string& segmentPath, const std::string& sourcePath, const std::string& backgroundPath) {
        previewSegment.load(segmentPath);
        previewSource.load(sourcePath);
        previewBackground.load(backgroundPath);
    }

    void savePreviewImages(const std::string& outputDir) {
        previewSegment.save(outputDir + "/previewSegment.png");
        previewSource.save(outputDir + "/previewSource.png");
        previewBackground.save(outputDir + "/previewBackground.png");
    }
};
