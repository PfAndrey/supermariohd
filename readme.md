# README #

This is HD remake of super mario (1985).
Project was created in learning purposes only. 


################################ Ubuntu ################################

# 1. install git cmake and gcc
sudo apt-get -y install cmake git gcc 

# 2. install addtional packets (OpenGL, OpenAL etc)
sudo apt-get -y install libx11-dev libudev-dev xorg-dev freeglut3-dev libalut-dev libvorbis-dev libflac-dev

# 2. download mario repo 
git clone https://bitbucket.org/AParfen/supermariohd.git --recursive

# 3. build project 
chdir supermariohd
mkdir build && cd build
cmake ..
cmake --build . --config Release
./SuperMario

################################ Windows ################################

# 1. Install MinGW: 
	https://osdn.net/projects/mingw/releases/ 
	OR VisulStudio Community: https://visualstudio.microsoft.com/downloads/

# 2. Install Git: 
	https://git-scm.com/downloads

# 3. Install CMake: 
	https://cmake.org/download

# 4. Install OpenAL: 
	https://www.openal.org/downloads

# 5. Open git-bash and type:
	git clone https://bitbucket.org/AParfen/supermariohd.git --recursive
	cd supermariohd 
	./build.sh
	./SuperMario





