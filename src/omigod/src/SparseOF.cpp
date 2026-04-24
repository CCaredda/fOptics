#include <SparseOF.h>

#include <Timer.h>
#include <NiftiIO.h>

#include <iostream>
#include <fstream> 
#include <cstdio>

# define M_PI		3.14159265358979323846	/* pi */


//#include <opencv2/highgui/highgui.hpp>


using namespace std;
using namespace cv;
using namespace omigod;



SparseOF::SparseOF ()
 : Reg()
{ levels   = 3;
  winsize  = 11;
  maxiter  = 10;

  maxKeyPt = 50;

  mt.tMax     = winsize+1;
  mt.tIncr    = 1;
  mt.thetamax = M_PI/8.0;
  mt.ntheta05 = 0;
  mt.winsize  = winsize;
  mt.descstep = 1;

  useLKfit = true;

  drawPts = true;

  setHarrisParam(10, 1e-10, 0.04);
  lkParam.fbSig = -1;

  borderSize = 0;
}
 



void SparseOF::setFrame0(const cv::Mat & img)
{ 
  Reg::setFrame0(img);

  // clear pts vector
  VecPts().swap(pts0);
  VecPts().swap(pts);
  
  // compute frame0 pyramid
  pframe0 = img;


  //detect keypoints
  int maxCorners      = maxKeyPt;
//  double qualityLevel = 0.1;
//  double qualityLevel = 0.000001;
//  double minDistance  = 10;
//  InputArray mask=noArray();
  int blockSize=11;
  bool useHarrisDetector=true;
//  double k=0.04;

 // apply the CLAHE algorithm to the L channel
// cv::Mat imgEq;
// cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(4,Size(8,8));
//// cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(1,Size(2,2));
// clahe->apply(img, imgEq);
 
// GaussianBlur( img, imgEq, Size(11,11), 1, 1, BORDER_REPLICATE );
// imgEq = img - imgEq;
// imgEq=imgEq*10;
// 
// imwrite("img.png"  ,img  );
// imwrite("imgEq.png",imgEq);

 Mat imgEq = img;
  
//  if (harrisGridSize>0)
//    { uniformHarrisPts(imgEq, harrisGridSize, harrisGridSize, harris.k, blockSize, pts0);
//    }
//  else
//    { 

        maxCorners = SHRT_MAX -1;
        pts0.clear();
      goodFeaturesToTrack(imgEq, pts0, maxCorners, harris.qualityLevel, harris.minDist, noArray(), blockSize, useHarrisDetector, harris.k);
//    }
  
  // remove pts to close to the image border
  pts.clear();
  pts.reserve(pts0.size());
  for (unsigned int i=0;i<pts0.size();i++)
    { const VecPts::value_type & p = pts0[i];
      if (    p.x>=borderSize && p.x< img.cols-borderSize
           && p.y>=borderSize && p.y< img.rows-borderSize )
        { pts.push_back(p);
        }
    } 
  VecPts(pts).swap(pts);
//  pts.shrink_to_fit();

  // pts on current frame: copy
//  pts0.clear()
//  pts0.shrink_to_fit();
  VecPts().swap(pts0);
  pts0 = pts;

//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << pts0[0].x << " " << pts[0].x << std::endl; 
  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << pts0.size() << " " << pts.size() << std::endl; 

//  // train  on patch around keypoint 
//  if (!useLKfit)
//    { trainkeypointmotion(); 
//    }

//  int nlevel = buildOpticalFlowPyramid(InputArray img
//  OutputArrayOfArrays pyramid
//  Size winSize
//  int maxLevel
//  bool withDerivatives=true
//  int pyrBorder=BORDER_REFLECT_101
//  int derivBorder=BORDER_CONSTANT
//  bool tryReuseInputImage=true)
//  int nlevel = buildOpticalFlowPyramid(img, OutputArrayOfArrays pyramid, Size winSize, int maxLevel, bool withDerivatives=true, int pyrBorder=BORDER_REFLECT_101, int derivBorder=BORDER_CONSTANT, bool tryReuseInputImage=true)
//
//    Parameters:	
//
//        img – 8-bit input image.
//        pyramid – output pyramid.
//        winSize – window size of optical flow algorithm. Must be not less than winSize argument of calcOpticalFlowPyrLK(). It is needed to calculate required padding for pyramid levels.
//        maxLevel – 0-based maximal pyramid level number.
//        withDerivatives – set to precompute gradients for the every pyramid level. If pyramid is constructed without the gradients then calcOpticalFlowPyrLK() will calculate them internally.
//        pyrBorder – the border mode for pyramid layers.
//        derivBorder – the border mode for gradients.
//        tryReuseInputImage – put ROI of input image into the pyramid if possible. You can pass false to force data copying.
//

}
 


void SparseOF::setParam(int levels_, int winsize_, int maxiter_)
{ 
  Reg::setParam(levels_, winsize_, maxiter_);
  mt.winsize = winsize;
}
 

void SparseOF::regPts(const cv::Mat & img, const cv::Mat * prev, const cv::Mat * affine)
{ 
//  int n = pts.size();
 
//  lkParam.status; .resize(n));
//  vector<float> err;


  Size winSize=Size(winsize,winsize);
  int maxLevel=levels-1;
//  TermCriteria criteria=TermCriteria(TermCriteria::COUNT+TermCriteria::EPS,maxiter,0.01);
  lkParam.criteria=TermCriteria(TermCriteria::COUNT,maxiter,0.01);
  lkParam.flags=OPTFLOW_USE_INITIAL_FLOW;
  lkParam.minEigThreshold=1e-4;
  

  // forwarb backward Error
  TermCriteria fbCriteria(TermCriteria::COUNT,maxiter,0.01);
  VecPts fbPts0;
  if (lkParam.fbSig>0)
    { fbPts0 = pts0;
      lkParam.fbError.resize(pts0.size());
    }


  if (affine!=0)   // Large Lagrangian
    { 
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
//      std::cout << *affine << std::endl; 
      
      if (affine->rows==2) // affine
        { 
          Mat imgA;    warpAffine(img, imgA, *affine, img.size(), INTER_LINEAR|WARP_INVERSE_MAP, BORDER_REPLICATE);
          Mat invAff;  invertAffineTransform(*affine, invAff);
          VecPts ptsA; transform 	(pts, ptsA, invAff );

          calcOpticalFlowPyrLK(pframe0, imgA, pts0, ptsA, lkParam.status, lkParam.err, winSize, maxLevel, lkParam.criteria, lkParam.flags, lkParam.minEigThreshold);
          if (lkParam.fbSig>0)
            { calcOpticalFlowPyrLK(imgA, pframe0, ptsA, fbPts0,  lkParam.status, lkParam.err, winSize, maxLevel, fbCriteria, lkParam.flags, lkParam.minEigThreshold);
              for (unsigned int i=0;i<pts0.size();i++) lkParam.fbError[i] = hypotf(pts0[i].x-fbPts0[i].x, pts0[i].y-fbPts0[i].y);
            }
          transform 	(ptsA, pts, *affine );
        }
      else // persp
        { Mat U = *affine / norm(*affine);
          Mat imgU;    warpPerspective(img, imgU, U, img.size(), INTER_LINEAR|WARP_INVERSE_MAP, BORDER_REPLICATE);
          Mat invU = U.inv();
          VecPts ptsU; perspectiveTransform 	(pts, ptsU, invU );

          calcOpticalFlowPyrLK(pframe0, imgU, pts0, ptsU, lkParam.status, lkParam.err, winSize, maxLevel, lkParam.criteria, lkParam.flags, lkParam.minEigThreshold);
          if (lkParam.fbSig>0)
            { calcOpticalFlowPyrLK(imgU, pframe0, ptsU, fbPts0,  lkParam.status, lkParam.err, winSize, maxLevel, fbCriteria, lkParam.flags, lkParam.minEigThreshold);
              for (unsigned int i=0;i<pts0.size();i++) lkParam.fbError[i] = hypotf(pts0[i].x-fbPts0[i].x, pts0[i].y-fbPts0[i].y);
            }
          perspectiveTransform(ptsU, pts, U);
        }
    }
  else if (prev!=0) // Eulerian
    { 
//      VecPts ptsB = pts;
//      calcOpticalFlowPyrLK(pframe0, img, pts0, ptsB, lkParam.status, lkParam.err, winSize, maxLevel, lkParam.criteria, lkParam.flags, lkParam.minEigThreshold);
      VecPts ptsPrev = pts;
      calcOpticalFlowPyrLK(*prev, img, ptsPrev, pts, lkParam.status, lkParam.err, winSize, maxLevel, lkParam.criteria, lkParam.flags, lkParam.minEigThreshold);
      if (lkParam.fbSig>0)
        { calcOpticalFlowPyrLK(img, *prev, pts, fbPts0,  lkParam.status, lkParam.err, winSize, maxLevel, fbCriteria, lkParam.flags, lkParam.minEigThreshold);
          for (unsigned int i=0;i<pts0.size();i++) lkParam.fbError[i] = hypotf(ptsPrev[i].x-fbPts0[i].x, ptsPrev[i].y-fbPts0[i].y);
        }
//      for (unsigned int i=0;i<pts.size();i++) cout << pts0[i] << "  " << pts[i] << "   " << ptsB[i] << " " << pts[i] - ptsB[i] << endl;
    }
  else // Lagrangian
    {
      calcOpticalFlowPyrLK(pframe0, img, pts0, pts, lkParam.status, lkParam.err, winSize, maxLevel, lkParam.criteria, lkParam.flags, lkParam.minEigThreshold);

      if (lkParam.fbSig>0)
        {
          calcOpticalFlowPyrLK(img, pframe0, pts, fbPts0,  lkParam.status, lkParam.err, winSize, maxLevel, fbCriteria, lkParam.flags, lkParam.minEigThreshold);
          for (unsigned int i=0;i<pts0.size();i++) lkParam.fbError[i] = hypotf(pts0[i].x-fbPts0[i].x, pts0[i].y-fbPts0[i].y);
        }
    }
  
}
 


void SparseOF::computeReg(const cv::Mat & img)
{ 

//    cout<<"In SparseOF::computeReg"<<endl;

  int n = pts.size();
 
//  vector<unsigned char> status(pts0.size());
//  vector<float> err;
//  Size winSize=Size(winsize,winsize);
//  int maxLevel=levels-1;
////  TermCriteria criteria=TermCriteria(TermCriteria::COUNT+TermCriteria::EPS,maxiter,0.01);
//  TermCriteria criteria=TermCriteria(TermCriteria::COUNT,maxiter,0.01);
////  int flags=OPTFLOW_USE_INITIAL_FLOW;
//  int flags=OPTFLOW_USE_INITIAL_FLOW;
//  double minEigThreshold=1e-4;
    
//  lkParam.status; .resize(n));
//  vector<float> err;
  Size winSize=Size(winsize,winsize);
  int maxLevel=levels-1;
//  TermCriteria criteria=TermCriteria(TermCriteria::COUNT+TermCriteria::EPS,maxiter,0.01);
  lkParam.criteria=TermCriteria(TermCriteria::COUNT,maxiter,0.01);
  lkParam.flags=OPTFLOW_USE_INITIAL_FLOW;
  lkParam.minEigThreshold=1e-4;
    
  if (useLKfit)
    {
//        std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << winSize<< std::endl; 
      calcOpticalFlowPyrLK(pframe0, img, pts0, pts, lkParam.status, lkParam.err, winSize, maxLevel, lkParam.criteria, lkParam.flags, lkParam.minEigThreshold);
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 

//      for (int i=0;i<n;i++)
//        { printf("%4d %d    %f %f\n",i,status[i], pts[i].y-pts0[i].y, pts[i].x-pts0[i].x);
//        }


//  vector<unsigned char> beststatus(pts0.size(),1);
//  vector<float>         besterr   (pts0.size(),1e10);
//  VecPts trans0(5), ptscurr(pts0.size()), bestpts(pts0.size());
//  trans0[0] = Point2f( 0, 0);
//  trans0[1] = Point2f( 2, 0);
//  trans0[2] = Point2f(-2, 0);
//  trans0[3] = Point2f( 0, 2);
//  trans0[4] = Point2f( 0,-2);
// 
//  for (unsigned k=0;k<trans0.size();k++)
//    { 
//      for (int i=0;i<n;i++) ptscurr[i] = pts[i]+trans0[k];
//      calcOpticalFlowPyrLK(pframe0, next, pts0, ptscurr, status, err, winSize, maxLevel, criteria, flags, minEigThreshold);
//      for (int i=0;i<n;i++)
//        { 
////          nerr += status[i];
//          if (status[i]==1 && err[i]<=besterr[i])
//            { 
//              beststatus[i] = 1;
//              besterr   [i] = err[i];
//              bestpts   [i] = ptscurr[i];
////              nerr2+=status[i];
//            }  
//        }
//    }
//  for (int i=0;i<n;i++)
//    { pts[i] = beststatus[i]==1 ? bestpts[i] : pts[i];
//      std::cout <<"cxcy : "<< i << " " << (int)beststatus[i] << " " << besterr[i] << std::endl; 
//    }


//  for (int i=0;i<n;i++)
//    { nerr += status[i];
//      if (status[i]==0 && err[i]<=besterr[i])
//        { 
////          vector<unsigned char> status2(1);
////          vector<float> err2;
////          VecPts pts20(1), pts2(1); 
////          pts20[0] = pts0[i];
////          pts2[0]  = pts[i];
////          calcOpticalFlowPyrLK(pframe0, next, pts20, pts2, status2, err2, winSize, maxLevel, criteria, 0, minEigThreshold);
////                pts0todo2.push_back();
////              pts0todo2.push_back();
//          nerr2+=status[i];
//          pts0[i] = pts20[0];
//          pts[i]  = pts2[0];
//        }  
//    }

    }

  else
    {
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
      // local displacement using trained matrices
      int Npatch = (2*mt.winsize+1)*(2*mt.winsize+1);
      int Ndesc  = Npatch/mt.descstep;
      double * desc  = new double[2*Ndesc];
      double * desc0 = desc + Ndesc;
      Mat patch(2*mt.winsize+1,2*mt.winsize+1,pframe0.type());
      for (int i=0;i<n;i++)
        {
          getPatch(pframe0, pts0[i].x, pts0[i].y, 0, patch); localDesc(patch, desc0);
          getPatch(img    , pts [i].x, pts [i].y, 0, patch); localDesc(patch, desc );

//          localDesc(pframe0, pts0[i].x, pts0[i].y, desc0);
//          localDesc(next,    pts [i].x, pts [i].y,  desc);
//          for (int ip=0;ip<Npatch;ip++) desc[ip] = desc0[ip] - desc[ip];

          double tx = 0, ty = 0;
          for (int ip=0;ip<Ndesc;ip++) 
            { double di = desc0[ip] - desc[ip];
              tx += motionMat[i].at<double>(0,ip) * di;
              ty += motionMat[i].at<double>(1,ip) * di;
            }
          
          pts[i].x -= tx;
          pts[i].y -= ty;

//          std::cout <<"pxpy : "<< i << " " << pts[i].x << " " << pts[i].y << " " << tx << " " << ty<< std::endl; 
          std::cout <<"txty : "<< i << " " << tx << " " << ty<< std::endl; 
        }
      delete[] desc;
    }


}

void SparseOF::setDrawPoints(bool _drawpts)
{ drawPts = _drawpts;
}


void SparseOF::apply(const cv::Mat & img, cv::Mat & out, int interpolation) const
{
  Reg::apply(img, out, interpolation);
//
//  remap(img, out, transfo1, transfo2, interpolation);

//  out = img;

  if (drawPts)
    { 
      for (unsigned int i=0;i<pts0.size();i++)
        { //Point center();
          int thickness = 1, linetype = 8;
          line  ( out, pts0[i], pts[i], Scalar( 255, 255, 0 ), thickness, linetype);
          circle( out, pts0[i], 2,      Scalar( 0  , 255, 0 ), thickness, linetype );
    //      circle( out, pts [i], 2, Scalar( 255, 255, 0 ), thickness, linetype );

        }
    }
}
 




void SparseOF::trainkeypointmotion()
{ 
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << pframe0.rows << " " << pframe0.cols << std::endl; 
//  ofstream file0; file0.open("img0.txt"); file0 << pframe0; file0.close();
  // motion matrix size
  int Ntransf=0;
  for (int ty=-mt.tMax;ty<=mt.tMax;ty+=mt.tIncr) 
  for (int tx=-mt.tMax;tx<=mt.tMax;tx+=mt.tIncr) Ntransf++;
    
  Ntransf *= 2*mt.ntheta05+1;
  Ntransf--;

  // reallocate motionMat vector
  vector<Mat>().swap(motionMat);
  motionMat.resize(pts0.size());
  
  int Npatch = (2*mt.winsize+1)*(2*mt.winsize+1);
  int Ndesc  = Npatch/mt.descstep;
  // allocate linear system 
  cv::Mat H(Ntransf,Ndesc,CV_64FC1);
  Mat A;//(Npatch,2,CV_64FC1);      
  cv::Mat Y(Ntransf,2,CV_64FC1);
  cv::Mat Y1(Ntransf,1,CV_64FC1);
  int it=0;
  for (int tt=-mt.ntheta05;tt<=mt.ntheta05;tt++)
  for (int ty=-mt.tMax;    ty<=mt.tMax;    ty+=mt.tIncr)
  for (int tx=-mt.tMax;    tx<=mt.tMax;    tx+=mt.tIncr)
    { 
      if (tx==0&&ty==0&&tt==0) continue;
      Y.at<double>(it, 0) = tx;
      Y.at<double>(it, 1) = ty;
      Y1.at<double>(it, 0) = tx;
      it++;
    }

//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 

  Mat patch(2*mt.winsize+1,2*mt.winsize+1,pframe0.type());
  double * desc  = new double[2*Ndesc];
  double * desc0 = desc + Ndesc;
  // keypoints
  for (unsigned int i=0;i<pts0.size();i++)
    { const Point2f & pi = pts0[i];

//      localDesc(pframe0, pi.x, pi.y, desc0);
      getPatch(pframe0, pi.x, pi.y, 0, patch); 
      localDesc(patch, desc0);
          
      cout << patch << endl;


//      localDesc(pframe0, pi.x, pi.y, desc);
//      double diff = 0.0;
//      for (int k=0;k<Ndesc;k++) diff += std::abs(desc[k]-desc0[k]);
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << diff/double(Ndesc) << std::endl; 
        
      

//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << i << std::endl; 
      H = 0.0;
      // translation
      int it = 0;
      for (int tt=-mt.ntheta05;tt<=mt.ntheta05;tt++)
      for (int ty=-mt.tMax;    ty<=mt.tMax;    ty+=mt.tIncr)
      for (int tx=-mt.tMax;    tx<=mt.tMax;    tx+=mt.tIncr)
        { //localDesc(pframe0, pi.x+tx, pi.y+ty, desc);
          double theta = mt.ntheta05==0 ? 0.0 : tt * mt.thetamax / double(mt.ntheta05);

          if (tx==0&&ty==0&&tt==0) continue;
      
          getPatch(pframe0, pi.x+tx, pi.y+ty, theta, patch); 
          localDesc(patch, desc);
          cout << tx << " " << ty << endl << " " << patch << endl;

          for (int ip=0;ip<Ndesc;ip++) H.at<double>(it, ip) = desc0[ip] - desc[ip];
//          for (int ip=0;ip<Ndesc;ip++) cout << ip << " " << H.at<double>(it, ip) << " " << desc0[ip] << " " << desc[ip] << endl;

          it++;
        }
       exit(1) ;
      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " H " << H.rows << " " << H.cols << " " << H.channels() << std::endl; 
      bool ret = solve(H, Y, A, DECOMP_SVD); //DECOMP_QR also ok
      // compute residual
      Mat R0; subtract(H*A,Y,R0); 
      Mat R; absdiff(H*A,Y,R); 
      double resid2x = -1; for (int l=0;l<Ntransf;l++) { double rl = R.at<double>(l,0); if (rl>resid2x) resid2x=rl; }
      double resid2y = -1; for (int l=0;l<Ntransf;l++) { double rl = R.at<double>(l,1); if (rl>resid2y) resid2y=rl; }
      for (int l=0;l<Ntransf;l++) 
          { double rx = R0.at<double>(l,0),yx = Y.at<double>(l,0); 
            double ry = R0.at<double>(l,1),yy = Y.at<double>(l,1); 
              cout << "res: " <<  l << " " << yx << " " << rx+yx << " " << rx << " | " 
                                    << " " << yy << " " << ry+yy << " " << ry <<endl;
          }
//      double resid = mean(R).val[0];
      double resid  = mean(abs(H*A-Y)).val[0];
      double ny     = mean(abs(Y)).val[0];
      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " ret " << ret << " " << resid << " " << ny << " " << resid/ny << " | " << resid2x << " " << resid2y << std::endl; 
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " H " << H.rows << " " << H.cols << " " << H.channels() << std::endl; 
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " Y " << Y.rows << " " << Y.cols << " " << Y.channels() << std::endl; 
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " A " << A.rows << " " << A.cols << " " << A.channels() << std::endl; 

//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
      transpose(A,motionMat[i]);
       
      char filename[128];
      ofstream file;
//      cout << H << endl;
      sprintf(filename,"H_%03d.txt",i); file.open(filename); file << H; file.close();
      sprintf(filename,"Y_%03d.txt",i); file.open(filename); file << Y; file.close();
      sprintf(filename,"A_%03d.txt",i); file.open(filename); file << A; file.close();
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " ret " << ret << std::endl; 
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " H " << H.rows << " " << H.cols << " " << H.channels() << std::endl; 
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " Y " << Y.rows << " " << Y.cols << " " << Y.channels() << std::endl; 
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " A " << A.rows << " " << A.cols << " " << A.channels() << std::endl; 


//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
    }
  delete[] desc;

//          std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << ty << " " << tx << std::endl; 
  // keypoints
//  for (unsigned int i=0;i<pts0.size();i++)
//    { const Point2f & pi = pts0[i];
//
////  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << i << std::endl; 
//      H = 0.0;
//      // translation
//      int it = 0;
//      for (int ty=-mt.tMax;ty<=mt.tMax;ty+=mt.tIncr)
//        { 
//      for (int tx=-mt.tMax;tx<=mt.tMax;tx+=mt.tIncr,it++)
//        { 
////          std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << ty << " " << tx << std::endl; 
//          // pixel of the patch
//          int ip=0;
//          if (    pi.y   -mt.winsize>=0&&pi.y   +mt.winsize<pframe0.rows // the whole patch is in the image
//               && pi.x   -mt.winsize>=0&&pi.x   +mt.winsize<pframe0.cols
//               && pi.y+ty-mt.winsize>=0&&pi.y+ty+mt.winsize<pframe0.rows // the whole shifted patch is in the image
//               && pi.x+tx-mt.winsize>=0&&pi.x+tx+mt.winsize<pframe0.cols
//             )
//            { 
//              for (int y=pi.y-mt.winsize; y<=pi.y+mt.winsize; ++y)
//                {
//              for (int x=pi.x-mt.winsize; x<=pi.x+mt.winsize; ++x,ip++)
//                { 
//                  H.at<double>(it, ip) = (int)pframe0.at<uchar>(y, x) - (int)pframe0.at<uchar>(y+ty, x+tx);
////                  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << it << " " << ip << " " << H.at<double>(it, ip) 
////                            << " | " << y << " " << x << " " << pframe0.at<uchar>(y, x) << " " << pframe0.at<uchar>(y+ty, x+tx) << std::endl; 
//                }
//                }
//            }
//          else
//            { 
//              for (int y=pi.y-mt.winsize; y<=pi.y+mt.winsize; ++y)
//                {
//              for (int x=pi.x-mt.winsize; x<=pi.x+mt.winsize; ++x,ip++)
//                { 
////                  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << y << " " << x << " " << pframe0.rows << " " << pframe0.cols << std::endl; 
//                  int  y2 = borderInterpolate(y,    pframe0.rows, BORDER_REPLICATE);
//                  int  x2 = borderInterpolate(x,    pframe0.cols, BORDER_REPLICATE);
//                  int sy2 = borderInterpolate(y+ty, pframe0.rows, BORDER_REPLICATE);
//                  int sx2 = borderInterpolate(x+tx, pframe0.cols, BORDER_REPLICATE);
////                  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << y2 << " " << x2 << " | " << sy2 << " " << sx2 << std::endl; 
//                  
//                  H.at<double>(it, ip) = (int)pframe0.at<uchar>(y2, x2) - (int)pframe0.at<uchar>(sy2, sx2);
////                  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << it << " " << ip << H.at<double>(it, ip) << std::endl; 
//                }
//                }
//            }
//        }
//        }
////  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << i << std::endl; 
////      SVD svd(H);
////  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << i << std::endl; 
////      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " H " << H.rows << " " << H.cols << " " << H.channels() << std::endl; 
////      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " Y " << Y.rows << " " << Y.cols << " " << Y.channels() << std::endl; 
////      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " A " << A.rows << " " << A.cols << " " << A.channels() << std::endl; 
////
////      ofstream file;
////      cout << H << endl;
////      file.open("H.txt"); file << H; file.close();
////      file.open("Y.txt"); file << Y; file.close();
////      bool ret = solve(H, Y1, A, DECOMP_SVD); //DECOMP_QR also ok
////      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " ret " << ret << std::endl; 
//      bool ret = solve(H, Y, A, DECOMP_SVD); //DECOMP_QR also ok
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " ret " << ret << std::endl; 
////      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " H " << H.rows << " " << H.cols << " " << H.channels() << std::endl; 
////      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " Y " << Y.rows << " " << Y.cols << " " << Y.channels() << std::endl; 
////      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " A " << A.rows << " " << A.cols << " " << A.channels() << std::endl; 
//
////      motionMat[i] = 
////      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
////      motionMat[i].create(A.cols,A.rows,CV_64FC1);
////      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
//      transpose(A,motionMat[i]);
////      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
//
////      motionMat[i] = H.inv(DECOMP_SVD);
////      SVD svd(A);
////      Mat pinvA = svd.vt.t()*Mat::diag(1./svd.w)*svd.u.t();
//
//    }
}



void SparseOF::localDesc(const cv::Mat & img, int px, int py, double * desc) const
{ 
// std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << ty << " " << tx << std::endl; 
  // pixel of the patch
  int ip=0;
  if (    py-mt.winsize>=0 && py+mt.winsize<pframe0.rows // the whole patch is in the image
       && px-mt.winsize>=0 && px+mt.winsize<pframe0.cols
     )
    { 
      for (int y=py-mt.winsize; y<=py+mt.winsize; ++y)
      for (int x=px-mt.winsize; x<=px+mt.winsize; ++x,ip++)
        { desc[ip] = (int)img.at<uchar>(y, x);
//                  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << it << " " << ip << " " << H.at<double>(it, ip) 
//                            << " | " << y << " " << x << " " << pframe0.at<uchar>(y, x) << " " << pframe0.at<uchar>(y+ty, x+tx) << std::endl; 
        }
    }
  else
    { 
      for (int y=py-mt.winsize; y<=py+mt.winsize; ++y)
      for (int x=px-mt.winsize; x<=px+mt.winsize; ++x,ip++)
        { 
//                  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << y << " " << x << " " << pframe0.rows << " " << pframe0.cols << std::endl; 
          int  y2 = borderInterpolate(y, img.rows, BORDER_REPLICATE);
          int  x2 = borderInterpolate(x, img.cols, BORDER_REPLICATE);
//                  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << y2 << " " << x2 << " | " << sy2 << " " << sx2 << std::endl; 
          
          desc[ip] = (int)img.at<uchar>(y2, x2);
//                  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << it << " " << ip << H.at<double>(it, ip) << std::endl; 
        }
    }
}



 




//void SparseOF::useLK(bool on)
//{ useLKfit = on;
//}
 


void SparseOF::getPatch(const cv::Mat & img, int x, int y, double theta, cv::Mat & out) const
{
   // compute matrix
   Point center = Point( x, y );
   double angle = theta*M_PI/180.0;
   double scale = 1;
   Mat mat = getRotationMatrix2D( center, angle, scale );

//   std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
//   cout << center << endl;
//   cout << mat << endl;

   // fix offset
   int cpx = out.cols/2, cpy = out.rows/2;
   mat.at<double>(0,2) = x - (mat.at<double>(0,0) * cpx + mat.at<double>(0,1) * cpy); 
   mat.at<double>(1,2) = y - (mat.at<double>(1,0) * cpx + mat.at<double>(1,1) * cpy); 


   // get the patch
   warpAffine( img, out, mat, out.size(), INTER_LINEAR+WARP_INVERSE_MAP, cv::BORDER_REPLICATE);

}
 


void SparseOF::localDesc(const cv::Mat & img, double * desc) const
{ 
//  int i=0;
//  for (int y=0;y<img.rows;y++)
//  for (int x=0;x<img.cols;x++,i++)
//    { desc[i] = img.at<>;
//    }

  // accept only char type matrices
  CV_Assert(img.depth() != sizeof(uchar));

  int i = 0;
  switch(img.channels())
    {
      case 1:
        { MatConstIterator_<uchar> it, end = img.end<uchar>();
//          for( it = img.begin<uchar>(); it != end; ++it, ++i) desc[i] = *it;
          for( it = img.begin<uchar>(); it != end; it+=mt.descstep, ++i) desc[i] = *it;
          break;
        }
      case 3:
        { MatConstIterator_<Vec3b> it, end = img.end<Vec3b>();
//          for( it = img.begin<Vec3b>(); it != end; ++it, i+=3)
          for( it = img.begin<Vec3b>(); it != end; it+=mt.descstep, i+=3)
            { desc[i  ] = (*it)[0];
              desc[i+1] = (*it)[1];
              desc[i+2] = (*it)[2];
            }
        }
    }

}





void SparseOF::setMaxKeypoints(int m)
{
    maxKeyPt = m;
}



//void SparseOF::uniformHarrisPts(const cv::Mat & img, int hx, int hy, double k, int blocksize, VecPts & out)
//{  
//  Mat harrisImg;
//  int ksobelsize = 3;
//  cornerHarris(img, harrisImg, blocksize, ksobelsize, k, BORDER_REPLICATE);
//
//  double maxVal;
//  cv::Point maxLoc;
//  for (int y=0; y < img.rows; y+=hy)
//    {
//      Rect rect(0,y,hx, y+hy<img.rows ? hy : img.rows-1-y);
//      Mat patch(harrisImg,rect);
//      for (int x = 0; x < img.cols; x+=hx)
//        { 
//          minMaxLoc(patch, 0, &maxVal, 0, &maxLoc);
//          maxLoc.x += x;
//          maxLoc.y += y;
//          out.push_back(maxLoc);
////          out[i] = maxLoc;
//          patch.adjustROI(0, 0, -hx, hx); 
//        }
//    }
//}
 




void SparseOF::setGridHarrisSize(int size)
{ harrisGridSize = size;
}
 


void SparseOF::setHarrisParam(double minDist, double qualityLevel, double k)
{ harris.minDist      = minDist;
  harris.qualityLevel = qualityLevel;
  harris.k            = k;

}
 




void SparseOF::setBorder(int bs)
{ borderSize = bs;
}
 




