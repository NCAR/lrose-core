#ifndef __GRAPHUTILS_H
#define __GRAPHUTILS_H

#ifdef OPEN_GL
#include <GL/gl.h>
#endif

#ifdef IRIS_GL
typedef fmfonthandle FONTHANDLE;
#endif
#ifdef OPEN_GL
typedef int FONTHANDLE;
#include "oglutils.h"
#endif

// the following functions have implementations for both IRIS_GL and OPEN_GL
inline void depthTestAlways()
{
#ifdef IRIS_GL
  zfunction(ZF_ALWAYS);
#endif
#ifdef OPEN_GL
  glDepthFunc(GL_ALWAYS);
#endif
}

inline void depthTestLess()
{
#ifdef IRIS_GL
  zfunction(ZF_LESS);
#endif
#ifdef OPEN_GL
  glDepthFunc(GL_LESS);
#endif
}

inline void beginLineStrip()
{
#ifdef IRIS_GL
    bgnline();
#endif
#ifdef OPEN_GL
    glBegin(GL_LINE_STRIP);
#endif
}

inline void endLineStrip()
{
#ifdef IRIS_GL
    endline();
#endif
#ifdef OPEN_GL
    glEnd();
#endif
}

inline void beginLineStyle(int factor, ushort pattern)
{
#ifdef IRIS_GL
  linestyle(factor, pattern);
  setlinestyle(1);
#endif
#ifdef OPEN_GL
  glLineStipple(factor, pattern);
  glEnable(GL_LINE_STIPPLE);
#endif
}
    
inline void endLineStyle()
{
#ifdef IRIS_GL
  setlinestyle(0);
#endif
#ifdef OPEN_GL
  glDisable(GL_LINE_STIPPLE);
#endif
}

inline void lineWidth(float width)
{
#ifdef IRIS_GL
  linewidth(int(width));
#endif
#ifdef OPEN_GL
  glLineWidth(width);
#endif
}

inline void scalef(float scalex, float scaley, float scalez)
{
#ifdef IRIS_GL
  scale(scalex, scaley, scalez);
#endif
#ifdef OPEN_GL
  glScalef(scalex, scaley, scalez);
#endif
}

inline void vertex3fv(float *vert)
{
#ifdef IRIS_GL
  v3f(vert);
#endif
#ifdef OPEN_GL
  glVertex3fv(vert);
#endif
}

inline void vertex3f(float x, float y, float z)
{
  float vert[3];
  vert[0] = x;
  vert[1] = y;
  vert[2] = z;
#ifdef IRIS_GL
  v3f(vert);
#endif
#ifdef OPEN_GL
  glVertex3fv(vert);
#endif
}

inline void vertex2fv(float *vert)
{
#ifdef IRIS_GL
  v2f(vert);
#endif
#ifdef OPEN_GL
  glVertex2fv(vert);
#endif
}

inline void vertex2f(float x, float y)
{
  float vert[2];
  vert[0] = x;
  vert[1] = y;
#ifdef IRIS_GL
  v2f(vert);
#endif
#ifdef OPEN_GL
  glVertex2fv(vert);
#endif
}

inline void translatef (float x, float y, float z)
{
#ifdef IRIS_GL
  translate(x,y,z);
#endif
#ifdef OPEN_GL
  glTranslatef(x,y,z);
#endif
}

inline void translatefv (float *vert)
{
#ifdef IRIS_GL
  translate(vert[0], vert[1], vert[2]);
#endif
#ifdef OPEN_GL
  glTranslatef(vert[0], vert[1], vert[2]);
#endif
}

inline void rotatef(float rotx, float roty, float rotz)
{
#ifdef IRIS_GL
  if (rotx != 0.0)
    rotate(int(rotx * 10), 'X');
  if (roty != 0.0)
    rotate(int(roty * 10), 'Y');
  if (rotz != 0.0)
    rotate(int(rotz * 10), 'Z');
#endif
#ifdef OPEN_GL
  if (rotx != 0.0)
    glRotatef(rotx, 1.0, 0.0, 0.0);
  if (roty != 0.0)
    glRotatef(roty, 0.0, 1.0, 0.0);
  if (rotz != 0.0)
    glRotatef(rotz, 0.0, 0.0, 1.0);
#endif
}
  
inline void rotatexf(float rotx)
{
#ifdef IRIS_GL
  if (rotx != 0.0)
    rotate(int(rotx * 10), 'X');
#endif
#ifdef OPEN_GL
  if (rotx != 0.0)
    glRotatef(rotx, 1.0, 0.0, 0.0);
#endif
}
  
inline void rotateyf(float roty)
{
#ifdef IRIS_GL
  if (roty != 0.0)
    rotate(int(roty * 10), 'Y');
#endif
#ifdef OPEN_GL
  if (roty != 0.0)
    glRotatef(roty, 0.0, 1.0, 0.0);
#endif
}
  
inline void rotatezf(float rotz)
{
#ifdef IRIS_GL
  if (rotz != 0.0)
    rotate(int(rotz * 10), 'Z');
#endif
#ifdef OPEN_GL
  if (rotz != 0.0)
    glRotatef(rotz, 0.0, 0.0, 1.0);
#endif
}
  

void setAntiAlias(bool state);

void circle(float x, float y, float z, float rng, int angres = 10);

void drawStr(float x, float y, float z, char *MpStr, float scale = 1.0);

void drawStr(char *MpStr);




FONTHANDLE setFont(FONTHANDLE fh = 0);

#endif /* __GRAPHUTILS_H */
