# Git Check out commands
git clone https://github.com/sfoxover/RpiCameraService.git

## Update submodules after initial init command
git submodule update --progress --recursive -- "Imports/AiDetectLib"
git submodule update --progress --recursive -- "Imports/GTestLib"
git submodule update --progress --recursive -- "Imports/JsoncppLib"
git submodule update --progress --recursive -- "Imports/MessagesLib"

## git switch submodule to origin/master so you can commit submodule to the server instead of a local headless branch
git checkout -B master remotes/origin/master --

## Recommended extensions for Visual Studio Code
* "Remote Development" - Microsoft. https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.vscode-remote-extensionpack
* c/c++ - Microsoft
* CMake - twxs
* CMake Tools - vector-of-bool
* Native Debug - WebFreak

# RPiCamera
OpenCV Camera code for RPi4

## How to build for Linux

## OpenCV
* sudo apt-get install build-essential
* sudo apt-get install cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
* sudo apt-get install python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libdc1394-22-dev
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

## Fix for header file build error
vim /usr/local/include/dlib/opencv/cv_image.h
line 37
temp = cvIplImage(img);


## ZeroMQ 
* git clone https://github.com/zeromq/libzmq
* cd libzmq
* mkdir build && cd build
* cmake .. 
* make -j 4
* sudo make install 
* sudo ldconfig

## DLib 
* go to site http://dlib.net/compile.html
* download latest zip file
* wget http://dlib.net/files/dlib-19.19.zip
* unzip dlib-19.19.zip
* cd dlib-19.19
* mkdir build
* cd build
* cmake ..
* cmake --build . --config Release
* sudo make install 

GPIO for C++ for old linux kernels
* https://github.com/JoachimSchurig/CppGPIO
* make -j4
* sudo make install
* make demo

Set permission for Ubuntu Kernel 4.8 +
* sudo chown root.gpio /dev/gpiochip*
* sudo chmod g+rw /dev/gpiochip*

## GTest unit tests
* download https://github.com/google/googletest/archive/master.zip
* unzip master.zip
* cd master
* mkdir build
* cd build
* cmake ..
* cmake --build . --config Release
* sudo make install 

## CMake 3.16.2 Install on Ubuntu
* sudo apt-get purge cmake
* sudo apt-get install libssl-dev
* wget https://github.com/Kitware/CMake/releases/download/v3.16.2/cmake-3.16.2.tar.gz
* tar -xzvf cmake-3.16.2.tar.gz
* cd cmake-3.16.2
* ./bootstrap
* make -j4
* sudo make install

