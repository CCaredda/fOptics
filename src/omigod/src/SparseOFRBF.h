#ifndef __SPARSEOFRBF_
#define __SPARSEOFRBF_


#include <opencv2/video/tracking.hpp>

#include <SparseOF.h>

namespace omigod {

/**
 * @author Michael Sdika
 */
class SparseOFRBF : public SparseOF
{
public :
  
  /* ----------------------------------  */
  /* --       Public Types       ------  */
  /* ----------------------------------  */
  
  /* ----------------------------------  */
  /* --       Public Functions   ------  */
  /* ----------------------------------  */

  /** Constructor */
  SparseOFRBF ();

//  /** Destructor */
//  virtual ~SparseOFRBF ();

  /** initial frame*/
  void setFrame0(const cv::Mat & img);

  /** perform the registration. */
  void computeReg(const cv::Mat & next);

  /** set Dbg level */
  void setDbgLevel(int lev, const char * root=0) {}


//  /** apply the last transformation to an image. */ 
//  void apply(const cv::Mat & img, cv::Mat & out, int interpolation=cv::INTER_LINEAR) const;
//   
//  /** set the parameters of the optical flow computation. */
//  virtual void setParam(int levels_, int winsize_, int iterations_);

protected :
  
  /* ----------------------------------  */
  /* --       Internal Types     ------  */
  /* ----------------------------------  */
//  typedef std::vector<cv::Point2f> VecPts;

  
  /* ----------------------------------  */
  /* --       Member variables   ------  */
  /* ----------------------------------  */
//  /** keypoint detected on frame0. */
//  VecPts pts0;
//
//  /** keypoint in the currect frame. */
//  VecPts pts;
//
//  /** pyramid of the frame0. */
//  cv::Mat pframe0;

  /** inverse of the RBF interpolation matrix. */
  cv::Mat invRBFMat;
  
//  /** nb of level of the pyramid. */
//  int levels;
//
//  /** windows size for the fit. */
//  int winsize;
//  
//  /** nb of iteration. */
//  int maxiter;
//
//  /** components of the last transformation. */
//  cv::Mat transfo1, transfo2;
  
//  /** motion matrix. */
//  std::vector<cv::Mat> motionMat;
//
//  struct MotionTrain
//    { int tMax;
//      int tIncr;
//
//      double thetamax;
//      int    ntheta05;
//
//      int winsize;
//      int descstep;
////      int winIncr;
//    };
//  MotionTrain mt;
//
//  bool useLKfit;

  struct RBF 
    { virtual double operator()(double x1, double x2, double y1, double y2) const=0;
      virtual double getSupp() const=0;
    } * rbfptr;

  struct TPS : public RBF
    { virtual double operator()(double x1, double x2, double y1, double y2) const;
      virtual double getSupp() const;
    } tps;

  struct Wendland : public RBF
    { double r0, r0_sqr;
      virtual double operator()(double x1, double x2, double y1, double y2) const;
      virtual double getSupp() const;
    } wendland;


  /* ----------------------------------  */
  /* --       Member functions   ------  */
  /* ----------------------------------  */

  /** */
  void computeInvRBFMat(const VecPts & p);

};

}

#endif

