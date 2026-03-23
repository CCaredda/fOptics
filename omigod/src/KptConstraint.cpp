#include <KptConstraint.h>

#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>

# define M_PI		3.14159265358979323846	/* pi */


using namespace omigod;
using namespace std;
using namespace cv;







void KptConstraint::locNorm(int imgNum, const VecPts & pvec, const cv::Mat & img)
{
  if (pvec.size()!=normT[imgNum].size()) normT[imgNum].resize(pvec.size());
    

  int ksize = 7; // max = 7
  int ksize05 = ksize / 2;
  bool normalize = true;
  Mat k0, k1, k2;
  getDerivKernels(k0, k1, 0, 1, ksize, normalize, CV_32F );
//  getDerivKernels(k0, k2, 0, 2, ksize, normalize, CV_32F );
  const float * k0p = k0.ptr<float>(0) + ksize05;
  const float * k1p = k1.ptr<float>(0) + ksize05;
//  const float * k2p = k2.ptr<float>(0) + ksize05;
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << k0.size() << std::endl;
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << k1.size() << std::endl;
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << k2.size() << std::endl;
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << k0 << std::endl;
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << k1 << std::endl;
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << k2 << std::endl;
  assert(img.type()==CV_8UC1);
  for (unsigned int i=0;i<pvec.size();i++)
    { const Point2f & pf = pvec[i];
      const Point     pi(round(pvec[i].x),round(pvec[i].y));
      double gx = 0.0, gy = 0.0;
      if ( pi.y-ksize05<0 || pi.x-ksize05<0 || pi.y+ksize05>=img.rows || pi.x+ksize05>=img.cols) 
        { normT[imgNum][i].init(pf.x,pf.y,gx,gy);
          normT[imgNum][i].out = true;
          continue;
        }

      // compute grad
      for (int y=pi.y-ksize05 ; y<=pi.y+ksize05 ; y++)
        { const unsigned char * ip = img.ptr<unsigned char>(y); 
          for (int x=pi.x-ksize05 ; x<=pi.x+ksize05 ; x++)
            { gx += ip[x]*k1p[pi.x-x]*k0p[pi.y-y];
              gy += ip[x]*k0p[pi.x-x]*k1p[pi.y-y];
            }
        }

      normT[imgNum][i].init(pf.x,pf.y,gx,gy);
    }

}
 



void KptConstraint::NormTransfo::init(double _px, double _py, double gx, double gy)
{ out=false;
  r.init(gx, gy);
  px = _px;
  py = _py;
}
 




const cv::Mat & KptConstraint::localTransforms()
{
  assert(normT[0].size()==normT[1].size());
 
  int npts = normT[0].size();
  locTrans.create(npts,3,CV_32FC1);

  static int nf = -1; nf++;
  
  // local norm patch In constant w.r. t
  // At, pt: local aff transfo estimated around keypt
  // In(x) = I0(A0(x-p0)) = It(At(x-pt))
  // I0(y) = It( At*A0^-1*y + At(p0-pt) )
  // or
  // It(z) = I0( A0*At^-1*z + A0(pt-p0) )
  for (int i=0;i<npts;i++)
    { 
      const NormTransfo & nt0 = normT[0][i];
      const NormTransfo & nt1 = normT[1][i];

      double dtheta = nt0.r.theta - nt1.r.theta;
      double dpx = nt1.px - nt0.px,
             dpy = nt1.py - nt0.py;
      double bx, by;
      nt0.r.r(dpx,dpy,bx,by);
      
      float * feat = locTrans.ptr<float>(i);
      feat[0] = bx;
      feat[1] = by;
      feat[2] = dtheta;

////      if (nf==10||nf==58)
//      if (i<1)
//        { cout << "Kpt "  << nf << " " << i << " " << bx << " " << by << " " << dtheta << endl;
//          cout << "Kpt2 " << nf << " " << i << " " << nt0.px << " " << nt0.py << " " <<  nt1.px << " " << nt1.py << endl;
////                     << " | " << glob.ptsData  [i].gy << " " << glob.ptsData  [i].gx
////                     << " | " << pDataNext[i].gy << " " << pDataNext[i].gx << endl;
////          cout << "Kpt2 " << nf << " " << i << " " << glob.pts0[i] << " " << pts[i] 
////                     << " | " << glob.ptsData  [i].gy << " " << glob.ptsData  [i].gx
////                     << " | " << pDataNext[i].gy << " " << pDataNext[i].gx << endl;
//        }
    }

  return (locTrans);
}
 




void KptConstraint::findOutliers(cv::Mat & weight)
{ 
  histogram.clear();
  
  int npts = locTrans.rows;
  weight.create(npts,1,CV_32FC1);
  float * w = weight.ptr<float>(0);
  
  // add pts in the histogram
  for (int i=0;i<npts;i++)
    { const float * lt = locTrans.ptr<float>(i);
      histogram.add(lt[0],lt[1]);
    }

  // find bin with max count
  Histogram::const_iterator it = histogram.begin();
  Bin   bmax = it->first;
  float hmax = it->second; 
  for (;it!=histogram.end();++it)
    { if (it->second>hmax)
        { bmax = it->first;
          hmax = it->second; 
        } 
    }

  // attribute weight for each pts
  // window around center of max bin
  double binfact = 3;
  double c[2] = { bmax.i[0]*histogram.h[0] ,
                  bmax.i[1]*histogram.h[1] };
  for (int i=0;i<npts;i++)
    { const float * lt = locTrans.ptr<float>(i);
      double d[2] = { std::abs(c[0] - lt[0])/histogram.h[0], 
                      std::abs(c[1] - lt[1])/histogram.h[1] };
      double r = sqrt(d[0]*d[0]+d[1]*d[1]);
      w[i] = (r>binfact) ? 0 : 0.5 * ( 1 + cos( M_PI*r/binfact ) ); 
    }

}



void KptConstraint::Histogram::add(double x, double y)
{ Bin b;
  b.i[0] = floor(x / h[0]);
  b.i[1] = floor(y / h[1]);

  double dx = x/h[0] - b.i[0];
  double dy = y/h[1] - b.i[1];

            (*this)[b] += (1-dx)*(1-dy);
  b.i[0]++; (*this)[b] +=    dx *(1-dy);
  b.i[1]++; (*this)[b] +=    dx *   dy;
  b.i[0]--; (*this)[b] += (1-dx)*   dy;
}
 




void KptConstraint::setBinSize(double h)
{ histogram.h[0] = h;
  histogram.h[1] = h;
}
 

