# MicroPython ImagiMob Integration for Infineon's PSoC6 port

This directory contains the imagimob generated model files and the interface for it's enablement in micropython. To access your edge ai models developed for PSoC6 based kit from a micropython application, please follow the description below.

## Folder structure
Currently the existing sample model files are generated for [Human Activity Recognition](https://developer.imagimob.com/getting-started/modus-toolbox-solution-and-Imagimob-Studio#human-activity-recognition) code from imagimob.

**model.c** : Imagimob generated source code in C for your developed model in the studio.

**model.h** : Imagimob generated source code in C for your developed model in the studio.

**imai_mp_iface.c** : MicroPython interface for ImagiMob models.

**micropython.mk** : Makefile configurations for firmware generation.

## Installation

Please check for the pre-requisites mentioned [here](../../ports/psoc6/README.md#Pre-requisites).

## Usage

Follow the steps below to generate micropython bindings for your imagimob generated model.

1. The imagimob micropytthon integration is available in imai-mp-integration, so checkout to that branch after cloning this repository.

        git clone https://github.com/Infineon/micropython.git

        git checkout --track origin/ports-psoc6-main

2. Then initialize the ModusToolbox™ environment: 

        cd ports/psoc6

        make mtb_init BOARD=<board-name>

3. Retrieve submodules:

        make submodules

4. In the examples/usercimaimodule/imagimob/ folder, replace the **model.c** and **model.h** files by your imagimob generated model files.

5. Compile it alongwith psoc6 port source code and generate the final .hex to be flashed into your device. From your root:

        cd ports/psoc6
        
        make USER_C_MODULES=../../examples/usercimaimodule/

        make program

# Run micropython

Use any micropython supported IDE (like Thonny) and establish a session with your device. Now you should be able to import your model as shown below:

    import imai as model

    model.init() # Initialize the model

    model.enqueue(data_in) # Send the data inputs for the model

    model.dequeue(data_out) # Get the classifications

**Note**: Above is only an abstract code to show the usage in micropython. Please take care of populating the data-in and data_out with right dimensions and values. The provided api's abide by its operation in imagimob. Please refer to API documentation [here](https://developer.imagimob.com/edge-optimization/edge-api). 