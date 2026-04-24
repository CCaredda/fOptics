/**
 * @file conversion.h
 *
 * @brief Set of functions enabling data conversion.
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */


#ifndef CONVERSION_H
#define CONVERSION_H

#include <QDebug>
#include <QImage>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/types_c.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc_c.h"

#define Row_offset 3
#define Col_offset 0
#define Size_filter 5


using namespace cv;
using namespace std;

typedef struct
{
    Mat     img;
    int     index;
}_HS_img;



/** Convert QImage images (Qt images) to Mat image (OpenCV format) */
Mat qimage_to_mat_ref(QImage &img, int format);
/** Convert Mat image (OpenCV format) to QImage images (Qt images) */
QImage mat_to_qimage_ref(cv::Mat &mat, QImage::Format format);
/** Convert std vector to QVector */
void vector2QVector(const vector<vector<Point> > &c, QVector<QVector<QPoint> > &C, Point offset);
/** Convert OpenCV image format as QString */
QString type2str(int type);

/** Convert QVector to Mat */
Mat QVector2Mat(QVector<QVector<float> > &c);


#endif // CONVERSION_H
