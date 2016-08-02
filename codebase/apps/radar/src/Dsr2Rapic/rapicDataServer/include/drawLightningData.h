/*
 * drawLightningData.h - 
 * */
#ifndef __DRAWLIGHTNINGDATA_H
#define __DRAWLIGHTNINGDATA_H

/*
  The drawLightningData class is simply an lightningData renderer
  i.e. it will be called repeatedly with new lightningData with 
  little state kept between renders
*/

#include "draw.h"
#include "drawtext.h"
#include "obsData.h"

extern char lightningDataSettingsFname[];


class drawLightningDataSettings
{
 public:
  drawLightningDataSettings(char *inifname = NULL);
  void setDefaults();
  void init(char *inifname = NULL);
  RGBA 
    posColor,
    negColor;
  float fullColorKAmps;   // kamps for full color
  float minColorFade;     // minimum faded factor
  RGBA bgOutlineCol;
  float  bgOutlineThickness;   // if >0 will draw bg colored outline under shapes
  float lineWidth;
  float ltningRadius;
  bool showFlash;
  bool showCircle;
  float flashSize;
  bool  useDlist;
  float scaleFactorLimit;
};  

class DrawLightningData : public DrawData {
 public:
  DrawLightningData();
  virtual ~DrawLightningData();
  bool enabled;
  bool vertsFlag;
  float flashverts[4][2]; // 0,1,2,3 - lightning "flash"
  GLuint posDlist, negDlist;
  virtual bool setEnabled(bool state = true)  // return true if state changed
    {
      if (state == enabled)
	return false;
      else
	{
	  enabled = state;
	  return true;
	}
    };
  drawLightningDataSettings settings;
  void reloadSettings();
  // render gets llboundingbox from renderProps
  // render all ltning within BB and image start/end times
  virtual void render(renderProperties &renderProps);
  virtual void doRender(renderProperties &renderProps);
  virtual void renderStrike(bool pos);
  virtual void setKAmpsColor(float kamps);
};
  
extern DrawLightningData *globalLightningRenderer;
DrawLightningData *getGlobalLightningRenderer(bool autocreate = true);
void globalLightningRender(renderProperties &renderProps);

#endif
