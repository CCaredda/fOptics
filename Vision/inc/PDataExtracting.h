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
#include <QCoreApplication>

#define Filter_675_975nm 0
#define Filter_600_875nm 1

class PDataExtracting : public QThread
{
    Q_OBJECT
public:
    explicit PDataExtracting(QObject *parent = nullptr);
    ~PDataExtracting();

    /** require spectral correction */
    void requireSpectralCorrection(bool v)  {_M_correct_spectral_data=v;}

    /** Set pixel position inside the ROI */
    void setPixelPos(QVector<Point> p)  {_M_pixels_pos.clear(); _M_pixels_pos=p;}
    /** Set the frame number */
    void setTotFrames(int p)            {_M_tot_frames=p;}

    /** Set the Hyperspectral camera configuration */
    void setNew_HS_config(int v);

    /** init the private variables */
    void init();

    /** request acquisiton info */
    QString requestAcquisitionInfo(bool v);

signals:
    /** New extracted data (in raw image) */
    void newExtractedData(_Processed_img);

protected:
    /** Call process in a parallel thread */
    void run();

public slots:
    /** Extract data from RGB image */
    void extractData(_Processed_img);
    /** Extract data from hyperspectral image image */
    void extractData(_Processed_img_HS);

    /** New result directory */
    void onNewResultDirectory(QString v)    {_M_result_directory = v;}

private slots:
    /** Inform that thread finished */
    void onThreadFinished();

private:
    void _extractData_RGB(_Processed_img);
    void _extractData_HS(_Processed_img_HS);

    //Empty img buffers
    Mat                     _M_empty_img_buffer;
    vector<Mat>             _M_empty_img_buffer_HS;

    //Image FIFO
    QVector<_Processed_img>     _M_buffer_img;
    QVector<_Processed_img_HS>  _M_buffer_img_HS;

    QVector<Point>          _M_pixels_pos;
    int                     _M_tot_frames;
    bool                    _M_acquisition_is_over;
    bool                    _M_mode_RGB;
    QMutex                  _M_mutex;

    //Spectral correction matrix
    Mat                     _M_Spectral_Corr_Matrix;
    bool                    _M_correct_spectral_data;

    bool _M_in_thread;

    //Debug level
    bool                    _M_debug_level;

    //Result directory
    QString _M_result_directory;

};

#endif // PDATAEXTRACTING_H
