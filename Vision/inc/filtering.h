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


/** Low pass Filtering of datas with data correction */
void Auto_Process_Filtering(const QVector<float> &x, QVector<float> &Low_pass_data,
                            const fftwf_plan &p, const fftwf_plan &q,
                            const QVector<float> &filterLowPass);

/** Low pass Filtering of datas without data correction */
void Auto_Process_Filtering_whithout_redress(const QVector<float> &x, QVector<float> &Low_pass_data,
                            const fftwf_plan &p, const fftwf_plan &q, const QVector<float> &filterLowPass);

/** Create low pass spectral filter using a Blackman window */
void Blackman_Window(const double Fs, const double Fc, QVector<float> &filter, const int N);


/** Convolution */
float Singleconvolve(const QVector<float> &v1,const QVector<float> &v2);

/** Get a cardiac filter (Gaussian window) */
QVector<float> get_Cardia_Filter(float half_width, float mean_val, float F_min, float dF, int length);

/** apply filtering (low pass filtering + data correction) */
void Apply_Filtering(const QVector<float> &x, QVector<float> &data,const fftwf_plan &p, const fftwf_plan &q, const QVector<float> &filter);

/** apply data correction */
void Apply_Data_Correction(const QVector<float> &x, QVector<float> &data);

/** apply data correction with start and end index */
QVector<float> Apply_Data_Correction(const QVector<float> &data, int start_acquis, int end_acquis, int rest1_start, int rest1_end, int rest_last_start, int rest_last_end);

/** apply low pass filtering */
void Apply_Low_Pass_Filtering(const QVector<float> &x, QVector<float> &data,const fftwf_plan &p, const fftwf_plan &q, const QVector<float> &filter);



/** get phase difference between two temporal vectors */
QVector<float> calculatePhaseShift_Hilbert(const QVector<float> in1, const QVector<float> in2,fftwf_plan plan_forward, fftwf_plan plan_backward);

/** Hilbert transform */
fftwf_complex* hilbert(const QVector<float> &data, fftwf_plan plan_forward, fftwf_plan plan_backward);

/** Calculate Phase correlation signal */
QVector<float> calculate_correlation_Phase(const QVector<float> in1, const QVector<float> in2,fftwf_plan plan_forward, fftwf_plan plan_backward);

double calculatePhaseShift_xCorr(QVector<float>& signal1,QVector<float>& signal2);

#endif // FILTERING_H
