\mainpage
 
 \section intro_sec Introduction
 

This software allows to acquire RGB or Hyperspectral images, record theses images in an hardrive and process it in order to identify activated functional brain areas during a neurosurgical operation.

Papers related to this work are located in the directory papers.

Note that if you want to use other cameras than one used in this project you have to change th class ABaslerCamera.
The best practice should be to add a new class that should refer to the new camera.
Make sure to use the same names public functions, public slots and signals than the ones used in ABaslerCamera. 

If no camera is detected, a video dataset (set of images or video) can be loaded. These images are then processed in a standard way.

The code developed during the Pulsalys project "NEUROIMAGERIE" is contained in the PCoeff class.


 
 \section install_sec Installation on Fedora
This program is coded in C++ with the framework Qt and the compiler g++.

First Qt should be installed (>=6.10.1)
- Download the Qt online installer : https://www.qt.io/download-qt-installer?hsCtaTracking=99d9dd4f-5681-48d2-b096-470725510d34%7C074ddad0-fdef-4e53-8aa8-5e8a876d6ab4
- Install Qt with its additional libraries (custom installation) : Qt5 Compatibility mode, Qt charts, Qt multimedia, Qt Sensors, Qt serial bus, Qt serial port, Qt Shader tools

Different libraries and framework should be installed in order to use the softwares properly. 
- Opencv (latest version): sudo dnf install opencv opencv-contrib opencv-doc python3-opencv python3-matplotlib python3-numpy
- Boost (latest version): sudo dnf install boost boost-devel
- fftw (latest version): sudo dnf install fftw fftw-devel
- The Pylon framework has to be installed to acquire images from a Basler camera. Please download the installation guidelines here: https://www.baslerweb.com/en/sales-support/downloads/software-downloads/pylon-5-1-0-linux-x86-64-bit-debian/

The file has a .deb extension and need to be converted into .rpm to be installed under fedora:

- First install alien: sudo dnf intall alien
- Convert the deb file into rpm: alien -r pylon*.deb
- Install the rpm file: sudo rpm -i pylon*.rpm

Depending on the version of Pylon, symbolic links need to be created:
- cd /opt/pylon5/lib64
- sudo ln -s libGCBase_gcc_v3_1_Basler_pylon_v5_1.so libGCBase.so
- sudo ln -s libGenApi_gcc_v3_1_Basler_pylon_v5_1.so libGenApi.so


\section sec_deployment_Fedora Deployment on Fedora
Deployment scripts are located in the folder script.

- To deploy the soft on Fedora, please download the last release of the appimage of 
linuxdeploy-x86_64.AppImage (https://github.com/linuxdeploy/linuxdeploy/releases/tag/1-alpha-20251107-1) and 
linuxdeploy-plugin-qt-x86_64.AppImage (https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/tag/1-alpha-20250213-1)
- Place it in the following directory: ~/Soft/ and make it executable (chmod +x linuxdeploy*)
- In .bashrc file add the line: PATH: export PATH="~/Soft:$PATH"
- [If necessary] In the script script/linux_deploy_app.sh, adapt the path of the current Qt version
- Run the script linux_deploy.sh to create an appimage of the soft: ./linux_deploy.sh ~/Soft
It will create the an appImage of the soft (standalone) that could be use on other computers having the same fedora distribs.




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

The software architecture is roughly represented by the next figure (to simplify the reading, not all classes are represented in this schematic). The following steps describe how the software works :

- 1) The software automatically tries to connect an available Basler camera. If a camera is connected the graphical interface contained in HCameraPamars is displayed, otherwise, a video dataset (set of images or video) can be loaded and the graphical interface contained in HLoadDatas is displayed.
- 1) The user uses the graphical interface to launch data acquisition or to load dataset.
- 2) Acquired or loaded images are sent to the class APostAcquisition to compute pre processing (image cropping and undersampling).
- 3) Pre processed images are sent to the class ARegistration to compensate the motionin the images using the algorithm developped by Michaël Sdika https://doi.org/10.1016/j.media.2018.12.005
- 4) Images with motion compensated are sent to the class PExtractData for data extraction. The images are formatted in a single table according to the region of interest defined by the user (or automatically calculated, see PAutomatic_ROI_Detection).
- 5) Formated images are stored into the computer RAM.
- 6) When all images have been stored into the computer RAM, temporal filtering is computed (low-pass fitering, data correction, cardiac filtering)
- 7) Data filtered with the cardiac filter are sent to the PCoeff class. In this class, the automatic calibration procedure is executed in order to get a matrix of coefficient to automatically quantify hemodynamics in tissue.
- 8) The autocalibration matrix is sent to the PAnalysis class for quantifying hemodynamics in tissue using low-pass filtered data (filtered in PFiltering). 
- 9) The hemodynamic contrasts are obtained by projecting the absorbance changes measured at the surface of the tissue on the orthonormal basis calculated in step 7).
- 10) The P_SPM class is used to calculate statistic inferences. It allows to obtain t-stats matrix and a binary mask that indicates the extent of the activated functional brain areas.
- 11) The activation masks are merged with the initial image in HdisplayResult class. Then the class HImageDisplay is used to display results.


 \image html software_architecture_RT_process.png "Software architecture" width=1500px
 

 \section sec_user_guide User guide

[You can find the user guide here: doc_src/usage_doc.odt](../../usage_doc.odt)

 






