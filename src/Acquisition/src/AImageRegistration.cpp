#include "AImageRegistration.h"

AImageRegistration::AImageRegistration(QObject *parent) : QObject(parent)
{

    //registration progress
    connect(&_M_reg_RGB,SIGNAL(newRegistrationProgress(QString,int)),this,SIGNAL(newRegistrationProgress(QString,int)));

    //NCC results
    connect(&_M_reg_RGB,SIGNAL(registrationIsFinished()),this,SIGNAL(registrationIsFinished()));

    //Send registered image to analysis
    connect(&_M_reg_RGB,SIGNAL(newRegisteredImage(_Processed_img)),this,SIGNAL(newRegisteredImage(_Processed_img)));

}


AImageRegistration::~AImageRegistration()
{

}

//enable motion compensation
void AImageRegistration::enableMotionCompensation(bool v)
{
    _M_reg_RGB.enableMotionCompensation(v);
}


//Data processing is ready
void AImageRegistration::DataAcquisitionIsReady()
{
    _M_reg_RGB.DataAcquisitionIsReady();
}


//Set analysis zone (for NCC measure)
void AImageRegistration::setAnalysisZone(QVector<QPoint> c)
{
    _M_reg_RGB.setAnalysisZone(c);
}


//Set the number of frames to register
void AImageRegistration::setTotalNbFrames(int v)
{
    _M_reg_RGB.setTotalNbFrames(v);
}

//Set transformation model
void AImageRegistration::setTransfoModel()
{
    _M_reg_RGB.setTransfoModel();
}

//Called when a new registration type is selected
void AImageRegistration::newRegistrationtype(int v)
{
    _M_reg_RGB.newRegistrationtype(v);

}


void AImageRegistration::setFirstImg(Mat &img)
{
    _M_reg_RGB.setFirstImg(img);
}

//Request parallel thread process
void AImageRegistration::requestParallelThreadProcess(_Processed_img img)
{
    _M_reg_RGB.requestParallelThreadProcess(img);
}


