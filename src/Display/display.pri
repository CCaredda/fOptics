# Sub-project that contains all user interface classes

INCLUDEPATH+= $$absolute_path(./inc)

SOURCES +=  \
    $$PWD/src/HChooseAnalysis.cpp \
    $$absolute_path(.)/src/HImageDisplay.cpp \
    $$PWD/src/HPLotProfile.cpp \
    $$PWD/src/HAcquisitionDisplay.cpp \
    $$PWD/src/HLoadDatas.cpp \
    $$PWD/src/HMainDisplay.cpp \
    $$PWD/src/HTimeProcess.cpp \
    $$PWD/src/HAnalysisParams.cpp \
    $$PWD/src/HdisplayResult.cpp \
    $$PWD/src/HPlot.cpp \
    $$PWD/src/HStatistics.cpp \
    $$PWD/src/HROI.cpp \
    $$PWD/src/HSaveData.cpp


HEADERS  += \
    $$PWD/inc/HChooseAnalysis.h \
    $$absolute_path(.)/inc/HImageDisplay.h \
    $$PWD/inc/HPLotProfile.h \
    $$PWD/inc/HAcquisitionDisplay.h \
    $$PWD/inc/HLoadDatas.h \
    $$PWD/inc/HMainDisplay.h \
    $$PWD/inc/HTimeProcess.h \
    $$PWD/inc/HAnalysisParams.h \
    $$PWD/inc/HdisplayResult.h \
    $$PWD/inc/HPlot.h \
    $$PWD/inc/HStatistics.h \
    $$PWD/inc/HROI.h \
    $$PWD/inc/HSaveData.h

FORMS += \
    $$PWD/ui/HChooseAnalysis.ui \
    $$PWD/ui/HPLotProfile.ui \
    $$PWD/ui/HAcquisitionDisplay.ui \
    $$PWD/ui/HLoadDatas.ui \
    $$PWD/ui/HMainDisplay.ui \
    $$PWD/ui/HTimeProcess.ui \
    $$PWD/ui/HAnalysisParams.ui \
    $$PWD/ui/HdisplayResult.ui \
    $$PWD/ui/HPlot.ui \
    $$PWD/ui/HStatistics.ui \
    $$PWD/ui/HROI.ui \
    $$PWD/ui/HSaveData.ui



QT += printsupport
