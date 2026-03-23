#ifndef __KPTCONSTRAINT_
#define __KPTCONSTRAINT_

#include <vector>
#include <map>
#include <opencv2/core/core.hpp>

namespace omigod {

/**
 * @author Michael Sdika
 */
class KptConstraint 
{
public :
  
  /* ----------------------------------  */
  /* --       Public Types       ------  */
  /* ----------------------------------  */
  typedef std::vector<cv::Point2f> VecPts;
  
  /* ----------------------------------  */
  /* --       Public Functions   ------  */
  /* ----------------------------------  */

//  /** Constructor */
//  KptConstraint ();
//
//  /** Destructor */
//  virtual ~KptConstraint ();
      
  /** compute and store local transform to normalized patch using image derivative on keypoints.
   * only rot+trans for now
   * @param imgNum is 0 or 0
   * */
  void locNorm(int imgNum, const VecPts & pvec, const cv::Mat & img);

  /** estimate local affine transform for each match.
   * locNorm must have been called for each image
   * */
  const cv::Mat & localTransforms();

  /** find outliers from the local transforms. 
   * @param weight =0 for outliers, ]0,1] for inliers
   * */
  void findOutliers(cv::Mat & w);

  /** */
  void setBinSize(double h);

  /**
   * @param
   * @return
   * @see 
   * */
   
protected :
  
  /* ----------------------------------  */
  /* --       Internal Types     ------  */
  /* ----------------------------------  */
  struct Rot
    { double theta, c, s;
      void init     (double gx, double gy)                               { theta = atan2(gy,gx); c = cos(theta); s = sin(theta); };
      inline void  r(double x, double y, double & rx, double & ry) const { rx = c*x - s*y; ry = s*x + c*y; }
      inline void tr(double x, double y, double & rx, double & ry) const { rx = c*x + s*y; ry = -s*x + c*y; }
    };


  /** Affine transform to map keypoint neigh in a geometrically normalized space.
   *  In(x) = I(A(x-p))
   * */
  struct NormTransfo
    { bool out;
      Rot r;
      double px, py;

      /** compute transfo from pt and image grad. */
      void init(double _px, double _py, double gx, double gy);
    };


  /** histogram bin. */
  struct Bin
    { long i[2];
      inline bool operator<(const Bin & b) const { return ( i[0]<b.i[0] || ( i[0]==b.i[0] && i[1]<b.i[1] ) ); } 
    };

  /** sparse histogram. */
  struct Histogram : public std::map<Bin,float>
    { double h[2];
      
      /** fuzzy addition of a point. */
      void add(double x, double y);
    };

  /* ----------------------------------  */
  /* --       Member variables   ------  */
  /* ----------------------------------  */
  
  /** histogram. */
  Histogram histogram;

  std::vector<NormTransfo> normT[2];

  /**matrix of local transforms. */
  cv::Mat locTrans;
  
  /* ----------------------------------  */
  /* --       Member functions   ------  */
  /* ----------------------------------  */

};

}

#endif

