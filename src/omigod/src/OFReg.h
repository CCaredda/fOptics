#ifndef __OFREG_
#define __OFREG_

//#include <opencv2/core/core.hpp>
#include <opencv2/video/tracking.hpp>
#include <Reg.h>
#include <opencv2/optflow.hpp>



namespace omigod {

/** Optical Flow registration using opencv routine.
 * @author Michael Sdika
 */
class OFReg : public Reg
{
public :
  
  /* ----------------------------------  */
  /* --       Public Types       ------  */
  /* ----------------------------------  */
  
  /* ----------------------------------  */
  /* --       Public Functions   ------  */
  /* ----------------------------------  */

  /** Constructor */
  OFReg ();

  /** initial frame*/
  void setFrame0(const cv::Mat & img);

  /** perform the registration. */
  void computeReg(const cv::Mat & next);

  /** tv l1 optical flow */
  void useTVL1(bool on);

  /** set Dbg level */
  void setDbgLevel(int lev, const char * root=0) {}

protected :
  
  /* ----------------------------------  */
  /* --       Internal Types     ------  */
  /* ----------------------------------  */
  

  
  /* ----------------------------------  */
  /* --       Member variables   ------  */
  /* ----------------------------------  */
  /** initial frame. */
  cv::Mat frame0;

  /** last flow. */
  cv::Mat flow;

  /***/
  double pyr_scale;
  int poly_n;
  double poly_sigma;
  int offlags;

  /** for */

#if (CV_VERSION_MAJOR >= 4)
    typedef cv::Ptr<cv::optflow::DualTVL1OpticalFlow> TVL1OFPtr;

#else

    typedef cv::Ptr<cv::DualTVL1OpticalFlow> TVL1OFPtr;

#endif

  TVL1OFPtr tvl1OF;

  /* ----------------------------------  */
  /* --       Member functions   ------  */
  /* ----------------------------------  */



};

}

#endif

