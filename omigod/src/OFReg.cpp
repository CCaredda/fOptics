#include <OFReg.h>


#include <iostream>
#include <cstdio>

#include <opencv2/video/tracking.hpp>
//#include <opencv2/gpu/gpu.hpp>

using namespace cv;
//using namespace cv::gpu;
using namespace omigod;


#if (CV_VERSION_MAJOR >= 4)

    using namespace cv::optflow;
#endif

OFReg::OFReg ()
{ 
  levels      = 3;
  winsize     = 21;
  maxiter     = 3;

  pyr_scale   = 0.5;
  poly_n      = 5;
  poly_sigma  = 1;
//  offlags     = cv::OPTFLOW_USE_INITIAL_FLOW | OPTFLOW_FARNEBACK_GAUSSIAN;
  offlags     = cv::OPTFLOW_USE_INITIAL_FLOW;

  tvl1OF = TVL1OFPtr();
}
 


void OFReg::computeReg(const cv::Mat & next)
{

    // compute the OF using opencv routine
    if (tvl1OF.empty()) // use gunnar farneback
    {
        //Init optical flow
        flow = Mat::zeros(frame0.size(),CV_32FC2);
        calcOpticalFlowFarneback(frame0, next, flow, pyr_scale, levels, winsize, maxiter, poly_n, poly_sigma, offlags );
    }
  else
    {

//     //! @brief Time step of the numerical scheme
//     setTau(double val) = 0;
//     //! @brief Weight parameter for the data term, attachment parameter
//     setLambda(double val) = 0;
//     //! @brief Weight parameter for (u - v)^2, tightness parameter
//     setTheta(double val) = 0;
//     //! @brief coefficient for additional illumination variation term
//     setGamma(double val) = 0;
//     //! @brief Number of warpings per scale
//     setWarpingsNumber(int val) = 0;
//     //! @brief Stopping criterion threshold used in the numerical scheme, which is a trade-off between precision and running time
//     setEpsilon(double val) = 0;
//     //! @brief Median filter kernel size (1 = no filter) (3 or 5)
//     setMedianFiltering(int val) = 0;
     
//      tvl1OF->setInnerIterations(5);              //! @brief Inner iterations (between outlier filtering) used in the numerical scheme
      tvl1OF->setOuterIterations(maxiter);        //! @brief Outer iterations (number of inner loops) used in the numerical scheme
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << levels << std::endl;
      tvl1OF->setScalesNumber(levels);         //! @brief Number of scales used to create the pyramid of images
      //qDebug()<<"OFRef::computeReg 2.2";
      // c'est pas beau ais tant pis 
      static bool first=true;
      tvl1OF->setUseInitialFlow(!first);            //! @brief Use initial flow
      first = false;
      tvl1OF->setScaleStep(pyr_scale);                  //! @brief Step between scales (<1)
      tvl1OF->calc(frame0, next, flow);
    }

  
//  FarnebackOpticalFlow of;
//  of.numLevels    = levels;
//  of.pyrScale     = 0.5;
//  of.fastPyramids = false;
//  of.winSize      = winsize;
//  of.numIters     = maxiter;
//  of.polyN        = poly_n;
//  of.polySigma    = poly_sigma;
//  of.flags        = cv::OPTFLOW_USE_INITIAL_FLOW;
//
//  GpuMat flowx, flowy, gpuframe0(frame0), gpunext(next);
//  of(gpuframe0, gpunext, flowx, flowy);

//   Mat next2 = next;
//   calcOpticalFlowSF(frame0, next2, flow, levels, winsize, 40);
//
//C++: void calcOpticalFlowSF(Mat& from, Mat& to, Mat& flow, int layers, int averaging_block_size, int max_flow, double sigma_dist, double sigma_color, int postprocess_window, double sigma_dist_fix, double sigma_color_fix, double occ_thr, int upscale_averaging_radius, double upscale_sigma_dist, double upscale_sigma_color, double speed_up_thr)
//  

//    { double minVal, maxVal;
//      minMaxLoc(flow, &minVal, &maxVal);
//    }

//  if (transfo1.rows!=flow.rows||transfo1.cols!=flow.cols)
//    { transfo1.create(flow.rows, flow.cols, CV_32FC1);
//      transfo2.create(flow.rows, flow.cols, CV_32FC1);
//    }

//  double minflowx=1e10, maxflowx=-1e10;
//  double minflowy=1e10, maxflowy=-1e10;
//
  // convert the flow for resampling
  for (int y=0; y < flow.rows; ++y)
    {
      for (int x = 0; x < flow.cols; ++x)
        { Point2f f = flow.at<Point2f>(y, x);
          transfo1.at<float>(y, x) = x + f.x;
          transfo2.at<float>(y, x) = y + f.y;

//          if (f.x<=minflowx) minflowx = f.x;
//          if (f.y<=minflowy) minflowy = f.y;
//          if (f.x>=maxflowx) maxflowx = f.x;
//          if (f.y>=maxflowy) maxflowy = f.y;
            
        }
    }

//  printf("minflow: %f %f \tmaxflow: %f %f\n",minflowx,minflowy,maxflowx,maxflowy);
}
 



void OFReg::setFrame0(const cv::Mat & img)
{ Reg::setFrame0(img);
  frame0=img;
}
 

void OFReg::useTVL1(bool on)
{
//    tvl1OF = on ? createOptFlow_DualTVL1() : TVL1OFPtr();

    tvl1OF = TVL1OFPtr();

}
 

