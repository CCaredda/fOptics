/**
 * @file P_SPM.h
 *
 * @brief This class aims to apply the Statistical Parametric Mapping methodology (fMRI standard for identifying functional brain areas)
 * on images of chromophore's concentration changes. This output of this class is a T-Stat matrix that indicates at which point the
 * measured data reject the null hypothesis (no cortical activation).
 *
 * In this class, the random field theory is also implemented and aims to threshold the T-Stats matrix in order to get the extent
 * of the activated cortical areas.
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */


#ifndef P_SPM_H
#define P_SPM_H

#include <opencv2/core/cuda.hpp>
#ifdef __CUDA_ARCH__
    #include <opencv2/cudaarithm.hpp>
#endif

#include "Statistic_functions.h"
#include "acquisition.h"
#include <QObject>
#include <QCoreApplication>

class P_SPM : public QObject
{
    Q_OBJECT
public:
    explicit P_SPM(QObject *parent = nullptr);

    /** Apply the General linear model to test the linear association between X and Y */
    void GeneralLinearModel(const Mat &Y, const Mat &X);

    //Design Matrices
    /** Build the Design Matrix that contains the physiological apriori from a vector*/
    Mat Build_Design_Matrix(const QVector<float> &Signal, int nb_frames);

    QVector<float> get_Mean_Expected_DeltaC(const QVector<float> &HRF,float apriori,QVector<int> &activity_start,QVector<int> &activity_end);
    float get_Mean_Expected_DeltaC(const QVector<float> &HRF, float apriori, int activity_start, int activity_end);

    // Get statistic maps
    /** Get the Z-Stats map */
    Mat get_Z_map();

    /** Get the T-Stats map */
    Mat get_T_map() {return _M_T_map;}

    /** Set statistical significance */
    void set_Statistical_Significance_Level(int);

    /** On new z-stats threshold value (used for statistical inferences) */
    void setZThreshValue(double v) {_M_z_thresh =v;}


    /** set Nb of resels */
    void setNbResels(int nb);
    /** Get Nb of resels */
    int getNbResels()        {return _M_resels;}

    /** set FWHM (for the Gaussian kernel used in the random field theory */
    void setFWHM(double v)  {_M_FWHM=v;}
    /** get FWHM (for the Gaussian kernel used in the random field theory */
    double getFWHM()        {return _M_FWHM;}

    /** Convert r to z (Fisher transform) */
    Mat convert_Correlation_Map_to_Z_Map(const Mat &in);

    /** Get thresholded Z maps according to the Random Field theory */
    Mat getThresholdedZMap(const Mat &Z_map);

    /** Write SPM infos in temp directory */
    void Write_SPM_info(QString saving_dir);

    /** Process the whole SPM procedure to get activation map */
    Mat process_SPM(Mat Deltta_C_map, QVector<cv::Point> pixels_pos, Size img_size, QString path, QVector<float> &model,Mat &mask_SPM);
    void process_SPM_Phase_Correlation(Mat Delta_C_map, QString path, QVector<float> &model,QVector<cv::Point> pixels_pos, cv::Size img_size);

    /** Gaussian blur */
    Mat gaussianBlur(const Mat &in);

signals:
    /** progress statut */
    void newProgressStatut(QString,int);

    /** emit new z threshold calculated by random field theory */
    void newZThresh(double);

private:

    //Resels vs z threshold (obtained by solving E[EC] = 0.05)
    QVector<double> _M_z_thresh_possible_values;

    //T Map
    Mat _M_T_map;

    //Nb of resels
    int _M_resels;

    //Ful-width Half Maximum (FWHM)
    double _M_FWHM;

    //Z threshold
    double _M_z_thresh;

    //GPU acceleration
    bool    _M_GPU_enabled;

    //hemodynamic response function
    QVector<float> _M_HRF;


    //effective degree of freedom (Satterthwaithe approximation)
    //e: errors size(T,N)
    //R: Residuals in GLM computation size(T,T)
    QVector<double> _effective_degree_of_freedom_GLM(const Mat& R,const Mat &e);

    //GLM T statistics
    //invert_X = (X'X)-1 size (S=2,S=2)
    //B size(S=2,N)
    //var_e size (1,N)
    //T_map size(1,N)
    // stimulus_id : 0 (activation) 1 (non activation)
    void _GLM_t_stats(const Mat &invert_X,const Mat &B,const Mat &var_e,Mat &T_map,int stimulus_id);

    //Activation indicator
    void _get_Activation_indicator_Bonferroni(const Mat& T_map,Mat &activation_indicator,double alpha,QVector<double> &degree_of_freedom);
    void _get_Activation_indicator_Bonferroni2(const Mat& T_map,Mat &activation_indicator,double alpha,double &degree_of_freedom);


    //General model (CPU or GPU handling)
    void _CPU_GeneralLinearModel(const Mat &Y,const Mat &X);
    void _GPU_GeneralLinearModel(const Mat &Y,const Mat &X);


};


#endif // P_SPM_H
