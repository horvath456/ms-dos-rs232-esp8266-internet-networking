# MS-DOS RS232 ESP8266 Driver

This is the MS-DOS driver for the ESP8266. It communicates over a custom simple RS232 library with the device. 


## Prerequisites to compile

### Dosbox
To compile and run this project, you need dosbox.

Download it here: https://www.dosbox.com/download.php?main=1


#### Dosbox config file
You might also want to edit the dosbox config file (usually located under ~/.dosbox in linux)
and append lines to autoexec.bat (the bottom of the config file),
for example a "keyb gr" command or the mount command for c: (this msdos folder should be mounted to C:, so the compile script works properly)


### OpenWatcom
The compiler used is OpenWatcom: http://www.openwatcom.org/download.php

Download the DOS Version and install it in your Dosbox C:\ Drive under C:\WATCOM (default path)

Install the 16bit and 32bit compiler for dos.


## Compile
To compile the project, run COMPILE.BAT in the DRIVER directory.


## Run
To run the project, execute ESP8266.EXE in the DRIVER directory.


## Use it in your own project
Use the header files and the library file in the DIST directory to include the driver in your own projects.