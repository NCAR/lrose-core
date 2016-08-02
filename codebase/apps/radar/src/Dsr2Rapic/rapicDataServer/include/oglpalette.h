#ifndef __OGLPALETTE_H
#define __OGLPALETTE_H

#include "palette.h"

class OGlPalette {
 public:
  RGBPalette *Pal;
  char titlestr[128];
  float IndVal, IndVal2;
  bool vertAlign;
  OGlPalette(RGBPalette *pal = NULL);
  virtual ~OGlPalette();
  virtual void UpDate(DrawText *textrenderer, renderProperties *renderProps,
		      RGBPalette *pal = NULL, 
		      float indval = 0, float indval2 = 0, float minval = -9999);
  virtual void SetInd(DrawText *textrenderer, renderProperties *renderProps,
		      RGBPalette *pal = NULL,
		      float indval = 0, float indval2 = 0, bool forced = false);
};

#endif	/* __OGLPALETTE_H */
