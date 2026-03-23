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


void getMaxImageFromHypercube(vector<Mat> &img_in,Mat &img_out)
{
    img_out = Mat::zeros(img_in[0].size(),CV_16UC1);

    for(int row=0 ; row<img_out.rows ; row++)
    {
        ushort *ptr_out  = img_out.ptr<ushort>(row);
        ushort *ptr_in ;

        for(int col=0 ;col<img_out.cols ; col++)
        {
            ushort max_val = 0;
            for (int i = 0 ; i<Size_filter*Size_filter;i++)
            {
                 ptr_in  = img_in[i].ptr<ushort>(row);
                 max_val = (ptr_in[col] > max_val) ? ptr_in[col] : max_val;

            }
            ptr_out[col]     = max_val;
        }
    }
}


void getMinImageFromHypercube(vector<Mat> &img_in,Mat &img_out)
{
    img_out = Mat::zeros(img_in[0].size(),CV_16UC1);

    for(int row=0 ; row<img_out.rows ; row++)
    {
        ushort *ptr_out  = img_out.ptr<ushort>(row);
        ushort *ptr_in ;

        for(int col=0 ;col<img_out.cols ; col++)
        {
            ushort max_val = pow(2,16)-1;
            for (int i = 0 ; i<Size_filter*Size_filter;i++)
            {
                 ptr_in  = img_in[i].ptr<ushort>(row);
                 max_val = (ptr_in[col] < max_val) ? ptr_in[col] : max_val;

            }
            ptr_out[col]     = max_val;
        }
    }
}


//Create hyperpcube
void CreateHyperCube(const Mat &img_in,vector<Mat> &img_out)
{

    int nb_SpectralPixel_per_col    = (int)((img_in.cols-Col_offset)/Size_filter);
    int nb_SpectralPixel_per_row    = (int)((img_in.rows-Row_offset)/Size_filter);

    img_out.clear();
    img_out.resize(Size_filter*Size_filter);
    for(unsigned int i=0;i<img_out.size();i++)
    {
        img_out[i] = Mat::zeros(nb_SpectralPixel_per_row,nb_SpectralPixel_per_col,CV_16UC1);
    }



    for(int row=0 ; row<nb_SpectralPixel_per_row ; row++)
    {
        for(int col=0 ;col<nb_SpectralPixel_per_col ; col++)
        {
            // Hyper spectral filter
            int id=0;
            for (int i = 0 ; i<Size_filter;i++)
            {
                const ushort *ptr_in    = img_in.ptr<ushort>(Row_offset+row*Size_filter+i);

                for(int j=0;j<Size_filter;j++)
                {
                    ushort *ptr_out  = img_out[id].ptr<ushort>(row);
                    ptr_out[col]     = ptr_in[Col_offset+col*Size_filter+j];
                    id++;
                }
            }
        }
    }
}
void getSpecterAtHyperSpectralPixelPos(const vector<Mat> *img_in,QVector<double> &specter,int row,int col)
{

    if(img_in->empty())
        qDebug()<<"[Conversion] Empty img";

    specter.clear();

    for(unsigned int i=0;i<img_in->size();i++)
    {
        const ushort *ptr_in = ((*img_in)[i]).ptr<ushort>(row);
        specter.push_back((double)(ptr_in[col]));
    }
}


void CreateHyperCube(const Mat &img_in,vector<Mat> &img_out,int size_filter)
{
    int row_offset                  = 3;
    int col_offset                  = 0;
    int nb_SpectralPixel_per_col    = (int)(img_in.cols/size_filter);
    int nb_SpectralPixel_per_row    = (int)(img_in.rows/size_filter);

    img_out.clear();
    img_out.resize(size_filter*size_filter);
    for(unsigned int i=0;i<img_out.size();i++)
    {
        img_out[i] = Mat::zeros(nb_SpectralPixel_per_row,nb_SpectralPixel_per_col,CV_16UC1);
    }



    for(int row=0 ; row<nb_SpectralPixel_per_row ; row++)
    {
        for(int col=0 ;col<nb_SpectralPixel_per_col ; col++)
        {
            // Hyper spectral filter
            int id=0;
            for (int i = 0 ; i<size_filter;i++)
            {
                const ushort *ptr_in    = img_in.ptr<ushort>(row_offset+row*size_filter+i);

                for(int j=0;j<size_filter;j++)
                {
                    ushort *ptr_out  = img_out[id].ptr<ushort>(row);
                    ptr_out[col]     = ptr_in[col_offset+col*size_filter+j];
                    id++;
                }
            }
        }
    }
}

void convertOriginalPoint_to_HyperspectralPoint(Point &P_in,Point &P_out,int size_filter,Size outSize)
{
    int row_offset                  = 3;
    int col_offset                  = 0;


   P_out.x = (P_in.x - col_offset)>=0 ? (int)((P_in.x - col_offset)/size_filter) : 0;
   P_out.y = (P_in.y - row_offset)>=0 ? (int)((P_in.y - row_offset)/size_filter) : 0;

   P_out.x = (P_out.x > outSize.width) ? outSize.width-1 : P_out.x;
   P_out.y = (P_out.y > outSize.height) ? outSize.height-1 : P_out.y;



}
