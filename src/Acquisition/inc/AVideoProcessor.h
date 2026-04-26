/**
 * @file AVideoProcessor.h
 *
 * @brief Video reading using Qt Multimedia
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */


#ifndef AVIDEOPROCESSOR_H
#define AVIDEOPROCESSOR_H


#include <QObject>
#include <QMediaPlayer>
#include <QVideoSink>
#include <QVideoFrame>
#include "conversion.h"
#include "pobject.h"

class AVideoProcessor : public QObject
{
    Q_OBJECT
public:
    explicit AVideoProcessor(QObject *parent = nullptr);
    ~AVideoProcessor();

    void loadVideo(const QString &filePath);
    void startReadingFrames();
    void stop_reading();

    void set_Total_frame_number(int v)  {_M_tot_frames = v;}
    void init_frame_count()             {_M_video_framecount = 0;}

    void set_Start_pos_ms(qint64 v)     {_M_start_pos_ms = v;}
    void set_Start_frame(int v)         {_M_start_frame = v;}
    void set_Last_frame(int v)          {_M_last_frame = v;}

signals:
    void allFramesProcessed();
    void newImageAcquired(_Processed_img);
    void newImgSend(int,int);

private slots:
    void onVideoFrameChanged(const QVideoFrame &frame);

private:
    QMediaPlayer *_M_mediaPlayer;
    QVideoSink *_M_videoSink;
    int _M_frameCount;
    int _M_video_framecount;
    int _M_starting_frame_count;


    int _M_last_frame;
    int _M_start_frame;
    int _M_tot_frames;

    qint64 _M_start_pos_ms;
};

#endif // AVIDEOPROCESSOR_H
