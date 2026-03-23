#include "HdisplayResult.h"
#include "ui_HdisplayResult.h"

HdisplayResult::HdisplayResult(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HdisplayResult)
{
    ui->setupUi(this);

    //Z threshold RFT
    _M_z_thresh = 0.0;

    //Mode RGB or Hyperspectra
    _M_RGB_mode = true; //RGB
    onnewRGBMode(true);


    //last processed id
    _M_last_processed_id = -1;

    //Display contour
    connect(ui->_display,SIGNAL(analysisZoneisDrawn(bool)),ui->_display_ROI,SLOT(setChecked(bool)));
    connect(ui->_display_ROI,SIGNAL(clicked(bool)),ui->_display,SLOT(onDrawAnalysisZone(bool)));

    //grey the outside of the contour
    connect(ui->_grey_outside,SIGNAL(clicked(bool)),ui->_display,SLOT(onGreyOutsideContourIsRequested(bool)));
    connect(ui->_grey_outside,SIGNAL(clicked(bool)),this,SIGNAL(GreyOutsideContourIsRequested(bool)));

    //ROI Extraction type
    connect(this,SIGNAL(newROI_Extraction_Type(int)),ui->_display,SLOT(onnewROI_Extraction_Type(int)));

    //Rect ROI drawing
    connect(this,SIGNAL(RectROIRequested()),ui->_display,SLOT(onRectROIRequested()));
    connect(ui->_display,SIGNAL(newRectROI(QPoint,QPoint)),this,SIGNAL(newRectROI(QPoint,QPoint)));

    //Save img results
    //connect(ui->_save_results,SIGNAL(clicked(bool)),this,SIGNAL(requestSaveResults(bool)));
    connect(ui->_save_results,SIGNAL(clicked(bool)),ui->_display,SLOT(onrequestSaveResults(bool)));

    //alpha for image display
    connect(ui->_alpha_val,SIGNAL(valueEdited(double)),ui->_display,SLOT(onNewAlphaValue(double)));
    ui->_alpha_val->setRange(0,1,"");
    ui->_alpha_val->setValue(0.5);

    //Request line drawing
    connect(this,SIGNAL(RequestLineDrawing()),ui->_display,SLOT(RequestLineDrawing()));
    connect(ui->_display,SIGNAL(newLine(QPoint,QPoint)),this,SIGNAL(newLine(QPoint,QPoint)));

//    //New display image
//    connect(this,SIGNAL(newImage(Mat)),ui->_display,SLOT(newImage(Mat)));

    //Clear image requested
    connect(this,SIGNAL(ClearImg()),ui->_display,SLOT(ClearImg()));

    //Analysis zone drawing
    connect(this,SIGNAL(onAnalysisZoneRequested(int)),ui->_display,SLOT(onAnalysisZoneRequested(int)));
    connect(ui->_display,SIGNAL(newAnalysisZone(QVector<QPoint>,cv::Size)),this,SLOT(onnewAnalysisZone(QVector<QPoint>,cv::Size)));
    _M_mask_contrast_img = Mat::zeros(0,0,CV_8UC1);

    //Send initial img
    connect(ui->_display,SIGNAL(newInitialImg(Mat)),this,SIGNAL(newInitialImg(Mat)));

    // Contrast img
    _M_contrast_img.clear();

    // Cartography type changed
    _M_carto_changed    = true;

    //Min Max value display
    ui->_max_value->setIntegerMode(false);
    ui->_max_value->setRange(0,1000000,"uMol/L");
    ui->_max_value->setValue(1);
    connect(ui->_max_value,SIGNAL(valueEdited(double)),this,SLOT(onMinMaxDisplayValueChanged()));
    connect(ui->_max_value,SIGNAL(valueEdited(double)),this,SIGNAL(newDisplayValue(double)));
    onMinMaxDisplayValueChanged();

    //Median Filter
    connect(ui->_Median_Filter,SIGNAL(clicked(bool)),this,SLOT(onMedianFilterON(bool)));
    ui->_Median_Filter->setChecked(true);
    onMedianFilterON(true);

    //Window size (for median filter)
    connect(ui->_window_size,SIGNAL(valueEdited(double)),this,SLOT(onWindowSizeChanged(double)));
    ui->_window_size->setRange(3,30,"px");
    ui->_window_size->setIntegerMode();
    ui->_window_size->setValue(5);
    ui->_display->onWindowSizeChanged(5);

    //Display mode
    updateCartographyMode();

    //Cut off display
    connect(ui->_cutoff,SIGNAL(sliderReleased()),this,SLOT(onnewCutoffValue()));
    ui->_cutoff->setValue(0);
    onnewCutoffValue();
    _M_cutoff=0;
    _M_corr_coef=0;

    //update color bar
    _UpdateColorBar();

    //Update contrast list
    connect(ui->_display_type,SIGNAL(currentIndexChanged(int)),this,SLOT(onContrastImgchanged()));

    //point selected
    connect(ui->_display,SIGNAL(PointSelected(Point)),this,SIGNAL(PointSelected(Point)));
    //MEAN ROI
    connect(this,SIGNAL(newMeanROIRadius(double)),ui->_display,SLOT(onnewMeanROIRadius(double)));
    connect(this,SIGNAL(requestMeanROI(bool)),ui->_display,SLOT(onrequestMeanROI(bool)));


    //Processing type
    _M_processing_type = Process_Mean_Delta_C;

    /************************************************************************
     ************************************************************************
     *********************Define activated cortical area*********************
     ************************************************************************/

    //Activation maps
    connect(this,SIGNAL(newActivationMap(QVector<bool>)),ui->_display,SLOT(onnewActivationMap(QVector<bool>)));
    connect(this,SIGNAL(newActivationMap(QVector<bool>,int)),ui->_display,SLOT(onnewActivationMap(QVector<bool>,int)));

    //Statistic zone
    connect(this,SIGNAL(newSettingStatisticZone(double)),ui->_display,SLOT(onnewSettingStatisticZone(double)));

    //new Statistic mask
    connect(ui->_display,SIGNAL(newCorticalAreaDefinition(Rect,QVector<Rect>,QVector<Mat>)),this,SIGNAL(newCorticalAreaDefinition(Rect,QVector<Rect>,QVector<Mat>)));
    connect(ui->_display,SIGNAL(newCorticalAreaDefinition(QVector<Rect>,QVector<Mat>)),this,SIGNAL(newCorticalAreaDefinition(QVector<Rect>,QVector<Mat>)));

    /************************************************************************
     ************************************************************************
     *********************Resting state**************************************
     ************************************************************************/
    _M_resting_states_maps.clear();
    _M_grid_seed = false;
    _M_enable_resting_states = false;
    _M_resting_state_seed_connexion = false;
    _M_id_source_IC_resting_state = 0;
    _M_resting_state_method = 0; //Seed based method

    //init seeds
    connect(this,SIGNAL(requestSeedInit()),ui->_display,SLOT(onrequestSeedInit()));

    //Seed radius
    connect(this,SIGNAL(newSeedRadius(double)),ui->_display,SLOT(onnewSeedRadius(double)));

    //Grid seeds
    connect(this,SIGNAL(newGridSeeds(bool)),this,SLOT(onnewGridSeeds(bool)));

//    //request seeds
//    connect(this,SIGNAL(requestSeeds()),ui->_display,SLOT(onrequestSeeds()));

    //new seeds
    connect(ui->_display,SIGNAL(newRestingStateSeeds(QVector<Mat>)),this,SIGNAL(newRestingStateSeeds(QVector<Mat>)));
}


void HdisplayResult::onNewResultDirectory(QString v)
{
    ui->_display->onNewResultDirectory(v);
}



HdisplayResult::~HdisplayResult()
{
    delete ui;
}



/** New stat type (SPM, T test, ...) */
void HdisplayResult::onnewStatType(int v)
{
    ui->_display->onnewStatType(v);

    _M_processing_type = v;





//    onMinMaxDisplayValueChanged();
//    onnewCutoffValue();
}




//Set HMI mode (user, guru)
void HdisplayResult::onNew_Guru_Mode(bool v)
{
    if(v)
    {
        ui->groupBox_Filter->show();
        ui->_widget_advanced_settings->show();
    }
    else
    {
        ui->groupBox_Filter->hide();
        ui->_widget_advanced_settings->hide();
    }
}


void HdisplayResult::onnewGridSeeds(bool v)
{
    ui->_display->onnewGridSeeds(v);
    _M_grid_seed = v;
}


//set Analysis zone
void HdisplayResult::setAnalysisZone(QVector<QPoint> v,cv::Size s)
{
    ui->_display->setAnalysisZone(v,s);
    //size
    cv::Size size=s;
    _M_mask_contrast_img = Mat::zeros(size,CV_8UC1);

    //ROI
    vector<vector<Point> > contour;
    contour.resize(1);

    for(int i=0;i<v.size();i++)
        contour[0].push_back(Point(v[i].x(),v[i].y()));

    //compute mask
    drawContours(_M_mask_contrast_img,contour,0,Scalar(255),CV_FILLED);
}

//new analysis zone
void HdisplayResult::onnewAnalysisZone(QVector<QPoint> v,cv::Size s)
{
    emit newAnalysisZone(v,s);

    //size
    cv::Size size=s;
    _M_mask_contrast_img = Mat::zeros(size,CV_8UC1);

    //ROI
    vector<vector<Point> > contour;
    contour.resize(1);

    for(int i=0;i<v.size();i++)
        contour[0].push_back(Point(v[i].x(),v[i].y()));

    //compute mask
    drawContours(_M_mask_contrast_img,contour,0,Scalar(255),CV_FILLED);
}

//mode RGB or Hyperspectral
void HdisplayResult::onnewRGBMode(bool v)
{
    _M_RGB_mode = v;
    updateContrastList();
}

//Update chromophore list
void HdisplayResult::updateContrastList()
{
    updateCartographyMode();
}


//enabe resting state
void HdisplayResult::onenableRestingState(bool v)
{
    _M_enable_resting_states = v;
    ui->_display->onenableRestingState(v);
}

//resting states maps
void HdisplayResult::onnewRestingStateMaps(QVector<QVector<Mat> > v)
{
    _M_resting_states_maps.clear();
    _M_resting_states_maps = v;

    _ContrastImg_RestingState();
}

void HdisplayResult::onnewKmeans_RestingStateMaps(QVector<Mat> v)
{
    _M_resting_states_maps.clear();
    _M_resting_states_maps.push_back(v);
    _ContrastImg_RestingState();
}

//resting state apply seed connexion
void HdisplayResult::_Get_Seed_Connexion(Mat &mask,int seed_id,Mat &falseColormap)
{
//    Mat maskf       = Mat::zeros(mask.size(),mask.type());
    falseColormap   = Mat::zeros(mask.size(),CV_8UC3);

    QVector<Point> seeds_point = ui->_display->getSeeds_Point();

    vector<vector<Point> > c;
    findContours(mask,c,CV_RETR_LIST,CV_CHAIN_APPROX_NONE);
    mask = Mat::zeros(mask.size(),CV_8UC1);

    for(unsigned int i=0;i<c.size();i++)
    {
        if(pointPolygonTest(c[i],seeds_point[seed_id],false)>0)
        {
            vector<vector<Point> > c_temp;
            c_temp.push_back(c[i]);


//            Mat hsv(1,1, CV_8UC3, Scalar(rand()% 180,255,180));
//            Mat bgr;
//            cvtColor(hsv, bgr, CV_HSV2BGR);
//            Scalar color = Scalar(bgr.data[0], bgr.data[1], bgr.data[2]);

            Scalar color = ui->_display->getSeedColor(seed_id);
            drawContours(falseColormap,c_temp,0,color,CV_FILLED);
            drawContours(mask,c_temp,0,Scalar(255),CV_FILLED);
        }
    }



//    maskf.copyTo(mask);


}

//New resting state method
void HdisplayResult::onnewRestingStateMethod(int v)
{
    //init seeds
    _M_resting_state_method = v;
    ui->_display->onrequestSeedInit();
    ui->_display->setNewRestingStateMethod(v);
}

void HdisplayResult::onnewICASourceofInterest(int v)
{
    _M_id_source_IC_resting_state = v;
    _ContrastImg_RestingState();
}

//Seed extraction: Thee new seed become the resting state map in which the seed is included
void HdisplayResult::onrequestSeedsExtraction()
{
    if(!_M_resting_state_seed_connexion)
        return;

    if(_M_resting_states_maps.empty())
        return;

    QVector<Mat> new_seeds;

    //Get mask
    for(int id_map=0;id_map<_M_resting_states_maps.size();id_map++)
    {
        Mat in      = _M_resting_states_maps[id_map][ui->_display_type->currentIndex()];
        Mat mask    = Mat::zeros(in.size(),CV_8UC1);
        for(int row=0;row<in.rows;row++)
        {
            uchar * ptr_mask    = mask.ptr<uchar>(row);
            float *ptr_f        = in.ptr<float>(row);

            for(int col=0;col<in.cols;col++)
            {
                //Check positive and negative correlation
                if((ptr_f[col]>=_M_cutoff) || (ptr_f[col]<=-_M_cutoff))
                    ptr_mask[col]=255;
            }
        }
        if(countNonZero(mask)>0)
            new_seeds.push_back(mask);
    }

    if(!new_seeds.empty())
    {
        qDebug()<<"[HdisplayResult::onrequestSeedsExtraction] new seed emited";
        emit newRestingStateSeeds(new_seeds);
    }
    else
        qDebug()<<"[HdisplayResult::onrequestSeedsExtraction] empty seeds";
}

//Resting state: request seed connexion (if true: only display corralated points attached to the seed)
void HdisplayResult::onSeedConnexionRequested(bool v)
{
    _M_resting_state_seed_connexion = v;
    _ContrastImg_RestingState();
}

//process color maps
Mat HdisplayResult::_Process_Color_Maps(Mat in)
{
    double min, max;
    minMaxLoc(in, &min, &max);
    double max_ui = ui->_max_value->value();


    Mat img_contrast    = Mat::zeros(in.size(),CV_8UC1);
    Mat maskf           = Mat::zeros(in.size(),CV_8UC1);

    for(int row=0;row<img_contrast.rows;row++)
    {
        uchar *ptr_u        = img_contrast.ptr<uchar>(row);
        uchar * ptr_mask    = maskf.ptr<uchar>(row);
        float *ptr_f        = in.ptr<float>(row);

        for(int col=0;col<img_contrast.cols;col++)
        {

            //Ajout de 127 (255/2) qui correspond au 0
            //On normalise les valeurs entre 0->maxval à 127->255
            int v       = (ptr_f[col] * (255-127)/(max*max_ui) + 127);
            v           = (v>255) ? 255 : v;
            ptr_u[col]  = uchar(v);

            //Check positive and negative correlation
            if((ptr_f[col]>=_M_cutoff*max) || (ptr_f[col]<=-_M_cutoff*max))
                ptr_mask[col]=255;

        }
    }

    //bitwise and between maskf and the img mask
    bitwise_and(maskf,_M_mask_contrast_img,maskf);

    //Apply seed connextion
    Mat falseColorsMap;
    applyColorMap(img_contrast, falseColorsMap, COLORMAP_JET);



    for(int row=0;row<falseColorsMap.rows;row++)
    {
        _Mycolor *ptr =falseColorsMap.ptr<_Mycolor>(row);
        uchar *ptr_mask =maskf.ptr<uchar>(row);
        for(int col=0;col<falseColorsMap.cols;col++)
        {
            if(ptr_mask[col]==0)
            {
                ptr[col].b=0;
                ptr[col].g=0;
                ptr[col].r=0;
            }
        }
    }
    return falseColorsMap;
}


//Process resting state map i (seed based method)
void HdisplayResult::_Process_RestingStateMap_Seeds(Mat in,Mat &out,int seed_id)
{

    Mat img_contrast    = Mat::zeros(in.size(),CV_8UC1);
    Mat maskf           = Mat::zeros(in.size(),CV_8UC1);

    for(int row=0;row<img_contrast.rows;row++)
    {
        uchar *ptr_u        = img_contrast.ptr<uchar>(row);
        uchar * ptr_mask    = maskf.ptr<uchar>(row);
        float *ptr_f        = in.ptr<float>(row);

        for(int col=0;col<img_contrast.cols;col++)
        {

            //Ajout de 127 (255/2) qui correspond au 0
            //On normalise les valeurs entre 0->maxval à 127->255
            int v       = (ptr_f[col] * (255-127) + 127);
            v           = (v>255) ? 255 : v;
            ptr_u[col]  = uchar(v);

            //Check positive and negative correlation
            if((ptr_f[col]>=_M_cutoff) || (ptr_f[col]<=-_M_cutoff))
                ptr_mask[col]=255;

        }
    }

    //bitwise and between maskf and the img mask
    bitwise_and(maskf,_M_mask_contrast_img,maskf);

    //Apply seed connextion
    Mat falseColorsMap;
    if(_M_resting_state_seed_connexion)
        _Get_Seed_Connexion(maskf,seed_id,falseColorsMap);
    else
        applyColorMap(img_contrast, falseColorsMap, COLORMAP_JET);

    //If grid seed is not selected do not process colored seeds maps but apply colormap
    if(!_M_grid_seed)
        applyColorMap(img_contrast, falseColorsMap, COLORMAP_JET);

    for(int row=0;row<out.rows;row++)
    {
        _Mycolor *ptr       = falseColorsMap.ptr<_Mycolor>(row);
        _Mycolor *ptr_out   = out.ptr<_Mycolor>(row);
        uchar *ptr_mask     = maskf.ptr<uchar>(row);
        for(int col=0;col<out.cols;col++)
        {
            if(ptr_mask[col]!=0)
            {
                ptr_out[col].b = ptr[col].b;
                ptr_out[col].g = ptr[col].g;
                ptr_out[col].r = ptr[col].r;
            }
        }
    }
}

void HdisplayResult::_ContrastImg_RestingState()
{

    if(_M_resting_states_maps.empty())
        return;

    Mat falseColorsMap = Mat::zeros(_M_resting_states_maps[0][0].size(),CV_8UC3);

    switch (_M_resting_state_method)
    {
    //Seed based method
    case 0:
        //cumulate the resting state maps
        for(int i=0;i<_M_resting_states_maps.size();i++)
        {
            Mat in  = _M_resting_states_maps[i][ui->_display_type->currentIndex()];
            _Process_RestingStateMap_Seeds(in,falseColorsMap,i);
        }
        break;
    //ICA based method
    case 1:
        falseColorsMap = _Process_Color_Maps(_M_resting_states_maps[ui->_display_type->currentIndex()][_M_id_source_IC_resting_state]);
        break;
//    case 2:
//        falseColorsMap = _Process_Color_Maps(_M_resting_states_maps[0][ui->_display_type->currentIndex()]);
//        break;
//    case 3:
//        falseColorsMap = _M_resting_states_maps[0][ui->_display_type->currentIndex()];
//        break;
    //Seed method with SPM
    case 2:
        falseColorsMap = _Process_ActivationMap(_M_resting_states_maps[0][ui->_display_type->currentIndex()]);
        cvtColor(falseColorsMap,falseColorsMap,CV_BGR2RGB);
        break;
    default:
        break;
    }


    ui->_display->onNewContrastImage(falseColorsMap);
    ui->_display->repaint();
}


//Update color bar display
void HdisplayResult::_UpdateColorBar()
{
    //Colorbar
    Mat colorbar    = Mat::zeros(256,5,CV_8UC1);
    Mat falsecolor;
    for(int row=0;row<colorbar.rows;row++)
    {
        uchar *ptr=colorbar.ptr<uchar>(row);
        for(int col=0;col<colorbar.cols;col++)
            ptr[col] = 255-row;
    }
    applyColorMap(colorbar,falsecolor,COLORMAP_JET);



    //Create grey rgb image
    Mat black       = Mat::zeros(256,5,CV_8UC3);
    Mat falsecolor2 = Mat::zeros(256,5,CV_8UC3);

    //Addweight
    double alpha = 0.65;
    addWeighted( falsecolor, alpha,black, 1.0-alpha,0.0,falsecolor2);


    int start   = (int)(127 -127*_M_cutoff);
    int end     = (int)(127 +128*_M_cutoff);

    //Grise la partie non utilisée

    for(int row=start;row<end;row++)
    {
        _Mycolor *ptr=falsecolor.ptr<_Mycolor>(row);
        _Mycolor *ptr2=falsecolor2.ptr<_Mycolor>(row);

        for(int col=0;col<falsecolor.cols;col++)
        {
            ptr[col]=ptr2[col];
        }
    }



    ui->_colorBar->setInialImage(falsecolor);
    ui->_colorBar->repaint();

}

//new info bulle
void HdisplayResult::setPlotStats(QVector<double> stat)
{
    if(stat.empty())
        return;


    for(int i=0;i<stat.size();i++)
    {
        int temp = stat[i]*100;
        stat[i] = temp/100;
    }



    QStringList txt;
    for(int i=0;i<stat.size();i++)
        txt.append("\t Contrast"+QString::number(i+1)+ "\t");
    txt.append("\n");
    txt.append("Sim to Bold \t ");
    for(int i=0;i<stat.size();i++)
        txt.append(QString::number((stat[i]))+" \t ");

    ui->_display->onNewInfosBulle(txt);

}


//on New Img
void HdisplayResult::onNewImage(Mat img)
{
    ui->_display->newImage(img);
}

void HdisplayResult::onNewInitialImg(Mat v)
{
    ui->_display->setInialImage(v);
}

//on New ROI Rect
void HdisplayResult::onNewRectROI(QPoint p1,QPoint p2)
{
    ui->_display->onNewAutoROI(Point(p1.x(),p1.y()),Point(p2.x(),p2.y()));
}


// Cartography mode proba
void HdisplayResult::_ContrastImg_Mode_Correlation()
{
    float v0        = -1;
    float v1        = 1;
    float vmin      = 0;
    float vmax      = 255;

    Mat img_contrast    = Mat::zeros(_M_contrast_img[0].size(),CV_8UC1);
    Mat maskf           = Mat::zeros(img_contrast.size(),CV_8UC1);
    Mat in              = _M_contrast_img[ui->_display_type->currentIndex()];

    for(int row=0;row<img_contrast.rows;row++)
    {
        uchar *ptr_u        = img_contrast.ptr<uchar>(row);
        uchar * ptr_mask    = maskf.ptr<uchar>(row);
        float *ptr_f        = in.ptr<float>(row);

        for(int col=0;col<img_contrast.cols;col++)
        {
            ptr_u[col]      = (ptr_f[col]-v0)*(vmax-vmin)/(v1-v0) +vmin ;
            ptr_mask[col]   = ((ptr_u[col]>=127*(1+_M_cutoff)) || (ptr_u[col]<=127*(1-_M_cutoff)))? 255 : 0;
        }
    }


    //bitwise and between maskf and the img mask
    bitwise_and(maskf,_M_mask_contrast_img,maskf);

    Mat falseColorsMap;
    applyColorMap(img_contrast, falseColorsMap, COLORMAP_JET);
    for(int row=0;row<falseColorsMap.rows;row++)
    {
        _Mycolor *ptr =falseColorsMap.ptr<_Mycolor>(row);
        uchar *ptr_mask =maskf.ptr<uchar>(row);
        for(int col=0;col<falseColorsMap.cols;col++)
        {
            if(ptr_mask[col]==0)
            {
                ptr[col].b=0;
                ptr[col].g=0;
                ptr[col].r=0;
            }
        }
    }


    ui->_display->onNewContrastImage(falseColorsMap);
    ui->_display->repaint();
}


// Cartography mode mean value
void HdisplayResult::_ContrastImg_Mode_Mean(double min,double max,double cutoff)
{

    int id = ui->_display_type->currentIndex();
    Mat in              = _M_contrast_img[id];
    Mat img_contrast    = Mat::zeros(in.size(),CV_8UC1);
    Mat mask            = Mat::zeros(img_contrast.size(),CV_8UC1);

    for(int row=0;row<img_contrast.rows;row++)
    {
        uchar *ptr_u        = img_contrast.ptr<uchar>(row);
        uchar * ptr_mask    = mask.ptr<uchar>(row);
        float *ptr_f        = in.ptr<float>(row);

        for(int col=0;col<img_contrast.cols;col++)
        {
            if(ptr_f[col]!=0)
            {
                if(ptr_f[col]>0)
                {
                    //Ajout de 127 (255/2) qui correspond au 0
                    //On normalise les valeurs entre 0->maxval à 127->255
                    int v       = (ptr_f[col] * ((255-127)/(max))) + 127;
                    v           = (v>255) ? 255 : v;
                    ptr_u[col]  = uchar(v);

                    if((ptr_u[col]>=127 + (255-127)*cutoff))
                        ptr_mask[col]=255;
                }
                else
                {
                    //On ramène les valeurs négatives supérieure à 0 et inférieure à 127
                    //On normalise les valeurs entre -maxval->0 à 0->127
                    int v       = (ptr_f[col] -min)* (127/(-min));
                    v           = (v<0)? 0 : v;
                    ptr_u[col]  = uchar(v);

                    if((ptr_u[col]<=127 -(255-127)*cutoff))
                        ptr_mask[col]=255;
                }
            }
        }
    }

    //bitwise and between maskf and the img mask
    bitwise_and(mask,_M_mask_contrast_img,mask);

    Mat falseColorsMap;
    applyColorMap(img_contrast, falseColorsMap, COLORMAP_JET);
    for(int row=0;row<falseColorsMap.rows;row++)
    {
        _Mycolor *ptr   = falseColorsMap.ptr<_Mycolor>(row);
        uchar *ptr_mask = mask.ptr<uchar>(row);
        for(int col=0;col<falseColorsMap.cols;col++)
        {
            if(ptr_mask[col]==0)
            {
                ptr[col].b=0;
                ptr[col].g=0;
                ptr[col].r=0;
            }
        }
    }


    if(_M_last_processed_id!=-1)
        ui->_display->onNewContrastImage(falseColorsMap,_M_last_processed_id);
    else
        ui->_display->onNewContrastImage(falseColorsMap);



    ui->_display->repaint();
}


//Map mode SPM
void HdisplayResult::_ContrastImg_Mode_SPM(double min,double max,double cutoff)
{

    int id = ui->_display_type->currentIndex();
    Mat in              = _M_contrast_img[id];
    Mat img_contrast    = Mat::zeros(in.size(),CV_8UC1);
    Mat mask            = Mat::zeros(img_contrast.size(),CV_8UC1);

    for(int row=0;row<img_contrast.rows;row++)
    {
        uchar *ptr_u        = img_contrast.ptr<uchar>(row);
        uchar * ptr_mask    = mask.ptr<uchar>(row);
        float *ptr_f        = in.ptr<float>(row);

        for(int col=0;col<img_contrast.cols;col++)
        {
            if(ptr_f[col]!=0)
            {
                if(ptr_f[col]>0)
                {
                    //Ajout de 127 (255/2) qui correspond au 0
                    //On normalise les valeurs entre 0->maxval à 127->255
                    int v       = (ptr_f[col] * ((255-127)/(max))) + 127;
                    v           = (v>255) ? 255 : v;
                    ptr_u[col]  = uchar(v);

                    if((ptr_u[col]>=127 + (255-127)*cutoff))
                        ptr_mask[col]=255;
                }
                else
                {
                    //On ramène les valeurs négatives supérieure à 0 et inférieure à 127
                    //On normalise les valeurs entre -maxval->0 à 0->127
                    int v       = (ptr_f[col] -min)* (127/(-min));
                    v           = (v<0)? 0 : v;
                    ptr_u[col]  = uchar(v);
                }
            }
        }
    }

    //bitwise and between maskf and the img mask
    bitwise_and(mask,_M_mask_contrast_img,mask);

    Mat falseColorsMap;
    applyColorMap(img_contrast, falseColorsMap, COLORMAP_JET);
    for(int row=0;row<falseColorsMap.rows;row++)
    {
        _Mycolor *ptr   = falseColorsMap.ptr<_Mycolor>(row);
        uchar *ptr_mask = mask.ptr<uchar>(row);
        for(int col=0;col<falseColorsMap.cols;col++)
        {
            if(ptr_mask[col]==0)
            {
                ptr[col].b=0;
                ptr[col].g=0;
                ptr[col].r=0;
            }
        }
    }


    if(_M_last_processed_id!=-1)
        ui->_display->onNewContrastImage(falseColorsMap,_M_last_processed_id);
    else
        ui->_display->onNewContrastImage(falseColorsMap);



    ui->_display->repaint();
}


Mat HdisplayResult::_Process_ActivationMap(Mat map)
{
    //Convert binary image into red and green map
    Mat result = Mat::zeros(map.size(),CV_8UC3);
    for(int row=0;row<map.rows;row++)
    {
        uchar *ptr_in       = map.ptr<uchar>(row);
        _Mycolor *ptr_out   = result.ptr<_Mycolor>(row);
        uchar *ptr_mask     = _M_mask_contrast_img.ptr<uchar>(row);

        for(int col=0;col<map.cols;col++)
        {
//            //Non activated (green)
//            if(ptr_in[col] == 0 && ptr_mask[col]==255)
//                ptr_out[col].g = 255;

//            //Activated (red)
//            if(ptr_in[col] == 255 && ptr_mask[col]==255)
//                ptr_out[col].b = 255;

            //activated (magenta)
            if(ptr_in[col] == 255 && ptr_mask[col] == 255)
            {
                ptr_out[col].r = 255;
                ptr_out[col].g = 0;
                ptr_out[col].b = 255;
            }
        }
    }
    return result;

}


// New activation map
void HdisplayResult::onNewActivationMap(Mat map)
{
    ui->_display->onnewActivationMap(_Process_ActivationMap(map));
}


//On contrast img changed (max value, threshold, etc)
void HdisplayResult::onContrastImgchanged()
{

    //Send new chromophore ID to PAnalyse
    emit newChromophoreID(ui->_display_type->currentIndex());

    int id = ui->_display_type->currentIndex();

    if(_M_contrast_img.empty() && _M_resting_states_maps.empty())
        return;




    if(_M_enable_resting_states)
    {
        _ContrastImg_RestingState();
        return;
    }

    //Get min and max values
    double max=-1,min=-1;

    for(int i=0;i<_M_contrast_img.size();i++)
    {
        double mint,maxt;
        minMaxIdx(_M_contrast_img[i], &mint, &maxt);
        min = (mint<min) ? mint : min;
        max = (maxt>max) ? maxt : max;
    }
    int numerator = 1000000;


    switch (_M_processing_type)
    {
    case Process_Mean_Delta_C:
        max = ui->_max_value->value();
        min = -max;
        max = max/numerator;
        min = min/numerator;
        _ContrastImg_Mode_Mean(min,max,_M_cutoff);
        break;

    case Process_Correlation:
        _ContrastImg_Mode_Correlation();
        break;
    case Activation_GLM_Pixel_wise:
        _ContrastImg_Mode_SPM(min,max,_M_cutoff);
        break;
    default:
        _ContrastImg_Mode_Mean(min,max,_M_cutoff);
        break;
    }

}

void HdisplayResult::onnewContrastImg(QVector<Mat> img)
{
    // New contrast img
    _M_contrast_img.clear();
    _M_contrast_img = img;



    //Display Z-stats with RFT z threshold
    if(_M_processing_type == Activation_GLM_Pixel_wise)
    {
        //Get min and max values
        double max=-1,min=-1;
        for(int i=0;i<_M_contrast_img.size();i++)
        {
            double mint,maxt;
            minMaxIdx(_M_contrast_img[i], &mint, &maxt);
            min = (mint<min) ? mint : min;
            max = (maxt>max) ? maxt : max;
        }

        //Set Cutoff value
        ui->_cutoff->setValue(1000*(_M_z_thresh/max));
        _M_cutoff=(double)ui->_cutoff->value()/1000;

        //update color bar
        _UpdateColorBar();

        //set Min and max values
        ui->_max_value->setValue(max);
        ui->_min_value->setText(QString::number(-ui->_max_value->value()));
    }
    if(_M_processing_type == Process_Correlation)
    {
        //Set Cutoff value
        ui->_cutoff->setValue(0);
        _M_cutoff=(double)ui->_cutoff->value()/1000;

        //update color bar
        _UpdateColorBar();

        //set Min and max values
        ui->_max_value->setValue(1);
        ui->_min_value->setText(QString::number(-ui->_max_value->value()));
    }
    if(_M_processing_type == Process_Correlation)
    {
        //Set Cutoff value
        ui->_cutoff->setValue(0);
        _M_cutoff=(double)ui->_cutoff->value()/1000;

        //update color bar
        _UpdateColorBar();

        //set Min and max values
        ui->_max_value->setValue(1);
        ui->_min_value->setText(QString::number(-ui->_max_value->value()));
    }

    onContrastImgchanged();
}



void HdisplayResult::updateCartographyMode()
{
    int nb_chromophore = 3;

    if(ui->_display_type->count()>nb_chromophore)
    {
        while(ui->_display_type->count()!=nb_chromophore)
        {
            ui->_display_type->removeItem(0);
        }
    }
    if(ui->_display_type->count()<nb_chromophore)
    {
        while(ui->_display_type->count()!=nb_chromophore)
        {
            ui->_display_type->addItem("");
        }
    }

    ui->_display_type->setItemText(0,"HbO2");
    ui->_display_type->setItemText(1,"Hb");
    ui->_display_type->setItemText(2,"HbT");

}


//Display max value changed
void HdisplayResult::onMinMaxDisplayValueChanged()
{
    if(_M_processing_type == Process_Correlation) // Bold proba mode
    {
        //Pearson coeff
        ui->_max_value->setRange(-1,1,"");
        ui->_min_value->setText("-1");
    }

    if(_M_processing_type == Process_Mean_Delta_C || _M_processing_type == Activation_GLM_Pixel_wise)
    {
        ui->_max_value->setRange(-10000,10000,"");
        ui->_min_value->setText(QString::number(-ui->_max_value->value()));
    }

    onContrastImgchanged();
}

//Median Filter
void HdisplayResult::onMedianFilterON(bool v)
{
    ui->_display->onMedianFilterON(v);
    if(v)
        ui->_Median_Filter->setText("Median Filter ON");
    else
        ui->_Median_Filter->setText("Median Filter OFF");
}

//Window sier for median filter
void HdisplayResult::onWindowSizeChanged(double v)
{
    if(!((int)v & 1))   //Si nombre pair
    {
        if(v!=30)
            v+=1;
        else
            v-=1;

        ui->_window_size->setValue(v);
    }
    ui->_display->onWindowSizeChanged(v);
}


void HdisplayResult::onnewCutoffValue()
{
    //Display cut off value
    int v=ui->_cutoff->value();
    _M_cutoff=(double)v/1000;

    _M_corr_coef = _M_cutoff;

    //update color bar
    _UpdateColorBar();

    //update img contrast
    onContrastImgchanged();

}


