/**
 * @file PDataExtracting.h
 *
 * @brief Class that aims to store a 3D image (N.M pixels .K spectral channels) into an array.
 * This array has dimension P.T.K, with P the number of pixels, T the number of frames of
 * the video and K the number of spectral channels.
 *
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */


#ifndef PDATAEXTRACTING_H
#define PDATAEXTRACTING_H

#include <QMessageBox>
#include "acquisition.h"
#include <QObject>
#include "pobject.h"
#include "filtering.h"
#include <QThread>
#include <QFile>
#include <QElapsedTimer>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <QCoreApplication>
#include "loadinfos.h"

#define Filter_675_975nm 0
#define Filter_600_875nm 1

class PDataExtracting : public QThread
{
    Q_OBJECT
public:
    explicit PDataExtracting(QObject *parent = nullptr);
    ~PDataExtracting();

     /** Set pixel position inside the ROI */
    void setPixelPos(QVector<Point> p)  {_M_pixels_pos.clear(); _M_pixels_pos=p;}
    /** Set the frame number */
    void setTotFrames(int p)            {_M_tot_frames=p;}


    /** init the private variables */
    void init();


    /** stop thread */
    void stop();

signals:
    /** New extracted data (in raw image) */
    void newExtractedData(_Processed_img);

protected:
    /** Call process in a parallel thread */
    void run();

public slots:
    /** Extract data from RGB image */
    void extractData(_Processed_img);


    /** New result directory */
    void onNewResultDirectory(QString v)    {_M_result_directory = v;}

private slots:
    /** Inform that thread finished */
    void onThreadFinished();

private:
    void _extractData_RGB(_Processed_img);


    //Empty img buffers
    Mat                     _M_empty_img_buffer;


    //Image FIFO
    QQueue<_Processed_img>     _M_buffer_img;


    QVector<Point>          _M_pixels_pos;
    int                     _M_tot_frames;
    bool                    _M_acquisition_is_over;
    QMutex                  _M_mutex;
    QWaitCondition          _M_condition;
    bool                    _M_stop;



    bool _M_in_thread;

    //Debug level
    bool                    _M_debug_level;

    //Result directory
    QString _M_result_directory;

};

#endif // PDATAEXTRACTING_H
