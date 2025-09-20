# SuperMarioHD
![Win64](https://img.shields.io/github/actions/workflow/status/PfAndrey/SuperMarioHD/win64-build.yml?branch=master&label=windows%20x64&logo=windows&logoColor=white)
![Ubuntu](https://img.shields.io/github/actions/workflow/status/PfAndrey/SuperMarioHD/linux-build.yml?branch=master&label=ubuntu%20gcc&logo=ubuntu&logoColor=white)
![Latest Tag](https://img.shields.io/github/v/tag/PfAndrey/SuperMarioHD?display_name=tag)
---
![MARIO logo](https://github.com/PfAndrey/supermariohd/blob/master/res/Screen.jpg?raw=true)
Cross-platform HD remake of Super Mario Bros. (NES, 1985).
Project was created for learning purposes only.
The game demo on the YouTube: https://youtu.be/RJ181cc_AMI

## Windows
**1. Install a C++11 compiler:**  
Visual Studio Community: https://visualstudio.microsoft.com/downloads/ or MinGW https://osdn.net/projects/mingw/releases/

**2. Install Git:**  
	https://git-scm.com/downloads

**3. Install CMake:**  
	https://cmake.org/download

**4. Open git-bash and type:**
```console
git clone https://github.com/PfAndrey/supermariohd.git
cd supermariohd 
mkdir build && cd build
cmake ..
cmake --build . --config Release
./SuperMario
```

## Ubuntu
**1. install git cmake and g++**
```console
sudo apt-get -y install cmake git g++ 
```

**2. install additional packets (OpenGL, OpenAL etc)**
```console
sudo apt-get -y install libx11-dev libudev-dev xorg-dev freeglut3-dev libalut-dev libvorbis-dev libflac-dev
```

**3. download mario repo**
```console
git clone https://github.com/PfAndrey/supermariohd.git
```

**4. build project** 
```console
cd supermariohd
mkdir build && cd build
cmake ..
cmake --build . --config Release
./SuperMario
```

## Mac OS
**1. install xcode:**    
    https://developer.apple.com/xcode/

**2. Install Git:**  
    https://git-scm.com/downloads

**3. Install CMake:**  
    https://cmake.org/download

**4. build project**   
Go to terminal and type:
```console
git clone https://github.com/PfAndrey/supermariohd.git --recursive
cd supermariohd
mkdir build && cd build
cmake ..
sudo cmake --build . --config Release
./SuperMario
```
