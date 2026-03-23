#ifndef __SPARSEOF_
#define __SPARSEOF_


#include <opencv2/video/tracking.hpp>

#include <vector>

#include <Reg.h>

namespace omigod {

/**
 * @author Michael Sdika
 */
class SparseOF : public Reg
{
public :
  
  /* ----------------------------------  */
  /* --       Public Types       ------  */
  /* ----------------------------------  */
  
  /* ----------------------------------  */
  /* --       Public Functions   ------  */
  /* ----------------------------------  */

  /** Constructor */
  SparseOF ();

//  /** Destructor */
//  virtual ~SparseOF ();

  /** initial frame*/
  void setFrame0(const cv::Mat & img);

  /** perform the registration. 
   * obsolete.
   * */
  void computeReg(const cv::Mat & next);

  /** set Dbg level */
  void setDbgLevel(int lev, const char * root=0) {}


  /** perform the registration of the keypoints. 
   * @param affine: 2x3 matrix, affine transform
   * @param affine: 3x3 matrix, perspective transform
   * three methods:
   * - reg(img,         frame0) : Lagrangian           : if prev = affine = 0
   * - reg(img o affine,frame0) : Large def Lagrangian : if affine != 0
   * - reg(img,prev)            : Eulerian             : if prev   != 0
   * */
  void regPts(const cv::Mat & img, const cv::Mat * prev=0, const cv::Mat * affine=0);

  /** apply the last transformation to an image. */ 
  virtual void apply(const cv::Mat & img, cv::Mat & out, int interpolation=cv::INTER_LINEAR) const;

  /** draw points in the apply function. */
  void setDrawPoints(bool _drawpts);
   
  /** set the parameters of the optical flow computation. */
  virtual void setParam(int levels_, int winsize_, int iterations_);

  /** max number of keypoint. */
  void setMaxKeypoints(int m);

  /** best harris in each sub image of a grid. 
   * if <=0 use std harris keypoint
   * */
  void setGridHarrisSize(int size);
//  void useLK(bool on);
   
  /** harris keypt detection param. */
  void setHarrisParam(double minDist, double qualityLevel, double k=0.04);

  /***/
  void setBorder(int bs);

  /** sigma for forward backward. */
  void setFBSigma(double fb) {lkParam.fbSig = fb;}

protected :
  
  /* ----------------------------------  */
  /* --       Internal Types     ------  */
  /* ----------------------------------  */
  typedef std::vector<cv::Point2f> VecPts;
  
  /** Harris keypoint detection parameters. */
  struct HarrisParam
    { double minDist;
      double qualityLevel;
      double k;
    } harris;

  /** LK parameters. */
  struct LKParam
    { std::vector<unsigned char> status;
      std::vector<float> err;
//      Size winSize;
//      int maxLevel;
      cv::TermCriteria criteria;
      int flags;
      double minEigThreshold;

      double fbSig;
      std::vector<float> fbError;
//      VecPts fbError;
    } lkParam;
  
  /* ----------------------------------  */
  /* --       Member variables   ------  */
  /* ----------------------------------  */
  /** keypoint detected on frame0. */
  VecPts pts0;

  /** keypoint in the currect frame. */
  VecPts pts;

  /** pyramid of the frame0. */
  cv::Mat pframe0;

  /** motion matrix. */
  std::vector<cv::Mat> motionMat;

  /** max num of keypoint.*/
  int maxKeyPt;

  /** harrisGridSize. */
  int harrisGridSize;

  /** border size.
   * remove keypoints near image border. */
  int borderSize;

  /** draw key points in apply. */
  bool drawPts;

  struct MotionTrain
    { int tMax;
      int tIncr;

      double thetamax;
      int    ntheta05;

      int winsize;
      int descstep;
//      int winIncr;
    };
  MotionTrain mt;

  bool useLKfit;


  /* ----------------------------------  */
  /* --       Member functions   ------  */
  /* ----------------------------------  */

private:
  /***/
  void trainkeypointmotion();

  /** compute local image. */
  void localDesc(const cv::Mat & img, int x, int y, double * desc) const;

  /** compute local image. */
  void localDesc(const cv::Mat & img, double * desc) const;

  /** get local patch. */
  void getPatch(const cv::Mat & img, int x, int y, double theta, cv::Mat & out) const;

//  /***/
//  void uniformHarrisPts(const cv::Mat & img, int hx, int hy, double k, int blocksize, VecPts & out);


};

}

#endif

