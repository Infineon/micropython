.. _psoc6_intro:

Getting started with MicroPython on the PSoC6™
==============================================

This tutorial will guide you to get started with running micropython on the PSoC6™ microcontrollers. 
There are only a few step keeping you away from enjoy the python programming experience together
with the possibilities of PSoC6™ microcontrollers.
Let's get started!


Requirements
------------

The required hardware is only:
    * PSoC6™ board of the :ref:`Supported boards` list.
    * A microUSB cable.


Getting the firmware
--------------------

The binary .hex file is available in the `GitHub release section <https://github.com/jaenrig-ifx/micropython/releases>`_. 
All PSoC6™ firmware versions for each of the supported boards can be found there. 

Powering the board
------------------

Connect the USB cable to your computer and the microUSB to the board debugger. All the PSoC6™ boards
provide a on-board debugger required for flashing operation during development. Please refer to the
corresponding board manual of your board.

Deploying the firmware
----------------------

In order to deploy the firmware in our device, the easiest and quickest way is to use the `Cypress Programmer <https://softwaretools.infineon.com/tools/com.ifx.tb.tool.cypressprogrammer>`_
software. It allows to program the PSoC6™ microcontrollers family in a few clicks from your Windows,
Linux or Mac OS machine. 
Follow the instructions on the provided tool link to download and install the tool.

After that, select the downloaded micropython firmware .hex file to be deployed to the PSoC6™. And in
the upper menu, select the connected Probe/Kit, click on connect and then on program.
The log section will show the progress and inform when the firmware deployment in the controller is completed.

For a detailed explanation on how to use the Cypress programmer tool, please consult its `User Guide <https://www.infineon.com/dgdl/Infineon-Infineon_Programmer_4.0_GUI_User_Guide-Software-v01_00-EN.pdf?fileId=8ac78c8c7e7124d1017ed9abca6e365c>`_.

You are all set now to start programming with micropython!

.. note::
    Alternatively, users familiar with the ModusToolbox™ toolchain and makefile system can also use the corresponding
    tools for flashing the provided .hex file on target PSoC6™ board. 

Serial prompt
-------------

With micropython deployed in your PSoC6™ board, you can access the REPL mode using
the USB-UART interface from the on-board debugger. 
This is equivalent to running the *python* commmand (without passing a script) in your command line terminal of your machine.

Use your preferred serial terminal software to serial connect to the board. Examples of serial
terminal software are the cross-platform `Putty <https://www.putty.org/>`_, which works for Windows and
Unix machines, or other software platform specific such as `Tera Term <https://ttssh2.osdn.jp/index.html.en>`_, or `minicom <https://en.wikipedia.org/wiki/Minicom>`_. 

Configure the serial connection with **115200 bauds** and **8-N-1** (8 bits frame, no parity and 1 stop
bit), and connect to the board port, the micropython REPL prompt will appear:

..
    .. image:: img/mpy-psoc6-repl.jpg
..
    :alt: Micropython REPL prompt
..
    :width: 360px

Running a script
----------------

In order to execute more ellaborated programs, and use the embedded device stand-alone you can write
python scripts.

There are several IDEs that you can install that integrate a text editor with the tools to run your
python script in your micropython device, as well as handling the files system of your micropython
device. Known examples are:

++++++++ to be completed with suported ones ++++++

In micropython there are primarily two ways to execute a script:  

Command line mode 
~~~~~~~~~~~~~~~~~

In this mode, the micropython PSoC6™ board is connected through the serial interface to the
host development machine. 
Each of the line will be executed in the controller. Any output like print messages in your application or
exceptions will be send through the serial connection to the host machine, which will display them
in the serial terminal console. 

In this case, the REPL mode is used, but the IDE or used command line tool will take care of sending
each line of the script and process its output to show it in the terminal.

Find more information about the different REPL mode options, in this section.
 
Stand-alone target mode
~~~~~~~~~~~~~~~~~~~~~~~

++