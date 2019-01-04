![MARIO logo](https://orig00.deviantart.net/8ed6/f/2014/134/2/9/classic_mario_bros__3d_by_cezkid-d78civ3.gif)
# SuperMarioHD 
----
Cross-platform HD remake of super mario (NES, 1985).  
Project was created for learning purposes only.  

## Authors
* Andriy Parfenyuk.

# Windows
----
**1. Install a C++11 compiler:**  
Visual Studio Community: https://visualstudio.microsoft.com/downloads/ or MinGW https://osdn.net/projects/mingw/releases/

**2. Install Git:**  
	https://git-scm.com/downloads

**3. Install CMake:**  
	https://cmake.org/download

**4. Install OpenAL:**  
	https://www.openal.org/downloads

**5. Open git-bash and type:**  
```console
git clone https://github.com/PfAndrey/supermariohd.git --recursive
cd supermariohd 
mkdir build && cd build
cmake ..
cmake --build . --config Release
./SuperMario
```

# Ubuntu
----
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
git clone https://github.com/PfAndrey/supermariohd.git --recursive
```

**4. build project** 
```console
cd supermariohd
mkdir build && cd build
cmake ..
cmake --build . --config Release
./SuperMario
```
