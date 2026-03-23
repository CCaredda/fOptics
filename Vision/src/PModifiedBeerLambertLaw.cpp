#include "PModifiedBeerLambertLaw.h"

PModifiedBeerLambertLaw::PModifiedBeerLambertLaw(QObject *parent) : QObject(parent)
{

    _M_start_ref        = 0;
    _M_end_ref          = 0;
    _M_start_activity   = 0;
    _M_end_activity     = 0;
    _M_start_correlation= 0;
    _M_end_correlation  = 0;
    _M_wavelength_IDX.clear();
    _M_coeff_multi.clear();
    _M_coeff_multi.fill(1,4);

    //Indexes last and first rest steps (for data correction)
    _M_start_first_rest = 0;
    _M_end_first_rest = 0;
    _M_start_last_rest = 0;
    _M_end_last_rest = 0;

}

PModifiedBeerLambertLaw::~PModifiedBeerLambertLaw()
{

}

void PModifiedBeerLambertLaw::setWavelengthIDX(QVector<QVector<int> > wavelength_IDX)
{
    qDebug()<<"PModifiedBeerLambertLaw::setWavelengthIDX";
    _M_wavelength_IDX.clear();
    for(int i=0;i<wavelength_IDX.size();i++)
        _M_wavelength_IDX.push_back(wavelength_IDX[i]);
}

void PModifiedBeerLambertLaw::onnewWavelengthIDX(QVector<QVector<int> > wavelength_IDX)
{
    qDebug()<<"PModifiedBeerLambertLaw::onnewWavelengthIDX";
    _M_wavelength_IDX.clear();
    for(int i=0;i<wavelength_IDX.size();i++)
        _M_wavelength_IDX.push_back(wavelength_IDX[i]);
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


//    qDebug()<<"[ PModifiedBeerLambertLaw::getAveragedConcentrationChanges] start id: "<<_M_start_correlation;
//    qDebug()<<"[ PModifiedBeerLambertLaw::getAveragedConcentrationChanges] end id: "<<_M_end_correlation;
//    qDebug()<<"[ PModifiedBeerLambertLaw::getAveragedConcentrationChanges] start ref: "<<_M_start_ref;
//    qDebug()<<"[ PModifiedBeerLambertLaw::getAveragedConcentrationChanges] end ref: "<<_M_end_ref;


//    // Apply data correction
//    QVector<QVector<float> > corrected_temporal_vec;
//    for(int k=0;k<temporal_vec.size();k++)
//        corrected_temporal_vec.push_back(Apply_Data_Correction(temporal_vec[k],start_id,end_id,_M_start_first_rest,_M_end_first_rest,_M_start_last_rest,_M_end_last_rest));

    if(coeff.size()==1)
        _Single_Deconvolution(temporal_vec,coeff[0],results,start_act,end_act,start_ref,end_ref);
    else
        _Multiple_Deconvolution(temporal_vec,coeff,results,start_act,end_act,start_ref,end_ref);
}

/*****************************************************
***************get absorbance*************************
*************** **************************************/
void PModifiedBeerLambertLaw::getAbsorbancechanges(const QVector<QVector<float> > &temporal_vec,int start_ref,int end_ref,int start_id,int end_id,QVector<QVector<float> > &absorbance)
{
    //Init
    int NbChannels  = temporal_vec.size();
    absorbance.clear();

    QVector<float> mean_val;
    mean_val.fill(0,NbChannels);


//    // Apply data correction
//    QVector<QVector<float> > corrected_temporal_vec;
//    for(int k=0;k<temporal_vec.size();k++)
//        corrected_temporal_vec.push_back(Apply_Data_Correction(temporal_vec[k],start_id,end_id,_M_start_first_rest,_M_end_first_rest,_M_start_last_rest,_M_end_last_rest));


    if(!getReference_Single_Deconvolution(temporal_vec,start_ref,end_ref,NbChannels,mean_val))
        return;

    //Get absorbance
    for(int i=start_id;i<=end_id;i++)
    {
        QVector<float> temp;
        for(int k=0;k<NbChannels;k++)
            temp.push_back((temporal_vec[k][i]==0) ? 0 :  log10((mean_val[k])/(temporal_vec[k][i])));
        absorbance.push_back(temp);
    }

//    if(!getReference_Single_Deconvolution(corrected_temporal_vec,start_ref,end_ref,NbChannels,mean_val))
//        return;

//    //Get absorbance
//    for(int i=start_id;i<=end_id;i++)
//    {
//        QVector<float> temp;
//        for(int k=0;k<NbChannels;k++)
//            temp.push_back((corrected_temporal_vec[k][i]==0) ? 0 :  log10((mean_val[k])/(corrected_temporal_vec[k][i])));
//        absorbance.push_back(temp);
//    }
}


////Mean concentration
//void PModifiedBeerLambertLaw::_Multiple_Deconvolution(const QVector<QVector<float> > &temporal_vec, const QVector<Mat> &coeff,const QVector<int> &mask, QVector<float> &results)
//{
//  // Init
//  int nb_chromophore = _M_wavelength_IDX.size();

//  //Check if size corresponds
//  if(coeff.size()!=nb_chromophore)
//  {
//      qDebug()<<"[PModifiedBeerLambertLaw multiple deconvolution] coeff size is wrong, size: "<<coeff.size()<<" expected: "<<nb_chromophore;
//      return;
//  }

//  if(mask.size()!=nb_chromophore+1)
//  {
//      qDebug()<<"[PModifiedBeerLambertLaw multiple deconvolution] mask wrong size, size: "<<mask.size()<<" expected: "<<nb_chromophore+1;
//      return;
//  }

//  for(int i=0;i<nb_chromophore;i++)
//  {
//      if(coeff[i].cols!=_M_wavelength_IDX[i].size() || coeff[i].rows != nb_chromophore)
//      {
//          qDebug()<<"[PModifiedBeerLambertLaw multiple deconvolution] Wrong matrix size";
//          qDebug()<<"col : "<<coeff[i].cols<<" expected : "<<_M_wavelength_IDX[i].size();
//          qDebug()<<"row : "<<coeff[i].rows<<" expected : "<<nb_chromophore;

//          return ;
//      }
//  }

//  QVector<float> contrast;
//  contrast.fill(0,nb_chromophore);

//  QVector<QVector<float> > OD;
//  OD.resize(nb_chromophore);
//  for(int i=0;i<nb_chromophore;i++)
//    OD[i].fill(0,_M_wavelength_IDX[i].size());

//  QVector<QVector<float> > mean_val;
//  mean_val.resize(nb_chromophore);
//  for(int i=0;i<nb_chromophore;i++)
//    mean_val[i].fill(0,_M_wavelength_IDX[i].size());

//  int tot=0;

//  //Init results
//  results.clear();
//  results.fill(0,nb_chromophore+1);


//  /////////////////////////////////////
//  //Get reference
//  /////////////////////////////////////

//  if(!getReference_Mutliple_Deconvolution(temporal_vec,_M_start_ref,_M_end_ref,nb_chromophore,_M_wavelength_IDX,mean_val))
//      return;

//  /////////////////////////////////////
//  //Get chromophore concentration changes averaged over the patient activity period
//  /////////////////////////////////////

//  for(int t=_M_start_activity;t<=_M_end_activity;t++)
//  {
//      //Process MBLL
//      process_Multiple_Deconvolution(temporal_vec,t,OD,mean_val,nb_chromophore,_M_wavelength_IDX,coeff,contrast);

//      //HbO2, Hb (oxCCO)
//      for(int mes=0;mes<nb_chromophore;mes++)
//          results[mes] += contrast[mes];

//      tot++;
//  }



//  if(tot==0)
//      return;

//  for(int mes=0;mes<nb_chromophore;mes++)
//      results[mes] = (mask[mes]==1) ? results[mes]/tot : 0;

//  results[nb_chromophore] = (mask[nb_chromophore]==1) ? results[0]+results[1] : 0;
//}

void PModifiedBeerLambertLaw::_Multiple_Deconvolution(const QVector<QVector<float> > &temporal_vec, const QVector<Mat> &coeff, QVector<float> &results, int start_activity, int end_activity, int start_ref, int end_ref)
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

    //init
  int nb_chromophore = _M_wavelength_IDX.size();

  //Check if size corresponds
  if(coeff.size()!=nb_chromophore)
  {
      qDebug()<<"[PModifiedBeerLambertLaw multiple deconvolution] coeff size is wrong, size: "<<coeff.size()<<" expected: "<<nb_chromophore;
      return;
  }


  for(int i=0;i<nb_chromophore;i++)
  {
      if(coeff[i].cols!=_M_wavelength_IDX[i].size() || coeff[i].rows != nb_chromophore)
      {
          qDebug()<<"[PModifiedBeerLambertLaw multiple deconvolution] Wrong matrix size";
          qDebug()<<"col : "<<coeff[i].cols<<" expected : "<<_M_wavelength_IDX[i].size();
          qDebug()<<"row : "<<coeff[i].rows<<" expected : "<<nb_chromophore;

          return ;
      }
  }

  QVector<float> contrast;
  contrast.fill(0,nb_chromophore);

  QVector<QVector<float> > OD;
  OD.resize(nb_chromophore);
  for(int i=0;i<nb_chromophore;i++)
    OD[i].fill(0,_M_wavelength_IDX[i].size());

  QVector<QVector<float> > mean_val;
  mean_val.resize(nb_chromophore);
  for(int i=0;i<nb_chromophore;i++)
    mean_val[i].fill(0,_M_wavelength_IDX[i].size());

  int tot=0;

  //Init results
  results.clear();
  results.fill(0,nb_chromophore+1);


  /////////////////////////////////////
  //Get reference
  /////////////////////////////////////

  if(!getReference_Mutliple_Deconvolution(temporal_vec,start_ref,end_ref,nb_chromophore,_M_wavelength_IDX,mean_val))
      return;

  /////////////////////////////////////
  //Get chromophore concentration changes averaged over the patient activity period
  /////////////////////////////////////

  for(int t=start_activity;t<=end_activity;t++)
  {
      //Process MBLL
      process_Multiple_Deconvolution(temporal_vec,t,OD,mean_val,nb_chromophore,_M_wavelength_IDX,coeff,contrast);

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


//void PModifiedBeerLambertLaw::_Single_Deconvolution(const QVector<QVector<float> > &temporal_vec, const Mat &coeff,const QVector<int> &mask, QVector<float> &results)
//{
//  // Init
//  int nb_chromophore    = coeff.rows;
//  int NbChannels        = temporal_vec.size();

//  if(mask.size()!=nb_chromophore+1)
//  {
//      qDebug()<<"[PModifiedBeerLambertLaw Single deconvolution] mask wrong size, size: "<<mask.size()<<" expected: "<<nb_chromophore+1;
//      return;
//  }

//  if(coeff.cols!=NbChannels || coeff.rows != nb_chromophore)
//  {
//      qDebug()<<"[Mean_Coh_Cod_Measure] matrice coeff de mauvaise taille";
//      qDebug()<<"col : "<<coeff.cols<<" expected : "<<NbChannels;
//      qDebug()<<"row : "<<coeff.rows<<" expected : "<<nb_chromophore;

//      return ;
//  }

//  QVector<float> contrast;
//  contrast.fill(0,nb_chromophore);

//  QVector<float> OD;
//  OD.fill(0,NbChannels);

//  QVector<float> mean_val;
//  mean_val.fill(0,NbChannels);

//  int tot=0;

//  //Init results (HbO2,Hb,(oxCCO),HbT)
//  results.clear();
//  results.fill(0,nb_chromophore+1);



//  /////////////////////////////////////
//  //Get reference
//  /////////////////////////////////////

//  if(!getReference_Single_Deconvolution(temporal_vec,_M_start_ref,_M_end_ref,NbChannels,mean_val))
//      return;



//  /////////////////////////////////////
//  //Process time of interest Cod and Coh
//  /////////////////////////////////////

//  for(int i=_M_start_activity;i<=_M_end_activity;i++)
//  {
//      //Process MBLL
//      process_Single_Deconvolution(temporal_vec,i,OD,mean_val,NbChannels,nb_chromophore,coeff,contrast);



//      //HbO2, Hb (oxCCO)
//      for(int mes=0;mes<nb_chromophore;mes++)
//          results[mes] += contrast[mes];

//      tot++;
//  }

//  if(tot==0)
//      return;

//  for(int mes=0;mes<nb_chromophore;mes++)
//      results[mes] = (mask[mes]==1) ? results[mes]/tot : 0;

//  results[nb_chromophore] = (mask[nb_chromophore]==1) ? results[0]+results[1] : 0;
//}

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

//    // Apply data correction
//    QVector<QVector<float> > corrected_temporal_vec;
//    for(int k=0;k<temporal_vec.size();k++)
//        corrected_temporal_vec.push_back(Apply_Data_Correction(temporal_vec[k],start_id,end_id,_M_start_first_rest,_M_end_first_rest,_M_start_last_rest,_M_end_last_rest));


    if(coeff.size()==1)
        _Single_Deconvolution_Time_courses(temporal_vec,coeff[0],contrast,start_ref,end_ref,start_id,end_id);
    else
        _Multiple_Deconvolution_Time_courses(temporal_vec,coeff,contrast,start_ref,end_ref,start_id,end_id);


//    if(coeff.size()==1)
//        _Single_Deconvolution_Time_courses(corrected_temporal_vec,coeff[0],contrast,start_ref,end_ref,start_id,end_id);
//    else
//        _Multiple_Deconvolution_Time_courses(corrected_temporal_vec,coeff,contrast,start_ref,end_ref,start_id,end_id);
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
void PModifiedBeerLambertLaw::_Multiple_Deconvolution_Time_courses(const QVector<QVector<float> > &temporal_vec, const QVector<Mat> &coeff, QVector<QVector<float> > &contrast, int start_ref, int end_ref, int start_id, int end_id)
{
    //Clear vectors
    int nb_chromophore = _M_wavelength_IDX.size();

    //Check if size corresponds
    if(coeff.size()!=nb_chromophore)
    {
        qDebug()<<"[PModifiedBeerLambertLaw::_Multiple_Deconvolution_Time_courses] coeff size is wrong, size: "<<coeff.size()<<" expected: "<<nb_chromophore;
        return;
    }

    for(int i=0;i<nb_chromophore;i++)
    {
        if(coeff[i].cols!=_M_wavelength_IDX[i].size() || coeff[i].rows != nb_chromophore)
        {
            qDebug()<<"[PModifiedBeerLambertLaw::_Multiple_Deconvolution_Time_courses] matrice coeff de mauvaise taille";
            qDebug()<<"col : "<<coeff[i].cols<<" expected : "<<_M_wavelength_IDX[i].size();
            qDebug()<<"row : "<<coeff[i].rows<<" expected : "<<nb_chromophore;
            return ;
        }
    }


    contrast.clear();
    contrast.resize(nb_chromophore+1);

    //measure
    QVector<float> measure;
    measure.fill(0,nb_chromophore);

    QVector<QVector<float> > OD;
    OD.resize(nb_chromophore);
    for(int i=0;i<nb_chromophore;i++)
      OD[i].fill(0,_M_wavelength_IDX[i].size());

    QVector<QVector<float> > mean_val;
    mean_val.resize(nb_chromophore);
    for(int i=0;i<nb_chromophore;i++)
      mean_val[i].fill(0,_M_wavelength_IDX[i].size());



    /////////////////////////////////////
    //Get reference
    /////////////////////////////////////

    if(!getReference_Mutliple_Deconvolution(temporal_vec,start_ref,end_ref,nb_chromophore,_M_wavelength_IDX,mean_val))
        return;


    /////////////////////
    //Process Ref Cod Coh
    ////////////////////

    for(int i=start_id;i<=end_id;i++)
    {
        //Process MBLL
        process_Multiple_Deconvolution(temporal_vec,i,OD,mean_val,nb_chromophore,_M_wavelength_IDX,coeff,measure);


        for(int mes=0;mes<nb_chromophore;mes++)
            contrast[mes].push_back(measure[mes]);

        //Hb tot
        contrast[nb_chromophore].push_back(measure[0]+measure[1]);
    }
}


