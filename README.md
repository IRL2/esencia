# esencia

A rewrite from the **danceroom Spectroscopy** interactive art installation.

> Multi- award winning danceroom Spectroscopy (dS) is a new interactive visualisation of the nano-world. Fusing 3D imagery with real molecular dynamics, dS allows you to see your own energy field, and use it to interact with the otherwise invisible atomic world.

https://www.danceroom-spec.com/

https://www.theguardian.com/science/small-world/2013/oct/25/danceroom-spectroscopy-invisible-world-visible

## Goals

- Create a reinterpretation / rewrite
- Be able to exhibit it again
- Make it opensource for remixing

## Setup

Currently working on the [Orbbec Femto Bolt](https://www.orbbec.com/products/tof-camera/femto-bolt/) depth camera.

### Developing environment

1. [Install and setup openFrameworks 0.12](https://openframeworks.cc/download/)
	- [Windows visual studio 2019](https://openframeworks.cc/setup/vs/)
	- [Linux visual studio code](https://openframeworks.cc/setup/vscode/)
        - [OSX/macOS via Xcode](https://openframeworks.cc/setup/xcode/)

2. Clone this repo on apps folder `{openframeworks_folder}/apps/myApps/`

3. Clone the following addons on the addons folder `{openframeworks_folder}/addons/`
   - [ofxOrbbec](https://github.com/IRL2/ofxOrbbec/) (forked from [design-io](https://github.com/design-io/ofxOrbbec/))
   - [ofxGuiExtended](https://github.com/radamchin/ofxGuiExtended) (forked from [frauzufall](https://github.com/frauzufall/ofxGuiExtended) via [radamchin fix](https://github.com/radamchin/ofxGuiExtended))

\*Note this repo already includes the orbbec shared libraries on the bin and bin/libs folder, from the ofxOrbbec addon.

### Flags
- DEBUG_IMAGES shows a button to allow save each step of the video frame processing
- RECORD_TESTING_VIDEO shows a button to save frames directly form the camera, allowing to create test videos

## Usage

1. Connect the camera
- If ther camera is not connected or detected, it will automatically play a prerecorded video instead (file included).

2. Run the project:

- For Windows Visual Studio 2019 open the solution `esencia.sln`
- For Linux Visual Studio Code (min 1.80) open `esencia.code-workspace`
- For macOS Xcode (14.1 or newer) open `esencia.xcode-project`
- Or just run `make` from the terminal
*Release mode gives better performance.

3. Play whith the gui parameters [See the wiki](https://github.com/IRL2/esencia/wiki).
   - Currently the simulation runs on CPU so its limited to 200 particles.

