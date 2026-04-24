#include "Statistic_functions.h"


float fisher_r_to_z_transform(float r)
{
    float z=0.5*(log(1+r)-log(1-r));
    z = (std::isnan(z) || std::isinf(z))? 0 : z;
    return z;
}

//R : correlation coefficient
//df : degree of freeedom

float convert_Corr_to_T_stats(float R,int df)
{
    return float(sqrt((df*R*R)/(1-(R*R))));
}

float convert_T_to_Z_stats(float t)
{
    return (t-50.0f)/10.0f;
}




double get_Vector_sum(const QVector<float> &vec,int size)
{
    if(vec.empty())
        return -1;

    double sum = 0;

    for(int i=0;i<size;i++)
        sum +=vec[i];

    return sum;
}

double get_Vector_sum(const QVector<double> &vec)
{
    if(vec.empty())
        return -1;

    double sum = 0;

    for(int i=0;i<vec.size();i++)
        sum +=vec[i];

    return sum;
}



double get_Max(const QVector<float> &vec)
{
    if(vec.empty())
        return -1;

    double max = vec[0];

    for(int i=0;i<vec.size();i++)
        max = (vec[i]>max) ? vec[i] : max;
    return max;
}


/** Get the max value of a vector of float */
double get_Max(const QVector<QVector<float> > &vec)
{
    if(vec.empty())
        return -1;

    double max = vec[0][0];

    for(int i=0;i<vec.size();i++)
        max = get_Max(vec[i]);
    return max;
}





float get_Mean(const QVector<float> &vec, int id_start, int id_end)
{
    if(id_start == -1)
        id_start=0;
    if(id_end == -1)
        id_end = vec.size();

    if(id_start<0 || id_start>vec.size())
        return 0.0f;
    if(id_end<0 || id_end>vec.size())
        return 0.0f;

    float meanVal = 0.0;

    for(int i=id_start;i<id_end;i++)
        meanVal += vec[i];

    if(vec.empty())
    {
        meanVal= -1;
        return meanVal;
    }
    meanVal/=vec.size();
    return meanVal;
}







void NormalizeMaxVector(QVector<double> &vector)
{
    if(vector.empty())
        return;

    float max =vector[0];

    for(int i=0;i<vector.size();i++)
        max = (max<vector[i]) ? vector[i] : max;;

    for(int i=0;i<vector.size();i++)
        vector[i]/=max;

}






//Pearson correlation coeff

double mean(const QVector<float> &a,int size)
{
    return get_Vector_sum(a,size) / size;
}

double sqsum(const QVector<float> &a,int size)
{
    double s = 0;
    for (int i = 0; i < size; i++)
    {
        s += pow(a[i], 2);
    }
    return s;
}

double stdev(const QVector<float> &nums,int size)
{
    return pow(sqsum(nums,size) / size - pow(get_Vector_sum(nums,size) / size, 2), 0.5);
}

QVector<float> operator-(const QVector<float> &a,const float &b)
{
    QVector<float> retvect;

    for (int i = 0; i < a.size(); i++)
    {
        retvect.push_back(a[i] - b);
    }
    return retvect;
}
QVector<float> operator+(const QVector<float> &a,const QVector<float> &b)
{
    QVector<float> retvect;

    for (int i = 0; i < a.size(); i++)
    {
        retvect.push_back(a[i] + b[i]);
    }
    return retvect;
}


QVector<float> operator/(const QVector<float> &a,const float b)
{
    QVector<float> retvect;

    for (int i = 0; i < a.size(); i++)
    {
        retvect.push_back(a[i]/b);
    }
    return retvect;
}


QVector<double> operator/(const QVector<double> &a,const double b)
{
    QVector<double> retvect;

    for (int i = 0; i < a.size(); i++)
    {
        retvect.push_back(a[i]/b);
    }
    return retvect;
}

QVector<float> operator*(const QVector<float> &a,const QVector<float> &b)
{
    int size = (a.size()<b.size()) ? a.size() : b.size();

    QVector<float> retvect;
    for (int i = 0; i < size ; i++)
    {
        retvect.push_back(a[i] * b[i]);
    }
    return retvect;
}

double pearsoncoeff(const QVector<float> &X,const QVector<float> &Y)
{
    int process_duration = (X.size()<Y.size()) ? X.size() : Y.size();

//    return (double)(get_Vector_sum((X - mean(X,process_duration))*(Y - mean(Y,process_duration)),process_duration) / (process_duration*stdev(X,process_duration)* stdev(Y,process_duration)));
    return (double)(mean((X - mean(X,process_duration))*(Y - mean(Y,process_duration)),process_duration) / (stdev(X,process_duration)* stdev(Y,process_duration)));

}

double pearsoncoeff(const QVector<float> &X,const QVector<float> &Y,const int &process_duration)
{
    return (double)(mean((X - mean(X,process_duration))*(Y - mean(Y,process_duration)),process_duration) / (stdev(X,process_duration)* stdev(Y,process_duration)));
}









//Extract variance from Matrix
//in size(T,N)
//out size(1,N)
void extractVarianceFromMat(const Mat& in,Mat &out)
{
    out = Mat::zeros(1,in.cols,CV_32F);
    float *ptr = out.ptr<float>(0);
    for(int col=0;col<in.cols;col++)
    {
        Mat subMat = in.col(col);
        Scalar mean,stddev;

        meanStdDev(subMat,mean,stddev);
        ptr[col] = stddev[0]*stddev[0];
    }
}

// Autocorrelation function. correlation of in by itself
// Procedure, compute dft of in, then mutliply dft(in) to it self, and go back to the space domain
Mat autocorrelation(const Mat &in)
{
    //dft
    Mat out;
    dft(in,out);
    //Multiply the dft to itself
    mulSpectrums(out,out,out,0);
    //inverse dft
    idft(out,out);
    return out;
}

//in size(T,N)
Mat autocorrelation(const Mat &in,int spatial_id)
{
    qDebug()<<"autocorr pixel "<<spatial_id;
    qDebug()<<"autocorr step 1";

    Mat out = Mat::zeros(in.rows,in.rows,CV_32F);
    Mat subMat = in.col(spatial_id);

    //Process autocorrelation
    Mat auto_corr;
    dft(subMat,auto_corr);

    //Multiply the dft to its conjugate
    mulSpectrums(auto_corr,auto_corr,auto_corr,0,true);
    //inverse dft
    idft(auto_corr,auto_corr);

    qDebug()<<"autocorr step 2";

    //Compute the autocorrelation matrix
    for(int h=0;h<out.rows;h++)
    {
        float *ptr_out = out.ptr<float>(h);
        for(int l=0;l<out.cols;l++)
        {
            int id = abs(h-l);
            id = (id>auto_corr.rows-1) ? auto_corr.rows-1 : id;

            ptr_out[l] = auto_corr.at<float>(id,0);
        }
    }
    return out;
}

double getTrace_of_product(const Mat &a,const Mat &b)
{
    double tr = 0.0;
    for(int id=0;id<a.rows;id++)
    {
        Mat res = a.row(id)*b.col(id);
        tr += res.at<float>(0,0);
    }
    return tr;
}



