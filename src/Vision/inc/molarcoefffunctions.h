/**
 * @file molarcoefffunctions.h
 *
 * @brief Set of functions used to compute matrices of coefficients used in the modified Beer Lambert law.
 * These matrices and then used to get chromophore's concentration changes.
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef MOLARCOEFFFUNCTIONS_H
#define MOLARCOEFFFUNCTIONS_H

#include <QVector>
#include <QString>

#include "conversion.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QStringList>

using namespace cv;
using namespace std;

/** Function used in the image segmentation procedure. Remove the small contours. */
void EraseSmallContour(Mat &img,int area_lim_in_px);
/** Function used in the image segmentation procedure.*/
void fillContour1_If_Contour2_IsNotIncluded(Mat &img,vector<vector<Point> > &c1,vector<vector<Point> > &c2);

/** Function used in the image segmentation procedure. Handle internal contour, it calls fillContour1_If_Contour2_IsNotIncluded function*/
void handleInternalContour(Mat &img1, Mat &img2);


/** Sum of a QVector vector */
double vector_sum(QVector<double> a);

/** Element wise multiplication between two vectors */
QVector<double> element_wise_multiplication(QVector<double> &a,QVector<double> &b);

/** Multiply a vector by a factor */
void scale_vector(QVector<double> &in_out,double factor);

/** Matrix inversion */
void invertMatrix(Mat& in,Mat& out);
/** Matrix inversion */
void invertMatrix(QVector<Mat>& in,QVector<Mat>& out);

/** Data integration (3 chromophores) */
void integrateDatas_Hb_HbO2_oxCCO(QVector<Mat> &out, QVector<QVector<double> > &spectral_res_Hb, QVector<QVector<double> >&spectral_res_HbO2, QVector<QVector<double> >&spectral_res_oxCCO, QVector<double> &Mean_path, QVector<QVector<int> > &id);
/** Data integration (3 chromophores) */
void integrateDatas_Hb_HbO2_oxCCO(QVector<Mat> &out, QVector<QVector<double> > &spectral_res_Hb, QVector<QVector<double> >&spectral_res_HbO2, QVector<QVector<double> >&spectral_res_oxCCO, QVector<double> &Mean_path);
/** Data integration (2 chromophores) */
void integrateDatas(QVector<Mat> &out, QVector<QVector<double> > &spectral_res_Hb, QVector<QVector<double> >&spectral_res_HbO2, QVector<double> &Mean_path, QVector<QVector<int> > &id);
/** Data integration (2 chromophores) */
void integrateDatas(QVector<Mat> &out, QVector<QVector<double> > &spectral_res_Hb, QVector<QVector<double> >&spectral_res_HbO2, QVector<double> &Mean_path);

/** Get distance of grey matter pixels to the nearest blood vessel */
void getDistanceMatrix(float reso_in_mm,Mat &dst_matrix,Mat &blood_vessel_type,Mat &mask,Mat &S_BV,Mat &B_BV,Mat &unused);


/** return the id of the best spectral match */
void find_best_simulated_spectra(QVector<QVector<double> > &simulated_spectra, QVector<double> &in_spectra, int &id, float &error);

#endif // MOLARCOEFFFUNCTIONS_H
