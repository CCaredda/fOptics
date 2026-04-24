#include "conversion.h"


Mat qimage_to_mat_ref(QImage &img, int format)
{
    return Mat(img.height(), img.width(),
            format, img.bits(), img.bytesPerLine());
}

QImage mat_to_qimage_ref(cv::Mat &mat, QImage::Format format)
{
  return QImage(mat.data, mat.cols, mat.rows, mat.step, format);
}


void vector2QVector(const vector<vector<Point> > &c,QVector<QVector<QPoint> > &C,Point offset)
{
    for(unsigned int i=0;i<c.size();i++)
    {
        QVector<QPoint> temp;
        for(unsigned j=0;j<c[i].size();j++)
            temp.push_back(QPoint(c[i][j].x+offset.x,c[i][j].y+offset.y));

        C.push_back(temp);

    }
}


// Convert QVector to Mat
Mat QVector2Mat(QVector<QVector<float> > &c)
{
    if(c.empty())
        return Mat::zeros(0,0,CV_64F);

    Mat out = Mat::zeros(c[0].size(),c.size(),CV_64F);

    for(int k=0;k<c.size();k++)
    {
        for(int i=0;i<c[0].size();i++)
        {
            out.at<double>(i,k) = c[k][i];
        }
    }

    return out;
}


QString type2str(int type) {
  QString r;

  uchar depth = type & CV_MAT_DEPTH_MASK;
  uchar chans = 1 + (type >> CV_CN_SHIFT);

  switch ( depth ) {
    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;
  }

  r += "C";
  r += QChar('0' + chans);
  return r;
}




