/**
 * @file APostAcquisition.h
 *
 * @brief After the acquisition (through ACamera instance) or the loading of the images (through AloadDatas instance) images
 * are injected in this class to compute preprocessing steps (image cropping and undersampling). Then images are send to processing units.
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef APOSTACQUISITION_H
#define APOSTACQUISITION_H

#include <QObject>
#include <QProcess>
#include <QThread>
#include <QWaitCondition>
#include <QQueue>

#include "acquisition.h"
#include "pobject.h"
#include <QElapsedTimer>
#include <QMutex>
#include <QDir>
#include <QCoreApplication>

class APostAcquisition : public QThread
{
    Q_OBJECT
public:
    explicit APostAcquisition(QObject *parent = 0);
    ~APostAcquisition();

    /** get spatial sampling */
    double getSpatialSampling() {return _M_spatial_sampling;}


    /** Set spatial sampling */
    void setSpatialSampling(double v)   {_M_spatial_sampling = v;}

    /** get preprocessed img */
    Mat getPreProcessedImg()    {return _M_pre_processed_img;}

    /** Set Pre-ROI rect */
    void setRectROI(QPoint p1,QPoint p2);
    /** Set Pre-ROI rect */
    void setRectROI(QPoint p1,QPoint p2,double spatial_sampling);

    /** get initial image size */
    Size getInitialImgSize()    {return _M_initial_img.size();}

    /** get pre processed image size */
    Size getPreProcessedImgSize()   {return _M_pre_processed_img.size();}


    /** get Pre ROI Rect */
    Rect getPreROIRect()            {return _M_Pre_ROI_Rect;}

    /** Write infos */
    void WriteInfos();

    /** stop thread */
    void stop();


protected:
    /** Call parallel thread */
    void run();

signals:
    /** Send processed img (RGB) */
    void newPreProcessedImg(_Processed_img);




    /** new image for display */
    void newDisplayImage(Mat);

    /** new image to save results */
    void newInitialImg(Mat);

    /** request initial image */
    void request_Initial_Image();


public slots:

    /** Set maximum number of analyzed pixels */
    void on_New_Maximum_number_Analyzed_pixels(double v);


    /** Require image pre processing */
    void newImage(_Processed_img);
    void newImage(Mat,int);


    /** Init Pre-ROI */
    void InitROI();

    /** Request pre processing for a single img (RGB)*/
    void requestPreProcessing(Mat mat);


    /** New result directory */
    void onNewResultDirectory(QString v)    {_M_result_directory = v;}

private:
    /** pre processing */
    void _ProcessPreProcessing(_Processed_img img);

    /** Set Pre ROI */
    void _setROI(Rect roi);
    void _setROI(Rect roi,double spatial_sampling);
    void _applyRoi(bool v);

    //indicators
    bool _M_acquisition_is_over;
    bool _M_apply_Pre_ROI;

    //Pre-ROI Rect
    Rect    _M_Pre_ROI_Rect;

    //spatial sampling
    double _M_spatial_sampling;

    //Image buffer
    QQueue<_Processed_img> _M_img_buffer;


    //Mutex
    QMutex _M_mutex;
    QWaitCondition  _M_condition;
    bool            _M_stop;

    //Image size
    cv::Size _M_img_size;


    //Pre processed image (first image)
    Mat _M_pre_processed_img;

    //Get initial image (before undersampling and roi extraction)
    Mat _M_initial_img;

    //Max data amount
    int _M_max_data_amount;

    //enable under sampling
    bool _M_enable_under_sampling;

    //Result directory
    QString _M_result_directory;

};

#endif // APOSTACQUISITION_H
