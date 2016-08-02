#ifndef OGLWID_H
#define OGLWID_H

#include <X11/IntrinsicP.h>
#include <GL/glx.h>
#include <stdio.h>

extern GLXContext sharedOGLContext;

void setSharedOGLContext(GLXContext sharedcontext);

class oglwid_props {
 public:
  Widget	glWidget;
  GLXContext	oglwin_context;
  Window	oglwin_window;
  Display*	oglwin_display;
  long		oglwin_width, oglwin_height;
  long		oglwin_xorg, oglwin_yorg;
  XVisualInfo   *visualInfo;
  int           colormap_size; // from visualInfo
  // the following are from glXGetConfig
  int           auxBuffers;
  int           buffer_size;
  int           alpha_size;
  int           depth_size;
  int           rgba_mode;
  int           doublebuffer;
  int           red_size;
  int           green_size;
  int           blue_size;
  int           stencil_size;
  //	Colormap	oglwin_cmap;
  oglwid_props()
    {
      oglwin_context = 0;
      glWidget = 0;
      oglwin_window = 0;
      oglwin_display = 0;
      oglwin_width = oglwin_height = 0;
      oglwin_xorg = oglwin_yorg = 0;
      visualInfo = 0;
      auxBuffers = 0;
      colormap_size = 0;
      buffer_size = 0;
      alpha_size = 0;
      depth_size = 0;
      rgba_mode = 0;
      doublebuffer = 0;
      red_size = 0;
      green_size = 0;
      blue_size = 0;
      stencil_size = 0;
    }
      
};

#endif
