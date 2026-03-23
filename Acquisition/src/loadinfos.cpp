#include "loadinfos.h"


//Check if acquisition info file contains all required info
QStringList check_acquisition_info_file(const QString &path)
{
    QStringList required_items = { "Theorical", "RGB_camera", "Camera_name",
                                   "Light_source_type", "Start", "End", "Frame" };

    QString filePath = path + "/Acquisition_infos.txt";
    QFile file(filePath);

    QStringList missing_items;

    if (!file.exists()) {
        qDebug() << "[check_acquisition_info_file] File is missing:" << filePath;
        return required_items; // all are missing
    }

    QStringList found_items;

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.isEmpty())
                continue;

            QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if (!parts.isEmpty())
                found_items << parts[0];
        }
        file.close();
    } else {
        qDebug() << "[check_acquisition_info_file] Cannot open file:" << filePath;
        return required_items; // all missing if file can’t be read
    }

    // Compare found vs required
    for (const QString &item : required_items) {
        if (!found_items.contains(item)) {
            missing_items << item;
        }
    }

    return missing_items;
}
// Load Frame rate info
double LoadFrameRateInfo(QString path)
{
    double FPS=-1;
    QFile file(path+"/Acquisition_infos.txt");

    if(!file.exists())
        return FPS;

    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);

        while(!in.atEnd())
        {
            QStringList list = in.readLine().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if(!list.empty())
            {
                if(list[0]=="Theorical")
                {
                    FPS = round(list[list.size()-1].toDouble());
                    qDebug()<<"Found! "<<FPS;
                    break;
                }
            }
        }
    }
    return FPS;
}

//Load camera type (RGB or HSI)
int LoadCameraType(QString path)
{
    int is_camera_RGB = -1;
    QFile file(path+"/Acquisition_infos.txt");

    if(!file.exists())
        return is_camera_RGB;

    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);

        while(!in.atEnd())
        {
            QStringList list = in.readLine().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if(!list.empty())
            {
                if(list[0]=="RGB_camera")
                {
                    is_camera_RGB = int((list[list.size()-1].toInt()));
                    qDebug()<<"Found! "<<is_camera_RGB;
                    break;
                }
            }
        }
    }
    return is_camera_RGB;
}

//Load camera name
QString LoadCameraName(QString path)
{
    QString camera_name = "";
    QFile file(path+"/Acquisition_infos.txt");

    if(!file.exists())
        return camera_name;

    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);

        while(!in.atEnd())
        {
            QStringList list = in.readLine().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if(!list.empty())
            {
                if(list[0]=="Camera_name")
                {
                    camera_name = list[list.size()-1];
                    qDebug()<<"Found! "<<camera_name;
                    break;
                }
            }
        }
    }
    return camera_name;
}

//Load camera name
QString LoadLightSourceType(QString path)
{
    QString Light_source_type = "";
    QFile file(path+"/Acquisition_infos.txt");

    if(!file.exists())
        return Light_source_type;

    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);

        while(!in.atEnd())
        {
            QStringList list = in.readLine().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if(!list.empty())
            {
                if(list[0]=="Light_source_type")
                {
                    Light_source_type = list[list.size()-1];
                    qDebug()<<"Found! "<<Light_source_type;
                    break;
                }
            }
        }
    }
    return Light_source_type;
}


QVector<int> LoadTotalFrameCountInfo(QString path)
{
    QVector<int> Total_Frame;
    QFile file(path+"/Acquisition_infos.txt");

    if(!file.exists())
    {
        Total_Frame.push_back(-1);
        return Total_Frame;
    }

    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);
        while(!in.atEnd())
        {
            QStringList list = in.readLine().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if(!list.empty())
            {
                if(list[0]=="Frame")
                {
                    Total_Frame.push_back(round(list[list.size()-1].toInt()));
                }
            }
        }
    }
    return Total_Frame;
}
// Load process id infos
void LoadTimeProcessInfo(QString path,QVector<int> &process_id,int tot_frames)
{
    process_id.clear();

    QFile file(path+"/Acquisition_infos.txt");

    // //If file does not exist, indicate 2 steps (1 periode of stimulation)
    // if(!file.exists())
    // {
    //     process_id.push_back((int)(tot_frames/3));
    //     process_id.push_back((int)(2*tot_frames/3));
    //     return;
    // }

    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);

        while(!in.atEnd())
        {
            QStringList list = in.readLine().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if(!list.empty())
            {
                if(list[0]=="Step")
                    process_id.push_back((int)((list[list.size()-1].toInt())));
            }
        }
    }
}

// Load process id infos
//empty: no analysis writen in info file
//0: task-based
//1: resting state
//2: impulsion
QVector<int> LoadAnalysisType(QString path)
{
    QVector<int> analysis;

    QFile file(path+"/Acquisition_infos.txt");

    //If file does not exist return -1 and exit
    if(!file.exists())
    {
        return analysis;
    }

    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);

        while(!in.atEnd())
        {
            QStringList list = in.readLine().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if(!list.empty())
            {
                if(list.size()>2)
                {
                    QString key = list[0]+" "+list[1];
                    if(key == "Start task-based")
                        analysis.push_back(0);
                    if(key == "Start resting-state")
                        analysis.push_back(1);
                    if(key == "Start Impulsion")
                        analysis.push_back(2);
                }
            }
        }
    }

    return analysis;
}

/** Load start and end of the analysis
*  -1: no analysis
*  0 : task-based
*  1: resting state
*  2: Impulsion*/
void LoadStartEndAnalysisFrames(QString path,int analysis,int &start_frame,int &end_frame)
{
    QFile file(path+"/Acquisition_infos.txt");

    //If file does not exist, indicate 2 steps (1 periode of stimulation)
    if(!file.exists())
    {
        start_frame = 0;
        end_frame = 0;
        return;
    }

    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);

        while(!in.atEnd())
        {
            QStringList list = in.readLine().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if(!list.empty())
            {
                if(list.size()>2)
                {
                    QString key = list[0]+" "+list[1];

                    if(analysis == 0 && key == "Start task-based")
                        start_frame = (int)((list[list.size()-1].toInt()));
                    if(analysis == 0 && key == "End task-based")
                        end_frame = (int)((list[list.size()-1].toInt()));

                    if(analysis == 1 && key == "Start resting-state")
                        start_frame = (int)((list[list.size()-1].toInt()));
                    if(analysis == 1 && key == "End resting-state")
                        end_frame = (int)((list[list.size()-1].toInt()));


                    if(analysis == 2 && key == "Start Impulsion")
                        start_frame = (int)((list[list.size()-1].toInt()));
                    if(analysis == 2 && key == "End Impulsion")
                        end_frame = (int)((list[list.size()-1].toInt()));

                }
            }
        }
    }
}








