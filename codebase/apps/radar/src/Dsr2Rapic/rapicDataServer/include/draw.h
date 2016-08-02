#ifndef __DRAW_H__
#define __DRAW_H__

#include "bool.h"
#include <stdio.h>
#ifdef STDCPPHEADERS
#include <list>
#include <vector>
#include <string>
#include <bitset>
using namespace std;
#else
#include <list.h>
#include <vector.h>
#include <string>
#include <bitset.h>
#endif
#include <math.h>
#include <values.h>
#ifdef OPEN_GL
#include <GL/gl.h>
#endif

#include "rdr.h"
#include "uiftovrc.h"
#include "palette.h"

class DrawText; // fwd declaration

enum DrawDataType {
  DDT_Base,		    // base type, only contains type & visible flag
  DDT_Symbol,		    // Symbol type, adds pos
  DDT_Symbol_Text, 
  DDT_Symbol_Text_List,
  DDT_Symbol_BitImage, 
  DDT_Symbol_Image, 
  DDT_Symbol_Stroked, 
  DDT_Symbol_Circle, 
  DDT_Vector, 
  DDT_VectorArea,
  DDT_Map, 
  DDT_Map_Curve,
  DDT_Wind_Barb
};

enum DrawCoordType {
  DCT_LatLongHt,      // Coords are lat/long/ht
  DCT_CartEarth,      // km X,Y,Z relative to Earth's centre (spherical earth)
  DCT_CartLLHRel,     // km X,Y,Z cart relative to lat/long/ht tangent plane (LLH context defined externally) 
  DCT_CartLLHPol,     // az,el,rng polar relative to lat/long/ht (LLH context defined externally) 
  DCT_Undefined       // 
};

class point2d {
 public:
  float x, y;
  point2d() : x(0), y(0) {};
  point2d(float setx, float sety) : x(setx), y(sety) {};
  point2d(float *setv)
    {
      x = setv[0];
      y = setv[1];
    }
  virtual ~point2d() {};
  void set(float setx, float sety) 
    {
      x = setx;
      y = sety;
    }
  void set(float *setv) 
    {
      x = setv[0];
      y = setv[1];
    }
  void setEndSegment()	// set this point as a line segment delimiter
    {
      x = MAXFLOAT;
    }
  bool isEndSegment()		// return true if is a line segment delimiter
    {
      return x == MAXFLOAT;
    }
    
  void get(float *setv) 
    {
      setv[0] = x;
      setv[1] = y;
    }
  virtual double distSquared(point2d pt)  // dX*dX+dY*dY+dZ*dZ distance
    {
      double dx = x - pt.x;
      double dy = y - pt.y;
      return (dx*dx) + (dy*dy);
    };

  virtual double dist(point2d pt)  // dX*dX+dY*dY+dZ*dZ distance
    {
      return sqrt(distSquared(pt));
    };

  virtual bool equals(point2d pt)
    {
      return ((x == pt.x) && (y == pt.y));
    };
  virtual bool nearerThan(point2d pt, float dist)
    {
      return distSquared(pt) < (dist * dist); // compare squared dist, faster
    };
};

class point3d {
 public:
  float x, y, z;
  point3d() : x(0), y(0), z(0) {};
  point3d(float setx, float sety, float setz) : x(setx), y(sety), z(setz) {};
  point3d(float *setv)
    {
      x = setv[0];
      y = setv[1];
      z = setv[2];
    };
  virtual ~point3d() {};
  virtual void set(float setx, float sety, float setz) 
    {
      x = setx;
      y = sety;
      z = setz;
    };
  virtual void set(float *setv) 
    {
      x = setv[0];
      y = setv[1];
      y = setv[2];
    };
  virtual void setEndSegment()	// set this point as a line segment delimiter
    {
      x = MAXFLOAT;
    };
  virtual bool isEndSegment()		// return true if is a line segment delimiter
    {
      return x == MAXFLOAT;
    };
    
  virtual void get(float *setv) 
    {
      setv[0] = x;
      setv[1] = y;
      setv[2] = z;
    };
  virtual void get(GLdouble *setv) 
    {
      setv[0] = x;
      setv[1] = y;
      setv[2] = z;
    };
  virtual double distSquared(point3d pt)  // dX*dX+dY*dY+dZ*dZ distance
    {
      double dx = x - pt.x;
      double dy = y - pt.y;
      double dz = z - pt.z;
      return (dx*dx) + (dy*dy) + (dz*dz);
    };

  virtual double dist(point3d pt)  // dX*dX+dY*dY+dZ*dZ distance
    {
      return sqrt(distSquared(pt));
    };

  virtual bool equals(point3d pt)
    {
      return ((x == pt.x) && (y == pt.y) && (z == pt.z));
    };
  virtual bool nearerThan(point3d pt, float dist)
    {
      return distSquared(pt) < (dist * dist); // compare squared dist, faster
    };
};

class string3d : public point3d {
 public:
  string str;
  string3d() : point3d(0,0,0) {};
  string3d(float setx, float sety, float setz, char *newstring) : 
    point3d(setx, sety, setz) 
    {
      set(newstring);
    };
  string3d(float *setv, char *newstring) : 
    point3d(setv)
    {
      set(newstring);
    };
  virtual ~string3d()
    {
    };
  virtual void set(char *newstring)
    {
      if (newstring)
	str = newstring;
    };      
  virtual void set(float *setv, char *newstring)
    {
      point3d::set(setv);
      if (newstring)
	str = newstring;
    };      
  virtual void set(float setx, float sety, float setz, char *newstring)
    {
      point3d::set(setx, sety, setz);
      if (newstring)
	str = newstring;
    };      
};

class boundingBox {
 public:
  bool valid;
  float bbMin[3], bbMax[3]; // bounding box min and max values
  boundingBox() { clear(); };
  void clear();
  void openBox();
  void set(float x1, float y1, float z1,
	   float x2, float y2, float z2);
  void test(float x, float y, float z);  // expand bounding box if pt outside current
  void test(boundingBox *bbox);          // result is union of bounding boxes 
  bool inBox(float x1, float y1, float z1);
  bool inBox(float x1, float y1);
  void copy(boundingBox &bbox)
  {
    valid = bbox.valid;
    bbMin[0] = bbox.bbMin[0];
    bbMin[1] = bbox.bbMin[1];
    bbMin[2] = bbox.bbMin[2];
    bbMax[0] = bbox.bbMax[0];
    bbMax[1] = bbox.bbMax[1];
    bbMax[2] = bbox.bbMax[2];
  }
  bool isSame(boundingBox &bbox)
  {
    return
      valid == bbox.valid &&
      bbMin[0] == bbox.bbMin[0] &&
      bbMin[1] == bbox.bbMin[1] &&
      bbMin[2] == bbox.bbMin[2] &&
      bbMax[0] == bbox.bbMax[0] &&
      bbMax[1] == bbox.bbMax[1] &&
      bbMax[2] == bbox.bbMax[2];
  }
};

// child boundingBox class to handle lat/long inBox test specifics
//   for wrap around - move any -ve longs to +ve
// IMPORTANT NOTE: Uses lat, long order i.e. x1=lat, y1=long
class llBoundingBox : public boundingBox
{
 public:
  llBoundingBox() : boundingBox() {};
  bool inBox(float lat, float lng, float ht)
    {
      return boundingBox::inBox(lat, lng, ht);
    }
  bool inBox(float lat, float lng)
    {
      return boundingBox::inBox(lat, lng);
    }
  bool inBox(LatLongHt *latlonght)
    {
      if (!latlonght) return false;
      else
	return boundingBox::inBox(latlonght->Lat, latlonght->Long);
    }
  void set(float lat1, float lng1, float ht1,
	   float lat2, float lng2, float ht2)
    {
      if ((lng1 < 0) || (lng2 < 0))
	{
	  lng1 += 360;
	  lng2 += 360;
	}
      boundingBox::set(lat1, lng1, ht1, 
		       lat2, lng2, ht2);  // x=long, y=lat
    }
  void set(float lat1, float lng1,
	   float lat2, float lng2)
    {
      if ((lng1 < 0) || (lng2 < 0))
	{
	  lng1 += 360;
	  lng2 += 360;
	}
      boundingBox::set(lat1, lng1, -100, 
		       lat2, lng2, 100);  // x=long, y=lat
    }
  // set bb based on centre lat/long and rng
  // if invalid lat/long at rng returns false and clears bb
  bool setLatLongRng(float lat, float lng, float rng); 
  void test(float lat, float lng, float ht)  // 
    {
      if (lng < 0) 
	lng += 360;
      boundingBox::test(lat, lng, ht);
    }
};

class mouseState;

#define RPOLAYS_MAXLAYERS 32

/*
  Layers allocated will be 0 to layerCount i.e. layerCount+1 layers
  Layer 0 may be used as a special always on layer, and may be loaded
  using a .map file or specifying *Layer=0 in a map file
  There are currently only GUI controls for layers 1-10
*/
class overlayLayers
{
 public:
  vector<string> layerStrings;
  int layerCount;
  bitset<RPOLAYS_MAXLAYERS> layerFlags;
  void copy(overlayLayers& copylayers);
  void setDefaultLayerState();  // set the string "Layer n" if not already defined
  void loadLayerDefs(char *fname);
  void setLayerCount(int count);
  int getLayerCount() { return layerCount; };
  void setLayerString(int layer, char *newstr);
  const char* getLayerString(int layer);
  void setLayerState(int layer = -1, bool state = true);  // set the state of layer
  void setAllLayerState(bool state = true);  // set the state of all layers
  bool getLayerState(int layer);
  overlayLayers();
};

class renderProperties
{
 public:
  float rng;         // display range value, used for level of detail control
  float scaleFactor; // scale factor to apply, esp for Symbol rendering, e.g. scale with win size
  float scaleRatio;  // x axis scale ratio relative to y axis, for Symbol rendering where xscale != yscale e.g. RHI
  bool useDlist;     // use display list rendering
  bool allowColors;
  bool seqRunning;
  mouseState *MouseState;
  overlayLayers *olayLayers;  
  time_t 
    imgTime,
    imgTimeSpan,
    scanTime;
  renderProperties() { init(); };
  llBoundingBox latlongBB;  // 
  bool excludeOOBB;  // don't render stuff Out Of Bounding Box (OOBB)
  void init()
  {
    MouseState = NULL;
    rng = 1024;  // put something in there
    useDlist = false;
    allowColors = true;
    scaleFactor = scaleRatio = 1.0;
    seqRunning = true;
    olayLayers = NULL;
    clearTimes();
    excludeOOBB = false;
  };
  void copy(renderProperties &other)
  {
    copy(&other);
  };
  void copy(renderProperties *other)
  {
    if (!other) return;
    MouseState = other->MouseState;
    rng = other->rng;  // put something in there
    useDlist = other->useDlist;
    allowColors = other->allowColors;
    scaleFactor = other->scaleFactor;
    scaleRatio =  other->scaleRatio;
    seqRunning = other->seqRunning;
    olayLayers = other->olayLayers;
    imgTime = other->imgTime;
    imgTimeSpan = other->imgTimeSpan;
    scanTime = other->scanTime;
    excludeOOBB = other->excludeOOBB;
    latlongBB = other->latlongBB;
  };     
  void clearTimes()
    {
      imgTime = scanTime = 0;
      imgTimeSpan = 600;  // initial default to 10 minutes
    };
};

class DrawData {
 public:
  DrawDataType    dataType;
  float	    x, y, z; 
  DrawCoordType coordType;      // define coord type
  uchar	    color[4];
  bool	    useThisColor;	// allow this to override default color
  float     lineThickness;      // if lineThickness == 0, uses current
  bool	    visible;
  bool	    bbFlag;             // inside bounding box flag
  float     minRng, maxRng;     // only draw if display scope is >minRng, < MaxRng
  GLuint    dlist;              // display list ident, 0 if no display list
  bool      useDlist, dlistValid;         // if false new display list should be created
  int       layer;              // layer number
  bool      filled;
  DrawData(float POSX = 0, float POSY = 0, float POSZ = 0) : 
    dataType(DDT_Base), x(POSX), y(POSY), z(POSZ), 
    useThisColor(false), visible(true), bbFlag(true)
    {
      minRng = 0;
      maxRng = MAXFLOAT;
      coordType = DCT_Undefined;
      dlist = 0;
      useDlist = dlistValid = false;
      currentAttribsPushed = 0;
      lineThickness = 0;
      layer = -1;
      filled = false;
    };
  DrawData(float POSX, float POSY, float POSZ, 
	   uchar R, uchar G, uchar B) : 
    dataType(DDT_Base), x(POSX), y(POSY), z(POSZ), 
    useThisColor(true), visible(true), bbFlag(true) 
    {
      color[0] = R; 
      color[1] = G;
      color[2] = B;
      color[3] = 255;
      minRng = 0;
      maxRng = MAXFLOAT;
      coordType = DCT_Undefined;
      dlist = 0;
      useDlist = dlistValid = false;
      currentAttribsPushed = 0;
      lineThickness = 0;
      layer = -1;
      filled = false;
   };
  virtual void set(float POSX, float POSY, float POSZ)
    { 
      x = POSX;
      y = POSY;
      z = POSZ;
    }
  virtual void set(float POSX, float POSY, float POSZ, 
		   uchar R, uchar G, uchar B)
    {
      useThisColor = true;
      x = POSX;
      y = POSY;
      z = POSZ; 
      color[0] = R; 
      color[1] = G;
      color[2] = B;
      color[3] = 255;
    };
  virtual ~DrawData() 
    { 
      if (dlist)
	glDeleteLists(dlist, 1);
    };

  // set the color properties of this object
  virtual void setColor(uchar R, uchar G, uchar B);
  virtual void setColor(float R, float G, float B);
  virtual void setColor(RGBA *rgba);

  // set the current rendering color, doesn't change object color
  virtual void setRenderColor(float attenFactor = 1.0);
  virtual void setRenderColor(uchar R, uchar G, uchar B, float attenFactor = 1.0);
  virtual void setRenderColor(float R, float G, float B, float attenFactor = 1.0);
  virtual void setRenderColor(RGBA *rgba, float attenFactor = 1.0);
  // fades from rgba1 to rgba2 according to fadeFactor (0-1)
  virtual void setRenderColor(RGBA *rgba1, RGBA *rgba2, float fadeFactor);
  virtual void getRenderColor(RGBA *rgba1, RGBA *rgba2, RGBA *rgbaout, float fadeFactor);

  virtual void setLineThickness(float linethickness = 0) { lineThickness = linethickness; };

  virtual void setVisibility(bool state = true) { visible = state; };
  virtual void setVisibleRng(float MaxRng, float MinRng = 0) 
    { 
      maxRng = MaxRng;
      minRng = MinRng;
    };
  virtual void setLayer(int l) { layer = l; };
  virtual int getLayer() { return layer; };
  virtual void setFilled(bool l) { filled = l; };
  virtual bool getFilled() { return filled; };
  virtual bool Visible() { return visible; };
  virtual double dist2DSquared(float x, float y);  // dX*dX+dY*dY distance 
  virtual double dist2DSquared(point2d pt);  // dX*dX+dY*dY distance 
  virtual double dist3DSquared(float x, float y, float z);  // dX*dX+dY*dY+dZ*dZ distance
  virtual double dist3DSquared(point3d pt);  // dX*dX+dY*dY+dZ*dZ distance
  void setCoordType(DrawCoordType coordtype) { coordType = coordtype; };
  virtual bool stringSame(char *matchtext);

  // create a copy of this in the specified projection 
  virtual DrawData* makeProjCopy(DrawCoordType projType);
  
  // convert current x,y,z coords to new projection
  virtual bool projCoord(DrawCoordType projType);

  virtual void invalidateDlist() { dlistValid = false; };
  virtual void setUseDlist(bool state = true);
  virtual void render(renderProperties *renderProps = 0);
  virtual void render(DrawText *textrenderer, renderProperties *renderProps = 0);
  virtual void doRender(renderProperties *renderProps = 0);
  virtual void doRender(DrawText *textrenderer, renderProperties *renderProps = 0);

  // set up render properties before render
  int currentAttribsPushed; // inc for each set, dec for each restore
  virtual void setRenderProps(renderProperties *renderProps = 0);
  virtual void restoreRenderProps();
  virtual bool shouldRender(renderProperties *renderProps = 0);

  virtual boundingBox* getBB()
    {
      return NULL;
    };
  // 
  virtual bool inLLBB(llBoundingBox &bbox)
  {
    return bbox.inBox(x, y);  // y=long, x=lat
  };
};

class DrawDataVector : public DrawData {
  vector<point3d> points;
  vector<point3d>::iterator thispoint;
  char *Label;  // optional text label, uses base class x,y,z as Label location
 protected:
  bool closedVector;   // if true draw from last point to first
 public:
  DrawDataVector(float POSX = 0, float POSY = 0, float POSZ = 0) : 
    DrawData(POSX, POSY, POSZ) 
    { 
      init();
    };
  DrawDataVector(float POSX, float POSY, float POSZ, 
		 uchar R, uchar G, uchar B) : 
    DrawData(POSX, POSY, POSZ, R, G, B)  
    { 
      init();
    };
  virtual ~DrawDataVector() 
    { 
      Clear();
      if (Label)
	delete[] Label;
    };
  virtual void init();
  virtual void    append(point3d *newpoint);		// add point to end
  virtual void    insert(point3d *newpoint, int index);	// insert point before index
  //  virtual void    create(int numpoints);	// create numpoints
  virtual void    setPoint(point3d *newpoint, int index);	// replace point at index with newpoint
  virtual void    setLabel(char *label, float lx, float ly, float lz);
  virtual void    insertEndLineSegment(int index = -1);  // insert an end of line segment marker
  point3d* first();
  point3d* next();    
  point3d* elem(int n);    
  int	   elems();
  virtual void Clear();
  virtual void setClosedVector(bool closedvector = true) 
    { closedVector = closedvector; };
  virtual void checkBB(point3d *newpoint);
  virtual void setUseBB(bool usebb = true) { useBB = usebb; };
  bool useBB;
  boundingBox BB;
  int size;

  // create a copy of this in the specified projection 
  virtual DrawDataVector* makeProjCopy(DrawCoordType projType);

  // convert current x,y,z coords to new projection
  virtual bool projCoord(DrawCoordType projType);

  // convert current x,y,z coords to new projection
  virtual bool projPoint(point3d *point, DrawCoordType projType);

  // doRender will actually render the data withhout using display list
  virtual void doRender(renderProperties *renderProps = 0);
  // doRender will actually render the data withhout using display list
  virtual void doRenderFilled(renderProperties *renderProps = 0);

  virtual boundingBox* getBB()
    {
      return &BB;
    }
};

class DrawDataVectorArea : public DrawDataVector {
 public:
  DrawDataVectorArea(float POSX = 0, float POSY = 0, float POSZ = 0) : 
    DrawDataVector(POSX, POSY, POSZ), calcAreaFlag(true)
    { 
      closedVector = true;
      dataType = DDT_VectorArea; 
      area = 0; 
    };
  DrawDataVectorArea(float POSX, float POSY, float POSZ, 
		 uchar R, uchar G, uchar B) : 
    DrawDataVector(POSX, POSY, POSZ, R, G, B), calcAreaFlag(true) 
    { 
      closedVector = true;
      dataType = DDT_VectorArea; 
      area = 0; 
    };
  virtual ~DrawDataVectorArea() { };
  virtual void    append(point3d *newpoint);		// add point to end
  virtual void    insert(point3d *newpoint, int index);	// insert point before index
  virtual void    setPoint(point3d *newpoint, int index);	// replace point at index with newpoint
  bool    calcAreaFlag;   // if true calculate area as points added
  double  area;  
  double   calcClosedArea();	// assumes posx,y are lat/long, returns km^2
  // assumes posx,y are lat/long, returns km^2, calc spherical degree area
  // then multiplies by AreaPerSphDeg. use SqMi= 273218.4 for square miles etc.
  double   calcClosedLatLongArea(float AreaPerSphDeg = 707632.4);	
  double  Area() { return area; };
  virtual void    Clear();

};

class DrawDataSymbol : public DrawData {
 public:
  // rotation may be used to orientate symbol correctly
  // e.g. if x,y,z are Cartesian km relative to centre of earth
  // rotX=-lat, rotY=long will orientate cartesian XY plane to
  // north aligned tangent plane at that lat/long after XYZ translation
  float   rotX, rotY, rotZ;
  float   Scale;      // Scale factor, 1=default size
  float   Rotate;     // rotation around final Z axis
  
  // scaleRotDlist can be reset per render to set final symbol scaling/rotation
  // Allows rest of symbol rendering dlist to be reused
  // Particularly useful where text scaling varies btwn windows, e.g. map text 
  GLuint    scaleRotDlist;

  DrawDataSymbol(float POSX, float POSY, float POSZ) : 
    DrawData(POSX, POSY, POSZ)
    { 
      init();
    };
  DrawDataSymbol(float POSX, float POSY, float POSZ, 
		 uchar R, uchar G, uchar B) : 
    DrawData(POSX, POSY, POSZ, R, G, B)  
    { 
      init();
    };
  void init()
    {
      dataType = DDT_Symbol;
      rotX = rotY = rotZ = 0.0;
      Scale = 1.0;
      Rotate = 0.0;
    };
  void setRotXYZ(float RotX, float RotY, float RotZ)
    {
      rotX = RotX;
      rotY = RotY;
      rotZ = RotZ;
    };
  void setScale(float scale)
    {
      Scale = scale;
    };
  void setRotate(float rotate)
    {
      Rotate = rotate;
    };
  virtual ~DrawDataSymbol() { };

  // create a copy of this in the specified projection 
  virtual DrawDataSymbol* makeProjCopy(DrawCoordType projType);

  // convert current x,y,z coords to new projection
  virtual bool projCoord(DrawCoordType projType);

  virtual void setRenderProps(renderProperties *renderProps = 0);

};

extern vector<GLuint> circDlists; 

class DrawDataSymCircle : public DrawDataSymbol {
 public:
  float Radius; // radius in km for coordtype DCT_LatLongHt
  bool filled;
  int angres;
  DrawDataSymCircle(float POSX, float POSY, float POSZ, 
		  float radius) : 
    DrawDataSymbol(POSX, POSY, POSZ)
    {
      dataType = DDT_Symbol_Circle;
      Radius = radius;
      filled = false;
      angres = 5;
      useDlist = false;
      setUseDlist(true);
      dlistValid = false;
    };
  virtual ~DrawDataSymCircle() { };

  // create a copy of this in the specified projection 
  virtual DrawDataSymCircle* makeProjCopy(DrawCoordType projType);
  virtual void setFilled(bool state = true) { filled = state; };
  virtual void setAngRes(int res = 5) 
    {   
      if (res < 1)
	angres = 1;
      else if (res > 30)
	angres = 30;
      else
	angres = res; 
    };

  virtual void setRenderProps(renderProperties *renderProps = 0);

  // circle object uses a global circle display list instead of one per instance
  virtual void render(renderProperties *renderProps = 0);
  virtual void render(DrawText *textrenderer, renderProperties *renderProps = 0)
    {
      render(renderProps);
    };
  virtual void doRender(renderProperties *renderProps = 0);
};

/*
 * makes a new copy of newstring
 */
class DrawDataSymText : public DrawDataSymbol {
 public:
  char    *textstring;
  int	  fontID;     // if fontID==0 drawer should use default 
  DrawDataSymText(float POSX, float POSY, float POSZ, 
		  char *newstring = 0);
  DrawDataSymText(float POSX, float POSY, float POSZ,
		  uchar R, uchar G, uchar B, 
		  char *newstring = 0);
  virtual void set(float POSX, float POSY, float POSZ, 
		   char *newstring = 0);
  virtual void set(float POSX, float POSY, float POSZ,
		   uchar R, uchar G, uchar B, 
		   char *newstring = 0);
  virtual ~DrawDataSymText();
  virtual bool stringSame(char *matchstring);
  virtual void setFontID(int fontid) { fontID = fontid; };

  // create a copy of this in the specified projection 
  virtual DrawDataSymText* makeProjCopy(DrawCoordType projType);

  virtual void doRender(renderProperties *renderProps = 0);
  virtual void doRender(DrawText *textrenderer, renderProperties *renderProps = 0);
};


/*
 * Lower overhead per string than above
 * Uses same scale/rotation, color etc for all
 * Can use DrawDataSymText::textstring and x,y,z for label
 */
class DrawDataSymTextList : public DrawDataSymText {
 public:
  vector<string3d> strings3d;
  vector<string3d>::iterator thisString;
  virtual void checkBB(string3d *newstr);
  virtual void setUseBB(bool usebb = true) { useBB = usebb; };
  boundingBox BB;
  bool useBB;
  int size;
  
  DrawDataSymTextList() : DrawDataSymText(0,0,0)
    {
      dataType = DDT_Symbol_Text_List;      
      init();
    };
  DrawDataSymTextList(float POSX, float POSY, float POSZ, 
		  char *newstring) :
    DrawDataSymText(POSX, POSY, POSZ, newstring) 
    {
      dataType = DDT_Symbol_Text_List;      
      init();
    };
  DrawDataSymTextList(float POSX, float POSY, float POSZ,
		  uchar R, uchar G, uchar B, 
		  char *newstring) :
    DrawDataSymText(POSX, POSY, POSZ, R, G, B, newstring)
    {
      dataType = DDT_Symbol_Text_List;      
      init();
    };
  virtual ~DrawDataSymTextList();
  virtual void init();
  virtual void add(float POSX, float POSY, float POSZ, char *newstring);
  virtual void add(string3d *newstring3d);
  virtual void Clear();
  virtual int stringCount() { return strings3d.size(); };
  virtual void setCentre(); // set the location of this to the centre if BB avail
  // else use first entry


  // create a copy of this in the specified projection 
  virtual DrawDataSymTextList* makeProjCopy(DrawCoordType projType);
  // convert current x,y,z coords to new projection
  virtual bool projString(string3d *str, DrawCoordType projType);

  virtual boundingBox* getBB()
    {
      return &BB;
    }
  virtual void setRenderProps(renderProperties *renderProps = 0);
};

class DrawDataList {
 public:
  list<DrawData*>  dataList;	// list of pointers to DrawData objects
  list<DrawData*>::iterator  thisData;// 
  bool		thisDataValid;
  int		size;
  DrawDataList();
  virtual ~DrawDataList();	    // deletes all referred DrawData
  void		add(DrawData *DRAWDATA);    // take over control of DRAWDATA, will delete when this closed
  void		remove(DrawData *DRAWDATA); // look for matching pointer and delete it
  void		clear();		    // delete ALL entries
  DrawData*	nearest2(float x, float y);  // return nearest pos 		
  DrawData*	nearest3(float x, float y, float z);  // return nearest pos 		
  DrawData*	findData(DrawData *THISDATA); // 
  DrawData*	findText(char *text);	    // return matching data
  DrawData*	first();
  DrawData*	last();
  DrawData*	next(DrawData *THISDATA = 0);   // check that *thisData == THISDATA, if not search for THISDATA
  DrawData*	prev(DrawData *THISDATA = 0);
  DrawData*	nextByType(DrawData *THISDATA, DrawDataType type);
  DrawData*	prevByType(DrawData *THISDATA, DrawDataType type);
  int		dataCount();	// return number of nodes
  virtual void  checkBB(DrawData *DRAWDATA);
  virtual void  setUseBB(bool usebb = true) { useBB = usebb; };
  bool          useBB;
  boundingBox   BB;

  // create a copy of this in the specified projection 
  virtual DrawDataList* makeProjCopy(DrawCoordType projType);

  virtual void render(renderProperties *renderProps = 0);
  virtual void render(DrawText *textrenderer, renderProperties *renderProps = 0);

  virtual boundingBox* getBB()
    {
      return &BB;
    };
};

class DrawObj {
 public:
  DrawObj();
  virtual ~DrawObj();
  uchar	 defaultColor[3];
  virtual void setColor(uchar R, uchar G, uchar B);
  virtual void setColorv(uchar *RGB);
  virtual void setColorf(float R, float G, float B);
  virtual void setColor();
  virtual void Draw(DrawData *data, renderProperties *renderprops = 0);
  // the following allows the object's x,y,z to be overridden, e.g. for projection
  virtual void Draw(DrawData *data, float x, float y, float z, renderProperties *renderprops = 0);
  virtual void Draw(DrawDataList *datalist, renderProperties *renderprops = 0);
  virtual float stringWidth(char *string);    // return width of string
  virtual float stringHeight(char *string);    // return width of string
  virtual void  stringSize(char *string, float *w, float *h);    // return width of string
};

#endif
