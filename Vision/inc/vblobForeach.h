/**
 * @file vblobForeach.h
 *
 * @brief Set of functions used to apply a function line by line inside a contour.
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef VBLOBFOREACH_H
#define VBLOBFOREACH_H

#include "opencv2/core/core.hpp"
typedef struct
{
    int y;
    int x1;
    int x2;
}vSegment;

void foreachPixelIn(const std::vector<cv::Point> &contour, cv::Size image_size, void(processSegment(int y,int x1,int x2)));
void foreachPixelIn(const  std::vector< cv::Point>& contour,cv::Size image_size, void(processSegment(int y,int x1,int x2,void *data)),void* data);



void vExtractBlobSegments(const std::vector< cv::Point>& contour,cv::Size image_size, std::vector< vSegment> & segments);

template <typename Functor> void foreachPixelIn(const  std::vector< cv::Point>& contour,cv::Size image_size, Functor fn )
{
    std::vector< vSegment>  segments;
    vExtractBlobSegments( contour, image_size,  segments);
    for_each(segments.begin(), segments.end(),[&fn](vSegment seg  ) { fn(seg.y,seg.x1 , seg.x2 ); });
}

#endif // VBLOBFOREACH_H
