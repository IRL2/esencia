# esencia

A rewrite from the **danceroom Spectroscopy** interactive art installation.

> Multi- award winning danceroom Spectroscopy (dS) is a new interactive visualisation of the nano-world. Fusing 3D imagery with real molecular dynamics, dS allows you to see your own energy field, and use it to interact with the otherwise invisible atomic world.

https://www.danceroom-spec.com/

https://www.theguardian.com/science/small-world/2013/oct/25/danceroom-spectroscopy-invisible-world-visible

## Goals

- Create a rewrite and reinterpretation of the original
- Be able to exhibit it again on modern hardware with more accessible hardware
- Make it open source for remixing

## Setup

Currently working on the [Orbbec Femto Bolt](https://www.orbbec.com/products/tof-camera/femto-bolt/) depth camera.

### Developing environment

1. [Install and setup openFrameworks 0.12](https://openframeworks.cc/download/)
    - [Windows visual studio 2019/2022](https://openframeworks.cc/setup/vs/) Recommended
    - [Linux visual studio code](https://openframeworks.cc/setup/vscode/) Not tested
    - [OSX/macOS via Xcode](https://openframeworks.cc/setup/xcode/) Orbbec Femto Bolt camera is not currently supported on macOS

2. Clone this repo on apps folder `{openframeworks_folder}/apps/myApps/`

3. Clone the following addons on the addons folder `{openframeworks_folder}/addons/`
   - [ofxOrbbec](https://github.com/IRL2/ofxOrbbec/) (forked from [design-io](https://github.com/design-io/ofxOrbbec/))
   - [ofxGuiExtended](https://github.com/IRL2/ofxGuiExtended) (forked from [frauzufall](https://github.com/frauzufall/ofxGuiExtended) via [radamchin fix](https://github.com/radamchin/ofxGuiExtended))
   - [ofxEasing](https://github.com/arturoc/ofxEasing)
   - [ofxPresets](https://github.com/IRL2/ofxPresets)

\*Note this repo already includes the required orbbec shared libraries on the bin and bin/libs folder, from the ofxOrbbec addon.


## Usage

1. Connect the camera
- If ther camera is not connected or detected, it will automatically play a prerecorded video instead (file included).
- Place the camera and turn it 90 degrees to the right.
 
2. Run the project:

- For Windows Visual Studio 2019 open the solution `esencia.sln`
- For Linux Visual Studio Code (min 1.80) open `esencia.code-workspace`
- For macOS Xcode (14.1 or newer) open `esencia.xcode-project`
- Or just run `make` from the terminal
**Release mode gives better performance.**

3. Save a background reference image (for the background subtraction)
 
- Once its running, expand the video source panel source and background groups
- Ensure the camera is pointing to the right space
- Clear the space
- Click on the `Save Background` button or press `Ctrl+b` on the keyboard

4. Play with the gui parameters [See the wiki](https://github.com/IRL2/esencia/wiki).

### Shortcuts

On the GUI window:
- `Ctrl+b` Save background

On the main simulation window:
- `f` Toggle fullscreen
- `1-9` Change the presets (combine with shift for 10-19)
- `Shift+` `1-9` Save the current preset
- `space` Start/stop the sequencer
- `m` Mutates the current preset
- `Ctrl+s` Save the preset on the current slot
- `Ctrl+c` Clear the current preset slot