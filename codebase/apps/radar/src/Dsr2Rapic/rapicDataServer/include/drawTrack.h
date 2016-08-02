/*

  drawTrack.h

*/

#ifndef __DRAW_TRACK_H
#define __DRAW_TRACK_H

#include "bool.h"
#include <time.h>
#include "rdr.h"
#include "draw.h"
#include "drawtext.h"
#include "displ.h"

enum drawTrackShapeMode
{
  rpTrack_NoShape,
  rpTrack_ShapeEllipse,
  rpTrack_ShapePolygon,
};

enum drawTrackPathMode
{
  rpTrack_NoTrackPath,
  rpTrack_PathLineOnly,
  rpTrack_PathLineArrow
};

enum drawTrackAnnotMode
  {
    rpTrack_Annot_ShowAll,
    rpTrack_Annot_ShowCurPos,
    rpTrack_Annot_ShowCurClick,
    rpTrack_Annot_ShowNone
  };
    

class trackAnnotProps
{
 public:
  bool enabled;        // if not enabled, don't display
  float showAtRng;     // if display rng > this - don't display
  char *prefix;        // prefix string
  char *suffix;        // suffix string
  float lastPrefixWidth;
  float lastPrefixWidthScale;

  void setPrefix(char *str)
    {
      if (prefix)
	free(prefix);
      if (str)
	prefix = strdup(str);
      lastPrefixWidth = 0;
      lastPrefixWidthScale = 0;
    };
  void setSuffix(char *str)
    {
      if (suffix)
	free(suffix);
      if (str)
	suffix = strdup(str);
    };
#ifdef USE_GLF
  float getPrefixWidth(DrawGLFText *textRenderer, 
		       renderProperties *renderProps);
  // check will check whether to display at rng
  // and if so if prefix width exceeds maxwidth will update maxwidth
  void checkPrefixWidth(DrawGLFText *textRenderer, 
			renderProperties *renderProps,
			float rng,
			float &maxwidth);
#endif
  bool show(float rng)         // return true if enabled and rng <= showAtRng
    {
      return (enabled && (rng <= showAtRng));
    };
  trackAnnotProps(bool en = true, float showrng = 64, char *prefixstr = "", char *suffixstr = "")
    {
      prefix = NULL;
      suffix = NULL;
      set(en, showrng, prefixstr, suffixstr);
      lastPrefixWidth = 0;
      lastPrefixWidthScale = 0;
    }
  ~trackAnnotProps()
    {
      if (prefix)
	free(prefix);
      if (suffix)
	free(suffix);
    };
  void set(bool en = true, float showrng = 64, char *prefixstr = "", char *suffixstr = "")
    

    {
      enabled = en;
      showAtRng = showrng;
      setPrefix(prefixstr);
      setSuffix(suffixstr);
      lastPrefixWidth = 0;
      lastPrefixWidthScale = 0;
    };

  // annotString constructs a string into the passed char buffer
  // returns false if not enabled or rng <= show rng
  bool annotString(char *buffer, int buffsize, float rng, char *c_fmt_str, 
		   float val, bool val_is_flag = false);
  // annotString constructs a string into the passed char buffer
  // returns false if not enabled or rng <= show rng
  bool annotString(char *buffer, int buffsize, float rng, char *c_fmt_str, 
		   float val, char *suff1, float val2, char *suff2);
  // this version uses the prefix with the passed c_string
  bool annotString(char *buffer, int buffsize, float rng, char *c_str);
};

class drawTrackAnnotSettings
{
 public:
  trackAnnotProps
    stormNumP;       // 
  drawTrackAnnotSettings(char *fname = NULL);
  virtual ~drawTrackAnnotSettings();
  virtual void init(char *fname = NULL);
  virtual int propCountRng(float rng);  // return number of annotProps visible at rng
};

class drawTrackSettings
{
 public:
  drawTrackSettings(char *fname = NULL);
//  drawTrackSettings(drawTrackSettings *copyfrom);
  virtual ~drawTrackSettings() {};
  virtual void init(char *fname = NULL);
  drawTrackShapeMode pastShapeMode;
  drawTrackShapeMode currentShapeMode;
  drawTrackShapeMode fwdShapeMode;
  float shapeThickness,
    shapeThicknessCurrentMin, shapeThicknessCurrentMax;
  bool fillShape;
  float  bgOutlineThickness;   // if >0 will draw bg colored outline under shapes
  float  bgOutlineThicknessCurrent;   // if >0 will draw bg colored outline under shapes
  time_t currentTimeWin;     // +/-seconds for current
  time_t timeBefore;         // seconds of "history" to render. if 0 render whole track
  time_t timeAfter;
  time_t fcstStepSecs;
  bool   drawFcstOnly;       // if set will draw fcst only from render time, NO Fwd
  bool drawFcstOnlyLocked; // if set will draw fcst only from render time, NO Fwd and prevent kbd from changing to fwd
  void   setDrawFcstOnly(bool state=true)
    {
      if (drawFcstOnlyLocked && !state) // don't allow false state
	{
	  fprintf(stdout, "setDrawFcstOnly false failed - drawFcstOnlyLocked is set\n");
	  drawFcstOnly = true;
	}
      else 
	drawFcstOnly = state;
    };

  drawTrackPathMode pastTrackLineMode;
  drawTrackPathMode fwdTrackLineMode;
  float trackThickness;
  bool enableFade;
  bool enableAnnot;
  drawTrackAnnotMode annotMode;
  float singleAnnotScale;
  float maxFadeFactor;
  float fontSize;
  int  fontID;
  float trackSymbolMagnifyRng; // if the display rng is > this, magnify symbols proportionately
  float trackSymbolMagnifyStdSize; // if the display rng is > this, magnify symbols proportionately
  int  fcstArcLength;       // arc length +/- degrees, default is 85

  virtual void setDefaults();
  virtual void setSettings(drawTrackSettings *copyfrom);

  RGBA currentShapeColMin;
  RGBA currentShapeColMax;
  RGBA pastShapeCol;
  RGBA fwdShapeCol;  // fwd shapes are for actual track entries beyond the render time
  RGBA fcstShapeCol;    // fcast shapes are for faorecast track extraoplations based on speed/dir 
  RGBA trackCol;
  RGBA fwdTrackCol;
  RGBA fcstTrackCol;
  RGBA textPropsCol;
  RGBA bgOutlineCol;
};


class drawTrack : public DrawData
{
 public:
  drawTrack(char *fname = NULL);
  virtual ~drawTrack();
  int trackStn;
  bool enabled;
  virtual bool setEnabled(bool state = true);  // return true if state changed
  virtual void setStn(int stn);
  virtual int  getStn();
  virtual void init(char *fname = NULL);
  virtual void copySettings(drawTrackSettings *copyfrom);
  virtual void copySettings(drawTrack *copyfrom);
  /*
    render assumes the appropriate GLwDrawingAreaMakeCurrent
    call has been made to set the rendering widget and context.
    The rendering will be done with respect to the passed rendertime
  */
  virtual void doRender(time_t rendertime, renderProperties *renderProps, 
			int stn);
  virtual void doRender(time_t rendertime, renderProperties *renderProps, 
			int stn, 
			float curlat, float curlng);
  virtual void doRenderAnnot(time_t rendertime, renderProperties *renderProps, 
			     int stn, 
			     float curlat, float curlng);

  virtual bool newCursorPosRedrawReqd(float lat, float lng); 
  // return true if cursor pos requires redraw

  virtual void renderTracks(time_t rendertime, 
		    renderProperties *renderProps);
  drawTrackSettings *settings;
  drawTrackAnnotSettings *annotSettings;
  virtual void drawTrackRadarMark(float lat, float lng, char *markStr,
			  time_t rendertime,
			  renderProperties *renderProps);
  virtual drawTrackShapeMode getShapeMode();
  virtual drawTrackShapeMode nextShapeMode();
  virtual time_t nearestCellTime()
    {
      return 0;
    };
#ifdef USE_GLF
  void setTextRenderer(DrawGLFText *textrenderer)
    {
      textRenderer = textrenderer;
    };
  bool textRendererSet() { return textRenderer != 0; };
#endif
  virtual void          setParentWin(DisplWin *parentwin);
  virtual DisplWin*     getParentWin();
  virtual void          settingsChanged();

 private:
 protected:
  virtual float getMaxCellTextPrefixWidth(renderProperties *renderProps,
				  drawTrackAnnotSettings *annotsettings);

#ifdef USE_GLF
  DrawGLFText   *textRenderer;
#endif
  DisplWin      *parentWin;
};

#endif
