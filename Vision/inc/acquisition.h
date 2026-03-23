/**
 * @file acquisition.h
 *
 * @brief Set of functions used to extract data from images.
 * Definition of global variables to facilitate the reading of the code
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef ACQUISITION_H
#define ACQUISITION_H


#include <QObject>
#include <QVector>
#include "pobject.h"
#include <QDebug>
#include "vblobForeach.h"
#include "oxygenation.h"
#include "filtering.h"
#include <QRegularExpression>
#include <QDir>
#include <QCoreApplication>

#define Cluster_Grey_Matter 1
#define Cluster_Blood_Surface 2
#define Cluster_Burried_Blood 3

#define Spectral_bands 25


#define Activation_GLM_Pixel_wise 0
#define Process_Mean_Delta_C 1
#define Process_Correlation 2
#define No_Analysis 3





#define Camera_RGB_Basler 0
#define Camera_HS_Ximea_corr 1
#define Camera_HS_Ximea_no_corr 2


#define Light_source_Halogen 0


//#define Activation_stat_2_Samples_T_Tests 3

//STRUCTURES
typedef struct
{
    QVector<QVector<QVector<float> > >  *data;
    QVector<Point>                      *pixels_pos;
    const Mat                           *img;
    int                                 *spatial_sampling;
    int                                 *id;
    int                                 *img_id;

}_Datas_infos;

typedef struct
{
    QVector<QVector<QVector<float> > >  *data;
    QVector<Point>                      *pixels_pos;
    const vector<Mat>                   *img;
    int                                 *spatial_sampling;
    int                                 *id;
    const Mat                           *coeff_corr;
}_HyperSpectral_Datas_infos;

typedef struct
{
    const Mat   *imgref;
    const Mat   *img2;
    float      *sum_ref;
    float      *sum_squared_ref;
    float      *sum_cp;
    float      *sum_squared_cp;
    float      *sum_prod;
}_Datas_NCC;

typedef struct
{
    int   *id;
    Mat   *raw_data;
    Mat   *img;
    int   *spatial_sampling;
}
_Raw_Datas;



/** Decode Light source type */
int decode_Light_source_type(QString code);

/** Decode camera name */
int decode_Camera_name(QString code);


/** Add image intensities to Datas vectors */
void addData(const Mat &mat,std::vector<cv::Point> &ROI_contour,QVector<QVector<QVector<float> > > &_data,int spatial_sampling,int img_id);
/** Add image intensities to Datas vectors */
void addData(const Mat &mat, std::vector<cv::Point> &ROI_contour, QVector<QVector<QVector<float> > > &_data, int spatial_sampling);

/** Get pixel pos vector */
void getPixelPos(QVector<cv::Point> &pixels_pos, std::vector<cv::Point> &ROI_contour, cv::Size size, int spatial_sampling);

/** Reconstruct a 2D image from an inline image */
void reConstructImg(const QVector<int> &labels, Mat &img_out, const QVector<Point> &pixels_pos, cv::Size size, int Sampling);
/** Reconstruct a 2D image from an inline image */
void reConstructImg(const Mat &labels, Mat &img_out, const QVector<Point> & pixels_pos, cv::Size size, int Sampling);
/** Construct an inline image from a 2D image */
void CreateRawDataVector(Mat &datas_row,std::vector<cv::Point> &ROI_contour,Mat &img,int spatial_sampling);

/** Write temporal vector */
void WriteTemporalVector(const QString path,const QVector<QVector<float> > &tempoal_vec);
void WriteTemporalVector(const QString path,const QVector<float> &tempoal_vec);
void WriteTemporalVector(const QString path,fftwf_complex  *tempoal_vec, int N);
void WriteTemporalVector(const QString path,const QVector<double> &tempoal_vec);
void WriteTemporalVector(const QString path,const QVector<int> &tempoal_vec);
void WriteTemporalVector(const QString path,const QVector<QVector<double> > &tempoal_vec);


/** Write Point info */
void WritePointVector(const QString path,const QVector<Point> &P);
void WritePointVector(const QString path,const QVector<QPoint> &P);
/** Write BGR intensities */
void WriteBGRVector(const QString path,const QVector<Scalar> &bgr);
/** Write float image */
void WriteFloatImg(const QString path,const Mat img);
/** Write Double image */
void WriteDoubleImg(const QString path,const Mat img);

/** Read float image */
void ReadFloatImg(const QString path,Mat &img);
/** Read vector */
void ReadVector(const QString path,QVector<double> &out);
void ReadVector(const QString path,QVector<float> &out);
void ReadVector(const QString path,QVector<QVector <float> > &out);
void ReadDoubleVector(const QString path,QVector<QVector <double> > &out);

/** Load point vector */
bool LoadPointVector(const QString path, QVector<QPoint> &point_vec);
/** Load rect ROI */
bool LoadRectROI(const QString path, QPoint &P1,QPoint &P2);
/** Load number in file */
double LoadNumberinFile(const QString path, double value_if_no_file);

/** Load matrix */
void load_Matrix(QString path, QVector<QVector<double> > &matrix);
void load_Matrix(QString path,QVector<QVector<int> > &matrix);

/** Compute Normalized cross correlation */
float NormalizeCrossCorrelation(const Mat &matref,const Mat &mat2,std::vector<cv::Point> &ROI_contour);
/** Create contrast image */
void CreateContrastImg(Mat &img_contrast, int spatialSampling, QVector<Point> &Pixel_pos, QVector<double> &contrast);

/** Create Buffer img*/
void FirstUpdateBuffer(const vector<Point> &contour,const Mat &img,QVector<QVector<QVector<float> > > &buffer);
void addValueToBuffer(const vector<Point> &contour,const Mat &img,QVector<QVector<QVector<float> > > &buffer);
void updateBuffer(const vector<Point> &contour,const Mat &img,QVector<QVector<QVector<float> > > &buffer);

/** Create random color */
Scalar getRandomColor();
/** Get random color */
QVector<Scalar> getRandomColor(int nb_colors);

/** Merge two images with transparency */
Mat mergeImg(const Mat &in,Mat &mask,double alpha);

/** Copy a Hypercube */
_Processed_img_HS copyHyperCube(_Processed_img_HS &in);


/** Extract alias command from ~./bashrc file */
QString extractAliasCommand(const QString& aliasName);


/** Launch Python script */
bool launchPythonScript(QString python_script_filename, QString path_res, QString path_models);



#endif // ACQUISITION_H
