# Motion compensation project coded by Michaël Sdika : https://doi.org/10.1016/j.media.2018.12.005.

INCLUDEPATH+= \
    $$absolute_path(./src) \
    $$absolute_path(./3rdparty/fsl)


QT += printsupport



HEADERS += \
    $$PWD/src/Timer.h \
    $$PWD/src/SparseOFPCA.h \
    $$PWD/src/OFReg.h \
    $$PWD/src/SparseOFRBF.h \
    $$PWD/src/SparseOF.h \
    $$PWD/src/Reg.h \
    $$PWD/src/NiftiIO.h \
    $$PWD/src/KptConstraint.h \
    $$PWD/src/KernelOper.h \
    $$PWD/3rdparty/fsl/znzlib.h \
    $$PWD/3rdparty/fsl/nifti1_io.h \
    $$PWD/3rdparty/fsl/nifti1.h \
    $$PWD/3rdparty/fsl/fslio.h \
    $$PWD/3rdparty/fsl/dbh.h

SOURCES += \
    $$PWD/src/Timer.cpp \
    $$PWD/src/SparseOFPCA.cpp \
    $$PWD/src/SparseOFRBF.cpp \
    $$PWD/src/SparseOF.cpp \
    $$PWD/src/Reg.cpp \
    $$PWD/src/OFReg.cpp \
    $$PWD/src/NiftiIO.cpp \
    $$PWD/src/KptConstraint.cpp \
    $$PWD/src/KernelOper.cpp \
    $$PWD/3rdparty/fsl/znzlib.c \
    $$PWD/3rdparty/fsl/nifti1_io.c \
    $$PWD/3rdparty/fsl/fslio.c
