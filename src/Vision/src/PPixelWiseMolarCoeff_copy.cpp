#include "PPixelWiseMolarCoeff.h"
#include "molarcoefffunctions.h"
#include "acquisition.h"

PPixelWiseMolarCoeff::PPixelWiseMolarCoeff(QObject *parent) : QThread(parent)
{
    _M_mode_HyperSpectral = true;
    _M_quantify_oxCCO = false;
    _M_consider_blood_vessel_distance = true;

    _M_init_or_process = true; //true: init, false: process

    _M_reso_in_mm                       = -1;
    _M_vessel_lim_in_px                 = 6; // 1mm * 1 px/ 0.17718mm
    _M_area_lim_in_px                   = _M_vessel_lim_in_px*_M_vessel_lim_in_px;

    _M_eps_Hb_x_res_spectral.clear();
    _M_eps_HbO2_x_res_spectral.clear();
    _M_eps_oxCCO_x_res_spectral.clear();
    _M_Mean_path_Surface_Blood.clear();
    _M_Mean_path_Buried_Blood.clear();

    //img
    _M_img_Surface_blood_in             = Mat::zeros(0,0,CV_8UC1);
    _M_img_Buried_Blood_in              = Mat::zeros(0,0,CV_8UC1);
    _M_non_used                         = Mat::zeros(0,0,CV_8UC1);
    _M_img_Buried_Blood_preprocessed    = Mat::zeros(0,0,CV_8UC1);
    _M_img_Surface_blood_preprocessed   = Mat::zeros(0,0,CV_8UC1);
    _M_first_HS_img.clear();

    //Blood vessel contours
    _M_contours_SBV.clear();
    _M_contours_BBV.clear();

    //pixels pos
    _M_pixels_pos.clear();

    //Molar coeff
    _M_Molar_coeff.clear();
    _M_Molar_coeff_BuriedBlood.clear();
    _M_Molar_coeff_GM.clear();
    _M_Molar_coeff_SurfaceBlood.clear();

    //id of segmented elements
    _M_id_GM.clear();
    _M_id_BuriedBlood.clear();
    _M_id_SurfaceBlood.clear();

    //Hyperspectral camera config (combination of wavelength)
    _Init_Hyperspectral_wavelength_config();

    qDebug()<<"init data";
    _initData();
    qDebug()<<"end init data";


    //optimize HS quantification using optimal wavelength
    _M_optimize_HS_quantification = false;

}

PPixelWiseMolarCoeff::~PPixelWiseMolarCoeff()
{

}


//Get molar coeff
bool PPixelWiseMolarCoeff::getMolarCoeff(int id,QVector<Mat> &coeff)
{

    if(isEmptyMatrix(id))
        return false;


    coeff.clear();
    for(int i=0;i<_M_Molar_coeff[id].size();i++)
        coeff.push_back(_M_Molar_coeff[id][i]);

    return true;
}

//Hyperspectral camera config (combination of wavelength)
void PPixelWiseMolarCoeff::_Init_Hyperspectral_wavelength_config()
{
    _M_HbO2_Hb_oxCCO_HS_cam_idx.clear();
    _M_HbO2_Hb_HS_cam_idx.clear();

    load_Matrix(QString(PROPATH)+"/../files/HS_cam_idx/HbO2_Hb_oxCCO_wavelength_grp.txt",_M_HbO2_Hb_oxCCO_HS_cam_idx);
    load_Matrix(QString(PROPATH)+"/../files/HS_cam_idx/HbO2_Hb_wavelength_grp.txt",_M_HbO2_Hb_HS_cam_idx);

    if(_M_quantify_oxCCO)
        emit newHyperspectralWavelengthConfig(_M_HbO2_Hb_oxCCO_HS_cam_idx);
    else
        emit newHyperspectralWavelengthConfig(_M_HbO2_Hb_HS_cam_idx);
}

QVector<QVector<int> > PPixelWiseMolarCoeff::get_HS_optimal_cam_idx()
{
    if(_M_quantify_oxCCO)
        return _M_HbO2_Hb_oxCCO_HS_cam_idx;
    else
        return _M_HbO2_Hb_HS_cam_idx;

    return _M_HbO2_Hb_HS_cam_idx;
}


//Quantify oxCCO
void PPixelWiseMolarCoeff::Quantify_oxCCO(bool v)
{
    if(_M_mode_HyperSpectral)
        _M_quantify_oxCCO = v;
    else
        _M_quantify_oxCCO = false;

    if(_M_quantify_oxCCO)
        emit newHyperspectralWavelengthConfig(_M_HbO2_Hb_oxCCO_HS_cam_idx);
    else
        emit newHyperspectralWavelengthConfig(_M_HbO2_Hb_HS_cam_idx);

    initMolarCoeff();
}

//get number of chromophore quantified
int PPixelWiseMolarCoeff::getChromophoreNumber()
{
    if(_M_quantify_oxCCO)
        return 3;

    return 2;
}

//optimize HS quantification using optimal wavelength
void PPixelWiseMolarCoeff::setHSOptimizedQuantification(bool v)
{
    _M_optimize_HS_quantification = v;
    initMolarCoeff();
}


//Segmentation method
void PPixelWiseMolarCoeff::ConsiderBloodVesselDistance(bool v)
{
    _M_consider_blood_vessel_distance = v;

    if(!_M_img_Surface_blood_in.empty() && !_M_img_Buried_Blood_in.empty() && !_M_non_used.empty())
        this->start();
}

//Protected function run in a parallel thread
void PPixelWiseMolarCoeff::run()
{
    //init data
    _initData();

    //Preprocessing
    if(!_PreProcessing())
    {
        emit processFinished();
        return;
    }

    //Segmentation
    if(!_Segmentation())
    {
        emit processFinished();
        return;
    }

    //emit signal to inform to PAnalyse class that the processing is over
    emit processFinished();
}

bool PPixelWiseMolarCoeff::_Segmentation()
{
    if(_M_pixels_pos.empty() || _M_img_Surface_blood_preprocessed.empty() || _M_img_Buried_Blood_preprocessed.empty())
        return false;

    if(_M_consider_blood_vessel_distance)
    {
        if(_M_reso_in_mm == -1)
            _ClusterMethod();
        else
            _PixelWiseMethod();
    }
    else
    {
        _ClusterMethod();
    }

    return true;
}


bool PPixelWiseMolarCoeff::_PreProcessing()
{
//    //init Data
//    _initData();

    if(_M_img_Surface_blood_in.empty() || _M_img_Buried_Blood_in.empty() || _M_non_used.empty())
        return false;

    //Step 1: Closing
    Mat element = getStructuringElement(MORPH_ELLIPSE,Size(3,3));
    morphologyEx(_M_img_Surface_blood_in, _M_img_Surface_blood_preprocessed, MORPH_CLOSE, element);
    morphologyEx(_M_img_Buried_Blood_in, _M_img_Buried_Blood_preprocessed, MORPH_CLOSE, element);
    //morphologyEx(_M_non_used_in, _M_non_used_preprocessed, MORPH_CLOSE, element);

    //Opening
    morphologyEx(_M_img_Surface_blood_preprocessed, _M_img_Surface_blood_preprocessed, MORPH_OPEN, element);
    morphologyEx(_M_img_Buried_Blood_preprocessed, _M_img_Buried_Blood_preprocessed, MORPH_OPEN, element);

    //imwrite(QString(path+"Step1_BS.tif").toStdString(),img_Surface_blood);
    //imwrite(QString(path+"Step1_BB.tif").toStdString(),img_Buried_Blood);
    //imwrite(QString(path+"Step1_non_used.tif").toStdString(),non_used);

    //Step 2: consider blood vessels as GM if size is inferior to 1 mm (minimum resolution sets in MCX)
    EraseSmallContour(_M_img_Buried_Blood_preprocessed,_M_area_lim_in_px);
    EraseSmallContour(_M_img_Surface_blood_preprocessed,_M_area_lim_in_px);

    //imwrite(QString(path+"Step2_BS.tif").toStdString(),img_Surface_blood);
    //imwrite(QString(path+"Step2_BB.tif").toStdString(),img_Buried_Blood);

    //Step 3: if blood vessel has internal contour and if it is not a blood vessel or unclassified data -> consider it as blood vessel
    handleInternalContour(_M_img_Surface_blood_preprocessed,_M_img_Buried_Blood_preprocessed);


//    imwrite(QString(TEMPORARY_SAVING_DIR).toStdString()+"Surface_Blood_layer.png",_M_img_Surface_blood_preprocessed);
//    imwrite(QString(TEMPORARY_SAVING_DIR).toStdString()+"Buried_Blood_layer.png",_M_img_Buried_Blood_preprocessed);
//    imwrite(QString(TEMPORARY_SAVING_DIR).toStdString()+"Non_used_layer.png",_M_non_used_preprocessed);

    return true;
}



//Load spectral resultant
void PPixelWiseMolarCoeff::_Load_spectral_resultant()
{
    qDebug()<<"[PPixelWiseMolarCoeff] in _Load_spectral_resultant";
    if(_M_mode_HyperSpectral)
    {
        //Load spectral resultant x eps_Hb (get lambda interpolation 400:1:1000 )
        _M_eps_Hb_x_res_spectral.clear();
        load_Matrix(QString(PROPATH)+"/../files/spectral_resultant/HS/HS_Hb_res_675_975.txt",_M_eps_Hb_x_res_spectral);

        //Load spectral resultant x eps_HbO2
        _M_eps_HbO2_x_res_spectral.clear();
        load_Matrix(QString(PROPATH)+"/../files/spectral_resultant/HS/HS_HbO2_res_675_975.txt",_M_eps_HbO2_x_res_spectral);

        //load oxCCO
        _M_eps_oxCCO_x_res_spectral.clear();
        load_Matrix(QString(PROPATH)+"/../files/spectral_resultant/HS/HS_oxCCO_res_675_975.txt",_M_eps_oxCCO_x_res_spectral);
    }
    else
    {
        //Load spectral resultant x eps_Hb (get lambda interpolation 400:1:1000 )
        _M_eps_Hb_x_res_spectral.clear();
        load_Matrix(QString(PROPATH)+"/../files/spectral_resultant/RGB/RGB_Hb_res.txt",_M_eps_Hb_x_res_spectral);

        //Load spectral resultant x eps_HbO2
        _M_eps_HbO2_x_res_spectral.clear();
        load_Matrix(QString(PROPATH)+"/../files/spectral_resultant/RGB/RGB_HbO2_res.txt",_M_eps_HbO2_x_res_spectral);

//        //Cam sensitivities
//        QVector<QVector<double> > RGB_cam_sensitivity;
//        load_Matrix(QString(PROPATH)+"/../files/spectra/RGB_cam_sensitivities.txt",RGB_cam_sensitivity);

//        //Light source spectra
//        QVector<double> light_source_spectra;
//        ReadVector(QString(PROPATH)+"/../files/spectra/light_spectra.txt",light_source_spectra);

//        //eps_Hb
//        _M_eps_Hb_x_res_spectral.clear();
//        _M_eps_Hb_x_res_spectral.resize(RGB_cam_sensitivity.size());
//        QVector<double> eps_Hb;
//        ReadVector(QString(PROPATH)+"/../files/spectra/eps_Hb_RGB.txt",eps_Hb);

//        //eps_HbO2
//        _M_eps_HbO2_x_res_spectral.clear();
//        _M_eps_HbO2_x_res_spectral.resize(RGB_cam_sensitivity.size());
//        QVector<double> eps_HbO2;
//        ReadVector(QString(PROPATH)+"/../files/spectra/eps_HbO2_RGB.txt",eps_HbO2);

//        //Mutliply light source to camera sensitivities
//        for(int c=0;c<RGB_cam_sensitivity.size();c++)
//        {
//            for(int w=0;w<RGB_cam_sensitivity[c].size();w++)
//                RGB_cam_sensitivity[c][w]*=light_source_spectra[w];

//            //Normalize by its sum
//            double sum = get_Vector_sum(RGB_cam_sensitivity[c]);
//            RGB_cam_sensitivity[c] = RGB_cam_sensitivity[c]/sum;

//            //Spectral resultant
//            for(int w=0;w<RGB_cam_sensitivity[c].size();w++)
//            {
//                _M_eps_Hb_x_res_spectral[c].push_back(eps_Hb[w]*RGB_cam_sensitivity[c][w]);
//                _M_eps_HbO2_x_res_spectral[c].push_back(eps_HbO2[w]*RGB_cam_sensitivity[c][w]);
//            }
//        }
    }
}

//ini data
void PPixelWiseMolarCoeff::_initData()
{
    //If initialisation has already been done
    if(!_M_init_or_process)
        return;

    //init
    _M_Molar_coeff.clear();

    //Use different filename because we use different wavelength
    // RGB 400 -> 1000
    // HS 600 -> 1000
    QString Surface_Blood_filename,Buried_Blood_filename;
    if(_M_mode_HyperSpectral)
    {
        Surface_Blood_filename  = "Mean_path/Mean_path_Surface_blood_HS.txt";
        Buried_Blood_filename   = "Mean_path/Mean_path_Buried_Blood_HS.txt";
    }
    else
    {
        Surface_Blood_filename  = "Mean_path/Mean_path_Surface_blood_RGB.txt";
        Buried_Blood_filename   = "Mean_path/Mean_path_Buried_Blood_RGB.txt";
    }




    _Load_spectral_resultant();

    _M_img_Buried_Blood_preprocessed    = Mat::zeros(0,0,CV_8UC1);
    _M_img_Surface_blood_preprocessed   = Mat::zeros(0,0,CV_8UC1);






    //Load Matrix of mean path length (in mm)
    // Resolution 100 µm
    // distance to blood vessel (first dimension of the matrix) 18 : 0
    _M_Mean_path_Surface_Blood.clear();
    _M_Mean_path_Buried_Blood.clear();

    //Size 181,601 (RGB) or 181,401 (HS)
    load_Matrix(QString(PROPATH)+"/../files/"+Surface_Blood_filename,_M_Mean_path_Surface_Blood);
    load_Matrix(QString(PROPATH)+"/../files/"+Buried_Blood_filename,_M_Mean_path_Buried_Blood);


    QVector<Mat> matrix_sum;
    //Surface blood vessel
    if(_M_mode_HyperSpectral)
    {
        if(_M_optimize_HS_quantification)
        {
            if(_M_quantify_oxCCO)
                integrateDatas_Hb_HbO2_oxCCO(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_eps_oxCCO_x_res_spectral,_M_Mean_path_Surface_Blood[_M_Mean_path_Surface_Blood.size()-1],_M_HbO2_Hb_oxCCO_HS_cam_idx);
            else
                integrateDatas(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_Mean_path_Surface_Blood[_M_Mean_path_Surface_Blood.size()-1],_M_HbO2_Hb_HS_cam_idx);

        }
        else
        {
            if(_M_quantify_oxCCO)
                integrateDatas_Hb_HbO2_oxCCO(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_eps_oxCCO_x_res_spectral,_M_Mean_path_Surface_Blood[_M_Mean_path_Surface_Blood.size()-1]);
            else
                integrateDatas(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_Mean_path_Surface_Blood[_M_Mean_path_Surface_Blood.size()-1]);
        }
    }
    else
        integrateDatas(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_Mean_path_Surface_Blood[_M_Mean_path_Surface_Blood.size()-1]);


//    qDebug()<<"Matrix sum surface blood";
//    _DisplayMatrix(matrix_sum);

    //inverse matrix
    invertMatrix(matrix_sum,_M_Molar_coeff_SurfaceBlood);


//    qDebug()<<"Matrix surface blood";
//    _DisplayMatrix(_M_Molar_coeff_SurfaceBlood);

    //Buried blood vessel
    if(_M_mode_HyperSpectral)
    {
        if(_M_optimize_HS_quantification)
        {
            if(_M_quantify_oxCCO)
                integrateDatas_Hb_HbO2_oxCCO(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_eps_oxCCO_x_res_spectral,_M_Mean_path_Buried_Blood[_M_Mean_path_Buried_Blood.size()-1],_M_HbO2_Hb_oxCCO_HS_cam_idx);
            else
                integrateDatas(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_Mean_path_Buried_Blood[_M_Mean_path_Buried_Blood.size()-1],_M_HbO2_Hb_HS_cam_idx);

        }
        else
        {
            if(_M_quantify_oxCCO)
                integrateDatas_Hb_HbO2_oxCCO(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_eps_oxCCO_x_res_spectral,_M_Mean_path_Buried_Blood[_M_Mean_path_Buried_Blood.size()-1]);
            else
                integrateDatas(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_Mean_path_Buried_Blood[_M_Mean_path_Buried_Blood.size()-1]);
        }
    }
    else
        integrateDatas(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_Mean_path_Buried_Blood[_M_Mean_path_Buried_Blood.size()-1]);


    //inverse matrix
    invertMatrix(matrix_sum,_M_Molar_coeff_BuriedBlood);


    //Grey matter
    if(_M_mode_HyperSpectral)
    {
        if(_M_optimize_HS_quantification)
        {
            if(_M_quantify_oxCCO)
                integrateDatas_Hb_HbO2_oxCCO(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_eps_oxCCO_x_res_spectral,_M_Mean_path_Surface_Blood[0],_M_HbO2_Hb_oxCCO_HS_cam_idx);
            else
                integrateDatas(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_Mean_path_Surface_Blood[0],_M_HbO2_Hb_HS_cam_idx);

        }
        else
        {
            if(_M_quantify_oxCCO)
                integrateDatas_Hb_HbO2_oxCCO(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_eps_oxCCO_x_res_spectral,_M_Mean_path_Surface_Blood[0]);
            else
                integrateDatas(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_Mean_path_Surface_Blood[0]);
        }
    }
    else
        integrateDatas(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_Mean_path_Surface_Blood[0]);


    //inverse matrix
    invertMatrix(matrix_sum,_M_Molar_coeff_GM);


    for(int i=0;i<_M_pixels_pos.size();i++)
    {
        _M_Molar_coeff.push_back(_M_Molar_coeff_GM);
    }


     _M_init_or_process = false;

    //Write GM Molar coeff
    QString saving_dir  =QString(TEMPORARY_SAVING_DIR)+"MBLL_Matrices/";
    QDir dir(saving_dir);
    if(!dir.exists())
        dir.mkdir(saving_dir);

    for(int i=0;i<_M_Molar_coeff_GM.size();i++)
    {
        QString filename = _M_mode_HyperSpectral ? saving_dir+"E_inv_GM_HS_"+QString::number(i)+".txt" : saving_dir+"E_inv_GM_RGB_"+QString::number(i)+".txt";
        WriteDoubleImg(filename,_M_Molar_coeff_GM[i]);

        filename = _M_mode_HyperSpectral ? saving_dir+"E_GM_HS_"+QString::number(i)+".txt" : saving_dir+"E_GM_RGB_"+QString::number(i)+".txt";
        WriteDoubleImg(filename,matrix_sum[i]);
    }


    return;
}




//Process segmentation using simulated spectra
/*
void PPixelWiseMolarCoeff::_Process_Hyperspectral_Segmentation()
{
    if(_M_first_HS_img.empty())
        return;
    float thresh_error = 10000;

    //Init
    Mat Molar_coeff;
    Mat matrix_sum;
    Mat empty_matrix = Mat::zeros(0,0,CV_64FC1);
    QVector<int> mean_path_id;
    QVector<float> errors;

    //Scan hyperspectral img
    qDebug()<<"simulated spectra size: "<<_M_Simulated_Spectra.size()<<" "<<_M_Simulated_Spectra[0].size();

    QVector<QVector<double> > spectra;

    for(int pos=0;pos<_M_pixels_pos.size();pos++)
    {
        //Get the spectra for each camera pixel
        QVector<double> acquired_spectra;
        for(int spectral_id=0;spectral_id<_M_first_HS_img.size();spectral_id++)
            acquired_spectra.push_back((double)_M_first_HS_img[spectral_id].at<ushort>(_M_pixels_pos[pos].y,_M_pixels_pos[pos].x));

        spectra.push_back(acquired_spectra);
        //Get the id of the simulated spectra wich is the most similar to the acquired spectra
        int spectral_id;
        float error;
        find_best_simulated_spectra(_M_Simulated_Spectra,acquired_spectra,spectral_id,error);
        errors.push_back(error);

        //If the error is to big set empty matrix to the current pixel
        if(error>thresh_error)
        {
            mean_path_id.push_back(-1);
            _M_Molar_coeff.push_back(empty_matrix);
            continue;
        }

        mean_path_id.push_back(spectral_id);

        //If the error is acceptable get the mean path length at the choosen spectral id
        //Get Molar coeff matrix
        if(_M_quantify_oxCCO)
            integrateDatas_Hb_HbO2_oxCCO(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_eps_oxCCO_x_res_spectral,_M_Mean_path_Surface_Blood[spectral_id],_M_wavelength[_M_wavelength_id]);
        else
            integrateDatas(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_Mean_path_Surface_Blood[spectral_id],_M_wavelength[_M_wavelength_id]);


        //inverse matrix
        invertMatrix(matrix_sum,Molar_coeff);
        //Set molar coeff matrix
        _M_Molar_coeff.push_back(Molar_coeff);
    }

    WriteTemporalVector(QString(TEMPORARY_SAVING_DIR)+"spectra.txt",spectra);
    WriteTemporalVector(QString(TEMPORARY_SAVING_DIR)+"error_vec.txt",errors);
    WriteTemporalVector(QString(TEMPORARY_SAVING_DIR)+"id_vec.txt",mean_path_id);


    emit newHyperspectralSegmentation(mean_path_id);
}
*/

//Display Matrix
void PPixelWiseMolarCoeff::_DisplayMatrix(Mat img)
{
    qDebug()<<"Display matrix";
    for(int row =0;row<img.rows;row++)
    {
        double *ptr = img.ptr<double>(row);
        for(int col=0;col<img.cols;col++)
        {
            qDebug()<<"pos: ["<<row<<";"<<col<<"]"<<ptr[col];
        }
    }
}


void PPixelWiseMolarCoeff::initMolarCoeff()
{
    _M_init_or_process = true; //true: init, false: process
    this->start();
}

//get GM matrix for Hb and HbO2 or Hb, HbO2 and oxCCO deconvolution
/*
Mat PPixelWiseMolarCoeff::getMolarCoeff(int pos,bool Hb_HbO2_deconvolution,QVector<int> wavelength_grp)
{
    if(!_M_mode_HyperSpectral)
        return _M_Molar_coeff_GM;

    Mat matrix_sum,Molar_coeff;


    //cluster method
    if(_M_reso_in_mm == -1)
    {
        //Surface blood
        if(_M_img_Surface_blood_preprocessed.at<uchar>(_M_pixels_pos[pos].y,_M_pixels_pos[pos].x) == 255)
        {
            if(Hb_HbO2_deconvolution)
                integrateDatas(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_Mean_path_Surface_Blood[_M_Mean_path_Surface_Blood.size()-1],wavelength_grp);
            else
                integrateDatas_Hb_HbO2_oxCCO(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_eps_oxCCO_x_res_spectral,_M_Mean_path_Surface_Blood[_M_Mean_path_Surface_Blood.size()-1],wavelength_grp);

            //inverse matrix
            invertMatrix(matrix_sum,Molar_coeff);
            return Molar_coeff;

        }

        //Buried blood
        if(_M_img_Buried_Blood_preprocessed.at<uchar>(_M_pixels_pos[pos].y,_M_pixels_pos[pos].x) == 255)
        {
            if(Hb_HbO2_deconvolution)
                integrateDatas(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_Mean_path_Buried_Blood[_M_Mean_path_Buried_Blood.size()-1],wavelength_grp);
            else
                integrateDatas_Hb_HbO2_oxCCO(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_eps_oxCCO_x_res_spectral,_M_Mean_path_Buried_Blood[_M_Mean_path_Buried_Blood.size()-1],wavelength_grp);

            //inverse matrix
            invertMatrix(matrix_sum,Molar_coeff);
            return Molar_coeff;
        }

        //Grey matter
        if(Hb_HbO2_deconvolution)
            integrateDatas(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_Mean_path_Surface_Blood[0],wavelength_grp);
        else
            integrateDatas_Hb_HbO2_oxCCO(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_eps_oxCCO_x_res_spectral,_M_Mean_path_Surface_Blood[0],wavelength_grp);

        //inverse matrix
        invertMatrix(matrix_sum,Molar_coeff);
        return Molar_coeff;

    }
    //Segmentation method
    else
    {
        //Surface blood
        if(_M_img_Surface_blood_preprocessed.at<uchar>(_M_pixels_pos[pos].y,_M_pixels_pos[pos].x) == 255)
        {
            if(Hb_HbO2_deconvolution)
                integrateDatas(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_Mean_path_Surface_Blood[_M_Mean_path_Surface_Blood.size()-1],wavelength_grp);
            else
                integrateDatas_Hb_HbO2_oxCCO(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_eps_oxCCO_x_res_spectral,_M_Mean_path_Surface_Blood[_M_Mean_path_Surface_Blood.size()-1],wavelength_grp);

            //inverse matrix
            invertMatrix(matrix_sum,Molar_coeff);
            return Molar_coeff;

        }

        //Buried blood
        if(_M_img_Buried_Blood_preprocessed.at<uchar>(_M_pixels_pos[pos].y,_M_pixels_pos[pos].x) == 255)
        {
            if(Hb_HbO2_deconvolution)
                integrateDatas(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_Mean_path_Buried_Blood[_M_Mean_path_Buried_Blood.size()-1],wavelength_grp);
            else
                integrateDatas_Hb_HbO2_oxCCO(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_eps_oxCCO_x_res_spectral,_M_Mean_path_Buried_Blood[_M_Mean_path_Buried_Blood.size()-1],wavelength_grp);

            //inverse matrix
            invertMatrix(matrix_sum,Molar_coeff);
            return Molar_coeff;
        }

        //GREY MATTER
        QVector<double> Mean_path;
        double min_SBV =100000;
        for(unsigned int c=0;c<_M_contours_SBV.size();c++)
        {
            Point2f P(_M_pixels_pos[pos].x,_M_pixels_pos[pos].y);
            double dst = abs(pointPolygonTest(_M_contours_SBV[c],P,true));
            min_SBV = (dst<min_SBV) ? dst : min_SBV;
        }

        //Get min dst between point P(col,row) and the buried blood contours
        double min_BBV =100000;
        for(unsigned int c=0;c<_M_contours_BBV.size();c++)
        {
            Point2f P(_M_pixels_pos[pos].x,_M_pixels_pos[pos].y);
            double dst = abs(pointPolygonTest(_M_contours_BBV[c],P,true));
            min_BBV = (dst<min_BBV) ? dst : min_BBV;
        }

        //Store min dist between min_SBV and min_BBV in dst_matrix
        double min_dist = (min_SBV<min_BBV)? _M_reso_in_mm*min_SBV : _M_reso_in_mm*min_BBV;

        //Store if the closest blood contour is surfacic (1) or buried (2)
        int tissue_type = (min_SBV<min_BBV)? 1 : 2;

        //GM mean path
        _ChooseMeanPathGM(Mean_path,min_dist,tissue_type);

        //Grey matter
        if(Hb_HbO2_deconvolution)
            integrateDatas(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,Mean_path,wavelength_grp);
        else
            integrateDatas_Hb_HbO2_oxCCO(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_eps_oxCCO_x_res_spectral,Mean_path,wavelength_grp);

        //inverse matrix
        invertMatrix(matrix_sum,Molar_coeff);
        return Molar_coeff;

    }

    return Molar_coeff;
}
*/



//Get buried blood matrix
/*
Mat PPixelWiseMolarCoeff::getBuriedBloodMatrix()
{
    return _M_Molar_coeff_BuriedBlood;
}
*/

//get Surface Blood matrix
/*
Mat PPixelWiseMolarCoeff::getSurfaceBloodMatrix()
{
    return _M_Molar_coeff_SurfaceBlood;
}
*/

//is blood vessel Matrix
/*
bool PPixelWiseMolarCoeff::isBloodVesselMatrix(int id)
{
    Mat dst;
    bitwise_xor(_M_Molar_coeff[id], _M_Molar_coeff_BuriedBlood, dst);
    if(countNonZero(dst) > 0) //check non-0 pixels
    {
        //Different
        bitwise_xor(_M_Molar_coeff[id], _M_Molar_coeff_SurfaceBlood, dst);
        if(countNonZero(dst) > 0) //check non-0 pixels
        {
            //Different
            return false;
        }
        else
        {
            //Equal
            return true;
        }
    }
    else
    {
        //Equal
        return true;
    }
    return false;
}
*/


//process molar coeff matrix
void PPixelWiseMolarCoeff::processMolarCoeff()
{
    _M_init_or_process = false; //true: init, false: process
    this->start();
}

//Set molar coeff according to the cluster id
void PPixelWiseMolarCoeff::_ClusterMethod()
{
    qDebug()<<"PPixelWiseMolarCoeff::_ClusterMethod";

    //id of segmented elements
    _M_id_GM.clear();
    _M_id_BuriedBlood.clear();
    _M_id_SurfaceBlood.clear();

    //molar coeff
    _M_Molar_coeff.clear();

    QVector<Mat> empty_matrix;
    empty_matrix.push_back(Mat::zeros(0,0,CV_64FC1));

    for(int i=0;i<_M_pixels_pos.size();i++)
    {
        //Surface blood
        if(_M_img_Surface_blood_preprocessed.at<uchar>(_M_pixels_pos[i].y,_M_pixels_pos[i].x) == 255)
        {
            _M_id_SurfaceBlood.push_back(i);
            _M_Molar_coeff.push_back(_M_Molar_coeff_SurfaceBlood);
            continue;
        }

        //Buried blood
        if(_M_img_Buried_Blood_preprocessed.at<uchar>(_M_pixels_pos[i].y,_M_pixels_pos[i].x) == 255)
        {
            _M_id_BuriedBlood.push_back(i);
            _M_Molar_coeff.push_back(_M_Molar_coeff_BuriedBlood);
            continue;
        }
        //unused pixel
        if(_M_non_used.at<uchar>(_M_pixels_pos[i].y,_M_pixels_pos[i].x) == 255)
        {
            _M_Molar_coeff.push_back(empty_matrix);
            continue;
        }

        //Grey matter
        _M_id_GM.push_back(i);
        _M_Molar_coeff.push_back(_M_Molar_coeff_GM);
    }
}

//Get Mean path length
QVector<double> PPixelWiseMolarCoeff::getMeanPath(int pos)
{
    //Surface blood
    if(_M_img_Surface_blood_preprocessed.at<uchar>(_M_pixels_pos[pos].y,_M_pixels_pos[pos].x) == 255)
        return getMeanPathSBV();

    //Buried blood
    if(_M_img_Buried_Blood_preprocessed.at<uchar>(_M_pixels_pos[pos].y,_M_pixels_pos[pos].x) == 255)
        return getMeanPathBBV();

    //unused pixel
    if(_M_non_used.at<uchar>(_M_pixels_pos[pos].y,_M_pixels_pos[pos].x) == 255)
        return getMeanPathMG();

    //Grey matter
    //Get min dst between point P(col,row) and the surface blood contours
    _M_id_GM.push_back(pos);
    double min_SBV =100000;
    for(unsigned int c=0;c<_M_contours_SBV.size();c++)
    {
        Point2f P(_M_pixels_pos[pos].x,_M_pixels_pos[pos].y);
        double dst = abs(pointPolygonTest(_M_contours_SBV[c],P,true));
        min_SBV = (dst<min_SBV) ? dst : min_SBV;
    }

    //Get min dst between point P(col,row) and the buried blood contours
    double min_BBV =100000;
    for(unsigned int c=0;c<_M_contours_BBV.size();c++)
    {
        Point2f P(_M_pixels_pos[pos].x,_M_pixels_pos[pos].y);
        double dst = abs(pointPolygonTest(_M_contours_BBV[c],P,true));
        min_BBV = (dst<min_BBV) ? dst : min_BBV;
    }

    //Store min dist between min_SBV and min_BBV in dst_matrix
    double min_dist = (min_SBV<min_BBV)? _M_reso_in_mm*min_SBV : _M_reso_in_mm*min_BBV;

    //Store if the closest blood contour is surfacic (1) or buried (2)
    int tissue_type = (min_SBV<min_BBV)? 1 : 2;

    //GM mean path
    QVector<double> mean_path_GM;
    _ChooseMeanPathGM(mean_path_GM,min_dist,tissue_type);
    return mean_path_GM;
}

//Set molar coeff according to the distance to blood vessel
void PPixelWiseMolarCoeff::_PixelWiseMethod()
{
    qDebug()<<"[PPixelWiseMolarCoeff] in Pixel wise method";

    //id of segmented elements
    _M_id_GM.clear();
    _M_id_BuriedBlood.clear();
    _M_id_SurfaceBlood.clear();

    //init matrix
    _M_Molar_coeff.clear();

    //Get blood vessel contours
    //Blood vessel contours
    _M_contours_SBV.clear();
    _M_contours_BBV.clear();

//    imwrite(QString(TEMPORARY_SAVING_DIR).toStdString()+"Surface_blood.png",_M_img_Surface_blood_preprocessed);
//    imwrite(QString(TEMPORARY_SAVING_DIR).toStdString()+"Buried_blood.png",_M_img_Buried_Blood_preprocessed);

    findContours(_M_img_Surface_blood_preprocessed.clone(),_M_contours_SBV,CV_RETR_TREE,CV_CHAIN_APPROX_NONE);
    findContours(_M_img_Buried_Blood_preprocessed.clone(),_M_contours_BBV,CV_RETR_TREE,CV_CHAIN_APPROX_NONE);

    QVector<Mat> matrix_sum;
    QVector<Mat> empty_matrix;
    empty_matrix.push_back(Mat::zeros(0,0,CV_64FC1));

    for(int pos=0;pos<_M_pixels_pos.size();pos++)
    {
        //Surface blood
        if(_M_img_Surface_blood_preprocessed.at<uchar>(_M_pixels_pos[pos].y,_M_pixels_pos[pos].x) == 255)
        {
            _M_id_SurfaceBlood.push_back(pos);
            _M_Molar_coeff.push_back(_M_Molar_coeff_SurfaceBlood);
            continue;
        }

        //Buried blood
        if(_M_img_Buried_Blood_preprocessed.at<uchar>(_M_pixels_pos[pos].y,_M_pixels_pos[pos].x) == 255)
        {
            _M_id_BuriedBlood.push_back(pos);
            _M_Molar_coeff.push_back(_M_Molar_coeff_BuriedBlood);
            continue;
        }
        //unused pixel
        if(_M_non_used.at<uchar>(_M_pixels_pos[pos].y,_M_pixels_pos[pos].x) == 255)
        {
            _M_Molar_coeff.push_back(empty_matrix);
            continue;
        }

        //Grey matter
        //Get min dst between point P(col,row) and the surface blood contours
        _M_id_GM.push_back(pos);
        double min_SBV =100000;
        for(unsigned int c=0;c<_M_contours_SBV.size();c++)
        {
            Point2f P(_M_pixels_pos[pos].x,_M_pixels_pos[pos].y);
            double dst = abs(pointPolygonTest(_M_contours_SBV[c],P,true));
            min_SBV = (dst<min_SBV) ? dst : min_SBV;
        }

        //Get min dst between point P(col,row) and the buried blood contours
        double min_BBV =100000;
        for(unsigned int c=0;c<_M_contours_BBV.size();c++)
        {
            Point2f P(_M_pixels_pos[pos].x,_M_pixels_pos[pos].y);
            double dst = abs(pointPolygonTest(_M_contours_BBV[c],P,true));
            min_BBV = (dst<min_BBV) ? dst : min_BBV;
        }

        //Store min dist between min_SBV and min_BBV in dst_matrix
        double min_dist = (min_SBV<min_BBV)? _M_reso_in_mm*min_SBV : _M_reso_in_mm*min_BBV;

        //Store if the closest blood contour is surfacic (1) or buried (2)
        int tissue_type = (min_SBV<min_BBV)? 1 : 2;

//        qDebug()<<"Surface blood dist : "<<min_SBV<<" Buried blood dist : "<<min_BBV;
//        qDebug()<<"dist min : "<<min_dist<<" reso in mm : "<<_M_reso_in_mm;
//        qDebug()<<"Tissue type (1) surface blood (2) buried blood : "<<tissue_type;



        //GM mean path
        QVector<Mat> Molar_coeff;
        QVector<double> mean_path_GM;
        _ChooseMeanPathGM(mean_path_GM,min_dist,tissue_type);

        //Get Molar coeff matrix
        if(_M_mode_HyperSpectral)
        {
            if(_M_optimize_HS_quantification)
            {
                if(_M_quantify_oxCCO)
                    integrateDatas_Hb_HbO2_oxCCO(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_eps_oxCCO_x_res_spectral,mean_path_GM,_M_HbO2_Hb_oxCCO_HS_cam_idx);
                else
                    integrateDatas(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,mean_path_GM,_M_HbO2_Hb_HS_cam_idx);
            }
            else
            {
                if(_M_quantify_oxCCO)
                    integrateDatas_Hb_HbO2_oxCCO(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_eps_oxCCO_x_res_spectral,mean_path_GM);
                else
                    integrateDatas(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,mean_path_GM);
            }
        }
        else
            integrateDatas(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,mean_path_GM);

        //inverse matrix
        invertMatrix(matrix_sum,Molar_coeff);

        //Set molar coeff matrix
        _M_Molar_coeff.push_back(Molar_coeff);
    }
}


//Get distance to blood vessel
float PPixelWiseMolarCoeff::getDistanceToBloodVessels(Point p)
{
    //Surface blood
    if(_M_img_Surface_blood_preprocessed.at<uchar>(p.y,p.x) == 255)
        return 0.0f;

    //Buried blood
    if(_M_img_Buried_Blood_preprocessed.at<uchar>(p.y,p.x) == 255)
        return 0.0f;

    //unused pixel
    if(_M_non_used.at<uchar>(p.y,p.x) == 255)
        return 0.0f;

    //Grey matter
    //Get min dst between point P(col,row) and the surface blood contours
    double min_SBV =100000;
    for(unsigned int c=0;c<_M_contours_SBV.size();c++)
    {
        Point2f P(p.x,p.y);
        double dst = abs(pointPolygonTest(_M_contours_SBV[c],P,true));
        min_SBV = (dst<min_SBV) ? dst : min_SBV;
    }

    //Get min dst between point P(col,row) and the buried blood contours
    double min_BBV =100000;
    for(unsigned int c=0;c<_M_contours_BBV.size();c++)
    {
        Point2f P(p.x,p.y);
        double dst = abs(pointPolygonTest(_M_contours_BBV[c],P,true));
        min_BBV = (dst<min_BBV) ? dst : min_BBV;
    }

    //Store min dist between min_SBV and min_BBV in dst_matrix
    double dist =  (min_SBV<min_BBV)? _M_reso_in_mm*min_SBV : _M_reso_in_mm*min_BBV;
    return (float)dist;

//    //Store if the closest blood contour is surfacic (1) or buried (2)
//    int tissue_type = (min_SBV<min_BBV)? 1 : 2;
}

//Choose GM mean path length according to the distance of the current pixel to the nearest blood vessel
void PPixelWiseMolarCoeff::_ChooseMeanPathGM(QVector<double> &mean_path_GM,double dist,int tissue_type)
{
    mean_path_GM.clear();

    //If dist is far enough away from the nearest blood vessel : use GM mean path length
    if(dist>=19)//30)
    {
        mean_path_GM = _M_Mean_path_Buried_Blood[0];
        return;
    }

    int id = int(dist/Reso_MeanPath_mm);
    if(id>_M_Mean_path_Surface_Blood.size()-1)
    {
        mean_path_GM = _M_Mean_path_Buried_Blood[0];
        return;
    }

//    qDebug()<<"id Mean path: "<<id<<" dist: "<<dist;

    //Surfacic blood mean path length
    if(tissue_type == 1)
        mean_path_GM = _M_Mean_path_Surface_Blood[_M_Mean_path_Surface_Blood.size()-1 - id];
    //buried mean path length
    else
        mean_path_GM = _M_Mean_path_Buried_Blood[_M_Mean_path_Buried_Blood.size()-1 - id];
}



//is empty Matrix
bool PPixelWiseMolarCoeff::isEmptyMatrix(int id)
{
    bool empty = false;
    for(int i=0;i<_M_Molar_coeff[id].size();i++)
        empty = empty || _M_Molar_coeff[id][i].empty();

    return empty;
}
