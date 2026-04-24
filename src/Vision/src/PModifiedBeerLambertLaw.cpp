#include "PModifiedBeerLambertLaw.h"

PModifiedBeerLambertLaw::PModifiedBeerLambertLaw(QObject *parent) : QObject(parent)
{

    _M_start_ref        = 0;
    _M_end_ref          = 0;
    _M_start_activity   = 0;
    _M_end_activity     = 0;
    _M_start_correlation= 0;
    _M_end_correlation  = 0;


    //Indexes last and first rest steps (for data correction)
    _M_start_first_rest = 0;
    _M_end_first_rest = 0;
    _M_start_last_rest = 0;
    _M_end_last_rest = 0;

}

PModifiedBeerLambertLaw::~PModifiedBeerLambertLaw()
{

}



void PModifiedBeerLambertLaw::getAveragedConcentrationChanges(const QVector<QVector<float> > &temporal_vec,const QVector<Mat> &coeff, QVector<float> &results,int start_act,int end_act,
                                                              int start_ref, int end_ref, int start_id, int end_id)
{
    //Init idx if no values written in function
    if(start_id == -1)
        start_id = _M_start_correlation;

    if(end_id == -1)
        end_id = _M_end_correlation;

    if(start_ref == -1)
        start_ref = _M_start_ref;

    if(end_ref == -1)
        end_ref = _M_end_ref;



    _Single_Deconvolution(temporal_vec,coeff[0],results,start_act,end_act,start_ref,end_ref);

}



void PModifiedBeerLambertLaw::_Single_Deconvolution(const QVector<QVector<float> > &temporal_vec, const Mat &coeff, QVector<float> &results, int start_activity, int end_activity, int start_ref, int end_ref)
{
    //Init start and end activity idx if no values written in function
    if(start_activity == -1)
        start_activity = _M_start_activity;

    if(end_activity == -1)
        end_activity = _M_end_activity;

    if(start_ref == -1)
        start_ref = _M_start_ref;

    if(end_ref == -1)
        end_ref = _M_end_ref;

  // Init
  int nb_chromophore    = coeff.rows;
  int NbChannels        = temporal_vec.size();

  if(coeff.cols!=NbChannels || coeff.rows != nb_chromophore)
  {
      qDebug()<<"[Mean_Coh_Cod_Measure] matrice coeff de mauvaise taille";
      qDebug()<<"col : "<<coeff.cols<<" expected : "<<NbChannels;
      qDebug()<<"row : "<<coeff.rows<<" expected : "<<nb_chromophore;

      return ;
  }

  QVector<float> contrast;
  contrast.fill(0,nb_chromophore);

  QVector<float> OD;
  OD.fill(0,NbChannels);

  QVector<float> mean_val;
  mean_val.fill(0,NbChannels);

  int tot=0;

  //Init results (HbO2,Hb,(oxCCO),HbT)
  results.clear();
  results.fill(0,nb_chromophore+1);


  /////////////////////////////////////
  //Get reference
  /////////////////////////////////////

  if(!getReference_Single_Deconvolution(temporal_vec,start_ref,end_ref,NbChannels,mean_val))
      return;



  /////////////////////////////////////
  //Process time of interest Cod and Coh
  /////////////////////////////////////

  for(int i=start_activity;i<=end_activity;i++)
  {
      //Process MBLL
      process_Single_Deconvolution(temporal_vec,i,OD,mean_val,NbChannels,nb_chromophore,coeff,contrast);



      //HbO2, Hb (oxCCO)
      for(int mes=0;mes<nb_chromophore;mes++)
          results[mes] += contrast[mes];

      tot++;
  }

  if(tot==0)
      return;

  for(int mes=0;mes<nb_chromophore;mes++)
      results[mes] = results[mes]/tot;

  results[nb_chromophore] = results[0]+results[1];
}


//Time courses
void PModifiedBeerLambertLaw::get_Chromophore_Time_courses(const QVector<QVector<float> > &temporal_vec, const QVector<Mat> &coeff, QVector<QVector<float> > &contrast,
                                                           int start_ref, int end_ref, int start_id, int end_id)
{

    //Init idx if no values written in function
    if(start_id == -1)
        start_id = _M_start_correlation;

    if(end_id == -1)
        end_id = _M_end_correlation;

    if(start_ref == -1)
        start_ref = _M_start_ref;

    if(end_ref == -1)
        end_ref = _M_end_ref;


    _Single_Deconvolution_Time_courses(temporal_vec,coeff[0],contrast,start_ref,end_ref,start_id,end_id);

}


//Concentration changes time courses
void PModifiedBeerLambertLaw::_Single_Deconvolution_Time_courses(const QVector<QVector<float> > &temporal_vec, const Mat &coeff, QVector<QVector<float> > &contrast,int start_ref,int end_ref,int start_id,int end_id)
{
    //Init
    int nb_chromophore  = coeff.rows;
    int NbChannels      = temporal_vec.size();

    if(coeff.cols!=NbChannels || coeff.rows != nb_chromophore)
    {
        qDebug()<<"[Coh_Cod_Measure_SinglePoint] matrice coeff de mauvaise taille";
        qDebug()<<"nb Channels: "<<NbChannels<<" expected: "<<coeff.cols;
        qDebug()<<"Nb chromophore: "<<nb_chromophore<<" expected: "<<coeff.rows;

        return;
    }

    //Clear vectors
    contrast.clear();
    contrast.resize(nb_chromophore+1);

    //measure
    QVector<float> measure;
    measure.fill(0,nb_chromophore);

    QVector<float> OD;
    OD.fill(0,NbChannels);

    QVector<float> mean_val;
    mean_val.fill(0,NbChannels);


    /////////////////////////////////////
    //Get reference
    /////////////////////////////////////

    if(!getReference_Single_Deconvolution(temporal_vec,start_ref,end_ref,NbChannels,mean_val))
        return;


    /////////////////////
    //Process Ref Cod Coh
    ////////////////////

    for(int i=start_id;i<=end_id;i++)
    {
        //Process MBLL
        process_Single_Deconvolution(temporal_vec,i,OD,mean_val,NbChannels,nb_chromophore,coeff,measure);

        for(int mes=0;mes<nb_chromophore;mes++)
            contrast[mes].push_back(measure[mes]);

        //Hb tot
        contrast[nb_chromophore].push_back(measure[0]+measure[1]);
    }
}


