#include "molarcoefffunctions.h"



void EraseSmallContour(Mat &img,int area_lim_in_px)
{
    //Find contours
    vector<vector<Point> > contours;
    findContours(img,contours,CV_RETR_TREE,CV_CHAIN_APPROX_NONE);

    //itterate over contour idx
    for(unsigned int i=0;i<contours.size();i++)
    {
        //cannot process the fit ellipse
        if(contours[i].size()<5)
        {
            drawContours(img,contours,i,Scalar(0),CV_FILLED);
            continue;
        }
        if(contourArea(contours[i])<area_lim_in_px)
            drawContours(img,contours,i,Scalar(0),CV_FILLED);
//        else
//        {
//            if(contourArea(contours[i])<area_lim_in_px)
//                drawContours(img,contours,i,Scalar(0),CV_FILLED);
//            else
//            {
//                //Fit an ellipse for each contour
//                RotatedRect rec = fitEllipse(contours[i]);

//                //control the height and width of the rotated rect
//                // If it is inferiror to 1 mm -> define the blood vessel as GM
//                if( rec.size.width<reso_lim_px)
//                    drawContours(img,contours,i,Scalar(0),CV_FILLED);
//            }
//        }
    }
}

void fillContour1_If_Contour2_IsNotIncluded(Mat &img,vector<vector<Point> > &c1,vector<vector<Point> > &c2)
{
    //loop over c1
    for(unsigned int i=0;i<c1.size();i++)
    {
        bool isinside = false;
        //loop over c2
        for(unsigned int j=0;j<c2.size();j++)
        {
            //Loop over the points of c2[j] contour
            for(unsigned int k=0;k<c2[j].size();k++)
            {
                if(pointPolygonTest(c1[i], c2[j][k], false)>0)
                {
                    isinside = true;
                    break;
                }
            }

            if(isinside)
                break;
        }
        //Fill contour
        if(!isinside)
            drawContours(img,c1,i,Scalar(255),CV_FILLED);
    }
}

void handleInternalContour(Mat &img1, Mat &img2)
{
    //Find contours and hierarchy
    vector<vector<Point> > c1;
    vector<vector<Point> > c2;
    vector<Vec4i> hierarchy1;
    vector<Vec4i> hierarchy2;

    findContours(img1,c1,hierarchy1,CV_RETR_TREE,CV_CHAIN_APPROX_NONE);
    findContours(img2,c2,hierarchy2,CV_RETR_TREE,CV_CHAIN_APPROX_NONE);

//    //Find internal contours
//    vector<int> id1;
//    //iterate over contour 1 idx
//    for(unsigned int i=0;i<c1.size();i++)
//    {
//        // check if ith contours is a child (or has a parent) -> is an internal contour
//        if(hierarchy1[i][3]!=-1)
//            id1.push_back(i);
//    }

//    vector<int> id2;
//    //iterate over contour 2 idx
//    for(unsigned int i=0;i<c2.size();i++)
//    {
//        // check if ith contours is a child (or has a parent) -> is an internal contour
//        if(hierarchy2[i][3]!=-1)
//            id2.push_back(i);
//    }

    //Check if c2 is inside c1
    //If not fill c1 contour
    fillContour1_If_Contour2_IsNotIncluded(img1,c1,c2);

    //Check if c1 is inside c2
    //If not fill c2 contour
    fillContour1_If_Contour2_IsNotIncluded(img2,c2,c1);

}


double vector_sum(QVector<double> a)
{
    double res = 0.0;

    for(int i=0;i<a.size();i++)
        res+= a[i];
    return res;
}

QVector<double> element_wise_multiplication(QVector<double> &a,QVector<double> &b)
{
    QVector<double> res;
    res.clear();
    if(a.size()!=b.size())
    {
        qDebug()<<"[element_wise_multiplication] differnt size of input vectors";
        return res;
    }
    for(int i=0;i<a.size();i++)
        res.push_back(a[i]*b[i]);

    return res;
}

void scale_vector(QVector<double> &in_out,double factor)
{
    if(factor==0)
        return;

    for(int i=0;i<in_out.size();i++)
    {
        in_out[i]*=factor;
    }
}


void invertMatrix(Mat& in,Mat& out)
{
    // E_RGB_inv_A = inv(E_RGB_detectA'*E_RGB_detectA)*E_RGB_detectA' ;
    Mat tr_in;
    transpose(in,tr_in);
//    out = tr_in * in;
    mulTransposed(in,out,true);
    invert(out,out,DECOMP_SVD);
    out = out *tr_in;
}

void invertMatrix(QVector<Mat>& in,QVector<Mat>& out)
{
    // E_RGB_inv_A = inv(E_RGB_detectA'*E_RGB_detectA)*E_RGB_detectA' ;
    out.clear();
    for(int i=0;i<in.size();i++)
    {
        Mat temp;
        invertMatrix(in[i],temp);
        out.push_back(temp);
    }
}



void integrateDatas(QVector<Mat> &out, QVector<QVector<double> > &spectral_res_Hb, QVector<QVector<double> >&spectral_res_HbO2, QVector<double> &Mean_path, QVector<QVector<int> > &id)
{
    out.clear();
    for(int i=0;i<id.size();i++)
        out.push_back(Mat::zeros(id[i].size(),id.size(),CV_64F));

    for(int n = 0;n<id.size();n++)
    {
        for(int row=0;row<out[n].rows;row++)
        {
            double *ptr = out[n].ptr<double>(row);
            for(int i=0;i<Mean_path.size();i++)
            {
                ptr[1]  = ptr[1]  + spectral_res_Hb[id[n][row]][i]*Mean_path[i];
                ptr[0]  = ptr[0]  + spectral_res_HbO2[id[n][row]][i]*Mean_path[i];
            }
        }
    }
}

void integrateDatas_Hb_HbO2_oxCCO(QVector<Mat> &out, QVector<QVector<double> > &spectral_res_Hb, QVector<QVector<double> >&spectral_res_HbO2, QVector<QVector<double> >&spectral_res_oxCCO, QVector<double> &Mean_path, QVector<QVector<int> > &id)
{
    out.clear();
    for(int i=0;i<id.size();i++)
        out.push_back(Mat::zeros(id[i].size(),id.size(),CV_64F));

    for(int n = 0;n<id.size();n++)
    {
        for(int row=0;row<out[n].rows;row++)
        {
            double *ptr = out[n].ptr<double>(row);
            for(int i=0;i<Mean_path.size();i++)
            {
                ptr[1]  = ptr[1]  + spectral_res_Hb[id[n][row]][i]*Mean_path[i];
                ptr[0]  = ptr[0]  + spectral_res_HbO2[id[n][row]][i]*Mean_path[i];
                ptr[2]  = ptr[2]  + spectral_res_oxCCO[id[n][row]][i]*Mean_path[i];
            }
        }
    }

}

void integrateDatas_Hb_HbO2_oxCCO(QVector<Mat> &out, QVector<QVector<double> > &spectral_res_Hb, QVector<QVector<double> >&spectral_res_HbO2, QVector<QVector<double> >&spectral_res_oxCCO, QVector<double> &Mean_path)
{
    out.clear();
    out.push_back(Mat::zeros(spectral_res_Hb.size(),3,CV_64F));
    for(int row = 0;row<spectral_res_Hb.size();row++)
    {
        double *ptr = out[0].ptr<double>(row);
        for(int i=0;i<Mean_path.size();i++)
        {
            ptr[1]  = ptr[1]  + spectral_res_Hb[row][i]*Mean_path[i];
            ptr[0]  = ptr[0]  + spectral_res_HbO2[row][i]*Mean_path[i];
            ptr[2]  = ptr[2]  + spectral_res_oxCCO[row][i]*Mean_path[i];
        }
    }
}

void integrateDatas(QVector<Mat> &out, QVector<QVector<double> > &spectral_res_Hb, QVector<QVector<double> >&spectral_res_HbO2, QVector<double> &Mean_path)
{
    out.clear();
    out.push_back(Mat::zeros(spectral_res_Hb.size(),2,CV_64F));

    for(int row = 0;row<out[0].rows;row++)
    {
        double *ptr = out[0].ptr<double>(row);
        ptr[0] = vector_sum(element_wise_multiplication(spectral_res_HbO2[row],Mean_path));
        ptr[1] = vector_sum(element_wise_multiplication(spectral_res_Hb[row],Mean_path));
    }
}


void getDistanceMatrix(float reso_in_mm,Mat &dst_matrix,Mat &blood_vessel_type,Mat &mask,Mat &S_BV,Mat &B_BV,Mat &unused)
{
    //init dst_matrix (dst between the closest contour
    dst_matrix          = Mat::zeros(mask.size(),CV_64FC1);
    //init blood type - the GM point is close to a surface blood vessel (1) or a buried blood vessel (2)
    blood_vessel_type   = Mat::zeros(mask.size(),CV_8UC1);
    //GM layer
    Mat GM              = Mat::zeros(mask.size(),CV_8UC1);


    //Find contours
    vector<vector<Point> > c_SBV;
    vector<vector<Point> > c_BBV;
    vector<vector<Point> > c_unused;
    vector<vector<Point> > c_mask;

    findContours(mask,c_mask,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
    findContours(S_BV,c_SBV,CV_RETR_TREE,CV_CHAIN_APPROX_NONE);
    findContours(B_BV,c_BBV,CV_RETR_TREE,CV_CHAIN_APPROX_NONE);
    findContours(unused,c_unused,CV_RETR_TREE,CV_CHAIN_APPROX_NONE);

    //Create GM layer
    drawContours(GM,c_mask,-1,Scalar(255),CV_FILLED);
    drawContours(GM,c_SBV,-1,Scalar(0),CV_FILLED);
    drawContours(GM,c_BBV,-1,Scalar(0),CV_FILLED);
    drawContours(GM,c_unused,-1,Scalar(0),CV_FILLED);

//    imwrite("/home/caredda/DVP/C++/imagerie_fonctionnelle_cerveau/test_matrice_distance/Calcul_distance_matrix/GM_layer.tif",GM);


    //Get min dst from blood vessels
    for(int row=0;row<mask.rows;row++)
    {
        uchar *ptr_GM           = GM.ptr<uchar>(row);
        double *ptr_dst_mat     = dst_matrix.ptr<double>(row);
        uchar *ptr_blood_type   = blood_vessel_type.ptr<uchar>(row);

        for(int col=0;col<mask.cols;col++)
        {
            if(ptr_GM[col] == 255)
            {
                //Get min dst between point P(col,row) and the surface blood contours
                double min_SBV =100000;
                for(unsigned int i=0;i<c_SBV.size();i++)
                {
                    double dst = abs(pointPolygonTest(c_SBV[i],Point2f(col,row),true));
                    min_SBV = (dst<min_SBV) ? dst : min_SBV;
                }

                //Get min dst between point P(col,row) and the buried blood contours
                double min_BBV =100000;
                for(unsigned int i=0;i<c_BBV.size();i++)
                {
                    double dst = abs(pointPolygonTest(c_BBV[i],Point2f(col,row),true));
                    min_BBV = (dst<min_BBV) ? dst : min_BBV;
                }

                //Store min dist between min_SBV and min_BBV in dst_matrix
                ptr_dst_mat[col]    = (min_SBV<min_BBV)? reso_in_mm*min_SBV : reso_in_mm*min_BBV;
                //Store if the closest blood contour is surfacic or buried
                ptr_blood_type[col] = (min_SBV<min_BBV)? 1 : 2;
            }
        }
    }
}

//return the id of the best spectral match
void find_best_simulated_spectra(QVector<QVector<double> > &simulated_spectra,QVector<double> &in_spectra,int &id,float &error)
{
    error   = 1000000000000;
    id      = 0;

    for(int i=0;i<simulated_spectra.size();i++)
    {
        float sum=0;
        for(int j=0;j<simulated_spectra[i].size();j++)
            sum += pow(simulated_spectra[i][j] - in_spectra[j],2);

        sum= sum/1638400; //2^16 * 25

        if(sum<error)
        {
            error   = sum;
            id      = i;
        }
    }
}
