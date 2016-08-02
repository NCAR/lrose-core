#ifndef __XUTILS_H__
#define __XUTILS_H__

#include <Xm/ToggleB.h>
#include <Xm/Scale.h>
#include <X11/X.h>

#ifdef __cplusplus
extern "C" {
#endif

  void setXmToggleState(Widget _togglewid, int state);
  int getXmToggleState(Widget _togglewid);
  void setXmScaleVal(Widget _scalewid, int _val);
  int getXmScaleVal(Widget _scalewid);
  void setXmScaleMax(Widget _scalewid, int _max);
  void setXmScaleMin(Widget _scalewid, int _max);
  int getXmScaleMax(Widget _scalewid);
  int getXmScaleMin(Widget _scalewid);
  void setXmScaleTitle(Widget _scalewid, char *title);
  void setXmScaleProps(Widget _scalewid, int _val, int _min, int _max, 
		       int _decpts, char *title);
  void setXmScaleDecPts(Widget _scalewid, int decpts);
  void NewLabel(Widget w, char *newtext);
  void setXmBackground(Widget w,  Pixel newbgcolor);
  void getXmBackground(Widget w, Pixel *bg);
#ifdef __cplusplus
}
#endif


#endif /* __XUTILS_H__ */
