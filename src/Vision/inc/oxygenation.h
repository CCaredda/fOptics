/**
 * @file oxygenation.h
 *
 * @brief Set of functions used to calculate chromophore's concentration changes using the modified Beer Lambert law
 * and to compute the Pearson correlation coefficient between expected and measure time curves.
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef OXYGENATION_H
#define OXYGENATION_H

#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <cmath>


#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include "vblobForeach.h"
#include "pobject.h"
// #include "filtering.h"
#include "Statistic_functions.h"


using namespace cv;
using namespace std;

/*****************************************************
***************Mesure quantitative********************
*************** **************************************/

/*****************************************************
***************Mesure probabiliste********************
*************** **************************************/

/** Compare measure contrast to the theoretical one using Pearson correlation coefficient */
void Compare_Bold_Vs_Oxy(QVector<QVector<float> > &contrast, QVector<QVector<float> > &model, QVector<double> &stat_vec);

/*****************************************************
***************MBLL functions*************************
*************** **************************************/
/** Get intensities averaged during the first rest period */
bool getReference_Single_Deconvolution(const QVector<QVector<float> > &temporal_vec,int start,int end,int NbChannels,QVector<float> &mean_val);
/** Process chromophore deconvolution using a single Matrix to project data */
void process_Single_Deconvolution(const QVector<QVector<float> > &temporal_vec, int t, QVector<float> &OD, QVector<float> &mean_val, int NbChannels, int nb_chromophore, const Mat &Molar_coeff, QVector<float> &measure);


#endif //OXYGENATION_H




