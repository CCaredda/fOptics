-----------------
Compile Opencv on Windows with msvc22
-----------------

1) Download opencv source (normal src + contrib)
2) Install cmake
   Install Visual Studio 2022 (with “Desktop development with C++” workload).
3) In C:\Downloads, create build dir next to opencv sources:
├─ C:\Downloads\opencv\
 │   ├─ opencv\
 │   ├─ opencv_contrib\
 │   └─ build\

4) Open developer MSVC command prompt with administrator privilege
5) go in build dir, for example:
cd C:\Users\PRIMES\Downloads\opencv\build
6) Compile Opencv and Opencv-contrib with cmake:

cmake ^
  -D CMAKE_BUILD_TYPE=Release ^
  -D CMAKE_INSTALL_PREFIX=C:\opencv\install ^
  -D OPENCV_EXTRA_MODULES_PATH=C:\Users\PRIMES\Downloads\opencv\opencv_contrib\modules ^
  -D BUILD_EXAMPLES=OFF ^
  -D BUILD_opencv_world=ON ^
  -D WITH_OPENMP=ON ^
  C:\Users\PRIMES\Downloads\opencv\opencv
  

cmake --build . --config Release --target INSTALL


7) Add C:\opencv\install\x64\vc17\bin to PATH environment variable





-----------------
Compile FFTW on Windows with msvc22
-----------------


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

cmake -D CMAKE_POLICY_VERSION_MINIMUM=3.5 -D CMAKE_BUILD_TYPE=Release -D CMALE_INSTALL_PREFIX=C:\fftw -D BUILD_SHARED_LIBS=OFF C:\Users\PRIMES\Downloads\fftw-3.3.10\src
cmake --build . --config Release --target INSTALL

7) Go to build_f dir, build fftw with float precision

cmake -D CMAKE_POLICY_VERSION_MINIMUM=3.5 -D CMAKE_BUILD_TYPE=Release -D CMALE_INSTALL_PREFIX=C:\fftw -D ENABLE_FLOAT=ON -D ENABLE_THREADS=ON -D BUILD_SHARED_LIBS=OFF C:\Users\PRIMES\Downloads\fftw-3.3.10\src
cmake --build . --config Release --target INSTALL



8) Go to build_openmp dir, build fftw with openmp
cmake -D CMAKE_POLICY_VERSION_MINIMUM=3.5 -D CMAKE_BUILD_TYPE=Release -D CMALE_INSTALL_PREFIX=C:\fftw -D ENABLE_FLOAT=ON -D ENABLE_OPENMP=ON -D ENABLE_THREADS=ON -D BUILD_SHARED_LIBS=OFF -DFFTW3F_LIB=../build/Release/fftw3f.lib C:\Users\PRIMES\Downloads\fftw-3.3.10\src
cmake --build . --config Release --target INSTALL

9) Move directory C:\Program Files (x86)\fftw to C:\fftw

10) Add C:\fftw\bin to PATH environment variable



-----------------
Install boost library
-----------------

1) Download boost binary for msvc compiler 64 bits
2) Execute the .exe file with administrator right and install the library in C:\boost

