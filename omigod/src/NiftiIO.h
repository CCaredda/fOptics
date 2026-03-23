#ifndef __NIFTIIO_
#define __NIFTIIO_

#include <cstdio>
#include <opencv2/core/core.hpp>

#include <fslio.h>


namespace omigod {

/**
 * @author Michael Sdika
 */
class NiftiIO 
{
public :
  
  /* ----------------------------------  */
  /* --       Public Types       ------  */
  /* ----------------------------------  */
  
  /* ----------------------------------  */
  /* --       Public Functions   ------  */
  /* ----------------------------------  */

  /** Constructor */
  NiftiIO ();


  /** Destructor */
  virtual ~NiftiIO ();


//  bool open(const char * name, const char * mode);

  void save_old(const cv::Mat & mat, const char * rootname, int suffix=-1);
  void save(const cv::Mat & mat, const char * rootname, const double * ps=0, int suffix=-1) const;
  void save_test(const cv::Mat & mat, const char *f1name, const double * ps, int suffix=-1);
  //void save(const cv::Mat & mat, double * ps);
 
  /** read an image. */
  cv::Mat load(const char * filename, double * ps, int suffix=-1, int volnum=-1);

  cv::Mat loadConvert(const char * filename, double * ps, int returntype, int suffix=-1, int volnum=-1);

  cv::Mat loadConvert(const char * filename, double * ps, int returntype, double imin, double imax, int suffix=-1, int volnum=-1);
  
  int getNbVol() const;
  

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
//  /** IO file. */
//  FILE * file;

  /** fslio object. */ 
  //FSLIO *fslio;
  struct LastNii 
    {
      mat44 rigidmat, stdmat;
      short sform_code, qform_code;
      int nx, ny, nz, nt;
//      char aux_file, units;
//      short orig[5], x, y, z, v, t;
//      float dx, dy, dz, xx, yy, zz, tr, vx, vy, vz, mx,my, mz, min, max;
      float                                                    min, max;
//      size_t dim;
    } lastNii;
  /* ----------------------------------  */
  /* --       Member functions   ------  */
  /* ----------------------------------  */



};

}

#endif

