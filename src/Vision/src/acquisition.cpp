#include "acquisition.h"




/** Decode Light source type */
int decode_Light_source_type(QString code)
{
    int value = -1;

    if(code == "Halogen")
        value = Light_source_Halogen;
    // else if()

    return value;
}



/** Decode camera name */
int decode_Camera_name(QString code)
{
    int value = -1;
    if (code == "Basler")
        value = Camera_RGB_Basler;

    return value;
}




Mat mergeImg(const Mat &in,Mat &mask,double alpha)
{
    Mat dst = Mat::zeros(in.size(),CV_8UC3);
    resize(mask,mask,in.size());

    for(int row=0;row<in.rows;row++)
    {
        const _Mycolor *ptr_in  = in.ptr<_Mycolor>(row);
        const _Mycolor *ptr_mask= mask.ptr<_Mycolor>(row);
        _Mycolor *ptr_dst       = dst.ptr<_Mycolor>(row);


        for(int col=0;col<in.cols;col++)
        {
            if(ptr_mask[col].r==0 && ptr_mask[col].g==0 && ptr_mask[col].b==0)
            {
                //Do not grey outside contour
                ptr_dst[col].r = ptr_in[col].r;
                ptr_dst[col].g = ptr_in[col].g;
                ptr_dst[col].b = ptr_in[col].b;

            }
            else
            {
                ptr_dst[col].r = (uchar) (alpha*ptr_mask[col].r + (1-alpha)*ptr_in[col].r);
                ptr_dst[col].g = (uchar) (alpha*ptr_mask[col].g + (1-alpha)*ptr_in[col].g);
                ptr_dst[col].b = (uchar) (alpha*ptr_mask[col].b + (1-alpha)*ptr_in[col].b);
            }
        }
    }
    return dst;
}


void load_Matrix(QString path,QVector<QVector<double> > &matrix)
{
    QFile file(path);
    matrix.clear();

    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);
        while(!in.atEnd())
        {
            QVector<double> temp;
            QStringList list = in.readLine().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            for(int c=0;c<list.size();c++)
            {
                temp.push_back(list[c].toDouble());
            }
            matrix.push_back(temp);
        }
        file.close();
    }
    else
    {
        qDebug()<<"[load_Matrix] problem loading file: "<<path;
    }
}
void load_Matrix(QString path,QVector<QVector<int> > &matrix)
{
    QFile file(path);
    matrix.clear();

    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);
        while(!in.atEnd())
        {
            QVector<int> temp;
            QStringList list = in.readLine().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            for(int c=0;c<list.size();c++)
            {
                temp.push_back(list[c].toInt());
            }
            matrix.push_back(temp);
        }
        file.close();
    }
    else
    {
        qDebug()<<"[load_Matrix] problem loading file: "<<path;
    }
}




//PROCESS RGB images
void FirstprocessAddDatas(int j,int x1,int x2, void * d)
{
    _Datas_infos * data                             =(_Datas_infos*)d;
    const _Mycolor* ptr_in                          =data->img->ptr<_Mycolor>(j);
    QVector<QVector<QVector<float> > >  &img_data  =*(data->data);
    int &spatial_sampling                           =*(data->spatial_sampling);

    for(int i=x1 ; i<=x2 ; i+=spatial_sampling)
    {
        QVector<QVector<float> > temp;
        temp.resize(3);

        temp[0].push_back((float)(ptr_in[i].r));
        temp[1].push_back((float)(ptr_in[i].g));
        temp[2].push_back((float)(ptr_in[i].b));
        img_data.push_back(temp);
    }
}


void NormalprocessAddDatas(int j,int x1,int x2, void * d)
{
    _Datas_infos * data=(_Datas_infos*)d;

    const _Mycolor* ptr_in                          =data->img->ptr<_Mycolor>(j);
    QVector<QVector<QVector<float> > > &img_data   =*(data->data);
    int &id                                         =*(data->id);
    int &spatial_sampling                           =*(data->spatial_sampling);


    for(int i=x1 ; i<=x2 ; i+=spatial_sampling)
    {
        img_data[id][0].push_back((float)(ptr_in[i].r));
        img_data[id][1].push_back((float)(ptr_in[i].g));
        img_data[id][2].push_back((float)(ptr_in[i].b));
        id++;
    }
}


/*
//Process Hyper spectral images
void FirstprocessAddDatas_HyperSpectral(int j,int x1,int x2, void * d)
{
    _HyperSpectral_Datas_infos * data               =(_HyperSpectral_Datas_infos*)d;
    QVector<QVector<QVector<float> > >  &img_data  =*(data->data);
    int &spatial_sampling                           =*(data->spatial_sampling);


    QVector<float> specter;
    for(int i=x1 ; i<=x2 ; i+=spatial_sampling)
    {
        getSpecterAtHyperSpectralPixelPos(data->img,specter,j,i);

        QVector<QVector<float> > temp;
        temp.resize(specter.size());

        for(int i=0;i<specter.size();i++)
        {
            const double * ptr_coeff=data->coeff_corr->ptr<double>(i);
            float val_temp=0;
            for(int j=0;j<specter.size();j++)
            {
                val_temp += (float)(specter[i]*ptr_coeff[j]);
            }

            temp[i].push_back(val_temp);
        }

        img_data.push_back(temp);
    }
}

void NormalprocessAddDatas_HyperSpectral(int j,int x1,int x2, void * d)
{
    _HyperSpectral_Datas_infos * data               =(_HyperSpectral_Datas_infos*)d;

//    const _Mycolor* ptr_in                          =data->img->ptr<_Mycolor>(j);
    QVector<QVector<QVector<float> > > &img_data   =*(data->data);
    int &id                                         =*(data->id);
    int &spatial_sampling                           =*(data->spatial_sampling);
    QVector<float> specter;

    for(int i=x1 ; i<=x2 ; i+=spatial_sampling)
    {
        getSpecterAtHyperSpectralPixelPos(data->img,specter,j,i);

        for(int i=0;i<specter.size();i++)
        {
            const double * ptr_coeff=data->coeff_corr->ptr<double>(i);
            float val_temp=0;
            for(int j=0;j<specter.size();j++)
            {
                val_temp += (float)(specter[i]*ptr_coeff[j]);
            }

            img_data[id][i].push_back(val_temp);
        }
        id++;
    }
}
*/

//Process pixel pos

void setPixelPosProcess(int j ,int x1,int x2,void *d)
{
    _Datas_infos * data     =(_Datas_infos*)d;
    QVector<Point> &pos     =*(data->pixels_pos);
    int &spatial_sampling   =*(data->spatial_sampling);

    for(int i=x1 ; i<=x2 ; i+=spatial_sampling)
    {
        pos.push_back(Point(i,j));
    }
}

//process row vector

void RowDataProcess(int j ,int x1,int x2,void *d)
{
    _Raw_Datas * data       =(_Raw_Datas*)d;
    Mat &row_datas          =*(data->raw_data);
    int &spatial_sampling   =*(data->spatial_sampling);
    const _Mycolor* ptr_in  =data->img->ptr<_Mycolor>(j);
    int &id                 =*data->id;

    QVector<double> temp;
    temp.fill(0,3);



    for(int i=x1 ; i<=x2 ; i+=spatial_sampling)
    {
        id++;

        row_datas.at<float>(id, 0) = ((float)(ptr_in[i].b));
        row_datas.at<float>(id, 1) = ((float)(ptr_in[i].g));
        row_datas.at<float>(id, 2) = ((float)(ptr_in[i].r));
    }
}

//Create row vector

void CreateRawDataVector(Mat &datas_row,std::vector<cv::Point> &ROI_contour,Mat &img,int spatial_sampling)
{
    if(ROI_contour.empty())
    {
        qDebug()<<"[Acquisition] Error ROI is not set";
        return;
    }
    int id=-1;

    _Raw_Datas data;
    data.id =&id;
    data.img = &img;
    data.raw_data = &datas_row;
    data.spatial_sampling=&spatial_sampling;
    foreachPixelIn(ROI_contour,img.size(),RowDataProcess,&data);
}

// FUnctions

void getPixelPos(QVector<cv::Point> &pixels_pos,std::vector<cv::Point> &ROI_contour,cv::Size size,int spatial_sampling)
{
    pixels_pos.clear();
    if(ROI_contour.empty())
    {
        qDebug()<<"[Acquisition] Error ROI is not set";
        return;
    }

    _Datas_infos data;
    data.pixels_pos =&pixels_pos;
    data.spatial_sampling= &spatial_sampling;

    foreachPixelIn(ROI_contour,size,setPixelPosProcess,&data);
}

// Add RGB datas
void addData(const Mat &mat,std::vector<cv::Point> &ROI_contour,QVector<QVector<QVector<float> > > &_data,int spatial_sampling)
{
    if(mat.type()!=CV_8UC3)
    {
        qDebug()<<"[Acquisition] Wrong data type;";
        return;
    }

    if(ROI_contour.empty())
    {
        qDebug()<<"[Acquisition] ROI contour is empty";
        return;
    }

    int id=0;
    _Datas_infos data;
    data.img        =&mat;
    data.data       =&_data;
    data.spatial_sampling=&spatial_sampling;
    data.id         =&id;

    if(_data.empty())
    {
        foreachPixelIn(ROI_contour,mat.size(),FirstprocessAddDatas,&data);
    }
    else
    {
        foreachPixelIn(ROI_contour,mat.size(),NormalprocessAddDatas,&data);
    }

}





//New add datas
void processAddDatas(int j,int x1,int x2, void * d)
{
    _Datas_infos * data=(_Datas_infos*)d;

    const _Mycolor* ptr_in                          =data->img->ptr<_Mycolor>(j);
    QVector<QVector<QVector<float> > > &img_data    =*(data->data);
    int &id                                         =*(data->id);
    int &spatial_sampling                           =*(data->spatial_sampling);
    int &img_id                                     =*(data->img_id);


    for(int i=x1 ; i<=x2 ; i+=spatial_sampling)
    {
        img_data[id][0][img_id]=((float)(ptr_in[i].r));
        img_data[id][1][img_id]=((float)(ptr_in[i].g));
        img_data[id][2][img_id]=((float)(ptr_in[i].b));
        id++;
    }
}

void addData(const Mat &mat,std::vector<cv::Point> &ROI_contour,QVector<QVector<QVector<float> > > &_data,int spatial_sampling,int img_id)
{
    if(mat.type()!=CV_8UC3)
    {
        qDebug()<<"[Acquisition] Wrong data type;";
        return;
    }

    if(ROI_contour.empty())
    {
        qDebug()<<"[Acquisition] ROI contour is empty";
        return;
    }

    int id=0;
    _Datas_infos data;
    data.img        =&mat;
    data.data       =&_data;
    data.spatial_sampling=&spatial_sampling;
    data.id         =&id;
    data.img_id     =&img_id;


    foreachPixelIn(ROI_contour,mat.size(),processAddDatas,&data);


}

/*
// Add Hyper spectral datas
void addData_HyperSpectral(const vector<Mat> &mat, const Mat &corrCoeff, std::vector<cv::Point> &ROI_contour, QVector<QVector<QVector<float> > > &_data, int spatial_sampling, cv::Size img_size)
{
    if(mat.empty())
    {
        qDebug()<<"[Acquisition] Datas empty;";
        return;
    }

    if(ROI_contour.empty())
    {
        qDebug()<<"[Acquisition] ROI contour is empty";
        return;
    }

    int id=0;
    _HyperSpectral_Datas_infos data;
    data.img        =&mat;
    data.data       =&_data;
    data.spatial_sampling=&spatial_sampling;
    data.id         =&id;
    data.coeff_corr =&corrCoeff;


    if(_data.empty())
    {
        foreachPixelIn(ROI_contour,img_size,FirstprocessAddDatas_HyperSpectral,&data);
    }
    else
    {
        foreachPixelIn(ROI_contour,img_size,NormalprocessAddDatas_HyperSpectral,&data);
    }
}
*/

//Normalized cross correlation

void NCCprocess(int j,int x1,int x2, void * d)
{
    _Datas_NCC * data=(_Datas_NCC*)d;

    const uchar* ptr_ref    =data->imgref->ptr<uchar>(j);
    const uchar* ptr_cp            =data->img2->ptr<uchar>(j);
    float &sum_ref          =*(data->sum_ref);
    float &sum_squared_ref  =*(data->sum_squared_ref);
    float &sum_cp           =*(data->sum_cp);
    float &sum_squared_cp   =*(data->sum_squared_cp);
    float &sum_prod         =*(data->sum_prod);

    for(int i=x1 ; i<=x2 ; i++)
    {
        sum_ref+=ptr_ref[i];
        sum_squared_ref+=(ptr_ref[i]*ptr_ref[i]);

        sum_cp+=ptr_cp[i];
        sum_squared_cp+= (ptr_cp[i]*ptr_cp[i]);

        sum_prod+=ptr_ref[i]*ptr_cp[i];
    }
}
float NormalizeCrossCorrelation(const Mat &matref,const Mat &mat2,std::vector<cv::Point> &ROI_contour)
{
    if(ROI_contour.empty())
    {
        qDebug()<<"[Acquisition] NormalizeCrossCorrelation ROI contour is empty";
        return -1;
    }
    if(matref.size()!=mat2.size())
        qDebug()<<"[Acquisition] NormalizeCrossCorrelation different size between the 2 img";



    float IN=0;
    float sum_ref          = 0;
    float sum_squared_ref  = 0;
    float sum_cp           = 0;
    float sum_squared_cp   = 0;
    float sum_prod         = 0;
    int size        = matref.rows*matref.cols;

    _Datas_NCC data;
    data.imgref     =&matref;
    data.img2       =&mat2;
    data.sum_cp     =&sum_cp;
    data.sum_prod   =&sum_prod;
    data.sum_ref    =&sum_ref;
    data.sum_squared_cp = &sum_squared_cp;
    data.sum_squared_ref = &sum_squared_ref;

    foreachPixelIn(ROI_contour,matref.size(),NCCprocess,&data);

    IN = (size * sum_prod - sum_ref*sum_cp)/(sqrt( (size*sum_squared_ref - pow(sum_ref,2)) * (size*sum_squared_cp - pow(sum_cp,2))));

    return IN;
}

/*
void NCCprocess_HS(int j,int x1,int x2, void * d)
{
    _Datas_NCC * data=(_Datas_NCC*)d;

    const ushort* ptr_ref    =data->imgref->ptr<ushort>(j);
    const ushort* ptr_cp     =data->img2->ptr<ushort>(j);
    float &sum_ref          =*(data->sum_ref);
    float &sum_squared_ref  =*(data->sum_squared_ref);
    float &sum_cp           =*(data->sum_cp);
    float &sum_squared_cp   =*(data->sum_squared_cp);
    float &sum_prod         =*(data->sum_prod);

    for(int i=x1 ; i<=x2 ; i++)
    {
        sum_ref+=ptr_ref[i];
        sum_squared_ref+=(ptr_ref[i]*ptr_ref[i]);

        sum_cp+=ptr_cp[i];
        sum_squared_cp+= (ptr_cp[i]*ptr_cp[i]);

        sum_prod+=ptr_ref[i]*ptr_cp[i];
    }
}

float NormalizeCrossCorrelation_HS(const Mat &matref,const Mat &mat2,std::vector<cv::Point> &ROI_contour)
{
    if(ROI_contour.empty())
    {
        qDebug()<<"[Acquisition] ROI contour is empty";
        return -1;
    }

    if(matref.type()!=CV_16UC1 && mat2.type()!=CV_16UC1)
    {
        qDebug()<<"[Acquisition] Wrong img formats";
        return -1;
    }

    vector<Mat> HyperCubeRef,HyperCubeMes;
    CreateHyperCube(matref,HyperCubeRef,5);
    CreateHyperCube(mat2,HyperCubeMes,5);

    float IN               = 0;

    for(unsigned int i=0;i<HyperCubeMes.size();i++)
    {
        float sum_ref          = 0;
        float sum_squared_ref  = 0;
        float sum_cp           = 0;
        float sum_squared_cp   = 0;
        float sum_prod         = 0;
        int size               = HyperCubeMes[i].rows*HyperCubeMes[i].cols;

        _Datas_NCC data;
        data.imgref     =&HyperCubeRef[i];
        data.img2       =&HyperCubeMes[i];
        data.sum_cp     =&sum_cp;
        data.sum_prod   =&sum_prod;
        data.sum_ref    =&sum_ref;
        data.sum_squared_cp = &sum_squared_cp;
        data.sum_squared_ref = &sum_squared_ref;

        foreachPixelIn(ROI_contour,matref.size(),NCCprocess_HS,&data);

        IN += (size * sum_prod - sum_ref*sum_cp)/(sqrt( (size*sum_squared_ref - pow(sum_ref,2)) * (size*sum_squared_cp - pow(sum_cp,2))));

    }

    return IN/(HyperCubeMes.size());
}
*/

void reConstructImg(const Mat &labels, Mat &img_out, const QVector<Point> &pixels_pos, cv::Size size, int Sampling)
{
    img_out=Mat::zeros(size,CV_8UC3);
//    img_out=Mat::zeros(size,CV_8UC1);
    QVector<_Mycolor> color;
    _Mycolor c;
    //Blanc
    c.b = 255;
    c.g = 255;
    c.r = 255;
    color.push_back(c);
    //Cyan
    c.b = 255;
    c.g = 255;
    c.r = 0;
    color.push_back(c);
    //Vert
    c.b = 0;
    c.g = 255;
    c.r = 0;
    color.push_back(c);
    //Magenta
    c.b = 255;
    c.g = 0;
    c.r = 255;
    color.push_back(c);
    //Jaune
    c.b = 0;
    c.g = 255;
    c.r = 255;
    color.push_back(c);
    //Violet
    c.b = 132;
    c.g = 6;
    c.r = 161;
    color.push_back(c);
    //Orange
    c.b = 0;
    c.g = 89;
    c.r = 255;
    color.push_back(c);
    //Black
    c.b = 0;
    c.g = 0;
    c.r = 0;
    color.push_back(c);

    if(Sampling>1)
    {
        int y_pos       =-1;
        _Mycolor last_ref;
        for(int i=0;i<pixels_pos.size();i++)
        {
            // Get img value
            int cluster_idx = labels.at<int>(i,0);

            if(y_pos!=pixels_pos[i].y)
            {
                //changement de ligne
                y_pos   =pixels_pos[i].y;
                last_ref = color[cluster_idx];
                img_out.at<_Mycolor>(pixels_pos[i].y,pixels_pos[i].x)=c;
            }
            else
            {
                _Mycolor *ptr=img_out.ptr<_Mycolor>(y_pos);
                for(int col=pixels_pos[i].x-Sampling+1;col<pixels_pos[i].x;col++)
                {
                    ptr[col]    = color[cluster_idx];
                    last_ref    = color[cluster_idx];
                }
                img_out.at<_Mycolor>(pixels_pos[i].y,pixels_pos[i].x)= c;
            }

        }
    }
    else
    {
        for(int i=0;i<pixels_pos.size();i++)
        {
            int cluster_idx = labels.at<int>(i,0);
//            img_out.at<uchar>(pixels_pos[i].y,pixels_pos[i].x)        = cluster_idx+1;
            img_out.at<_Mycolor>(pixels_pos[i].y,pixels_pos[i].x)        = color[cluster_idx];
        }
    }

}

void reConstructImg(const QVector<int> &labels, Mat &img_out, const QVector<Point> &pixels_pos, cv::Size size, int Sampling)
{
    img_out=Mat::zeros(size,CV_8UC3);
//    img_out=Mat::zeros(size,CV_8UC1);
    QVector<_Mycolor> color;
    _Mycolor c;
    //Blanc
    c.b = 255;
    c.g = 255;
    c.r = 255;
    //Cyan
    color.push_back(c);
    c.b = 255;
    c.g = 255;
    c.r = 0;
    //Vert
    color.push_back(c);
    c.b = 0;
    c.g = 255;
    c.r = 0;
    //Magenta
    color.push_back(c);
    c.b = 255;
    c.g = 0;
    c.r = 255;
    //Jaune
    color.push_back(c);
    c.b = 0;
    c.g = 255;
    c.r = 255;
    //Violet
    color.push_back(c);
    c.b = 132;
    c.g = 6;
    c.r = 161;
    color.push_back(c);

    if(Sampling>1)
    {
        int y_pos       =-1;
        _Mycolor last_ref;
        for(int i=0;i<pixels_pos.size();i++)
        {
            // Get img value
            int cluster_idx = labels[i];

            if(y_pos!=pixels_pos[i].y)
            {
                //changement de ligne
                y_pos   =pixels_pos[i].y;
                last_ref = color[cluster_idx];
                img_out.at<_Mycolor>(pixels_pos[i].y,pixels_pos[i].x)=c;
            }
            else
            {
                _Mycolor *ptr=img_out.ptr<_Mycolor>(y_pos);
                for(int col=pixels_pos[i].x-Sampling+1;col<pixels_pos[i].x;col++)
                {
                    ptr[col]    = color[cluster_idx];
                    last_ref    = color[cluster_idx];
                }
                img_out.at<_Mycolor>(pixels_pos[i].y,pixels_pos[i].x)= c;
            }

        }
    }
    else
    {
        for(int i=0;i<pixels_pos.size();i++)
        {
            int cluster_idx = labels[i];
//            img_out.at<uchar>(pixels_pos[i].y,pixels_pos[i].x)        = cluster_idx+1;
            img_out.at<_Mycolor>(pixels_pos[i].y,pixels_pos[i].x)        = color[cluster_idx];
        }
    }

}


void WriteTemporalVector(const QString path,const QVector<QVector<float> > &tempoal_vec)
{
    QFile file( path);
    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream stream( &file );

        for(int i=0;i<tempoal_vec.size();i++)
        {
            for(int j=0;j<tempoal_vec[i].size();j++)
            {
                stream <<tempoal_vec[i][j]<<" ";
            }
            stream<<"\n";
        }
    }
    file.close();
}

void WriteBGRVector(const QString path,const QVector<Scalar> &bgr)
{
    QFile file( path);
    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream stream( &file );

        for(int i=0;i<bgr.size();i++)
        {

            stream <<bgr[i][0]<<" "<<bgr[i][1]<<" "<<bgr[i][2];
            stream<<"\n";
        }
    }
    file.close();
}

void WritePointVector(const QString path,const QVector<Point> &P)
{
    QFile file( path);
    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream stream( &file );

        for(int i=0;i<P.size();i++)
        {

            stream <<P[i].y<<" "<<P[i].x;
            stream<<"\n";
        }
    }
    file.close();
}

void WritePointVector(const QString path,const QVector<QPoint> &P)
{
    QFile file( path);
    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream stream( &file );

        for(int i=0;i<P.size();i++)
        {

            stream <<P[i].y()<<" "<<P[i].x();
            stream<<"\n";
        }
    }
    file.close();
}




void WriteTemporalVector(const QString path,const QVector<QVector<double> > &tempoal_vec)
{
    QFile file( path);
    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream stream( &file );

        for(int i=0;i<tempoal_vec[0].size();i++)
        {
            for(int k=0;k<tempoal_vec.size();k++)
            {
                stream <<tempoal_vec[k][i]<<" ";
            }
            stream<<"\n";
        }
    }
    file.close();
}

void WriteTemporalVector(const QString path,fftwf_complex  *tempoal_vec, int N)
{
    QFile file( path);

    //Remove file if exists
    if(file.exists())
        file.remove();

    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream stream( &file );
        for(int k=0;k<N;k++)
        {
            stream <<tempoal_vec[k][0]<<" "<<tempoal_vec[k][1]<<"\n";
        }
    }
    file.close();
}

void WriteTemporalVector(const QString path,const QVector<float> &tempoal_vec)
{
    QFile file( path);

    //Remove file if exists
    if(file.exists())
        file.remove();

    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream stream( &file );
        for(int k=0;k<tempoal_vec.size();k++)
        {
            stream <<tempoal_vec[k]<<"\n";
        }
    }
    file.close();
}

void WriteTemporalVector(const QString path,const QVector<double> &tempoal_vec)
{
    QFile file( path);

    //Remove file if exists
    if(file.exists())
        file.remove();

    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream stream( &file );
        for(int k=0;k<tempoal_vec.size();k++)
        {
            stream <<tempoal_vec[k]<<"\n";
        }
    }
    file.close();
}
void WriteTemporalVector(const QString path,const QVector<int> &tempoal_vec)
{
    QFile file( path);

    //Remove file if exists
    if(file.exists())
        file.remove();

    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream stream( &file );
        for(int k=0;k<tempoal_vec.size();k++)
        {
            stream <<tempoal_vec[k]<<"\n";
        }
    }
    file.close();
}

/*
void WriteItppmat(const QString path,const itpp::mat img)
{
    QFile file( path);

    //Remove file if exists
    if(file.exists())
        file.remove();

    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream stream( &file );

        for(int row=0;row<img.rows();row++)
        {
            for(int col=0;col<img.cols();col++)
            {
                stream <<img(row,col)<<" ";
            }
            stream<<"\n";;
        }
    }
}
*/

void WriteFloatImg_bin(const QString path, const Mat img)
{
    QFile file(path);
    if (file.exists()) file.remove();

    if (file.open(QIODevice::WriteOnly))
    {
        // Write dimensions first so you can reload it
        int rows = img.rows, cols = img.cols;
        file.write(reinterpret_cast<const char*>(&rows), sizeof(int));
        file.write(reinterpret_cast<const char*>(&cols), sizeof(int));

        // Write all rows at once
        for (int row = 0; row < img.rows; row++)
        {
            const float* ptr = img.ptr<float>(row);
            file.write(reinterpret_cast<const char*>(ptr), cols * sizeof(float));
        }
    }
}


void WriteFloatImg(const QString path,const Mat img)
{
    QFile file( path);

    //Remove file if exists
    if(file.exists())
        file.remove();

    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream stream( &file );

        for(int row=0;row<img.rows;row++)
        {
            const float *ptr=img.ptr<float>(row);
            for(int col=0;col<img.cols;col++)
            {
                stream <<ptr[col]<<" ";
            }
            stream<<"\n";
        }
    }
}

void WriteDoubleImg(const QString path,const Mat img)
{
    QFile file( path);
    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream stream( &file );

        for(int row=0;row<img.rows;row++)
        {
            const double *ptr=img.ptr<double>(row);
            for(int col=0;col<img.cols;col++)
            {
                stream <<ptr[col]<<" ";
            }
            stream<<"\n";
        }
    }
}


void ReadFloatImg(const QString path,Mat &img)
{
    QFile file(path);
    if (file.open(QIODevice::ReadOnly))
    {
        int row=0;

        QTextStream textStream(&file);
        while (!textStream.atEnd())
        {
            QStringList list = textStream.readLine().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

            float * ptr=img.ptr<float>(row);
            for(int col=0;col<list.size();col++)
                ptr[col]=list[col].toFloat();
            row++;
        }
    }
    file.close();
}

void CreateContrastImg(Mat &img_contrast,int spatialSampling,QVector<Point> &Pixel_pos,QVector<double> &contrast)
{
        if(spatialSampling>1)
        {

            int y_pos=-1;
            for(int i=0;i<Pixel_pos.size();i++)
            {
                if(y_pos!=Pixel_pos[i].y)
                {
                    //changement de ligne
                    y_pos=Pixel_pos[i].y;
                    float *ptr=img_contrast.ptr<float>(y_pos);
                    for(int col=Pixel_pos[i].x;col<Pixel_pos[i].x+spatialSampling;col++)
                        ptr[col] = (float)(contrast[i]);
                }
                else
                {
                    float *ptr=img_contrast.ptr<float>(y_pos);
                    for(int col=Pixel_pos[i].x-spatialSampling+1;col<Pixel_pos[i].x;col++)
                        ptr[col] = (float)(contrast[i]);

                    img_contrast.at<float>(Pixel_pos[i].y,Pixel_pos[i].x)        = (float)(contrast[i]);
                }

            }
        }
        else
        {
            for(int i=0;i<Pixel_pos.size();i++)
                img_contrast.at<float>(Pixel_pos[i].y,Pixel_pos[i].x)        = (float)(contrast[i]);
        }
}





void ReadVector(const QString path,QVector<double> &out)
{
    QFile file(path);
    out.clear();

    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);
        while(!in.atEnd())
        {
            QString line = in.readLine();
            out.push_back( line.toDouble());
        }
        file.close();
    }
}

void ReadVector(const QString path,QVector<float> &out)
{
    QFile file(path);
    out.clear();

    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);
        while(!in.atEnd())
        {
            QString line = in.readLine();
            out.push_back( line.toFloat());
        }
        file.close();
    }
}

void ReadVector(const QString path,QVector<QVector <float> > &out)
{
    QFile file(path);
    out.clear();

    if(file.open(QIODevice::ReadOnly))
    {

        QTextStream textStream(&file);
        while (!textStream.atEnd())
        {
            QStringList list = textStream.readLine().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            QVector<float> temp;
            for(int col=0;col<list.size();col++)
                temp.push_back(list[col].toFloat());
            out.push_back(temp);
        }
        file.close();
    }
}

void ReadDoubleVector(const QString path,QVector<QVector <double> > &out)
{
    QFile file(path);
    out.clear();

    if(file.open(QIODevice::ReadOnly))
    {

        QTextStream textStream(&file);
        while (!textStream.atEnd())
        {
            QStringList list = textStream.readLine().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            QVector<double> temp;
            for(int col=0;col<list.size();col++)
                temp.push_back(list[col].toDouble());
            out.push_back(temp);
        }
        file.close();
    }
}















void computeConvolvedSignals(const QVector<float>& bold,
                             const QVector<float>& yParadigm,
                             float                  Fs,
                             QVector<float>&        resConvolve)
{
    // ── Helpers ──────────────────────────────────────────────────────────

    auto arange = [](float start, float stop, float step) -> QVector<float>
    {
        QVector<float> out;
        for (float x = start; x < stop; x += step)
            out.push_back(x);
        return out;
    };

    auto nextPow2 = [](int n) -> int
    {
        int p = 1;
        while (p < n) p <<= 1;
        return p;
    };

    // Full linear convolution via FFTW (equivalent to np.convolve(a, b))
    auto fftConvolveFull = [&](const QVector<float>& a,
                               const QVector<float>& b) -> QVector<float>
    {
        int outSize = a.size() + b.size() - 1;
        int N       = nextPow2(outSize);

        double*       in1     = (double*)      fftw_malloc(sizeof(double)       * N);
        double*       in2     = (double*)      fftw_malloc(sizeof(double)       * N);
        fftw_complex* fft1    = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (N/2+1));
        fftw_complex* fft2    = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (N/2+1));
        fftw_complex* fftProd = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (N/2+1));
        double*       result  = (double*)      fftw_malloc(sizeof(double)       * N);

        for (int i = 0; i < N; ++i) { in1[i] = 0.0; in2[i] = 0.0; }
        for (int i = 0; i < a.size(); ++i) in1[i] = a[i];
        for (int i = 0; i < b.size(); ++i) in2[i] = b[i];

        fftw_plan p1   = fftw_plan_dft_r2c_1d(N, in1, fft1,    FFTW_ESTIMATE);
        fftw_plan p2   = fftw_plan_dft_r2c_1d(N, in2, fft2,    FFTW_ESTIMATE);
        fftw_plan pinv = fftw_plan_dft_c2r_1d(N, fftProd, result, FFTW_ESTIMATE);

        fftw_execute(p1);
        fftw_execute(p2);

        // Pointwise complex multiply
        for (int i = 0; i < N/2+1; ++i)
        {
            double re = fft1[i][0]*fft2[i][0] - fft1[i][1]*fft2[i][1];
            double im = fft1[i][0]*fft2[i][1] + fft1[i][1]*fft2[i][0];
            fftProd[i][0] = re;
            fftProd[i][1] = im;
        }

        fftw_execute(pinv);

        // FFTW inverse is unnormalized → divide by N
        QVector<float> out(outSize);
        for (int i = 0; i < outSize; ++i)
            out[i] = static_cast<float>(result[i] / N);

        fftw_destroy_plan(p1);
        fftw_destroy_plan(p2);
        fftw_destroy_plan(pinv);
        fftw_free(in1); fftw_free(in2);
        fftw_free(fft1); fftw_free(fft2);
        fftw_free(fftProd); fftw_free(result);

        return out;
    };

    // ── t_bold = np.arange(0, 20, 20/np.size(Bold)) ──────────────────────
    // t_bold_new = np.arange(0, 20, 1/Fs)
    QVector<float> tBoldNew;
    tBoldNew = arange(0.0f, 20.0f, 1.0f / Fs);

    // ── Boost cubic B-spline interpolation ───────────────────────────────
    // Equivalent to: tck = splrep(t_bold, Bold, s=0)
    //                y_bold_interp = splev(t_bold_new, tck, der=0)
    // t_bold is uniformly spaced with step = 20/size(Bold) → use cardinal spline
    std::vector<double> boldStd(bold.begin(), bold.end());
    const double t0 = 0.0;
    const double h  = 20.0 / static_cast<double>(bold.size()); // step of t_bold

    boost::math::interpolators::cardinal_cubic_b_spline<double> spline(
        boldStd.data(), boldStd.size(), t0, h
        );

    QVector<float> yBoldInterp;
    yBoldInterp.clear();
    yBoldInterp.reserve(tBoldNew.size());
    for (float t : tBoldNew)
        yBoldInterp.push_back(static_cast<float>(spline(t)));

    // ── res_convolve = np.convolve(y_bold_interp, y_paradigm) ────────────
    resConvolve = fftConvolveFull(yBoldInterp, yParadigm);


}