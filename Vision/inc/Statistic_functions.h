/**
 * @file Statistic_functions.h
 *
 * @brief Set of functions used to compute statistical tests (T tests, person correlation ...)
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef STATISTIC_FUNCTIONS_H
#define STATISTIC_FUNCTIONS_H

#include <QVector>
#include <QDebug>
#include <QObject>


#include <cmath>
#include <omp.h>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <boost/math/distributions/students_t.hpp>
// #include "acquisition.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

#define PI 3.141592653589793

using namespace cv;
using namespace std;
using namespace boost::math;

/** two samples t test. It returns true if the null hypothesis is rejected */
bool two_samples_t_test_different_sd(double Sm1,       // Sm1 = Sample 1 Mean.
        double Sd1,       // Sd1 = Sample 1 Standard Deviation.
        unsigned Sn1,     // Sn1 = Sample 1 Size.
        double Sm2,       // Sm2 = Sample 2 Mean.
        double Sd2,       // Sd2 = Sample 2 Standard Deviation.
        unsigned Sn2,     // Sn2 = Sample 2 Size.
        double alpha,       // alpha = Significance Level.
        double &pvalue, double &t_stat, int stat_type);

/** One sample t-test. Compare one sample to a true mean. It returns true of the considered hypothesis is rejected*/
bool single_sample_t_test(double True_Mean, double Sample_mean, double Sample_std, unsigned Sample_size, double alpha,int stat_type);
/** One sample t-test. Compare one sample to a true mean. It returns the t-stat associated to the considered hypothesis*/
float get_T_stats_Compare_To_True_Mean(double True_Mean, double Sample_mean, double Sample_std, unsigned Sample_size);

/** Two samples t-test. It returns the t-stat associated to the considered hypothesis*/
float get_T_stats_Compare_Two_Means(
        double Sm1,       // Sm1 = Sample 1 Mean.
        double Sd1,       // Sd1 = Sample 1 Standard Deviation.
        unsigned Sn1,     // Sn1 = Sample 1 Size.
        double Sm2,       // Sm2 = Sample 2 Mean.           -> reference
        double Sd2,       // Sd2 = Sample 2 Standard Deviation.
        unsigned Sn2     // Sn2 = Sample 2 Size.
        );

/** Test on correlation coefficient */
bool test_on_corr(double corr, int n, int stat_type, double alpha, double &pvalue);


/** Compute KullBacl Leibler divergence to compare to density probabilities */
double get_KL_Divergence(const QVector<double> &pk, const QVector<double> & qk);
/** Compute spectral information divergence */
float get_SID(QVector<float> pk, QVector<float> qk);

/** 1D Convolution */
QVector<float> conv(const QVector<float> &f, const QVector<float> &g);

/** get Gaussian function */
QVector<float> get_Gaussian(float Half_width,float mean_val,const QVector<float> &x);

/** Get std mean for a vector of float */
void get_Std_Mean(const QVector<float> &vec,float &stdVal,float &meanVal);

/** Get the mean value of a vector of float */
float get_Mean(const QVector<float> &vec,int id_start=-1,int id_end=-1);

/** Get the median value of a vector of float */
float get_Median(const QVector<float> &vec,int id_start=-1,int id_end=-1);

/** Get the max value of a vector of float */
double get_Max(const QVector<float> &vec);

/** Get the max value of a vector of float */
double get_Max(const QVector<QVector<float> > &vec);

/** Get the min value of a vector of float */
double get_Min(const QVector<float> &vec);

/** Get the min value of a vector of float */
double get_Min(const QVector<QVector<float> > &vec);

/** Get the idx that contains the max value of a vector of float */
int argMax(const vector<float> &vec);
int argMax(const vector<double> &v);
/** Get the idx that contains the min value of a vector of float */
int argMin(const vector<float> &vec);

/** Get the sum of a vector of double */
double get_Vector_sum(const QVector<double> &vec);

/** Normalize vectors by its sum: get a probability density */
void NormalizeVector(QVector<double> &vector);
/** Normalize vectors by its sum: get a probability density */
void NormalizeVector(QVector<float> &vector);

/** Normalize with vector max value */
void NormalizeMaxVector(QVector<double> &vector);

/** Process euclidian dist between two vectors */
double processReliabilityFactor(const QVector<float> &Mesure, const QVector<float> &Model);

/** Compute Pearson correlation coefficient between two random variables */
double pearsoncoeff(const QVector<float> &X, const QVector<float> &Y);
/** Compute Pearson correlation coefficient between two random variables */
double pearsoncoeff(const QVector<float> &X,const QVector<float> &Y,const int &process_duration);

/** Convert proba to percentage */
double _ThreshProbaToFitPercentage(double v);
/** Convert percentage to proba */
double _FitPercentageToThreshProba(int v);

/** Addition operator for two float vectors */
QVector<float> operator+(const QVector<float> &a,const QVector<float> &b);
/** Substraction operator between a vector of float and a constant */
QVector<float> operator-(const QVector<float> &a,const float &b);
/** Division operator between a vector of float and a constant */
QVector<float> operator/(const QVector<float> &a,const float b);
/** Division operator between a vector of double and a constant */
QVector<double> operator/(const QVector<double> &a,const double b);


/** Autocorrelation function. correlation of a vector by itself */
Mat autocorrelation(const Mat &in);
/** Autocorrelation function. correlation of a vector by itself */
Mat autocorrelation(const Mat &in,int spatial_id);

/** Extract variance from Matrix
in size(T,N)
out size(1,N)
*/
void extractVarianceFromMat(const Mat& in,Mat &out);


/** Trace of a matrix product */
double getTrace_of_product(const Mat &a,const Mat &b);

/** Fisher conversion. Convert correlation coefficient to z value */
float fisher_r_to_z_transform(float r);
/** Convert t-stats to z-stats */
float convert_T_to_Z_stats(float t);
/** Convert correlation coefficients to t stats */
float convert_Corr_to_T_stats(float R,int df);



#endif // STATISTIC_FUNCTIONS_H
