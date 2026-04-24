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


#ifndef PPIXELWISEMOLARCOEFF_COPY_H
#define PPIXELWISEMOLARCOEFF_COPY_H

#include <QDir>
#include <QObject>
#include <QThread>
#include "molarcoefffunctions.h"
#include "acquisition.h"

#define Reso_MeanPath_mm 0.1



class PPixelWiseMolarCoeff : public QThread
{
    Q_OBJECT
public:
    explicit PPixelWiseMolarCoeff(QObject *parent = nullptr);
    ~PPixelWiseMolarCoeff();

    /** Has init been done ? */
    bool IsInitHasBeenDone()        {return !_M_init_or_process;}

    /** Get distance to blood vessel for a specific point */
    float getDistanceToBloodVessels(Point p);

    /** Set image resolution */
    void setResolution(float reso)  {_M_reso_in_mm = reso;}

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


    /** set mode Hyperspectral */
    void setModeHyperSpectral(bool v)    {_M_mode_HyperSpectral=v;}


    /** Get id of segmented image (grey matter layer) */
    QVector<int> get_GM_id()            {return _M_id_GM;}
    /** Get id of segmented image (buried blood vessel layer) */
    QVector<int> get_BuriedBlood_id()   {return _M_id_BuriedBlood;}
    /** Get id of segmented image (surface blood vessel layer) */
    QVector<int> get_SurfaceBlood_id()  {return _M_id_SurfaceBlood;}


    /** Optimize hyperspectral quantification using optimal wavelength */
    void setHSOptimizedQuantification(bool v);
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
    void newHyperspectralWavelengthConfig(QVector<QVector<int> >);

public slots:
    /** Consider Blood vessel distance */
    void ConsiderBloodVesselDistance(bool v);

private:

    //Load spectral resultant
    void _Load_spectral_resultant();

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

    //Mode HS or RGB
    bool        _M_mode_HyperSpectral;

    //Consider blood vessel distance
    bool        _M_consider_blood_vessel_distance;

    //bool init MolarCoeff
    bool        _M_init_or_process; //true: init, false: process


    float       _M_reso_in_mm;
    int         _M_vessel_lim_in_px;
    int         _M_area_lim_in_px;

    //epsHb x spectral resultant
    QVector<QVector<double> > _M_eps_Hb_x_res_spectral;

    //epsHbO2 x spectral resultant
    QVector<QVector<double> > _M_eps_HbO2_x_res_spectral;

    //eps oxCCO x spectral resultant
    QVector<QVector<double> > _M_eps_oxCCO_x_res_spectral;

    //Mean path
    QVector<QVector<double> > _M_Mean_path_Surface_Blood;
    QVector<QVector<double> > _M_Mean_path_Buried_Blood;


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


};

#endif // PPIXELWISEMOLARCOEFF_COPY_H
