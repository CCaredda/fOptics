#include "PPixelWiseMolarCoeff.h"
#include "molarcoefffunctions.h"
#include "acquisition.h"




PPixelWiseMolarCoeff::PPixelWiseMolarCoeff(QObject *parent) : QThread(parent)
{
    //Init result directory
    _M_result_directory =  "/home/results/";

    //moveToThread(this);

    //HS spectral range
    _M_HS_lambda_min = 675;
    _M_HS_lambda_max = 975;


    _M_quantify_oxCCO = false;
    _M_consider_blood_vessel_distance = true;

    _M_init_or_process = true; //true: init, false: process

    _M_reso_in_mm                       = -1;
    _M_vessel_lim_in_px                 = 6; // 1mm * 1 px/ 0.17718mm
    _M_area_lim_in_px                   = _M_vessel_lim_in_px*_M_vessel_lim_in_px;

    //Clear distance map
    _M_distance_map.clear();

    //Extinction spectra  (in cm-1.mol-1.L)
    _M_epsilon_Hb.clear();
    _M_epsilon_HbO2.clear();
    _M_epsilon_oxCCO.clear();
    _M_wavelength_epsilon.clear();
    ReadVector(QString(QCoreApplication::applicationDirPath())+"/../share/files/extinction_coefficients/eps_HbO2.txt",_M_epsilon_HbO2);
    ReadVector(QString(QCoreApplication::applicationDirPath())+"/../share/files/extinction_coefficients/eps_Hb.txt",_M_epsilon_Hb);
    ReadVector(QString(QCoreApplication::applicationDirPath())+"/../share/files/extinction_coefficients/eps_oxCCO.txt",_M_epsilon_oxCCO);
    ReadVector(QString(QCoreApplication::applicationDirPath())+"/../share/files/extinction_coefficients/wavelength_eps.txt",_M_wavelength_epsilon);



    //Light source spectra (normalized in %)
    _M_id_light_source = Light_source_Halogen;
    _M_light_source_spectra.clear();
    _M_wavelength_light.clear();
    QVector<double> temp;
    ReadVector(QString(QCoreApplication::applicationDirPath())+"/../share/files/Light_sources/Halogen_spectra.txt",temp);
    //Normalize to max
    NormalizeMaxVector(temp);
    _M_light_source_spectra.push_back(temp);
    temp.clear();
    ReadVector(QString(QCoreApplication::applicationDirPath())+"/../share/files/Light_sources/wavelength_halogen.txt",temp);
    _M_wavelength_light.push_back(temp);



    //Camera spectral sensitivities  (normalized in %)
    _M_camera_spectral_sensitivities.clear();
    _M_wavelength_camera.clear();
    _M_id_camera = Camera_RGB_Basler;

    //RGB Basler
    QVector<QVector<double> > matrix_temp;
    load_Matrix(QString(QCoreApplication::applicationDirPath())+"/../share/files/Cameras/RGB_Basler/cam_RGB_Basler.txt",matrix_temp);
    _M_camera_spectral_sensitivities.push_back(matrix_temp);
    temp.clear();
    ReadVector(QString(QCoreApplication::applicationDirPath())+"/../share/files/Cameras/RGB_Basler/wavelength_RGB_Basler.txt",temp);
    _M_wavelength_camera.push_back(temp);




    //HS Ximea with spectral correction
    matrix_temp.clear();
    load_Matrix(QString(QCoreApplication::applicationDirPath())+"/../share/files/Cameras/HS_Ximea/HS_cam_Ximea_675_975nm_corr.txt",matrix_temp);
    _M_camera_spectral_sensitivities.push_back(matrix_temp);
    temp.clear();
    ReadVector(QString(QCoreApplication::applicationDirPath())+"/../share/files/Cameras/HS_Ximea/wavelength_HS_cam_Ximea_675_975nm.txt",temp);
    _M_wavelength_camera.push_back(temp);

    //HS Ximea without spectral correction
    matrix_temp.clear();
    load_Matrix(QString(QCoreApplication::applicationDirPath())+"/../share/files/Cameras/HS_Ximea/HS_cam_Ximea_675_975nm_no_corr.txt",matrix_temp);
    _M_camera_spectral_sensitivities.push_back(matrix_temp);
    temp.clear();
    ReadVector(QString(QCoreApplication::applicationDirPath())+"/../share/files/Cameras/HS_Ximea/wavelength_HS_cam_Ximea_675_975nm.txt",temp);
    _M_wavelength_camera.push_back(temp);




    //init spectral channel idx
    _M_spectral_idx.clear();
    for(int i=0;i<_M_camera_spectral_sensitivities[_M_id_camera].size();i++)
        _M_spectral_idx.push_back(i);





    //Load Matrix of mean path length (in cm)
    // Resolution 100 µm
    // distance to blood vessel (first dimension of the matrix) 18 : 0
    _M_Mean_path_Surface_Blood.clear();
    _M_Mean_path_Buried_Blood.clear();
    _M_wavelength_mean_path.clear();

    //Spectral resultant
    _M_eps_Hb_x_res_spectral.clear();
    _M_eps_HbO2_x_res_spectral.clear();
    _M_eps_oxCCO_x_res_spectral.clear();





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
//    _Init_Hyperspectral_wavelength_config();

    qDebug()<<"init data";
    _initData();
    qDebug()<<"end init data";


    //optimize HS quantification using optimal wavelength
    _M_optimize_HS_quantification = false;

}

PPixelWiseMolarCoeff::~PPixelWiseMolarCoeff()
{
    if (isRunning())
    {
        requestInterruption();
        wait();
    }
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


    load_Matrix(QString(QCoreApplication::applicationDirPath())+"/../share/files/Cameras/HS_Ximea/HS_cam_idx/HbO2_Hb_oxCCO_wavelength_grp.txt",_M_HbO2_Hb_oxCCO_HS_cam_idx);
    load_Matrix(QString(QCoreApplication::applicationDirPath())+"/../share/files/Cameras/HS_Ximea/HS_cam_idx/HbO2_Hb_wavelength_grp.txt",_M_HbO2_Hb_HS_cam_idx);



//    if(_M_quantify_oxCCO)
//        emit newHyperspectralWavelengthConfig(_M_HbO2_Hb_oxCCO_HS_cam_idx);
//    else
//        emit newHyperspectralWavelengthConfig(_M_HbO2_Hb_HS_cam_idx);
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
    if(_M_id_camera == Camera_HS_Ximea_corr || _M_id_camera == Camera_HS_Ximea_no_corr)
        _M_quantify_oxCCO = v;
    else
        _M_quantify_oxCCO = false;

//    if(_M_quantify_oxCCO)
//        emit newHyperspectralWavelengthConfig(_M_HbO2_Hb_oxCCO_HS_cam_idx);
//    else
//        emit newHyperspectralWavelengthConfig(_M_HbO2_Hb_HS_cam_idx);

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
//        emit processFinished();
        return;
    }

    //Segmentation
    if(!_Segmentation())
    {
//        emit processFinished();
        return;
    }
    //emit signal to inform to PAnalyse class that the processing is over
//    emit processFinished();
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
    qDebug()<<"[PPixelWiseMolarCoeff::_PreProcessing()] start";
//    //init Data
//    _initData();

    if(_M_img_Surface_blood_in.empty() || _M_img_Buried_Blood_in.empty() || _M_non_used.empty())
    {
        qDebug()<<"[PPixelWiseMolarCoeff::_PreProcessing()] segmentation empty";
        return false;
    }

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


//    imwrite(QString(_M_result_directory).toStdString()+"Surface_Blood_layer.png",_M_img_Surface_blood_preprocessed);
//    imwrite(QString(_M_result_directory).toStdString()+"Buried_Blood_layer.png",_M_img_Buried_Blood_preprocessed);
//    imwrite(QString(_M_result_directory).toStdString()+"Non_used_layer.png",_M_non_used_preprocessed);

    qDebug()<<"[PPixelWiseMolarCoeff::_PreProcessing()] end";
    return true;
}

//Load Mean path length
void PPixelWiseMolarCoeff::_Load_Mean_path_Length()
{

    //Load Matrix of mean path length (in cm)
    // Resolution 100 µm
    // distance to blood vessel (first dimension of the matrix) 18 : 0
    _M_Mean_path_Surface_Blood.clear();
    _M_Mean_path_Buried_Blood.clear();
    _M_wavelength_mean_path.clear();
    //Size 181,601
    // load_Matrix(QString(PROPATH)+"/../files/Mean_path_length/Mean_path_Surface_Blood_Vessel.txt",_M_Mean_path_Surface_Blood);
    // load_Matrix(QString(PROPATH)+"/../files/Mean_path_length/Mean_path_Buried_Blood_Vessel.txt",_M_Mean_path_Buried_Blood);
    // ReadVector(QString(PROPATH)+"/../files/Mean_path_length/wavelength_Mean_path.txt",_M_wavelength_mean_path);

    load_Matrix(QString(QCoreApplication::applicationDirPath())+"/../share/files/Mean_path_length/Mean_path_Surface_Blood_Vessel.txt",_M_Mean_path_Surface_Blood);
    load_Matrix(QString(QCoreApplication::applicationDirPath())+"/../share/files/Mean_path_length/Mean_path_Buried_Blood_Vessel.txt",_M_Mean_path_Buried_Blood);
    ReadVector(QString(QCoreApplication::applicationDirPath())+"/../share/files/Mean_path_length/wavelength_Mean_path.txt",_M_wavelength_mean_path);


    // Get wavelength idx of the camera
    // Add a flag or message (non corresponding wavelength)
    if(_M_wavelength_camera[_M_id_camera][0] < _M_wavelength_mean_path[0] ||
            _M_wavelength_camera[_M_id_camera][_M_wavelength_camera[_M_id_camera].size()-1] > _M_wavelength_mean_path[_M_wavelength_mean_path.size()-1])
    {
        qDebug()<<"[PPixelWiseMolarCoeff::_Load_spectral_resultant] camera and light source wavelengths do not match";
        return;
    }


    //Reduce vector size in accordance with camera sensitivity
    int wavelength_mp_idx0 = int(_M_wavelength_camera[_M_id_camera][0] - _M_wavelength_mean_path[0]);

    for(int i=0;i<_M_Mean_path_Surface_Blood.size();i++)
    {
        _M_Mean_path_Surface_Blood[i] = _M_Mean_path_Surface_Blood[i].mid(wavelength_mp_idx0,_M_wavelength_camera[_M_id_camera].size());
        _M_Mean_path_Buried_Blood[i] = _M_Mean_path_Buried_Blood[i].mid(wavelength_mp_idx0,_M_wavelength_camera[_M_id_camera].size());
    }
}

//Load spectral resultant
void PPixelWiseMolarCoeff::_Load_spectral_resultant()
{
    qDebug()<<"[PPixelWiseMolarCoeff] in _Load_spectral_resultant start";

    qDebug()<<"ID camera: "<<_M_id_camera;


    // Get wavelength idx of the camera
    // Add a flag or message (non corresponding wavelength)
    if(_M_wavelength_camera[_M_id_camera][0] < _M_wavelength_light[_M_id_light_source][0] ||
            _M_wavelength_camera[_M_id_camera][_M_wavelength_camera[_M_id_camera].size()-1] > _M_wavelength_light[_M_id_light_source][_M_wavelength_light[_M_id_light_source].size()-1])
    {
        qDebug()<<"[PPixelWiseMolarCoeff::_Load_spectral_resultant] camera and light source wavelengths do not match";
        return;
    }
    if(_M_wavelength_camera[_M_id_camera][0] < _M_wavelength_epsilon[0] ||
            _M_wavelength_camera[_M_id_camera][_M_wavelength_camera[_M_id_camera].size()-1] > _M_wavelength_epsilon[_M_wavelength_epsilon.size()-1])
    {
        qDebug()<<"[PPixelWiseMolarCoeff::_Load_spectral_resultant] camera and epsilon wavelengths do not match";
        return;
    }

    int wavelength_eps_idx0 = int(_M_wavelength_camera[_M_id_camera][0] - _M_wavelength_epsilon[0]);
    int wavelength_light_idx0 = int(_M_wavelength_camera[_M_id_camera][0] - _M_wavelength_light[_M_id_light_source][0]);


    //Init spectral resultant vectors
    _M_eps_Hb_x_res_spectral.clear();
    _M_eps_HbO2_x_res_spectral.clear();
    _M_eps_oxCCO_x_res_spectral.clear();
    // nb of detectors
    _M_eps_Hb_x_res_spectral.resize(_M_camera_spectral_sensitivities[_M_id_camera].size());
    _M_eps_HbO2_x_res_spectral.resize(_M_camera_spectral_sensitivities[_M_id_camera].size());
    _M_eps_oxCCO_x_res_spectral.resize(_M_camera_spectral_sensitivities[_M_id_camera].size());


    //Loop over spectral detector
    for(int c=0;c<_M_camera_spectral_sensitivities[_M_id_camera].size();c++)
    {
        QVector<double> channel_sensitivity;

        //Loop over wavelengths
        //Mutliply light source to camera sensitivities
        for(int w=0;w<_M_camera_spectral_sensitivities[_M_id_camera][c].size();w++)
            channel_sensitivity.push_back(_M_camera_spectral_sensitivities[_M_id_camera][c][w] * _M_light_source_spectra[_M_id_light_source][wavelength_light_idx0+w]);

        //Normalize by its sum
        double sum = get_Vector_sum(channel_sensitivity);
        channel_sensitivity = channel_sensitivity/sum;

        //Loop over wavelengths
        for(int w=0;w<_M_camera_spectral_sensitivities[_M_id_camera][c].size();w++)
        {
            _M_eps_HbO2_x_res_spectral[c].push_back(_M_epsilon_HbO2[wavelength_eps_idx0+w]*channel_sensitivity[w]);
            _M_eps_Hb_x_res_spectral[c].push_back(_M_epsilon_Hb[wavelength_eps_idx0+w]*channel_sensitivity[w]);
            _M_eps_oxCCO_x_res_spectral[c].push_back(_M_epsilon_oxCCO[wavelength_eps_idx0+w]*channel_sensitivity[w]);
        }
    }

    qDebug()<<"[PPixelWiseMolarCoeff] in _Load_spectral_resultant end";
}

//ini data
void PPixelWiseMolarCoeff::_initData()
{
    qDebug()<<"PPixelWiseMolarCoeff::_initData start";
    //If initialisation has already been done
    if(!_M_init_or_process)
        return;

    //init
    _M_Molar_coeff.clear();

    //Load spectral resultant
    _Load_spectral_resultant();

    //Load Mean path length
    _Load_Mean_path_Length();

    _M_img_Buried_Blood_preprocessed    = Mat::zeros(0,0,CV_8UC1);
    _M_img_Surface_blood_preprocessed   = Mat::zeros(0,0,CV_8UC1);


    //only use spectral bands included in the selected spectral range (hyperspectral imaging)
    if(_M_id_camera!=Camera_RGB_Basler)
    {
        QVector<QVector<double> > eps_Hb_x_res_spectral;
        QVector<QVector<double> > eps_HbO2_x_res_spectral;
        QVector<QVector<double> > eps_oxCCO_x_res_spectral;

        eps_Hb_x_res_spectral = _M_eps_Hb_x_res_spectral;
        eps_HbO2_x_res_spectral = _M_eps_HbO2_x_res_spectral;
        eps_oxCCO_x_res_spectral = _M_eps_oxCCO_x_res_spectral;

        _M_eps_Hb_x_res_spectral.clear();
        _M_eps_HbO2_x_res_spectral.clear();
        _M_eps_oxCCO_x_res_spectral.clear();

        for(int i=0;i<_M_spectral_idx.size();i++)
        {
            _M_eps_Hb_x_res_spectral.push_back(eps_Hb_x_res_spectral[_M_spectral_idx[i]]);
            _M_eps_HbO2_x_res_spectral.push_back(eps_HbO2_x_res_spectral[_M_spectral_idx[i]]);
            _M_eps_oxCCO_x_res_spectral.push_back(eps_oxCCO_x_res_spectral[_M_spectral_idx[i]]);
        }
    }


    //Surface blood vessel
    QVector<Mat> matrix_sum = _Compute_MBLL_Matrix(_M_Mean_path_Surface_Blood[_M_Mean_path_Surface_Blood.size()-1]);
    //inverse matrix
    invertMatrix(matrix_sum,_M_Molar_coeff_SurfaceBlood);


    //Buried blood vessel
    matrix_sum = _Compute_MBLL_Matrix(_M_Mean_path_Buried_Blood[_M_Mean_path_Buried_Blood.size()-1]);
    //inverse matrix
    invertMatrix(matrix_sum,_M_Molar_coeff_BuriedBlood);


    //Grey matter
    matrix_sum = _Compute_MBLL_Matrix(_M_Mean_path_Surface_Blood[0]);
    //inverse matrix
    invertMatrix(matrix_sum,_M_Molar_coeff_GM);


    for(int i=0;i<_M_pixels_pos.size();i++)
        _M_Molar_coeff.push_back(_M_Molar_coeff_GM);



     _M_init_or_process = false;

    //Write GM Molar coeff
    QString saving_dir  =QString(_M_result_directory)+"MBLL_Matrices/";
    QDir dir(saving_dir);
    if(!dir.exists())
        dir.mkdir(saving_dir);

    QString camera_name ="";

    switch (_M_id_camera) {
    case Camera_RGB_Basler:
        camera_name = "RGB_BASLER";
        break;
    case Camera_HS_Ximea_corr:
        camera_name = "HS_XIMEA_corr";
        break;
    case Camera_HS_Ximea_no_corr:
        camera_name = "HS_XIMEA_no_corr";
        break;
    default:
        camera_name = "RGB_BASLER";
        break;
    }

    for(int i=0;i<_M_Molar_coeff_GM.size();i++)
    {
        QString filename = saving_dir+"E_inv_GM_"+camera_name+"_"+QString::number(i)+".txt";
        WriteDoubleImg(filename,_M_Molar_coeff_GM[i]);

        filename = saving_dir+"E_GM_"+camera_name+"_"+QString::number(i)+".txt";
        WriteDoubleImg(filename,matrix_sum[i]);
    }

    qDebug()<<"PPixelWiseMolarCoeff::_initData end";
    return;
}





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
    qDebug()<<"[PPixelWiseMolarCoeff::initMolarCoeff] initMolarCoeff. Thread is running ? "<<isRunning();
    _M_init_or_process = true; //true: init, false: process
    this->start();
}


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

    //Clear distance map
    _M_distance_map.clear();

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

//    imwrite(QString(_M_result_directory).toStdString()+"Surface_blood.png",_M_img_Surface_blood_preprocessed);
//    imwrite(QString(_M_result_directory).toStdString()+"Buried_blood.png",_M_img_Buried_Blood_preprocessed);

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
            //Distance to blood vessel
            _M_distance_map.push_back(0);
            continue;
        }

        //Buried blood
        if(_M_img_Buried_Blood_preprocessed.at<uchar>(_M_pixels_pos[pos].y,_M_pixels_pos[pos].x) == 255)
        {
            _M_id_BuriedBlood.push_back(pos);
            _M_Molar_coeff.push_back(_M_Molar_coeff_BuriedBlood);
            //Distance to blood vessel
            _M_distance_map.push_back(0);
            continue;
        }
        //unused pixel
        if(_M_non_used.at<uchar>(_M_pixels_pos[pos].y,_M_pixels_pos[pos].x) == 255)
        {
            _M_Molar_coeff.push_back(empty_matrix);
            //Distance to blood vessel
            _M_distance_map.push_back(0);
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

        //Distance to blood vessel
        _M_distance_map.push_back(min_dist);


        //GM mean path
        QVector<Mat> Molar_coeff;
        QVector<double> mean_path_GM;
        _ChooseMeanPathGM(mean_path_GM,min_dist,tissue_type);
        //Compute MBLL matrix
        matrix_sum = _Compute_MBLL_Matrix(mean_path_GM);
        //inverse matrix
        invertMatrix(matrix_sum,Molar_coeff);

        //Set molar coeff matrix
        _M_Molar_coeff.push_back(Molar_coeff);
    }
}


/** Get ditance map (in mm) between grey matter points and blood vessels at pos id */
float PPixelWiseMolarCoeff::getDistanceMap_GreyMatter_BloodVessel(int pos)
{
    if(_M_distance_map.empty())
        return 0;
    if(pos>= _M_distance_map.size())
        return 0;

    return _M_distance_map[pos];
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


//Compute MBLL matrix
QVector<Mat> PPixelWiseMolarCoeff::_Compute_MBLL_Matrix(QVector<double> &mean_path)
{
    QVector<Mat> matrix_sum;

    if(_M_id_camera == Camera_HS_Ximea_corr || _M_id_camera == Camera_HS_Ximea_no_corr)
    {
        if(_M_optimize_HS_quantification)
        {
            if(_M_quantify_oxCCO)
                integrateDatas_Hb_HbO2_oxCCO(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_eps_oxCCO_x_res_spectral,mean_path,_M_HbO2_Hb_oxCCO_HS_cam_idx);
            else
                integrateDatas(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,mean_path,_M_HbO2_Hb_HS_cam_idx);

        }
        else
        {
            if(_M_quantify_oxCCO)
                integrateDatas_Hb_HbO2_oxCCO(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,_M_eps_oxCCO_x_res_spectral,mean_path);
            else
                integrateDatas(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,mean_path);
        }
    }
    else
        integrateDatas(matrix_sum,_M_eps_Hb_x_res_spectral,_M_eps_HbO2_x_res_spectral,mean_path);


    qDebug()<<"PPixelWiseMolarCoeff::_Compute_MBLL_Matrix, matrix size: "<<matrix_sum[0].rows<<" "<<matrix_sum[0].cols;
    return matrix_sum;
}


/** Set Optical device config */
void PPixelWiseMolarCoeff::setOpticalConfigIDx(int camera_id, int light_source_id)
{
    _M_id_light_source = light_source_id;

    _M_id_camera = camera_id;
    _M_init_or_process = true; //true: init, false: process
    _initData();

    if(_M_id_camera==Camera_RGB_Basler)
        initMolarCoeff();
    else
        setSpectralRange(_M_HS_lambda_min,_M_HS_lambda_max);
}



/** Hyperspectral spectral range */
void  PPixelWiseMolarCoeff::setSpectralRange(int lambda_min,int lambda_max)
{
    _M_HS_lambda_min = lambda_min;
    _M_HS_lambda_max = lambda_max;

    qDebug()<<"Lambda min max: "<<_M_HS_lambda_min<<" "<<_M_HS_lambda_max;



    //Get HS wavelength idx
    _M_spectral_idx.clear();
    for(int i=0;i<_M_camera_spectral_sensitivities[_M_id_camera].size();i++)
    {
        //Convert to std vector
        std::vector<float> v(_M_camera_spectral_sensitivities[_M_id_camera][i].constBegin(), _M_camera_spectral_sensitivities[_M_id_camera][i].constEnd());
        int w_idx = argMax(v);
        if(_M_wavelength_camera[_M_id_camera][w_idx]<=_M_HS_lambda_max && _M_wavelength_camera[_M_id_camera][w_idx]>=_M_HS_lambda_min)
            _M_spectral_idx.push_back(i);
    }

    emit newWavelengthIDX(_M_spectral_idx);

    initMolarCoeff();
}

