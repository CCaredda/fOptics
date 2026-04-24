#include "filtering.h"






//Hamming Low pass filter
void Blackman_Window(const double Fs,const double Fc,QVector<float> &filter,const int N)
{


    double T = 2*(int)(Fc*N/Fs);
    int id1 = T/2;
    int id2 = T/2;


    //Création du filtre
    filter.clear();
    for(int i=0;i<N;i++)
    {
        filter.push_back(0);
    }

    double dF   = Fs/N;

    for(int i=0;i<N;i++)
    {
        if(id1>=T || id2<=0)
            break;

        if(((i*dF)>0.5*Fc) &&(id1<T && id2>=0))
        {
            filter[i]       = 0.42 - 0.5*cos(2*PI*((double)id1/T)) + 0.08*cos(4*PI*((double)id1/T));
            filter[N-1-i]   = 0.42 - 0.5*cos(2*PI*((double)id2/T)) + 0.08*cos(4*PI*((double)id2/T));
            id1++;
            id2--;
        }
        else
        {
            filter[i]=1;
            filter[N-1 -i]=1;
        }

    }
}






void Apply_Filtering(const QVector<float> &x, QVector<float> &data,const fftwf_plan &p, const fftwf_plan &q, const QVector<float> &filter)
{
    if(filter.size()!=data.size() )
    {
        qDebug()<<"Error in filter and data size";
        return;
    }

    //init values
    int N                    = data.size();
    fftwf_complex  *in       = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex)*N);
    fftwf_complex  *out      = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex)*N);

    //Redressement des données
    vector<float> _p;
    if(polyfit(x,data,1,_p)==0)
    {
        qDebug()<<"[Polyfit] error";
    }


    for(int i=0;i<N;i++)
    {
        in[i][0]    = data[i]-(_p[1]*x[i] + _p[0]);
        in[i][1]    = 0;
    }

    ////////////////////////////////////////////////////////////////////////
    ///////////////////////Filtering///////////////////////////////
    ////////////////////////////////////////////////////////////////////////


    //Forward DFT
    fftwf_execute_dft(p,in,out);

    //Apply Low pass Filter and Band pass filter
    for ( int f = 0 ; f <N; f++ )
    {
        out [ f ] [ 0 ]     = out [ f ] [ 0 ] * filter[f] ;
        out [ f ] [ 1 ]     = out [ f ] [ 1 ] * filter[f] ;
    }

    //Inverse DFT
    fftwf_execute_dft(q,out,in);

    //Normalize
    for (int i = 0; i < N; i++)
        data[i]    = in[i][0]/N + _p[0];



    fftwf_free(in);
    fftwf_free(out);
}




void Apply_Data_Correction(const QVector<float> &x, QVector<float> &data)
{
    //init values
    int N                    = data.size();

    //Redressement des données
    vector<float> _p;
    if(polyfit(x,data,1,_p)==0)
    {
        qDebug()<<"[Polyfit] error";
    }

    for(int i=0;i<N;i++)
        data[i]    = data[i]-_p[1]*x[i];
}


// Apply data correction
//QVector<float> Apply_Data_Correction(const QVector<float> &x,const QVector<float> &x_first,const QVector<float> &x_last, const QVector<float> &data)
QVector<float> Apply_Data_Correction(const QVector<float> &data,int start_acquis, int end_acquis, int rest1_start, int rest1_end, int rest_last_start, int rest_last_end)
{
    //x and data have to be the same dimension
    // Calculate the linear regression between [start_id1 end_id] and [start_id2,end_id2]

    float mean_data_first_rest = get_Mean(data,rest1_start,rest1_end);
    float mean_data_last_rest = get_Mean(data,rest_last_start,rest_last_end);
//    float mean_x_first_rest = rest1_start+(rest1_end-rest1_start)/2;
//    float mean_x_last_rest = rest_last_start+(rest_last_end-rest_last_start)/2;

    float slope = (mean_data_last_rest - mean_data_first_rest)/(end_acquis-start_acquis);

    QVector<float> corr_data(data);

    corr_data.fill(end_acquis-start_acquis,0);
    int x=0;
    for(int i=start_acquis;i<end_acquis;i++)
    {
        corr_data[i] = (data[i]-slope*x);
        x++;
    }

    return corr_data;
}




void Apply_Low_Pass_Filtering(const QVector<float> &x, QVector<float> &data,const fftwf_plan &p, const fftwf_plan &q, const QVector<float> &filter)
{
    if(filter.size()!=data.size() )
    {
        qDebug()<<"Error in filter and data size";
        return;
    }

    //init values
    int N                    = data.size();
    fftwf_complex  *in       = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex)*N);
    fftwf_complex  *out      = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex)*N);

    //Redressement des données
    vector<float> _p;
    if(polyfit(x,data,1,_p)==0)
    {
        qDebug()<<"[Polyfit] error";
    }

    for(int i=0;i<N;i++)
    {
        in[i][0]    = data[i]-_p[0];
        in[i][1]    = 0;
    }

    ////////////////////////////////////////////////////////////////////////
    ///////////////////////Filtering///////////////////////////////
    ////////////////////////////////////////////////////////////////////////


    //Forward DFT
    fftwf_execute_dft(p,in,out);

    //Apply Low pass Filter and Band pass filter
    for ( int f = 0 ; f <N; f++ )
    {
        out [ f ] [ 0 ]     = out [ f ] [ 0 ] * filter[f] ;
        out [ f ] [ 1 ]     = out [ f ] [ 1 ] * filter[f] ;
    }

    //Inverse DFT
    fftwf_execute_dft(q,out,in);

    //Normalize
    for (int i = 0; i < N; i++)
        data[i]    = in[i][0]/N + _p[0];



    fftwf_free(in);
    fftwf_free(out);
}











































