#include <Reg.h>

#include <iostream>

using namespace cv;
using namespace omigod;


Reg::Reg ()
{

}

Reg::~Reg()
{

}



void Reg::setParam(int levels_, int winsize_, int maxiter_)
{ levels  = levels_,
  winsize = winsize_;
  maxiter = maxiter_;
}
 


void Reg::apply(const cv::Mat & img, cv::Mat & out, int interpolation) const
{
//   int interpolation = INTER_LINEAR;

//    INTER_NEAREST - a nearest-neighbor interpolation
//    INTER_LINEAR - a bilinear interpolation (used by default)
//    INTER_AREA - resampling using pixel area relation. It may be a preferred method for image decimation, as it gives moire’-free results. But when the image is zoomed, it is similar to the INTER_NEAREST method.
//    INTER_CUBIC - a bicubic interpolation over 4x4 pixel neighborhood
//    INTER_LANCZOS4 - a Lanczos interpolation over 8x8 pixel neighborhood

//  cv::Mat mask1 = cv::Mat(transfo1 != transfo1);
//  cv::Mat mask2 = cv::Mat(transfo2 != transfo2);
//  std::cout <<"dbg : "<<__FILE__<<" : "<<__FUNCTION__<<"  "<< __LINE__<< " " << mean(mask1) << " " << mean(mask2) << std::endl;
//    int borderMode    = BORDER_CONSTANT;
//    const Scalar& borderValue=Scalar();

    remap(img, out, transfo1, transfo2, interpolation, BORDER_CONSTANT, Scalar(0));


//BORDER_CONSTANT 	iiiiii|abcdefgh|iiiiiii with some specified i
//BORDER_REPLICATE 	aaaaaa|abcdefgh|hhhhhhh
//BORDER_REFLECT 	fedcba|abcdefgh|hgfedcb
//BORDER_WRAP 	        cdefgh|abcdefgh|abcdefg
//BORDER_REFLECT_101 	gfedcb|abcdefgh|gfedcba
//BORDER_TRANSPARENT 	uvwxyz|absdefgh|ijklmno
//BORDER_REFLECT101 	same as BORDER_REFLECT_101
//BORDER_DEFAULT 	same as BORDER_REFLECT_101
//BORDER_ISOLATED 	do not look outside of ROI 

}
 




void Reg::setFrame0(const cv::Mat & img)
{
  transfo1.create(img.rows, img.cols, CV_32FC1);
  transfo2.create(img.rows, img.cols, CV_32FC1);
}
 




const cv::Mat & Reg::getTransfo1() const
{ return (transfo1);
}
 




const cv::Mat & Reg::getTransfo2() const
{ return (transfo2);
}
 

