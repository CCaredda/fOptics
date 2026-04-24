#-------------------------------------------------
#
# Project created by QtCreator 2018-03-06T12:32:37
#
#-------------------------------------------------


QT       += core gui


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


QT +=   printsupport \
        concurrent \
        serialport \
        charts \
        multimedia

CONFIG += qt c++17

DEFINES += QT_NO_FLOAT16



include($$PWD/../Display/display.pri)
include($$PWD/../Acquisition/acquisition.pri)
include($$PWD/../Widget/Widget.pri)
include($$PWD/../Vision/vision.pri)
include($$PWD/../omigod/omigod.pri)


HEADERS += \
    HMain.h

SOURCES += \
    main.cpp \
    HMain.cpp

FORMS += \
    HMain.ui

# ---------- Linux (GCC) ----------
unix {
    QMAKE_CC = g++
    QMAKE_LINK = g++

    QMAKE_CXXFLAGS+= -fopenmp
    QMAKE_LFLAGS += -fopenmp "-Wl,--enable-new-dtags -Wl,-rpath"


    INCLUDEPATH += /usr/include/opencv4
    LIBS += -lopencv_core -lopencv_flann -lopencv_highgui -lopencv_core \
            -lopencv_imgproc -lopencv_objdetect -lopencv_features2d -lopencv_videoio \
            -lopencv_imgcodecs -lopencv_video \
            -lfftw3 -lfftw3f_omp -lfftw3f
}

# ---------- Windows + MSVC
win32:msvc {
    # Adjust this root if you extracted elsewhere
    OPENCV_ROOT = C:\opencv\install
    OPENCV_VER  = 4140           # e.g., 470 (4.7.0), 480, 490, 4100 (4.10.0)

    FFTW_ROOT = C:\fftw

    INCLUDEPATH +=  $$OPENCV_ROOT\include \
                    $$FFTW_ROOT\include \
                    C:\boost \
                    C:\vcpkg\installed\x64-windows\include

    LIBS += -L$$OPENCV_ROOT\x64\vc17\lib -lopencv_world$${OPENCV_VER} \
            -L$$FFTW_ROOT\lib -lfftw3 -lfftw3f -lfftw3f_omp \
            -LC:\vcpkg\installed\x64-windows\lib -lavcodec -lavformat -lavutil -lswscale -lavfilter -lswresample

    QMAKE_CXXFLAGS += /openmp


    VCPKG_BIN = C:/vcpkg/installed/x64-windows/bin

    QMAKE_POST_LINK += copy /Y $$shell_path($$VCPKG_BIN/avformat-62.dll)  $$shell_path($$OUT_PWD/release) &
    QMAKE_POST_LINK += copy /Y $$shell_path($$VCPKG_BIN/avcodec-62.dll)   $$shell_path($$OUT_PWD/release) &
    QMAKE_POST_LINK += copy /Y $$shell_path($$VCPKG_BIN/avutil-60.dll)    $$shell_path($$OUT_PWD/release) &
    QMAKE_POST_LINK += copy /Y $$shell_path($$VCPKG_BIN/swscale-9.dll)    $$shell_path($$OUT_PWD/release) &
    QMAKE_POST_LINK += copy /Y $$shell_path($$VCPKG_BIN/swresample-5.dll) $$shell_path($$OUT_PWD/release) &
    QMAKE_POST_LINK += copy /Y $$shell_path($$VCPKG_BIN/swresample-6.dll) $$shell_path($$OUT_PWD/release) &
    QMAKE_POST_LINK += copy /Y $$shell_path($$VCPKG_BIN/avfilter-11.dll)  $$shell_path($$OUT_PWD/release)

}


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
