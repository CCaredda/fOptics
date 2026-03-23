/**
 * @file PPixelWiseMolarCoeff.h
 *
 * @brief Class that aims to compute matrices of coefficients used in the modified Beer Lambert law.
 * These matrices are calculated in a pixel-wise manner accordingly to the image segmentation provided by the Clustering classes.
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */


#ifndef PPIXELWISEMOLARCOEFF_H
#define PPIXELWISEMOLARCOEFF_H

#include <QDir>
#include <QObject>
#include <QThread>
#include "molarcoefffunctions.h"
#include "acquisition.h"
#include <QCoreApplication>

#define Reso_MeanPath_mm 0.1



class PPixelWiseMolarCoeff : public QThread
{
    Q_OBJECT
public:
    explicit PPixelWiseMolarCoeff(QObject *parent = nullptr);
    ~PPixelWiseMolarCoeff();

    /** Has init been done ? */
    bool IsInitHasBeenDone()        {return !_M_init_or_process;}


    /** Set image resolution */
    void setResolution(float reso)  {_M_reso_in_mm = reso; }

    /** set surface blood vessel layer */
    void setImgSurfaceBlood(Mat img)    {img.copyTo(_M_img_Surface_blood_in);}
    /** set buried blood vessel layer */
    void setImgBuriedBlood(Mat img)     {img.copyTo(_M_img_Buried_Blood_in);}
    /** set non used pixels layer */
    void setImgNonUsedPixels(Mat img)   {img.copyTo(_M_non_used);}

    /** Set Pixel pos inside the ROI */
    void setPixelsPos(QVector<Point> &pixel_pos)    {_M_pixels_pos.clear(); _M_pixels_pos = pixel_pos;}

    /** init molar coeff set the Grey matter coeff matrix for all pixels inside the ROI*/
    void initMolarCoeff();

    /** Process molar coeff computation for all pixels according to the segmentation layers */
    void processMolarCoeff();


    /** Get ditance map (in mm) between grey matter points and blood vessels */
    QVector<float> getDistanceMap_GreyMatter_BloodVessel()  {return _M_distance_map;}

    /** Get ditance map (in mm) between grey matter points and blood vessels at pos id */
    float getDistanceMap_GreyMatter_BloodVessel(int pos);





//    //get GM matrix for Hb and HbO2 or Hb, HbO2 and oxCCO deconvolution
//    Mat getMolarCoeff(int pos,bool Hb_HbO2_deconvolution,QVector<int> wavelength_grp);

//    //Get buried blood matrix
//    Mat getBuriedBloodMatrix();

//    //get Surface Blood matrix
//    Mat getSurfaceBloodMatrix();

//    //is blood vessel Matrix
//    bool isBloodVesselMatrix(int id);


    /** Get Grey Matter Mean path length */
    QVector<double> getMeanPathMG() {return _M_Mean_path_Surface_Blood[0];}
    /** Get surface blood vessel Mean path length */
    QVector<double> getMeanPathSBV() {return _M_Mean_path_Surface_Blood[_M_Mean_path_Surface_Blood.size()-1];}
    /** Get buried blood vessel Mean path length */
    QVector<double> getMeanPathBBV() {return _M_Mean_path_Buried_Blood[_M_Mean_path_Buried_Blood.size()-1];}
    /** Get Mean path length computed at specific point*/
    QVector<double> getMeanPath(int pos);

    /** Get molar coeff at specific point */
    bool getMolarCoeff(int id,QVector<Mat> &coeff);

    /** get molar coeff for grey matter */
    QVector<Mat> getGreyMatterMatrix()  {return _M_Molar_coeff_GM;}


    /** Check if the molar coeff matrix is not empty at specific point */
    bool isEmptyMatrix(int id);


    /** Set Optical device idx */
    void setOpticalConfigIDx(int camera_id, int light_source_id);






    /** Get id of segmented image (grey matter layer) */
    QVector<int> get_GM_id()            {return _M_id_GM;}
    /** Get id of segmented image (buried blood vessel layer) */
    QVector<int> get_BuriedBlood_id()   {return _M_id_BuriedBlood;}
    /** Get id of segmented image (surface blood vessel layer) */
    QVector<int> get_SurfaceBlood_id()  {return _M_id_SurfaceBlood;}


    /** Optimize hyperspectral quantification using optimal wavelength */
    void setHSOptimizedQuantification(bool v);

    /** Hyperspectral spectral range */
    void setSpectralRange(int lambda_min,int lambda_max);

    /** Get optimal spectral configuration of the hyperspectral camera */
    QVector<QVector<int> > get_HS_optimal_cam_idx();

    /** Set new HS img */
    void setnewHSImg(vector<Mat> v)     {_M_first_HS_img.clear();_M_first_HS_img = v;}

    /** Quantify oxCCO */
    void Quantify_oxCCO(bool);

    /** get number of chromophore quantified */
    int getChromophoreNumber();



protected:
    /** Call process in parallel thread */
    void run();

signals:
    /** Inform that process is finished */
    void processFinished();

    /** Send the new Hyperspectral segmentation */
    void newHyperspectralSegmentation(QVector<int>);

    /** Send the new hyperspectral wavelength configuration */
//    void newHyperspectralWavelengthConfig(QVector<QVector<int> >);
    void newWavelengthIDX(QVector<int>);

public slots:
    /** Consider Blood vessel distance */
    void ConsiderBloodVesselDistance(bool v);

    /** New result directory */
    void onNewResultDirectory(QString v)    {_M_result_directory = v;}

private:

    //Compute MBLL matrix
    QVector<Mat> _Compute_MBLL_Matrix(QVector<double> &mean_path);

    //Load spectral resultant
    void _Load_spectral_resultant();

    //Load Mean path length
    void _Load_Mean_path_Length();

    //Hyperspectral camera config (combination of wavelength)
    void _Init_Hyperspectral_wavelength_config();

    //Process segmentation using simulated spectra
//    void _Process_Hyperspectral_Segmentation();

    //Pre processing
    bool _PreProcessing();

    //init data
    void _initData();

    //Display Matrix
    void _DisplayMatrix(Mat);

    //Segmentation
    bool _Segmentation();

    //Set molar coeff according to the cluster id
    void _ClusterMethod();

    //Set molar coeff according to the distance to blood vessel
    void _PixelWiseMethod();

    //Choose GM mean path length according to the distance of the current pixel to the nearest blood vessel
    void _ChooseMeanPathGM(QVector<double> &mean_path_GM, double dist, int tissue_type);

    //Consider blood vessel distance
    bool        _M_consider_blood_vessel_distance;

    //bool init MolarCoeff
    bool        _M_init_or_process; //true: init, false: process


    float       _M_reso_in_mm;
    int         _M_vessel_lim_in_px;
    int         _M_area_lim_in_px;


    //Extinction coefficients (in cm-1.mol-1.L)
    QVector<double> _M_epsilon_Hb;
    QVector<double> _M_epsilon_HbO2;
    QVector<double> _M_epsilon_oxCCO;
    QVector<double> _M_wavelength_epsilon;

    //Light source spectra (in a.u)
    QVector<QVector<double > > _M_light_source_spectra; //device.lambda
    QVector<QVector<double> >  _M_wavelength_light; //device.lambda
    int _M_id_light_source;

    //Camera spectral sensitivities (in a.u)
    QVector<QVector<QVector<double> > >_M_camera_spectral_sensitivities; //device.detector.lambda
    QVector<QVector<double> > _M_wavelength_camera; //Device.lambda
    int _M_id_camera;


    //eps x spectral resultant (detector.lambda)
    QVector<QVector<double> > _M_eps_Hb_x_res_spectral;
    QVector<QVector<double> > _M_eps_HbO2_x_res_spectral;
    QVector<QVector<double> > _M_eps_oxCCO_x_res_spectral;

    //Mean path (in cm)
    QVector<QVector<double> > _M_Mean_path_Surface_Blood;
    QVector<QVector<double> > _M_Mean_path_Buried_Blood;
    QVector<double> _M_wavelength_mean_path;


    //Preprocessing
    Mat _M_img_Surface_blood_in;
    Mat _M_img_Buried_Blood_in;
    Mat _M_non_used;
    Mat _M_img_Surface_blood_preprocessed;
    Mat _M_img_Buried_Blood_preprocessed;
    vector<Mat> _M_first_HS_img;

    //Blood vessel contours
    vector<vector<Point> > _M_contours_SBV;
    vector<vector<Point> > _M_contours_BBV;

    //Pixels pos
    QVector<Point>  _M_pixels_pos;


    //Distance map between grey matter points and the nearest blood vessel
    // for blood vessel points (surface and buried blood vessel: distance is null
    QVector<float> _M_distance_map;

    //Molar coeffs
    QVector<QVector<Mat> >  _M_Molar_coeff; //Size (nb pixel,nb deconvolution systems)

    QVector<Mat>            _M_Molar_coeff_BuriedBlood;
    QVector<Mat>            _M_Molar_coeff_SurfaceBlood;
    QVector<Mat>            _M_Molar_coeff_GM;

    //id of segmented elements
    QVector<int> _M_id_GM;
    QVector<int> _M_id_BuriedBlood;
    QVector<int> _M_id_SurfaceBlood;

    //HS wavelength
    QVector<QVector<int> > _M_HbO2_Hb_HS_cam_idx;
    QVector<QVector<int> > _M_HbO2_Hb_oxCCO_HS_cam_idx;


    //quantify oxCCO
    bool    _M_quantify_oxCCO;

    //optimize HS quantification using optimal wavelength
    bool _M_optimize_HS_quantification;

    //Hyperspectral spectral range
    int _M_HS_lambda_min;
    int _M_HS_lambda_max;

    //spectral idx
    QVector<int> _M_spectral_idx;


    //Result directory
    QString _M_result_directory;
};

#endif // PPIXELWISEMOLARCOEFF_H
