#pragma once

#include "EsenciaPanelBase.h"

class VideoProcessingPanel : public EsenciaPanelBase {

    const bool FLOODFILL_HOLES = { false };
    const bool PRESERVE_DEPTH = { false };

    const bool SHOW_POLYGONS = { false };
    const bool FIND_POLYGGON_HOLES = { false };

    const float BLOB_MIN_INITIAL = 0.05;
    const float BLOB_MIN_MIN = 0.0;
    const float BLOB_MIN_MAX = 0.3;

    const float BLOB_MAX_INITIAL = 8.0;
    const float BLOB_MAX_MIN = 0.5;
    const float BLOB_MAX_MAX = 1.0;

    const float MAXIMUM_BLOBS_INITIAL = 8;
    const float MAXIMUM_BLOBS_MIN = 0;
    const float MAXIMUM_BLOBS_MAX = 64;

    const float GAUSSIAN_BLUR_INITIAL = 1;
    const float GAUSSIAN_BLUR_MIN = 0;
    const float GAUSSIAN_BLUR_MAX = 130;

    const float POLYGON_APPROX_INITIAL = 1.0;
    const float POLYGON_APPROX_MIN = 0.0;
    const float POLYGON_APPROX_MAX = 5.0;

    const ofRectangle PANEL_RECT = ofRectangle(13, 9, 8, 0);
    const ofColor BG_COLOR = ofColor(30, 30, 200, 100);


public:
	void setup(ofxGui &gui, CameraParameters &params) {
		panel = gui.addPanel("video processing");
		
        ofxGuiGroup* cameraSourcePreview = panel->addGroup("preview");
        
        cameraSourcePreview->add<ofxGuiGraphics>("segment", &params.previewSegment.getTexture(), 
            ofJson({ {"height", 200} }));


        // PROCESSING
        ofxGuiGroup* cameraProcessingPanel = panel->addGroup("processing");
        
        cameraProcessingPanel->add(params.gaussianBlur.set("final gaussian blur", 
            GAUSSIAN_BLUR_INITIAL, GAUSSIAN_BLUR_MIN, GAUSSIAN_BLUR_MAX));

        params.gaussianBlur.addListener(this, &VideoProcessingPanel::onGaussianblurUpdate);


        cameraProcessingPanel->add(params.floodfillHoles.set("floodfill holes", 
            FLOODFILL_HOLES));

        cameraProcessingPanel->add(params.useMask.set("preserve depth", 
            PRESERVE_DEPTH));



        // POLYGONS
        ofxGuiGroup* cameraPolygonsPanel = panel->addGroup("polygons");
        
        cameraPolygonsPanel->add(params.blobMinArea.set("min size blob", 
            BLOB_MIN_INITIAL, BLOB_MIN_MIN, BLOB_MIN_MAX),
            ofJson({ {"precision", 2} }));
        
        cameraPolygonsPanel->add(params.blobMaxArea.set("max size blob", 
            BLOB_MAX_INITIAL, BLOB_MAX_MIN, BLOB_MAX_MAX),
            ofJson({ {"precision", 2} }));
        
        cameraPolygonsPanel->add(params.nConsidered.set("maximum blobs",
            MAXIMUM_BLOBS_INITIAL, MAXIMUM_BLOBS_MIN, MAXIMUM_BLOBS_MAX));
        
        cameraPolygonsPanel->add(params.showPolygons.set("show polygons", 
            SHOW_POLYGONS));
        
        cameraPolygonsPanel->add(params.fillHolesOnPolygons.set("find holes on polygon", 
            FIND_POLYGGON_HOLES));
        
        cameraPolygonsPanel->add(params.polygonTolerance.set("polygon approximation", 
            POLYGON_APPROX_INITIAL, POLYGON_APPROX_MIN, POLYGON_APPROX_MAX),
            ofJson({{"precision", 2}}));
        
        cameraPolygonsPanel->minimize();

        configVisuals(PANEL_RECT, BG_COLOR);
    }


    // gaussian blur needs to be an odd value
    void onGaussianblurUpdate(int& value) {
        if (value % 2 == 0.0) {
            value = value + 1;
        }
    }

};