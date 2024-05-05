#pragma once

#include "ofMain.h"
#include "ofxOrbbecCamera.h"
#include "../gui/Gui.h"
#include "ofxOpenCv.h"


class Camera
{
    public:
        void setup();
        void update();
        void draw();
        void exit();

        void processCameraFrame(ofxCvGrayscaleImage cameraFrame, ofxCvGrayscaleImage backgroundReference);
        void addSampleToBackgroundReference(ofxCvGrayscaleImage newFrame, ofxCvGrayscaleImage& output, int samples);
        void startBackgroundReferenceSampling(int samples);
        void startBackgroundReferenceSampling();
        void clearBackgroundReference();

        void getContourPolygonsFromImage(ofxCvGrayscaleImage image, vector<ofPolyline>* output);

        void saveDebugImage(ofxCvGrayscaleImage img, string name, string step);
        void recordTestingFrames(ofxCvGrayscaleImage frame);
        uint64 recordTestingFramesCounter = 0;
        void saveMeshFrame();

        // parameters points to the mainApp's GUI. linked in main.cpp

        Gui globalParameters;
        Gui::CameraParameters* parameters;

        ofMesh mPointCloudMesh;

        ofxOrbbecCamera orbbecCam;
        ofxOrbbec::Settings settings;

        ofTexture outputTex;
        ofTexture outputTexDepth;

        // used on gui preview
        ofxCvGrayscaleImage segment;  // final segmented image

        //void linkGui();
        void linkGuiParams(Gui::CameraParameters* params);
        void onGUIStartBackgroundReference(bool& value);

    private:
        ofxCvColorImage colorFrame; // to store>transform from video file or webcam
        ofxCvGrayscaleImage cameraImage; // camera frame
        ofxCvGrayscaleImage backgroundReference;  // background reference frame

        ofxCvGrayscaleImage processedImage; // intermediate frame using during the processing
        ofxCvGrayscaleImage backgroundNewFrame;  // temporary frame used for accumulating backgrounds
        ofxCvGrayscaleImage maskImage; // used for the masking strategy
        ofxCvGrayscaleImage fillMaskforHoles;

        CvConnectedComp* comp; // used in hole filling floodfill

        bool isTakingBackgroundReference = false;
        bool backgroundReferenceTaken = false;
        int backgroundReferenceLeftFrames;

        ofxCvContourFinder contourFinder;
        vector<ofPolyline> polygons;
        int _polyLineCount;

        int IMG_WIDTH;
        int IMG_HEIGHT;
        int IMG_WIDTH_2;
        int IMG_HEIGHT_2;
        int IMG_WIDTH_4;
        int IMG_HEIGHT_4;
        int IMG_WIDTH2;
        int IMG_HEIGHT2;

        ofPoint cameraResolutions[3];

        void saveBackgroundReference(ofxCvGrayscaleImage image);
        bool restoreBackgroundReference(ofxCvGrayscaleImage & outputImage);
        
        const int BG_SAMPLE_FRAMES = 2; 
        const string BG_REFERENCE_FILENAME = "BACKGROUND_REFERENCE.png";

        void loadVideoFile();
        ofVideoPlayer prerecordedVideo;

        enum class VideoSources : int {
            VIDEOSOURCE_ORBBEC, 
            VIDEOSOURCE_VIDEOFILE,
            VIDEOSOURCE_WEBCAM
        };
        VideoSources currentVideosource;
};

