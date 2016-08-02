#ifndef	__OGLUTILS_H
#define	__OGLUTILS_H

/*
 * oglutils.h
 * 
 * General radar related gl utilities,  e.g. Lat/Long/Ht transformations
 */

#include "rdr.h"
#include <X11/X.h>
#include <X11/Intrinsic.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glext.h>
#ifdef STDCPPHEADERS
#include <list>
#include <vector>
#include <string>
using namespace std;
#else
#include <list.h>
#include <vector.h>
#endif
#include "palette.h"

extern vector<GLuint> circDlists; 
extern GLuint rectDlist, rectFDlist;
extern char *oglExtensions;
extern bool oglUnavailable;
extern bool forceSingleBuffer;
extern string oglVendor;
extern string oglRenderer;
extern float oglVersion;
extern float oglDriverVersion;
extern float glxClientVersion;
extern float glxServerVersion;
extern float minUseVADriver;
extern float minUseCgDriver;

bool oglPalettedTexturesAvail();
bool oglTexture3DAvail();
bool oglVertexArrayAvail();
int glMax3DTextureSize();
int glMaxTextureSize();
bool oglVertexArrayAvail();
bool oglVBOArrayAvail();
int oglMultiTexturesAvail();
bool oglCgShaderAvail();

bool oglExtAvailable(char *extstr = NULL);

// To enableGLMatrixCheck, the file "enableGLMatrixCheck" must exists in the runtime dir.
// To enableGLErrorCheck, the file "enableGLErrorCheck" must exists in the runtime dir.
extern bool enableGLDebug;
/* extern bool enableGLMatrixCheck; */
/* extern bool enableGLErrorCheck; */

// OpenGl stack state debug class
// compare ModelView Projection & Attrib stack pos with expected value
// To use - call set() before Ogl code segment, then check(mssgstr) afterwards 
// to check stack
// is returned to same state
// Use of the pushAttrib/popAttrib calls with enableGLMatrixCheck also will check that the
// the attrib stack pos was incremented/decremented appropriately.

class glStackState
{
 public:
  int mvMatrixPos, projMatrixPos, attribStackPos, localAttribStackPos;
  bool isSet;
  void set();
  bool check(char *mssgstr);
  glStackState() : mvMatrixPos(0), projMatrixPos(0), attribStackPos(0), 
    localAttribStackPos(0), isSet(false) {};
};

char* glErrorString(GLenum _error);
GLenum checkGlError(char *_str, char *_str2 = NULL);

void pushMatrix();
void popMatrix();   
void pushAttrib( GLbitfield mask );
void popAttrib();
void pushClientAttrib( GLbitfield mask );
void popClientAttrib();
int glAttribMaxStack();
int glAttribMaxStackPos();
int glAttribCurrentStack();
int glAttribCurrentStackLocal();
int glModelViewMaxStack();
int glModelViewMaxStackPos();
int glModelViewCurrentStack();
int glProjectionMaxStack();
int glProjectionMaxStackPos();
int glProjectionCurrentStack();

#define USE_CIRC_DISPLAY_LIST true
// need to be careful using display list
// BY DEFAULT display list ids are per window, so cannot use
// global circle utilities from multiple ogl windows
// To allow shared display lists, a sharing context must be
// specified in the glXCreateContext call 

// draw a circle
void DrawOglCirc(float ofsx, float ofsy, float ofsz, 
		 float radius, int angres = 10, 
		 bool useDisplList = false);
// draw a circle arc
void DrawOglCircArc(float ofsx, float ofsy, float ofsz, 
		 float radius,  
		 int startaz, int endaz, 
		 int angres = 10,
		 bool useDisplList = false);
// draw a filled circle
void DrawOglCircF(float ofsx, float ofsy, float ofsz, 
		  float radius, int angres = 10, 
		  bool useDisplList = false);
// draw a filled circle arc
void DrawOglCircArcF(float ofsx, float ofsy, float ofsz, 
		 float radius,
		 int startaz, int endaz, 
		 int angres = 10, 
		 bool useDisplList = false);

// draw an ellipse
void DrawOglEllipse(float ofsx, float ofsy, float ofsz, 
		    float radius_minor, float radius_major, 
		    float rot_degs = 0, int angres = 10, 
		    bool useDisplList = false);
// draw an ellipse arc
void DrawOglEllipseArc(float ofsx, float ofsy, float ofsz, 
		    float radius_minor, float radius_major, 
		    int startaz, int endaz, 
		    float rot_degs = 0,
		    int angres = 10, 
		    bool useDisplList = false);
// draw a filled ellipse
void DrawOglEllipseF(float ofsx, float ofsy, float ofsz, 
		     float radius_minor, float radius_major, 
		     float rot_degs = 0, int angres = 10, 
		     bool useDisplList = false);
// draw a filled ellipse arc
void DrawOglEllipseArcF(float ofsx, float ofsy, float ofsz, 
		    float radius_minor, float radius_major, 
		    int startaz, int endaz, 
		    float rot_degs = 0,
		    int angres = 10,  
		    bool useDisplList = false);

// draw a circle
inline void DrawOglDLCirc(float ofsx, float ofsy, float ofsz, 
		 float radius, int angres = 10)
{
  DrawOglCirc(ofsx, ofsy, ofsz, radius, angres, true);
};

// draw a filled circle
inline void DrawOglDLCircF(float ofsx, float ofsy, float ofsz, 
		  float radius, int angres = 10)
{
  DrawOglCircF(ofsx, ofsy, ofsz, radius, angres, true);
};

// draw an ellipse
inline void DrawOglDLEllipse(float ofsx, float ofsy, float ofsz, 
		    float radius_minor, float radius_major, 
		    float rot_degs = 0, int angres = 10)
{
  DrawOglEllipse(ofsx, ofsy, ofsz, radius_minor, radius_major, 
		 rot_degs, angres, true);
};


// draw a filled ellipse
inline void DrawOglDLEllipseF(float ofsx, float ofsy, float ofsz, 
		     float radius_minor, float radius_major, 
		     float rot_degs = 0, int angres = 10)
{
  DrawOglEllipse(ofsx, ofsy, ofsz, radius_minor, radius_major, 
		 rot_degs, angres, true);
};

void DrawOglArrow(float linelen, float rot_degs,
		  float headlen = 0, float headwid = 0);

// draw a rectangle 
void DrawOglRect(float ofsx, float ofsy, float ofsz, 
		 float length, float width = 0, 
		 float rot_degs = 0);
// draw a filled ellipse
void DrawOglRectF(float ofsx, float ofsy, float ofsz, 
		  float length, float width = 0, 
		  float rot_degs = 0);
// draw a square 
inline void DrawOglSquare(float ofsx, float ofsy, float ofsz, 
			  float length, float rot_degs = 0) 
{
  DrawOglRect(ofsx, ofsy, ofsz, length, 0, rot_degs);
};

// draw a filled ellipse
inline void DrawOglSquareF(float ofsx, float ofsy, float ofsz, 
			   float length, float rot_degs = 0)
{
  DrawOglRectF(ofsx, ofsy, ofsz, length, 0, rot_degs);
};

void GlDumpCurrentMatrix(); // dumps the current gl matrix;

/*
  GlSetOrigin* will translate to appropriate latlonght and rotate
  so as to provide a north aligned tangent plane
*/ 
void GlSetOriginLatLng(LatLongHt *Dest);
void GlSetOriginLatLng(float Lat, float Long, float Ht);
void GlSetOriginRdrSite(int SiteDest, bool zeroHt = false);

/*
  GlRotate* will only rotate so as to provide a north aligned tangent plane
  to the given lat/long
  No translation occurs.
  Typically will be used where the cartesian x,y,z is already calculated
  but the tangent plane rotation needs to be set to render a symbol facing
  the correct way as to "sit on the earth's surface" or to face the lat/long
  point of view
  e.g. text redering etc.
*/ 
void GlRotateLatLng(LatLongHt *Dest);
void GlRotateLatLng(float Lat, float Long);
void GlRotateRdrSite(int SiteDest);

/*
  GlUnSetOrigin* assumes the origin is currently at the passed
  latlonght and will translate to the centre of the earth and rotate
  so as to reset the plane to perpendicular ot the earth centric Z-Axis
*/ 
void GlUnsetOriginLatLng(LatLongHt *Org);
void GlUnsetOriginLatLng(float Lat, float Long, float Ht);
void GlUnsetOriginRdrSite(int SiteOrg, bool zeroHt = false);

/*
  GlMvOrigin* will translate from the Org latlong ht to the Dest latlonght
  and rotate so as to provide a north aligned tangent plane at the Dest
*/ 
void GlMvOriginLatLng(LatLongHt *Org, LatLongHt *Dest);
void GlMvOriginRdrSite(LatLongHt *Org, int SiteDest);


void setPalColor(int index);
void setClearColor(int index);
void setRGBAColor(RGBA &color);

bool checkXGLVisualAttribs(Widget w, 
			    int &rgba, int &dblBuff,
			    int &rSize, int &gSize, int &bSize,
			    int &alphaSize, int &depthSize,
			    int &auxBuffs, int &stencilSize,
			    bool verbose = true);

int ogl_isExtensionSupported(const char *extension);

void printGLStatus(FILE *file);

/*
  General vertex Buffer class 
  Current implementation supports Vertex, Color and index buffers
  only
  Provides switches for optional indexed rendering and 
  optional Vertex Buffer Object use when rendering.
*/

class rpVertexBuffer
{
  friend class rpRadlVertBuffer;
 private:
  GLfloat *_vBuff;   // x,y,z vertex array buffer 
  GLubyte *_cBuff;   // unsigned char rgba color array buffer
  GLushort *_indices;
 public:
  int   index_count,
    last_index;
  int   buffSize,  // current buffer usage size, may be < allocBuffSize
    allocBuffSize;  // currently allocated buffer size
  rpVertexBuffer(int buffsize = 1024);
  virtual ~rpVertexBuffer();
  // if buffsize > current allocBuffSize, reallocate buffers, else leave as is
  // if buffSize INCREASED return true, caller may recalc verts etc.
  virtual bool  setBuffSize(int buffsize);  
  // default to copying all buffers, child classes may override defaults
  virtual void copy(rpVertexBuffer *srcbuffer,
		    bool do_v = true, 
		    bool do_c = true, 
		    bool do_i = true);
  GLfloat *vBuff(int index = 0)  // return pointer to vBuff at index
    {
      if (index >= buffSize) index = buffSize-1;
      if (_vBuff) return _vBuff + (index * 3);
      else return NULL;
    };
  GLubyte *cBuff(int index = 0)  // return pointer to cBuff at index 
    {
      if (index >= buffSize) index = buffSize-1;
      if (_cBuff) return _cBuff + (index * 4);
      else return NULL;
    };
  GLushort *indices(int index = 0) // return pointer to indice at index
    {
      if (index >= buffSize) index = buffSize-1;
      if (_indices) return _indices + index;
      else return NULL;
    };
  void clearCBuff();

  virtual bool setUsePackedVerts(bool state = true) // 
  {
    if (state == usePackedVerts) 
      return false;
    usePackedVerts = state;
    return true;
  };
  bool usePackedVerts; // using packed vtx/col arrays, not sparse indexed
  bool useIndices;
  bool useVBOs;
  uint VBO_Verts,    // VBO buffer "names"
    VBO_Colors,
    VBO_Indices;

  virtual void buildVBOs();
  // if do_? true, write buffer to render
  // if ?size = -1, write complete buffer
  // if ?size defined onoly write size elements
  virtual void bufferData(bool do_v = true, int vsize = -1, 
			  bool do_c = true, int csize = -1, // 
			  bool do_i = true, int isize = -1);
  virtual void doRender();
};  

#endif	/* __OGLUTILS_H */
