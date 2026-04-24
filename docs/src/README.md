\mainpage
 
 \section intro_sec Introduction
 
**fOptics** is a software tool for processing intraoperative RGB video or RGB image data to identify activated functional brain areas during neurosurgical operations.  




 \section install_sec_win Installation on Windows
 
 This program is coded in C++ with the framework Qt and the Microsoft Visual Studio 2022.

Install Visual Studio 2022 (not via Visual studio installer because it will install Visual studio 2026. Version 2022 is required to work with Qt.

Download visual studio professional 2022: https://my.visualstudio.com/Downloads?q=visual%20studio%202022&wt.mc_id=o~msft~vscom~older-downloads

Install visual Studio professional 2022, and tick the box Desktop Development with C++
Click on install
	
Download qt installer: https://www.qt.io/development/download-qt-installer-oss
- CLick on personalized installation
- Tick these boxes: 
	- Qt/ Qt version/ MSVC 20022 (or more recent)
	- Qt5 Compatibility Module, Qt Charts, Qt Graphs, Qt Multimedia, Qt Sensors, Qt Serial bus, Qt Serial Port, Qt shader tools, Qt Task Tree
-do not tick MinGW compiler
	
Download cmake binary and install it





\subsection compile_opencv Compile Opencv on Windows with msvc22

1) Download opencv
	- source https://github.com/opencv/opencv
	- contrib https://github.com/opencv/opencv_contrib
	
2) In C:\Downloads, create build dir next to opencv sources:
├─ C:\Downloads\opencv\

 │   ├─ opencv\
 
 │   ├─ contrib\
 
 │   └─ build\

3) Open Developer PowerShell for VS 2022 with administrator privilege
4) go in build dir, for example:
cd C:\Users\PRIMES\Downloads\opencv\build

5) Compile Opencv and Opencv-contrib with cmake:

cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=C:\opencv\install -D OPENCV_EXTRA_MODULES_PATH=C:\Users\ccaredda\Downloads\opencv\contrib\modules -D BUILD_EXAMPLES=OFF -D BUILD_opencv_world=ON -D WITH_OPENMP=ON C:\Users\ccaredda\Downloads\opencv\opencv
  
cmake --build . --config Release --target INSTALL


6) Add C:\opencv\install\x64\vc17\bin to PATH environment variable


\subsection compile_fftw Compile FFTW on Windows with msvc22

1) Download fftw3 source

2) Install cmake

   Install Visual Studio 2022 (with “Desktop development with C++” workload).
   
3) In C:\Downloads, create build dir next to opencv sources:
├─ C:\Downloads\fftw-3.3.10\

 │   ├─ src\
 
 │   └─ build\
 
 │   └─ build_f\
 
 │   └─ build_openmp\		

4) Open developer MSVC command prompt with administrator privilege
5) go in build dir, for example:
cd C:\Users\PRIMES\Downloads\fftw-3.3.10\build
6) Compile fftw with cmake (double precision):

cmake -D CMAKE_POLICY_VERSION_MINIMUM=3.5 -D CMAKE_BUILD_TYPE=Release -D CMALE_INSTALL_PREFIX=C:\fftw -D BUILD_SHARED_LIBS=OFF C:\Users\ccaredda\Downloads\fftw-3.3.10\src

cmake --build . --config Release --target INSTALL

7) Go to build_f dir, build fftw with float precision

cmake -D CMAKE_POLICY_VERSION_MINIMUM=3.5 -D CMAKE_BUILD_TYPE=Release -D CMALE_INSTALL_PREFIX=C:\fftw -D ENABLE_FLOAT=ON -D ENABLE_THREADS=ON -D BUILD_SHARED_LIBS=OFF C:\Users\ccaredda\Downloads\fftw-3.3.10\src

cmake --build . --config Release --target INSTALL



8) Go to build_openmp dir, build fftw with openmp

cmake -D CMAKE_POLICY_VERSION_MINIMUM=3.5 -D CMAKE_BUILD_TYPE=Release -D CMALE_INSTALL_PREFIX=C:\fftw -D ENABLE_FLOAT=ON -D ENABLE_OPENMP=ON -D ENABLE_THREADS=ON -D BUILD_SHARED_LIBS=OFF -DFFTW3F_LIB=../build/Release/fftw3f.lib C:\Users\ccaredda\Downloads\fftw-3.3.10\src

cmake --build . --config Release --target INSTALL

9) Move directory C:\Program Files (x86)\fftw to C:\fftw

10) Add C:\fftw\lib to PATH environment variable


\subsection compile_boost Install boost library

1) Download boost binary for msvc compiler 64 bits
2) Execute the .exe file with administrator right and install the library in C:\boost



\subsection Install ffmpeg for video reading

cd C:\

git clone https://github.com/microsoft/vcpkg.git 

cd vcpkg  

.\bootstrap-vcpkg.bat                                  
                                                                                          
.\vcpkg install ffmpeg:x64-windows   


\subsection compile_project Project Compilation

Before compiling, make sure that the .pro file link to the correct OpenCV version.
Change the line OPENCV_VER  = 4140 according to your OpenCV version (check filenames in C:\opencv\install\x64\vc17\lib




\section Correct video acquired from surgical microscopes

Videos acquired from surgical microscopes could be corrupted. If frames are not read, or the soft crash, it could be due to a corruption in the video file.
In that case, you need to correct the video:


ffmpeg -fflags +genpts -i Segment01.mp4 \
-vf "setpts=PTS-STARTPTS,fps=60000/1001" \
-af "asetpts=PTS-STARTPTS" \
-c:v libx264 -preset fast -crf 18 \
-c:a aac -b:a 192k \
-movflags faststart \
Segmentfixed_01.mp4






\section sec_deployment_Windows Deployment on Windows

To deploy the software for Windows users, use the virtual machine (foptics_windows_img with boxes).
Deployment scripts are located in the folder script.

Execute the bat script windows_create_venv.bat. [If necessary] In the script, adapt the path of the current Qt version.

 \section sec_soft_architecture Software architecture

The software architecture is roughly represented by the next figure (to simplify the reading, not all classes are represented in this schematic).
 \image html software_architecture_RT_process.png "Software architecture" width=1500px
 

 \section sec_user_guide User guide

[You can find the user guide here: doc_src/usage_doc.odt](../../usage_doc.odt)

 






