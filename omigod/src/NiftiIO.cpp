#include <NiftiIO.h>
#include <iostream>
#include <cstdio>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <iostream>

using namespace cv;
using namespace omigod;
using namespace std;



NiftiIO::NiftiIO ()
{				//fslio = FslInit();
  lastNii.sform_code = 0;
  lastNii.qform_code = 1;
  
  for (int i=0;i<4;i++)
    { for (int j=0;j<4;j++)
        { lastNii.rigidmat.m[i][j] = 0.0;
            lastNii.stdmat.m[i][j] = 0.0;
        }
      lastNii.rigidmat.m[i][i] = 1.0;
        lastNii.stdmat.m[i][i] = 1.0;
    }
}

NiftiIO::~NiftiIO ()
{
}




cv::Mat NiftiIO::loadConvert (const char *filename, double *ps, int returntype, int suffix, int volnum)
{
  cv::Mat mat = load (filename, ps, suffix, volnum);
  Mat mat_f;
  mat.convertTo (mat_f, returntype);
  return (mat_f);
}

cv::Mat NiftiIO::loadConvert (const char *filename, double *ps, int returntype, double imin, double imax, int suffix, int volnum)
{
  cv::Mat mat = load (filename, ps, suffix, volnum);

  // convert to return type
  double min, max;
  if (mat.dims == 3)
    {
      Mat mat2d (mat.size[2], mat.size[0] * mat.size[1], mat.type (), mat.data);
      cv::minMaxLoc (mat2d, &min, &max);
    }
  else
    {
      cv::minMaxLoc (mat, &min, &max);
    }

  Mat mat_f;
  double alpha = (imax - imin) / (max - min);
  double beta = (imin * max - imax * min) / (max - min);
  mat.convertTo (mat_f, returntype, alpha, beta);
  return (mat_f);

}

cv::Mat NiftiIO::load (const char *filename, double *ps, int suffix, int volnum)
{
  // add suffix
  std::vector<char> tab(strlen(filename) + 10);
  char *fullroot = tab.data();


  // char fullroot[strlen (filename) + 10];
  if (suffix >= 0)
    sprintf (fullroot, "%s_%06d", filename, suffix);
  else
    sprintf (fullroot, "%s", filename);

  std::vector<char> fsltab(strlen(fullroot));
  char *fslfilename = fsltab.data();

  // char fslfilename[strlen (fullroot)];
  strcpy (fslfilename, fullroot);
  /** open nifti dataset */
  //char fslfilename[strlen(filename)];
  //strcpy(fslfilename,filename);

  FSLIO * fslio = FslInit ();
  fslio = FslOpen (fslfilename, "rb");
  if (fslio == NULL)
    {
      fprintf (stderr, "\nError, could not read header info for %s.\n", fslfilename);
      exit (1);
    }
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
//  void *buffer = FslReadAllVolumes (fslio, fslfilename);

  void *buffer = 0;
  if (volnum<0)
    { buffer = FslReadAllVolumes (fslio, fslfilename);
    }
  else
    { int npixpervol = fslio->niftiptr->nx*fslio->niftiptr->ny*fslio->niftiptr->nz;
      buffer = malloc(16*npixpervol);
      int ret = FslSeekVolume(fslio, volnum);
      int nv = FslReadVolumes(fslio, buffer, 1);
      if (ret!=0||nv!=1)
        { fprintf (stderr, "\nerror opening and reading %s.\n", filename);
          exit (1);
        }
    }
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
  if (buffer == NULL)
    { fprintf (stderr, "\nerror opening and reading %s.\n", filename);
      exit (1);
    }
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 

  //fslio = FslReadHeader ( fslfilename); 


  int s[4];
  lastNii.nx = s[1] = fslio->niftiptr->nx;
  lastNii.ny = s[0] = fslio->niftiptr->ny;
  lastNii.nz = s[2] = fslio->niftiptr->nz;
  lastNii.nt = fslio->niftiptr->nt;
               s[3] = (volnum<0?fslio->niftiptr->nt:1);
//  int npix = s[0] * s[1] * s[2] * s[3];
  int npix = s[0] * s[1] * s[2] * s[3];
  //for (int i=0;i<4;i++) cout << "s values: " << s[i] << endl;
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << s[0] << std::endl; 
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << s[1] << std::endl; 
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << s[2] << std::endl; 
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << s[3] << std::endl; 

//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << lastNii.nx << std::endl; 
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << lastNii.ny << std::endl; 
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << lastNii.nz << std::endl; 
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << lastNii.nt << std::endl; 


  ps[1] = fslio->niftiptr->dx;
  ps[0] = fslio->niftiptr->dy;
  ps[2] = fslio->niftiptr->dz;
  ps[3] = fslio->niftiptr->dt;
  Mat mat;
  int nbdim = 2;		// (s[2] >1 || s[3] >1) ? 4 : 2;
  if (volnum<0)
    { nbdim = (s[2] > 1) ? 3 : 2;
    }
  else
    { if (s[3] > 1)
        { nbdim = 4;
        }
      else
        { if (s[2] > 1) nbdim = 3;
        }
    }
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << nbdim << std::endl; 

  //double slope = fslio->niftiptr->scl_slope;
  //double inter = fslio->niftiptr->scl_inter;
  /* if (fslio->niftiptr->scl_slope == 0)
   *   { slope = 1.0;
   *     inter = 0.0;
   }*/
  // cout << fslio->niftiptr->datatype << endl;

  switch (fslio->niftiptr->datatype)
    {
      case NIFTI_TYPE_UINT8:
	{ mat.create (nbdim, s, CV_8U);
	  memcpy (mat.ptr < unsigned char >(0), buffer, npix * sizeof (unsigned char));
	} break;
      case NIFTI_TYPE_INT8:
	{ mat.create (nbdim, s, CV_8S);
	  memcpy (mat.ptr < char >(0), buffer, npix * sizeof (char));
	} break;
      case NIFTI_TYPE_UINT16:
	{ mat.create (nbdim, s, CV_16U);
	  memcpy (mat.ptr < unsigned short >(0), buffer, npix * sizeof (unsigned short));
	} break;
      case NIFTI_TYPE_INT16:
	{ mat.create (nbdim, s, CV_16S);
	  memcpy (mat.ptr < short >(0), buffer, npix * sizeof (short));
	} break;
      case NIFTI_TYPE_INT32:
	{ mat.create (nbdim, s, CV_32S);
	  memcpy (mat.ptr < int >(0), buffer, npix * sizeof (int));
	} break;
      case NIFTI_TYPE_FLOAT32:
	{ mat.create (nbdim, s, CV_32F);
	  memcpy (mat.ptr < float >(0), buffer, npix * sizeof (float));
	} break;
      case NIFTI_TYPE_FLOAT64:
	{ mat.create (nbdim, s, CV_64F);
	  memcpy (mat.ptr < double >(0), buffer, npix * sizeof (double));
	} break;
      case NIFTI_TYPE_FLOAT128:
      case NIFTI_TYPE_COMPLEX128:
      case NIFTI_TYPE_COMPLEX256:
      case NIFTI_TYPE_COMPLEX64:
      default:
	fprintf (stderr, "\nWarning, cannot support %d yet.\n", fslio->niftiptr->datatype);
	exit (1);
	break;
    }
//    std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << mat.rows   << std::endl; 
//    std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << mat.cols   << std::endl; 
//    std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << mat.size() << std::endl; 
  lastNii.sform_code = FslGetStdXform (fslio, &(lastNii.stdmat));
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      { lastNii.stdmat.m[i][j] = fslio->niftiptr->sto_xyz.m[i][j];
      }
  lastNii.qform_code = FslGetRigidXform (fslio, &(lastNii.rigidmat));
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      { lastNii.rigidmat.m[i][j] = fslio->niftiptr->qto_xyz.m[i][j];
      }
  FslGetCalMinMax (fslio, &(lastNii.min), &(lastNii.max));
  //FslGetAuxFile(fslio,&aux_file);
  //FslGetMMCoord(stdmat, vx, vy, vz, &mx, &my, &mz);
  //FslGetTimeUnits(fslio, &units);

  FslClose (fslio);

  if (volnum>=0) free(buffer);
//    std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << mat.rows   << std::endl; 
//    std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << mat.cols   << std::endl; 
//    std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << mat.size() << std::endl; 

  return mat;

}

void NiftiIO::save_old (const cv::Mat & mat, const char *rootname, int suffix)
{
  //  printf("mv -f %s_1.img __correct_motion_tmpfile\n",svgtransbinname.c_str());
  //  printf("fslcreatehd %d %d 1 %d 1 1 1 %f 0 0 0 16 %s_1\n",xs,ys,count-2,fps,svgtransbinname.c_str());
  //  printf("mv -f __correct_motion_tmpfile %s_1.img\n",svgtransbinname.c_str());


  int datatype = 0;
  switch (mat.depth ())
    {
      case CV_8U:
	datatype = 2;
	break;
      case CV_8S:
	datatype = 256;
	break;
      case CV_16U:
	datatype = 512;
	break;
      case CV_16S:
	datatype = 4;
	break;
      case CV_32S:
	datatype = 8;
	break;
      case CV_32F:
	datatype = 16;
	break;
      case CV_64F:
	datatype = 64;
	break;
      default:
	break;
    }

  // add suffix
  std::vector<char> tab(strlen(rootname) + 10);
  char *fullroot = tab.data();
  // char fullroot[strlen (rootname) + 10];
  if (suffix >= 0)
    sprintf (fullroot, "%s_%06d", rootname, suffix);
  else
    sprintf (fullroot, "%s", rootname);


  // create header      
  std::vector<char> cmdtab(strlen (fullroot) + 128);
  char *command = cmdtab.data();
  // char command[strlen (fullroot) + 128];
  int nt = 1;
  double fps = 1;
  sprintf (command, "fslcreatehd %d %d 1 %d 1 1 1 %f 0 0 0 %d %s\n", mat.cols, mat.rows, nt, fps, datatype, fullroot);
  //  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << std::endl; 
  //  cout << command << endl;
  system (command);

  // img file
  FILE *f = fopen ((string (fullroot) + ".img").c_str (), "w");
  switch (mat.depth ())
    {
      case CV_8U:
	fwrite (mat.ptr < unsigned char >(0), sizeof (unsigned char), mat.rows * mat.cols, f);
	break;
      case CV_8S:
	fwrite (mat.ptr < char >(0), sizeof (char), mat.rows * mat.cols, f);
	break;
      case CV_16U:
	fwrite (mat.ptr < unsigned short >(0), sizeof (unsigned short), mat.rows * mat.cols, f);
	break;
      case CV_16S:
	fwrite (mat.ptr < short >(0), sizeof (short), mat.rows * mat.cols, f);
	break;
      case CV_32S:
	fwrite (mat.ptr < int >(0), sizeof (int), mat.rows * mat.cols, f);
	break;
      case CV_32F:
	fwrite (mat.ptr < float >(0), sizeof (float), mat.rows * mat.cols, f);
	break;
      case CV_64F:
	fwrite (mat.ptr < double >(0), sizeof (double), mat.rows * mat.cols, f);
	break;
      default:
	break;
    }

  fclose (f);

}

void NiftiIO::save (const cv::Mat & mat, const char *rootname, const double *ps, int suffix) const
{
  // add suffix
    std::vector<char> tab(strlen(rootname) + 10);
    char *fullroot = tab.data();
  // char fullroot[strlen (rootname) + 10];
  if (suffix >= 0)
    sprintf (fullroot, "%s_%06d", rootname, suffix);
  else
    sprintf (fullroot, "%s", rootname);

  // open output file
  FSLIO *fsliow = FslOpen (fullroot, "wb");
  if (fsliow == NULL)
    {
      fprintf (stderr, "\nError, could not write header info for %s.\n", fullroot);
      exit (1);
    }

  // find fsl datatype
  int datatype = 0;
  switch (mat.depth ())
    {
      case CV_8U:
	datatype = 2;
	break;
      case CV_8S:
	datatype = 256;
	break;
      case CV_16U:
	datatype = 512;
	break;
      case CV_16S:
	datatype = 4;
	break;
      case CV_32S:
	datatype = 8;
	break;
      case CV_32F:
	datatype = 16;
	break;
      case CV_64F:
	datatype = 64;
	break;
      default:
	break;
    }


  // set fsliow values
  FslSetStdXform (fsliow, lastNii.sform_code, lastNii.stdmat);
  FslSetRigidXform (fsliow, lastNii.qform_code, lastNii.rigidmat);
  //FslSetAuxFile(fsliow,&aux_file);
  //FslSetTimeUnits(fsliow, &units);
  FslSetCalMinMax (fsliow, lastNii.min, lastNii.max);
  FslSetDim (fsliow, mat.size[1], mat.size[0], mat.dims > 2 ? mat.size[2] : 1, mat.dims > 3 ? mat.size[3] : 1);
  if (ps!=0) FslSetVoxDim (fsliow, ps[1], ps[0], ps[2], ps[3]);
  FslSetDimensionality (fsliow, mat.dims);
  FslSetDataType (fsliow, datatype);

  FslWriteHeader (fsliow);

  // img file 
  FslWriteAllVolumes (fsliow, mat.ptr<void>(0));
  FslClose (fsliow);
}



/*void NiftiIO::save(const cv::Mat & mat, const char *rootname, const double * ps, int suffix)
{
  char fullroot[strlen(rootname)+10];
  if (suffix>=0) sprintf(fullroot,"%s_%06d",rootname,suffix); 
  else           sprintf(fullroot,"%s",     rootname);
 
  FSLIO *fsliowr;

  fsliowr = FslOpen(fullroot, "wb");
  if (fsliowr == NULL) {
    fprintf(stderr, "\nError, could not write header info for %s.\n",fullroot);
    //exit(1);
  }
  FslCloneHeader(fsliowr, fslio);

  int datatype = 0;
  switch (mat.depth())
    { case CV_8U : datatype = 2;            break;
      case CV_8S : datatype = 256;          break;
      case CV_16U: datatype = 512;          break;
      case CV_16S: datatype = 4;            break;
      case CV_32S: datatype = 8;            break;
      case CV_32F: datatype = 16;           break;
      case CV_64F: datatype = 64;           break;
      default: break;
    }  
  

  FslSetDim(fsliowr, mat.size[1], mat.size[0], mat.dims>2 ? mat.size[2] : 1, mat.dims>3 ? mat.size[3] : 1);
  FslSetVoxDim(fsliowr, ps[1], ps[0], ps[2], ps[3]);
  FslSetDimensionality(fsliowr, mat.dims);
  FslSetDataType(fsliowr, datatype );

  FslWriteHeader(fsliowr);
  FslWriteAllVolumes (fsliowr,mat.ptr<void>(0));
  //FslWriteVolumes(fsliowr, cbuff, 1);
  FslClose(fsliowr);
  FslClose(fslio);
}*/

int NiftiIO::getNbVol() const
{ return (lastNii.nt);
}
 

