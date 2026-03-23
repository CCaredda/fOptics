#include "filtering.h"





/******************************************************************************
 ***********************    passe_bas_frequentiel    ************************
 ******************************************************************************
 * Entrees :
 *  - INOUT: tableau contenant les donnees a filtrer
 *  - Fe : frequence d'echantillonnage du signal
 *  - Fc : frequence de coupure du passe-bas
 *  - ui_ordre : ordre du filtre
 * Sorties :
 *  - INOUT : tableau contenant les donnees apres filtrage
 ******************************************************************************/
void FiltragePasseBasFFTW_No_OpenMP ( QVector<double> &INOUT,const double Fe ,
const double Fc ,const unsigned int ui_ordre )
{
  /////////////////////////////////////////
  ////////////////////////////////////////
  // Filtrage dans le domaine fréquentiel avec DFT complex to complex
  ///////////////////////////////////////
  //////////////////////////////////////
  /////////////////////////////////////


    //init values

    double d_gain_pb=0.0;
    int N=INOUT.size();

    fftw_complex  *in   = new fftw_complex[N];
    fftw_complex  *out  = new fftw_complex[N];
    fftw_plan p,q;

    //Preapre complex in values
    for(int i=0;i<N;i++)
    {
        in[i][0] = INOUT[i];
        in[i][1] = 0;
    }

    //Forward DFT
    p = fftw_plan_dft_1d(N,in,out,FFTW_FORWARD,FFTW_ESTIMATE);

    fftw_execute(p);
    fftw_destroy_plan(p);


    //Apply butteerworth filter
    for ( int f = 0 ; f <N; f++ )
      {
          // w=2.pi.f ; wc = (w*Fe)/(fc*N) ; G4^2 = 1 / ( 1 + wc^(2*ordre) )
          //d_gain_pb = 1.0 / ( 1.0 + pow ( ( 2 * PI * f * d_frequence_echantillonnage ) / ( d_frequence_coupure_pb * ul_nb_donnees ) , 2 * ui_ordre ) ) ;
          d_gain_pb = 1.0 / (sqrt(1.0 + pow ( ( 2 * PI * f * Fe ) / (Fc* N) , 2 * ui_ordre ) )) ;

          out [ f ] [ 0 ] = out [ f ] [ 0 ] * d_gain_pb ;
          out [ f ] [ 1 ] = out [ f ] [ 1 ] * d_gain_pb ;
      }



    q = fftw_plan_dft_1d(N, out, in,FFTW_BACKWARD,FFTW_ESTIMATE);

    fftw_execute(q);
    fftw_destroy_plan(q);

    //Normalize
    for (int i = 0; i < N; i++)
    {
        INOUT[i]=in[i][0]/N;
    }

    fftw_free(in);
    fftw_free(out);

}


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



void Auto_Process_Filtering_whithout_redress(const QVector<float> &x, QVector<float> &Low_pass_data,
                            const fftwf_plan &p, const fftwf_plan &q, const QVector<float> &filterLowPass)
{
    if(filterLowPass.size()!=Low_pass_data.size() )
    {
        qDebug()<<"Error in filter and data size";
        return;
    }

    //init values
    int N                    = Low_pass_data.size();
    fftwf_complex  *in       = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex)*N);
    fftwf_complex  *out      = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex)*N);

    //Redressement des données
    vector<float> _p;
    if(polyfit(x,Low_pass_data,1,_p)==0)
    {
        qDebug()<<"[Polyfit] error";
    }


    for(int i=0;i<N;i++)
    {
        in[i][0]    = Low_pass_data[i]-(_p[0]);
        in[i][1]    = 0;
    }

    ////////////////////////////////////////////////////////////////////////
    ///////////////////////Fitrage passe bas spectral///////////////////////
    ////////////////////////////////////////////////////////////////////////


    //Forward DFT
    fftwf_execute_dft(p,in,out);

    //Apply Low pass Filter and Band pass filter
    for ( int f = 0 ; f <N; f++ )
    {
        out [ f ] [ 0 ]     = out [ f ] [ 0 ] * filterLowPass[f] ;
        out [ f ] [ 1 ]     = out [ f ] [ 1 ] * filterLowPass[f] ;
    }

    //Inverse DFT
    fftwf_execute_dft(q,out,in);

    //Normalize
    for (int i = 0; i < N; i++)
    {
        Low_pass_data[i]    = in[i][0]/N + _p[0];
    }

    fftwf_free(in);
    fftwf_free(out);
}




void Auto_Process_Filtering(const QVector<float> &x, QVector<float> &Low_pass_data,
                            const fftwf_plan &p, const fftwf_plan &q, const QVector<float> &filterLowPass)
{
    if(filterLowPass.size()!=Low_pass_data.size() )
    {
        qDebug()<<"Error in filter and data size";
        return;
    }

    //init values
    int N                    = Low_pass_data.size();
    fftwf_complex  *in       = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex)*N);
    fftwf_complex  *out      = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex)*N);

    //Redressement des données
    vector<float> _p;
    if(polyfit(x,Low_pass_data,1,_p)==0)
    {
        qDebug()<<"[Polyfit] error";
    }


    for(int i=0;i<N;i++)
    {
        in[i][0]    = Low_pass_data[i]-(_p[1]*x[i] + _p[0]);
        in[i][1]    = 0;
    }

    ////////////////////////////////////////////////////////////////////////
    ///////////////////////Low pass filtering///////////////////////////////
    ////////////////////////////////////////////////////////////////////////


    //Forward DFT
    fftwf_execute_dft(p,in,out);

    //Apply Low pass Filter and Band pass filter
    for ( int f = 0 ; f <N; f++ )
    {
        out [ f ] [ 0 ]     = out [ f ] [ 0 ] * filterLowPass[f] ;
        out [ f ] [ 1 ]     = out [ f ] [ 1 ] * filterLowPass[f] ;
    }

    //Inverse DFT
    fftwf_execute_dft(q,out,in);

    //Normalize
    for (int i = 0; i < N; i++)
        Low_pass_data[i]    = in[i][0]/N + _p[0];



    fftwf_free(in);
    fftwf_free(out);
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





float Singleconvolve(const QVector<float> &v1,const QVector<float> &v2)
{
    float out =0.0f;
//    int lenA = v1.size();
    int lenB = v2.size();


    int i1 = v1.size()-1;

    for (int j=0;j<lenB;j++)
    {
//        if (i1>=0 && i1<lenA)
        out = out + (v1[i1]*v2[j]);

        i1 = i1 -1;
    }
    return out;

}





QVector<float> get_Cardia_Filter(float half_width,float mean_val,float F_min,float dF,int length)
{


    //x vec
    QVector<float> x;
    for(int i=0;i<2*mean_val;i++)
        x.push_back(i);

    QVector<float> theo_filter = get_Gaussian(half_width,mean_val,x);

    theo_filter = theo_filter/(float)get_Max(theo_filter);

//    for(int i=theo_filter.size();i<int((F_max-F_min)/dF);i++)
//        theo_filter.push_back(0);
    for(int i=0;i<int(F_min/dF);i++)
        theo_filter.push_front(0);
    for(int i=theo_filter.size();i<length;i++)
        theo_filter.push_back(0);
    for(int i=0;i<(int)(theo_filter.size()/2);i++)
        theo_filter[theo_filter.size()-1-i] = theo_filter[i];

    return theo_filter;
}








// Hillbert transform
fftwf_complex* hilbert(const QVector<float> &data, fftwf_plan plan_forward, fftwf_plan plan_backward)
{
    // Get in array size
    int N = data.size();

    //init out
    fftwf_complex  *in      = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex)*N);
    fftwf_complex  *out      = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex)*N);



//    // copy the data to the complex array
//    for (int i = 0; i < N; ++i) {
//        out[i][REAL] = in[i];
//        out[i][IMAG] = 0;
//    }
    for(int i=0;i<N;i++)
    {
        in[i][0]    = data[i];
        in[i][1]    = 0;
    }


    // creat a DFT plan and execute it
    //fftw_plan plan = fftw_plan_dft_1d(N, out, out, FFTW_FORWARD, FFTW_ESTIMATE);


//    fftwf_execute(plan_forward);
    fftwf_execute_dft(plan_forward,in,out);

    // destroy a plan to prevent memory leak
    //fftw_destroy_plan(plan_forward);
    int hN = N >> 1; // half of the length (N/2)
    int numRem = hN; // the number of remaining elements

    // multiply the appropriate value by 2
    //(those should multiplied by 1 are left intact because they wouldn't change)
    for (int i = 1; i < hN; ++i) {
        out[i][0] *= 2;
        out[i][1] *= 2;
    }

    // if the length is even, the number of the remaining elements decrease by 1
    if (N % 2 == 0)
        numRem--;
    else if (N > 1) {
        out[hN][0] *= 2;
        out[hN][1] *= 2;
    }

    // set the remaining value to 0
    // (multiplying by 0 gives 0, so we don't care about the multiplicands)
    memset(&out[hN + 1][0], 0, numRem * sizeof(fftwf_complex));

    // creat a IDFT plan and execute it
    //plan = fftw_plan_dft_1d(N, out, out, FFTW_BACKWARD, FFTW_ESTIMATE);


//    fftwf_execute(plan_backward);
    fftwf_execute_dft(plan_backward,out,in);

    // do some cleaning
    //fftw_destroy_plan(plan_backward);
    //fftw_cleanup();
    // scale the IDFT output
    for (int i = 0; i < N; ++i) {
        in[i][0] /= N;
        in[i][1] /= N;
    }


    fftwf_free(out);

    return in;


}

/** Unwrap angle from interval [−π; π] to [0; 2π] */
QVector<float> unwrap(QVector<float> phase)
{
//    float unwrappedPhase = phase;
//    float previousPhase = phase;

//    while (unwrappedPhase - previousPhase < -PI) {
//        unwrappedPhase += 2 * PI;
//    }

//    while (unwrappedPhase - previousPhase > PI) {
//        unwrappedPhase -= 2 * PI;
//    }

//    return unwrappedPhase;

    QVector<float> unwrappedPhase(phase.size());
    unwrappedPhase[0] = phase[0];

    for (int i = 1; i < phase.size(); ++i)
    {
        double phaseDiff = phase[i] - phase[i - 1];
        if (phaseDiff < -PI)
            phaseDiff += 2 * PI;
        else if (phaseDiff > PI)
            phaseDiff -= 2 * PI;
        unwrappedPhase[i] = unwrappedPhase[i - 1] + phaseDiff;
    }
    return unwrappedPhase;
}


//Get angle of complex number  in the interval [−π; π]
QVector<float> get_unwrapped_angle(fftwf_complex  *in, int N)
{
//    //init output
//    QVector<float> output;

//    //Convert into std complex
//    std::complex<float> *in_std;
//    in_std = reinterpret_cast<std::complex<float> *>(in);

//    // Compute angle of in_std complex in the interval [−π; π].
//    for (int i=0;i<N;i++)
//        output.push_back(unwrap(std::arg(in_std[i])));

    QVector<float> unwrappedPhase(N);
    //Convert into std complex
    std::complex<float> *in_std;
    in_std = reinterpret_cast<std::complex<float> *>(in);

    unwrappedPhase[0] = std::arg(in_std[0]);

    for (std::size_t i = 1; i < N; ++i) {
        double phaseDiff = std::arg(in_std[i]) - std::arg(in_std[i - 1]);
        if (phaseDiff < -PI)
            phaseDiff += 2 * PI;
        else if (phaseDiff > PI)
            phaseDiff -= 2 * PI;
        unwrappedPhase[i] = unwrappedPhase[i - 1] + phaseDiff;
    }

    return unwrappedPhase;
}




std::vector<double> unwrapPhase(const std::vector<std::complex<double>>& signal)
{
    std::vector<double> unwrappedPhase(signal.size());
    unwrappedPhase[0] = std::arg(signal[0]);

    for (std::size_t i = 1; i < signal.size(); ++i) {
        double phaseDiff = std::arg(signal[i]) - std::arg(signal[i - 1]);
        if (phaseDiff < -PI)
            phaseDiff += 2 * PI;
        else if (phaseDiff > PI)
            phaseDiff -= 2 * PI;
        unwrappedPhase[i] = unwrappedPhase[i - 1] + phaseDiff;
    }

    return unwrappedPhase;
}








QVector<float> calculatePhaseShift_Hilbert(const QVector<float> in1, const QVector<float> in2,fftwf_plan plan_forward, fftwf_plan plan_backward)
{
    //Compute hillbert transform
    fftwf_complex  *in1_hil = hilbert(in1,plan_forward, plan_backward);
    fftwf_complex  *in2_hil = hilbert(in2,plan_forward, plan_backward);

    //get angle
    QVector<float> angle1 = get_unwrapped_angle(in1_hil,in1.size());
    QVector<float> angle2 = get_unwrapped_angle(in2_hil,in2.size());

    //phase diff
    for (int i=0;i<angle1.size();i++)
        angle1[i] = angle1[i] - angle2[i];

    //unwrap phase
    QVector<float> phase = unwrap(angle1);

    //Free complex numbers
    fftwf_free(in1_hil);
    fftwf_free(in2_hil);

    return phase;
}


QVector<float> calculate_correlation_Phase(const QVector<float> in1, const QVector<float> in2,fftwf_plan plan_forward, fftwf_plan plan_backward)
{
    //Compute hillbert transform
    fftwf_complex  *in1_hil = hilbert(in1,plan_forward, plan_backward);
    fftwf_complex  *in2_hil = hilbert(in2,plan_forward, plan_backward);

    //Convert into std complex
    std::complex<float> *in1_std;
    std::complex<float> *in2_std;

    in1_std = reinterpret_cast<std::complex<float> *>(in1_hil);
    in2_std = reinterpret_cast<std::complex<float> *>(in2_hil);

    //Compute cross transform
    //    ctc=c1.*conj(c2);     %Cross wavelet transform
    //    spt = atan2(imag(ctc),real(ctc)); %phase
    //    hpc_signal=cos(spt); %HPC signal complete
    QVector<float> hpc;
    for(int i=0;i<in1.size();i++)
    {
        //cross hilbert
        std::complex<float> cross_h = in1_std[i]*conj(in2_std[i]);
        hpc.push_back(cos((float)atan2((double)imag(cross_h),(double)real(cross_h))));
    }

    return hpc;
}




QVector<float> CrossCorrelation(QVector<float>& signal1,QVector<float>& signal2)
{
    const int size = signal1.size();

    // Create FFT plans
    fftwf_complex* fft_signal1 = reinterpret_cast<fftwf_complex*>(fftw_malloc(sizeof(fftwf_complex) * size));
    fftwf_complex* fft_signal2 = reinterpret_cast<fftwf_complex*>(fftw_malloc(sizeof(fftwf_complex) * size));
    fftwf_plan fft_plan1 = fftwf_plan_dft_r2c_1d(size, signal1.data(), fft_signal1, FFTW_ESTIMATE);
    fftwf_plan fft_plan2 = fftwf_plan_dft_r2c_1d(size, signal2.data(), fft_signal2, FFTW_ESTIMATE);

    // Compute Fourier transform
    fftwf_execute(fft_plan1);
    fftwf_execute(fft_plan2);

    // Compute cross correlation in Fourier domain
    for (int i = 0; i < size / 2 + 1; ++i) {
        double real1 = fft_signal1[i][0];
        double imag1 = fft_signal1[i][1];
        double real2 = fft_signal2[i][0];
        double imag2 = fft_signal2[i][1];
        fft_signal1[i][0] = real1 * real2 + imag1 * imag2;
        fft_signal1[i][1] = imag1 * real2 - real1 * imag2;
    }

    // Get back in temporal domain
    QVector<float> crossCorrelation(size);
    fftwf_plan ifft_plan = fftwf_plan_dft_c2r_1d(size, fft_signal1, crossCorrelation.data(), FFTW_ESTIMATE);
    fftwf_execute(ifft_plan);

    //Normalize cross correlation
    crossCorrelation= crossCorrelation/size;

    // center time
    QVector<float> shiftedCrossCorrelation(size);
    const int shift = size / 2;
    for (int i = 0; i < size; ++i)
    {
        shiftedCrossCorrelation[i] = crossCorrelation[(i + shift) % size];
    }

    // Free plans and complex arrays
    fftwf_destroy_plan(fft_plan1);
    fftwf_destroy_plan(fft_plan2);
    fftwf_destroy_plan(ifft_plan);
    fftwf_free(fft_signal1);
    fftwf_free(fft_signal2);

    return shiftedCrossCorrelation;
}


double calculatePhaseShift_xCorr(QVector<float>& signal1,QVector<float>& signal2)
{
    //Compute cross correlation
    QVector<float> cross_corr = CrossCorrelation(signal1,signal2);

    //Convert to std vector
    std::vector<float> v(cross_corr. constBegin(), cross_corr.constEnd());

    // Get max index of cross correlation
    int max_index = argMax(v);


    // Get temporal phase difference (in sample)
    int phase_shift_samples = signal1.size() - max_index - 1;

//    // Calcul de la période d'échantillonnage
//    float sampling_period = 1/_M_;  // Modifier si nécessaire

//    // Calcul du déphasage en temps (en secondes)
//    float phase_shift_time = phase_shift_samples * sampling_period;


    return phase_shift_samples;
}











