#include <SparseOFRBF.h>

#include <Timer.h>

#include <iostream>
#include <algorithm>
#include <fstream> 
#include <cstdio>


using namespace std;
using namespace cv;
using namespace omigod;




//static bool lessX(const Point2f & p1, const Point2f & p2)
//{ return (p1.x<p2.x);
//}

static bool lessY(const Point2f & p1, const Point2f & p2)
{ return (p1.y<p2.y);
}


SparseOFRBF::SparseOFRBF ()
  : SparseOF()
{ 
  wendland.r0     = 200;
  wendland.r0_sqr = wendland.r0*wendland.r0;
  rbfptr = &wendland;
}
 



void SparseOFRBF::setFrame0(const cv::Mat & img)
{ 
  SparseOF::setFrame0(img);

//  for (int i=0;i<(int)pts0.size();i++)
//    { std::cout <<"pts0 : "<< i << " " << pts0[i].x << " " << pts0[i].y << std::endl; 
//    }

  sort(pts0.begin(), pts0.end(), lessY);
  
  pts = pts0;
//  for (int i=0;i<(int)pts0.size();i++)
//    { std::cout <<"pts0 : "<< i << " " << pts0[i].x << " " << pts0[i].y << std::endl; 
//    }


  // compute inverse RBF matrix
  computeInvRBFMat(pts0);

}
 




void SparseOFRBF::computeInvRBFMat(const VecPts & p)
{ 
  int n = p.size(); 
  Mat mat(n+3,n+3,CV_64FC1);
  //  Mat mat(n,n,CV_64FC1);

  const RBF & rbf = *rbfptr;

  for (int i=0;i<n;i++)
    { for (int j=0;j<=i;j++)
        { mat.at<double>(i, j) = rbf(p[i].x, p[j].x, p[i].y, p[j].y);
          mat.at<double>(j, i) = mat.at<double>(i, j);
        }
    }

  for (int i=0;i<n;i++)
    { 
      mat.at<double>(i,n  ) = p[i].x;
      mat.at<double>(i,n+1) = p[i].y;
      mat.at<double>(i,n+2) = 1;

      mat.at<double>(n  ,i) = p[i].x;
      mat.at<double>(n+1,i) = p[i].y;
      mat.at<double>(n+2,i) = 1;
      
    }

  for (int i=n;i<n+3;i++)
    { for (int j=n;j<n+3;j++) 
        { mat.at<double>(i,j) = 0.0;
          mat.at<double>(j,i) = 0.0;
        }
    }

//  // precond
//  double * precvec = new double[n+3];
//  for (int i=0;i<n+3;i++)
//    { precvec[i] = 0.0;
//      for (int j=0;j<n+3;j++) 
//        { precvec[i] += std::abs(mat.at<double>(i,j));
//        }
//      precvec[i] /= (double)(n+3);
//    }
//  for (int i=0;i<n+3;i++)
//    { for (int j=0;j<n+3;j++) 
//        { mat.at<double>(i,j) /= (precvec[i]*precvec[j]);
//        }
//    }

//  invRGBMat = mat.inv(DECOMP_CHOLESKY);
  invRBFMat = mat.inv(DECOMP_SVD);
//  for (int i=0;i<n+3;i++)
//    { for (int j=0;j<n+3;j++) 
//        { invRBFMat.at<double>(i,j) *= (precvec[i]*precvec[j]);
//        }
//    }
//    
  invRBFMat.convertTo(invRBFMat,CV_32FC1);

//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
//  for (int i=0;i<n+3;i++)
//    { for (int j=0;j<n+3;j++) 
//        { printf("%e\t",mat.at<double>(i,j));
//        }
//      printf("\n");
//    }
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
//  for (int i=0;i<n+3;i++)
//    { for (int j=0;j<n+3;j++) 
//        { printf("%e\t",invRBFMat.at<float>(i,j));
//        }
//      printf("\n");
//    }
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
 
//  delete[] precvec;

}


void SparseOFRBF::computeReg(const cv::Mat & next)
{ 
//    cout<<"In SparseOFPCA::SparseOFRBF"<<endl;

  int n = pts.size();

  Timer timer;
  timer.doNothing(true);
  timer.tic();
  SparseOF::computeReg(next);
  timer.ticAndPrint("compReg-fitpts ");
 
  // compute rbf coefficients
  Mat cx(n+3,1,CV_32FC1);
  Mat cy(n+3,1,CV_32FC1);
  for (int i=0;i<n+3;i++)
    { 
      cx.at<float>(i,0) = 0.0;
      cy.at<float>(i,0) = 0.0;
      for (int j=0;j<n;j++)
        { cx.at<float>(i,0) += invRBFMat.at<float>(i,j) * pts[j].x;
          cy.at<float>(i,0) += invRBFMat.at<float>(i,j) * pts[j].y;
        }
    }
  
  timer.ticAndPrint("compReg-rbfcoeff ");
          
//  for (int i=0;i<n+3;i++)
////  for (int i=0;i<n;i++)
//    { std::cout <<"cxcy : "<< i << " " << cx.at<float>(i,0) << " " << cy.at<float>(i,0) << std::endl; 
//    }

  const RBF & rbf = *rbfptr;
  int supp = ceil(rbf.getSupp());
  // compute transfo maps
  // affine part
  for (int y=0; y < transfo1.rows; ++y)
    { for (int x = 0; x < transfo1.cols; ++x)
        { transfo1.at<float>(y, x) = cx.at<float>(n,0)*x + cx.at<float>(n+1,0)*y + cx.at<float>(n+2,0);
          transfo2.at<float>(y, x) = cy.at<float>(n,0)*x + cy.at<float>(n+1,0)*y + cy.at<float>(n+2,0);
        }
    }

  // RBF part
  for (int i=0;i<n;i++)
    {
      int yb = pts0[i].y-supp, ye = pts0[i].y+supp;
      int xb = pts0[i].x-supp, xe = pts0[i].x+supp;

      if (yb<0) yb = 0; if (ye>=transfo1.rows) ye = transfo1.rows-1;
      if (xb<0) xb = 0; if (xe>=transfo1.cols) xe = transfo1.cols-1;

      for (int y=yb; y <= ye; ++y)
        { for (int x=xb; x <= xe; ++x)
            { double t = rbf(x,pts0[i].x,y,pts0[i].y);
              transfo1.at<float>(y, x) += cx.at<float>(i,0) * t;
              transfo2.at<float>(y, x) += cy.at<float>(i,0) * t;
            }
        }
    }


//  const RBF & rbf = *rbfptr;
//  // compute transfo maps
//  for (int y=0; y < transfo1.rows; ++y)
//    {
//      // discard point based on Y   
//      int ib=n-1, ie=0;
//      int supp = ceil(rbf.getSupp());
//      while ( ib>0   && (y-supp<pts0[ib].y) ) ib--;
//      while ( ie<n-1 && (y+supp>pts0[ie].y) ) ie++;
////      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << y << " " << ib << " " << ie << std::endl; 
//        
//
//      for (int x = 0; x < transfo1.cols; ++x)
//        { 
//          float tx = cx.at<float>(n,0)*x + cx.at<float>(n+1,0)*y + cx.at<float>(n+2,0),
//                ty = cy.at<float>(n,0)*x + cy.at<float>(n+1,0)*y + cy.at<float>(n+2,0);
////          float tx = 0.0, ty = 0.0;
//
//          for (int i=ib;i<=ie;i++)
//            { double t = rbf(x,pts0[i].x,y,pts0[i].y);
//              tx += cx.at<float>(i,0) * t;
//              ty += cy.at<float>(i,0) * t;
//            }
//            
//          transfo1.at<float>(y, x) = tx;
//          transfo2.at<float>(y, x) = ty;
//        }
//    }


//  // compute transfo maps
//  for (int y=0; y < transfo1.rows; ++y)
//    {
//      for (int x = 0; x < transfo1.cols; ++x)
//        { 
//          float tx = cx.at<float>(n,0)*x + cx.at<float>(n+1,0)*y + cx.at<float>(n+2,0),
//                ty = cy.at<float>(n,0)*x + cy.at<float>(n+1,0)*y + cy.at<float>(n+2,0);
////          float tx = 0.0, ty = 0.0;
//
//          for (int i=0;i<n;i++)
//            { double t = rbf(x,pts0[i].x,y,pts0[i].y);
//              tx += cx.at<float>(i,0) * t;
//              ty += cy.at<float>(i,0) * t;
//            }
//            
//          transfo1.at<float>(y, x) = tx;
//          transfo2.at<float>(y, x) = ty;
//        }
//    }


  timer.ticAndPrint("compReg-comptransfo ");
}




double SparseOFRBF::TPS::operator()(double x1, double x2, double y1, double y2) const
{ double dx = x1-x2;
  double dy = y1-y2;
  double r2 = dx*dx + dy*dy;

  return(r2>0 ? r2 * std::log(r2) : 0.0);
}
 
double SparseOFRBF::Wendland::operator()(double x1, double x2, double y1, double y2) const
{ double dx = x1-x2;
  double dy = y1-y2;
  double r = dx*dx + dy*dy / r0_sqr;

  if (r>1) return (0.0);
  
  r = std::sqrt(r);
  double onemr4 = 1 - r;
  onemr4 = onemr4 * onemr4;
  onemr4 = onemr4 * onemr4;
  return( onemr4 * (4*r+1) );
//  double r = std::sqrt(dx*dx + dy*dy) / r0;
//
//  if (r>1) return (0.0);
//  
//  double onemr4 = 1 - r;
//  onemr4 = onemr4 * onemr4;
//  onemr4 = onemr4 * onemr4;
//  return( onemr4 * (4*r+1) );
}






double SparseOFRBF::TPS::getSupp() const
{ return (1e300);
}

double SparseOFRBF::Wendland::getSupp() const
{ return (r0);
}
 

