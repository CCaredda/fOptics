#include "HImageDisplay.h"

#include <QMouseEvent>
#include <QRect>
#include <QDebug>
#include <QPainter>


HImageDisplay::HImageDisplay(QWidget *parent):
    QWidget(parent)
{
    //Init result directory
    _M_result_directory =  "/home/results/";


    //Initial image
    _M_initial_image = Mat::zeros(0,0,CV_8UC1);
    _M_alpha            = 0.5;

    //MEAN ROI
    _M_MeanROI_radius = 5;
    _M_requestMeanROI = false;



    //Statist zone
    _M_setting_stat_zone        = 7;
    _M_stat_zone_type           = Process_Mean_Delta_C;





    //Display coeff
    //x : cols, y : rows
    _M_x_coeff = 1.0;
    _M_y_coeff = 1.0;

    _M_img_size=Size(0,0);
    _M_undersampled_img_size = Size(0,0);


    _M_mouse_click  =false;
    _M_mouse_move   =false;

    //Draw rect Roi
    _M_draw_Rect_ROI        =false;
    _M_Pre_ROI_firstPoint   =QPoint(0,0);
    _M_Pre_ROI_LastPoint    =QPoint(0,0);
    _M_Pre_ROI_firstPoint_display   =QPoint(0,0);
    _M_Pre_ROI_LastPoint_display    =QPoint(0,0);

    //Draw Analysis zone
    _M_draw_analysis_zone       =false;
    _ROI_closed                 =false;
    _M_ROI.clear();
    _M_ROI_display_img.clear();

    //Save img results
    _M_save_img_results         =false;
    _M_img_saved                =0;

    //user action
    _M_enable_user_action = false;

    //Contrast image

    //Contrast image
    _M_draw_contrast_img    =false;

    //median filter
    _M_median_Filter        =false;
    _M_window_Median_filter =5;

    //Draw point
    _M_draw_Point       = false;
    _M_point_is_drawn   = false;

    //ROI Type
    _M_ROI_type = 0; //0: draw ROI, 1: Auto ROI, 2: Rectangle ROI


    this->setAutoFillBackground(true);
}


HImageDisplay::~HImageDisplay()
{
}

QPoint HImageDisplay::_correct_ROI_Point(QPoint p)
{
    if(p.x()<0)
        p.setX(0);
    if(p.x()>(_M_initial_image.cols-1))
        p.setX(_M_initial_image.cols-1);

    if(p.y()<0)
        p.setY(0);
    if(p.y()>(_M_initial_image.rows-1))
        p.setY(_M_initial_image.rows-1);

    return p;
}


//On new initial image
void HImageDisplay::setInialImage(Mat img)
{

    img.copyTo(_M_initial_image);
    cvtColor(_M_initial_image,_M_initial_image,CV_BGR2RGB);

    _M_initial_image.copyTo(_M_display_img);
    _M_img_size= _M_display_img.size();
    qDebug()<<"[HImageDisplay] setInialImage size ("<<_M_img_size.height<<";"<<_M_img_size.width<<")";

    repaint();
}


void HImageDisplay::paintEvent(QPaintEvent *)
{
    if(_M_display_img.empty())
        return;

    QSize size=this->size();
    QPainter painter(this);
    QImage img2;

    Mat img_result;
    _M_display_img.copyTo(img_result);


    //DRAW MAPS
    if(_M_draw_contrast_img && !_M_contrast_img.empty() && (_M_stat_zone_type == Process_Mean_Delta_C || _M_stat_zone_type == Process_Correlation))
    {
        Mat contrast_img;
        cv::resize(_M_contrast_img,contrast_img,_M_display_img.size());
        if(contrast_img.size()==img_result.size())
        {
            if(_M_median_Filter)
                medianBlur(contrast_img,contrast_img,_M_window_Median_filter);
            img_result = mergeImg(img_result,contrast_img,_M_alpha);
        }
    }



    //Plot statistic zones Pixel-wise mode
    if(!_M_contrast_img.empty() && (_M_stat_zone_type == Activation_GLM_Pixel_wise || _M_stat_zone_type == Activation_GLM_auto_thesh))
    {
        Mat contrast_img;
        cv::resize(_M_contrast_img,contrast_img,_M_display_img.size());
        if(contrast_img.size()==img_result.size())
        {
//            if(_M_median_Filter)
//                medianBlur(contrast_img,contrast_img,_M_window_Median_filter);
            img_result = mergeImg(img_result,contrast_img,_M_alpha);
        }
    }



    //Resize image to the size of the widget
    cv::Size _size(size.width(),size.height());
    cv::resize(img_result,img_result,_size);

    //Convert Opencv image to QImage
    switch (img_result.type())
    {
    case CV_8UC3:
        img2=mat_to_qimage_ref(img_result,QImage::Format_RGB888);
        break;
    case CV_8UC1:
        img2=mat_to_qimage_ref(img_result,QImage::Format_Grayscale8);
        break;
    default:
        img2=mat_to_qimage_ref(img_result,QImage::Format_RGB888);
        break;
    }

    //Calcul resize coeff to indicate correct pixel values to analysis
//    _M_x_coeff  = ((double)(img2.size().width()))/(_M_img_size.width);
//    _M_y_coeff  = ((double)(img2.size().height()))/(_M_img_size.height);
    _M_x_coeff  = ((double)(img2.size().width()))/(_M_undersampled_img_size.width);
    _M_y_coeff  = ((double)(img2.size().height()))/(_M_undersampled_img_size.height);


    //Draw image
    painter.drawImage(QPoint(0,0), img2);


    //Draw Pre ROI
    if(_M_draw_Rect_ROI)
    {
        QPen pen;
        pen.setColor(QColor(Qt::red));
        pen.setWidth(3);
        painter.setPen(pen);

        QRect Pre_ROI_Rect(_M_Pre_ROI_firstPoint_display,_M_Pre_ROI_LastPoint_display);
        painter.drawRect(Pre_ROI_Rect);
    }


    //Draw Analysis zone
    if(_M_draw_analysis_zone)
    {
        if(_M_ROI_type==0)
        {
            QPen pen;
            pen.setColor(QColor(Qt::red));
            pen.setWidth(3);
            //Draw contour
            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush);

            // qDebug()<<"Drax analysis zone 1";
            QPainterPath path;
            QVector<QPoint> ROI_displayed;
            for(int i=0;i<_M_ROI.size();i++)
            {
                // qDebug()<<"Drax analysis zone 1.1"<<_M_ROI[i].x()*_M_x_coeff<<_M_ROI[i].y()*_M_y_coeff;
                ROI_displayed.push_back(QPoint(_M_ROI[i].x()*_M_x_coeff,_M_ROI[i].y()*_M_y_coeff));
            }

            // qDebug()<<"Drax analysis zone 2";
            if(!ROI_displayed.empty())
            {
                path.moveTo(ROI_displayed[0].x(),ROI_displayed[0].y());
                for( int i=1;i< ROI_displayed.size();i++)
                {
                    path.lineTo(ROI_displayed[i].x(),ROI_displayed[i].y());
                }

                if(_ROI_closed)
                {
                    path.lineTo(ROI_displayed[0].x(),ROI_displayed[0].y());
                    //do not display analysis zone
//                    _M_draw_analysis_zone = false;
                }
            }

            painter.drawPath(path);
        }

        if(_M_ROI_type==2)
        {
            QPen pen;
            pen.setColor(QColor(Qt::red));
            pen.setWidth(3);
            painter.setPen(pen);

            QPoint P1(_M_ROI[0].x()*_M_x_coeff,_M_ROI[0].y()*_M_y_coeff);
            QPoint P2(_M_ROI[1].x()*_M_x_coeff,_M_ROI[1].y()*_M_y_coeff);

            QRect Rect(P1,P2);

            painter.drawRect(Rect);
        }


    }


    //Draw Point
    if(_M_draw_Point)
    {
        QPen pen;
        pen.setColor(QColor(Qt::black));
        pen.setWidth(2);
        painter.setPen(pen);

        QPainterPath _path;
        _path.moveTo(_M_selected_Point.x(),_M_selected_Point.y()-3);
        _path.lineTo(_M_selected_Point.x(),_M_selected_Point.y()+3);
        _path.moveTo(_M_selected_Point.x()-3,_M_selected_Point.y());
        _path.lineTo(_M_selected_Point.x()+3,_M_selected_Point.y());
        painter.drawPath(_path);

        if(_M_requestMeanROI)
        {
            painter.setBrush(Qt::black);
            painter.drawEllipse(_M_selected_Point,(int)(_M_MeanROI_radius*_M_x_coeff),int(_M_MeanROI_radius*_M_y_coeff));
            painter.setBrush(Qt::NoBrush);
        }
    }

    painter.end();
}

//Update img
void HImageDisplay::onupdateImage(const Mat& img)
{
    setInialImage(img);
}

//Receive image
void HImageDisplay::newImage(const cv::Mat& image)
{

    switch (image.type()) {
    case CV_8UC3:
        cvtColor(image,_M_Matimage,CV_BGR2RGB);
        assert(_M_Matimage.isContinuous());
        break;
    case CV_8UC1:
        _M_Matimage=image;
        assert(_M_Matimage.isContinuous());
        break;
    default:
        break;
    }


    _M_undersampled_img_size = image.size();
    repaint();
}

//Rectangle ROI requested
void HImageDisplay::onRectROIRequested()
{
    _M_enable_user_action   =true;
    _M_draw_Rect_ROI        =true;
}

//Analysis zone requested
void HImageDisplay::onAnalysisZoneRequested()
{
    _M_ROI_type             = 0;
    _M_enable_user_action   =true;
    _M_draw_analysis_zone   =true;
    emit analysisZoneisDrawn(true);
    _ROI_closed             =false;

}





void HImageDisplay::onnewSettingStatisticZone(double v)
{
    _M_setting_stat_zone = (int)v;

    qDebug()<<"HImageDisplay::onnewSettingStatisticZone";
    if(_ROI_closed)
        _getStatisticZones();
}




//get stats zone
void HImageDisplay::_getStatisticZones()
{
    if (_M_Matimage.empty() || _M_ROI.empty())
        return;


    int resels_nb = 0;

    vector<Point> ROI;
    for(int i=0;i<_M_ROI.size();i++)
        ROI.push_back(Point(_M_ROI[i].x(),_M_ROI[i].y()));

    //Get bounding rect
    Rect r= boundingRect(ROI);

    //mask
    Mat dst     = Mat::zeros(_M_Matimage.size(),CV_8UC1);
    vector<vector<Point> > c;
    c.push_back(ROI);
    drawContours(dst,c,0, Scalar(255),CV_FILLED);

    // Scan the image with in bounding box
    for(int j=r.x;j<r.x+r.width;j=j+_M_setting_stat_zone)
    {
        for(int k=r.y;k<r.y+r.height;k=k+_M_setting_stat_zone)
        {
            Rect roi_rect(j,k,_M_setting_stat_zone,_M_setting_stat_zone);
            if(j+_M_setting_stat_zone<_M_Matimage.cols &&
                    k+_M_setting_stat_zone<_M_Matimage.rows &&
                    j>0 && k>0)
            {
                //Count roi pixels contains in roi_rect
                Mat roi = dst(roi_rect);
                int count = countNonZero(roi);

                //if roi pixels are fully contained in roi_rect
                if(count ==_M_setting_stat_zone*_M_setting_stat_zone)
                {
                    resels_nb++;
                }
            }
        }
    }

    qDebug()<<"[HImageDisplay] _getStatisticZones cortical areas: "<<resels_nb;

    if(resels_nb>0)
    {
        emit newReselNumber(resels_nb);
    }
    repaint();
}




//Mouse events
void HImageDisplay::mousePressEvent(QMouseEvent *ev)
{
    //Get contour ROI
    vector<Point> c;
    for(int i=0;i<_M_ROI.size();i++)
        c.push_back(Point(_M_ROI[i].x()*_M_x_coeff,_M_ROI[i].y()*_M_y_coeff));

    //Draw Lines or ROIs
    if(_M_enable_user_action)
    {
        //indicate that a mouse click has been done
        _M_mouse_click=true;


        //if draw pre ROI rect is requested
        if(_M_draw_Rect_ROI)
        {
            _M_Pre_ROI_firstPoint_display=ev->pos();
            _M_Pre_ROI_LastPoint_display=_M_Pre_ROI_firstPoint_display;

            //resize value to match the real sensor index
            _M_Pre_ROI_firstPoint.setX(_M_Pre_ROI_firstPoint_display.x()/_M_x_coeff);
            _M_Pre_ROI_firstPoint.setY(_M_Pre_ROI_firstPoint_display.y()/_M_y_coeff);

            // Check if value has the correct increment
            if(_M_Pre_ROI_firstPoint.x()%8!=0)
                _M_Pre_ROI_firstPoint.setX(_M_Pre_ROI_firstPoint.x() - (_M_Pre_ROI_firstPoint.x()%8));

            if(_M_Pre_ROI_firstPoint.y()%2!=0)
                _M_Pre_ROI_firstPoint.setY(_M_Pre_ROI_firstPoint.y() - _M_Pre_ROI_firstPoint.y()%2);


            repaint();
            return;
        }


        //If analysis zone drawing is requested
        if(_M_draw_analysis_zone)
        {
            _M_ROI.clear();
            //DrawROI
            if(_M_ROI_type==0)
            {
                QPoint newPoint=ev->pos();
                newPoint.setX(newPoint.x()/_M_x_coeff);
                newPoint.setY(newPoint.y()/_M_y_coeff);

                _M_ROI.push_back(newPoint);
                repaint();
                return;
            }
            //Draw rectangle
            if(_M_ROI_type==2)
            {
                QPoint newPoint=ev->pos();
                newPoint.setX(newPoint.x()/_M_x_coeff);
                newPoint.setY(newPoint.y()/_M_y_coeff);

                //Add two points (top left point et bottom right point of the rectangle)
                _M_ROI.push_back(newPoint);
                _M_ROI.push_back(newPoint);
                repaint();
                return;
            }

        }
    }

    //If point drawing is requested
    if(!_M_point_is_drawn && !_M_ROI.empty())
    {

        _M_selected_Point=ev->pos();
        Point P(_M_selected_Point.x(),_M_selected_Point.y());
        Point emitedPoint(P.x/_M_x_coeff,P.y/_M_y_coeff);


        if(pointPolygonTest(c,P,false)>0)
        {
            _M_draw_Point     = true;
            _M_point_is_drawn = true;

            emit PointSelected(emitedPoint);
        }
        else
        {
            _M_draw_Point     = false;
            _M_point_is_drawn = false;
        }
        repaint();
    }
    else
    {
        _M_point_is_drawn   = false;
        _M_draw_Point       = false;
        repaint();
    }
}

void HImageDisplay::mouseMoveEvent(QMouseEvent *ev)
{
    if(_M_enable_user_action && _M_mouse_click)
    {
        //if draw pre ROI rect is requested
        if(_M_draw_Rect_ROI)
        {
            _M_Pre_ROI_LastPoint_display=ev->pos();

            repaint();
            return;
        }
        //If analysis zone drawing is requested
        if(_M_draw_analysis_zone)
        {
            //Draw ROI
            if(_M_ROI_type==0)
            {
                QPoint newPoint=ev->pos();
                newPoint.setX(newPoint.x()/_M_x_coeff);
                newPoint.setY(newPoint.y()/_M_y_coeff);

                _M_ROI.push_back(newPoint);
                repaint();
                return;
            }
            //Rectangle ROI
            if(_M_ROI_type==2)
            {
                QPoint newPoint=ev->pos();
                newPoint.setX(newPoint.x()/_M_x_coeff);
                newPoint.setY(newPoint.y()/_M_y_coeff);

                //update the bottom right point
                _M_ROI[1] = newPoint;
                repaint();
            }
        }
    }
}

void HImageDisplay::mouseReleaseEvent(QMouseEvent *ev)
{
    if(_M_enable_user_action && _M_mouse_click)
    {
        //Stop enabling user to interact with the image
        _M_enable_user_action = false;
        _M_mouse_click=false;

        //if draw pre ROI rect is requested
        if(_M_draw_Rect_ROI)
        {
            qDebug()<<"[HImageDisplay::mouseReleaseEvent] 1";
            _M_draw_Rect_ROI=false;
            _M_Pre_ROI_LastPoint_display=ev->pos();

            //resize value to match the real sensor index
            _M_Pre_ROI_LastPoint.setX(_M_Pre_ROI_LastPoint_display.x()/_M_x_coeff);
            _M_Pre_ROI_LastPoint.setY(_M_Pre_ROI_LastPoint_display.y()/_M_y_coeff);

            // Check if value has the correct increment
            qDebug()<<"[HImageDisplay::mouseReleaseEvent] 2";

            if(_M_Pre_ROI_LastPoint.x()%8!=0)
                _M_Pre_ROI_LastPoint.setX(_M_Pre_ROI_LastPoint.x() - (_M_Pre_ROI_LastPoint.x()%8));

            if(_M_Pre_ROI_LastPoint.y()%2!=0)
                _M_Pre_ROI_LastPoint.setY(_M_Pre_ROI_LastPoint.y() - _M_Pre_ROI_LastPoint.y()%2);

            //Check if _M_Pre_ROI_firstPoint is the top left corner point of the ROI, if not exchange with _M_Pre_ROI_LastPoint
            if(_M_Pre_ROI_firstPoint.x()>_M_Pre_ROI_LastPoint.x() || _M_Pre_ROI_firstPoint.y()>_M_Pre_ROI_LastPoint.y())
            {
                QPoint p = _M_Pre_ROI_LastPoint;
                _M_Pre_ROI_LastPoint=_M_Pre_ROI_firstPoint;
                _M_Pre_ROI_firstPoint = p;
            }

            _M_Pre_ROI_firstPoint = _correct_ROI_Point(_M_Pre_ROI_firstPoint);
            _M_Pre_ROI_LastPoint = _correct_ROI_Point(_M_Pre_ROI_LastPoint);

            qDebug()<<"[HImageDisplay::mouseReleaseEvent] send newRectROI";
            emit newRectROI(_M_Pre_ROI_firstPoint,_M_Pre_ROI_LastPoint);


            //send init image to Analysis class
            Point P1(_M_Pre_ROI_firstPoint.x(),_M_Pre_ROI_firstPoint.y());
            Point P2(_M_Pre_ROI_LastPoint.x(),_M_Pre_ROI_LastPoint.y());

            qDebug()<<"[HImageDisplay::mouseReleaseEvent] ROI "<<P1.x<<";"<<P1.y<<" "<<P2.x<<";"<<P2.y<<" Image size "<<_M_initial_image.rows<<";"<<_M_initial_image.cols;

            Rect rect(P1,P2);
            _M_initial_image(rect).copyTo(_M_display_img);
            _M_img_size= _M_display_img.size();

            emit newInitialImg(_M_display_img);

            repaint();
            return;
        }

        //If analysis zone drawing is requested
        if(_M_draw_analysis_zone)
        {
            // Close ROI
            _ROI_closed=true;

            //Draw ROI
            if(_M_ROI_type==0)
            {
                QPoint newPoint=ev->pos();
                newPoint.setX(newPoint.x()/_M_x_coeff);
                newPoint.setY(newPoint.y()/_M_y_coeff);

                _M_ROI.push_back(newPoint);
            }

            //Rectangle ROI
            if(_M_ROI_type==2)
            {
                QPoint newPoint=ev->pos();
                newPoint.setX(newPoint.x()/_M_x_coeff);
                newPoint.setY(newPoint.y()/_M_y_coeff);

                //update bottom right point
                _M_ROI[1] = newPoint;

                //Cretate rect
                QRect Rect(_M_ROI[0],_M_ROI[1]);
                //Convert the Rect into contour of points
                _M_ROI.clear();
                _M_ROI.push_back(Rect.topLeft());
                _M_ROI.push_back(Rect.topRight());
                _M_ROI.push_back(Rect.bottomRight());
                _M_ROI.push_back(Rect.bottomLeft());
                //Display the contour using the contour method (Draw ROI)
                _M_ROI_type=0;

            }

            setAnalysisZone(_M_ROI,_M_undersampled_img_size);
            emit newAnalysisZone(_M_ROI,_M_undersampled_img_size);

            repaint();
            return;
        }
    }
}

void HImageDisplay::onNewAutoROI(Point P1, Point P2)
{
    Rect rect(P1,P2);
    _M_initial_image(rect).copyTo(_M_display_img);
    _M_img_size= _M_display_img.size();

    emit newInitialImg(_M_display_img);
    repaint();
}


//set analysis zone
void HImageDisplay::setAnalysisZone(QVector<QPoint> roi, cv::Size s)
{
    _M_draw_analysis_zone = true;
    _M_ROI.clear();
    _M_ROI = roi;
    _ROI_closed = true;
    _M_undersampled_img_size = s;


    //Define ROI for non sampled image
    _M_ROI_display_img.clear();
    vector<vector<Point> > c;
    c.resize(1);
    for(int i=0;i<_M_ROI.size();i++)
        c[0].push_back(Point(_M_ROI[i].x(),_M_ROI[i].y()));

    qDebug()<<"undersampled img size ("<<_M_undersampled_img_size.height<<";"<<_M_undersampled_img_size.width<<")";
    qDebug()<<"initial img size ("<<_M_img_size.height<<";"<<_M_img_size.width<<")";

    Mat mask = Mat::zeros(_M_undersampled_img_size,CV_8UC1);

    drawContours(mask,c,0,Scalar(255),CV_FILLED);
    cv::resize(mask,mask,_M_img_size);
    c.clear();
    findContours(mask,c,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
    for (unsigned int i=0;i<c[0].size();i++)
        _M_ROI_display_img.push_back(QPoint(c[0][i].x,c[0][i].y));

    //get stats zone
    _getStatisticZones();
}


//Clear Img
void HImageDisplay::ClearImg()
{
    _M_enable_user_action   =false;
    _M_draw_analysis_zone   =false;
    _M_draw_Rect_ROI        =false;
    emit analysisZoneisDrawn(false);


    repaint();
}



//New contrast image
void HImageDisplay::onNewContrastImage(Mat img)
{
    cvtColor(img,_M_contrast_img,CV_BGR2RGB);
    _M_draw_contrast_img=true;

    if(_M_save_img_results && !_M_contrast_img.empty() && !_M_display_img.empty())
    {

        if(_M_median_Filter)
            medianBlur(img,img,_M_window_Median_filter);

        Mat temp;
        cvtColor(_M_display_img,temp,CV_RGB2BGR);
        Mat out     = mergeImg(temp,img,_M_alpha);

        QString file = QString(_M_result_directory)+"img_mask_idx"+QString::number(_M_img_saved)+".png";
        imwrite(file.toStdString(),out);
        _M_img_saved++;

    }

    repaint();
}

void HImageDisplay::saveImg(Mat img,int proc_index,QString path)
{
    if(_M_median_Filter)
        medianBlur(img,img,_M_window_Median_filter);

    Mat temp;
    cvtColor(_M_display_img,temp,CV_RGB2BGR);
    Mat out     = mergeImg(temp,img,_M_alpha);

    QString file = path+QString::number(proc_index)+".png";
    imwrite(file.toStdString(),out);
}

void HImageDisplay::onNewContrastImage(Mat img,int proc_index)
{
    cvtColor(img,_M_contrast_img,CV_BGR2RGB);
    _M_draw_contrast_img=true;

    if(_M_save_img_results && !_M_contrast_img.empty() && !_M_display_img.empty())
        saveImg(img,proc_index,QString(_M_result_directory)+"img_mask_idx");

    repaint();
}



