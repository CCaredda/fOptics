#include "PPixelWiseMolarCoeff.h"
#include "molarcoefffunctions.h"
#include "acquisition.h"




PPixelWiseMolarCoeff::PPixelWiseMolarCoeff(QObject *parent) : QThread(parent)
{
    //Init result directory
    _M_result_directory =  "/home/results/";



    _M_init_or_process = true; //true: init, false: process



    //Extinction spectra  (in cm-1.mol-1.L)
    _M_epsilon_Hb.clear();
    _M_epsilon_HbO2.clear();
    _M_wavelength_epsilon.clear();

    QString share_dir = getShareDirPath("share")+"/";
    // QString share_dir = QString(QCoreApplication::applicationDirPath())+"/../share/;

    ReadVector(share_dir+"files/extinction_coefficients/eps_HbO2.txt",_M_epsilon_HbO2);
    ReadVector(share_dir+"files/extinction_coefficients/eps_Hb.txt",_M_epsilon_Hb);
    ReadVector(share_dir+"files/extinction_coefficients/wavelength_eps.txt",_M_wavelength_epsilon);



    //Light source spectra (normalized in %)
    _M_id_light_source = Light_source_Halogen;
    _M_light_source_spectra.clear();
    _M_wavelength_light.clear();
    QVector<double> temp;
    ReadVector(share_dir+"files/Light_sources/Halogen_spectra.txt",temp);
    //Normalize to max
    NormalizeMaxVector(temp);
    _M_light_source_spectra.push_back(temp);
    temp.clear();
    ReadVector(share_dir+"files/Light_sources/wavelength_halogen.txt",temp);
    _M_wavelength_light.push_back(temp);



    //Camera spectral sensitivities  (normalized in %)
    _M_camera_spectral_sensitivities.clear();
    _M_wavelength_camera.clear();
    _M_id_camera = Camera_RGB_Basler;

    //RGB Basler
    QVector<QVector<double> > matrix_temp;
    load_Matrix(share_dir+"files/Cameras/RGB_Basler/cam_RGB_Basler.txt",matrix_temp);
    _M_camera_spectral_sensitivities.push_back(matrix_temp);
    temp.clear();
    ReadVector(share_dir+"files/Cameras/RGB_Basler/wavelength_RGB_Basler.txt",temp);
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



    //pixels pos
    _M_pixels_pos.clear();

    //Molar coeff
    _M_Molar_coeff.clear();
    _M_Molar_coeff_BuriedBlood.clear();
    _M_Molar_coeff_GM.clear();
    _M_Molar_coeff_SurfaceBlood.clear();



    qDebug()<<"init data";
    _initData();
    qDebug()<<"end init data";



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



//get number of chromophore quantified
int PPixelWiseMolarCoeff::getChromophoreNumber()
{
    return 2;
}




//Protected function run in a parallel thread
void PPixelWiseMolarCoeff::run()
{
    //init data
    _initData();

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

    QString share_dir = getShareDirPath("share")+"/";
    // QString share_dir = QString(QCoreApplication::applicationDirPath())+"/../share/;

    load_Matrix(share_dir+"files/Mean_path_length/Mean_path_Surface_Blood_Vessel.txt",_M_Mean_path_Surface_Blood);
    load_Matrix(share_dir+"files/Mean_path_length/Mean_path_Buried_Blood_Vessel.txt",_M_Mean_path_Buried_Blood);
    ReadVector(share_dir+"files/Mean_path_length/wavelength_Mean_path.txt",_M_wavelength_mean_path);


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
    // nb of detectors
    _M_eps_Hb_x_res_spectral.resize(_M_camera_spectral_sensitivities[_M_id_camera].size());
    _M_eps_HbO2_x_res_spectral.resize(_M_camera_spectral_sensitivities[_M_id_camera].size());


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



    //only use spectral bands included in the selected spectral range (hyperspectral imaging)
    if(_M_id_camera!=Camera_RGB_Basler)
    {
        QVector<QVector<double> > eps_Hb_x_res_spectral;
        QVector<QVector<double> > eps_HbO2_x_res_spectral;

        eps_Hb_x_res_spectral = _M_eps_Hb_x_res_spectral;
        eps_HbO2_x_res_spectral = _M_eps_HbO2_x_res_spectral;

        _M_eps_Hb_x_res_spectral.clear();
        _M_eps_HbO2_x_res_spectral.clear();

        for(int i=0;i<_M_spectral_idx.size();i++)
        {
            _M_eps_Hb_x_res_spectral.push_back(eps_Hb_x_res_spectral[_M_spectral_idx[i]]);
            _M_eps_HbO2_x_res_spectral.push_back(eps_HbO2_x_res_spectral[_M_spectral_idx[i]]);
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

    initMolarCoeff();

}



