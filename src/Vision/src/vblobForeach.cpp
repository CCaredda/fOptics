#include "vblobForeach.h"
#include "opencv2/core/core.hpp"
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;




enum { XY_SHIFT = 16, XY_ONE = 1 << XY_SHIFT, DRAWING_STORAGE_BLOCK = (1<<12) - 256 };

struct PolyEdge
{
    PolyEdge() : y0(0), y1(0), x(0), dx(0), next(0) {}
    //PolyEdge(int _y0, int _y1, int _x, int _dx) : y0(_y0), y1(_y1), x(_x), dx(_dx) {}

    int y0, y1;
    int x, dx;
    PolyEdge *next;
};



struct CmpEdges
{
    bool operator ()(const PolyEdge& e1, const PolyEdge& e2)
    {
        return e1.y0 - e2.y0 ? e1.y0 < e2.y0 :
                               e1.x - e2.x ? e1.x < e2.x : e1.dx < e2.dx;
    }
};

#define ICV_HLINE( ptr, xl, xr, color, pix_size )            \
{                                                            \
    uchar* hline_ptr = (uchar*)(ptr) + (xl)*(pix_size);      \
    uchar* hline_max_ptr = (uchar*)(ptr) + (xr)*(pix_size);  \
    \
    for( ; hline_ptr <= hline_max_ptr; hline_ptr += (pix_size))\
{                                                        \
    int hline_j;                                         \
    for( hline_j = 0; hline_j < (pix_size); hline_j++ )  \
{                                                    \
    hline_ptr[hline_j] = ((uchar*)color)[hline_j];   \
    }                                                    \
    }                                                        \
    }


static void
CollectPolyEdges( const Point* v, int count, vector<PolyEdge>& edges    )
{
    int i ;
    Point pt0 = v[count-1], pt1;
    pt0.x = (pt0.x  ) << (XY_SHIFT );

    edges.reserve( edges.size() + count );

    for( i = 0; i < count; i++, pt0 = pt1 )
    {
        PolyEdge edge;
        pt1 = v[i];
        pt1.x = (pt1.x  ) << (XY_SHIFT  );
        if( pt0.y == pt1.y )
            continue;

        if( pt0.y < pt1.y )
        {
            edge.y0 = pt0.y;
            edge.y1 = pt1.y;
            edge.x = pt0.x;
        }
        else
        {
            edge.y0 = pt1.y;
            edge.y1 = pt0.y;
            edge.x = pt1.x;
        }
        edge.dx = (pt1.x - pt0.x) / (pt1.y - pt0.y);
        edges.push_back(edge);
    }
}


static void
FillEdgeCollection(  vector<PolyEdge>& edges, Size size , void(processSegment(int y,int x1,int x2)))
{
    PolyEdge tmp;
    int i, y, total = (int)edges.size();
    PolyEdge* e;
    int y_max = INT_MIN, x_max = INT_MIN, y_min = INT_MAX, x_min = INT_MAX;

    if( total < 2 )
        return;

    for( i = 0; i < total; i++ )
    {
        PolyEdge& e1 = edges[i];
        assert( e1.y0 < e1.y1 );
        y_min = std::min( y_min, e1.y0 );
        y_max = std::max( y_max, e1.y1 );
        x_min = std::min( x_min, e1.x );
        x_max = std::max( x_max, e1.x );
    }

    if( y_max < 0 || y_min >= size.height || x_max < 0 || x_min >= (size.width<<XY_SHIFT) )
        return;

    std::sort( edges.begin(), edges.end(), CmpEdges() );

    // start drawing
    tmp.y0 = INT_MAX;
    edges.push_back(tmp); // after this point we do not add
    // any elements to edges, thus we can use pointers
    i = 0;
    tmp.next = 0;
    e = &edges[i];
    y_max = MIN( y_max, size.height );

    for( y = e->y0; y < y_max; y++ )
    {
        PolyEdge *last, *prelast, *keep_prelast;
        int sort_flag = 0;
        int draw = 0;
        int clipline = y < 0;

        prelast = &tmp;
        last = tmp.next;
        while( last || e->y0 == y )
        {
            if( last && last->y1 == y )
            {
                // exclude edge if y reachs its lower point
                prelast->next = last->next;
                last = last->next;
                continue;
            }
            keep_prelast = prelast;
            if( last && (e->y0 > y || last->x < e->x) )
            {
                // go to the next edge in active list
                prelast = last;
                last = last->next;
            }
            else if( i < total )
            {
                // insert new edge into active list if y reachs its upper point
                prelast->next = e;
                e->next = last;
                prelast = e;
                e = &edges[++i];
            }
            else
                break;

            if( draw )
            {
                if( !clipline )
                {
                    // convert x's from fixed-point to image coordinates

                    int x1 = keep_prelast->x;
                    int x2 = prelast->x;

                    if( x1 > x2 )
                    {
                        int t = x1;

                        x1 = x2;
                        x2 = t;
                    }

                    x1 = (x1 + XY_ONE - 1) >> XY_SHIFT;
                    x2 = x2 >> XY_SHIFT;

                    // clip and draw the line
                    if( x1 < size.width && x2 >= 0 )
                    {
                        if( x1 < 0 )
                            x1 = 0;
                        if( x2 >= size.width )
                            x2 = size.width - 1;
                        processSegment(y,x1,x2);
                    }
                }
                keep_prelast->x += keep_prelast->dx;
                prelast->x += prelast->dx;
            }
            draw ^= 1;
        }

        // sort edges (using bubble sort)
        keep_prelast = 0;

        do
        {
            prelast = &tmp;
            last = tmp.next;

            while( last != keep_prelast && last->next != 0 )
            {
                PolyEdge *te = last->next;
                // swap edges
                if( last->x > te->x )
                {
                    prelast->next = te;
                    last->next = te->next;
                    te->next = last;
                    prelast = te;
                    sort_flag = 1;
                }
                else
                {
                    prelast = last;
                    last = te;
                }
            }
            keep_prelast = prelast;
        }
        while( sort_flag && keep_prelast != tmp.next && keep_prelast != &tmp );
    }
}


static void
FillEdgeCollection(  vector<PolyEdge>& edges, Size size , void(processSegment(int y,int x1,int x2,void * data)),void* data)
{
    PolyEdge tmp;
    int i, y, total = (int)edges.size();
    PolyEdge* e;
    int y_max = INT_MIN, x_max = INT_MIN, y_min = INT_MAX, x_min = INT_MAX;

    if( total < 2 )
        return;

    for( i = 0; i < total; i++ )
    {
        PolyEdge& e1 = edges[i];
        assert( e1.y0 < e1.y1 );
        y_min = std::min( y_min, e1.y0 );
        y_max = std::max( y_max, e1.y1 );
        x_min = std::min( x_min, e1.x );
        x_max = std::max( x_max, e1.x );
    }

    if( y_max < 0 || y_min >= size.height || x_max < 0 || x_min >= (size.width<<XY_SHIFT) )
        return;

    std::sort( edges.begin(), edges.end(), CmpEdges() );

    // start drawing
    tmp.y0 = INT_MAX;
    edges.push_back(tmp); // after this point we do not add
    // any elements to edges, thus we can use pointers
    i = 0;
    tmp.next = 0;
    e = &edges[i];
    y_max = MIN( y_max, size.height );

    for( y = e->y0; y < y_max; y++ )
    {
        PolyEdge *last, *prelast, *keep_prelast;
        int sort_flag = 0;
        int draw = 0;
        int clipline = y < 0;

        prelast = &tmp;
        last = tmp.next;
        while( last || e->y0 == y )
        {
            if( last && last->y1 == y )
            {
                // exclude edge if y reachs its lower point
                prelast->next = last->next;
                last = last->next;
                continue;
            }
            keep_prelast = prelast;
            if( last && (e->y0 > y || last->x < e->x) )
            {
                // go to the next edge in active list
                prelast = last;
                last = last->next;
            }
            else if( i < total )
            {
                // insert new edge into active list if y reachs its upper point
                prelast->next = e;
                e->next = last;
                prelast = e;
                e = &edges[++i];
            }
            else
                break;

            if( draw )
            {
                if( !clipline )
                {
                    // convert x's from fixed-point to image coordinates

                    int x1 = keep_prelast->x;
                    int x2 = prelast->x;

                    if( x1 > x2 )
                    {
                        int t = x1;

                        x1 = x2;
                        x2 = t;
                    }

                    x1 = (x1 + XY_ONE - 1) >> XY_SHIFT;
                    x2 = x2 >> XY_SHIFT;

                    // clip and draw the line
                    if( x1 < size.width && x2 >= 0 )
                    {
                        if( x1 < 0 )
                            x1 = 0;
                        if( x2 >= size.width )
                            x2 = size.width - 1;
                        processSegment(y,x1,x2,data);
                    }
                }
                keep_prelast->x += keep_prelast->dx;
                prelast->x += prelast->dx;
            }
            draw ^= 1;
        }

        // sort edges (using bubble sort)
        keep_prelast = 0;

        do
        {
            prelast = &tmp;
            last = tmp.next;

            while( last != keep_prelast && last->next != 0 )
            {
                PolyEdge *te = last->next;
                // swap edges
                if( last->x > te->x )
                {
                    prelast->next = te;
                    last->next = te->next;
                    te->next = last;
                    prelast = te;
                    sort_flag = 1;
                }
                else
                {
                    prelast = last;
                    last = te;
                }
            }
            keep_prelast = prelast;
        }
        while( sort_flag && keep_prelast != tmp.next && keep_prelast != &tmp );
    }
}



void foreachPixelIn(const   std::vector< cv::Point>& contour,cv::Size image_size, void(processSegment(int y,int x1,int x2)))
{
    vector<PolyEdge> edges;
    CollectPolyEdges( &contour[0],(int)contour.size(),edges   );
    FillEdgeCollection(   edges,image_size ,processSegment);
}


void foreachPixelIn( const  std::vector< cv::Point>& contour,cv::Size image_size, void(processSegment(int y,int x1,int x2,void *data)),void* data)
{
    vector<PolyEdge> edges;
    CollectPolyEdges( &contour[0],(int)contour.size(),edges   );
    FillEdgeCollection(   edges,image_size ,processSegment,data);
}


struct __vExtractBlobSegments{
    vector<vSegment> * v;
};

void ____vExtractBlobSegmentsProcessSegment(int y,int x1, int x2,void *data)
{
   vector<vSegment> &v=*( (__vExtractBlobSegments*)data) ->v;
   vSegment s;
   s.y=y;
   s.x1=x1;
   s.x2=x2;
   v.push_back(s);
}

void vExtractBlobSegments(const std::vector< cv::Point>& contour, cv::Size image_size, std::vector<vSegment> &segments)
{
    __vExtractBlobSegments data;
    vector<PolyEdge> edges;
    segments.clear();
    if(contour.empty())
        return;
    data.v=&segments;
    CollectPolyEdges( &contour[0],(int)contour.size(),edges   );
    FillEdgeCollection(   edges,image_size ,____vExtractBlobSegmentsProcessSegment,&data);
}
