/**
 * @file loadinfos.h
 *
 * @brief Set of functions used to load acquisition info from a image dataset.
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */


#ifndef LOADINFOS_H
#define LOADINFOS_H

#include <QString>
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QObject>
#include <QVector>
#include <QDebug>
#include <cmath>
#include <QRegularExpression>
#include <QCoreApplication>

#include <QFileInfo>
#include <opencv2/opencv.hpp>


using namespace std;

/** get video backend */
int preferredBackendForFile(const QString& filePath);

/** Get share path */
QString getShareDirPath(QString key_name);

/** Load frame rate */
double LoadFrameRateInfo(QString path);
/** Load number of frames */
QVector<int> LoadTotalFrameCountInfo(QString path);

/** Load time process */
void LoadTimeProcessInfo(QString path, QVector<int> &process_id, int tot_frames);

/** Load start and end of the analysis
*  -1: no analysis
*  0 : task-based
*  1: resting state
*  2: Impulsion*/
void LoadStartEndAnalysisFrames(QString path, int analysis, int &start_frame, int &end_frame);

/** Load analysis */
QVector<int> LoadAnalysisType(QString path);


/** Load camera type (RGB or HSI) */
int LoadCameraType(QString path);


/** Load camera name */
QString LoadCameraName(QString path);

/** Load light source type */
QString LoadLightSourceType(QString path);


/** Check if acquisition info file contains all required info */
QStringList check_acquisition_info_file(const QString &path);

#endif // LOADINFOS_H
