/**
 * @file ARegistration.h
 *
 * @brief Real time motion compensation of RGB images.
 * This class aims a direct link to code provided by Michaël Sdika : https://doi.org/10.1016/j.media.2018.12.005.
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef AREGISTRATION_H
#define AREGISTRATION_H

#include <QCoreApplication>
#include <QDir>
#include <QObject>
#include <QThread>
#include <QDebug>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/tracking.hpp>

#include <iostream>
#include <vector>
#include <stdio.h>
#include <ctime>

#include <OFReg.h>
#include <SparseOFPCA.h>
#include "conversion.h"
#include "pobject.h"
#include "acquisition.h"
#include <QMutex>

#include <QElapsedTimer>

//#include <Timer.h>

using namespace cv;
using namespace std;
using namespace omigod;


class ARegistration : public QThread
{
    Q_OBJECT
public:
    explicit ARegistration(QObject *parent = 0);
    ~ARegistration();

    /** Set the first Img you want to consider for the registration */
    void setFirstImg(Mat &img);



    /** New registration type */
    void newRegistrationtype(int);

    /**  Set Harris dist */
    void setHarrisDistValue(double v)   {_M_harrisDist=v;newRegistrationtype(_M_reg_type);}
    /**  Set Nb of learning frames */
    void setNbLearningFrames(double v)  {_M_Lnframe=v;newRegistrationtype(_M_reg_type);}
    /** Set Nb of Key Points */
    void setNbKeyPoints(double v)       {_M_maxkey=v;newRegistrationtype(_M_reg_type);}
    /** Set Ponderation sigma */
    void setSigDyValue(double v)        {_M_dySig=v;newRegistrationtype(_M_reg_type);}

    /** Set FileNames */
    void setTransfoModel();

    /** Set the number of frames to register */
    void setTotalNbFrames(int v);

    /** Set analysis zone (for NCC measure) */
    void setAnalysisZone(QVector<QPoint> c);
    /**  Get analysis zone */
    vector<Point> getAnalysisZone();

    /** Get NCC result */
    QVector<QVector<float> > getNCCResults()    {return _M_NCC;}

    /** Inform that the rest of the software is ready to store the registred images */
    void DataAcquisitionIsReady()   {_M_soft_ready = true;}

    /** Enable motion compensation */
    void enableMotionCompensation(bool v)   {_M_enable_motion_compensation=v;}


signals:
    /** Registration is finished */
    void registrationIsFinished();

    /** new registered image */
    void newRegisteredImage(_Processed_img);

    /**  registration progress */
    void newRegistrationProgress(QString,int);


public slots:
    /** Request parallel thread process */
    void requestParallelThreadProcess(_Processed_img img);

    /** New result directory */
    void onNewResultDirectory(QString v)    {_M_result_directory = v;}


protected:
    /**  Call parallel thread */
    void run();

private:
    //PRIVATE FUNCTION
    /** init sparse of PCA analysis */
    void initSparseofPCA();

    /** register image */
    void _RegisterImage(_Processed_img );

    /** Receive image to process learning */
    void _LearnModel(_Processed_img img);


    //Video frames infos
    int _M_Lnframe;     //nb of frames used for learning
    int _M_tot_reg_frames; //Nb of frames to register
    int _M_id_learning_frame; //id learning frame

    //Registration infos
    int _M_reg_type;    //Registration type (0 : of; 1 : tvl1, 2 : ofpca)


    //SparseOfPCA registration
    SparseOFPCA _M_sparseofPCA;
    int _M_maxkey;          //max num of keypoint for sparse reg
    double _M_dySig;        //sigma for init weighting with dy
    double _M_harrisDist;   //min dist between harris keypoints
    double _M_harrisQL;     //harris quality level
    int _M_npyr;            //1st param : learning number of level in the pyramid
    int _M_ws;              //2nd param : Learning window size
    int _M_iter;            //number of iterations per level
    int _M_Lnpyr;            //1st param : learning number of level in the pyramid
    int _M_Lws;              //2nd param : Learning window size
    int _M_Liter;            //number of iterations per level

    double _M_irlsSig0;     //sigma at iteration 0 for IRLS
    int _M_irlsN;           //number of IRLS iterations
    double _M_constPCA0;    //delta in the constraint on the pca coeff: ||L(lpca-lpca0)||^2/npca < delta
    string _M_ptsFit;       //Lagrangian (LAG), Large def Lag (LLAG), Eulerian (EULER)
    bool  _M_camcomp;       // compose camera motion model (for ofpca)
    int _M_border;          //border size (remove kpt in the border)


    Reg * _M_reg;       //Registration objects
    OFReg _M_ofreg;
    Reg * _M_lreg;      //Registration objects of learning phase
    OFReg _M_lofreg;
    Reg * _M_creg;      // current registration object


//    Mat     _M_NCCframe0; //first frame for NCC measure
    Mat     _M_gframe0; //First frame for learning

    bool    _M_enable_process;  //Flag to enable the process

    //Check if learning model has been done
    bool    _M_learning_Model_is_done;

    //Check if acquisition is over
    bool    _M_acquisition_is_over;

    //Inform that the rest of the software is ready to store the registred images
    bool    _M_soft_ready;

    //Analysis zone
    vector<Point> _M_analysis_zone;

    //NCC vector
    QVector<QVector<float> > _M_NCC;

    //image
    vector<_Processed_img>  _M_img;

    //Mutex
    QMutex _M_mutex;

    //Buffer empty img
    Mat _M_empty_img_buffer;

    //enable motion compensation
    bool _M_enable_motion_compensation;


    QFile file;

    //Result directory
    QString _M_result_directory;
};

#endif // AREGISTRATION_H
