#ifndef __DRAWTEXT_H__
#define __DRAWTEXT_H__

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/GLwMDrawA.h>
#include "draw.h"

/*
 * these classes actually render text data objects to the screen
 */

extern char *DEFAULTFONT;
extern int DEFAULTFONTID;
extern int MAXFONTID;
typedef unsigned char uchar;

class DrawText : public DrawObj {
public:
    DrawText();
    virtual ~DrawText();
    float fontSize, rot, dflt_fontSize, dflt_rot;
    char thisFontName[256];
    bool drawCentred, drawVCentred;
    uchar bgcolor[4];
    bool useSolidBG;
    virtual void setDefault(float size, float rot = 0.0);
    virtual void setDefault();
    virtual void setSizeRotate(float size, float rot = 0.0);
    virtual void setSize(float size);
    virtual void setRotate(float rot);
    virtual void setColor();
    virtual void setBGColor(uchar R, uchar G, uchar B, uchar A = 255);
    virtual void setFont(char *fontName);
    virtual void setDrawCentred(bool state = true) { drawCentred = state; };
    virtual void setDrawVCentred(bool state = true) { drawVCentred = state; };
    virtual void Draw(char *str); // simply draw the string, assumes ALL modelling xforms already done
    virtual void Draw(char *str, GLfloat x, GLfloat y, GLfloat z = 0.0, renderProperties *renderprops = 0);
    void Draw(DrawData *data, renderProperties *renderprops = 0);	// draw a drawdata object, if appropriate type
// the following allows the object's x,y,z to be overridden, e.g. for projection
    void Draw(DrawData *data, float x, float y, float z, renderProperties *renderprops = 0);
};

class DrawOGLXBitmapText : public DrawText {
public:
    DrawOGLXBitmapText(char *fontName, Display *DPY, 
	Widget WID, GLXContext CONTEXT);
    ~DrawOGLXBitmapText();
    Display *dpy;
    Widget wid;
    GLXContext context;
    XFontStruct *fontInfo;
    GLuint base, range;			// display list parameters
    void closeFont();			// close this font, release display lists
    void init();
    void Draw(char *str, renderProperties *renderprops = 0);
    void Draw(char *str, GLfloat x, GLfloat y, GLfloat z = 0.0, renderProperties *renderprops = 0);
    void Draw(DrawData *data, renderProperties *renderprops = 0);
// the following allows the object's x,y,z to be overridden, e.g. for projection
    void Draw(DrawData *data, float x, float y, float z, renderProperties *renderprops = 0);
    void setColor(uchar R, uchar G, uchar B);
    void setColorf(float R, float G, float B);
    void setColor();
    void setFont(char *fontName);
    void makeRasterFont();
    void makeSimpleRasterFont();
};

#ifdef USE_GLC
class DrawGLCText : public DrawText {
public:
    DrawGLCText(char *fontName, Widget WID, GLXContext CONTEXT);
    ~DrawGLCText();
    GLint glccontext;
    void setContext();
    Widget wid;
    GLXContext glcontext;
    GLint fontID, allocFontID;
    GLint	drawStyle;
    void closeFont();	// close this font, release display lists
    bool init();
    void Draw(char *str, renderProperties *renderprops = 0);
    void Draw(char *str, GLfloat x, GLfloat y, GLfloat z = 0.0, renderProperties *renderprops = 0);
    void Draw(DrawData *data, renderProperties *renderprops = 0);
// the following allows the object's x,y,z to be overridden, e.g. for projection
    void Draw(DrawData *data, float x, float y, float z, renderProperties *renderprops = 0);
    void Draw(DrawDataList *datalist, renderProperties *renderprops = 0);
    void setColor(uchar R, uchar G, uchar B);
    void setColorf(float R, float G, float B);
    void setColor();
    void setFont(char *fontName);
    void setFace(char *faceName);
    void setDefault(float size, float rot = 0.0);
    void setDefault();
    void setSizeRotate(float size, float rot = 0.0);
    void setSize(float size);
    void setRotate(float rot);
    void makeRasterFont();
    void makeSimpleRasterFont();
    float stringWidth(char *string);    // return width of string
    float stringHeight(char *string);    // return width of string
    void  stringSize(char *string, float *w, float *h);    // return width of string
};
#endif

#ifdef USE_GLF

extern char GLF_FontPath[];
extern char GLF_Times[];
extern char GLF_Courier[];
extern char GLF_Arial[];
extern char GLF_Techno[];
extern char GLF_Techno1[];
extern char GLF_Crystal[];

extern char GLF_B_Arbat[];
extern char GLF_B_Arial[];
extern char GLF_B_Brushtype[];
extern char GLF_B_Chicago[];
extern char GLF_B_Courier[];
extern char GLF_B_Cricket[];
extern char GLF_B_Crystal[];
extern char GLF_B_Fixedsys[];
extern char GLF_B_Gals[];
extern char GLF_B_Greek[];
extern char GLF_B_Impact[];
extern char GLF_B_Proun[];
extern char GLF_B_Techno[];
extern char GLF_B_Times[];

enum glf_DrawMode {glf_Wired, glf_Solid, glf_3dWired, glf_3dSolid, 
      glf_Bitmap, glf_BitmapMask};

class DrawGLFText : public DrawText {
public:
    DrawGLFText(char *fontName = 0);
    DrawGLFText(int fontid);
    ~DrawGLFText();
    int getFontDesc(char *fontName);
    int fontDesc;
    int fontID;
    int isBMF;
    float BMFScaleFactor;
    bool bitmapMask;
    void setContext();
    glf_DrawMode drawMode;
    bool init();
    void Draw(char *str);
    void Draw(char *str, GLfloat x, GLfloat y, GLfloat z = 0.0, 
	      renderProperties *renderprops = NULL);
    void Draw(char *str, GLfloat x, GLfloat y, GLfloat z, 
	      renderProperties *renderprops,
	      float *strwid, float *strht);
    void Draw(DrawData *data, renderProperties *renderprops = 0);
// the following allows the object's x,y,z to be overridden, e.g. for projection
    void Draw(DrawData *data, float x, float y, float z, 
	      renderProperties *renderprops = 0);
    void Draw(DrawDataList *datalist, renderProperties *renderprops = 0);
    void Draw(DrawDataSymText *datalist, renderProperties *renderprops = 0);
    void Draw(DrawDataSymTextList *datalist, 
	      renderProperties *renderprops = 0);
    void setDrawCentred(bool state = true) { drawCentred = state; };
    void setColor();
    void setFont(char *fontName, bool is_bmf = false);
    void setFont(int fontid = 0);  // no param will set to default
    void setDefault(float size, float rot = 0.0);
    void setDefault();
    void setSizeRotate(float size, float rot = 0.0);
    void setSize(float size);
    void setRotate(float rot);
    void setBitmapMask(bool state = false) { bitmapMask = state; };
    float stringWidth(char *string, renderProperties *renderprops = 0);    // return width of string
    float stringHeight(char *string, renderProperties *renderprops = 0);    // return width of string
    float maxHeight(renderProperties *renderprops = 0);    // return max height of this font
    void  stringSize(char *string, float *w, float *h, renderProperties *renderprops = 0);    // return width of string
    void  drawSolidBG(char *str, renderProperties *renderprops = 0);
    void  drawSolidBG(float w, float h);
};
extern DrawGLFText *defaultGLFTextRenderer;
DrawGLFText *getDefaultGLFTextRenderer();
#endif

extern DrawText *defaultTextRenderer;
DrawText *getDefaultTextRenderer();


#endif
