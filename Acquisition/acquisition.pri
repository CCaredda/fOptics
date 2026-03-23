# Sub-project that contains classes to acquire and pre-process images


INCLUDEPATH+= $$absolute_path(./inc)


QT += printsupport



HEADERS += \
   # $$PWD/inc/xiApiPlusOcv.hpp \
   # $$PWD/inc/AXimeaCamera.h \
    $$PWD/inc/conversion.h \
    $$PWD/inc/ARegistration.h \
    $$PWD/inc/AloadDatas.h \
    $$PWD/inc/loadinfos.h \
    $$PWD/inc/APostAcquisition.h \
    $$PWD/inc/AImageRegistration.h \
    $$PWD/inc/AVideoProcessor.h




SOURCES += \
   # $$PWD/src/xiApiPlusOcv.cpp \
   # $$PWD/src/AXimeaCamera.cpp \
    $$PWD/src/AVideoProcessor.cpp \
    $$PWD/src/conversion.cpp \
    $$PWD/src/ARegistration.cpp \
    $$PWD/src/AloadDatas.cpp \
    $$PWD/src/loadinfos.cpp \
    $$PWD/src/APostAcquisition.cpp \
    $$PWD/src/AImageRegistration.cpp



