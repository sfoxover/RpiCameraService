# RPiCamera
OpenCV Camera code for RPi4

## How to build for Windows / Linux

OpenCV
* [compiler] sudo apt-get install build-essential
* [required] sudo apt-get install cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
* [optional] sudo apt-get install python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libdc1394-22-dev
* sudo apt-get install libxvidcore-dev libx264-dev libavcodec-dev libavformat-dev libswscale-dev libv4l-dev

* mkdir ~/opencv
* cd ~/opencv
* git clone https://github.com/opencv/opencv.git
* git clone https://github.com/opencv/opencv_contrib.git
* cd opencv
* mkdir build
* cd build
* cmake .. -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D INSTALL_C_EXAMPLES=OFF -D INSTALL_PYTHON_EXAMPLES=OFF -D WITH_TBB=ON -D WITH_V4L=ON -D WITH_QT=OFF -D WITH_OPENGL=ON -D OPENCV_EXTRA_MODULES_PATH=~/opencv/opencv_contrib/modules -D BUILD_EXAMPLES=OFF ..
* make -j4
* sudo make install


ZeroMQ for Windows

* clone http://github.com/zeromq/libzmq
* build with cmake
* load project file in Visual Studio
* install 
* set ZeroMQ_DIR as environment variable

ZeroMQ for linux
* git clone https://github.com/zeromq/libzmq
* cd libzmq
* mkdir cmake-build && cd cmake-build
* cmake .. && make -j 4
* make test && make install && sudo ldconfig

DLib 
* go to site http://dlib.net/compile.html
* download latest zip file
* mkdir build
cd build
cmake ..
cmake --build . --config Release

Fix for header file
vim /usr/local/include/dlib/opencv/cv_image.h
line 37
temp = cvIplImage(img);

GPIO for C++
* https://github.com/JoachimSchurig/CppGPIO
* make -j4
* sudo make install
* make demo

Set permission for Ubuntu
* sudo chown root.gpio /dev/gpiochip*
* sudo chmod g+rw /dev/gpiochip*

GTest unit tests
* download https://github.com/google/googletest/archive/master.zip
* use CMake gui
build, install and set environment with gtest_DIR to path C:\Program Files (x86)\googletest-distribution\lib\cmake

CMake 3.16.2 Install on Ubuntu
* sudo apt-get purge cmake
* sudo apt-get install libssl-dev
* wget https://github.com/Kitware/CMake/releases/download/v3.16.2/cmake-3.16.2.tar.gz
* tar -xzvf cmake-3.16.2.tar.gz
* cd cmake-3.16.2
* ./bootstrap
* make -j4
* sudo make install


## Recommended extensions for Visual Studio Code
* "Remote Development" - Microsoft. https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.vscode-remote-extensionpack
* c/c++ - Microsoft
* CMake - twxs
* CMake Tools - vector-of-bool
* Native Debug - WebFreak
