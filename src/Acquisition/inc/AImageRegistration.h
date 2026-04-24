/**
 * @file AImageRegistration.h
 *
 * @brief This class is the motion compensation scheduler.
 * Two objects are declared here that aims to compensate motion in RGB and hyperspectral images.
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef AIMAGEREGISTRATION_H
#define AIMAGEREGISTRATION_H

#include <QObject>
#include "ARegistration.h"

class AImageRegistration : public QObject
{
    Q_OBJECT
public:
    explicit AImageRegistration(QObject *parent = nullptr);
    ~AImageRegistration();

    /** Data processing is ready */
    void DataAcquisitionIsReady();


    /** Set analysis zone (for NCC measure) */
    void setAnalysisZone(QVector<QPoint> c);

    /** Set the number of frames to register */
    void setTotalNbFrames(int v);

    /** Set transformation model */
    void setTransfoModel();

    /** Called when a new registration type is selected */
    void newRegistrationtype(int v);

    /** set First image (RGB) */
    void setFirstImg(Mat &img);

signals:
    /** Registration is finished */
    void registrationIsFinished();

    /** new registered image (RGB) */
    void newRegisteredImage(_Processed_img);

    /**  registration progress */
    void newRegistrationProgress(QString,int);

public slots:
    /** Request parallel thread process (RGB) */
    void requestParallelThreadProcess(_Processed_img img);


    /** enable motion compensation */
    void enableMotionCompensation(bool);

    /** Set result directory */
    void onNewResultDirectory(QString v)    {_M_reg_RGB.onNewResultDirectory(v);}

private:

    //Registration class
    ARegistration                   _M_reg_RGB;

};

#endif // AIMAGEREGISTRATION_H
