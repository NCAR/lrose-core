#ifndef __CSATNAV_GRID_H__
#define __CSATNAV_GRID_H__

#include "bool.h"
#include "projection.h"

struct texVertex {	  // texture vertex
  float   vert_xyz[3];    // vertex co-ordinates
  float   tex_xy[2];	  // corresponding texture co-ordinates
  public:
  texVertex(float vertX = 0, float vertY = 0, float vertZ = 0, 
	    float Tex_X = 0, float Tex_Y = 0);
  void    set(float vertX, float vertY, float vertZ, 
	      float Tex_X, float Tex_Y);
  void    set(texVertex *copyVert);
  void    get(float &vertX, float &vertY, float &vertZ, 
	      float &Tex_X, float &Tex_Y);
  bool	  IsSame(texVertex *vert);
};

class projGrid {
 public:
  rpProjection    Projection;		// Type of projection
  float	          boundBox[2][3];	// bounding box of vertexes, min x,y,z max x,y,z
  int		  dim_x, dim_y;	// x and y array dimensions
  texVertex       *vert_array;	// actual projection vertex vs texture data
  // NOTE points stored as a [y][x] array, ie x varies fastest
  projGrid(rpProjection Proj, int Dim_X = 0, int Dim_Y = 0);
  projGrid(projGrid *copygrid);
  ~projGrid();
  void            RedefineGrid(rpProjection Proj, int Dim_X = 0, int Dim_Y = 0);
  void            Copy(projGrid *copygrid);
  void            GetBoundBox();			// traverse the vert_array to find bounding box
  texVertex*	  getTextVert(int x, int y);	// return pointer to vertex
  bool	          IsSame(projGrid *copygrid);
};

#endif	// __CSATNAV_GRID_H
