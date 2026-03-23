# Sub-project that contains all classes for image analysis


INCLUDEPATH+= $$absolute_path(./inc)


QT += printsupport

HEADERS += \
    $$PWD/inc/oxygenation.h \
 #   $$PWD/inc/PData.h \
    $$PWD/inc/vblobForeach.h \
    $$PWD/inc/polyfit.hpp \
    $$PWD/inc/filtering.h \
    $$PWD/inc/pobject.h \
    $$PWD/inc/acquisition.h \
    $$PWD/inc/PAnalyse.h \
    $$PWD/inc/Statistic_functions.h \
    $$PWD/inc/PFiltering.h \
    $$PWD/inc/PDataExtracting.h \
    $$PWD/inc/PPixelWiseMolarCoeff.h \
    $$PWD/inc/molarcoefffunctions.h \
    $$PWD/inc/PProcessTimes.h \
    $$PWD/inc/PModifiedBeerLambertLaw.h \
    $$PWD/inc/P_SPM.h \
    $$PWD/inc/logger.h

SOURCES += \
    $$PWD/src/logger.cpp \
    $$PWD/src/oxygenation.cpp \
   # $$PWD/src/PData.cpp \
    $$PWD/src/vblobForeach.cpp \
    $$PWD/src/filtering.cpp \
    $$PWD/src/acquisition.cpp \
    $$PWD/src/PAnalyse.cpp \
    $$PWD/src/Statistic_functions.cpp \
    $$PWD/src/PFiltering.cpp \
    $$PWD/src/PDataExtracting.cpp \
    $$PWD/src/PPixelWiseMolarCoeff.cpp \
    $$PWD/src/molarcoefffunctions.cpp \
    $$PWD/src/PProcessTimes.cpp \
    $$PWD/src/PModifiedBeerLambertLaw.cpp \
    $$PWD/src/P_SPM.cpp


