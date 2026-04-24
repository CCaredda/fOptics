## Sub-projet that contains class of graphic interfaces used in the sub-project display


INCLUDEPATH+= $$absolute_path(./inc)

SOURCES +=  \
    $$absolute_path(.)/src/HValueEdit.cpp \
    $$absolute_path(.)/src/HKeypad.cpp \
    $$PWD/src/WClickableLabel.cpp


HEADERS  += \
    $$absolute_path(.)/inc/HValueEdit.h \
    $$absolute_path(.)/inc/HKeypad.h \
    $$PWD/inc/WClickableLabel.h 


FORMS += \
    $$absolute_path(.)/ui/HKeypad.ui 

#RESOURCES += $$absolute_path(.)/hmi.qrc

QT += printsupport


