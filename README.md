# esencia

A rewrite from the **danceroom Spectroscopy** interactive art installation.

> Multi- award winning danceroom Spectroscopy (dS) is a new interactive visualisation of the nano-world. Fusing 3D imagery with real molecular dynamics, dS allows you to see your own energy field, and use it to interact with the otherwise invisible atomic world.

https://www.danceroom-spec.com/

https://www.theguardian.com/science/small-world/2013/oct/25/danceroom-spectroscopy-invisible-world-visible

## Goals

- A reinterpretation / rewrite
- To be able to exhibit it again
- To opensource it for renewals

## Setup

Currently working on the [Orbbec Femto Bolt](https://www.orbbec.com/products/tof-camera/femto-bolt/) depth camera.

### Developing environment

1. [Install and setup openFrameworks 0.12](https://openframeworks.cc/download/)
	- [Windows visual studio 2019](https://openframeworks.cc/setup/vs/)
	- [Linux visual studio code](https://openframeworks.cc/setup/vscode/)

2. Pull this repo on apps folder `{openframeworks_folder}/apps/myApps/`

3. Pull [ofxOrbbec addon](https://github.com/IRL2/ofxOrbbec/) on addons folder `{openframeworks_folder}/addons/`
	- ([official design-io addon](https://github.com/design-io/ofxOrbbec/))

4. Manually copy the compiled shared camera libraries into the project
	- From `{openframeworks_folder}/addons/ofxOrbbec/libs/orbbec/lib/{your operating system}`
	- Into `{openframeworks_folder}/apps/myApps/esencia/bin/libs/`

### Running

Open the project
- For Windows Visual Studio 2019 open the solution `esencia.sln`
- For Linux Visual Studio Code (min 1.80) open `esencia.code-workspace`
- For MacOS XCode () open ``




