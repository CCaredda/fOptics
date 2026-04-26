#include "ARegistration.h"


//NOTE :
// 1) La première frame pour l'apprentissage est la 1ere de la vidéo

ARegistration::ARegistration(QObject *parent) : QThread(parent)
{
    //moveToThread(this);

    //init
//    file.setFileName(_M_result_directory+"log_reg.txt");
//    file.open(QIODevice::WriteOnly | QIODevice::Append);

    _M_learning_Model_is_done   = false;
    _M_acquisition_is_over      = false;
    _M_enable_process           = false;
    _M_soft_ready               = false;

    //enable motion compensation
    _M_enable_motion_compensation   =true;

    _M_id_learning_frame        = 0;


    _M_maxkey=10000000; //max num of keypoint for sparse reg
    _M_Lnframe=100;     //learning number of frame
    _M_tot_reg_frames=1;
    _M_harrisDist=10;   //min dist between harris keypoints
    _M_harrisQL = 1e-7; //harris quality level, default 1e-10
    _M_dySig=12;         //sigma for init weighting with dy

    _M_npyr = 3;        //Learning number of level in the pyramid
    _M_ws = 11;         //Learning window size 
    _M_iter = 3;        //Learning number of iterations per level
    _M_Lnpyr = 3;        //Learning number of level in the pyramid
    _M_Lws = 21;         //Learning window size
    _M_Liter = 3;        //Learning number of iterations per level


    _M_irlsSig0 = 1e-3; //sigma at iteration 0 for IRLS
    _M_irlsN = 6;       //number of IRLS iterations
    _M_constPCA0 = 1e6; //delta in the constraint on the pca coeff: ||L(lpca-lpca0)||^2/npca < delta
    _M_ptsFit = "LLAG"; //Lagrangian (LAG), Large def Lag (LLAG), Eulerian (EULER)
    _M_camcomp = true;  // compose camera motion model (for ofpca)
    _M_border = 40;     //border size (remove kpt in the border)
    _M_reg_type=0;      //registration type (ofPCA)

    _M_img.clear();


    _M_reg =0;
    _M_lreg=0;
    _M_creg=0;

    _M_analysis_zone.clear();
    _M_NCC.clear();
    _M_NCC.resize(2);

    _M_stop = false;
    start(); //Start the thread

}




ARegistration::~ARegistration()
{
    if (isRunning()) // Only stop/wait if not already done in closeEvent()
    {
        QMutexLocker locker(&_M_mutex);
        _M_stop = true;
        _M_condition.wakeOne();
        // Must release lock before wait()
    }

    if (isRunning())
        wait();
}

void ARegistration::stop()
{
    qDebug()<<"ARegistration::stop";
    QMutexLocker locker(&_M_mutex);
    _M_stop = true;
    _M_condition.wakeOne(); // wake it so it can exit
}

//Called when a new registration type is selected
void ARegistration::newRegistrationtype(int v)
{
    qDebug()<<"ARegistration::newRegistrationtype 1";
    _M_reg_type=v;
    _M_learning_Model_is_done=false;
    _M_acquisition_is_over=false;
    _M_img.clear();

    switch (v)
    {
    case 0: //reg type : of
        _M_lreg=&_M_lofreg;
        _M_reg =&_M_ofreg;

        _M_lofreg.useTVL1(false);
        _M_ofreg.useTVL1(false);

        _M_enable_process=true;
        break;
    case 1: //reg type : tvl1
        _M_lreg=&_M_lofreg;
        _M_lofreg.useTVL1(true);

        _M_reg =&_M_ofreg;
        _M_ofreg.useTVL1(true);

        _M_enable_process=true;
        break;
    case 2: //reg type : ofpca
        _M_reg =&_M_sparseofPCA;
        initSparseofPCA();
        break;
    default:
        _M_lofreg.useTVL1(false);
        _M_enable_process=true;
        break;
    }

    //Send parameters to LReg & Reg
    //1st param : learning number of level in the pyramid
    //2nd param : Learning window size
    //3rd param : learning number of iterations per level
    _M_lreg->setParam(_M_Lnpyr,_M_Lws,_M_Liter);
    _M_reg->setParam(_M_npyr,_M_ws,_M_iter);

    //Init current registration object
    _M_creg = (_M_reg==&_M_sparseofPCA) ? _M_lreg : _M_reg;

    qDebug()<<"ARegistration::newRegistrationtype 2";
}

//Set the number of frames to register
void ARegistration::setTotalNbFrames(int v)
{
    _M_tot_reg_frames=v;
    _M_NCC.clear();
    _M_NCC.resize(2);
    _M_NCC[0].fill(0,v);
    _M_NCC[1].fill(0,v);
}

//Set analysis zone (for NCC measure)
void ARegistration::setAnalysisZone(QVector<QPoint> c)
{
    _M_analysis_zone.clear();

    for(int i=0;i<c.size();i++)
        _M_analysis_zone.push_back(Point(c[i].x(),c[i].y()));
}

vector<Point> ARegistration::getAnalysisZone()
{
    return _M_analysis_zone;
}


//Transformation input files
// save transform to text file

void ARegistration::setTransfoModel()
{
    qDebug()<<"ARegistration::setTransfoModel 1";
    if(_M_reg_type==2)
    {
        if (_M_lreg==0)
        {
            _M_enable_process=false;
            qDebug() << "Error: when using ofpca registration you must either set Lreg or loadtrans";
            return;
        }
        if (_M_Lnframe<=0)
        {
            _M_enable_process=false;
            qDebug() << "Error: when using ofpca with Lreg you must set Lnframe>0";
            return;
        }
        _M_enable_process=true;
        _M_sparseofPCA.setSampleSize(_M_Lnframe);
    }

    qDebug()<<"ARegistration::setTransfoModel 2";
}

void ARegistration::setFirstImg(Mat &img)
{
    qDebug()<<"ARegistration::setFirstImg 1";
    if(img.empty())
        return;

    switch (img.type()) {
    case CV_8UC3:
        cvtColor(img,_M_gframe0,CV_BGR2GRAY);
        break;
    case CV_8UC1:
        _M_gframe0=img;
        break;
//    case CV_16UC1:
//        //img.convertTo(_M_gframe0,CV_8UC1,0.00390625);
//        break;
    default:
        break;
    }

    //Send first img to Reg class
    _M_creg->setFrame0(_M_gframe0);
    _M_lreg->setFrame0(_M_gframe0);
    _M_reg->setFrame0(_M_gframe0);

    qDebug()<<"ARegistration::setFirstImg 2";
}

//Request parallel thread process
void ARegistration::requestParallelThreadProcess(_Processed_img img)
{
    if(!_M_enable_motion_compensation)
    {
        emit newRegistrationProgress("Registration",(100*img.thread_id)/(_M_tot_reg_frames-1));
        emit newRegisteredImage(img);
        return;
    }

    // QMutexLocker locker(&_M_mutex);
    // _M_img.push_back(img);
    // locker.unlock();

    // if(img.thread_id==0 || !this->isRunning())
    // {
    //     this->start();
    // }

    QMutexLocker locker(&_M_mutex);
    _M_img.enqueue(img); // push to back
    _M_condition.wakeOne(); // wake the sleeping thread
}

//Protected
void ARegistration::run()
{
    for (;;)
    {
        if(_M_stop)
            break;
        if(_M_learning_Model_is_done)
        {
            _Processed_img img;
            if(_M_soft_ready)
            {
                {
                    QMutexLocker locker(&_M_mutex);

                    // Sleep here until there is work or a stop request
                    while (_M_img.isEmpty() && !_M_stop)
                        _M_condition.wait(&_M_mutex);

                    if (_M_stop)
                        break;

                    _Processed_img tmp = _M_img.dequeue();
                    img.thread_id = tmp.thread_id;
                    img.img = tmp.img.clone(); // deep copy — fully independent
                } // mutex unlocks here

                _RegisterImage(img);
            }
        }
        else
        {
            if(_M_img.size()>=_M_id_learning_frame+1)
            {
                _Processed_img img;
                {
                    QMutexLocker locker(&_M_mutex);

                    // Sleep here until there is work or a stop request
                    while (_M_img.isEmpty() && !_M_stop)
                        _M_condition.wait(&_M_mutex);

                    if (_M_stop)
                        break;

                    // _Processed_img tmp = _M_img.dequeue();
                    _M_img[_M_id_learning_frame].img.copyTo(img.img);
                    img.thread_id = _M_img[_M_id_learning_frame].thread_id;
                } // mutex unlocks here

                _LearnModel(img);
            }

        }
    }
}



void ARegistration::_LearnModel(_Processed_img img)
{
    if(!_M_enable_process)
        return;

    if(img.img.empty())
    {
        qDebug()<<"[ARegistration] _LearnModel img "<<img.thread_id<<" empty";
        return;
    }

    qDebug()<<"[ARegistration::_LearnModel] thread id "<<img.thread_id;

    //convert BGR to Gray
    Mat gframe;//,rframe;

    //convert BGR to Gray
    switch (img.img.type())
    {
    case CV_8UC3:
        cvtColor(img.img,gframe,CV_BGR2GRAY);
        break;
    case CV_8UC1:
        gframe=img.img;
        break;
    default:
        break;
    }

    //increase id learning frames
    _M_id_learning_frame++;

    // compute reg
    _M_creg->computeReg(gframe);
//    _M_creg->apply(img,rframe,INTER_LINEAR);


    //Handle Learning if in learning phase
    if (_M_reg==&_M_sparseofPCA && img.thread_id<=(_M_Lnframe-1))
    {
        _M_sparseofPCA.setSample(img.thread_id,_M_creg->getTransfo1(),_M_creg->getTransfo2(),gframe);
    }


    // end of learning: train and use sparseofPCA
    if (_M_reg==&_M_sparseofPCA && img.thread_id==(_M_Lnframe-1))
    {
        qDebug()<<"[ARegistration::_LearnModel] learning finished";
        _M_learning_Model_is_done=true;
        _M_creg = &_M_sparseofPCA;
        _M_sparseofPCA.setFrame0(_M_gframe0);
        qDebug()<<"[ARegistration::_LearnModel] learning finished 2";
//        emit newRegistrationProgress("Learn model",100);
//        emit MotionLearningisFinished();
    }

    emit newRegistrationProgress("Learn model",(100*img.thread_id)/(_M_Lnframe-1));

}



void ARegistration::_RegisterImage(_Processed_img img)
{
    //    QFile file("/home/caredda/Charly/Thèse/rapports/imagerie_cérébrale/6_temps_reel/img/data/Registration_elapsed_time_"+QString::number(img.img.cols*img.img.rows)+".txt");
//    if (!file.open(QIODevice::WriteOnly | QIODevice::Append))
//        return;
//    QTextStream Qt( &file );

//    QElapsedTimer timer;
//    timer.start();

    if(!_M_enable_process)
    {
        return;
    }

    /****************************************************************/
    /**********************Handle empty img**************************/
    /****************************************************************/
    if(img.img.empty())
    {
        qDebug()<<"[ARegistration] _RegisterImage img "<<img.thread_id<<" empty";
        if(_M_empty_img_buffer.empty())
        {
            emit newRegisteredImage(img);
            _M_NCC[0][img.thread_id]=1;
            _M_NCC[1][img.thread_id]=1;
            return;
        }
        img.img = _M_empty_img_buffer;
    }
    else
    {
        //Create buffer if the next img is empty
        img.img.copyTo(_M_empty_img_buffer);
    }

    //convert BGR to Gray
    Mat gframe,rframe;
    switch (img.img.type()) {
    case CV_8UC3:
        cvtColor(img.img,gframe,CV_BGR2GRAY);
        break;
    case CV_8UC1:
        gframe=img.img;
        break;
    default:
        break;
    }

    //process NCC[0] measure (before registration)
    _M_NCC[0][img.thread_id] = NormalizeCrossCorrelation(_M_gframe0,gframe,_M_analysis_zone);


//    //TEMP
//    QTextStream stream( &file );
//    stream <<"Img "<<img.thread_id<<endl;
//    stream <<QString::fromStdString(_M_creg->getLsMatrixDebug());
//    //END TEMP

    // compute reg
    _M_creg->computeReg(gframe);

//    //TEMP
//    stream <<QString::fromStdString(_M_creg->getLsMatrixDebug());
//    //END TEMP

    // apply reg
    _M_creg->apply(img.img,rframe,INTER_LINEAR);

    //process NCC[1] measure (after registration)
    Mat img_ng;
    cvtColor(rframe,img_ng,CV_BGR2GRAY);

    _M_NCC[1][img.thread_id] = NormalizeCrossCorrelation(_M_gframe0,img_ng,_M_analysis_zone);

/*
    //If NNC after reg is less than 0.98 or nan, write debug info
    if(_M_NCC[1][img.thread_id]<0.985 || std::isnan(_M_NCC[1][img.thread_id]))
    {
        cout<<"Img "<<img.thread_id<<"NCC after reg: "<<_M_NCC[1][img.thread_id]<<endl;

        //Create directory to store debug info
        QString path = _M_result_directory+QString::number(img.thread_id);
        QDir().mkdir(path);
        path = path+"/";

        //set dbg level to 1
        _M_creg->setDbgLevel(2,(path.toStdString()).c_str());

        // Recompute reg
        _M_creg->computeReg(gframe);

        //set dbg level to 0
        _M_creg->setDbgLevel(0,(path.toStdString()).c_str());

        QString img_path = path+"img_RGB_reg.png";
        imwrite(img_path.toStdString(),rframe);

        img_path = path+"img_grey_in.png";
        imwrite(img_path.toStdString(),gframe);
        if(std::isnan(_M_NCC[1][img.thread_id]))
            QCoreApplication::exit();
    }
    */

//    if(img.thread_id == 50)
//    {

//        QString path = _M_result_directory + "OK";
//        QDir().mkdir(path);

//        QString img_path = path+"/img_RGB_reg.png";
//        imwrite(img_path.toStdString(),rframe);

//        img_path = path+"/img_grey_in.png";
//        imwrite(img_path.toStdString(),gframe);


//        img_path = path+"/img_0.png";
//        imwrite(img_path.toStdString(),_M_gframe0);

//        img_path = path+"/transfo1.txt";
//        WriteFloatImg(img_path,_M_creg->getTransfo1());

//        img_path = path+"/transfo2.txt";
//        WriteFloatImg(img_path,_M_creg->getTransfo1());


////        _M_sparseofPCA.setDrawPoints(true);
////        _M_creg->apply(img.img,rframe,INTER_LINEAR);
////        img_path = path+"/img_RGB_reg_keypoints.png";
////        imwrite(img_path.toStdString(),rframe);

////        _M_sparseofPCA.setDrawPoints(false);
//    }


//    if(std::isnan(_M_NCC[1][img.thread_id]))
//    {
////       stream <<"[ARegistration] img "<<img.thread_id<<" has NCC 1 is nan value"<<endl;
//        qDebug()<<"[ARegistration] img "<<img.thread_id<<" has NCC 1 is nan value";
////        QString path = _M_result_directory +QString::number(img.thread_id);
////        QDir().mkdir(path);

////        QString img_path = path+"/img_RGB_reg.png";
////        imwrite(img_path.toStdString(),rframe);

////        img_path = path+"/img_grey_in.png";
////        imwrite(img_path.toStdString(),gframe);


////        img_path = path+"/img_0.png";
////        imwrite(img_path.toStdString(),_M_gframe0);

////        img_path = path+"/transfo1.txt";
////        WriteFloatImg(img_path,_M_creg->getTransfo1());

////        img_path = path+"/transfo2.txt";
////        WriteFloatImg(img_path,_M_creg->getTransfo1());


//////        _M_sparseofPCA.setDrawPoints(true);
//////        _M_creg->apply(img.img,rframe,INTER_LINEAR);
//////        img_path = path+"/img_RGB_reg_keypoints.png";
//////        imwrite(img_path.toStdString(),rframe);

//////        _M_sparseofPCA.setDrawPoints(false);



//    }


//    //Temp
//    if(img.thread_id>10819)
//    {
//        imwrite(QString("/home/caredda/temp/video/in_"+QString::number(img.thread_id)+".png").toStdString(),img.img);
//        imwrite(QString("/home/caredda/temp/video/out_"+QString::number(img.thread_id)+".png").toStdString(),rframe);
//    }


    if(img.thread_id==_M_tot_reg_frames-1)
    {
        _M_acquisition_is_over =true;
        emit registrationIsFinished();
        WriteTemporalVector(_M_result_directory+"NCC_RGB.txt",_M_NCC);
    }

    _Processed_img res;
    res.thread_id   = img.thread_id;
    res.img         = rframe;

    //emit results
    // QString path = "C:/Users/ccaredda/Desktop/tmp/reg_"+QString::number(res.thread_id)+".png";
    // imwrite(path.toStdString(),res.img);

    emit newRegistrationProgress("Registration",(100*img.thread_id)/(_M_tot_reg_frames-1));
    emit newRegisteredImage(res);

//    Qt<<(double)(timer.elapsed())<<endl;
}



//PRIVATE FUNCTIONS

//Slot called when a parameter related to _M_sparseofPCA is changed
void ARegistration::initSparseofPCA()
{
    qDebug()<<"ARegistration::initSparseofPCA 1";
    _M_sparseofPCA.setMaxKeypoints(_M_maxkey);
    _M_sparseofPCA.setDrawPoints(false);
//    _M_sparseofPCA.setDrawPoints(true);

//    _M_sparseofPCA.setDbgLevel(cmd.get<int>("dbg"),(out+"_dbg").c_str());

    //1st param : min dist between harris keypoints
    //2nd param : Harris distance QL
    _M_sparseofPCA.setHarrisParam(_M_harrisDist,_M_harrisQL,0.04);

    //1st param : camera motion model (trans or aff or persp) (for ofpca)
    //2nd param : use pixel with grad higher than this for camera motion fit (no grad weight if <0)
    //3rd param : compose camera motion model (for ofpca)
    _M_sparseofPCA.setCamMotion("persp",0,_M_camcomp);
    //Lagrangian (LAG), Large def Lag (LLAG), Eulerian (EULER)
    _M_sparseofPCA.setPtsFit(SparseOFPCA::str2pfit("LLAG"));
    //forwarb backward error weighting  (param is the Gaussian sigma)
    _M_sparseofPCA.setFBSigma(0);
    //number of PC keept in SparseOFPCA
    _M_sparseofPCA.setNPC(5);
    //pca is component by component
    _M_sparseofPCA.setPCAModel(false);


    //sigma at iteration 0 for IRLS and number of IRLS iterations
    _M_sparseofPCA.setIRLS(_M_irlsSig0,_M_irlsN);
    //keypoint consistency: param is the bin size of histo, <=0 not used
    _M_sparseofPCA.setKptConsist(-1);
    //border size (remove kpt in the border)
    _M_sparseofPCA.setBorder(_M_border);
    //sigma for init weighting with dy
    _M_sparseofPCA.setDySigma(_M_dySig);
    //1st param : regularisation weight, order 0, affine part
    //2nd param : regularisation weight, order 1, affine part
    //3rd param : regularisation weight, order 0, pca    part
    //4th param : regularisation weight, order 1, pca    part
    //5th param : delta in the constraint on the pca coeff: ||L(lpca-lpca0)||^2/npca < delta
    _M_sparseofPCA.setRegulWeight(0,1.0e-10,0,1.0e-10,_M_constPCA0);


    if (_M_lreg==0)
    {
        _M_enable_process=false;
        qDebug() << "Error: when using ofpca registration you must either set Lreg or loadtrans";
        return;
    }
    if (_M_Lnframe<=0)
    {
        _M_enable_process=false;
        qDebug() << "Error: when using ofpca with Lreg you must set Lnframe>0";
        return;
    }
    _M_enable_process=true;
    _M_sparseofPCA.setSampleSize(_M_Lnframe);

    qDebug()<<"ARegistration::initSparseofPCA end";

}

