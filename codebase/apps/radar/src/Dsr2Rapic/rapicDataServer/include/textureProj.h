#ifndef __TEXTURE_PROJ_H__
#define __TEXTURE_PROJ_H__

#include <vector>
using namespace std;
#include "bool.h"
#include "projection.h"
#include "draw.h"

struct texVertex {	  // texture vertex
  float   vert_xyz[3];    // vertex co-ordinates
  float   tex_xy[2];	  // corresponding texture co-ordinates
  public:
  texVertex(float vertX = 0, float vertY = 0, float vertZ = 0, 
	    float Tex_X = 0, float Tex_Y = 0);
  void    set(float vertX, float vertY, float vertZ, 
	      float Tex_X, float Tex_Y);
  void    set(texVertex *copyVert);
  void    set(texVertex& copyVert);
  void    get(float &vertX, float &vertY, float &vertZ, 
	      float &Tex_X, float &Tex_Y);
  bool	  IsSame(texVertex *vert);
};

class projGrid {
 public:
  rpProjection    Projection;		// Type of projection
  boundingBox	  boundBox;	// bounding box of vertexes, min x,y,z max x,y,z
  // NOTE points stored as a [y][x] array, ie x varies fastest
  projGrid(rpProjection Proj, int Dim_X = 0, int Dim_Y = 0);
  projGrid(projGrid *copygrid);
  projGrid(projGrid *copygrid, rpProjection newproj);
  ~projGrid();
  void            RedefineGrid(rpProjection Proj, int Dim_X = 0, int Dim_Y = 0);
  void            ProjectFromGrid(projGrid *projgrid, rpProjection newproj); // copy i/p grid into this with specified proj
  void            Copy(projGrid *copygrid);
  projGrid*       CreateProjGrid(rpProjection newproj); // make a copy of this grid with new projection
  void            GetBoundBox();			// traverse the vert_array to find bounding box
  texVertex*	  getTextVert(int x, int y);	// return pointer to vertex
  void	          setTextVert(int x, int y,
			      texVertex *vert);	// set vertex
  void	          setTextVert(int x, int y,
			      texVertex& vert);	// set vertex
  void	          setTextVert(int x, int y,
			      float vertX, float vertY, float vertZ, 
			      float Tex_X, float Tex_Y);	// set vertex
  bool	          IsSame(projGrid *copygrid);
  int             getDim_x() { return dim_x; };
  int             getDim_y() { return dim_y; };
 private:
  texVertex       *vert_array;	// actual projection vertex vs texture data
  int		  dim_x, dim_y;	// x and y array dimensions
  bool            ProjectLLtoXY(rpProjection proj, float Lat, float Long, float Height, 
				float *projX, float *projY, float *projZ);
  bool            ProjectLLVertToXYVert(rpProjection proj, texVertex llvert, texVertex& projvert);
  bool            ProjectXYtoLL(rpProjection proj, float *Lat, float *Long, float *Height, 
				float projX, float projY, float projZ);
  bool            ProjectXYtoLL(rpProjection proj, texVertex& llvert, texVertex projvert);
};

// The following classes are more specialised - based on lat/long texture vertices 
// which can be projected to the desired render projection

class llProjTexVertex {     // lat/long and projected texture vertex
  friend class llProjGrid;
 public:
  rpProjection Proj;
  llProjTexVertex(float lat = 0, float lng = 0, float ht = 0,
		    float Tex_X = 0, float Tex_Y = 0, rpProjection proj = projUndefined);
  void    set(float lat, float lng, float ht, 
	      float Tex_X, float Tex_Y);
  void    copy(llProjTexVertex *copyVert); // copy copyvert into this
  void    copy(llProjTexVertex& copyVert); // copy copyvert into this
  void    getLL(float &lat, float &lng, float &ht, 
	      float &Tex_X, float &Tex_Y);
  void    getProjXYZ(float &x, float &y, float &z, 
	      float &Tex_X, float &Tex_Y);
  void    getProjTexVertex(texVertex& texvert);
  bool    project(rpProjection newproj = projUndefined);  // do the projection
  bool	  IsSame(llProjTexVertex *vert);
  float*  getLLVertPtr() { return ll_vert; };
  float   getLLVertX() { return ll_vert[0]; };
  float   getLLVertY() { return ll_vert[1]; };
  float   getLLVertZ() { return ll_vert[2]; };
  float*  getProjVertPtr() { return proj_vert; };
  float   getProjVertX() { return proj_vert[0]; };
  float   getProjVertY() { return proj_vert[1]; };
  float   getProjVertZ() { return proj_vert[2]; };
  float*  getTexXYPtr() { return tex_xy; };
  float   getTexX() { return tex_xy[0]; };
  float   getTexY() { return tex_xy[1]; };
 private:
  float   ll_vert[3];    // lat/long vertex co-ordinates
  float   proj_vert[3];  // projected vertex co-ordinates
  float   tex_xy[2];	 // corresponding texture co-ordinates
};

class llProjGrid {
 public:
  rpProjection    Projection;	    // Type of projection
  boundingBox     projBoundBox;	    // bounding box of projected vertices
  boundingBox     llBoundBox;	    // bounding box of lat/long vertices
  // NOTE points stored as a [y][x] array, ie x varies fastest
  llProjGrid(int Dim_X = 0, int Dim_Y = 0, rpProjection Proj = projUndefined);
  llProjGrid(llProjGrid *copygrid);
  llProjGrid(llProjGrid *copygrid, rpProjection newproj);
  ~llProjGrid();
  void            redefineGrid(int Dim_X, int Dim_Y, rpProjection Proj = projUndefined);
  void            copy(llProjGrid *copygrid, rpProjection Proj = projUndefined);
  void            getBoundBox();		// traverse the vert_array to find bounding box
  bool	          getTexVert(int x, int y, llProjTexVertex& texvert);	// return pointer to vertex
  llProjTexVertex* getTexVert(int x, int y);	// return pointer to vertex
  void	          setTexVert(int x, int y,
			     llProjTexVertex *vert);	// set vertex
  void	          setTexVert(int x, int y,
			     llProjTexVertex& vert);	// set vertex
  void	          setTexVert(int x, int y,
			     float lat, float lng, float ht, 
			     float Tex_X, float Tex_Y);	// set vertex
  int             getDim_x() { return dim_x; };
  int             getDim_y() { return dim_y; };
  void            project(rpProjection newproj = projUndefined);
  bool            coordsOK(int x, int y) 
    { 
      return (x >= 0) && (x < dim_x) && (y >= 0) && (y < dim_y);
    };
  vector<llProjTexVertex> vert_array;	// actual projection vertex vs texture data
 private:
  int		  dim_x, dim_y;	// x and y array dimensions
};


#endif	// __TEXTURE_PROJ_H__
