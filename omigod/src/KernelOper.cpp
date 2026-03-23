#include <KernelOper.h>

using namespace omigod;
using namespace cv;
using namespace std;


KernelOper::KernelOper ()
{ kernel = &linKer;
}



KernelOper::~KernelOper ()
{ 
}
 

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void KernelOper::Kernel::calc (int vcount, int n, const float *vecs, const float *another, float *results)
{ const KernelOper::Kernel & constthis = *this;
  constthis.calc (vcount, n, vecs, another, results);
}
 
int  KernelOper::Kernel::getType () const
{ return (cv::ml::SVM::CUSTOM);
}
 
////////////////////////////////////////////////////////////////////////////

void KernelOper::LinKernel::calc (int vcount, int n, const float *vecs, const float *another, float *results) const
{
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCfloatION__<<"  "<< __LINE__<< " " << vcount << " " << n << std::endl; 
  for( int j = 0; j < vcount; j++ )
    { 
      const float * sample = vecs + j*n;

      double lin = 0;
      for( int k = 0; k < n ; k ++ ) lin += sample[k]*another[k];
      results[j] = lin;
//      for( int k = 0; k < n ; k ++ ) lin += double(sample[k])*double(another[k]);

      // using the following opencv code, we can find the same results as the opencv LINEAR kernel
      // however huge lost of precision: they have 3 / 4 of float operation when I have all in double
////      const float* sample = &vecs[j*var_count];
//            double s = 0;
//            int k;
//            for( k = 0; k <= n - 4; k += 4 )
//                s += sample[k]*another[k] + sample[k+1]*another[k+1] +
//                sample[k+2]*another[k+2] + sample[k+3]*another[k+3];
//            for( ; k < n; k++ )
//                s += sample[k]*another[k];
//            results[j] = s;
////            results[j] = (Qfloat)(s*alpha + beta);

    }

}


//////////////////////////////////////////////////////////////////////////////////////////////
void KernelOper::RBFKernel::calc (int vcount, int n, const float *vecs, const float *another, float *results) const
{
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << vcount << " " << n << std::endl; 
  double fact =  - 0.5 / (sigma*sigma);
  for( int j = 0; j < vcount; j++ )
    { 
      const float * sample = vecs + j*n;

      double res2 = 0;
      for( int k = 0; k < n ; k ++ ) { double d=sample[k]-another[k]; res2 += d*d; }
      results[j] = exp(fact*res2);
    }
}



//////////////////////////////////////////////////////////////////////////////////////////////
void KernelOper::PolyKernel::calc (int vcount, int n, const float *vecs, const float *another, float *results) const
{
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << vcount << " " << n << std::endl; 
  double fact =  1.0 / (sigma*sigma);
  for( int j = 0; j < vcount; j++ )
    { const float * sample = vecs + j*n;
      double lin = 0;
      for( int k = 0; k < n ; k ++ ) lin += sample[k]*another[k];
      results[j] = pow(1+lin*fact,deg);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////

const char * KernelOper::ker2str(KernelType k)
{ switch (k)
    { case LIN      : return ("LIN     "); break;
      case GAUSSIAN : return ("GAUSSIAN"); break;
      case POLY     : return ("POLY"    ); break;
      default:        return ("UNKNOWN" ); break;
    }
}

KernelOper::KernelType KernelOper::str2ker(const char * s)
{ if (strcmp(s,"LIN"     )==0) return (LIN     );
  if (strcmp(s,"GAUSSIAN")==0) return (GAUSSIAN);
  if (strcmp(s,"POLY"    )==0) return (POLY    );
  return (UNKNOWN );
}
 







void KernelOper::setKernel(KernelType kt, double sig, int deg)
{ switch (kt)
    { case LIN      : kernel=&linKer;  break;
      case GAUSSIAN : kernel=&rbfKer;  break;
      case POLY     : kernel=&polyKer; break;
      default:        assert(false);   break;
    }

  kernel->sigma = sig;
  kernel->deg   = deg;
}
 




void KernelOper::buildKernelMatrix(const cv::Mat & data, cv::Mat & kmat, bool centered) const
{
  assert(data.type()==CV_32FC1);

  int ndata = data.rows;
  int dim   = data.cols;

  kmat.create(ndata,ndata,CV_32FC1);

  // compute kernel on half
  for (int i=0;i<ndata;i++)
    { 
      const float * di = data.ptr<float>(i); 
            float * ki = kmat.ptr<float>(i); 
      for (int j=0;j<=i;j++)
        { const float * dj = data.ptr<float>(j); 
          kernel->calc(1, dim, di, dj, ki+j);
        }
    }

  // complete second half
  for (int i=0;i<ndata;i++)
    { const float * ki = kmat.ptr<float>(i); 
      for (int j=0;j<i;j++) kmat.ptr<float>(j)[i] = ki[j]; 
    }

  if (centered)
    { // TODO center the matrix
      // kmat sym: sumrow = sumcol
      Mat kmat0, kmat01;
      cv::reduce (kmat,  kmat0,  0, REDUCE_AVG, -1);
      cv::reduce (kmat0, kmat01, 1, REDUCE_AVG, -1);
      const float * k0 = kmat0.ptr<float>(0);
            float  k01 = kmat01.at<float>(0,0);
      for (int i=0;i<ndata;i++)
        { float * ki = kmat.ptr<float>(i); 
          for (int j=0;j<ndata;j++) ki[j] = ki[j] - k0[i] - k0[j] + k01; 
        }
//      Mat kmat0, kmat1, kmat01;
//      cv::reduce (kmat,  kmat0,  0, REDUCE_AVG, -1);
//      cv::reduce (kmat,  kmat1,  1, REDUCE_AVG, -1);
//      cv::reduce (kmat0, kmat01, 1, REDUCE_AVG, -1);
//      const float * k0 = kmat0.ptr<float>(0);
//      const float * k1 = kmat1.ptr<float>(0);
//            float  k01 = kmat01.at<float>(0,0);
//      for (int i=0;i<ndata;i++)
//        { const float * ki = kmat.ptr<float>(i); 
////          for (int j=0;j<=i;j++)
////            { ki[j] = ki[j] - k0[]
////            }
//        }

    }
}
 




void KernelOper::kernelPCA(const cv::Mat & data, cv::Mat & evec, cv::Mat & eval, int K, bool centered) const
{
  // kernel mat
  Mat kmat;
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " buildKernel matrix" <<  std::endl;
  buildKernelMatrix(data, kmat, centered);

  // eigenvalue decomposition
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " eigenval decomp" <<  std::endl;
  bool ret = eigen(kmat, eval, evec);

//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << ret << std::endl; 
  cout << eval << endl;
  // keep K first principal components
  if( K>0 && K<kmat.rows )
    { eval = eval.rowRange(0,K).clone();
      evec = evec.rowRange(0,K).clone();
    }
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
//  cout << eval << endl;

  // normalize
  assert(eval.type()==CV_32FC1);
  for (int i=0;i<eval.rows;i++)
    { evec.row(i) /= sqrt(eval.at<float>(i,0));
    } 

}
 

void KernelOper::kernelPCAProj(const cv::Mat & data, const cv::Mat & evec, const cv::Mat & eval, const cv::Mat & xInput, cv::Mat & xProj) const
{ 
  assert(data  .type()==CV_32FC1);
  assert(xInput.type()==CV_32FC1);

  int nX    = xInput.rows;
  int K     = eval.rows;
  int ndata = data.rows;
  int dim   = data.cols;
  const float * data0 = data.ptr<float>(0);
  
  xProj.create(nX,K,CV_32FC1);
  
  for (int i=0;i<nX;i++)
    { const float * x  = xInput.ptr<float>(i);
            float * xp = xProj .ptr<float>(i);
      for (int k=0;k<K;k++)
        { const float * evk = evec.ptr<float>(k);
          double xpk = 0;
          for (int j=0;j<ndata;j++)
            { float xd;
              kernel->calc(1, dim, x, data0+j*dim, &xd);
              xpk += xd * evk[j];
            }
          xp[k] = xpk;
        }
    }

}







void KernelOper::kernelDistance(const cv::Mat & kmat, cv::Mat & dist) const
{
  assert(kmat.isContinuous());
  assert(dist.isContinuous());
  assert(kmat.rows==dist.rows && kmat.cols==dist.cols);
  assert(kmat.type()==CV_32FC1 && dist.type()==CV_32FC1);
  
  int n = kmat.rows;
  const float * kdiag = kmat.ptr<float>(0); 

  // compute distance
  for (int i=0;i<n;i++)
    { const float * ki = kmat.ptr<float>(i); 
            float * di = dist.ptr<float>(i); 
      for (int j=0;  j<i;j++) di[j] = kdiag[i*(1+n)] + kdiag[j*(1+n)] - 2*ki[j]; 
      for (int j=i+1;j<n;j++) di[j] = kdiag[i*(1+n)] + kdiag[j*(1+n)] - 2*ki[j]; 
    }

  // fill diag with 0 (allow inplace)
  float * ddiag = dist.ptr<float>(0); 
  for (int i=0;i<n;i++) ddiag[i*(1+n)] = 0; 
}
 

void KernelOper::kernelKNN(const cv::Mat & dist, int K, cv::Mat & kdist) const
{
  assert(dist.type()==CV_32FC1);
  kdist.create(dist.rows,1,CV_32FC1);
  float * kd = kdist.ptr<float>(0);

  int n = dist.rows;
  float * buff = new float[K+1];

  for (int i=0;i<n;i++)
    { const float * di = dist.ptr<float>(i); 
      partial_sort_copy(di, di+n, buff, buff+K+1);
      kd[i] = buff[K];
    }

  delete[] buff;
}
 

void KernelOper::kernelKmeans(const cv::Mat & kmat, int K, cv::Mat & part) const
{
  assert(kmat.type()==CV_32FC1);

  int n = kmat.rows;
  const float * kmatdiag = kmat.ptr<float>(0);

  // cord of the means in feature space;
  // m_k = sum_i gamma_ik phi(xi)
  Mat gamma(K,n,CV_32FC1);

  // dist from each pt to each mean
  Mat dist(K,n,CV_32FC1);

  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " WARNING: not finished : only compute distnce, do not iter" << std::endl; 
  // 
  for (int k=0;k<K;k++)
    { Mat gk      = gamma.row(k);
      Mat gkKMat  = gk * kmat;
      float * gkK = gkKMat.ptr<float>(0);
      float gkKgk = gkKMat.dot(gk);
      
      // dkl = || xl - mk ||^2
      //     = gkKgk -2 (Kgk)_l + k(xl,xl)
      float * dk = dist.ptr<float>(k);
      for (int l=0;l<n;l++)
        { dk[l] = gkKgk - 2*gkK[l] + kmatdiag[l*(n+1)];
        }
    }
}
 

