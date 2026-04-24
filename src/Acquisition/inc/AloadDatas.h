/**
 * @file AloadDatas.h
 *
 * @brief This class aims to simulate the acquiwition of RGB or hyperspectral images and send images at video framerate
 * to the processing units. Images have to be located in the computer hardrive in raw format (.png or .tiff) or in video format (.avi).
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef ALOADDATAS_H
#define ALOADDATAS_H

#include <QObject>
#include <QImage>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/types_c.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"
#include "loadinfos.h"

#include <QDebug>
#include "conversion.h"
#include <QThread>
#include <QWaitCondition>
#include <QQueue>


#include <QFile>
#include <QDir>
#include <QStringList>
#include <QFileInfo>
#include <QtConcurrent/QtConcurrent>
#include "pobject.h"

#include "APostAcquisition.h"

#include "AVideoProcessor.h"
#include "FFmpegVideoReader.h"


class AloadDatas : public QThread
{
    Q_OBJECT
public:
    explicit AloadDatas(QObject *parent = nullptr);
    ~AloadDatas();

    /** Check if path is loaded */
    bool isPathLoaded() {return _M_is_video_loaded;}

    /** Request first img */
    Mat RequestFirstImg();



    /** Get the total number of frames */
    int getTotalFrameNumber()   {return _M_tot_frames;}

    /** Get start Frame number */
    int getStartFrameNumber()   {return _M_start_frame;}

    /** Get Last Frame number */
    int getLastFrameNumber()   {return _M_last_frame;}



    /** Get FPS */
    double getFPS()             {return _M_FPS;}


    /** Request start and last acquisition frames depending on the analysis
     *  -1: no analysis
     *  0 : task-based
     *  1: resting state
     *  2: Impulsion*/
    void getStartEndAnalysisFrames(int analysis);


    /** Get camera name */
    QString get_Camera_name()   {return _M_camera_name;}

    /** Get light source type */
    QString get_Light_source_type() {return _M_Light_source_type;}

    /** stop thread */
    void stop();

protected:
    /** Call parallel thread */
    void run();

signals:

    /** First and last analysis frames */
    void newFirstLastAnalysisFrames(int,int);

    /** New images for pre processing */
    void newImageAcquired(_Processed_img);

    /**  Message */
    void newMessage(QString);

    /** Error */
    void newStatut(bool);

    /** Process times */
    void newProcessid(QVector<int>,double);

    /** Frame rate */
    void newFrameRate(double);

    /** img send */
    void newImgSend(int,int);

    /** request pre processing */
    void requestPreProcessing(Mat);

    /** request analysis choose */
    void requestAnalysisChoose(QVector<int>);

public slots:

    /**  Set Video path */
    void setVideoPath(QString);

    /**  Load datas */
    void LoadDatas();


    /** request new Image */
    void onRequestNewImage();

    /** Set reading method */
    void onnewVideoReadingMethod(QString v)   {_M_mode_video_reading = v;}


private:
    /** Get resolution reference */
    void _GetResolutionReference();

    /** read RGB img from directory containing img files (.tiff or .png) */
    void _LoadDatasFromImgFiles();


    /** read img from directory containing a video file (.avi) */
    void _LoadDatasFromVideoFile_Qt();
    void _LoadDatasFromVideoFile_Opencv();
    void _LoadDatasFromVideoFile_ffmpeg();

    /** Request first image */
    Mat _RequestFirstImg_opencv(QString video);
    Mat _RequestFirstImg_Qt(QString video);
    Mat _RequestFirstImg_ffmpeg(QString video);


    /** Get video informations */
    void _getVideoInformations();


    // List of img files
    QStringList _M_img_files;

    // List of video files
    QStringList _M_video_files;
    QVector<int> _M_idx_video;

    //Video directory
    QString _M_video_dir;
    QString _M_video_name;


    //Mode video (true) or mode img (false)
    bool    _M_mode_video;

    //Check if a video is loaded
    bool    _M_is_video_loaded;

    //Frame per second
    double  _M_FPS;

    //Nb total of frames in the video
    int     _M_tot_frames;
    QVector<int> _M_tot_frames_per_video;

    //Start frame
    int     _M_start_frame;

    //Last frame
    int     _M_last_frame;

    //First last frames (videos)
    QVector<int> _M_first_frames_video;
    QVector<int> _M_last_frames_video;

    //Image size
    cv::Size _M_img_size;


    QString _M_camera_name;
    QString _M_Light_source_type;


    AVideoProcessor _M_video_processor;
    int             _M_video_id;

    //Frame idx loaded
    bool _M_frame_idx_loaded;

    //mode video reading (Qt, opencv, ffmpeg)
    QString _M_mode_video_reading;
};

#endif // ALOADDATAS_H
