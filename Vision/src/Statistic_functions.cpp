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

double _ThreshProbaToFitPercentage(double v)
{
    double val = (-v*100) +100;
    return (val);
}

double _FitPercentageToThreshProba(int v)
{
    return (100.0f-v)*0.01f;
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

double get_Min(const QVector<float> &vec)
{
    if(vec.empty())
        return -1;

    double min = vec[0];

    for(int i=0;i<vec.size();i++)
        min = (vec[i]<min) ? vec[i] : min;
    return min;
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

/** Get the max value of a vector of float */
double get_Min(const QVector<QVector<float> > &vec)
{
    if(vec.empty())
        return -1;

    double min = vec[0][0];

    for(int i=0;i<vec.size();i++)
        min = get_Min(vec[i]);
    return min;
}


int argMax(const vector<float> &v)
{
    return std::max_element(v.begin(),v.end()) - v.begin();
//    int maxElement = *std::max_element(v.begin(), v.end());
}

int argMax(const vector<double> &v)
{
    return std::max_element(v.begin(),v.end()) - v.begin();
//    int maxElement = *std::max_element(v.begin(), v.end());
}

int argMin(const vector<float> &v)
{
    return std::min_element(v.begin(),v.end()) - v.begin();
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


float get_Median(const QVector<float> &vec,int id_start,int id_end)
{
    if(id_start == -1)
        id_start=0;
    if(id_end == -1)
        id_end = vec.size()-1;

    if(id_start<0 || id_start>vec.size())
        return 0.0f;
    if(id_end<0 || id_end>vec.size())
        return 0.0f;

    QVector<float> vec_copy = vec.mid(id_start,id_end);

    std::sort(vec_copy.begin(), vec_copy.end());
    float median = vec_copy.size()%2? vec_copy[vec_copy.size() / 2] : ((double)vec_copy[vec_copy.size() / 2 - 1] + vec_copy[vec_copy.size() / 2]) * .5;

    return median;
}



void get_Std_Mean(const QVector<float> &vec,float &stdVal,float &meanVal)
{
    meanVal = 0.0;
    stdVal  = 0.0;

    for(int i=0;i<vec.size();i++)
    {
        stdVal  += vec[i]*vec[i];
        meanVal += vec[i];
    }

    if(vec.empty())
    {
        meanVal= -1;
        stdVal =-1;
        return;
    }
    meanVal/=vec.size();
    stdVal/=vec.size();

    stdVal = sqrt(stdVal - (meanVal*meanVal));
}

QVector<float> get_Gaussian(float Half_width,float mean_val,const QVector<float> &x)
{
    float sig = Half_width/2.3548;
    QVector<float> out;
    out.fill(0,x.size());

    for(int i=0;i<x.size();i++)
        out[i] = (1/(sig*sqrt(2*PI)))*exp(-pow(x[i]-mean_val,2)/(2*pow(sig,2)));

    return out;
}




QVector<float> conv(const QVector<float> &f, const QVector<float> &g)
{
  int const nf = f.size();
  int const ng = g.size();
  int const n  = nf + ng - 1;
  QVector<float> out;
  out.fill(0,n);

  for(int i=0; i < n; i++)
  {
    int const jmn = (i >= ng - 1)? i - (ng - 1) : 0;
    int const jmx = (i <  nf - 1)? i            : nf - 1;
    for(int j = jmn; j <= jmx; j++)
    {
      out[i] += (f[j] * g[i - j]);
    }
  }
  return out;

//    QVector<float> out;
//    out.fill(0,f.size());


//    for ( int i = 0; i < f.size(); i++ )
//    {
//        out[i] = 0;                       // set to zero before sum
//        for (int j = 0; j < g.size(); j++ )
//        {
//            out[i] += f[i - j] * g[j];    // convolve: multiply and accumulate
//        }
//    }
//    return out;
}




double get_KL_Divergence(const QVector<double> & pk, const QVector<double> &qk)
{
    double S=0.0f;

    for(int i=0;i<pk.size();i++)
    {
        if(qk[i]!=0 && pk[i]!=0)
        {
            S += (pk[i] * (log10(abs((pk[i]/qk[i])))));
//            qDebug()<<"S : "<<S<<" log(...) : "<<(log(abs((pk[i]/qk[i]))))<<" pk : "<<pk[i]<<" qk : "<<qk[i];
        }
    }

    return (1-S);
}

float get_SID(QVector<float> pk, QVector<float> qk)
{
    double SID=0.0f;

    if(pk.size()!=qk.size())
        return -1;

    pk = pk-(float)get_Min(pk);
    qk = qk-(float)get_Min(qk);

    //Normalize vector
    NormalizeVector(pk);
    NormalizeVector(qk);

    for(int i=0;i<pk.size();i++)
    {
        if(qk[i]!=0 && pk[i]!=0)
        {
            SID += (pk[i] * (log((pk[i]/qk[i])))) + (qk[i] * (log((qk[i]/pk[i]))));
//            qDebug()<<"S : "<<S<<" log(...) : "<<(log(abs((pk[i]/qk[i]))))<<" pk : "<<pk[i]<<" qk : "<<qk[i];
        }
    }

    return SID;
}

void NormalizeVector(QVector<double> &vector)
{
    float sum = 0.0f;

    for(int i=0;i<vector.size();i++)
        sum +=vector[i];

    for(int i=0;i<vector.size();i++)
        vector[i]/=sum;

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

void NormalizeVector(QVector<float> &vector)
{
    float sum = 0.0f;

    for(int i=0;i<vector.size();i++)
        sum +=vector[i];

    for(int i=0;i<vector.size();i++)
        vector[i]/=sum;
}

//// Get probability density

//void getProbabilityDensity(QVector<float> &vector_in_out)
//{
//    // Set minimum to 0
//    double minVal = get_Min(vector_in_out);
//    for(int i = 0;i<vector_in_out.size();i++)
//        vector_in_out[i]=vector_in_out[i] - minVal;

//    // Get density probability
//    double sum = get_Vector_sum(vector_in_out);
//    for(int i = 0;i<vector_in_out.size();i++)
//        vector_in_out[i] /= sum;
//}

//void getProbabilityDensity(const QVector<float> &vector_in,QVector<float> &density_proba)
//{
//    density_proba.clear();

//    // Set minimum to 0
//    double minVal = get_Min(vector_in);
//    for(int i = 0;i<vector_in.size();i++)
//        density_proba.push_back(vector_in[i] - minVal);

//    // Get density probability
//    double sum = get_Vector_sum(density_proba);
//    for(int i = 0;i<density_proba.size();i++)
//        density_proba[i] /= sum;
//}

//void getProbabilityDensity(const QVector<float> &vector_in,QVector<float> &density_proba,int coeff_multi)
//{
//    density_proba.clear();

//    for(int i = 0;i<vector_in.size();i++)
//        density_proba.push_back(vector_in[i]*coeff_multi);

//    getProbabilityDensity(density_proba);
//}



double processReliabilityFactor(const QVector<float> &Mesure,const QVector<float> &Model)
{
    if(Mesure.size()!=Model.size() || Mesure.empty() || Model.empty())
        return -1;

    double dist = 0.0;
//    for(int i=0;i<Mesure.size();i++)
//        dist += (abs(Mesure[i] - Model[i]));
    double denum =0.0;
    for(int i=0;i<Mesure.size();i++)
    {
        dist    += (Mesure[i] - Model[i])*(Mesure[i] - Model[i]);
        denum   += Model[i]*Model[i];
    }

    dist = sqrt(dist/denum);

    return _ThreshProbaToFitPercentage(dist);
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


bool two_samples_t_test_different_sd(
        double Sm1,       // Sm1 = Sample 1 Mean.
        double Sd1,       // Sd1 = Sample 1 Standard Deviation.
        unsigned Sn1,     // Sn1 = Sample 1 Size.
        double Sm2,       // Sm2 = Sample 2 Mean.           -> reference
        double Sd2,       // Sd2 = Sample 2 Standard Deviation.
        unsigned Sn2,     // Sn2 = Sample 2 Size.
        double alpha,       // alpha = Significance Level.
        double &pvalue,
        double &t_stat,
        int stat_type)      //stat type :  0 The Null-hypothesis: there is no difference in means
                            //             1 The alternative hypothesis: m1<m2
                            //             2 The alternative hypothesis: m1>m2

{
    //control is input are valid
    if(std::isnan(Sm1) || std::isinf(Sm1) || std::isnan(Sd1) || std::isinf(Sd1) ||
       std::isnan(Sm2) || std::isinf(Sm2) || std::isnan(Sd2) || std::isinf(Sd2))
        return false;

    // Degrees of freedom:
    double v = (Sd1 * Sd1) / Sn1 + (Sd2 * Sd2) / Sn2;
    v *= v;
    double t1 = (Sd1 * Sd1) / Sn1;
    t1 *= t1;
    t1 /=  (Sn1 - 1);
    double t2 = Sd2 * Sd2 / Sn2;
    t2 *= t2;
    t2 /= (Sn2 - 1);
    v /= (t1 + t2);

    // t-statistic:
    t_stat = (Sm1 - Sm2) / sqrt(((Sd1 * Sd1) / Sn1) + ((Sd2 * Sd2) / Sn2));

    bool results = false;

    if(std::isnan(v) || std::isinf(v) || std::isnan(t_stat) || std::isinf(t_stat))
        return false;

    students_t dist(v);


    if(stat_type==0)
    {
        //    The Null-hypothesis: there is no difference in means
        //   Reject if complement of CDF for |t| < significance level:
        //    double pvalue = cdf(complement(dist, fabs(t)))*2 < alpha;
        pvalue = cdf(complement(dist, fabs(t_stat)))*2;
        results = (pvalue<alpha) ? true : false;
    }
    if(stat_type==1)
    {
        //The Alternative-hypothesis: Sample 1 Mean is less than Sample 2 Mean.
        //Reject if CDF of t > significance level:
        pvalue = cdf(dist, t_stat);
        results =  (pvalue> alpha) ? false : true;
    }
    if(stat_type==2)
    {
        //The Alternative-hypothesis: Sample 1 Mean is greater than Sample 2 Mean.
        //Reject if complement of CDF of t > significance level:
        pvalue = cdf(complement(dist, t_stat));
        results =  (pvalue> alpha) ? false : true;
    }

    return results;
}



float get_T_stats_Compare_Two_Means(
        double Sm1,       // Sm1 = Sample 1 Mean.
        double Sd1,       // Sd1 = Sample 1 Standard Deviation.
        unsigned Sn1,     // Sn1 = Sample 1 Size.
        double Sm2,       // Sm2 = Sample 2 Mean.           -> reference
        double Sd2,       // Sd2 = Sample 2 Standard Deviation.
        unsigned Sn2     // Sn2 = Sample 2 Size.
        )
{
    //control is input are valid
    if(std::isnan(Sm1) || std::isinf(Sm1) || std::isnan(Sd1) || std::isinf(Sd1) ||
       std::isnan(Sm2) || std::isinf(Sm2) || std::isnan(Sd2) || std::isinf(Sd2))
        return 0.0f;

    // Degrees of freedom:
    double v = (Sd1 * Sd1) / Sn1 + (Sd2 * Sd2) / Sn2;
    v *= v;
    double t1 = (Sd1 * Sd1) / Sn1;
    t1 *= t1;
    t1 /=  (Sn1 - 1);
    double t2 = Sd2 * Sd2 / Sn2;
    t2 *= t2;
    t2 /= (Sn2 - 1);
    v /= (t1 + t2);

    // t-statistic:
    return float((Sm1 - Sm2) / sqrt(((Sd1 * Sd1) / Sn1) + ((Sd2 * Sd2) / Sn2)));

}

float get_T_stats_Compare_To_True_Mean(double True_Mean, double Sample_mean, double Sample_std, unsigned Sample_size)
{
    // True_Mean = true mean.
    // Sample_mean = Sample Mean.
    // Sample_std = Sample Standard Deviation.
    // Sample_size = Sample Size.
    // alpha = Significance Level.

     //control is input are valid
     if(std::isnan(True_Mean) || std::isinf(True_Mean) ||
        std::isnan(Sample_mean) || std::isinf(Sample_mean) || std::isnan(Sample_std) || std::isinf(Sample_std) || Sample_size==0)
         return 0.0f;

     // Difference in means:
     double diff = Sample_mean - True_Mean;

//     // Degrees of freedom:
//     unsigned v = Sample_size - 1;

     // t-statistic:
     return float(diff * sqrt(double(Sample_size)) / Sample_std);
}


bool single_sample_t_test(double True_Mean, double Sample_mean, double Sample_std, unsigned Sample_size, double alpha,int stat_type)
{
   // True_Mean = true mean.
   // Sample_mean = Sample Mean.
   // Sample_std = Sample Standard Deviation.
   // Sample_size = Sample Size.
   // alpha = Significance Level.

    //control is input are valid
    if(std::isnan(True_Mean) || std::isinf(True_Mean) ||
       std::isnan(Sample_mean) || std::isinf(Sample_mean) || std::isnan(Sample_std) || std::isinf(Sample_std) || Sample_size==0)
        return false;

    // Difference in means:
    double diff = Sample_mean - True_Mean;
    // Degrees of freedom:
    unsigned v = Sample_size - 1;
    // t-statistic:
    double t_stat = diff * sqrt(double(Sample_size)) / Sample_std;

    if(std::isnan(t_stat) || std::isinf(t_stat))
        return false;

    students_t dist(v);
    bool results    = false;
    double pvalue   = 0.0;

    if(stat_type==0)
    {
        //    The Null-hypothesis: there is no difference in means
        //   Reject if complement of CDF for |t| < significance level:
        //    double pvalue = cdf(complement(dist, fabs(t)))*2 < alpha;
        pvalue = cdf(complement(dist, fabs(t_stat)))*2;
        results = (pvalue<alpha) ? true : false;
    }
    if(stat_type==1)
    {
        //The Alternative-hypothesis: Sample 1 Mean is less than Sample 2 Mean.
        //Reject if CDF of t > significance level:
        pvalue = cdf(dist, t_stat);
        results =  (pvalue> alpha) ? false : true;
    }
    if(stat_type==2)
    {
        //The Alternative-hypothesis: Sample 1 Mean is greater than Sample 2 Mean.
        //Reject if complement of CDF of t > significance level:
        pvalue = cdf(complement(dist, t_stat));
        results =  (pvalue> alpha) ? false : true;
    }

    return results;
}

//corr      : correlation coefficient
//n         : nb of element for the calculation of corr
//v         : degree of freedom
//alpha     : Significance Level.
//stat type :  0 The Null-hypothesis: there is no difference in means
//             1 The alternative hypothesis: m1<m2
//             2 The alternative hypothesis: m1>m2

/* Idee: utiliser le test de différences de corrélation (voir annexe cocor : /home/caredda/Charly/Thèse/docs/statistic/compare_correlations.pdf)
 * convert z score en p value avec la distribution normale (et non la distribution de student)
 * comparer le coeff de corrélation mesurée au pixel p(x,y) à un coeff de correlation calculé entre spectre de reflectance sur tissue non biologique (étalon blanc) et réponse hemodynamique attendue)
 */

bool test_on_corr(double corr,int n,int stat_type,double alpha,double &pvalue)
{
    //control is input are valid
    if(std::isnan(corr) || std::isinf(corr) || n==0)
        return false;


    //Calculate degree of freedom
    unsigned v = n-2;

    //Compute t_stat
    double t_stat = corr*sqrt(double(v))/sqrt(1-(corr*corr));

    if(std::isnan(t_stat) || std::isinf(t_stat))
        return false;

    bool results = false;
    students_t dist(v);


//    For a lower-tailed test, p-value = P(TS < ts | H0 is true) = cdf(ts)
//    For an upper-tailed test, p-value = P(TS > ts | H0 is true) = 1 - cdf(ts)
//    assuming that the distribution of the test statistic under H0 is symmetric about 0, a two-sided test is specified by: p-value = 2 * P(TS |ts| | H0 is true) = 2 * (1 - cdf(|ts|))

    //Significant negative correlation
    if(stat_type==1)
    {
        if(corr>=0)
            return false;
    }

    //Significant positive correlation
    if(stat_type==2)
    {
        if(corr<=0)
            return false;
    }


    if(stat_type==0)
    {
        //    The Null-hypothesis: there is no difference in means
        //   Reject if complement of CDF for |t| < significance level:
        //    double pvalue = cdf(complement(dist, fabs(t)))*2 < alpha;
        pvalue = cdf(complement(dist, fabs(t_stat)))*2;
        results = (pvalue<alpha) ? true : false;
    }
    if(stat_type==1)
    {
        //The Alternative-hypothesis: Sample 1 Mean is less than Sample 2 Mean.
        //Reject if CDF of t > significance level:
        pvalue = cdf(dist, t_stat);
        results =  (pvalue> alpha) ? false : true;
    }
    if(stat_type==2)
    {
        //The Alternative-hypothesis: Sample 1 Mean is greater than Sample 2 Mean.
        //Reject if complement of CDF of t > significance level:
        pvalue = cdf(complement(dist, t_stat));
        results =  (pvalue> alpha) ? false : true;
    }

    return results;

    /*

    if(stat_type==0)
    {
        //    The Null-hypothesis: there is no difference in means
//        Reject if complement of CDF for |t| < significance level / 2:
        pvalue = cdf(complement(dist, fabs(t_stat)));
        results = (pvalue< alpha / 2) ? false : true;
    }
    if(stat_type==1)
    {
        //The Alternative-hypothesis: the sample mean is less than the true mean.
        //Reject if CDF of t > 1 - significance level:
        pvalue = cdf(complement(dist, t_stat));
        results = (pvalue< alpha) ? false : true;
    }
    if(stat_type==2)
    {
        //The Alternative-hypothesis: the sample mean is greater than the true mean.
        //Reject if complement of CDF of t < significance level:
        pvalue = cdf(dist, t_stat);
        results = (pvalue< alpha)? false : true;
    }
    */

    return results;
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



