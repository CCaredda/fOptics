/**
 * @file pobject.h
 *
 * @brief File containing structures used in the whole project
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef POBJECT_H
#define POBJECT_H



#include <QObject>
#include "conversion.h"


/** Structure used to assess RGB values of a RGB image */
#pragma pack(1)
typedef struct
{
    uchar b;
    uchar g;
    uchar r;
}_Mycolor;
#pragma pack()

/** Structure used after the acquisition of a RGB image */
typedef struct
{
    Mat     img;
    int     thread_id;
    int     process_val;
}_Processed_img;

/** Structure used after the acquisition of a hyperspectral image */
typedef struct
{
    vector<Mat> img;
    int         thread_id;
    int         process_val;
}_Processed_img_HS;

/** Structure used to get the normalized cross-correlation (indicator of the efficiency of the motion compensation) */
typedef struct
{
    float NCC;
    int thread_id;
}_NCC_result;


/** Structure used to indicate data to save in PVison class */
typedef struct
{
    bool save_non_filtered_data;
    bool save_camera_intensity;
    bool save_Delta_A;
    bool save_Delta_C;
    bool save_Delta_C_rest_activity;
    bool save_SPM;
}_data_saving_info;


#endif // POBJECT_H
