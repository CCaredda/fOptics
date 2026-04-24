#ifndef __REG_
#define __REG_

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>

namespace omigod {

/** base class for registration algorithms.
 * @author Michael Sdika
 */
class Reg 
{
public :
  
  /* ----------------------------------  */
  /* --       Public Types       ------  */
  /* ----------------------------------  */
  
  /* ----------------------------------  */
  /* --       Public Functions   ------  */
  /* ----------------------------------  */

  /** Constructor */
  Reg ();

  /** Destructor */
  virtual ~Reg ();

  /** iinitial frame. */
  virtual void setFrame0(const cv::Mat & img);
 
  /** perform the registration between frame0 and the next frame. */
  virtual void computeReg(const cv::Mat & next)=0;

  /** apply the last transformation to an image. */ 
  virtual void apply(const cv::Mat & img, cv::Mat & out, int interpolation=cv::INTER_LINEAR) const;
   
  /** set the parameters of the optical flow computation. */
  virtual void setParam(int levels_, int winsize_, int maxiter_);

  /** x component of the last transformation. */
  virtual const cv::Mat & getTransfo1() const;

  /** y component of the last transformation. */
  virtual const cv::Mat & getTransfo2() const;

  /** set Dbg level */
  virtual void setDbgLevel(int lev, const char * root=0) {}


  /**
   * @param
   * @return
   * @see 
   * */
   
protected :
  
  /* ----------------------------------  */
  /* --       Internal Types     ------  */
  /* ----------------------------------  */
  

  
  /* ----------------------------------  */
  /* --       Member variables   ------  */
  /* ----------------------------------  */
  
  /** nb of level of the pyramid. */
  int levels;

  /** windows size for the fit. */
  int winsize;
  
  /** nb of iteration. */
  int maxiter;

  /** components of the last transformation. */
  cv::Mat transfo1, transfo2;

  
  /* ----------------------------------  */
  /* --       Member functions   ------  */
  /* ----------------------------------  */



};

}

#endif

