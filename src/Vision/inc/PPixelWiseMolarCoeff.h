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
#include "loadinfos.h"
#define Reso_MeanPath_mm 0.1



class PPixelWiseMolarCoeff : public QThread
{
    Q_OBJECT
public:
    explicit PPixelWiseMolarCoeff(QObject *parent = nullptr);
    ~PPixelWiseMolarCoeff();

    /** Has init been done ? */
    bool IsInitHasBeenDone()        {return !_M_init_or_process;}


    /** Set Pixel pos inside the ROI */
    void setPixelsPos(QVector<Point> &pixel_pos)    {_M_pixels_pos.clear(); _M_pixels_pos = pixel_pos;}

    /** init molar coeff set the Grey matter coeff matrix for all pixels inside the ROI*/
    void initMolarCoeff();

    /** Process molar coeff computation for all pixels according to the segmentation layers */
    void processMolarCoeff();





    /** Get molar coeff at specific point */
    bool getMolarCoeff(int id,QVector<Mat> &coeff);


    /** Check if the molar coeff matrix is not empty at specific point */
    bool isEmptyMatrix(int id);


    /** Set Optical device idx */
    void setOpticalConfigIDx(int camera_id, int light_source_id);




    /** get number of chromophore quantified */
    int getChromophoreNumber();



protected:
    /** Call process in parallel thread */
    void run();

signals:
    /** Inform that process is finished */
    void processFinished();

public slots:

    /** New result directory */
    void onNewResultDirectory(QString v)    {_M_result_directory = v;}

private:

    //Compute MBLL matrix
    QVector<Mat> _Compute_MBLL_Matrix(QVector<double> &mean_path);

    //Load spectral resultant
    void _Load_spectral_resultant();

    //Load Mean path length
    void _Load_Mean_path_Length();

    //init data
    void _initData();

    //Display Matrix
    void _DisplayMatrix(Mat);


    //bool init MolarCoeff
    bool        _M_init_or_process; //true: init, false: process





    //Extinction coefficients (in cm-1.mol-1.L)
    QVector<double> _M_epsilon_Hb;
    QVector<double> _M_epsilon_HbO2;
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

    //Mean path (in cm)
    QVector<QVector<double> > _M_Mean_path_Surface_Blood;
    QVector<QVector<double> > _M_Mean_path_Buried_Blood;
    QVector<double> _M_wavelength_mean_path;


    //Pixels pos
    QVector<Point>  _M_pixels_pos;


    //Molar coeffs
    QVector<QVector<Mat> >  _M_Molar_coeff; //Size (nb pixel,nb deconvolution systems)

    QVector<Mat>            _M_Molar_coeff_BuriedBlood;
    QVector<Mat>            _M_Molar_coeff_SurfaceBlood;
    QVector<Mat>            _M_Molar_coeff_GM;




    //spectral idx
    QVector<int> _M_spectral_idx;


    //Result directory
    QString _M_result_directory;
};

#endif // PPIXELWISEMOLARCOEFF_H
