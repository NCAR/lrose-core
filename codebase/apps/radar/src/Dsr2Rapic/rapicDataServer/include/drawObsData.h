/*
 * drawObsData.h - 
 * */
#ifndef __DRAWOBSDATA_H
#define __DRAWOBSDATA_H

/*
  The DrawObsData class is simply an obsData renderer
  i.e. it will be called repeatedly with new obsData with 
  little state kept between renders
*/

#include "draw.h"
#include "drawtext.h"
#include "obsData.h"

extern char obsDataSettingsFname[];


class drawObsDataSettings
{
 public:
  drawObsDataSettings(char *inifname = NULL);
  void setDefaults();
  void init(char *inifname = NULL);
  bool 
    tempEnabled, 
    DPEnabled,
    rain10Enabled,
    rain9amEnabled,
    WGEnabled;
  float 
    tempShowRng,
    DPShowRng,
    rainShowRng,
    WGShowRng;
  RGBA 
    currentColor,
    olderColor,
    newerColor;
  time_t 
    maxDisplayAge,   // don't draw if older/newer than this (secs)
    maxAgePeriod;    // seconds for max Age color
  RGBA bgOutlineCol;
  float  bgOutlineThickness;   // if >0 will draw bg colored outline under shapes
  float fontSize;
  int  fontID;
  float barbLength;
  float barbWidth;
  float featherLength;  // if not defined use barbLength/3.0
  float calmWidth;
  float calmRadius;
  float calmSpeed;
  bool  useAntiAlias;
  float scaleFactorLimit;
  // factor to scale obs that exceeds wind speed warning threshold
  float warningThreshold[3];
  float warningScaleFactor[3];  
  RGBA  warningColor[3];
};  

class DrawObsData : public DrawData {
 public:
  DrawObsData();
  virtual ~DrawObsData();
  bool enabled;
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
  drawObsDataSettings settings;
  void reloadSettings();
  void setVerts();
  bool vertsflag;   // are verts set?
  float barbverts[6][2]; // 0,1,2,3 outline, 4,5 age line (1-barblength*age)
  float triverts[3][2];
  float featherverts[2][2];
  float calmverts[2][2];
  float featherdy;
#ifdef USE_GLF
  DrawGLFText *textRenderer;
  void setTextRenderer(DrawGLFText *textrenderer = NULL)
    {
      if (textrenderer)
	textRenderer = textrenderer;
      else
	textRenderer = getDefaultGLFTextRenderer();
    };
  bool textRendererSet() { return textRenderer != 0; };
#endif
  // render gets llboundingbox from renderProps
  //   will get  stnObsDataSet from getGlobalObsData and render nearest
  // to renderProps time for each in set
  virtual void render(renderProperties &renderProps);
  virtual void doRender(renderProperties &renderProps);
  virtual bool doRenderObsBarb(baseObsData* obsdata,
			       renderProperties &renderProps,
			       bool background = false);
  virtual bool doRenderObsCalm(baseObsData* obsdata,
			       renderProperties &renderProps,
			       bool background = false);
  virtual void doRenderObsFeathers(baseObsData* obsdata,
				   renderProperties &renderProps);
  virtual void setAgeColor(time_t secs_old, float minFactor = 0.4);
  virtual void doRenderObsText(baseObsData* obsdata,
			       renderProperties &renderProps);
  
};
  
extern DrawObsData *globalObsRenderer;
DrawObsData *getGlobalObsRenderer(bool autocreate = true);
void globalObsRender(renderProperties &renderProps);

#endif
