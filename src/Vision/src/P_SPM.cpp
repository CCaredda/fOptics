#include "P_SPM.h"

P_SPM::P_SPM(QObject *parent) : QObject(parent)
{
    //Check if GPU is accessible
    _M_GPU_enabled = (cuda::getCudaEnabledDeviceCount()>0) ? true : false;
    qDebug()<<cuda::getCudaEnabledDeviceCount()<<" CUDA GPU is available";

    //T Map
    _M_T_map = Mat(0,0,CV_32FC1);

    //Nb of resels
    _M_resels=1;

    //Ful-width Half Maximum (FWHM)
    _M_FWHM = 7.0;

    //Z thresh
    _M_z_thresh = 5.0f;

    //HRF
    _M_HRF.clear();

    //z threshold  for 1 to 1000 nb of resels
    _M_z_thresh_possible_values.clear();
    QString share_dir = getShareDirPath("share")+"/";
    // QString share_dir = QString(QCoreApplication::applicationDirPath())+"/../share/;

    ReadVector(share_dir+"files/SPM/z_threshold_5percent.txt",_M_z_thresh_possible_values);
}

/** Write SPM infos in temp directory */
void P_SPM::Write_SPM_info(QString saving_dir)
{
    QFile f(saving_dir+"info_SPM.txt");
    if (f.open(QIODevice::ReadWrite))
    {
        QTextStream Qt( &f );
        Qt<<"z_thresh: "<<_M_z_thresh<<"\n";
        Qt<<"FWHM: "<<_M_FWHM<<"\n";
        Qt<<"resels: "<<_M_resels<<"\n";
    }
    f.close();
}

//Set statistical significance
void P_SPM::set_Statistical_Significance_Level(int v)
{
    _M_z_thresh_possible_values.clear();
    QString share_dir = getShareDirPath("share")+"/";
    // QString share_dir = QString(QCoreApplication::applicationDirPath())+"/../share/;

    ReadVector(share_dir+"files/SPM/z_threshold_"+QString::number(v)+"percent.txt",_M_z_thresh_possible_values);

    //Find z threshold
    if(_M_resels>_M_z_thresh_possible_values.size())
        _M_z_thresh = 10.0f;
    else
        _M_z_thresh = _M_z_thresh_possible_values[_M_resels-1];

    emit newZThresh(_M_z_thresh);

    qDebug()<<"[P_SPM] Z thresh : "<<_M_z_thresh<<" for "<<_M_resels<<" resels";
}


//set Nb of resels
void P_SPM::setNbResels(int nb)
{
    _M_resels=nb;

    if(_M_resels>_M_z_thresh_possible_values.size())
        _M_z_thresh = 10.0f;
    else
        _M_z_thresh = _M_z_thresh_possible_values[_M_resels-1];

    emit newZThresh(_M_z_thresh);

    qDebug()<<"[P_SPM] Z thresh : "<<_M_z_thresh<<" for "<<_M_resels<<" resels";
}

Mat P_SPM::gaussianBlur(const Mat &in)
{
    //Gaussian Blur on Z maps
    double sigma    = _M_FWHM/2.355;
    int ksize       = int(3*_M_FWHM);
    ksize           = (ksize%2==0) ? ksize+1 : ksize;

    Mat out;
    GaussianBlur( in, out, Size(ksize,ksize), 0, sigma );
    return out;
}

//Get thresholded Z maps according to the Random Field theory
Mat P_SPM::getThresholdedZMap(const Mat &Z_map)
{
    Mat out = Mat::zeros(Z_map.size(),CV_8UC1);

    for(int row=0;row<Z_map.rows;row++)
    {
        const float *ptr_zmap = Z_map.ptr<float>(row);
        uchar *ptr_out  = out.ptr<uchar>(row);

        for(int col=0;col<Z_map.cols;col++)
            ptr_out[col] = (ptr_zmap[col]>_M_z_thresh) ? 255 : 0;
    }

    return out;
}




//Process Fisher transformation (correlation r to z stats)
Mat P_SPM::convert_Correlation_Map_to_Z_Map(const Mat &in)
{
    Mat Zmap = Mat::zeros(in.size(),in.type());
    for(int row=0;row<in.rows;row++)
    {
        float *ptr_z = Zmap.ptr<float>(row);
        const float *ptr_in = in.ptr<float>(row);
        for(int col=0;col<in.cols;col++)
        {
            ptr_z[col]=fisher_r_to_z_transform(ptr_in[col]);
        }
    }

    return Zmap;
}


//Process Z map from T maps
Mat P_SPM::get_Z_map()
{
    Mat Z_Map = Mat::zeros(_M_T_map.size(),_M_T_map.type());

    for(int row=0;row<_M_T_map.rows;row++)
    {
        float *ptrT = _M_T_map.ptr<float>(row);
        float *ptrZ = Z_Map.ptr<float>(row);
        for(int col=0;col<_M_T_map.cols;col++)
            ptrZ[col] = convert_T_to_Z_stats(ptrT[col]);;
    }
    return Z_Map;
}

// General linear Model
// Notations:
// T: time
// N: nb spatial pixels
// S: nb of stimulis: 1
//

// Y = X.B + e

// Y: data (Hb, HbO2 concentration changes) Size: (T,N)
// X: Design matrix (Bold signal with physiological a priori on amplitude) Size: (T,S).
// S: number of stimulus or condition, here S=2 because two condition or explored (activation,non activation)
// x = HR(t=0)  0
//      .       .
//      .       .
//      .       .
//     HR(t=N)  0
//
// with HR, the hemodynamic response
//
// B: spatial pattern of responses. Size(S=2,N)
// e: error term. Size: (T,N)
// R: residuals = I-X(X'X)-1X'
// res_stats: binary indicator of the activation (1: activated, 0: non activated) size(1,N)
void P_SPM::GeneralLinearModel(const Mat &Y,const Mat &X)
{
    _CPU_GeneralLinearModel(Y,X);
//    if(_M_GPU_enabled)
//        _GPU_GeneralLinearModel(Y,X);
//    else
//        _CPU_GeneralLinearModel(Y,X);
}



void P_SPM::_CPU_GeneralLinearModel(const Mat &Y,const Mat &X)
{
    //resolve linerar model
    // B = (X'X)-1 X'Y
    Mat tr_X;
    Mat invert_X;
    transpose(X,tr_X);

    mulTransposed(X,invert_X,true);
    invert(invert_X,invert_X,DECOMP_SVD);

//    qDebug()<<"Y size("<<Y.rows<<";"<<Y.cols<<")";
//    qDebug()<<"X_tr size("<<tr_X.rows<<";"<<tr_X.cols<<")";
//    qDebug()<<"(X_tr.X)-1 size("<<invert_X.rows<<";"<<invert_X.cols<<")";



    //Spatial pattern of responses
    Mat B = (invert_X*tr_X)*Y;

    //errors
    Mat e = Y - X*B;

    Mat var_e;
    extractVarianceFromMat(e,var_e);

    //Residuals forming matrix
    Mat R = X*invert_X*tr_X;
    R = Mat::eye(R.size(),R.type()) - R;


//    //Get effective degrees of freedom
//    QVector<double> df = _effective_degree_of_freedom_GLM(R,e);
//    //TEMP
//    double df =  Y.cols-1;

    //Get T maps
    _GLM_t_stats(invert_X,B,var_e,_M_T_map,0); //Stimulus id: 0

    //Get the indicator of activation (1: activated, 0: non activated)
//    _get_Activation_indicator_Bonferroni(T_map,res_stats,0.05/T_map.cols,df);

//    double alpha = 0.05/T_map.cols;
//    get_Activation_indicator_Bonferroni2(T_map,res_stats,alpha,df);
}

void P_SPM::_GPU_GeneralLinearModel(const Mat &Y,const Mat &X)
{
    //resolve linerar model
    // B = (X'X)-1 X'Y
    Mat tr_X;
    Mat invert_X;
    transpose(X,tr_X);

    mulTransposed(X,invert_X,true);
    invert(invert_X,invert_X,DECOMP_SVD);

//    qDebug()<<"Y size("<<Y.rows<<";"<<Y.cols<<")";
//    qDebug()<<"X_tr size("<<tr_X.rows<<";"<<tr_X.cols<<")";
//    qDebug()<<"(X_tr.X)-1 size("<<invert_X.rows<<";"<<invert_X.cols<<")";



    //Spatial pattern of responses
    Mat B = (invert_X*tr_X)*Y;

    //errors
    Mat e = Y - X*B;

    Mat var_e;
    extractVarianceFromMat(e,var_e);

    //Residuals forming matrix
    Mat R = X*invert_X*tr_X;
    R = Mat::eye(R.size(),R.type()) - R;


//    //Get effective degrees of freedom
//    QVector<double> df = _effective_degree_of_freedom_GLM(R,e);
//    //TEMP
//    double df =  Y.cols-1;

    //Get T maps
    _GLM_t_stats(invert_X,B,var_e,_M_T_map,0); //Stimulus id: 0

    //Get the indicator of activation (1: activated, 0: non activated)
//    _get_Activation_indicator_Bonferroni(T_map,res_stats,0.05/T_map.cols,df);

//    double alpha = 0.05/T_map.cols;
//    get_Activation_indicator_Bonferroni2(T_map,res_stats,alpha,df);
}

//Get design matrix
//HRF: Hemodynamic response function size(T)
//X Design matrix (Bold signal with physiological a priori on amplitude) Size: (T,S).
//Concentration changes apriori
// x = HR(t=0)  0
//      .       .
//      .       .
//      .       .
//     HR(t=N)  0

Mat P_SPM::Build_Design_Matrix(const QVector<float> &Signal, int nb_frames)
{
    if (nb_frames>Signal.size())
        nb_frames = Signal.size();

    Mat X = Mat::zeros(nb_frames,2,CV_32F);

    for(int row=0;row<X.rows;row++)
        X.at<float>(row,0) = Signal[row];

    return X;
}

QVector<float> P_SPM::get_Mean_Expected_DeltaC(const QVector<float> &HRF,float apriori,QVector<int> &activity_start,QVector<int> &activity_end)
{
    float max = float(get_Max(HRF));
    QVector<float> mean_val;

    for(int id = 0;id<activity_start.size();id++)
    {
        float mean = 0.0f;
        int tot=0;
        for(int t=activity_start[id];t<=activity_end[id];t++)
        {
            tot++;
            mean += apriori*(HRF[t]/max);
        }
        mean_val.push_back(0.25*(mean/tot));
        qDebug()<<0.25*(mean/tot);
    }
    return mean_val;
}

float P_SPM::get_Mean_Expected_DeltaC(const QVector<float> &HRF,float apriori,int activity_start,int activity_end)
{
    float max = float(get_Max(HRF));

    float mean = 0.0f;
    int tot=0;
    for(int t=activity_start;t<=activity_end;t++)
    {
        tot++;
        mean += apriori*(HRF[t]/max);
    }
    qDebug()<<0.25*(mean/tot);
    return 0.25*(mean/tot);
}

//effective degree of freedom (Satterthwaithe approximation)
//e: errors size(T,N)
//R: Residuals in GLM computation size(T,T)
QVector<double> P_SPM::_effective_degree_of_freedom_GLM(const Mat& R,const Mat &e)
{
    QVector<double> df;

    for(int spatial_id=0;spatial_id<e.cols;spatial_id++)
    {
        //Get autocorrelation matrix
        Mat V = autocorrelation(e,spatial_id);

        qDebug()<<"df step 1";
        Mat temp = R*V;
        qDebug()<<"df step 2";
        double a = getTrace_of_product(R,V);
        qDebug()<<"df step 3";
        double b = getTrace_of_product(temp,temp);
        qDebug()<<"df step 4";
        df.push_back(pow(a,2)/(b));
    }

    return df;
}


//GLM T statistics
//invert_X = (X'X)-1 size (S=2,S=2)
//B size(S=2,N)
//var_e size (1,N)
//T_map size(1,N)
// stimulus_id : 0 (activation) 1 (non activation)
void P_SPM::_GLM_t_stats(const Mat &invert_X,const Mat &B,const Mat &var_e,Mat &T_map,int stimulus_id)
{
    //Build contrast vector
    Mat c   = Mat::zeros(2,1,CV_32F);
    Mat c_t = Mat::zeros(1,2,CV_32F);

    c.at<float>(stimulus_id,0)      = 1;
    c_t.at<float>(0,stimulus_id)    = 1;

    //size (1,N)
    T_map = c_t*B;

    // Size (1,1)
    Mat denom = c_t*invert_X*c;

    float *ptr_Tmap     = T_map.ptr<float>(0);
    const float *ptr_var_e    = var_e.ptr<float>(0);

    for(int col=0;col<T_map.cols;col++)
    {
        float val = sqrt(denom.at<float>(0,0)*ptr_var_e[col]);
        ptr_Tmap[col] = (val==0.0f)? 0.0f : ptr_Tmap[col]/val;
    }
}

//Activation indicator
void P_SPM::_get_Activation_indicator_Bonferroni(const Mat& T_map,Mat &activation_indicator,double alpha,QVector<double> &degree_of_freedom)
{
    activation_indicator = Mat::zeros(T_map.size(),CV_8UC1);

    for(int n=0;n<T_map.cols;n++)
    {
        students_t dist(degree_of_freedom[n]);
        double pvalue = cdf(complement(dist, fabs(T_map.at<float>(0,n))))*2;
        //    The Null-hypothesis: there is no difference in means
        //   Reject if complement of CDF for |t| < significance level:
        //    double pvalue = cdf(complement(dist, fabs(t)))*2 < alpha;
        activation_indicator.at<uchar>(0,n) = (pvalue<alpha) ? 1: 0;
    }
}

void P_SPM::_get_Activation_indicator_Bonferroni2(const Mat& T_map,Mat &activation_indicator,double alpha,double &degree_of_freedom)
{
    activation_indicator = Mat::zeros(T_map.size(),CV_8UC1);
    students_t dist(degree_of_freedom);

    for(int n=0;n<T_map.cols;n++)
    {
        double pvalue = cdf(complement(dist, fabs(T_map.at<float>(0,n))))*2;
        //    The Null-hypothesis: there is no difference in means
        //   Reject if complement of CDF for |t| < significance level:
        //    double pvalue = cdf(complement(dist, fabs(t)))*2 < alpha;
        activation_indicator.at<uchar>(0,n) = (pvalue<alpha) ? 1: 0;
    }
}


// Process the whole SPM procedure to get activation map
Mat P_SPM::process_SPM(Mat Delta_C_map, QVector<cv::Point> pixels_pos, cv::Size img_size, QString path, QVector<float> &model, Mat &mask_SPM)
{
    //Get number of frames
    int T = Delta_C_map.rows;

    //Compute GLM
    Mat Design_Matrix = Build_Design_Matrix(model,T);
    GeneralLinearModel(Delta_C_map,Design_Matrix);

    //Get Z maps
    Mat Z_map = get_Z_map();
    Mat Z_maps_reconstructed = Mat::zeros(img_size,CV_32F);
    for(int i=0;i<pixels_pos.size();i++)
        Z_maps_reconstructed.at<float>(pixels_pos[i].y,pixels_pos[i].x)    = Z_map.at<float>(0,i);


    //Process RFT
    Z_maps_reconstructed = gaussianBlur(Z_maps_reconstructed);
    mask_SPM = getThresholdedZMap(Z_maps_reconstructed);


    //Write GLM results
    Mat T_map = get_T_map();
    Mat T_maps_reconstructed = Mat::zeros(img_size,CV_32F);
    for(int i=0;i<pixels_pos.size();i++)
        T_maps_reconstructed.at<float>(pixels_pos[i].y,pixels_pos[i].x)    = T_map.at<float>(0,i);


    WriteFloatImg(path+"Z_Stat_blured"+".txt",Z_maps_reconstructed);
    WriteFloatImg(path+"T_Stat"+".txt",T_maps_reconstructed);
    imwrite(QString(path+"SPM.png").toStdString(),mask_SPM);

    return Z_maps_reconstructed;
}

Mat P_SPM::process_SPM_auto_thresh(Mat Delta_C_map, QVector<cv::Point> pixels_pos, cv::Size img_size, QString path, QVector<float> &model, Mat &img_mask, float auto_thresh)
{

    //Get number of frames
    int T = Delta_C_map.rows;

    //Compute GLM
    Mat Design_Matrix = Build_Design_Matrix(model,T);
    GeneralLinearModel(Delta_C_map,Design_Matrix);

    //Get Z maps
    Mat Z_map = get_Z_map();
    Mat Z_maps_reconstructed = Mat::zeros(img_size,CV_32F);
    for(int i=0;i<pixels_pos.size();i++)
        Z_maps_reconstructed.at<float>(pixels_pos[i].y,pixels_pos[i].x)    = Z_map.at<float>(0,i);


    //Process auto thresholding
    Z_maps_reconstructed = gaussianBlur(Z_maps_reconstructed);

    cv::Scalar mean_val, std_val;
    cv::meanStdDev(Z_maps_reconstructed, mean_val, std_val, img_mask); // only computes on mask pixels

    float thresh_value = static_cast<float>(mean_val[0] + auto_thresh * std_val[0]);

    Mat mask_auto= Mat::zeros(img_size,CV_8UC1);
    Mat Z_maps_auto = Mat::zeros(img_size,CV_32F);
    for(int row=0;row<Z_maps_auto.rows;row++)
    {
        float *ptrZ = Z_maps_reconstructed.ptr<float>(row);
        float *ptrZ_T = Z_maps_auto.ptr<float>(row);
        float *ptr_mask = mask_auto.ptr<float>(row);

        for(int col=0;col<Z_maps_auto.cols;col++)
        {
            if(ptrZ[col]>thresh_value)
            {
                ptrZ_T[col] = ptrZ[col];
                ptr_mask[col] = 255;
            }
        }
    }

    //Write GLM results
    Mat T_map = get_T_map();
    Mat T_maps_reconstructed = Mat::zeros(img_size,CV_32F);
    for(int i=0;i<pixels_pos.size();i++)
        T_maps_reconstructed.at<float>(pixels_pos[i].y,pixels_pos[i].x)    = T_map.at<float>(0,i);


    WriteFloatImg(path+"Z_Stat_blured"+".txt",Z_maps_reconstructed);
    WriteFloatImg(path+"Z_Stat_auto"+".txt",Z_maps_auto);
    WriteFloatImg(path+"T_Stat"+".txt",T_maps_reconstructed);
    imwrite(QString(path+"auto_mask.png").toStdString(),mask_auto);

    return Z_maps_auto;
}



// Y: data (Hb, HbO2 concentration changes) Size: (T,N)
// X: Design matrix (Bold signal with physiological a priori on amplitude) Size: (T,S).
// S: number of stimulus or condition, here S=2 because two condition or explored (out of phase, phase)
// B: spatial pattern of responses. Size(S=2,N)
// e: error term. Size: (T,N)
// Process the whole SPM procedure to get activation map
void P_SPM::process_SPM_Phase_Correlation(Mat Delta_C_map, QString path, QVector<float> &model,QVector<cv::Point> pixels_pos, cv::Size img_size)
{
    //Compute GLM
    Mat Design_Matrix = Mat::ones(model.size(),2,CV_32F);

    for(int row=0;row<Design_Matrix.rows;row++)
        Design_Matrix.at<float>(row,0) = model[row];

//    GeneralLinearModel(Delta_C_map,Design_Matrix);

    //resolve linerar model
    // B = (X'X)-1 X'Y
    Mat tr_X;
    Mat invert_X;
    transpose(Design_Matrix,tr_X);

    mulTransposed(Design_Matrix,invert_X,true);
    invert(invert_X,invert_X,DECOMP_SVD);

    //    qDebug()<<"Y size("<<Y.rows<<";"<<Y.cols<<")";
    //    qDebug()<<"X_tr size("<<tr_X.rows<<";"<<tr_X.cols<<")";
    //    qDebug()<<"(X_tr.X)-1 size("<<invert_X.rows<<";"<<invert_X.cols<<")";



    //Spatial pattern of responses
    Mat B = (invert_X*tr_X)*Delta_C_map;

    //errors
    Mat e = Delta_C_map - Design_Matrix*B;

    Mat var_e;
    extractVarianceFromMat(e,var_e);

    //Residuals forming matrix
    Mat R = Design_Matrix*invert_X*tr_X;
    R = Mat::eye(R.size(),R.type()) - R;


    //Get T maps out of phase
    _GLM_t_stats(invert_X,B,var_e,_M_T_map,0); //Stimulus id: 0

//    Mat T_maps_reconstructed = Mat::zeros(img_size,CV_32F);
//    for(int i=0;i<pixels_pos.size();i++)
//        T_maps_reconstructed.at<float>(pixels_pos[i].y,pixels_pos[i].x)    = _M_T_map.at<float>(0,i);
//    WriteFloatImg(path+"T_Stat_out_phase"+".txt",T_maps_reconstructed);

    WriteFloatImg(path+"beta.txt",B);
    //Get Z maps
    Mat Z_map = get_Z_map();
    Mat Z_maps_reconstructed = Mat::zeros(img_size,CV_32F);
    for(int i=0;i<pixels_pos.size();i++)
        Z_maps_reconstructed.at<float>(pixels_pos[i].y,pixels_pos[i].x)    = Z_map.at<float>(0,i);


    //Process RFT
    Z_maps_reconstructed = gaussianBlur(Z_maps_reconstructed);
    Mat mask_SPM = getThresholdedZMap(Z_maps_reconstructed);

    //Write GLM results
    Mat T_map = get_T_map();
    Mat T_maps_reconstructed = Mat::zeros(img_size,CV_32F);
    for(int i=0;i<pixels_pos.size();i++)
        T_maps_reconstructed.at<float>(pixels_pos[i].y,pixels_pos[i].x)    = T_map.at<float>(0,i);


    WriteFloatImg(path+"Z_Stat_blured.txt",Z_maps_reconstructed);
    WriteFloatImg(path+"T_Stat.txt",T_maps_reconstructed);
    imwrite(QString(path+"SPM.png").toStdString(),mask_SPM);
    WriteFloatImg(path+"X.txt",Design_Matrix);
}


