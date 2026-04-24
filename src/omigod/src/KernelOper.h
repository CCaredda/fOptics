#ifndef __MULTIPLEKERNEL_
#define __MULTIPLEKERNEL_

#include <opencv2/ml.hpp>

namespace omigod {

/**
 * @author Michael Sdika
 */
class KernelOper 
{
public :
  
  /* ----------------------------------  */
  /* --       Public Types       ------  */
  /* ----------------------------------  */
  /** available kernel.
   * */
  enum KernelType { UNKNOWN=-1, LIN=0, GAUSSIAN=1, POLY=2};
  static const char * ker2str(KernelType k);
  static KernelType   str2ker(const char * s);
  
  /* ----------------------------------  */
  /* --       Public Functions   ------  */
  /* ----------------------------------  */

  /** Constructor */
  KernelOper ();

  /** Destructor */
  virtual ~KernelOper ();

  /** set kernel and its param. */
  void setKernel(KernelType kt, double sig, int deg);

  /** build the kernel matrix. 
   * data vector are row of the data matrix.
   * */
  void buildKernelMatrix(const cv::Mat & data, cv::Mat & kmat, bool centered=true) const;

  /** build the kernel matrix. 
   * data vector are row of the data matrix.
   * */
  void kernelPCA(const cv::Mat & data, cv::Mat & evec, cv::Mat & eval, int K=-1, bool centered=true) const;

  /** build the kernel matrix. 
   * data vector are row of the data matrix.
   * */
  void kernelPCAProj(const cv::Mat & data, const cv::Mat & evec, const cv::Mat & eval, const cv::Mat & xInput, cv::Mat & xProj) const;

  /** build the distance matrix from the kernel matrix. 
   * inplace is ok ().
   * */
  void kernelDistance(const cv::Mat & kmat, cv::Mat & dist) const;

  /** for each point return the distance to the K-th nearest neighbors. 
   * */
  void kernelKNN(const cv::Mat & dist, int K, cv::Mat & kdist) const;

  /** kernel k-means
   * part is the partition into k clusters
   * */
  void kernelKmeans(const cv::Mat & kmat, int K, cv::Mat & part) const;

protected :
  
  /* ----------------------------------  */
  /* --       Internal Types     ------  */
  /* ----------------------------------  */
  struct Kernel : public cv::ml::SVM::Kernel
    { 
//      int fsize;
      double sigma;
      int deg;
//      KernelType kt;
  
      int  getType () const;

      virtual void init() {};
      
              void calc (int vcount, int n, const float *vecs, const float *another, float *results);
      virtual void calc (int vcount, int n, const float *vecs, const float *another, float *results) const = 0;
    };

  struct LinKernel : public KernelOper::Kernel
    { void calc (int vcount, int n, const float *vecs, const float *another, float *results) const;
    } linKer;
  
  struct RBFKernel : public KernelOper::Kernel
    { void calc (int vcount, int n, const float *vecs, const float *another, float *results) const;
    } rbfKer;

  struct PolyKernel : public KernelOper::Kernel
    { void calc (int vcount, int n, const float *vecs, const float *another, float *results) const;
    } polyKer;

    
  
  /* ----------------------------------  */
  /* --       Member variables   ------  */
  /* ----------------------------------  */
  /** the kernel. */
  Kernel * kernel;
  
  /* ----------------------------------  */
  /* --       Member functions   ------  */
  /* ----------------------------------  */

};

}

#endif

