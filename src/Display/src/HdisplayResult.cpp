#include "HdisplayResult.h"
#include "ui_HdisplayResult.h"

HdisplayResult::HdisplayResult(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HdisplayResult)
{
    ui->setupUi(this);

    //Z threshold RFT
    _M_z_thresh = 0.0;


    //last processed id
    _M_last_processed_id = -1;


    //Rect ROI drawing
    connect(this,SIGNAL(RectROIRequested()),ui->_display,SLOT(onRectROIRequested()));
    connect(ui->_display,SIGNAL(newRectROI(QPoint,QPoint)),this,SIGNAL(newRectROI(QPoint,QPoint)));


    //alpha for image display
    connect(ui->_alpha_val,SIGNAL(valueEdited(double)),ui->_display,SLOT(onNewAlphaValue(double)));
    ui->_alpha_val->setRange(0,1,"");
    ui->_alpha_val->setValue(0.5);





    //Clear image requested
    connect(this,SIGNAL(ClearImg()),ui->_display,SLOT(ClearImg()));

    //Analysis zone drawing
    connect(this,SIGNAL(onAnalysisZoneRequested()),ui->_display,SLOT(onAnalysisZoneRequested()));
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
    ui->_max_value->setValue(5);
    ui->_min_value->setText("-5");
    ui->_units->setText("(µM)");
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


    //Cut off display
    connect(ui->_cutoff,SIGNAL(sliderReleased()),this,SLOT(onnewCutoffValue()));
    ui->_cutoff->setValue(0);
    onnewCutoffValue();
    _M_cutoff=0;
    _M_corr_coef=0;

    //update color bar
    _UpdateColorBar();


    //Set chromophore list
    ui->_display_type->addItem("HbO2");
    ui->_display_type->addItem("Hb");
    ui->_display_type->addItem("HbT");
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

    //Statistic zone
    connect(this,SIGNAL(newSettingStatisticZone(double)),ui->_display,SLOT(onnewSettingStatisticZone(double)));

    //New number of resels
    connect(ui->_display,SIGNAL(newReselNumber(int)),this,SIGNAL(newReselNumber(int)));

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

    QString unit = "";

    switch (v) {
    case Process_Mean_Delta_C:
        unit = "(µM)";
        break;
    case Process_Correlation:
        unit = "(Person coefficient)";
        break;
    case Activation_GLM_Pixel_wise:
        unit = "(Z-stats)";
        break;
    case Activation_GLM_auto_thesh:
        unit = "(Z-stats)";
        break;
    default:
        unit = "";
        break;
    }

    ui->_units->setText(unit);
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





//On contrast img changed (max value, threshold, etc)
void HdisplayResult::onContrastImgchanged()
{

    //Send new chromophore ID to PAnalyse
    emit newChromophoreID(ui->_display_type->currentIndex());

    if(_M_contrast_img.empty())
        return;





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
    case Activation_GLM_auto_thesh:
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
    if(_M_processing_type == Activation_GLM_Pixel_wise || _M_processing_type == Activation_GLM_auto_thesh)
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
    if(_M_processing_type == Process_Mean_Delta_C)
    {
        //Set Cutoff value
        ui->_cutoff->setValue(0);
        _M_cutoff=(double)ui->_cutoff->value()/1000;

        //update color bar
        _UpdateColorBar();

        //set Min and max values
        ui->_max_value->setValue(5);
        ui->_min_value->setText(QString::number(-ui->_max_value->value()));
    }

    onContrastImgchanged();
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

    if(_M_processing_type == Process_Mean_Delta_C || _M_processing_type == Activation_GLM_Pixel_wise || _M_processing_type == Activation_GLM_auto_thesh)
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


