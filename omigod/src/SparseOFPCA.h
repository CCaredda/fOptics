#ifndef __SPARSEOFPCA_
#define __SPARSEOFPCA_

#include <KptConstraint.h>
#include <SparseOF.h>

#include <iostream>

#include <opencv2/opencv.hpp>

namespace omigod {

/**
 * @author Michael Sdika
 */
class SparseOFPCA : public SparseOF
{
public :
  
  /* ----------------------------------  */
  /* --       Public Types       ------  */
  /* ----------------------------------  */
  enum PtsFitType { LAG, LLAG, EULER };
  static const char * pfit2str(PtsFitType t);
  static PtsFitType   str2pfit(const char * s);
  

  /* ----------------------------------  */
  /* --       Public Functions   ------  */
  /* ----------------------------------  */

  /** Constructor */
  SparseOFPCA ();

  /** Destructor */
  virtual ~SparseOFPCA ();

  /** set the number of samples that will be used for traning. */
  void setSampleSize(int s);

  /** set the i-th sample. */
  void setSample(int i, const cv::Mat & tr1, const cv::Mat & tr2);
  void setSample(int i, const cv::Mat & tr1, const cv::Mat & tr2, const cv::Mat & img);

  /** initial frame. 
   * samples should already be here for training
   * */
  void setFrame0(const cv::Mat & img);

  /** perform the registration. */
  void computeReg(const cv::Mat & next);

  /** perform the registration. 
   * @param model in trans, aff, persp.
   * @param edgeweight: <0:none, >=0 zero edge lower than edgecut 
   * */
  void setCamMotion(const std::string & model, double edgecut, bool composition);

  /** compbycomp */
  void setPCAModel(bool _compbycomp);
  
  /** compbycomp */
  void setPtsFit(PtsFitType pft);

  /** number of principal components. */
  void setNPC(int npc);

  /** IRLS parameters. */
  void setIRLS(double s0, int _niter); 

  /***/
  void setKptConsist(double h);

  /** set Dbg level */
  void setDbgLevel(int lev, const char * root=0);


  /** regularization parameters. */
  void setRegulWeight(float a0, float a1, float p0, float p1, float c0delta) { regul[0].init(a0, a1, p0, p1, c0delta); regul[1].init(a0, a1, p0, p1, c0delta); }
  
  /** regularization parameters. */
  void setDySigma(float sig) { dySig = sig; }

  /** apply the last transformation to an image. 
   * circle outliers if present
   * */ 
  void apply(const cv::Mat & img, cv::Mat & out, int interpolation=cv::INTER_LINEAR) const;
  
protected :
  
  /* ----------------------------------  */
  /* --       Internal Types     ------  */
  /* ----------------------------------  */
  
  /** camera and patient motion */
  struct TCam
    { double edgecut;
      cv::Mat projMat;
      virtual void init();
      virtual void addSample(double x, double y, double t1p, double t2p, double w, bool fitInverse);
      virtual void addSample(double x, double y, double t1p, double t2p, double w)=0;
      virtual void fit() = 0;
      virtual int nCol() = 0;
//      double weight(double edgeStrength) const;
    } * tcamP;

  // FIXME : does not work with TCamID
  struct TCamId : public TCam
    { void addSample(double , double , double , double , double ) {};
      void fit() {};
      int nCol() { return (0); }
    } tcamId;

  struct TCamTrans : public TCam
    { double t1, t2, sw;
      void init();
      void addSample(double x, double y, double t1p, double t2p, double w);
      void fit();
      int nCol() { return (1); }
    } tcamTrans;

  struct TCamAff : public TCam
    { cv::Mat MW2M, MWt1, MWt2;
      void init();
      void addSample(double x, double y, double t1p, double t2p, double w);
      void fit();
      int nCol() { return (3); }
    } tcamAff;

  // FIXME : for the fit
  struct TCamProj : public TCamAff
    { int nCol() {return (6);}
    } tcamPersp;

//  struct TCamProj
//    { void addSample(double x, double y, double t1p, double t2p, double w);
//      void fit();
//    } tcamPersp;


  /***/
  struct IRLS
    { double sig0;
      int niter;
    } irls;

  /** Data about keypoint. */
  struct PtsData
    { bool  outlier;
//      bool  outlierKC;
      float weightKC;
      float gx, gy;
    };

  /* ----------------------------------  */
  /* --       Member variables   ------  */
  /* ----------------------------------  */
  /** frame0 edge map.  */
  cv::Mat edge0;

  /** mean images of the flow. */
  cv::Mat mean[2];

//  /** projection coefficient vector of the samples. */
//  std::vector<cv::Mat> proj1, proj2; 

  /** values of mean images at pts0. */
  VecPts meanpts0;

  /** number of pca component. */
  int npca;

  /** training transformations. */
  std::vector<cv::Mat> samples[2];
  
  /** trained basis images. */
  std::vector<cv::Mat> basis[2];
  
  /** trained basis images (as 2 multi channel images). */
  cv::Mat mcBasis[2];
      
//  bool testcvtransform;
//  cv::Mat mcBasis2[2];
//  cv::Mat mcBasis3;

  /** training images. */
  std::vector<cv::Mat> trainImg;


  bool write_Debug_Info;


//  cv::Mat xImg, yImg;

  /** data about keypoints. */
  std::vector<PtsData> ptsData;
  
  /** Linear system. */
  struct LinSys
    { bool compose;
      bool compbycomp;
      bool persp;
      int npca;
      int npts;
      int nresol;

      /** resolution matrices to find the registration coefficients. */
      cv::Mat resolmat[2];
      cv::Mat pinvrmat[2];

      /** right precond. */
      cv::Mat rprec[2];
      
      /***/
      cv::Mat rhs[2];

      /** alloc internal struct. */
      void init(int npts_, int npca_, bool compbycomp_, bool compose_, bool persp_, int tcCol);

      /** access half submatrix part of mat or full matrix.
       * idx=1 : bottom submat
       * idx=0,  compbycomp top submat
       * idx=0, !compbycomp full mat
       * @param idx 0 for half top, 1 for half bottom
       * */
      inline cv::Mat submat(const cv::Mat & mat, int idx)
        { int npts = mat.rows / 2;
          return ( (compbycomp||idx==1) ? mat(cv::Rect(0,idx*npts,mat.cols,npts)) : mat ); 
        }

    } linsys; 

//  /** resolution matrices to find the registration coefficients. */
//  cv::Mat resolmat[2];
//  cv::Mat pinvrmat[2];

//  /** access half submatrix part of mat or full matrix.
//   * idx=1 : bottom submat
//   * idx=0,  compbycomp top submat
//   * idx=0, !compbycomp full mat
//   * @param idx 0 for half top, 1 for half bottom
//   * */
//  inline cv::Mat submat(const cv::Mat & mat, int idx)
//    { int npts = mat.rows / 2;
//      return ( (compbycomp||idx==1) ? mat(cv::Rect(0,idx*npts,mat.cols,npts)) : mat ); 
//    }

//  /** right precond. */
//  cv::Mat rprec[2];
  
  /** struct to handle composition model. */
  struct TCompParams 
    { bool compose;
      int icurr;
      bool empty;
      cv::Mat p[2];
      std::vector< cv::Mat > ls;
      std::vector< VecPts  > ptsFit;
//      std::vector< cv::Mat > defs;
//      std::vector< cv::Mat > tFit;

//      std::vector< cv::Mat > d_ls;
//      std::vector< cv::Mat > d_defs;
////      std::vector< cv::Mat > d_tFit;

      void init(const VecPts & pts);
      
//      void incr() { icurr = (icurr+2)%ls.size(); std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " <<  std::endl;}
      void incr() { icurr = (icurr+2)%ls.size(); empty=false;}

      /** 0: curr, 1: prev, ...*/
      cv::Mat * get      (int i) { return ( &(ls    [(icurr-2*i)%ls.size()])); }
      VecPts &  getPtsFit(int i) { return (   ptsFit[(icurr/2-i)%ptsFit.size()]) ; }
//      cv::Mat * getDefs(int i) { return ( &(defs[(icurr-2*i)%ls.size()])); }
//      cv::Mat * getTFit(int i) { return ( &(tFit[(icurr-2*i)%ls.size()])); }
      
      /** extract the affine part from the parameter vector. */
      void getAffineAb(int i, float * A, float * b, bool compbycomp);
      
      /** extract the homography/lambda from the parameter vector. */
      void getHomo(int i, cv::Mat & U, cv::Mat & lambda, int npca, int ncols, int nrows, double tickonovReg=0.0);
      

//      cv::Mat * getd    (int i) { return ( &(d_ls  [(icurr-2*i)%ls.size()])); }
//      cv::Mat * getdDefs(int i) { return ( &(d_defs[(icurr-2*i)%ls.size()])); }
////      cv::Mat * getdTFit(int i) { return ( &(d_tFit[(icurr-2*i)%ls.size()])); }
      void printL()
        { 
          std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
          for (unsigned int i=0;i<ls.size();i++)
          {
              std::cout <<"ls["<<i<<"] size "<< ls[i].rows<<" "<<ls[i].cols<<std::endl;
              if(!ls[i].empty())
                  std::cout << ls[i].t()<<std::endl;
          }

//          for (unsigned int i=0;i<ls.size();i+=2)
//            {
//              std::cout << ls[i].t() << " *** " << ls[i+1].t() << std::endl;
//            }
        }

      std::string getL()
      {
            std::string v;
            for (unsigned int i=0;i<ls.size();i++)
            {
                v+="ls["+std::to_string(i)+"] size "+std::to_string(ls[i].rows)+" "+std::to_string(ls[i].cols)+"\n";
                if(!ls[i].empty())
                {
                    for(int r=0;r<ls[i].rows;r++)
                        v+=std::to_string(ls[i].at<float>(0,r))+" ";
                    v+="\n";
                }
            }
            return v;
      }

    } tParams;
  

  class Regul 
    { 
      float affCoeff[2];
      float pcaCoeff[2];
      float a[2];
      float p[2];
      float sa,sp;
      bool useconst;

      /** || L (x-x0) || < delta. 
       * for pca variables. 
       * */
      cv::Mat c0_tLL;
      cv::Mat c0_tLx0;
      cv::Mat c0_x0;

      public:
      float c0_delta;
      void init(float a0, float a1, float p0, float p1, float c0delta);
      void scaleP0(float sp0);
      
      /** init metric matrix for ||L(lpca-lpca_0)||<delta constraint.
       * store tLL and tLLx0 from lmin and lmax 
       * */
      void init(const cv::Mat & lmin, cv::Mat & lmax);

      /** ||L(lpca-lpca_0)||^2 / npca. */
      double c0Const(const cv::Mat & l) const;

      /***/
//      bool useConst() const {return (!c0_x0.empty());}
      bool useConst() const {return (useconst);}

      /** tMM * x0 */
      cv::Mat tMMx0(const cv::Mat & tMM) const { return (tMM*c0_x0); }

      void regulSystem(const cv::Mat & tMM, const cv::Mat & tMrhs, const float * prev1, cv::Mat & tMMreg, cv::Mat & tMrhsreg) const;
      
      /** one Newton iteration to solve the constrained least square pb.
       * cf: More & Sorensen 1983, Computing a trust region step.
       * mu: input current mu, output next mu
       * */
      float newtonIter4c0(const cv::Mat & tMM, const cv::Mat & tMrhs, const cv::Mat & tMMx0, cv::Mat & l, float & mu) const;

    } regul[2];

  /** build comp linear system.
   * fill resolmat and rhs.
   * */
//  void buildCompLinSys(cv::Mat * rhs);
  void buildCompLinSys();

  /** build comp linear system.
   * fill resolmat and rhs.
   * */
  void buildHomoCompLinSys();
  
  /** solve comp linear system.
   * */
  void solveCompLinSys(const cv::Mat & weight);
//  void solveCompLinSys(const cv::Mat & weight, cv::Mat * rhs);
  
  /** solve generalized eigenvalue for persp pb. */
  void solveGeneralizedEigen(const cv::Mat & tMM, cv::Mat & sol);

  /** fill resol matrix for additive model. 
   * and also tParams.p
   * */
  void buildAddResolMat();
      
  /** dump debug data to disk. */
  int dbgLevel;
  
  /** dump debug data to disk. */
  std::string dbgRoot;

  /** one PCA model for each component or not. */
  bool compbycomp;

  /** affine constraints on keypoints. */
  KptConstraint kptCons;
  bool useKptCons;

  PtsFitType ptsFitType;

  /** previous frame. */
  cv::Mat prevFrame;

  /** sigma for initial weighting with dy=y-rhsfit. */
  float dySig;
  
  /** sigma for initial weighting with LK fit error. */
  float errSig;

  /* ----------------------------------  */
  /* --       Member functions   ------  */
  /* ----------------------------------  */

  /** get the translation part from a transformation component map and remove it. */
  void fitCamMotion(const cv::Mat & transfo1, const cv::Mat & transfo2, cv::Mat & otransfo1, cv::Mat & otransfo2);
  
  /** save matrix to a file. */
  void savemat(const cv::Mat & mat, const char * root, int i=-1, std::ios_base::openmode mode=std::ios::trunc) const;

  /** */
  void edgeMap(const cv::Mat & img, cv::Mat & edge, double alpha) const;

  /** train the model using the samples. */
  void train();

  /** compute model coefficients from current pts position. 
   * */
  void computeDecompCoeff(cv::Mat & weight);

  /** compute model coefficients from current pts position. 
   * T = A x + b + d(x)
   * */
  void computeDecompCoeffAdd(cv::Mat & weight);

  /** estimate model coeff. 
   * T = A (x+d(x)) + b
   * */
  void computeDecompCoeffComp(cv::Mat & weight, cv::Mat * residual=0);

  /** compute transformation map from model coefficients. */
  void computeTransfo(const cv::Mat * l);

  /** initialise the weigth for weighted fit. */
  void initWeight(cv::Mat & w, const cv::Mat & img);

  /** shrinkage operator. */
  template <typename T> inline T shrink(T x, T k) const 
    { if (x>k)  return (x-k);
      if (x<-k) return (x+k);
      return (0);
    }

  /** general Reeves SBS algo for observable selection. */
  void reevesSBS(const cv::Mat & mat, int k, std::vector<bool> & keep) const;

  /** Reeves SBS algo for observable selection for SparsePCA.
   * keypoints and not single matrix rows are removed,
   * rows are removed 2 by two.
   * */
  void reevesSBS2(const cv::Mat & mat, int k, std::vector<bool> & keep) const;

  /** estimate brain vs non brain mask using motion segmentation. */
  void motionSeg() const;

  /** res = alpha A*x+ beta b.   */
  void gemm_omp(const cv::Mat & A, const cv::Mat & x, double alpha, const cv::Mat & bm, double beta, cv::Mat & res);

  /** select bet keypoints. */
  void keypointSelection();

};

}

#endif

