/**
 * @file PAnalyse.h
 *
 * @brief Main class that schedules all signal and image processing classes.
 *
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef PANALYSE_H
#define PANALYSE_H

#include <QObject>
#include <QImage>
#include <QDir>
#include <QTime>



#include <QDebug>
#include "filtering.h"
#include "conversion.h"
#include "oxygenation.h"

#include <QElapsedTimer>
#include "pobject.h"
#include "acquisition.h"
#include "PFiltering.h"
#include <QTimer>

#include <omp.h>
#include <QProcess>
#include <QThread>
#include "Statistic_functions.h"

#include "PPixelWiseMolarCoeff.h"
#include "PProcessTimes.h"
#include "PModifiedBeerLambertLaw.h"


#include "P_SPM.h"



using namespace cv;
using namespace std;

class PAnalyse : public QThread
{
    Q_OBJECT
public:
    explicit PAnalyse(QObject *parent = 0);
    ~PAnalyse();

    /** Check if learning has been done (learning of motion) */
    bool isLearningDone()                   {return _M_learningDone;}
    /** Check if the ROI is empty */
    bool isROIempty()                       {return _M_ROI.empty();}

    /** Set paradigme times (rest and activity periods) */
    void setProcessTimes(QVector<int> v)    {_M_paradigm_times.setProcessTimes(v);}

    /** Set First and last frames for analysis */
    void setFirstLastFrameForAnalysis(int start, int end)   {_M_paradigm_times.setFirstLastFrameForAnalysis(start, end);}


    /** Initialize private variables when methode changed */
    void Initialize();


    /** New statistics areas definition */
    void onnewCorticalAreaDefinition(Rect, QVector<Rect>,QVector<Mat>);
    /** New statistics areas definition */
    void onnewCorticalAreaDefinition(QVector<Rect>, QVector<Mat>);

    /** Set path for exporting data */
    void setExportedDataPath(QString dir)       {_M_exported_data_path = dir;}
    /** Enable data export */
    void enableDataExport(bool v)               {_M_enable_data_export=v;}


signals:

    /** request acquisition info */
    void requestAcquisitionInfo(bool);

    /** Send acquisition info */
    void newAcquisitionInfo(QString);

    /** Emit a new Bold signal */
    void newBoldSignal(QVector<float>);

    /** Emit new activation times */
    void newActivationtimes(QVector<double>);

    /** Inform that Data acquisition is ready (ROI is set and data vector resize) */
    void DataAcquisitionIsReady();

    /** Indicator of the process */
    void newProgressStatut(QString,int);


    /** Computation processing time */
    void Elapsed_ProcessingTime(int);

    /** Emit an error message */
    void Error(QString);

    /** Emit a new concetration changes time courses for plotting */
    void newContrastplot(QVector<QVector<float> >);

    /** Emit a new Contrast image */
    void newContrastImage(QVector<Mat>);


    /** Emit a new pixel pos (position of pixels inside the ROI) */
    void newPixelPos(QVector<Point>);

    /** Consider Blood vessel distance */
    void ConsiderBloodVesselDistance(bool);

    /** Send new hyperspectral segmentation layers */
    void newHyperspectralSegmentation(QVector<int>);

    /** Emit resting state maps */
    void newRestingStateMaps(QVector<QVector<Mat> >);
    /** Emit K-Means resting state maps */
    void newKmeans_RestingStateMaps(QVector<Mat>);

    /** Emit Activation maps */
    void newActivationMap(QVector<bool>);
    /** Emit Activation maps */
    void newActivationMap(Mat);
    /** Emit Activation maps */
    void newActivationMap(QVector<bool>,int);

    /** emit new z threshold calculated by random field theory */
    void newZThresh(double);



public slots:

    /** grey outside contour is requested */
    void onGreyOutsideContourIsRequested(bool v)    {_M_grey_outside_contour = v;}

    /** Get a new max display value */
    void onnewDisplayValue(double v)        {_M_max_display_value = v;}

    /** Get a new initial img */
    void onnewInitialImg(Mat v)             {v.copyTo(_M_initial_img);}

    /** enable or disable filtering (low pass + cardiac + data correction) */
    void onEnableFiltering(bool v)          {_M_filtering.enableFiltering(v);}

    /** enable or disable low pass filtering */
    void onEnableLowPassFiltering(bool v)   {_M_filtering.enableLowPassFiltering(v);}

    /** enable or disable data correction */
    void onEnableDataCorrection(bool v)     {_M_filtering.enableDataCorrection(v);}

    /** Create a video of concentration changes over time */
    void onCreateNewVideo();
    /** Set the nb of frames of the video of results */
    void onnewVideo_nb_Frames(double v)     {_M_video_nb_frames = v;}

    /** Set the framerate of the video (Delta C changes) */
    void onnewVideoFramerate(double v)      {_M_video_framerate = v;}

    /** Set the MEan ROI Radius */
    void onnewMeanROIRadius(double v)       {_M_Mean_ROI_radius = (int)v;}

    /** Enable the measurement at the level of a ROI */
    void onrequestMeanROIMeasure(bool v)    {_M_request_Mean_ROI_Measure = v;}

    /** Request results saving */
    void onrequestSaveResults(_data_saving_info info);

    /** Option for saving non filtered data */
    void onrequestNonFilteredDataSaving(bool);

    /** Request analysis processing */
    void LaunchProcess();


    /*****************************/
    /*********Statistics**********/
    /*****************************/

    /** Require the calculation of the distance to the nearest blood vessel */
    void onRequireDistanceToBloodVessels(bool v)    {_M_get_distance_to_Blood_Vessels = v;}

    /** Require non filtered and filtered signals */
    void onRequireFilteredNonFilteredSignals(bool v)    {_M_get_filtered_non_filtered_signals = v;}

    /** on new stat type */
    void onnewStatType(int v);

    /** on new FHWM value (used in RTF to smooth the data the Gaussian fitering) */
    void onnewFHWM(double v)                    {_M_SPM.setFWHM(v);}

    /** On new level of statistical significance */
    void onnewStatisticalLevel(int v)           {_M_SPM.set_Statistical_Significance_Level(v);}

    /*****************************/
    /*********Resting state*******/
    /*****************************/

    /** Set new resting state method (0: seed based method, 1: ICA based method) */
    void onnewRestingStateMethod(int);

    /** Set new independent nb of sources (resting state)*/
    void onNewIndependantNbofSources(int v);

    /** Enable or disable Resting state analysis */
    void onenableRestingState(bool v)             {_M_enable_resting_state=v; _M_filtering.setModeRestingState(v); _M_paradigm_times.setModeRestingState(v); LaunchProcess();}
    /** Get resting state seeds */
    void onnewRestingStateSeeds(QVector<Mat>);
    /** Init resting state seeds */
    void InitRestingStateSeeds();
    /** Process resting state analysis */
    void _ProcessRestingState();

    /** Setresting state maps undersampling */
    void onNewRestingStateMapsSampling(double v)   {_M_resting_state_map_sampling=int(v);}

    /** Set cluster number for K-Means resting state analysis */
    void onnewClustersRestingState(double v)        {_M_nb_clusters_resting_state = (int)v;LaunchProcess();}


    /*****************************/
    /*********Segmentation********/
    /*****************************/
    /** Set segmentation layers (grey matter, surface and buried blood vessels and non used pixels) */
    void onnewSegmentationLayers(Mat SurfaceBlood,Mat BuriedBlood,Mat nonused);


    /*****************************/
    /*********Camera config*******/
    /*****************************/

    /** On new hyperspectral spectral range */
    void onnewSpectralRange(int lambda_min,int lambda_max)    {_M_PixelWise_Molar_Coeff.setSpectralRange(lambda_min,lambda_max);}

    /** on require spectral correction */
    void onRequireSpectralCorrection(bool v)    {_M_filtering.requireSpectralCorrection(v);}

    /** new optical device ID */
    void onNewOpticalConfigIDx(int camera_id, int light_id)   {_M_PixelWise_Molar_Coeff.setOpticalConfigIDx(camera_id, light_id);}



    //Hyperspectral imaging
    /** Optimize quantification with optimal wavelength group of the hyperspectral camera */
    void onrequireHSOptimizedQuantification(bool v) {_M_PixelWise_Molar_Coeff.setHSOptimizedQuantification(v);}

    /*****************************/
    /*********New Images**********/
    /*****************************/

    /** new first hyperspectral img */
    void onnewFirstHSImage(vector<Mat> v)   {_M_PixelWise_Molar_Coeff.setnewHSImg(v);}

    /** Send RGB data to the PFiltering class */
    void addDatas(_Processed_img &img);
    /** Send hyperspectral data to the PFiltering class */
    void addDatas(_Processed_img_HS &img);




    /** Temporary function */
    void _function_temporaire();

    /** Set Camera Frame rate */
    void setFrameRate(double v)                    {_M_frame_rate=v;_M_filtering.setFrameRate(v);_M_paradigm_times.setFrameRate(v);}

    /** Set Region of interest */
    void setAnalysisZone(QVector<QPoint> c, Size img_size, int nbChannels, int NbFrames);

    /** Set Point of interest (clicked on graphic interface) */
    void setStudiedPoint(Point P);

    /** Inform Data finish loading */
    void DataFinishLoading();

    /** Get a New correlation threshold */
    void onNewCorrelationThreshold(double v);

    /** Send new Fitlering type (FFT, FIR or IIR) */
    void onFilteringTypeChanged(int v)          {_M_filtering.setFilteringType(v);}

    /** Set chromophore id: HbO2, Hb, (oxCCO), HbT */
    void onNewChromophoreID(int v)              {_M_chromophore_id = v;}

    /** Indicate the number of activation step to consider for the calculation of correlation and SPM */
    void onNewActivationStepsToConsider(int v)           {_M_paradigm_times.setActivationStepsToConsider(v); LaunchProcess();}


    /** New result directory */
    void onNewResultDirectory(QString v);

protected:
    /** Call process in parallel thread */
  void run();

private:
    //Private function

    // Get Delta C maps for statistical tests
    QVector<Mat> _Get_Delta_C_Maps(int nb_chromopores, bool use_filtered_data = true);

    //Get camera intensities
    QVector<Mat> _Get_Camera_Intensity(bool use_filtered_data = true);

    //Get Absorbance changes
    QVector<Mat> _Get_Absorbance_Changes(bool use_filtered_data = true);

    //Save results
    void _Save_Results();




    void _General_Linear_Model_pixel_wise();


    // Get stats mask position
    void _Get_Stats_Mask_pos(Mat mask, QVector<int> &pos);
    void _Get_Stats_Mask_pos(Rect mask,QVector<int> &pos);

    //Write Img frame (for img clustering)
    void  _WriteImg(Mat &img);

    //Process mean concentration measure
    void _ProcessMeanConcentrationMeasure();
    QVector<Mat> _Process_Mean_Delta_C();

    //Process Correlation of BOLD signal
    void _ProcessCorrelationMeasure();
    void _ProcessCorrelation(QVector<QVector<float> > &Concentration_map);

    //Get QMAP and corr
    void _Get_QMAP_AND_CORR(int i,QVector<double> &Corr_C,QVector<QVector<double> > &Mean_C,QVector<int> &activity_id_start,QVector<int> &activity_id_end);
    void _Get_QMAP_AND_CORR(QVector<double> &stat_vec, QVector<QVector<double> > &QMap_temp, QVector<QVector<float> > contrast, QVector<int> coeff_multi, QVector<int> &activity_id_start, QVector<int> &activity_id_end);

    //Display oxynation concentration results
    Mat _Display(QVector<float> &contrast_proc);
    Mat _Display(QVector<bool> &contrast_proc);
    Mat _Display(Mat &contrast_proc);
    Mat _Display(const Mat &contrast_proc,const QVector<int> &id);
    Mat _Display_ROI_activation(QVector<bool> &contrast_proc);
    Mat _Display_ROI_activation(QVector<float> &contrast_proc);
    Mat _Display_ROI_activation(Mat &contrast_proc);

    //Write img result
    Mat _writeCartography(Mat &contrast_proc, QString filename);
    void _writeFloatCartography(Mat &contrast_proc,QString filename);

    //reconstruct img
    Mat _reconstruct_Float_Carography(Mat &contrast_proc);
    Mat _reconstruct_uchar_Carography(Mat &contrast_proc);

    //Create falsecolormap
    Mat _Convert_to_FalseColor(Mat &in, Mat &mask_img, double &min, double &max);
    void _GreyOutsideContour(Mat mask, Mat &inout);
    Mat _create_colorbar(Mat in_img);

    //Draw text on image
    Mat _drawTextOnImg(Mat img, String text, int pos, double font_size, int thick);

    Mat _reconstruct_Kmeans_img(Mat &label,QVector<Scalar> &color_cluster);


    int                         _M_point_of_interest;

    //Data info
    int                         _M_tot_frames;
    int                         _M_nb_channels;
    int                         _M_nb_temporal_vectors;
    double                      _M_frame_rate;


    //ROI
    vector<Point>               _M_ROI;

    //Pixel pos
    QVector<cv::Point>          _M_pixels_pos;

    //Indicators (Learning, acquisition, processing)
    bool                        _M_learningDone;

    //Video frames size
    cv::Size                    _M_img_size;

    //Coefficient class

    PPixelWiseMolarCoeff        _M_PixelWise_Molar_Coeff;
    PModifiedBeerLambertLaw     _M_MBLL;


    //Statistical Parametric Mapping
    P_SPM                        _M_SPM;


    //Processing times in not RT mode (reference times and process times)
    PProcessTimes               _M_paradigm_times;


    //Methode choice
    //Correlation threshold
    double                      _M_correlation_threshold;

    //Fitlering class
    PFiltering                  _M_filtering;

    //Statistics
    bool                        _M_get_distance_to_Blood_Vessels;
    bool                        _M_get_filtered_non_filtered_signals;

    //Processing type (SPM, t-tests, Mean concentration changes, correlation....
    int                         _M_processing_type;
    double                      _M_statistical_significance;
    double                      _M_HbO2_apriori;
    double                      _M_Hb_apriori;
    double                      _M_oxCCO_apriori;


    //areas
    QVector<int>                _M_reference_area_pos;
    QVector<QVector<int> >      _M_cortical_areas_pos;


    //Exported data
    QString                     _M_exported_data_path;
    bool                        _M_enable_data_export;

    //Resting state
    void _Process_Resting_state_ICA_based_method();
    void _Process_Resting_state_seed_based_method();
    void _Process_Resting_state_seed_SPM();
    void _Process_Resting_state_Low_Frequency_Power_Method();
    void _Process_Resting_state_K_means();
    bool                        _M_enable_resting_state;
    QVector<Mat>                _M_resting_state_seeds;
    int                         _M_resting_state_method;
    int                         _M_resting_states_independant_sources;
    int                         _M_resting_state_map_sampling;
    int                         _M_nb_clusters_resting_state;


    //Save resutls
    bool                        _M_save_results;
    _data_saving_info           _M_saving_info;

    //MEan ROI
    void _getMeanReflectance_ROI_MEasurement(Point P, QString saving_dir);
    void _getMeanROIMeasurement(Point P, int rest_start, int rest_end, int correlation_start, int correlation_end, QVector<QVector<float> > &mean_contrast, QString saving_dir);
//    void _getMeanContrast_AND_Stats(int nb_wavelength,bool Hb_HBO2_deco,QVector<int> id,QString write_file);
    void _getMeanContrast_AND_Stats(QVector<int> id, QString write_file);
    void _getMeanContrast(QVector<QVector<float> > &mean_contrast, QVector<int> id, QString write_file, int rest_start, int rest_end, int correlation_start, int correlation_end);
    void _getMeanReflectance(QVector<int> id,QString write_file);

    //Radius
    int _M_Mean_ROI_radius;

    //Enable measurement
    bool _M_request_Mean_ROI_Measure;


    //Max display value
    double _M_max_display_value;

    //Create video
    Mat     _M_initial_img;
    bool    _M_create_video;
    double  _M_video_nb_frames;
    double  _M_video_framerate;
    void _Create_Video();

    //chromophore ID (HbO2, Hb, (oxCCO))
    int _M_chromophore_id;


    //Grey outside contour
    bool _M_grey_outside_contour;

    //Result directory
    QString _M_result_directory;


};

#endif // PANALYSE_H
