#ifndef __OGLTEXT_H__
#define __OGLTEXT_H__

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

class DrawText : public Draw {
    void setColor(GLfloat R, GLfloat G, GLfloat B);
    void setColor();
    void setFont(char *fontName);
    void printString(char *str);
    void printString(char *str, GLfloat x, GLfloat y, GLfloat z = 0.0);
};

class DrawOGLXBitmapText : public DrawText {
public:
    OGLXBitmapText(char *fontName, Display *DPY, 
	Widget WID, GLXContext CONTEXT);
    ~OGLXBitmapText();
    Display *dpy;
    Widget wid;
    GLXContext context;
    XFontStruct *fontInfo;
    GLuint base, range;			// display list parameters
    char thisFontName[256];
    GLfloat	color[3];
    void closeFont();			// close this font, release display lists
    void init();
    void printString(char *str);
    void printString(char *str, GLfloat x, GLfloat y, GLfloat z = 0.0);
    void setColor(GLfloat R, GLfloat G, GLfloat B);
    void setColor();
    void setFont(char *fontName);
    void makeRasterFont();
    void makeSimpleRasterFont();
};

#endif
