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

#include<QVariant>
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


/** Get the mean value of a vector of float */
float get_Mean(const QVector<float> &vec,int id_start=-1,int id_end=-1);


/** Get the max value of a vector of float */
double get_Max(const QVector<float> &vec);

/** Get the max value of a vector of float */
double get_Max(const QVector<QVector<float> > &vec);


/** Get the sum of a vector of double */
double get_Vector_sum(const QVector<double> &vec);


/** Normalize with vector max value */
void NormalizeMaxVector(QVector<double> &vector);


/** Compute Pearson correlation coefficient between two random variables */
double pearsoncoeff(const QVector<float> &X, const QVector<float> &Y);
/** Compute Pearson correlation coefficient between two random variables */
double pearsoncoeff(const QVector<float> &X,const QVector<float> &Y,const int &process_duration);



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
