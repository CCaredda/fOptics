/**
 * @file PFiltering.h
 *
 * @brief Class that aims to apply a temporal filtering of data extracted using the PDataExtracting class.
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */


#ifndef PFILTERING_H
#define PFILTERING_H

#include <QDir>
#include <QThread>
#include <QObject>
#include <QFile>
#include <QTextStream>
#include "filtering.h"
#include "acquisition.h"
#include "pobject.h"
#include "polyfit.hpp"
#include <QElapsedTimer>
#include <PDataExtracting.h>
#include <QMutex>
#include <omp.h>

#include <QRegularExpression>

#define FFT_FILTERING 0
#define FIR_FILTERING 1
#define IIR_FILTERING 2

#define Low_Pass_Cut_OFF_Frequency 0.05
#define Low_Pass_Cut_OFF_Frequency_Resting_State 0.08 //0.05
#define MAX_data_WEIGHT_FILTERING_GB 11000000000

class PFiltering : public QThread
{
    Q_OBJECT
public:
    explicit PFiltering(QObject *parent = nullptr);
    ~PFiltering();

    /*********************************************************/
    /*********************************************************/
    /*********************SETTERS*****************************/
    /*********************************************************/
    /*********************************************************/

    /** require spectral correction */
    void requireSpectralCorrection(bool v)  {_M_data_extract.requireSpectralCorrection(v);}


    /** Option for saving non filtered signals */
    void save_NonFilteredSignals(bool v)    {_M_save_NonFilteredSignals = v;}

    /** Set Mode resting state */
    void setModeRestingState(bool v)        {_M_mode_resting_state = v;}  

    /** Set frame rate value */
    void setFrameRate(double v)             {_M_frame_rate=v;}

    /** Set filtering type (FFT filtering: 0, FIR filtering: 1, IIR filtering: 2)*/
    void setFilteringType(int v);


    /** Set new Hyperspectral cam config */
    void setNew_HS_config(int v)            {_M_data_extract.setNew_HS_config(v);}

    /** Set Point of interest (for debug purpose) */
    void setPointOfInterest(int i)          {_M_Point_Of_Interest.push_back(i);}

    /** Enable or disable filtering  (low pass + cardiac filtering + data correction) */
    void enableFiltering(bool v)            {_M_enable_filtering=v;}

    /** Enable or disable low pass filtering */
    void enableLowPassFiltering(bool v)     {_M_enable_low_pass_filtering=v;}

    /** Enable or disable Data corection */
    void enableDataCorrection(bool v)       {_M_enable_data_correction=v;}

    /*********************************************************/
    /*********************************************************/
    /*********************INIT********************************/
    /*********************************************************/
    /*********************************************************/

    /** init buffer size */
    void init(int nbElements,int nbChannels,int nb_frames);


    /*********************************************************/
    /*********************************************************/
    /*********************GETTERS*****************************/
    /*********************************************************/
    /*********************************************************/

    /** is filtering enabled ? */
    bool isFilteringEnabled()   {return _M_enable_filtering;}

    /** Get temporal vectors */
    QVector<float> getTemporalVector(int spatial_id,int spectral_id)    {return _M_ROI_row_data[spatial_id][spectral_id];}
    /** Get temporal vectors */
    QVector<QVector<float> > getTemporalVector(int spatial_id);

    /** Get non filtered temporal vectors */
    QVector<QVector<float> > getNonFilteredTemporalVector(int spatial_id);


    /** Get temporal vectors */
    QVector<QVector<float> > getTemporalVector(int spatial_id,QVector<int> wavelength);
    /** Get temporal vectors */
    QVector<QVector<float> > getTemporalVector(int spatial_id,QVector<int> wavelength,int t_start,int t_end);

    /** Get number of frames */
    int getNbOfTemporalElement();


    /** Get spectral vector */
    QVector<float> get_spectral_vector(int spatial_id);
    /** Get spectral vector */
    QVector<QVector<float> > get_spectral_vectors(int spatial_id);

    /** Get Buffer values (used in IIR filtering) */
    QVector<float> getFirstValues(int id);

    /** Get intensity at specific point, frame and spectral channel */
    float getValue(int spatial_id,int spectral_id,int temporal_id)      {return _M_ROI_row_data[spatial_id][spectral_id][temporal_id];}

    /** Check if Data is valid (non empty vector) */
    bool isRowDataValid();

    /** get current output value (last filtered value: used in IIR filtering) */
    int getLastFilteredId();

    /** Get first value (for data redressing in real time filtering) */
    QVector<float> *getFirstValuePointer();

    /** Get filtering type (FFT, FIR or IIR) */
    int getFilteringType()  {return _M_filtering_type;}



protected:
    /** Call process in parallel thread */
    void run();

signals:
    /** Send acquisition info */
    void newAcquisitionInfo(QString);

    /** Start not real time process */
    void readyForNotRealTimeProcess();

    /** Inform progress statut */
    void newProgressStatut(QString,int);

public slots:

    /** Request aquisition info */
    void onrequestAcquisitionInfo(bool v);

    /** Send to data for extraction (RGB data) */
    void new_Filtering(_Processed_img &mat);
    /** FSend to data for extraction (hyperspectral data) */
    void new_Filtering(_Processed_img_HS &mat);

    /** Set pixel pos inside the ROI */
    void setPixelPos(QVector<Point> p)  {_M_nb_Spatial_pixels=p.size();_M_data_extract.setPixelPos(p);}

    /** on New spectral bands idx */
    void onnewWavelengthIDX(QVector<int>);

    /** New result directory */
    void onNewResultDirectory(QString v)    {_M_result_directory = v; _M_data_extract.onNewResultDirectory(v);}

private slots:
    /** Filter data extracted from PDataExtracting */
    void onNewDataExtracted(_Processed_img);

private:
    //Normalize data
    void _NormalizeData(Mat &inout);


    //Process FFT filtering
    void _Process_FFT_Filtering();

    //Get mean signal value (for IIR filtering)
    void _GetMeanSignalValue(_Processed_img &img);


    //Store data
    void _StoreData(_Processed_img);

    //Redress Data
    void _ProcessDataRedress();

    //Datas
    QVector<QVector<QVector<float> > >  _M_ROI_row_data;
    QVector<QVector<QVector<float> > >  _M_ROI_row_data_non_filtered;


    //Number of spatial pixels
    int                                 _M_nb_Spatial_pixels;
    int                                 _M_nb_spectral_channels;

    //Frame rate
    double                              _M_frame_rate;

    //Buffers
    QVector<QVector<QVector<float> > >  _M_in_buffer_filtering;
    QVector<QVector<QVector<float> > >  _M_out_buffer_filtering;
    QVector<QVector<QVector<float> > >  _M_remain_values;
    QVector<QVector<float> >            _M_buffer_first_value;
    Mat                                 _M_empty_img_buffer;


    //Image buffer
    QVector<_Processed_img>             _M_img_buffer;

    //output vector temporal id
    int                                 _M_output_id;

    //Nb total of frames
    int                                 _M_tot_frames;

    //indicator
    bool                                _M_acquisition_is_over;
    int                                 _M_nb_Learning_frames;

    //Data extraction
    PDataExtracting                     _M_data_extract;

    //Mutex
    QMutex                              _M_mutex;

    //Filtering type
    int                                 _M_filtering_type;

    //points of interest
    QVector<int>                        _M_Point_Of_Interest;

    //mode resting state
    bool                                _M_mode_resting_state;

    //Enable or disable filtering (low pass + data correction)
    bool                                _M_enable_filtering;

    //Enable or disable data correction
    bool                                _M_enable_data_correction;

    //Enable or disable low pass filtering
    bool                                _M_enable_low_pass_filtering;

    //option to store non filtered signals into RAM
    bool                                _M_save_NonFilteredSignals;

    //thread is running ?
    bool _M_in_thread;


    //Spectral bands idx
    QVector<int>    _M_spectral_bands_idx;


    //Result directory
    QString _M_result_directory;
};

#endif // PFILTERING_H
