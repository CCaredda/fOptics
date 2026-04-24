#include "oxygenation.h"


bool getReference_Single_Deconvolution(const QVector<QVector<float> > &temporal_vec,int start,int end,
                                       int NbChannels,QVector<float> &mean_val)
{
    int tot = 0;
    /////////////////////////////////////
    //Get reference
    /////////////////////////////////////

    // Calcul mean reference signal for each wavelength
    for(int i=start;i<=end;i++)
    {
        for(int k=0;k<NbChannels;k++)
            mean_val[k]+=temporal_vec[k][i];
        tot++;
    }
    if(tot==0)
        return false;

    for(int k=0;k<NbChannels;k++)
        mean_val[k]/=tot;

    return true;
}


void process_Single_Deconvolution(const QVector<QVector<float> > &temporal_vec,int t,QVector<float> &OD,
                                  QVector<float> &mean_val,int NbChannels,int nb_chromophore,
                                  const Mat &Molar_coeff,QVector<float> &measure)
{
    //process Optical density
    for(int k=0;k<NbChannels;k++)
        OD[k] = (temporal_vec[k][t]==0) ? 0 :  log10((mean_val[k])/(temporal_vec[k][t]));

    //process Concentration changes
    for(int mes=0;mes<nb_chromophore;mes++)
        measure[mes] = 0.0f;

    for(int k=0;k<NbChannels;k++)
    {
        for(int mes=0;mes<nb_chromophore;mes++)
            measure[mes] += Molar_coeff.at<double>(mes,k)*OD[k];
    }

    //Control values
    for(int mes=0;mes<nb_chromophore;mes++)
    {
        if(std::isinf(measure[mes])   || std::isnan(measure[mes]))
            measure[mes] = 0.0f;

    }
}


/*****************************************************
***************Mesure probabiliste********************
*************** **************************************/

// Compare bold vs contrast measures
void Compare_Bold_Vs_Oxy(QVector<QVector<float> > &contrast,QVector<QVector<float> > &model, QVector<double> &stat_vec/*,double proba_thresh*/)
{
    stat_vec.fill(0,contrast.size());
    // Get probability density
    for(int i=0;i<contrast.size();i++)
    {

        // process pearson correlation coeff between Model and mesured concentration density probability
        double dist = pearsoncoeff(contrast[i],model[i]);

        if(std::isnan(dist) || std::isinf(dist))
        {
            dist=0;
        }
        stat_vec[i] = dist;
    }
}


void Compare_Bold_Vs_studied_Point(const QVector<QVector<float> > &contrast,const QVector<float> &model,QVector<double> &stat_vec/*,double proba_thresh*/)
{
    stat_vec.fill(0,contrast.size());
    // Get probability density
    for(int i=0;i<contrast.size();i++)
    {
        QVector<float> density2;


        // Set mutliplier coeff to -1
        //QVector<float> contrast2;
        for(int j=0;j<contrast[i].size();j++)
        {
            density2.push_back(-contrast[i][j]);
        }



        // process pearson correlation coeff between Model and mesured concentration density probability
        double dist1 = pearsoncoeff(contrast[i],model);
        double dist2 = pearsoncoeff(density2,model);

        if(std::isnan(dist1))
            dist1=0;
        if(std::isnan(dist2))
            dist2=0;

        double dist = (dist1 > dist2) ? dist1 : dist2;

        stat_vec[i] = dist;
    }
}


