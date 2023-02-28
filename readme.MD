# Compilation

## With mingw1120_64
Config: Qt_6_4_2_MinGW_64_bit-Debug
OpenCV has to be recompiled with mingw1120_64
The library exiv2 (used for getting iso from photo) is too difficult to compile (too much dependencies)
Swithching on mscv2019-64 compiler

## With mscv2019-64
The compiler has to be installed with Microsoft Visual Sutdio Community
Windows Developper SDK has to be installed to link the user32.lib (for windows.h functions)


# Installation

After compilation, copy following files / folders in Release directory:
- settings.ini
- ressources/
- lib\opencv\x64\mingw\bin\libopencv_core470.dll
- libopencv_imgcodecs470.dll
- libopencv_imgproc470.dll
- libopencv_videoio470.dll


# Libraries

## qtEXIF2File
cexif.h
https://github.com/birkeh/qtEXIF2File
