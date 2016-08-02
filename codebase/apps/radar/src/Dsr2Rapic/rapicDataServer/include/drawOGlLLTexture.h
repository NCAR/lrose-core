#ifndef __DRAWOGLLLTEXTURE_H__
#define __DRAWOGLLLTEXTURE_H__

/*
  Class to render lat/long navigated 2d arrays to projected screen space
  Default is render to 3d ortho world km coords
*/

#define GL_GLEXT_PROTOTYPES
#include "textureProj.h"
#include "draw.h"
#include <GL/gl.h>
#include "palette.h"
#include <string>

#ifdef USE_CG
#include "rapicCgLumPalette.h"
#endif

using namespace std;

enum oglTextureFmt { OTF_index, OTF_RGB, OTF_RGBA };
extern char defaultByteArraySuffix[];
extern char defaultLLProjGridSuffix[];
extern char defaultRGBPalSuffix[];

/*
  

  LLProjGrid file format
  texw=w texh=h
  projgridw=w projgridh=h
  [texfname=fname] optional, if texfname defined and suffix is .gif, use readGIF to read it and palette
  [palfname=fname] optional

  Followed by projgridw*projgridh lines of
  lat   long   tex_x  tex_y

  readLLProjGrid will fail if insufficient lines
*/  

/*
  tileDefParams struct for typical tile creation parameters
*/

struct tileDefParams
{
  int row1, col1, h, w;
  int spacing;
  tileDefParams(int Row1 = 0, int Col1 = 0, int H = 0, int W = 0, int Spacing = 1)
  {
    set(Row1, Col1, H, W, Spacing);
  };

  void set(int Row1, int Col1, int H, int W, int Spacing = 1)
  {
    row1 = Row1;
    col1 = Col1;
    h = H;
    w = W;
    spacing = Spacing;
  };
};

class oglTextureTile
{
 private:
  uchar *texarray;
  int   texarraysize;
  bool  useExtArray;
 public:
  uchar         *texArray(int pos = 0)
    {
      if (!useExtArray) 
	{
	  if (pos > int(texVector.size()-1))
	    pos = 0;  //return something safe;
	  return (&(texVector[pos]));
	}
      else 
	{
	  if (pos > texarraysize-1)
	    pos = 0;  //return something safe;	
	  return texarray+pos;
	}
    };
  void          setExtTexArray(uchar *exttexarray, int w, int h) // use external texarray
    {
      texVector.clear();       // clear internal tex array
      texarray = exttexarray;  // point to external tex array
      texarraysize = w * h;
      tex_w = w;
      tex_h = h;
      useExtArray = true;;
    };
  void          setIntTexArray(int w, int h) // use internal array
    {
      useExtArray = false;
      if (texVector.size() != size_t(w * h))
	texVector.resize(w * h);
      tex_w = w;
      tex_h = h;
    };
  size_t        texSize() 
    { 
      if (texVector.size()) return texVector.size();
      else return texarraysize;
    };
  vector<uchar> texVector;
  oglTextureFmt texFormat;
  int           tex_w, tex_h;
  RGBPalette    *palette;      // assume externally alloc'd pal, don't delete
  llProjGrid    projGrid;
  GLuint        boundID;            // bound texture "name"
  bool          useBinding;
  bool          useBlending;        // if true use blending
  bool          reloadTextures;    // force textures to reload
  GLenum        srcBlendFn,         // src & dest Blend fns
                destBlendFn;
  GLenum        texEnvParam;
  int		TexMinFunction;
  int		TexMagFunction;
  bool	        visible;
  float         minRng, maxRng;     // only draw if display scope is >minRng, < MaxRng
  GLuint        dlist;              // display list ident, 0 if no display list
  bool          useDlist, dlistValid; // if false new display list should be created
  int           layer;              // layer number
  bool          showGrid;
  bool          showBorder;
  RGBA          borderColor;
  float         borderWidth;
  float         attenFactor;
  int           backGroundAlpha;    // alpha value for the background (index 0)
  bool          useCg;
  void          setAtten(float attenfactor = 1.0) 
    { attenFactor = attenfactor; };
 
  oglTextureTile()
    {
      tex_w = tex_h = 0;
      palette = NULL;
      texFormat = OTF_index;
      boundID = 0;
      // typically only use binding with static textures, i.e. not changing
      useBinding = true;
      useBlending = true;
      //      srcBlendFn = GL_ONE_MINUS_DST_COLOR;
      //      destBlendFn = GL_ONE;
      srcBlendFn = GL_SRC_ALPHA;
      destBlendFn = GL_ONE_MINUS_SRC_ALPHA;
      texEnvParam = GL_REPLACE;
      TexMinFunction = TexMagFunction = GL_LINEAR;
      visible = true;
      minRng = 0; maxRng = 9999999;
      dlist = 0;
      useDlist = false;
      dlistValid = false;
      showGrid = showBorder = false;
      borderColor.setRGBA(200,200,200);
      borderWidth = 3;
      layer = -1;
      attenFactor = 1.0;
      texarray = NULL;
      texarraysize = 0;
      useExtArray = false;
      backGroundAlpha = 40;  
      reloadTextures = true;
      useCg = true;
    };
  ~oglTextureTile()  // don't delete texarray or palette, assume they
    {                // are allocated externally
      if (boundID)
	glDeleteTextures(1, &boundID);
      if (dlist)
	glDeleteLists(dlist, 1);
    };
};

class drawOGlLLTexture
{
 public:
  oglTextureFmt texFormat;
  int   tex_w, tex_h;
  uchar *texArray;
  bool localTexArray; // true if texArray allocated by this
  void clearTexArray();
  RGBPalette    *palette;
  llProjGrid    *projGrid;
  string byteArrayFName;
  string rgbPalFName;
  int		TexMinFunction;
  int		TexMagFunction;
  drawOGlLLTexture(char *fname = NULL);
  ~drawOGlLLTexture();
  void close();
  void renderTile(oglTextureTile &tile, bool blend = true);
  void render(rpProjection newproj = proj3DOrthog, renderProperties *renderprops = 0);
  bool loadTextureFiles(const char *fname, int W = 0, int H = 0); // load texture file, prj grid file and palette file (if present)
  bool readByteArrayFile(const char *fname, int W, int H);
  bool readLLProjGrid(const char *fname);
  bool readGIF(const char *fname);
  bool readRGBPalette(const char *fname);
  void setTexArray(uchar *newTexArray, int W, int H);    // set texArray to externally allocated texture array
  void setPalette(RGBPalette *newpal);
  bool useCgFragmentPalette();
};

#endif
