/**
 * @file filtering.h
 *
 * @brief Set of functions used to process temporal filtering in the Fourier domain.
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */


#ifndef FILTERING_H
#define FILTERING_H

#include "polyfit.hpp"
#include <fftw3.h>
#include <omp.h>
#include "Statistic_functions.h"
#include "acquisition.h"
#include <QtMath>

#include <QVector>
#define PI 3.141592653589793



/** Create low pass spectral filter using a Blackman window */
void Blackman_Window(const double Fs, const double Fc, QVector<float> &filter, const int N);



/** apply filtering (low pass filtering + data correction) */
void Apply_Filtering(const QVector<float> &x, QVector<float> &data,const fftwf_plan &p, const fftwf_plan &q, const QVector<float> &filter);

/** apply data correction */
void Apply_Data_Correction(const QVector<float> &x, QVector<float> &data);

/** apply data correction with start and end index */
QVector<float> Apply_Data_Correction(const QVector<float> &data, int start_acquis, int end_acquis, int rest1_start, int rest1_end, int rest_last_start, int rest_last_end);

/** apply low pass filtering */
void Apply_Low_Pass_Filtering(const QVector<float> &x, QVector<float> &data,const fftwf_plan &p, const fftwf_plan &q, const QVector<float> &filter);




#endif // FILTERING_H
