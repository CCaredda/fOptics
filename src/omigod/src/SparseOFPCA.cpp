#include <SparseOFPCA.h>

#include <Timer.h>
#include <NiftiIO.h>
#include <KernelOper.h>

#include <iostream>
#include <fstream>


#include <limits>
#include <cmath>

using namespace cv;
using namespace omigod;
using namespace std;




//////////////////////////////////////////////////////////////////////////////////////////////
// cholesky decomp
// inplace
// A = L*Lt
template<typename _Tp> 
static bool choleskyDecomp(_Tp* A, size_t astep, int m)
{
  _Tp* L = A;
  int i, j, k;
  double s;
  astep /= sizeof(A[0]);

  for( i = 0; i < m; i++ )
    {
      for( j = 0; j < i; j++ )
        { s = A[i*astep + j];
          for( k = 0; k < j; k++ ) s -= L[i*astep + k]*L[j*astep + k];
          L[i*astep + j] = (_Tp)(s*L[j*astep + j]);
        }
      s = A[i*astep + i];
      for( k = 0; k < j; k++ )
        { double t = L[i*astep + k];
          s -= t*t;
        }
      if( s < std::numeric_limits<_Tp>::epsilon() ) return false;
      L[i*astep + i] = (_Tp)(1./std::sqrt(s));
    }

  return true;

}

// cholesky backsolve
// A = L*Lt
template<typename _Tp> 
static void choleskySolve(_Tp* A, size_t astep, int m, _Tp* b, size_t bstep, int n, bool transpose)
{
  _Tp* L = A;
  int i, j, k;
  double s;
  astep /= sizeof(A[0]);
  bstep /= sizeof(b[0]);
    // LLt x = b
    // 1: L y = b
    // 2. Lt x = y

    /*
     [ L00             ]  y0   b0
     [ L10 L11         ]  y1 = b1
     [ L20 L21 L22     ]  y2   b2
     [ L30 L31 L32 L33 ]  y3   b3

     [ L00 L10 L20 L30 ]  x0   y0
     [     L11 L21 L31 ]  x1 = y1
     [         L22 L32 ]  x2   y2
     [             L33 ]  x3   y3
     */

  if (!transpose)
    { 
      for( i = 0; i < m; i++ )
        {
          for( j = 0; j < n; j++ )
            { s = b[i*bstep + j];
              for( k = 0; k < i; k++ ) s -= L[i*astep + k]*b[k*bstep + j];
              b[i*bstep + j] = (_Tp)(s*L[i*astep + i]);
            }
        }
    }
  else
    {
      for( i = m-1; i >= 0; i-- )
        {
          for( j = 0; j < n; j++ )
            { s = b[i*bstep + j];
              for( k = m-1; k > i; k-- ) s -= L[k*astep + i]*b[k*bstep + j];
              b[i*bstep + j] = (_Tp)(s*L[i*astep + i]);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////

SparseOFPCA::PtsFitType SparseOFPCA::str2pfit(const char * s)
{ if (strcmp(s,"LAG"  )==0) return (LAG  );
  if (strcmp(s,"LLAG" )==0) return (LLAG );
  if (strcmp(s,"EULER")==0) return (EULER);
  assert(false && "Unknown PtsFit");
}
 
const char * SparseOFPCA::pfit2str(PtsFitType t)
{ switch (t)
    { case LAG   : return ("LAG"  ); break;
      case LLAG  : return ("LLAG" ); break;
      case EULER : return ("EULER"); break;
      default:  assert(false && "Unknown PtsFit");   break;
    }
  return ("");
}

//////////////////////////////////////////////////////////////////////////////////////////////
SparseOFPCA::SparseOFPCA ()
  : SparseOF()
{ 
//  testcvtransform = true;

    write_Debug_Info = false;
  npca = 4;

  dbgLevel = 0;
  dbgRoot  = "";

  tcamP = &tcamId;
  tcamId   .edgecut = 0;
  tcamTrans.edgecut = 0;
  tcamAff  .edgecut = 0;
  tcamPersp.edgecut = 0;

  irls.sig0  = 50;
  irls.niter = 7;

  compbycomp = true;

  useKptCons = false;

//  tParams.compose = true;
  tParams.compose = false;
  tParams.icurr = 0;
  tParams.ls.resize(4);
  tParams.ptsFit.resize(2);
  tParams.empty = true;
  
  ptsFitType = LLAG;

  regul[0].init(0,0,0,0,10000);
  regul[1].init(0,0,0,0,10000);

  dySig = 3;
  errSig = 0;
  
}
 
void SparseOFPCA::setSampleSize(int s)
{ vector<Mat>().swap(samples[0]); 
  vector<Mat>().swap(samples[1]); 
  samples[0].resize(s);
  samples[1].resize(s);

  trainImg.resize(s);
}

void SparseOFPCA::setSample(int i, const cv::Mat & tr1, const cv::Mat & tr2)
{ tr1.convertTo(samples[0][i], CV_32FC1);
  tr2.convertTo(samples[1][i], CV_32FC1);
} 
 

void SparseOFPCA::setSample(int i, const cv::Mat & tr1, const cv::Mat & tr2, const cv::Mat & img)
{ tr1.convertTo(samples[0][i], CV_32FC1);
  tr2.convertTo(samples[1][i], CV_32FC1);

  trainImg[i] = img.clone();
} 
 
void SparseOFPCA::TCam::init()
{ projMat.create(3,3,CV_64FC1);
  projMat                 = 0.0;
  projMat.at<double>(0,0) = 1.0;
  projMat.at<double>(1,1) = 1.0;
  projMat.at<double>(2,2) = 1.0;
}

void SparseOFPCA::TCamTrans::init()
{ SparseOFPCA::TCam::init();
  t1 = t2 = sw = 0.0;
}
 
void SparseOFPCA::TCamAff::init()
{ SparseOFPCA::TCam::init();
  MW2M.create(3,3,CV_64FC1);
  MWt1.create(3,1,CV_64FC1);
  MWt2.create(3,1,CV_64FC1);
  MW2M = 0.0;
  MWt1 = 0.0;
  MWt2 = 0.0;
}

void SparseOFPCA::TCam::addSample(double x, double y, double t1p, double t2p, double w, bool fitInverse)
{ if (fitInverse) addSample(t1p, t2p, x,   y,   w); 
  else            addSample(x,   y,   t1p, t2p, w);
}
 
void SparseOFPCA::TCamTrans::addSample(double x, double y, double t1p, double t2p, double w)
{ t1 += w * (t1p-x);
  t2 += w * (t2p-y);
  sw += w;
}
 
void SparseOFPCA::TCamAff::addSample(double x, double y, double t1p, double t2p, double w)
{ 
  double * pM  = MW2M.ptr<double>(0);
  double * pV1 = MWt1.ptr<double>(0);
  double * pV2 = MWt2.ptr<double>(0);
  pM[0] += w * x*x; pM[1] += w * x*y; pM[2] += w * x;
                    pM[4] += w * y*y; pM[5] += w * y;
                                      pM[8] += w    ;

  pV1[0] += w*t1p*x; pV1[1] += w*t1p*y; pV1[2] += w*t1p;
  pV2[0] += w*t2p*x; pV2[1] += w*t2p*y; pV2[2] += w*t2p;
}

void SparseOFPCA::TCamTrans::fit()
{ 
  t1 /= sw;
  t2 /= sw;
  projMat.at<double>(0,2) = t1;
  projMat.at<double>(1,2) = t2;
}


void SparseOFPCA::TCamAff::fit()
{ // symmeterize  
  double * pM  = MW2M.ptr<double>(0);
  pM[3] = pM[1];
  pM[6] = pM[2];
  pM[7] = pM[5];

  // solve MW2M * x = MWt1
  Mat sol;
  bool ret = solve(MW2M, MWt1, sol, DECOMP_SVD); //DECOMP_QR also ok
  double * p    = projMat.ptr<double>(0);
  double * pSol = sol.ptr<double>(0);
  for (int i=0;i<3;i++) p[i] = pSol[i];

  // solve MW2M * x = MWt2
  ret = solve(MW2M, MWt2, sol, DECOMP_SVD); //DECOMP_QR also ok
  pSol = sol.ptr<double>(0);
  for (int i=0;i<3;i++) p[3+i] = pSol[i];

//  cout << MW2M << endl;
//  cout << MWt1<< endl;
//  cout << MWt2<< endl;

}

string _type2str(int type) {
  string r;

  uchar depth = type & CV_MAT_DEPTH_MASK;
  uchar chans = 1 + (type >> CV_CN_SHIFT);

  switch ( depth ) {
    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;
  }

  r += "C";
  r += (chans+'0');

  return r;
}

void SparseOFPCA::fitCamMotion(const cv::Mat & transfo1, const cv::Mat & transfo2, cv::Mat & otransfo1, cv::Mat & otransfo2)
{
  assert(transfo1.channels()==1);
  assert(transfo1.type()==CV_32FC1);
  assert(transfo2.type()==CV_32FC1);
  assert(otransfo1.type()==CV_32FC1);
  assert(otransfo2.type()==CV_32FC1);
  
  TCam & tcam = *tcamP; 
  tcam.init();

  bool fullImgCamFit = false;
  if (fullImgCamFit) // use all the pixel to estimate the affine fit
    { 
      for(int y = 0; y < transfo1.rows; ++y)
        { const float * t1p = transfo1.ptr<float>(y);
          const float * t2p = transfo2.ptr<float>(y);
          const float * e   = edge0   .ptr<float>(y);
          for(int x = 0; x < transfo1.cols; x++)
            { tcam.addSample(x,y,t1p[x],t2p[x], tcam.edgecut<0 ? 1.0 : ( sqrt( e[x]*e[x] + tcam.edgecut*tcam.edgecut ) - tcam.edgecut ) );
            }
        }
  
      tcam.fit();
    }
  else // use only keypoints to estimate the cam motion
    { 
      for(unsigned int i=0 ; i < pts0.size() ; ++i)
        { const Point & p0 = pts0[i];
          float t1p = transfo1.at<float>(p0.y,p0.x);
          float t2p = transfo2.at<float>(p0.y,p0.x);
          tcam.addSample(p0.x,p0.y,t1p,t2p, 1.0 , tParams.compose);
        }
      
      tcam.fit();



//      // IRLS: SEEM TO WORK BUT DOES NOT IMPROVE RESULTS
      static int nf = -1; nf++;
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << nf << std::endl;
//      cout << tcam.projMat << endl;
//      int iter = 0; Mat dump(pts0.size(),2,CV_64FC1);
//      for (double s=40;s>0.5;s*=0.5)
//        { 
//          Mat A = tcam.projMat(Rect(0,0,2,2));
//          Mat b = tcam.projMat(Rect(2,0,1,2));
//          Mat t(2,1,CV_64FC1), mp0;
//          for(unsigned int i=0 ; i < pts0.size() ; ++i)
//            { const Point & p0 = pts0[i];
//              Mat(p0).convertTo(mp0, CV_64FC1);
//              double t1p = t.at<double>(0,0) = transfo1.at<float>(p0.y,p0.x);
//              double t2p = t.at<double>(1,0) = transfo2.at<float>(p0.y,p0.x);
//              Mat r    = A * mp0 + b - t;
//              double nr = norm(r);
////              std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << norm(r) << std::endl;
//              std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << iter << " " << s << " " << i << " " << mp0.t() << " " << (A * mp0 + b).t() << " " << t1p << " " << t2p << "  " << r.t() << std::endl;
//
//              double w = exp (-0.5*nr*nr/(s*s));
////              std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " : " << norm(r) << " " << w << std::endl;
//              tcam.addSample(p0.x,p0.y,t1p,t2p, w);
//
//              dump.ptr<double>(i)[0] = r.at<double>(0,0);
//              dump.ptr<double>(i)[1] = r.at<double>(1,0);
//
//            }
//          cout << tcam.projMat << endl;
////          if (iter!=5) tcam.fit();
////          cout << tcam.projMat << endl;
//          if (nf==83) savemat(dump, "pts83", iter++); 
//        }
//
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl;
      
      if (dbgLevel>0)
        { Mat A = tcam.projMat(Rect(0,0,2,2));
          Mat b = tcam.projMat(Rect(2,0,1,2));
          Mat t(2,1,CV_64FC1), mp0;
          Mat dump(pts0.size(),6,CV_32FC1);
          cout << tcam.projMat << endl;
          for(unsigned int i=0 ; i < pts0.size() ; ++i)
            { const Point & p0 = pts0[i];
              Mat(p0).convertTo(mp0, CV_64FC1);
              float t1p = transfo1.at<float>(p0.y,p0.x);
              float t2p = transfo2.at<float>(p0.y,p0.x);
              Mat pf   = A * mp0 + b;
              float * d = dump.ptr<float>(i);
              d[0] = p0.x;               d[1] = p0.y;
              d[2] = t1p ;               d[3] = t2p;
              d[4] = pf.at<double>(0,0); d[5] = pf.at<double>(1,0);
//              std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << " " << i << " " << mp0.t() << " " << (A * mp0 + b).t() << " " << t1p << " " << t2p << std::endl;
            }
          savemat(dump, (dbgRoot+"pts").c_str(), nf);
        } 
    }


//  cout << tcam.projMat << endl;
          
  const double * p0 = tcam.projMat.ptr<double>(0);
  const double * p1 = tcam.projMat.ptr<double>(1);
  const double * p2 = tcam.projMat.ptr<double>(2);

  if (tParams.compose)
    { 
      for(int y = 0; y < transfo1.rows; ++y)
        { const float *  t1p =  transfo1.ptr<float>(y);
          const float *  t2p =  transfo2.ptr<float>(y);
                float * ot1p = otransfo1.ptr<float>(y);
                float * ot2p = otransfo2.ptr<float>(y);
          for(int x = 0; x < transfo1.cols; x++)
            { double xp = p0[0]*t1p[x]+p0[1]*t2p[x]+p0[2];
              double yp = p1[0]*t1p[x]+p1[1]*t2p[x]+p1[2];
              double zp = p2[0]*t1p[x]+p2[1]*t2p[x]+p2[2];
              ot1p[x] = xp / zp - x;
              ot2p[x] = yp / zp - y;
            }
        }
    }
  else
    {
      for(int y = 0; y < transfo1.rows; ++y)
        { const float *  t1p =  transfo1.ptr<float>(y);
          const float *  t2p =  transfo2.ptr<float>(y);
                float * ot1p = otransfo1.ptr<float>(y);
                float * ot2p = otransfo2.ptr<float>(y);
          for(int x = 0; x < transfo1.cols; x++)
            { double xp = p0[0]*x+p0[1]*y+p0[2];
              double yp = p1[0]*x+p1[1]*y+p1[2];
              double zp = p2[0]*x+p2[1]*y+p2[2];
              ot1p[x] = t1p[x] - xp / zp;
              ot2p[x] = t2p[x] - yp / zp;
            }
        }
    }

}



void SparseOFPCA::train()
{
    // pca
      #if (CV_VERSION_MAJOR >= 4)
          int myDATA_AS_ROW = cv::PCA::DATA_AS_ROW;
      #else
          int myDATA_AS_ROW = CV_PCA_DATA_AS_ROW;
      #endif



  int nsamples = samples[0].size();

//#warning REMOVE IT
//  motionSeg();

  // remove unpredictible camera/patient motion
  for (int i=0;i<nsamples;i++)
    { // pts constraints
      if (dbgLevel>0 && useKptCons)
        { VecPts vpts(pts0.size());
          for(unsigned int j=0 ; j < pts0.size() ; ++j)
            { vpts[j].x = samples[0][i].at<float>(pts0[j].y,pts0[j].x);
              vpts[j].y = samples[1][i].at<float>(pts0[j].y,pts0[j].x);
            }
//          std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << vpts[0] << std::endl;
          kptCons.locNorm(1, vpts, trainImg[i]);
          const Mat & locAff = kptCons.localTransforms();
          Mat wLocAff; kptCons.findOutliers(wLocAff);
          Mat kptsvg; hconcat ( locAff, wLocAff, kptsvg);
          savemat(kptsvg,  (dbgRoot+"kpt").c_str(), i);
        }
      
      // remove affine part
      fitCamMotion(samples[0][i], samples[1][i], samples[0][i], samples[1][i]);

      if (dbgLevel>1)
        { NiftiIO niiIO;
          niiIO.save(samples[0][i],(dbgRoot+"def-1").c_str(),0,i);
          niiIO.save(samples[1][i],(dbgRoot+"def-2").c_str(),0,i);
        }

      samples[0][i].convertTo(samples[0][i], CV_64FC1);
      samples[1][i].convertTo(samples[1][i], CV_64FC1);
    }
//  if (kptConstraint.useit) cout << " kptLocAff = " << localAffine << endl;;
    
//  if (kptConstraint.useit && 0)
//    {
//      KernelOper kop;
//      kop.setKernel(KernelOper::POLY,1,2);
//      Mat evec, eval, kpcaproj;
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << localAffine.size() << std::endl;
//      kop.kernelPCA    (localAffine, evec, eval, 4, true);
//      kop.kernelPCAProj(localAffine, evec, eval, localAffine, kpcaproj);
//      cout << "kpcaproj = " << kpcaproj << endl;
//    }


  // pca 
  int nrow = samples[0][0].rows;
  int ncol = samples[0][0].cols;
  Mat eigval[2];
  vector<Mat>().swap(basis[0]); basis[0].resize(npca);
  vector<Mat>().swap(basis[1]); basis[1].resize(npca);
  if (compbycomp) // perform two PCA: one for each component of the flow
    { 
      PCA pca[2];
      Mat samplesMat(nsamples,samples[0][0].total(),CV_64FC1);

      for (int d=0;d<2;d++)
        { // copy samples
          for (int i=0;i<nsamples;i++) samples[d][i].reshape(1, 1).convertTo(samplesMat.row(i), CV_64FC1, 1, 0); 

          // pca      
          pca[d](samplesMat, Mat(), myDATA_AS_ROW, npca);
          
          // store mean, eigval, eigvec
          pca[d].mean.reshape(1, nrow).convertTo(mean[d],CV_32FC1);
          eigval[d] = pca[d].eigenvalues; 
          for (int k=0;k<npca;k++)
            { basis[d][k] = pca[d].eigenvectors.row(k).clone().reshape(1, nrow);
            }
        }
    }
  else // perform only one PCA using both components of the flow
    { 
      // copy both comp of the flow in the sample mat
      Mat samplesMat(2*nsamples,samples[0][0].total(),CV_64FC1);
      for (int i=0;i<nsamples;i++) samples[0][i].reshape(1, 1).convertTo(samplesMat.row(2*i),   CV_64FC1, 1, 0); 
      for (int i=0;i<nsamples;i++) samples[1][i].reshape(1, 1).convertTo(samplesMat.row(2*i+1), CV_64FC1, 1, 0); 
      samplesMat = samplesMat.reshape(1,nsamples);

      // pca
      PCA pca(samplesMat, Mat(), myDATA_AS_ROW, npca);
      
      // store eigval, mean, eigvec 
      eigval[1] = pca.eigenvalues; 
      Mat pcamean = pca.mean.reshape(1,2);
      Mat ev = pca.eigenvectors.reshape(1,2*npca);

      for (int d=0;d<2;d++)
        { pcamean.row(d).reshape(1, nrow).convertTo(mean[d],CV_32FC1);
          for (int k=0;k<npca;k++)
            { basis[d][k] = ev.row(2*k+d).clone().reshape(1, nrow);
            }
        }
    }

  // scale for conditionning of resolmat and convert to float
  double normfact = sqrt(nrow*ncol);
  for (int k=0;k<npca;k++)
    { basis[0][k].convertTo(basis[0][k],CV_32FC1,normfact);
      basis[1][k].convertTo(basis[1][k],CV_32FC1,normfact);
    }

  if (dbgLevel>0)
    { 
                      savemat(eigval[0],(dbgRoot+"eigval1.txt").c_str());
      if (compbycomp) savemat(eigval[1],(dbgRoot+"eigval2.txt").c_str());
      if (dbgLevel>1)
        { NiftiIO niiIO;
          niiIO.save(mean[0],(dbgRoot+"mean1").c_str());
          niiIO.save(mean[1],(dbgRoot+"mean2").c_str());
      
          for (int k=0;k<npca;k++)
            { niiIO.save(basis[0][k],(dbgRoot+"basis1").c_str(),0,k);
              niiIO.save(basis[1][k],(dbgRoot+"basis2").c_str(),0,k);
            }
        }
    }
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
//  printf("-------------------\n");
}
 



SparseOFPCA::~SparseOFPCA ()
{ 
}
 




////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
/////////////////               LinSys
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void SparseOFPCA::LinSys::init(int npts_, int npca_, bool compbycomp_, bool compose_, bool persp_, int tcCol)
{ npts       = npts_;
  npca       = npca_;
  compbycomp = compbycomp_;
  persp      = persp_;
  compose    = compose_;
  nresol     = compbycomp ? 2 : 1;

//  int tcCol = tcamP->nCol();
  int nbVar = compbycomp ? npca+tcCol : npca+2*tcCol;
  if (persp)
    { 
#ifdef PL2
      nbVar = 9+6*npca;
#else
      nbVar = 9+3*npca;
#endif
      assert(!compbycomp);
    }

  Mat allocRes(2*npts,nbVar,compose ? CV_32FC1 : CV_64FC1 );
  allocRes = 0;
  resolmat[0] = submat(allocRes,0);
  resolmat[1] = submat(allocRes,1);
  
  rprec[0].create(nbVar,1,CV_32FC1);  rprec[0] = 1;
  rprec[1].create(nbVar,1,CV_32FC1);  rprec[1] = 1;

  Mat rhsF;
  rhsF.create(2*npts,1,CV_32FC1);
  rhs[0] = submat(rhsF,0);
  rhs[1] = submat(rhsF,1);
}
 

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
/////////////////               LinSys
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////



void SparseOFPCA::setFrame0(const cv::Mat & img)
{ 

  ///////////////////////////////////////////////////////
  // find keypoints
  int maxKeyPtSvg = maxKeyPt;
  maxKeyPt = numeric_limits<int>::max();
  SparseOF::setFrame0(img);
  maxKeyPt = maxKeyPtSvg;
  int npts = pts0.size();


  ///////////////////////////////////////////////////////
  // compute edge map for affine fit
  edgeMap(pframe0, edge0, 1);
  edge0.convertTo(edge0,CV_32FC1);
//  imwrite("frame0.png",img);
//  imwrite("edge0.png", edge0);
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << edge0.type() << std::endl;

  ///////////////////////////////////////////////////////
  // kpt constraints
  ptsData.resize(npts);
  for (int i=0;i<npts;i++) ptsData[i].outlier = false;
  
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
  // derivatives at keypoints
  if (useKptCons) kptCons.locNorm(0, pts0, img);

//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 

  ///////////////////////////////////////////////////////
  // train
  assert(!samples[0][samples[0].size()-1].empty());

  train();


//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
  // basis images as 2 multi chanel images (for efficiency)
  cv::merge(basis[0], mcBasis[0]);
  cv::merge(basis[1], mcBasis[1]);



//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
//  ///////// test for cv::transform
//  if (testcvtransform)
//    { 
//  Mat meanp[2];
//  meanp[0] = mean[0].clone();
//  meanp[1] = mean[1].clone();
//
//  for (int y=0; y < mean[0].rows; ++y)
//    { float * mup1 = meanp[0].ptr<float>(y);
//      float * mup2 = meanp[1].ptr<float>(y);
//      for (int x = 0; x < mean[0].cols; ++x)
//        { mup1[x] += x;
//          mup2[x] += y;
//        }
//    }
//
////  // create multi channel basis images
////  for (int d=0;d<2;d++)
////    { basis[d].push_back(meanp[d]);
////      cv::merge(basis[d], mcBasis2[d]);
////      basis[d].pop_back();
////    }
//
//  basis[0].push_back(meanp[0]);
//  basis[1].push_back(meanp[1]);
//  
//  std::vector<cv::Mat> b = basis[0];
//  b.insert(b.end(), basis[1].begin(), basis[1].end());
//
//  cv::merge(b, mcBasis3);
//
//  basis[0].pop_back();
//  basis[1].pop_back();
//    }
//  /////////////////////

  

//  // x and y images
//  xImg.create(img.rows,img.cols,CV_32FC1); float * xI = xImg.ptr<float>(0);
//  yImg.create(img.rows,img.cols,CV_32FC1); float * yI = yImg.ptr<float>(0);
//  for (int y=0; y < img.rows; ++y)
//    { for (int x=0; x < img.cols; ++x, ++xI, ++yI) { *xI = x; *yI = y; }
//    }
   

//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << mcBasis[0].size() << std::endl;
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << mcBasis[0].channels() << std::endl;
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << mcBasis[0].type() << std::endl;
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << mcBasis[0].depth() << std::endl;
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 

//  cout<<"[SparseOFPCA] setFrame0 : remat function 1, input size : ("<<mean[0].rows<<";"<<mean[0].cols<<")";
//  cout<<"[SparseOFPCA] setFrame0 : remat function 2, input size : ("<<mean[1].rows<<";"<<mean[1].cols<<")";
//  cout<<"[SparseOFPCA] setFrame0 : shrt max param : "<<SHRT_MAX;

  ///////////////////////////////////////////////////////
  // value of pts0 in means images
//  //TEMP
//  VecPts pts;
//  for(unsigned int i=0;i<pts0.size();i++)
//  {
//      if(i== SHRT_MAX -1)
//          break;
//      pts.push_back(pts0[i]);
//  }
//  //END TEMP


  Mat mu[2],p0(pts0);//, p0(pts);
//  cout<<"[SparseOFPCA] setFrame0 : p0 size : "<<p0.rows<<";"<<p0.cols;

  remap(mean[0], mu[0], p0, cv::noArray(), cv::INTER_LINEAR, cv::BORDER_REPLICATE);
//  cout<<"[SparseOFPCA] setFrame0 : step702";
  remap(mean[1], mu[1], p0, cv::noArray(), cv::INTER_LINEAR, cv::BORDER_REPLICATE);

//  cout<<"[SparseOFPCA] setFrame0 : step71";
  VecPts().swap(meanpts0); meanpts0.resize(npts);

//  cout<<"[SparseOFPCA] setFrame0 : step72";
  assert(mu[0].type()==CV_32FC1);
  for (int i=0;i<npts;i++)
    { meanpts0[i].x = mu[0].at<float>(i, 0);
      meanpts0[i].y = mu[1].at<float>(i, 0);
    }
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 


//  cout<<"[SparseOFPCA] setFrame0 : step8";

  // principal component values on keypoints
  tParams.p[0].create(npts,npca,CV_32FC1);
  tParams.p[1].create(npts,npca,CV_32FC1);
  for (int j=0;j<npca;j++)
    { 
//      int startcol = compbycomp ? tcCol : 2*tcCol;
      for (int d=0;d<2;d++)
        { // d-th comp of the basis vectors for the pts in p0
          Mat p;
          remap(basis[d][j], p, p0, cv::noArray(), cv::INTER_LINEAR, cv::BORDER_REPLICATE);
          p.copyTo(tParams.p[d].col(j));
        }
    }

//  cout<<"[SparseOFPCA] setFrame0 : step9";
  ///////////////////////////////////////////////////////
  // alloc resolution matrices 
  linsys.init(npts, npca, compbycomp, tParams.compose, tcamP==&tcamPersp, tcamP->nCol());

  // build resol matrix for additive model
  if (!tParams.compose)
    { buildAddResolMat();
      ///////////////////////////////////////////////////////
      // compute pseudo inverse
                      linsys.pinvrmat[0] = linsys.resolmat[0].inv(DECOMP_SVD);
      if (compbycomp) linsys.pinvrmat[1] = linsys.resolmat[1].inv(DECOMP_SVD);

      linsys.resolmat[0].convertTo(linsys.resolmat[0],CV_32FC1);
      linsys.resolmat[1].convertTo(linsys.resolmat[1],CV_32FC1);
      linsys.pinvrmat[0].convertTo(linsys.pinvrmat[0],CV_32FC1);
      linsys.pinvrmat[1].convertTo(linsys.pinvrmat[1],CV_32FC1);
//    //  cout << "linsys.resolmat[0]_d = " << linsys.resolmat[0] << endl; 
   }


#if 1
  Mat fmin(linsys.resolmat[0].rows,1,CV_32FC1,Scalar(-1)), fmax(linsys.resolmat[0].rows,1,CV_32FC1,Scalar(1));
  for (int d=0;d<2;d++)
    { regul[d].init(fmin,fmax);
    }
#else
  // fit model on training frame for regularization metric estimation
  // should not be necessary if preconditionner valid
  int nresol = compbycomp ? 2 : 1;

//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
  //////////////////////////////////////////////////////////
  // fit model on training frames
//  if (dbgLevel||tParams.compose)
    { Mat residual[2], residuals[2];
      residuals[0].create(pts0.size(),samples[0].size()-1,CV_32FC1);
      residuals[1].create(pts0.size(),samples[0].size()-1,CV_32FC1);
      Mat weight(pts0.size(),1,CV_32FC1,Scalar(1));
      Mat fittedL[2];
      fittedL[0].create(linsys.resolmat[0].cols,samples[0].size(),CV_32FC1);
      fittedL[1].create(linsys.resolmat[1].cols,samples[0].size(),CV_32FC1);
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;

//      if (!tParams.compose)
        { linsys.pinvrmat[0] = linsys.resolmat[0].inv(DECOMP_SVD);
          if (compbycomp) linsys.pinvrmat[1] = linsys.resolmat[1].inv(DECOMP_SVD);
          linsys.resolmat[0].convertTo(linsys.resolmat[0],CV_32FC1);
          linsys.resolmat[1].convertTo(linsys.resolmat[1],CV_32FC1);
          linsys.pinvrmat[0].convertTo(linsys.pinvrmat[0],CV_32FC1);
          linsys.pinvrmat[1].convertTo(linsys.pinvrmat[1],CV_32FC1);
        }
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;

      for (unsigned int s=0;s<samples[0].size();s++)
        { // pos of keypoint in  
//  tParams.printL();
//          printf("%3d / %d : ",s,samples[0].size());
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
          SparseOF::computeReg(trainImg[s]);
//          std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
//          cout << regul[0].c0_tLL    << endl;
//          cout << regul[0].c0_tLx0   << endl;
//          cout << regul[0].c0_x0     << endl;
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
          if (s==0||!tParams.compose) 
            { 
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
              computeDecompCoeffAdd(weight); 
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
              if (tParams.compose) 
                { tParams.getPtsFit(0) = pts;
                  tParams.init(pts);
                  tParams.incr();
                }
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
            }
          else 
            { 
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
              computeDecompCoeffComp(weight,residual);
              tParams.getPtsFit(0) = pts;
              tParams.incr();
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
            }
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
  
          Mat * l = tParams.get(0);
          for (int d=0;d<nresol;d++)
            { 
              l[d].copyTo(fittedL[d].col(s));
              if (s>0)
                { residual[d].copyTo(residuals[d].col(s-1));
                }
//              l[0].copyTo(fittedL[0].col(s));
//              l[1].copyTo(fittedL[1].col(s));
//              if (s>0)
//                { residual[0].copyTo(residuals[0].col(s-1));
//                  residual[1].copyTo(residuals[1].col(s-1));
//                }
            }
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
        }

//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
//      cout << fittedL[0] << endl;

      Mat favg[2], fmin[2], fmax[2];
      for (int d=0;d<2;d++)
        { Mat fmin, fmax;
          reduce(fittedL[d], fmin, 1, REDUCE_MIN);
          reduce(fittedL[d], fmax, 1, REDUCE_MAX);
          regul[d].init(fmin,fmax);
        }
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
//      reduce(fittedL[0], favg[0], 1, REDUCE_AVG);
//      reduce(fittedL[0], fmin[0], 1, REDUCE_MIN);
//      reduce(fittedL[0], fmax[0], 1, REDUCE_MAX);
//      reduce(fittedL[1], favg[1], 1, REDUCE_AVG);
//      reduce(fittedL[1], fmin[1], 1, REDUCE_MIN);
//      reduce(fittedL[1], fmax[1], 1, REDUCE_MAX);
//      regul.init(fmin[0],fmax[0]);

//          cout << favg[0].t() << endl;
//          cout << favg[1].t() << endl;
//          cout << endl;
//          cout << fmin[0].t() << endl;
//          cout << fmin[1].t() << endl;
//          cout << endl;
//          cout << fmax[0].t() << endl;
//          cout << fmax[1].t() << endl;
//          cout << endl;
//          cout << favg[1].t() << endl;
//          cout << endl;
//          cout << fmin[1].t() << endl;
//          cout << endl;
//          cout << fmax[1].t() << endl;
//          cout << endl;
//          regul.init(fmin[0],fmax[0]);
    }
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
//  compbycomp = compbycompSVG;
//  tcamP      = tcamPSVG;
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
#endif
  
//  ///////////////////////////////////////////////////////
//  // diag precond
//  Mat dprec1, dprec2;
//  reduce(abs(linsys.resolmat[0]), dprec1, 0, CV_REDUCE_AVG);
//  reduce(abs(linsys.resolmat[1]), dprec2, 0, CV_REDUCE_AVG);
//  cout << "dprec1=   " << dprec1    << endl;
//  cout << "dprec2=   " << dprec2    << endl;

//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << pts0.size() <<  std::endl;
  // keypoint selection using reeves sbs algo 
#if 0
  if (pts0.size()>(unsigned int)maxKeyPt)
    { keypointSelection();
      npts = pts0.size();
    }
#endif
  
//  ///////////////////////////////////////////////////////
//  // compute pseudo inverse
//                  linsys.pinvrmat[0] = linsys.resolmat[0].inv(DECOMP_SVD);
//  if (compbycomp) linsys.pinvrmat[1] = linsys.resolmat[1].inv(DECOMP_SVD);
//
////  cout << "linsys.resolmat[0]=" << linsys.resolmat[0] << endl;
////  cout << "linsys.pinvrmat[0]=" << linsys.pinvrmat[0] << endl;
////  
////  cout << "linsys.resolmat[1]=" << linsys.resolmat[1] << endl;
////  cout << "linsys.pinvrmat[1]=" << linsys.pinvrmat[1] << endl;
//
//  linsys.resolmat[0].convertTo(linsys.resolmat[0],CV_32FC1);
//  linsys.resolmat[1].convertTo(linsys.resolmat[1],CV_32FC1);
//  linsys.pinvrmat[0].convertTo(linsys.pinvrmat[0],CV_32FC1);
//  linsys.pinvrmat[1].convertTo(linsys.pinvrmat[1],CV_32FC1);
////  cout << "linsys.resolmat[0]_d = " << linsys.resolmat[0] << endl; 

  ///////////////////////////////////////////////////////
  // pts on last frame
////  SparseOF::computeReg(trainImg[samples[0].size()-1]);
////  tParams.getPtsFit(0) = pts;
////  SparseOF::computeReg(trainImg[samples[0].size()-2]);
////  tParams.getPtsFit(0) = pts;
////  tParams.incr();
//  if (!tParams.compose)
//    { 
//      PtsFitType ptsFitTypeSvg = ptsFitType;
//      ptsFitType = LAG;
//      SparseOF::computeReg(trainImg[samples[0].size()-1]);
//      ptsFitType = ptsFitTypeSvg;
//      if (ptsFitType==EULER) prevFrame = trainImg[samples[0].size()-1].clone();
//      tParams.incr();
//    }
//

  if (ptsFitType==EULER) prevFrame = trainImg[samples[0].size()-1].clone();
}
 

void SparseOFPCA::computeReg(const cv::Mat & next)
{ 

//    if(dbgLevel>0)
//    {
//        std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl;
//        tParams.printL();
//    }

  Timer timer;
  timer.doNothing(true);
  timer.tic();

  ///////////////////////////// estimate pts
//  SparseOF::computeReg(next);
//  static int nf = 0; nf++;
  if (ptsFitType==LAG || (ptsFitType==LLAG && tParams.empty) )
    {
      regPts(next);
    }
//  else if (ptsFitType==LLAG || (ptsFitType==EULER&&nf%2==0))
  else if (ptsFitType==LLAG)
    {
      if (tcamP==&tcamPersp)
        {
//          if(dbgLevel>0)
//              std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl;

          Mat U, lambdaPersp;
          tParams.getHomo(1, U, lambdaPersp, npca, pframe0.cols,pframe0.rows);
          regPts(next, 0, &U);

//          if(dbgLevel>0)
//              std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl;
        }
      else
        {
          float A[4], b[2];
          tParams.getAffineAb(1, A, b, compbycomp);
          Mat affine = (Mat_<float>(2,3) 
              << A[0], A[1], b[0],
                 A[2], A[3], b[1]);
          regPts(next, 0, &affine);
        }
    }
  else // EULER
    {
      regPts(next, &prevFrame);
    }

  timer.ticAndPrint("compReg-fitpts ");
  
  ///////////////////////////////////////////////
//  if(dbgLevel>0)
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl;

  Mat weight;
  initWeight(weight,next);

//  if(dbgLevel>0)
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl;

//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
////  cout << weight.size() << endl;
//  cout << weight.t() << endl;
 

  ////////////////////////////////////////////////
  // compute decomposition coefficients
  if (tParams.compose) computeDecompCoeffComp(weight); else computeDecompCoeffAdd(weight);
//  if(dbgLevel>0)
//  {
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl;
//      std::cout << weight.size() << std::endl;
//  }

  tParams.getPtsFit(0) = pts;
  Mat * l = tParams.get(0);

//  if(dbgLevel>0)
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl;

  // compute transfo maps from model coeff
  computeTransfo(l);

//  if(dbgLevel>0)
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl;


  // increment tParams
  if (ptsFitType==EULER) prevFrame = next.clone();
  tParams.incr();
  timer.ticAndPrint("compReg-comptransfo ");


//  if(dbgLevel>0)
//  {
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl;
//      tParams.printL();
//  }
}
 



void SparseOFPCA::computeTransfo(const cv::Mat * l)
{
  assert(l[0].type()==CV_32FC1);
  assert(!compbycomp||l[1].type()==CV_32FC1);
  assert(mean[0].type()==CV_32FC1);
  assert(mean[1].type()==CV_32FC1);
  assert(npca==0||basis[0][0].type()==CV_32FC1);
  assert(npca==0||basis[1][0].type()==CV_32FC1);
  assert(npca==0||mcBasis[0].depth()==CV_32F);
  assert(npca==0||mcBasis[1].depth()==CV_32F);
//  assert(npca==0||mcBasis2[0].depth()==CV_32F);
//  assert(npca==0||mcBasis2[1].depth()==CV_32F);
//  assert(npca==0||mcBasis3.depth()==CV_32F);
  assert(mcBasis[0].channels()==npca);

//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
  ////////////////////////////////////////////////
  // set the affine part, depending on the camera model and whether compbycomp or not
  double axx=1.0, axy=0.0, ayx=0.0, ayy=1.0, bx=0.0, by=0.0;
  double awx=0.0, awy=0.0, bw=1.0;
  Mat lambdaPersp; 
  bool persp = (tcamP==&tcamPersp);  
  if (tParams.compose)
    {
      Mat U; 
      float A[4],b[2];
      if (persp)
        { tParams.getHomo(0, U, lambdaPersp, npca, pframe0.cols,pframe0.rows);
          const float * u = U.ptr<float>(0); 
          axx=u[0], axy=u[1], bx = u[2],
          ayx=u[3], ayy=u[4], by = u[5],
          awx=u[6], awy=u[7], bw = u[8];
        }
      else
        { tParams.getAffineAb(0, A, b, compbycomp);
          axx=A[0], axy=A[1],
          ayx=A[2], ayy=A[3],
          bx =b[0], by =b[1];
        }
      
 
//      float A[4],b[2];
//      tParams.getAffineAb(0, A, b, compbycomp);
//      axx=A[0], axy=A[1],
//      ayx=A[2], ayy=A[3],
//      bx =b[0], by =b[1];
    }
  else
    {
      if (tcamP==&tcamTrans)
        { bx = l[0].at<float>(0,0);
          by = compbycomp ? l[1].at<float>(0,0) : l[0].at<float>(1,0);
        }
      else if (tcamP==&tcamAff)
        { if (compbycomp)
            { bx=l[0].at<float>(0,0);    axx=l[0].at<float>(1,0);    axy=l[0].at<float>(2,0);
              by=l[1].at<float>(0,0);    ayx=l[1].at<float>(1,0);    ayy=l[1].at<float>(2,0);
            }
          else
            { bx=l[0].at<float>(0,0);    axx=l[0].at<float>(2,0);    axy=l[0].at<float>(3,0);
              by=l[0].at<float>(1,0);    ayx=l[0].at<float>(4,0);    ayy=l[0].at<float>(5,0);
            }
        }
    }

  // compute transfo maps
  int tcCol = tcamP->nCol();
//        float ** b1 = new float*[2*npca],
//              ** b2 = b1 + npca;
  const float * l0pc = l[0].ptr<float>(tcCol,0),
              * l1pc = l[1].ptr<float>(tcCol,0);
  int bstep = mcBasis[0].channels();
  if (!compbycomp) l0pc = l1pc = l[0].ptr<float>(2*tcCol,0);
  if (persp)
    { l0pc = lambdaPersp.ptr<float>(0);
//#warning check compbycomp
      l1pc = lambdaPersp.ptr<float>(1); 
    }

//  //////////////// testcvtransform
//  if (testcvtransform && tParams.compose)
//    { 
//      Mat m0 (1, npca+1, CV_32FC1);
//      Mat m1 (1, npca+1, CV_32FC1);
//      for (int i=0;i<npca;i++) m0.at<float>(i) = l0pc[i]; m0.at<float>(npca) = 1;
//      for (int i=0;i<npca;i++) m1.at<float>(i) = l1pc[i]; m1.at<float>(npca) = 1;
//      
//      Mat td0, td1;
//      cv::transform(mcBasis2[0], td0, m0);
//      cv::transform(mcBasis2[1], td1, m1);
// 
////      transfo1 = bx + axx*td0 + axy*td1;
////      transfo2 = by + ayx*td0 + ayy*td1;
//      addWeighted (td0, axx, td1, axy, bx, transfo1, CV_32FC1);
//      addWeighted (td0, ayx, td1, ayy, by, transfo2, CV_32FC1);
////      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
//
//////      Mat m(2, 2*(npca+1), CV_32FC1); m = 0;
//////      for (int i=0;i<npca;i++) 
//////        { m.at<float>(0,i)        = l0pc[i]; 
//////          m.at<float>(1,i+npca+1) = l1pc[i]; 
//////        }
//////      m.at<float>(0,npca)     = 1;
//////      m.at<float>(1,2*npca+1) = 1;
////      Mat m(3, 2*(npca+1), CV_32FC1); m = 0;
////      for (int i=0;i<npca;i++) 
////        { m.at<float>(0,i)        = l0pc[i]; 
////          m.at<float>(1,i+npca+1) = l1pc[i]; 
////        }
////      m.at<float>(0,npca)     = 1;
////      m.at<float>(1,2*npca+1) = 1;
////      m.at<float>(2,2*npca+1) = 1;
////      
////
////      Mat mAb(2, 3, CV_32FC1);
////      mAb.at<float>(0,0) = axx;         mAb.at<float>(0,1) = axy;       mAb.at<float>(0,2) = bx;
////      mAb.at<float>(1,0) = ayx;         mAb.at<float>(1,1) = ayy;       mAb.at<float>(1,2) = by;
//////      cv::transform(td, transfo, mAb);
////
////      Mat mAbm = mAb*m;
////      
//////      Mat td;
//////      cv::transform(mcBasis3, td, m);
////      Mat transfo;
////      cv::transform(mcBasis3, transfo, mAbm);
////
////      Mat transfos[2] = {transfo1, transfo2};
////      split(transfo,transfos);
//// 
//////      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
//
//    }
//  else
//    {

//#pragma omp parallel for reduction(+:nbw,nlab) schedule(dynamic,1)
#pragma omp parallel for schedule(dynamic,5)
  for (int y=0; y < transfo1.rows; ++y)
    {
            float * t1 = transfo1.ptr<float>(y);
            float * t2 = transfo2.ptr<float>(y);

      const float * mu1 = mean[0].ptr<float>(y);
      const float * mu2 = mean[1].ptr<float>(y);
          
      const float * b1  = mcBasis[0].ptr<float>(y);
      const float * b2  = mcBasis[1].ptr<float>(y);

      if (tParams.compose)
        {
          for (int x = 0; x < transfo1.cols; ++x,b1+=bstep,b2+=bstep)
            { 
              double dx = x + mu1[x];
              double dy = y + mu2[x];

              for (int i=0;i<npca;i++)
                { dx += l0pc[i] * b1[i];
                  dy += l1pc[i] * b2[i];
                }
                
              t1[x] = bx + axx*dx + axy*dy;
              t2[x] = by + ayx*dx + ayy*dy;

              if (persp)
                { double w = bw + awx*dx + awy*dy;
                  t1[x] /= w;
                  t2[x] /= w;
                }
            }
        }
      else
        { 
          double tx0 = bx + axy*y;
          double ty0 = by + ayy*y;

          for (int x = 0; x < transfo1.cols; ++x,b1+=bstep,b2+=bstep)
            { 
              double tx = mu1[x] + tx0 + axx*x;
              double ty = mu2[x] + ty0 + ayx*x;

              for (int i=0;i<npca;i++)
                { tx += l0pc[i] * b1[i];
                  ty += l1pc[i] * b2[i];
                }
                
              t1[x] = tx;
              t2[x] = ty;
            }
        }
    }

//    } // testcvtransform
      
  // dump deformation to disk
  if (dbgLevel>1)
    { static int idx = samples[0].size()-1; idx++;
      Mat def1 = mean[0].clone();
      Mat def2 = mean[1].clone();
      for (int i=0;i<npca;i++)
        { def1 += l0pc[i]*basis[0][i];
          def2 += l1pc[i]*basis[1][i];
        }
      NiftiIO niiIO;
      niiIO.save(def1,(dbgRoot+"def-1").c_str(),0,idx);
      niiIO.save(def2,(dbgRoot+"def-2").c_str(),0,idx);
    }

}





void SparseOFPCA::savemat(const cv::Mat & mat, const char * root, int i, std::ios_base::openmode mode) const
{
    std::vector<char> tab(strlen(root) + 32);
    char *buffer = tab.data();

    // char buffer[strlen(root)+32];
    const char * filename;
    if (i<0)
    {
        filename = root;
    }
    else
    {
        filename = buffer;
        sprintf(buffer,"%s-%05d.txt",root,i);
    }

    ofstream file(filename,mode);
    file << mat << endl;
    file.close();
}
 




void SparseOFPCA::edgeMap(const cv::Mat & img, cv::Mat & edge, double alpha) const
{ 
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
  GaussianBlur( img, edge, Size(3,3), 1, 1, BORDER_REPLICATE );
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 

//  /// Convert it to gray
//  cvtColor( src, src_gray, CV_RGB2GRAY );

  /// Generate grad_x and grad_y
  double scale = 1, delta = 0;
  Mat grad_x, grad_y;
  Sobel( edge, grad_x, CV_32FC1, 1, 0, 3, scale, delta, BORDER_DEFAULT );
  Sobel( edge, grad_y, CV_32FC1, 0, 1, 3, scale, delta, BORDER_DEFAULT );
    
  convertScaleAbs( grad_x, grad_x );
  convertScaleAbs( grad_y, grad_y );

  /// Total Gradient (approximate)
  addWeighted( grad_x, 0.5, grad_y, 0.5, 0, edge );
//  resize  ( InputArray src,    OutputArray dst, Size dsize,          double fx=0,     double fy=0,      int interpolation=INTER_LINEAR );
//  calcHist( const Mat* images, int nimages,     const int* channels, InputArray mask, OutputArray hist, int dims, const int* histSize, const float** ranges, bool uniform=true, bool accumulate=false );

}
 





void SparseOFPCA::setCamMotion(const string & model, double edgecut, bool composition)
{ 
//       if (model=="id")    { tcamP=&tcamId;    }
//  else if (model=="trans") { tcamP=&tcamTrans; }
       if (model=="trans") { tcamP=&tcamTrans; }
  else if (model=="aff")   { tcamP=&tcamAff;   }
  else if (model=="persp") { tcamP=&tcamPersp; }
  else { CV_Error(1, "unknow cam motion");  }
 
  tcamP->edgecut = edgecut;

  tParams.compose = composition;
}
 




void SparseOFPCA::setNPC(int npc)
{ npca = npc;
}
 




void SparseOFPCA::computeDecompCoeffAdd(cv::Mat & weight)
{
  Mat * l = tParams.get(0);

//  int tcCol = tcamP->nCol();
  int npts  = pts0.size();
  
  // number of linear system and number of row in resolmat0
  // depending on compbycomp or not   
  int nrow0  = compbycomp ? npts : 2*npts;

  float * prhs[2] = 
    { linsys.rhs[0].ptr<float>(0),
      linsys.rhs[1].ptr<float>(0)
    };

  // compute rhs
  if (tcamP==&tcamTrans)
    { for (int j=0;j<npts;j++)
        { prhs[0][j]     = (pts[j].x - pts0[j].x - meanpts0[j].x);
          prhs[1][j]     = (pts[j].y - pts0[j].y - meanpts0[j].y);
        }
    }
  else
    { for (int j=0;j<npts;j++)
        { prhs[0][j]     = (pts[j].x - meanpts0[j].x);
          prhs[1][j]     = (pts[j].y - meanpts0[j].y);
        }
    }
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
//  cout << linsys.resolmat[0] << endl;
//  cout << rhs[0] << endl;

  if (weight.empty())
    {
      // L2 fit: l = pinv(linsys.resolmat) * rhs
      for (int d=0;d<linsys.nresol;d++) gemm(linsys.pinvrmat[d], linsys.rhs[d], 1, Mat(), 0, l[d]); 	
//    for (int d=0;d<nresol;d++) gemm_omp(linsys.pinvrmat[d], linsys.rhs[d], 1, Mat(), 0, l[d]); 	
    }
  else
    { assert(weight.type()==CV_32FC1);
    }
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
//  cout << l[0] << endl;

  // robust fit: Gaussian IRLS
    { 
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
//      static int kF = 0; kF++;
      float sig = irls.sig0;

      // allocate weighted matrix/rhs
      Mat res[2], wmatF, wmat[2], wrhsF, wrhs[2];
//      wmatF.create(2*npts,l[0].rows,CV_32FC1);
      wmatF.create(2*npts,linsys.pinvrmat[0].rows,CV_32FC1);
      wrhsF.create(2*npts,1,                      CV_32FC1);
      
       
      // 0: top of the matrix (or full mat)
      // 1: bottom 
      wmat[0] = wmatF(Rect(0,0,   wmatF.cols,nrow0));
      wmat[1] = wmatF(Rect(0,npts,wmatF.cols,npts ));
      wrhs[0] = wrhsF(Rect(0,0,   1,         nrow0));
      wrhs[1] = wrhsF(Rect(0,npts,1,         npts ));

      float *wrhsptr[2], *rhsptr[2];
      for (int d=0;d<2;d++)
        { wrhsptr[d] =        wrhs[d].ptr<float>(0);
           rhsptr[d] =  linsys.rhs[d].ptr<float>(0);
        }

      // iterate
      for (int iter=0;iter<irls.niter;iter++,sig*=0.5)
        {
          float fact = -0.5/(sig*sig);
       
          // build linear system
          if (!weight.empty() && iter==0)
            { 
              // matrix/rhs of weighted pb: 
              // wmat = W * resolmat, 
              // wrhs = W * rhs
//              std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << iter << std::endl;

//              int nin = 0;
              for (int i=0;i<npts;i++) 
//                { float w = ptsData[i].weightKC;
                { float w = weight.at<float>(i,0);
//                  std::cout <<"0dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << iter << " " << w << std::endl;
//                  w = 1;
//                  nin += w>0;
                  for (int d=0;d<2;d++) 
                    { float * wm =     wmat[d].ptr<float>(i);
                      float *  m = linsys.resolmat[d].ptr<float>(i);
                      for (int j=0;j<wmat[d].cols;j++) wm[j] = w * m[j]; 
                      wrhsptr[d][i] = rhsptr[d][i] * w;
                    }
                }
//              std::cout <<"0dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << iter << " " << nin << std::endl;
            }  
          else
            {     
//              for (int d=0;d<nresol;d++)
//                {
//                  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << iter << " " << d << " " << linsys.resolmat[d].size()<<  std::endl;
//                  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << iter << " " << d << " " << l[d].size()<<  std::endl;
//                  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << iter << " " << d << " " << res[d].size()<<  std::endl;
//                }
              // res[d] = resolmat[d]*l[d] - rhs[d];
              for (int d=0;d<linsys.nresol;d++) gemm(linsys.resolmat[d], l[d], 1, linsys.rhs[d], -1, res[d]); 	
       
              float * rptr0 =              res[0].ptr<float>(0);
              float * rptr1 = compbycomp ? res[1].ptr<float>(0) : rptr0+npts;
              assert(res[0].type()==CV_32FC1);

              // matrix/rhs of weighted pb: 
              // wmat = W * linsys.resolmat, 
              // wrhs = W * rhs
              for (int i=0;i<npts;i++) 
                { float w = exp( fact * (rptr0[i]*rptr0[i]+rptr1[i]*rptr1[i]) );
//                  std::cout <<"0dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << iter << " " << w << std::endl;
                  for (int d=0;d<2;d++) 
                    { float * wm =     wmat[d].ptr<float>(i);
                      float *  m = linsys.resolmat[d].ptr<float>(i);
                      for (int j=0;j<wmat[d].cols;j++) wm[j] = w * m[j]; 
                      wrhsptr[d][i] = rhsptr[d][i] * w;
                    }
                }
            }














//          std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << l[0].size() << std::endl;
          // solve weigthed least square
          for (int d=0;d<linsys.nresol;d++)
            { bool ok = solve(wmat[d], wrhs[d], l[d], DECOMP_NORMAL|DECOMP_CHOLESKY); //DECOMP_QR DECOMP_SVD also ok
              if (!ok) std::cout <<"dbg : "<<__FILE__<<" : "<< __LINE__<< " IRLS SOLVE FAILURE " << std::endl;
            }
//          std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << l[0].size() << std::endl;
//          std::cout << l[0] << std::endl;
//          std::cout << l[1] << std::endl;

        }
    }
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
//  cout << l[0] << endl;

  ////////////////////////////////////////////////
  // adjust pts using the model just fitted
  Mat rhsFit[2];
  rhsFit[0] = linsys.resolmat[0]*l[0];
  rhsFit[1] = compbycomp ? linsys.resolmat[1]*l[1] : rhsFit[0](Rect(0,npts,1,npts));
  
  if (tcamP==&tcamTrans)
    { for (int j=0;j<npts;j++)
        { pts[j].x = rhsFit[0].at<float>(j,0) + pts0[j].x + meanpts0[j].x;
          pts[j].y = rhsFit[1].at<float>(j,0) + pts0[j].y + meanpts0[j].y;
        }
    }
  else
    { for (int j=0;j<npts;j++)
        { pts[j].x = rhsFit[0].at<float>(j,0) + meanpts0[j].x;
          pts[j].y = rhsFit[1].at<float>(j,0) + meanpts0[j].y;
        }
    }
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
//  cout << l[0] << endl;

  ////////////////////////////////////////////////
  // keep which point is an outlier
  for (int j=0;j<npts;j++) 
    { ptsData[j].outlier = hypot(rhsFit[0].at<float>(j,0)-linsys.rhs[0].at<float>(j,0),
                                 rhsFit[1].at<float>(j,0)-linsys.rhs[1].at<float>(j,0)) > 5;
    }

  ////////////////////////////////////////////////
  // rescale (conditionning)
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
//  cout << linsys.rprec[0] << endl;
//  cout << linsys.rprec[1] << endl;
//  cout << l[0] << endl;
  for (int d=0;d<linsys.nresol;d++) for (int i=0;i<l[d].rows;i++) l[d].at<float>(i,0) *= linsys.rprec[d].at<float>(i,0);
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
//  cout << l[0].t() << endl;
//  cout << l[1].t() << endl;
    
  ////////////////////////////////////////////////
  // print residual at keypoints
  if (dbgLevel>0)
    { 
      // compute residual of lambda fit
      Mat res1(1,npts,CV_32FC1), res2(1,npts,CV_32FC1);
      if (compbycomp)
        { for (int j=0;j<npts;j++) 
            { res1.at<float>(0,j) = rhsFit[0].at<float>(j,0)-linsys.rhs[0].at<float>(j,0);
              res2.at<float>(0,j) = rhsFit[1].at<float>(j,0)-linsys.rhs[1].at<float>(j,0);
            }
        }
      
      static int first = 0; first++;
      savemat(l[0].t(),  (dbgRoot+"l1.txt").c_str(),  -1, first==1 ? ios::trunc : ios::app);
      savemat(l[1].t(),  (dbgRoot+"l2.txt").c_str(),  -1, first==1 ? ios::trunc : ios::app);
      savemat(res1,(dbgRoot+"res1.txt").c_str(),-1, first==1 ? ios::trunc : ios::app);
      savemat(res2,(dbgRoot+"res2.txt").c_str(),-1, first==1 ? ios::trunc : ios::app);
      savemat(linsys.resolmat[0],  (dbgRoot+"resolmat1").c_str(),  first);
      savemat(linsys.resolmat[1],  (dbgRoot+"resolmat2").c_str(),  first);
    }
  
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
//  cout << l[0] << endl;
//  timer.ticAndPrint("compReg-compcoeff ");
          
//  for (int d=0;d<linsys.nresol;d++) for (int i=0;i<l[d].rows;i++) printf("%f  ",l[d].at<float>(i,0)); printf("\n")
//  printf("a ");
//  for (int i=0;i<l[0].rows;i++) printf("%f  ",l[0].at<float>(i,0)); printf("\n");
}
 

void SparseOFPCA::buildAddResolMat()
{ 
//  assert(compbycomp);

  int npts = pts0.size();
  int tcCol = tcamP->nCol();  

  double fcol = 1.0/(double)(pframe0.cols);
  double frow = 1.0/(double)(pframe0.rows);

  for (int i=0;i<npts;i++)
    { double * r0i = linsys.resolmat[0].ptr<double>(i);
      double * r1i = linsys.resolmat[1].ptr<double>(i);
      if (compbycomp)
        { 
          if (tcamP==&tcamTrans||tcamP==&tcamAff)
            { r0i[0] = 1.0;
              r1i[0] = 1.0;
            }
          if (tcamP==&tcamAff)
            { r0i[1] = pts0[i].x*fcol;      linsys.rprec[0].at<float>(1,0) = 1.0*fcol;
              r1i[1] = pts0[i].x*fcol;      linsys.rprec[1].at<float>(1,0) = 1.0*fcol;
              r0i[2] = pts0[i].y*frow;      linsys.rprec[0].at<float>(2,0) = 1.0*frow;
              r1i[2] = pts0[i].y*frow;      linsys.rprec[1].at<float>(2,0) = 1.0*frow;
            }
        }
      else
        { 
          if (tcamP==&tcamTrans || tcamP==&tcamAff)
            { r0i[0] = 1.0; r0i[1] = 0.0;
              r1i[0] = 0.0; r1i[1] = 1.0; 
//              linsys.resolmat[0].at<double>(i+npts, 0) = 0.0; linsys.resolmat[0].at<double>(i+npts, 1) = 1.0; 
            }
          if (tcamP==&tcamAff)
            { r0i[2] = pts0[i].x*fcol;  linsys.rprec[0].at<float>(2,0) = 1.0*fcol;
              r0i[3] = pts0[i].y*frow;  linsys.rprec[0].at<float>(3,0) = 1.0*frow;
              r0i[4] = 0.0;
              r0i[5] = 0.0;
                                                                                                                        
              r1i[2] = 0.0;
              r1i[3] = 0.0;
              r1i[4] = pts0[i].x*fcol;  linsys.rprec[0].at<float>(4,0) = 1.0*fcol; // c'est bien 0!!
              r1i[5] = pts0[i].y*frow;  linsys.rprec[0].at<float>(5,0) = 1.0*frow;
//              linsys.resolmat[0].at<double>(i+npts, 2) = 0.0;
//              linsys.resolmat[0].at<double>(i+npts, 3) = 0.0;
//              linsys.resolmat[0].at<double>(i+npts, 4) = pts0[i].x*fcol;  linsys.rprec[0].at<float>(4,0) =1.0/ (double)(pframe0.cols);
//              linsys.resolmat[0].at<double>(i+npts, 5) = pts0[i].y*frow;  linsys.rprec[0].at<float>(5,0) =1.0/ (double)(pframe0.rows);
            }
        }
    }
  
  // fill mat (pca part)
  int startcol = compbycomp ? tcCol : 2*tcCol;
  for (int j=0;j<npca;j++)
    { for (int d=0;d<2;d++)
        { for (int i=0;i<npts;i++) linsys.resolmat[d].at<double>(i, j+startcol) = tParams.p[d].at<float>(i, j); 
        }
    }
}
 

void SparseOFPCA::buildCompLinSys()
{
  assert(tcamP==&tcamAff);

  int npts  = pts0.size();
  
  // number of linear system and number of row in resolmat0
  // depending on compbycomp or not   
  int colY=0, colPCA=3;
  if (!compbycomp)
    { colY   = 3;
      colPCA = 6;
    }
  
  float * prhs[2] = 
    { linsys.rhs[0].ptr<float>(0),
      linsys.rhs[1].ptr<float>(0)
    };

  // CAUTION: RPREC, MUST NOT BE ADAPTIVE, MUST BE FILLED WITH STATIC DATA 
  linsys.rprec[0] = 1;
  float * rprec0 = linsys.rprec[0].ptr<float>(0);
  double fcol = 1.0/(double)(pframe0.cols);
  double frow = 1.0/(double)(pframe0.rows);
//  rprec0[0] = 1/sqrt(double(npts));
  rprec0[0] = 1;
  rprec0[1] = fcol;
  rprec0[2] = frow;      
  if (!compbycomp)
    { rprec0[3] = rprec0[0];      // 0 1 2 and 3 4 5 must be equal, see filling below
      rprec0[4] = rprec0[1];
      rprec0[5] = rprec0[2];      
    }
 
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
//  cout << linsys.rprec[0].t() << endl;
  linsys.rprec[1] = linsys.rprec[0];
  
  // build linear system
//  assert(weight.empty() || weight.type()==CV_32FC1);
  for (int i=0;i<npts;i++)
    { 
      prhs[0][i] = pts0[i].x + meanpts0[i].x;
      prhs[1][i] = pts0[i].y + meanpts0[i].y;
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << i << " "   << rhs[0].at<float>(i,0) << " " << pts[i].x << " " << tm0[i] // << std::endl; 
//                                                                                      << " | " << rhs[1].at<float>(i,0) << " " << pts[i].y << " " << tm1[i] << std::endl; 

      float * r0i = linsys.resolmat[0].ptr<float>(i);
      float * r1i = linsys.resolmat[1].ptr<float>(i);
      float * p0  = tParams.p[0].ptr<float>(i);
      float * p1  = tParams.p[1].ptr<float>(i);
      r0i[0] = r1i[colY+0] = rprec0[0];
      r0i[1] = r1i[colY+1] = pts[i].x * rprec0[1] - 0.5; // -0.5 to center pts
      r0i[2] = r1i[colY+2] = pts[i].y * rprec0[2] - 0.5;

      for (int k=0;k<npca;k++)
        { r0i[k+colPCA] = - p0[k];
          r1i[k+colPCA] = - p1[k];
        }
    }
}
 


void SparseOFPCA::buildHomoCompLinSys()
{
  assert(tcamP==&tcamPersp);
  assert(!compbycomp);

//  int tcCol = tcamP->nCol();
  int npts  = pts0.size();
  
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << npts << std::endl;
  // fill column preconditionner
  float * rprec0 = linsys.rprec[0].ptr<float>(0);
  double fcol = 1.0/(double)(pframe0.cols);
  double frow = 1.0/(double)(pframe0.rows);
////  double f05  = 0.5/(double)(pframe0.cols+pframe0.rows);
//////  double fconst = 1 / sqrt(2);
////  double fconst = 1/sqrt(npts);
//  double fconst = 0.5;
  
//  float shiftx = pframe0.cols/2;
//  float shifty = pframe0.rows/2;

  linsys.rprec[0] = 1;
//  rprec0[0] = rprec0[3] = fcol;      // r0 and r4 must be equal, see filling below
//  rprec0[1] = rprec0[4] = frow;
//  rprec0[2] = rprec0[5] = fconst;      
//  rprec0[6] = f05*fcol; 
//  rprec0[7] = f05*frow;
//  rprec0[8] = f05;
//
//#ifdef PL2
//  for (int k=0;k<npca;k++)
//    { int off = 6*k+9;
//      rprec0[off+0] = fcol;
//      rprec0[off+1] = frow;
//      rprec0[off+2] = fconst;
//      rprec0[off+3] = fcol;
//      rprec0[off+4] = frow;
//      rprec0[off+5] = fconst;
//    }
//#else
//  for (int k=0;k<npca;k++)
//    { int off = 3*k+9;
//      rprec0[off+0] = fcol;
//      rprec0[off+1] = frow;
//      rprec0[off+2] = fconst;
//    }
//#endif

  // build linear system
  for (int i=0;i<npts;i++)
    { 
      const float w = 1;

      float px = pts[i].x * fcol - 0.5 ,
            py = pts[i].y * frow - 0.5 ;
//      float tmupX = ( pts0[i].x + meanpts0[i].x ) * fcol - 0.5,
//            tmupY = ( pts0[i].y + meanpts0[i].y ) * frow - 0.5;
      float tmupX = ( pts0[i].x + meanpts0[i].x ),// * fcol,
            tmupY = ( pts0[i].y + meanpts0[i].y );// * frow;

//      float px = (pts[i].x - shiftx),
//            py = (pts[i].y - shifty);
//      float tmupX = ( pts0[i].x + meanpts0[i].x ),
//            tmupY = ( pts0[i].y + meanpts0[i].y );

      float * r0i = linsys.resolmat[0].ptr<float>(i);
      float * r1i = linsys.resolmat[1].ptr<float>(i);
      float * p0  = tParams.p[0].ptr<float>(i);
      float * p1  = tParams.p[1].ptr<float>(i);
        { 
          r0i[0] = r1i[3] = w * (px * rprec0[0]); // -0.5 to center pts
          r0i[1] = r1i[4] = w * (py * rprec0[1]);
          r0i[2] = r1i[5] = w       * rprec0[2];

          r0i[6]          = - w * tmupX * px * rprec0[6]; // -0.5 to center pts
                   r1i[6] = - w * tmupY * px * rprec0[6]; // -0.5 to center pts
          r0i[7]          = - w * tmupX * py * rprec0[7];
                   r1i[7] = - w * tmupY * py * rprec0[7];
          r0i[8]          = - w * tmupX      * rprec0[8];
                   r1i[8] = - w * tmupY      * rprec0[8];

#ifdef PL2
          for (int k=0;k<npca;k++)
            { int off = 6*k+9;

              r0i[off+0] = - w * ( p0[k] * px * rprec0[off+0]);
              r0i[off+1] = - w * ( p0[k] * py * rprec0[off+1]);
              r0i[off+2] = - w * ( p0[k]      * rprec0[off+2]);
                                                                  
              r1i[off+3] = - w * ( p1[k] * px * rprec0[off+3]);
              r1i[off+4] = - w * ( p1[k] * py * rprec0[off+4]);
              r1i[off+5] = - w * ( p1[k]      * rprec0[off+5]);
            }
#else
          for (int k=0;k<npca;k++)
            { int off = 3*k+9;

              r0i[off+0] = - w * ( p0[k] * px * rprec0[off+0]);
              r0i[off+1] = - w * ( p0[k] * py * rprec0[off+1]);
              r0i[off+2] = - w * ( p0[k]      * rprec0[off+2]);
                                                                  
              r1i[off+0] = - w * ( p1[k] * px * rprec0[off+0]);
              r1i[off+1] = - w * ( p1[k] * py * rprec0[off+1]);
              r1i[off+2] = - w * ( p1[k]      * rprec0[off+2]);
            }
#endif
        }

    }
}
 



void SparseOFPCA::solveGeneralizedEigen(const cv::Mat & tMM, cv::Mat & sol)
{ 
  static int count=0; count++;
  Mat eval, evec;
  /*bool resEigen =*/ cv::eigen (tMM, eval, evec);
//std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
//  cout << tMM << endl;
  evec.row(tMM.rows-1).reshape(1,tMM.rows).convertTo(sol, CV_32FC1); // smallest ev
//  evec.row(tMM.rows-1).reshape(1,tMM.rows); // smallest ev

//  // partition matrix
//  // tMM = [A B; B' C]
//  int a = 9, b = tMM.rows-a;
//  assert(a+b<=tMM.cols); 
////  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << tMM.cols << " " << a << " " << b << std::endl; 
//  Mat A(tMM,Rect(0,0,a,a));
////  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << tMM.cols << " " << a << " " << b << std::endl; 
//  Mat B(tMM,Rect(a,0,b,a));
////  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << tMM.cols << " " << a << " " << b << std::endl; 
//  Mat C(tMM,Rect(a,a,b,b));
////  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << tMM.cols << " " << a << " " << b << std::endl; 
////  /*bool resEigen =*/ cv::eigen (A, eval, evec);
////std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << A.rows << " " << A.cols <<std::endl; 
////  cout << eval << endl;
////  /*bool resEigen =*/ cv::eigen (C, eval, evec);
////std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << C.rows << " " << C.cols << std::endl; 
////  cout << eval << endl;
//  Mat CinvBt = C.inv(DECOMP_CHOLESKY) * B.t();
//  Mat schur = A - B * CinvBt;
//  /*bool resEigen =*/ cv::eigen (schur, eval, evec);
////std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
////  cout << eval << endl;
////  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << tMM.cols << " " << a << " " << b << std::endl; 
////  evec.row(tMM.rows-1).reshape(1,tMM.rows).convertTo(sol, CV_32FC1); // smallest ev
//  Mat solX = evec.row(schur.rows-1).reshape(1,schur.rows); // smallest ev
////  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << tMM.cols << " " << a << " " << b << std::endl; 
//  Mat solY = - CinvBt*solX; 
////  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << tMM.cols << " " << a << " " << b << std::endl; 
//  vconcat(solX,solY,sol);
////  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << tMM.cols << " " << a << " " << b << std::endl; 
//  sol.convertTo(sol, CV_32FC1); // smallest ev
////  cout << solX.t() << endl;
////  cout << solY.t() << endl;
////  cout << sol.t() << endl;
}
 



void SparseOFPCA::solveCompLinSys(const cv::Mat & weight)
{
  int npts = pts.size();
  Mat * l = tParams.get(0);
  
  assert(weight     .type()==CV_32FC1||weight.empty()); 
  assert(linsys.resolmat[0].type()==CV_32FC1); 
  assert(linsys.rhs[0].empty()|| (linsys.rhs[0].type()==CV_32FC1 && linsys.rhs[0].cols==1 && ( (compbycomp&&linsys.rhs[0].rows==npts) || (!compbycomp&&linsys.rhs[0].rows==2*npts)) ) ); 

  const float * w = weight.ptr<float>(0);
      
  // if persp: then rhs is not used!!
  bool persp = (tcamP==&tcamPersp); // perspective camera

  for (int d=0;d<linsys.nresol;d++)
    { 
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
      int nvar = linsys.resolmat[d].cols;

      // compute tM*M and tM*rhs
      Mat tMM  (nvar,nvar,CV_64FC1,Scalar(0)), 
          tMrhs(nvar,1,   CV_64FC1,Scalar(0));
      const float * m   = linsys.resolmat[d].ptr<float>(0);
      const float * r   = linsys.rhs[d]     .ptr<float>(0);
            double * tmm = tMM  .ptr<double>(0);
            double * tmr = tMrhs.ptr<double>(0);
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << d << std::endl; 
//      for (int k=0;k<npts;k++)
      for (int k=0;k<linsys.resolmat[d].rows;k++)
        { const float * mk = m+k*nvar;
          float wk  = w==0 ? 1 : (k<npts? w[k] : w[k-npts]), 
                wk2 = wk*wk;

          // unconstrained linear least square
          if (persp)
            { for (int i=0;i<nvar;i++)
                { for (int j=0;j<=i;j++) tmm[i+j*nvar] += wk2 * mk[i] * mk[j]; 
                }
            }
          else
            { for (int i=0;i<nvar;i++)
                { for (int j=0;j<=i;j++) tmm[i+j*nvar] += wk2 * mk[i] * mk[j]; 
                                         tmr[i]        += wk2 * mk[i] * r[k];
                }
            }
        }
      // symmetric completion
//      for (int i=0;i<nvar;i++) for (int j=i+1;j<nvar;j++) tmm[i+j*nvar] += tmm[j+i*nvar];
      for (int i=0;i<nvar;i++) for (int j=i+1;j<nvar;j++) tmm[i+j*nvar] = tmm[j+i*nvar];

      if (persp) // perspective camera
        { 
//          if (!weight.empty())
//            { 
//              Mat w2;
//              vconcat(weight,weight,w2); 
//              Mat W=Mat::diag(w2); 
//              Mat wr = W*linsys.resolmat[d];
//              SVD svd(wr);
//              assert(svd.w.type()==CV_32FC1);
////              cout << "cond: " << svd.w.at<float>(0) / svd.w.at<float>(nvar-1) << "  " << svd.w.at<float>(nvar-1) / svd.w.at<float>(nvar-2)  << endl;
////              svd.vt.row(nvar-1).reshape(1,nvar).convertTo(l[0], CV_64FC1); // smallest ev
//              svd.vt.row(nvar-1).reshape(1,nvar).convertTo(l[0], CV_32FC1); // smallest ev
//            }
//          else
            {
              solveGeneralizedEigen(tMM,l[d]);
//              // scale normalize for v3^2 = 1;
//              std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << l[d].t() << std::endl;
//              std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << l[d](Range(6,9)).t() << std::endl;
//              double n = norm(l[d](Range(6,9)));
//              l[d] *= 1.0/n;
            }
        }
      else // affine
        {
          tMM.  convertTo(tMM,   CV_32FC1);
          tMrhs.convertTo(tMrhs, CV_32FC1);
          
          // get solution at previous frame for regularization
          Mat lprec = tParams.get(1)[d].clone();
          if (!lprec.empty())
            { float * lp = lprec.ptr<float>(0);
              for (int i=0;i<lprec.rows;i++) lp[i] /= linsys.rprec[d].at<float>(i,0);
                               lp[0] = lp[0]/linsys.rprec[d].at<float>(0,0) + 0.5*(lp[1]+lp[2]);
              if (!compbycomp) lp[3] = lp[3]*linsys.rprec[d].at<float>(3,0) + 0.5*(lp[4]+lp[5]);
//              lprec.at<float>(0,0) = lprec.at<float>(0,0)/linsys.rprec[d].at<float>(0,0) + 0.5*(lprec.at<float>(1,0)+lprec.at<float>(2,0));
            }

    //      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << ( lprec.ptr<float>(0)==tParams.ls[0].ptr<float>(0) ) << std::endl;
    ////      tParams.printL();
    ////      std::cout << tParams.ptsFit[0] << endl;
    ////      std::cout << weight << endl;
    //      std::cout << lprec.t() << endl;
    //      if (d==0) { cout << lprec.t() << endl; }

          // regularized linear system
          Mat tMMreg=tMM.clone(), tMrhsreg=tMrhs.clone();
//          regul[d].regulSystem(tMM,tMrhs,lprec.ptr<float>(0),tMMreg,tMrhsreg);
          
          // solve unconstrained
          if (!regul[d].useConst())
    //      if (1)
            {    
              bool ok = solve(tMMreg, tMrhsreg, l[d], DECOMP_CHOLESKY); //DECOMP_QR DECOMP_SVD also ok
              if (!ok) std::cout <<"dbg : "<<__FILE__<<" : "<< __LINE__<< " WEIGHTED FIT SOLVE FAILURE " << std::endl;
            }
          else // solve constrained
            {
              // center for constrained pb
              Mat tMMregx0 = regul[d].tMMx0(tMMreg);

              float normLp2 = 2*regul[d].c0_delta*regul[d].c0_delta + 1;
              float mu = 0;
    // TODO test normLp2>1.1*regul[d].c0_delta*regul[d].c0_delta || si trop petit....
              int constIter = 0;
              while (normLp2>1.1*regul[d].c0_delta*regul[d].c0_delta && constIter++ < 10 )
                { 
    //  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
                  normLp2 = regul[d].newtonIter4c0(tMMreg, tMrhsreg, tMMregx0, l[d], mu);
    //  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << normLp2 << std::endl;

    //              std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << normLp2 << " " << mu << std::endl;
    //              if (normLp2>regul[d].c0_delta*regul[d].c0_delta)
    //                { 
    //                  std::cout <<"newton dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << normLp2 << " " << mu << std::endl;
    //                  for (int i=0;i<100;i++)
    //                    { float nu = i*0.01;
    //                      normLp2 = regul[d].newtonIter4c0(tMMreg, tMrhsreg, tMMregx0, l[d], nu);
    //                      cout << "newton " << i*0.01<< " " << normLp2 << " " << nu << endl;
    //                    }
    //                  exit(1);
    //                }
                }

            }
    //      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl;
    //      if (d==0) { cout << "           " << l[d].t() << endl; }
          //////////////////////
        } // affine
      
    }
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;

}
 

void SparseOFPCA::computeDecompCoeffComp(cv::Mat & weight, cv::Mat * residual)
{
  int npts  = pts0.size();
  // number of linear system and number of row in resolmat0
  // depending on compbycomp or not   
//  int nresol = compbycomp ? 2 : 1;
 
  // perspective camera
  bool persp = (tcamP==&tcamPersp);

  // fill linear system
  if (persp) buildHomoCompLinSys(); else buildCompLinSys();

  // solve weighted linear system
  Mat * l = tParams.get(0);
  Mat rhsFit[2];

  // residual;
  Mat res[2];
     
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
  for (int iter=0;iter<irls.niter;iter++)
    { 
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << iter << std::endl; 
      solveCompLinSys(weight);
      
      if (persp)
        { // residual

//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
//          std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
//          cout << linsys.resolmat[0].size() << endl;
//          cout << l[0].size() << endl;
          res[0] = linsys.resolmat[0]*l[0];
          res[1] = linsys.submat(res[0],1);
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << l[0].rows<< std::endl; 
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << l[0].cols<< std::endl; 
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << norm(l[0])  << std::endl; 
////  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << res[0]  << std::endl; 
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << norm(res[0]) / res[0].rows  << std::endl; 
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << linsys.rprec[0].t()  << std::endl; 
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << l[0]    .t()  << std::endl; 
          
          for (int i=0;i<l[0].rows;i++) l[0].at<float>(i,0) *= linsys.rprec[0].at<float>(i,0);
          
          /*
          // rescale (conditionning)
          float shiftx = pframe0.cols/2,
                shifty = pframe0.rows/2;
          double fcol = 1.0/(double)(pframe0.cols),
                 frow = 1.0/(double)(pframe0.rows);
//          std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << l[0].rows << "  " << l[0].cols << std::endl;
//          std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << l[0].rowRange(0,8).rows << "  " << l[0].rowRange(0,8).cols << std::endl;
          assert(l[0].type()==CV_32FC1);
          Mat precMatTmu = (Mat_<float>(3,3) 
              << fcol, 0,    0,
                 0,    frow, 0,
                 0,    0,    1);
          Mat precMat = (Mat_<float>(3,3) 
              << fcol, 0,    -0.5,
                 0,    frow, -0.5,
                 0,    0,    1);
          Mat v = l[0].rowRange(0,9).reshape(1,3);
//          std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << v.rows << "  " << v.cols << std::endl;
//          cout << v << endl;
          Mat vp = precMatTmu.inv() * v * precMat;
//          Mat vp = v * precMat;
          vp.copyTo(v);
//          cout << v << endl;
//          Mat v = l[0].rowRange(0,9).reshape(1,3);
//          std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << v.rows << "  " << v.cols << std::endl;
//          cout << v << endl;
//          Mat vp = precMat.inv() * v * precMat;
//          Mat vp = v * precMat;
//          vp.copyTo(v);
//          cout << v << endl;

          */

#if 0
          assert(l[0].type()==CV_32FC1);
          Mat precMat = (Mat_<float>(3,3) 
              << 1, 0, -shiftx,
                 0, 1, -shifty,
                 0, 0,    1);
          Mat v = l[0].rowRange(0,9).reshape(1,3);
//          std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << v.rows << "  " << v.cols << std::endl;
//          cout << v << endl;
          Mat vp = v * precMat;
          vp.copyTo(v);
//          cout << v << endl;
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << l[0].t()  << std::endl; 
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
#endif
        }
      else // orthographic camera
        {
          for (int d=0;d<linsys.nresol;d++)
            { 
              rhsFit[d] = compbycomp || d==0 ? linsys.resolmat[d]*l[d] : rhsFit[0](Rect(0,npts,1,npts));
                    
    //          if (residual!=0) residual[d] = rhsFit[d] - rhs[d];
              if (iter+1<irls.niter || residual!=0) res[d] = rhsFit[d] - linsys.rhs[d];

              float * ld = l[d].ptr<float>(0);
              // center / rescale (rprec)
                               ld[0] = ld[0]*linsys.rprec[d].at<float>(0,0) - 0.5*(ld[1]+ld[2]);
              if (!compbycomp) ld[3] = ld[3]*linsys.rprec[d].at<float>(3,0) - 0.5*(ld[4]+ld[5]);
              for (int i=0;i<l[d].rows;i++) ld[i] *= linsys.rprec[d].at<float>(i,0);
            }

          if (!compbycomp) res[1] = linsys.submat(res[0],1);
        }

//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << l[0].t() << std::endl;
//
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << norm(res[0],NORM_L1)/npts << std::endl;
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << norm(res[1],NORM_L1)/npts << std::endl;
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << norm(weight,NORM_L1)/npts << std::endl;
////      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << res[0].size() << std::endl;
////      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << res[1].size() << std::endl;
////      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << weight.size() << std::endl;
      if (iter+1<irls.niter)
        { float *w=weight.ptr<float>(0), *r0=res[0].ptr<float>(0), *r1=res[1].ptr<float>(0);
          float fact = - 0.5 / (irls.sig0*irls.sig0);
          for (int i=0;i<npts;i++) w[i] *= exp( fact * (r0[i]*r0[i]+r1[i]*r1[i]) );
        }
    }

  if (residual!=0) 
    { residual[0] = res[0];
      residual[1] = res[1];
    }

//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
    
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
  // /////////////////////////////////////////////////////////////////////////
  // init next LK using fitted value
  // separate camera/nonrigid motion param 
  float * lptr[2];
  Mat U, lambda; 
  float A[4],b[2];
  if (persp)
    { tParams.getHomo(0, U, lambda, npca, pframe0.cols,pframe0.rows);
      lptr[0] = lambda.ptr<float>(0);
      lptr[1] = lambda.ptr<float>(1); 
// #warning COMPBYCOMP check here
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
//      cout << U<< endl;
//      cout << U / norm(U) << endl;
//      cout << lambda << endl;
    }
  else
    { tParams.getAffineAb(0, A, b, compbycomp);
//      lptr[0] = l[0].ptr<float>(0);
//      lptr[1] = compbycomp ? l[1].ptr<float>(1) : lptr[0];
      if (compbycomp)
        { lptr[0] = l[0].ptr<float>(0)+3;
          lptr[1] = l[1].ptr<float>(1)+3;
        }
      else
        { lptr[0] = l[0].ptr<float>(0)+6;
          lptr[1] = lptr[0];
        }
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << A[0] << " " << A[1]<< " "  << b[0] << std::endl;
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << A[2] << " " << A[2]<< " "  << b[1] << std::endl;
    }
      
  // compute fitted position for all keypoints    
  Mat dMat(3,1,CV_32FC1);
  dMat.at<float>(2,0) = 1;
  for (int i=0;i<npts;i++)
    { double d0 = pts0[i].x + meanpts0[i].x;
      double d1 = pts0[i].y + meanpts0[i].y;

      float * p0  = tParams.p[0].ptr<float>(i);
      float * p1  = tParams.p[1].ptr<float>(i);

//      for (int k=0;k<npca;k++)
//        { d0 += p0[k]*lptr[0][3+k];
//          d1 += p1[k]*lptr[1][3+k];
//        }
      for (int k=0;k<npca;k++)
        { d0 += p0[k]*lptr[0][k];
          d1 += p1[k]*lptr[1][k];
        }

      if (persp)
        { dMat.at<float>(0,0) = d0;
          dMat.at<float>(1,0) = d1;
          Mat ud = U*dMat;

//          Point2f psvg = pts[i];
          pts[i].x = ud.at<float>(0) / ud.at<float>(2);
          pts[i].y = ud.at<float>(1) / ud.at<float>(2);
//          cout << pts[i]-psvg << "\t" << pts[i] << "\t" << psvg <<endl;
        }
      else
        { pts[i].x = b[0] + A[0]*d0 + A[1]*d1;
          pts[i].y = b[1] + A[2]*d0 + A[3]*d1;
        }
    }

  ////////////////////////////////////////////////
  // print residual at keypoints
//  const Mat * rhsFit = tParams.getTFit(0);
  if (dbgLevel>0)
    {
      static int first = 0; first++;
      if(!l[0].empty())
        savemat(l[0].t(),  (dbgRoot+"l1.txt").c_str(),  -1, first==1 ? ios::trunc : ios::app);
      if(!l[1].empty())
        savemat(l[1].t(),  (dbgRoot+"l2.txt").c_str(),  -1, first==1 ? ios::trunc : ios::app);
      if (persp && !lambda.empty())
        { savemat(lambda.row(0),  (dbgRoot+"lambda.txt").c_str(),  -1, first==1 ? ios::trunc : ios::app);
        }

      // compute residual of lambda fit
      Mat res1(1,npts,CV_32FC1), res2(1,npts,CV_32FC1);
      if (compbycomp)
        { for (int j=0;j<npts;j++) 
            { res1.at<float>(0,j) = rhsFit[0].at<float>(j,0)-linsys.rhs[0].at<float>(j,0);
              res2.at<float>(0,j) = rhsFit[1].at<float>(j,0)-linsys.rhs[1].at<float>(j,0);
//              if (j==0||j==500)
//                { cout << "dbg: res " << first << " " << j << " " << res2.at<float>(0,j) << " " << rhsFit[1].at<float>(j,0) << " " << pts[j].y << endl;
//                }
            }
        }

      savemat(res1,                (dbgRoot+"res1.txt").c_str(),-1, first==1 ? ios::trunc : ios::app);
      savemat(res2,                (dbgRoot+"res2.txt").c_str(),-1, first==1 ? ios::trunc : ios::app);

      savemat(weight,              (dbgRoot+"weight").c_str(),first, ios::out);

      savemat(linsys.resolmat[0],  (dbgRoot+"resolmat1").c_str(),  first);
      savemat(linsys.resolmat[1],  (dbgRoot+"resolmat2").c_str(),  first);

//      // compute residual of lambda fit
////      Mat res1(1,npts,CV_32FC1), res2(1,npts,CV_32FC1);
//      if (compbycomp)
//        { 
//          Mat defFit0 = rhs[0] + linsys.resolmat[0](Rect(3,0,npca,npts)) * l[0](Rect(0,3,1,npca));
//          Mat defFit1 = rhs[1] + linsys.resolmat[1](Rect(3,0,npca,npts)) * l[1](Rect(0,3,1,npca));
//
//          for (int j=0;j<npts;j++) 
//            { 
//              
//              res1.at<float>(0,j) = A.at<float>(0,0) * defFit0.at<float>(j,0) + A.at<float>(0,1) * defFit1.at<float>(j,0) + b.at<float>(0,0) - pts[j].x;
//              res2.at<float>(0,j) = A.at<float>(1,0) * defFit0.at<float>(j,0) + A.at<float>(1,1) * defFit1.at<float>(j,0) + b.at<float>(1,0) - pts[j].y;
////              if (j==0||j==500)
////                { cout << "dbg: res " << first << " " << j << " " << res2.at<float>(0,j) << " " << rhsFit[1].at<float>(j,0) << " " << pts[j].y << endl;
////                }
//            }
//        }
//      savemat(res1,"resb1.txt",-1, first==1 ? ios::trunc : ios::app);
//      savemat(res2,"resb2.txt",-1, first==1 ? ios::trunc : ios::app);
    }

//  ////////////////////////////////////////////////
//  // keep which point is an outlier
//  for (int j=0;j<npts;j++) 
////    { ptsData[j].outlier = hypot(rhsFit[0].at<float>(j,0)-rhs[0].at<float>(j,0),
////                                 rhsFit[1].at<float>(j,0)-rhs[1].at<float>(j,0)) > 5;
//    { ptsData[j].outlier = hypot(rhsFit[0].at<float>(j,0)-pts[j].x,
//                                 rhsFit[1].at<float>(j,0)-pts[j].y) > 5;
//    }

//  ////////////////////////////////////////////////
//  // rescale (conditionning)
//  for (int d=0;d<nresol;d++) for (int i=0;i<l[d].rows;i++) l[d].at<float>(i,0) *= linsys.rprec[d].at<float>(i,0);
    
                  
//  timer.ticAndPrint("compReg-compcoeff ");
          
//  for (int d=0;d<nresol;d++) for (int i=0;i<l[d].rows;i++) printf("%f  ",l[d].at<float>(i,0)); printf("\n")
//  printf("c ");
//  for (int i=0;i<l[0].rows;i++) printf("%f  ",l[0].at<float>(i,0)); printf("\n");
}
 

void SparseOFPCA::TCompParams::init(const VecPts & pts)
{
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
//  cout << ls[0] << endl;
//  cout << ls[1] << endl;

  for (unsigned int i=1;i<ls.size();i++)
    { ls[i] = ls[i%2].clone();
    }
  for (unsigned int i=0;i<ls.size()/2;i++)
    { ptsFit[i] = pts;
    }
}
 




void SparseOFPCA::TCompParams::getAffineAb(int i, float * A, float * b, bool compbycomp)
{ 
  cv::Mat * lmat = get(i);
  float * l0 = lmat[0].ptr<float>(0);
  float * l1 = lmat[1].ptr<float>(0);
  assert(lmat[0].type()==CV_32FC1);

  double axx=1.0, axy=0.0, ayx=0.0, ayy=1.0, bx=0.0, by=0.0;
  if (compbycomp)
    { bx=l0[0];    axx=l0[1];    axy=l0[2];
      by=l1[0];    ayx=l1[1];    ayy=l1[2];
    }
  else
    { bx=l0[0];    axx=l0[1];    axy=l0[2];
      by=l0[3];    ayx=l0[4];    ayy=l0[5];
    }
  double det = axx*ayy-axy*ayx;
  A[0] =  ayy/det; A[1] = -axy/det,
  A[2] = -ayx/det, A[3] =  axx/det;
  b[0] = - ( A[0]*bx+A[1]*by ), 
  b[1] = - ( A[2]*bx+A[3]*by ); 
}



void SparseOFPCA::TCompParams::getHomo(int i, cv::Mat & U, cv::Mat & lambda, int npca, int ncols, int nrows, double tickonovReg)
{ 
  // get vprime
  cv::Mat * vpmat = get(i);
//  float * vp = vpmat[0].ptr<float>(0);
//  assert(vpmat[0].type()==CV_32FC1);
  typedef float VTYPE;
  assert(vpmat[0].type()==CV_32FC1);
  VTYPE * vp = vpmat[0].ptr<VTYPE>(0);

  // number of principal comp
#ifdef PL2
  assert(vpmat[0].rows==9+6*npca);
#else
  assert(vpmat[0].rows==9+3*npca);
#endif

//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << npca << std::endl; 
  Mat V(3,3,CV_64FC1);
  double * v  = V.ptr<double>(0); 
  double * v3 = v+6; 

    // get v
    for (int k=0;k<9;k++) v[k] = vp[k];
//  // get v
//  for (int k=0;k<9;k++)
//  {
//      cout<< vp[k]<<" ";
//      v[k] = vp[k];
//  }
//  cout<<endl;

  // get U = V^{-1}
  double fcol = 1.0/(double)(ncols),
         frow = 1.0/(double)(nrows);
//          Mat precMatTmu = (Mat_<double>(3,3) 
//              << fcol, 0,    0,
//                 0,    frow, 0,
//                 0,    0,    1);
  Mat precMatTmu = (Mat_<double>(3,3) 
      << 1, 0,    0,
         0, 1, 0,
         0,    0,    1);
  Mat precMat = (Mat_<double>(3,3) 
      << fcol, 0,    -0.5,
         0,    frow, -0.5,
         0,    0,    1);
  U = precMat.inv() * V.inv() * precMatTmu;
  U.convertTo(U,CV_32FC1);


  // find lambda
  lambda.create (2, npca, CV_32FC1);
  VTYPE * lamPtr[2] = { lambda.ptr<VTYPE>(0), lambda.ptr<VTYPE>(1)};
  for (int k=0;k<npca;k++)
    {
#ifdef PL2 
      const int off = 6*k+9;
      for (int c=0;c<2;c++)
        { double vl = 0.0, v2 = 0.0;
          for (int l=0;l<3;l++)
            { vl += v3[l]*vp[off+3*c+l];
              v2 += v3[l]*v3[l];
            }
          lamPtr[c][k] = vl / v2;
        }
#else
      const int off = 3*k+9;
      double vl = 0.0, v2 = 0.0;
      for (int l=0;l<3;l++)
        { vl += v3[l]*vp[off+l];
          v2 += v3[l]*v3[l];
        }
      lamPtr[0][k] = lamPtr[1][k] = vl / v2;

//      // regularization
//      lamPtr[0][k] = (lamPtr[0][k]+tickonovReg)/(1+tickonovReg) ;
//      lamPtr[1][k] = (lamPtr[1][k]+tickonovReg)/(1+tickonovReg) ;

#endif
    }

//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
//  cout << vpmat[0].t() << endl;
//  cout << V << endl;
//  cout << U << endl;
//  cout << lambda << endl;
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
}
 





void SparseOFPCA::setIRLS(double s0, int _niter)
{ irls.sig0  = s0;
  irls.niter = _niter;
}
 




void SparseOFPCA::reevesSBS(const Mat & mat, int k, std::vector<bool> & keep) const
{
  int n = mat.rows;
      
  Mat pinv = mat.inv(DECOMP_SVD);

  vector<bool>().swap(keep); 
  keep.resize(n,true);

  for (;n>k;n--)
    { 
      // find row that minimize Reeves criterion
      int imin = -1; 
      double smin = -1, denommin = -1;
      for (unsigned int i=0;i<keep.size();i++)
        { 
          if (!keep[i]) continue;
          
          const Mat & ai =  mat.row(i);
          const Mat & pi = pinv.col(i);
//          std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << ai.size() << " " << pi.size() << std::endl;
//          double denom = 1 - ai.t() * pi, si = (pi.t() * pi) / denom ;
          double denom = 1 - ai.dot(pi.t()), si = pi.dot(pi) / denom ;
          if (imin==-1 || si<smin)
            { smin = si;
              imin = i;
              denommin = denom;
            }
        }

      // remove line
      keep[imin] = false;

      // update pinv
      const Mat & pi = pinv.col(imin);
      Mat Api  = mat * pi;
          Api *= 1.0/denommin;

      pinv += pi*Api.t();

#if 0
      // test
        { Mat mat2;
          for (unsigned int i=0;i<keep.size();i++)
            { 
              if (!keep[i]) continue;
              mat2.push_back(mat.row(i));
              std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << mat.size() << " " << mat2.size()<< std::endl;
            }
          std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
          cout << "pinv="     << pinv << endl;
          cout << "pinv2="    << mat2.inv(DECOMP_SVD) << endl;
          std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << pinv.size() << " " << mat2.inv(DECOMP_SVD).size()<< std::endl;
        }
#endif
    }

}


void SparseOFPCA::reevesSBS2(const cv::Mat & mat0, int k, std::vector<bool> & keep) const
{
  Mat rPrec = (mat0.t() * mat0).inv(DECOMP_CHOLESKY).diag();
  sqrt(rPrec,rPrec);
//  for (int i=0;i<3;i++)
//    { rPrec.at<double>(i)   *= 100;
//      rPrec.at<double>(i+7) *= 100;
//    }
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
//  cout << rPrec << endl;
  Mat mat = mat0 * Mat::diag(rPrec);


  int n0 = mat.rows / 2;
      
//  SVD svd(mat);
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
//  cout << svd.w << endl;
//  cout << svd.vt << endl;
//  cout << (mat.t() * mat).inv(DECOMP_CHOLESKY)        << endl;
  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
  cout << (mat.t() * mat).inv(DECOMP_CHOLESKY).diag() << endl;



  Mat pinv = mat.inv(DECOMP_SVD);

  vector<bool>().swap(keep); 
  keep.resize(n0,true);

  for (int n=n0;n>k;n--)
    { 
      // find row that minimize Reeves criterion
      int imin = -1; 
      double smin = -1, denommin[2] = {-1,-1};
      for (int i=0;i<n0;i++)
        { 
          if (!keep[i]) continue;
          
          double denom[2], si = 0.0;
          for (int j=0;j<2;j++)
            { const Mat & ai =  mat.row(i+j*n0);
              const Mat & pi = pinv.col(i+j*n0);
              denom[j] = 1 - ai.dot(pi.t());
              si += pi.dot(pi) / denom[j] ;
            }


          if (imin==-1 || si<smin)
            { smin = si;
              imin = i;
              for (int j=0;j<2;j++) denommin[j] = denom[j];
            }
        }

      // remove line
      keep[imin] = false;

      // update pinv
      for (int j=0;j<2;j++)
        { 
          const Mat & pi = pinv.col(imin+j*n0);
          Mat Api  = mat * pi;
              Api *= 1.0/denommin[j];
          pinv += pi*Api.t();
        }

#if 0
      // test
        { Mat mat2;
          for (unsigned int i=0;i<keep.size();i++)
            { 
              if (!keep[i]) continue;
              mat2.push_back(mat.row(i));
              std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << mat.size() << " " << mat2.size()<< std::endl;
            }
          std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
          cout << "pinv="     << pinv << endl;
          cout << "pinv2="    << mat2.inv(DECOMP_SVD) << endl;
          std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << pinv.size() << " " << mat2.inv(DECOMP_SVD).size()<< std::endl;
        }
#endif
    }

}





void SparseOFPCA::setPCAModel(bool _compbycomp)
{ compbycomp = _compbycomp;
}
 








void SparseOFPCA::apply(const cv::Mat & img, cv::Mat & out, int interpolation) const
{
   SparseOF::apply(img, out, interpolation);

//    cout<<"SparseOFPCA::apply";
  if (drawPts)
    { for (unsigned int i=0;i<pts0.size();i++)
        { 
          if (ptsData[i].outlier)
            { int thickness = 2, linetype = 8;
              circle( out, pts0[i], 5,      Scalar( 0  , 0, 255 ), thickness, linetype );
            //circle( out, pts [i], 2, Scalar( 255, 255, 0 ), thickness, linetype );
            }
////          if (useKptCons && ptsData[i].weightKC==0)
////          if (              ptsData[i].weightKC==0)
////            { int thickness = 2, linetype = 8;
////              circle( out, pts0[i], 2,      Scalar( 255 , 0, 0 ), thickness, linetype );
////            //circle( out, pts [i], 2, Scalar( 255, 255, 0 ), thickness, linetype );
////            }
          if (              ptsData[i].weightKC>0.1)
            { int thickness = 2, linetype = 8;
              circle( out, pts0[i], 2,      Scalar( 255 , 0, 0 ), thickness, linetype );
            //circle( out, pts [i], 2, Scalar( 255, 255, 0 ), thickness, linetype );
            }
          else if (              ptsData[i].weightKC>0.0)
            { int thickness = 2, linetype = 8;
              circle( out, pts0[i], 2,      Scalar( 255 , 0, 255 ), thickness, linetype );
            //circle( out, pts [i], 2, Scalar( 255, 255, 0 ), thickness, linetype );
            }

        }
    }

}
 


 




void SparseOFPCA::motionSeg() const
{
  int nsamples = samples[0].size();

  Mat sumal[20];

  NiftiIO niiIO;
  for (int i=0;i<nsamples;i++)
    { 
//      int ksize = 3;

      Mat lap[2];

      for (int k = 0; k < 5 ; k++)
        { int ksize = 3 + 4*k;
          
          for (int d=0;d<2;d++)
            { Laplacian (samples[d][i], lap[d], CV_32FC1, ksize, 1, 0, BORDER_REPLICATE );
            }
          
          char root[128]; 
          sprintf(root,"lapdef-1-%02d",ksize); niiIO.save(lap[0],root,0,i);
          sprintf(root,"lapdef-2-%02d",ksize); niiIO.save(lap[1],root,0,i);

          Mat abslap = abs(lap[0]) + abs(lap[1]);
          sprintf(root,"abslap-%02d",ksize); niiIO.save(abslap,root,0,i);
          
          if (i==0)
            { sumal[k]  = abslap;
            }
          else
            { sumal[k] += abslap;
            }
          sprintf(root,"sum-%02d",ksize); niiIO.save(sumal[k],root,0,i);
        }
    }
}
 



void SparseOFPCA::setKptConsist(double h)
{ useKptCons = h>0;
  if (useKptCons)
    { kptCons.setBinSize(h);
    }
}
 





void SparseOFPCA::gemm_omp(const cv::Mat & A, const cv::Mat & xm, double alpha, const cv::Mat & bm, double beta, cv::Mat & res)
{ 
  assert(A.cols==xm.rows);
  assert(A.rows==bm.rows     || bm.empty());
  assert(A .type()==CV_32FC1);
  assert(xm.type()==CV_32FC1);
  assert(bm.type()==CV_32FC1 || bm.empty());

  res.create(A.rows,1,CV_32FC1);
  float * r = res.ptr<float>(0);
  
  const float * b = bm.ptr<float>(0);
  const float * x = xm.ptr<float>(0);

  if (bm.empty())
    { 
//      #pragma omp parallel for schedule(dynamic,5)
      for (int i=0;i<A.rows;i++)
        { const float * Ai = A.ptr<float>(i);
          double ax = 0;
          for (int j=0;j<A.cols;j++) ax += Ai[j]*x[j];
          r[i] = alpha*ax;
        }
    }
  else
    {
//      #pragma omp parallel for schedule(dynamic,5)
      for (int i=0;i<A.rows;i++)
        { const float * Ai = A.ptr<float>(i);
          double ax = 0;
          for (int j=0;j<A.cols;j++) ax += Ai[j]*x[j];
          r[i] = alpha*ax + beta*b[i];
        }
    }
}
 




void SparseOFPCA::setDbgLevel(int lev, const char * root)
{ dbgLevel = lev;
  dbgRoot  = root==0 ? "" : root;
}
 





void SparseOFPCA::initWeight(cv::Mat & w, const Mat & next)
{ 
  w.create(pts0.size(),1,CV_32FC1);
//  return;

//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl;
  
  if (useKptCons)
    { 
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl;
//      w.create(pts0.size(),1,CV_32FC1);

    //  Mat weight(npts,1,CV_32FC1);
      kptCons.locNorm(1, pts, next);

//      kptCons.locNorm(1, pts, next);
      const Mat & locAff = kptCons.localTransforms();
//            Mat wLocAff;   kptCons.findOutliers(wLocAff);
      Mat & wLocAff = w;   kptCons.findOutliers(wLocAff);

      if (dbgLevel>0)
        { static int nf=samples[0].size()-1; nf++; 
          Mat kptsvg;    hconcat ( locAff, wLocAff, kptsvg);
          savemat(kptsvg,  (dbgRoot+"kpt").c_str(), nf);
        }
  
//      int nin = 0;
      for (unsigned int j=0;j<pts.size();j++) 
        { ptsData[j].weightKC  = wLocAff.at<float>(j,0);
//          ptsData[j].outlierKC = ptsData[j].weightKC==0;
//          nin += ptsData[j].weightKC>0;
        }

      return;
    }

//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl;

//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
  if (lkParam.fbSig>0) // skip first frame
    { 
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl;
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
//      w.create(pts0.size(),1,CV_32FC1);
//
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__
//        << " " << pts0.size() 
//        << " " << pts.size() 
//        << " " << prevPts.size() 
//        << " " << dpts.size() 
//        << std::endl;
      Rect rnext(Point(0,0),next.size());
      double sumw = 0;
      float fact = - 0.5 / (lkParam.fbSig*lkParam.fbSig);
      for (unsigned int j=0;j<pts.size();j++) 
        { //float sig = 3; 
//          std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << j<<  std::endl;
//          std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << j<<  std::endl;
          w.at<float>(j,0)     = exp( fact * lkParam.fbError[j]*lkParam.fbError[j] );
          ptsData[j].weightKC  = w.at<float>(j,0);
          if (!rnext.contains(pts[j]))
            { ptsData[j].weightKC  = 0;
              w.at<float>(j,0)     = 0;
            }
          sumw += w.at<float>(j,0);
//          std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << j<<  std::endl;
    //          ptsData[j].outlierKC = ptsData[j].weightKC==0;
        }
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << sumw<<  std::endl;
      if (dbgLevel>0)
        { static int nf=samples[0].size()-1; nf++; 
    //      Mat kptsvg;    hconcat ( locAff, wLocAff, kptsvg);
//          savemat(Mat(lkParam.fbError),  "dpts", nf);
          savemat(Mat(lkParam.fbError).t(),  (dbgRoot+"fbError.txt").c_str(), -1, nf==(int)samples[0].size() ? ios::trunc : ios::app);
//          savemat(Mat(dpts).reshape(1).col(1).t(),  (dbgRoot+"dpts2.txt").c_str(), -1, nf==(int)samples[0].size() ? ios::trunc : ios::app);
        }
  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl;
      return;
    }

//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl;
  if (errSig>0)
    { 
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl;
//      w.create(pts0.size(),1,CV_32FC1);
      Rect rnext(Point(0,0),next.size());
      float fact = - 0.5 / (errSig*errSig);
      double sumw = 0.0;
      for (unsigned int j=0;j<pts.size();j++) 
        { 
//          Point2f dpts = pts[j] - prevPts[j];
          ptsData[j].weightKC  = lkParam.status[j] * exp( fact * lkParam.err[j]*lkParam.err[j] );
          w.at<float>(j,0)     = ptsData[j].weightKC;
          if (!rnext.contains(pts[j]))
            { ptsData[j].weightKC  = 0;
              w.at<float>(j,0)     = 0;
            }
          sumw += w.at<float>(j,0);
        }
      w /= sumw;
      
      if (dbgLevel>0)
        { static int nf=samples[0].size()-1; nf++; 
          savemat(Mat(lkParam.err),  "lkErrr", nf);
          savemat(Mat(lkParam.err).t(),  (dbgRoot+"lkErr.txt"     ).c_str(), -1, nf==(int)samples[0].size() ? ios::trunc : ios::app);
          savemat(w.t(),                 (dbgRoot+"initWeight.txt").c_str(), -1, nf==(int)samples[0].size() ? ios::trunc : ios::app);
        }
      
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl;
      return;
    }

//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl;
  VecPts & prevPts = tParams.getPtsFit(1);
  if (prevPts.size()>0) // skip first frame
    { 
//      std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl;
//      double sumw = 0;
//      long sumfailed = 0;

//      w.create(pts0.size(),1,CV_32FC1);
      Rect rnext(Point(0,0),next.size());
      float fact = - 0.5 / (dySig*dySig);
      for (unsigned int j=0;j<pts.size();j++) 
        { Point2f dpts = pts[j] - prevPts[j];
//          sumfailed += lkParam.status[j];
          ptsData[j].weightKC  = lkParam.status[j] * exp( fact * dpts.dot(dpts) );
          w.at<float>(j,0)     = ptsData[j].weightKC;
          if (!rnext.contains(pts[j]))
            { ptsData[j].weightKC  = 0;
              w.at<float>(j,0)     = 0;
            }
//          sumw += w.at<float>(j,0);
        }

//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl;
//      static int nf=samples[0].size()-1; nf++; 
//      std::cout << endl << "dbg : "<< nf << " " << sumfailed << " " << sumw << std::endl;
      if (dbgLevel>0)
        { static int nf=samples[0].size()-1; nf++; 
          VecPts dpts(pts0.size());
          for (unsigned int j=0;j<pts.size();j++) dpts[j] = pts[j] - prevPts[j];
            
          savemat(Mat(dpts), (dbgRoot+"dpts.txt").c_str(), nf);
          savemat(Mat(dpts).reshape(1).col(0).t(),  (dbgRoot+"dpts1.txt").c_str(), -1, nf==(int)samples[0].size() ? ios::trunc : ios::app);
          savemat(Mat(dpts).reshape(1).col(1).t(),  (dbgRoot+"dpts2.txt").c_str(), -1, nf==(int)samples[0].size() ? ios::trunc : ios::app);
          
          savemat(Mat(dpts).reshape(1).col(1).t(),  (dbgRoot+"initWeight.txt").c_str(), -1, nf==(int)samples[0].size() ? ios::trunc : ios::app);
        }
      return;
    }
  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl;

  w = 1;
}
 

void SparseOFPCA::setPtsFit(PtsFitType pft)
{ ptsFitType = pft;
}
 


/////////////////////////////////////////////////////////////////////////////////////////////
//////////////  Regul
/////////////////////////////////////////////////////////////////////////////////////////////

void SparseOFPCA::Regul::init(const cv::Mat & lmin, cv::Mat & lmax)
{
  useconst=false;

//  c0Metric = 2.0 / (lmax-lmin);
  cv::pow(0.5*(lmax-lmin),-2,c0_tLL);
  
  c0_x0 = 0.5*(lmin+lmax);
  divide( c0_x0, 0.5*(lmax-lmin), c0_tLx0);

//  Mat c0_x0 = 0.5*(lmin+lmax);

//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
//  cout << c0Metric.t() << endl;
//  cout << 0.5*(lmax-lmin).t() << endl;
}
 


double SparseOFPCA::Regul::c0Const(const cv::Mat & l) const
{ Mat diff = l-c0_x0;
  const float * d  =   diff.ptr<float>(0);
  const float *tll = c0_tLL.ptr<float>(0);

  double res = 0.0;
  for (int i=3;i<c0_x0.rows;i++) res += tll[i]*d[i]*d[i]; 
  return (res/(c0_x0.rows-3));
}
 


void SparseOFPCA::Regul::init(float a0, float a1, float p0, float p1, float c0delta)
{ 
  useconst=false;

  affCoeff[0] = a0;
  affCoeff[1] = a1;
  pcaCoeff[0] = p0;
  pcaCoeff[1] = p1;
  
  sa=0.0, sp=0.0;
  for (int i=0;i<2;i++)
    { a[i] = affCoeff[i]*affCoeff[i];
      p[i] = pcaCoeff[i]*pcaCoeff[i];
      sa += a[i];
      sp += p[i];
    }

  c0_delta = c0delta;
}
 

void SparseOFPCA::Regul::regulSystem(const cv::Mat & tMM, const cv::Mat & tMrhs, const float * prev1, cv::Mat & tMMreg, cv::Mat & tMrhsreg) const
{
  assert(tMM.type()==CV_32FC1 && tMrhs.type()==CV_32FC1);
  const float * m  = tMM     .ptr<float>(0);
  const float * r  = tMrhs   .ptr<float>(0);
        float * mr = tMMreg  .ptr<float>(0);
        float * rr = tMrhsreg.ptr<float>(0);

  const float * tll  = c0_tLL .ptr<float>(0);
  const float * tlx0 = c0_tLx0.ptr<float>(0);

  if (prev1==0)
    {             for (int i=0;i<3       ;i++) { mr[i*(tMM.rows+1)] = m[i*(tMM.rows+1)] + a[0];                                     }
      if (tll==0) for (int i=3;i<tMM.rows;i++) { mr[i*(tMM.rows+1)] = m[i*(tMM.rows+1)] + p[0];                                     }
      else        for (int i=3;i<tMM.rows;i++) { mr[i*(tMM.rows+1)] = m[i*(tMM.rows+1)] + p[0]*tll[i]; rr[i] = r[i] + p[0]*tlx0[i]; }
    }
  else
    {             for (int i=0;i<3       ;i++) { mr[i*(tMM.rows+1)] = m[i*(tMM.rows+1)] + sa;                rr[i] = r[i] + a[1]*prev1[i];                  }
      if (tll==0) for (int i=3;i<tMM.rows;i++) { mr[i*(tMM.rows+1)] = m[i*(tMM.rows+1)] + sp;                rr[i] = r[i] +                p[1] * prev1[i]; }
      else        for (int i=3;i<tMM.rows;i++) { mr[i*(tMM.rows+1)] = m[i*(tMM.rows+1)] + p[0]*tll[i]+p[1];  rr[i] = r[i] + p[0]*tlx0[i] + p[1] * prev1[i]; }
    }
}


float SparseOFPCA::Regul::newtonIter4c0(const cv::Mat & tMM, const cv::Mat & tMrhs, const cv::Mat & tMMx0, cv::Mat & l, float & mu) const
{ 
  assert(tMM.type()==CV_32FC1 && tMrhs.type()==CV_32FC1);
 
//  Mat tMMc0   = tMM  .clone();
//  Mat tMrhsc0 = tMrhs.clone();
  Mat tMMc0  ; tMM  .convertTo(tMMc0  ,CV_64FC1);
  Mat tMrhsc0; tMrhs.convertTo(tMrhsc0,CV_64FC1);

//        float * mc = tMMc0  .ptr<float>(0);
//        float * rc = tMrhsc0.ptr<float>(0);
        double * mc = tMMc0  .ptr<double>(0);
        double * rc = tMrhsc0.ptr<double>(0);

  const float * tll   = c0_tLL.ptr<float>(0);
  const float * tmmx0 = tMMx0.ptr<float>(0);

  // augment linear system + compute first rhs
  for (int i=0;i<3       ;i++) {                                  rc[i] -= tmmx0[i]; }
  for (int i=3;i<tMM.rows;i++) { mc[i*(tMM.rows+1)] += mu*tll[i]; rc[i] -= tmmx0[i]; }


  // solve for p and q
//  choleskyDecomp(tMMc0.ptr<float>(), tMMc0.step, tMMc0.cols);
  choleskyDecomp(tMMc0.ptr<double>(), tMMc0.step, tMMc0.cols);

//  Mat & p = l;
//  p = tMrhsc0.clone();
//  choleskySolve (tMMc0.ptr<float>(), tMMc0.step, tMMc0.cols, p.ptr<float>(), p.step, 1, false);
//  choleskySolve (tMMc0.ptr<float>(), tMMc0.step, tMMc0.cols, p.ptr<float>(), p.step, 1, true);
  Mat p = tMrhsc0.clone();
  choleskySolve (tMMc0.ptr<double>(), tMMc0.step, tMMc0.cols, p.ptr<double>(), p.step, 1, false);
  choleskySolve (tMMc0.ptr<double>(), tMMc0.step, tMMc0.cols, p.ptr<double>(), p.step, 1, true);
  
//  Mat q(p.rows,1,CV_32FC1); 
//  float * p_ = p.ptr<float>(0);
//  float * q_ = q.ptr<float>(0);
//  for (int i=0;i<3       ;i++) { q_[i] = 0.0;          }
//  for (int i=3;i<tMM.rows;i++) { q_[i] = tll[i]*p_[i]; }
//  choleskySolve (tMMc0.ptr<float>(), tMMc0.step, tMMc0.cols, q.ptr<float>(), q.step, 1, false);
  Mat q(p.rows,1,CV_64FC1); 
  double * p_ = p.ptr<double>(0);
  double * q_ = q.ptr<double>(0);
  for (int i=0;i<3       ;i++) { q_[i] = 0.0;          }
  for (int i=3;i<tMM.rows;i++) { q_[i] = tll[i]*p_[i]; }
  choleskySolve (tMMc0.ptr<double>(), tMMc0.step, tMMc0.cols, q.ptr<double>(), q.step, 1, false);

  // compute newton update 
  double normLp2 = 0.0, normq2 = 0.0;
  for (int i=0;i<3       ;i++) {                                normq2 += q_[i]*q_[i]; }
  for (int i=3;i<tMM.rows;i++) { normLp2 += tll[i]*p_[i]*p_[i]; normq2 += q_[i]*q_[i]; }

//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << normLp2 << " " << c0_delta << std::endl;

  mu = mu + (sqrt(normLp2)-c0_delta) * normLp2 / (c0_delta * normq2);

  // shift back
  const float * x0_ = c0_x0.ptr<float>(0);
        float * l_  = l    .ptr<float>(0);
//  for (int i=0;i<tMM.rows;i++) { l_[i] += x0_[i]; }
  for (int i=0;i<tMM.rows;i++) { l_[i] = p_[i] + x0_[i]; }

  return (normLp2);

}



///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////



void SparseOFPCA::keypointSelection()
{
//  int npts = pts0.size();

  // reeves
  { 
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << pts0.size() <<  std::endl;
      std::vector<bool> keep;
      if (compbycomp)
        { 

          if (1) // reeves of full matrix
            { 
              int r = linsys.resolmat[0].rows, c = linsys.resolmat[0].cols;
              Mat fullResol(2*r, 2*c, CV_64FC1, Scalar(0));
              linsys.resolmat[0].copyTo(fullResol(Rect(0,0,c,r)));
              linsys.resolmat[1].copyTo(fullResol(Rect(c,r,c,r)));
              reevesSBS2(fullResol, maxKeyPt, keep);
            }
          else // reeves on pca only
            {

              int tcCol = tcamP->nCol();
              int r = linsys.resolmat[0].rows;//, c = linsys.resolmat[0].cols;
              Mat fullResol(2*r, 2*npca, CV_64FC1, Scalar(0));
//              std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
//              cout << linsys.resolmat[0](Rect(tcCol,0,npca,r)).size() << endl;
//              std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
//              cout << fullResol(Rect(0,0,npca,r)).size()       << endl;
//              std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;
              linsys.resolmat[0](Rect(tcCol,0,npca,r)).copyTo(fullResol(Rect(0,   0,npca,r)));
              linsys.resolmat[1](Rect(tcCol,0,npca,r)).copyTo(fullResol(Rect(npca,r,npca,r)));
              reevesSBS2(fullResol, maxKeyPt, keep);
            }
        }
      else
        { reevesSBS2(linsys.resolmat[0], maxKeyPt, keep);
        }
          
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << pts0.size() <<  std::endl;

      Mat newmat[2];
      for (int d=0;d<(compbycomp?2:1);d++)
        { newmat[d].create(compbycomp?maxKeyPt:2*maxKeyPt,linsys.resolmat[0].cols,CV_64FC1);
          for (unsigned int i=0,j=0;i<keep.size();i++) if (keep[i]) linsys.resolmat[d].row(i).copyTo(newmat[d].row(j++));
          swap(linsys.resolmat[d],newmat[d]);
        }


      VecPts newPts0(maxKeyPt);
      for (unsigned int i=0,j=0;i<keep.size();i++) if (keep[i]) newPts0[j++] = pts0[i];
      VecPts newPts = newPts0;
      pts .swap(newPts); 
      pts0.swap(newPts0); 
      
      vector<PtsData> newPtsData(maxKeyPt);
      for (unsigned int i=0,j=0;i<keep.size();i++) if (keep[i]) newPtsData[j++] = ptsData[i];

//      npts = pts0.size();
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << pts0.size() <<  std::endl;
    }
}
 



