#pragma once

#include "ofMain.h"
#include "ofxOrbbecCamera.h"
#include "../gui/GuiApp.h"
#include "ofxOpenCv.h"


class Camera
{
    enum class VideoSources : int {
        VIDEOSOURCE_ORBBEC = 1,
        VIDEOSOURCE_VIDEOFILE = 2,
        VIDEOSOURCE_WEBCAM = 3,
        VIDEOSOURCE_NONE = 0
    };

    public:
        void setup(CameraParameters* params);
        void update();
        void draw();
        void exit();

        void keyReleased(ofKeyEventArgs& e);

        void stopCurrentSource();

        void processCameraFrame(ofxCvGrayscaleImage &cameraFrame, ofxCvGrayscaleImage &backgroundReference);
        void addSampleToBackgroundReference(ofxCvGrayscaleImage newFrame, ofxCvGrayscaleImage& output, int samples);
        void startBackgroundReferenceSampling(int samples);
        void startBackgroundReferenceSampling();
        void clearBackgroundReference();

        void getContourPolygonsFromImage(ofxCvGrayscaleImage image, vector<ofPolyline>* output);

        void saveDebugImage(ofxCvGrayscaleImage img, string name, string step);
        void saveDebugImage(ofPixels img, string name, string step);
        void recordTestingFrames(ofxCvGrayscaleImage frame);
        uint64 recordTestingFramesCounter = 0;
        void saveMeshFrame();

        // parameters points to the mainApp's GUI. linked in main.cpp

        GuiApp globalParameters;
        CameraParameters* parameters;

        ofMesh mPointCloudMesh;

        ofxOrbbecCamera orbbecCam;
        ofxOrbbec::Settings orbbecSettings;

        ofTexture outputTex;
        ofTexture outputTexDepth;

        // used on gui preview
        ofxCvGrayscaleImage segment;  // final segmented image

        //void linkGui();
        //void linkGuiParams(GuiApp::CameraParameters* params);
        void onGUIStartBackgroundReference(bool& value);
        void onGUIChangeSource(bool& _);
        void changeSource(VideoSources newSource);

        ofShader blurHorizontal;
        ofShader blurVertical;
        ofFbo   fboBlurOnePass;
        ofFbo   fboBlurTwoPass;

        // We'll wrap the GPU blur in a helper method:
        void gpuBlur(ofxCvGrayscaleImage& input, float sigma);

    private:
        ofxCvColorImage colorFrame; // to store>transform from video file or webcam
        ofxCvGrayscaleImage source; // camera frame
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

        // to-do: resolutions should came from getDeviceList, but it doesnt in the current ofxaddon
        // For Femto: [2]512x512 is WFOV binned, [0] 640x576 is NFOV, [1] 320x288 is NFOV binned
        const ofPoint cameraResolutions[3] = {
            ofPoint(640, 576),
            ofPoint(320, 288),
            ofPoint(512, 512)
        };
        int selectedOrbbecResolution = 0;

        void saveBackgroundReference(ofxCvGrayscaleImage image);
        bool restoreBackgroundReference(ofxCvGrayscaleImage & outputImage);
        
        const int BG_SAMPLE_FRAMES = 2; 
        const string BG_REFERENCE_FILENAME = "BACKGROUND_REFERENCE.png";

        void setupOrbbecCamera();
        void setupWebcam();
        void loadVideoFile();
        ofVideoPlayer prerecordedVideo;

        void setFrameSize(int width, int height);

        VideoSources currentVideosource;
};

void convertToTransparent(ofxCvGrayscaleImage &grayImage, ofImage &rgbaImage);
