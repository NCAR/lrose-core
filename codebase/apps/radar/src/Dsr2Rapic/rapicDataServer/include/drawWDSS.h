/*

  drawWDSS.h

*/

#ifndef __DRAW_WDSS_H
#define __DRAW_WDSS_H

#include "displ.h"
#include "drawtext.h"
#include "bool.h"
#include <time.h>
#include "wdssData.h"
#include "rdr.h"
#include "draw.h"
#include "drawTrack.h"

class drawWDSSCellAnnotSettings : public drawTrackAnnotSettings
{
 public:
  trackAnnotProps
    topP,            // top (km)
    baseP,           // base (km)
    massP,           // mass (ktons)
    dbZmaxP,         // dbZ
    dbZmaxHtP,       // km
    vilP,            //
    dirP,            // degrees
    spdP,            // km/hr
    hailP,           // hail prob/sev/size
    volP,            // volume (km^3)
    circP,           // mesoDetect type
    ddStrengthP,     // DamagingDownburst strength
    ddTypeP,         // DamagingDownburst type
    rankP;           // alg rank
  drawWDSSCellAnnotSettings(char *fname = NULL);
  virtual ~drawWDSSCellAnnotSettings();
  void init(char *fname = NULL);
  int propCountRng(float rng);  // return number of annotProps visible at rng
};

class drawWDSSMesoAnnotSettings : public drawTrackAnnotSettings
{
 public:
  trackAnnotProps
    rankP,           // alg rank
    MDATypeP,        // 
    LowTopP, 
    dirP,            // degrees
    spdP,            // km/hr
    stormCellIDP, 
    tvsIDP,
    htMaxLevDiaP,
    maxLevDiaP,
    htMaxLevGtGdVP,
    maxLevGtGdVP,
    topP,            // top (km)
    baseP,           // base (km)
    depthP;
  drawWDSSMesoAnnotSettings(char *fname = NULL);
  virtual ~drawWDSSMesoAnnotSettings();
  void init(char *fname = NULL);
  int propCountRng(float rng);  // return number of annotProps visible at rng
};

class drawWDSSTvsAnnotSettings : public drawTrackAnnotSettings
{
 public:
  trackAnnotProps
    rankP,           // alg rank
    circTypeP,
    dirP,            // degrees
    spdP,            // km/hr
    assocMesoIDP,
    assocStormIDP,
    lowLevdVP,
    htMaxLevdVP,
    maxLevdVP,
    topP,            // top (km)
    baseP,           // base (km)
    depthP;           // base (km)
  drawWDSSTvsAnnotSettings(char *fname = NULL);
  virtual ~drawWDSSTvsAnnotSettings();
  void init(char *fname = NULL);
  int propCountRng(float rng);  // return number of annotProps visible at rng
};

class drawWDSSSettings : public drawTrackSettings
{
 public:
  drawWDSSSettings(char *fname = NULL);
//  drawWDSSSettings(drawWDSSSettings *copyfrom);
  virtual ~drawWDSSSettings() {};
  void init(char *fname = NULL);
  void setDefaults();
  void setSettings(drawWDSSSettings *copyfrom);
  float maxVILHighlightVal; // VIL value for currentShapeColMax, all VIL > this will be max color
  float minVal;             // min track Val to display. Track Val is MAX value found in track
  float shapeScale;
  RGBA currentMesoColor;      // non-severe meso color
  RGBA currentMesoColSev;   // Severe meso color
  RGBA pastMesoCol;
  RGBA fwdMesoCol;  
  RGBA currentTvsColor;      // 
  float mesoRadius;
  float mesoSymbolMagnifyRng; // if the display rng is > this, magnify symbols proportionately
  float mesoSymbolLabelRng;   // if the display rng is < this, show meso label
  float mesoSymbolsOnCellRng;   // if the display rng is > this, show meso symbols on cell symbol
  float tvsSize;
  drawTrackPathMode mesoPastTrackLineMode;
  drawTrackPathMode mesoFwdTrackLineMode;
  bool showMesoSymbolOnCell;
  bool showTVSSymbolOnCell;
  bool suppressDopplerSymbols;
  bool suppressCirc;
  bool suppressLowAlt;
  bool suppressTVS;
  bool suppressDownburst;
  int minMesoSeverity;
};

class drawWDSS : public drawTrack
{
 public:
  drawWDSS(char *initfile = NULL);
  virtual ~drawWDSS();
  virtual int  getStn();
  virtual bool setEnabled(bool state = true);  // return true if state changed
  bool setWDSSClientPtrLatest(int stn);  // point to the most up to date client for stn , return true if found
  void setWDSSClientPtr(wdssTracksRadarEventHandler *wdssclient);
  void init(char *initfile = NULL);
  void copySettings(drawWDSSSettings *copyfrom);
  void copySettings(drawWDSS *copyfrom);
  /*
    render assumes the appropriate GLwDrawingAreaMakeCurrent
    call has been made to set the rendering widget and context.
    The rendering will be done with respect to the passed rendertime
  */
  void doRender(time_t rendertime, renderProperties *renderProps, int stn);
  virtual void doRender(time_t rendertime, renderProperties *renderProps, 
			int stn, float curlat, float curlng);
  virtual void doRenderAnnot(time_t rendertime, renderProperties *renderProps, 
			     int stn, float curlat, float curlng);
  drawWDSSSettings *wdssSettings;
  drawWDSSCellAnnotSettings *wdssCellAnnotSettings;
  drawWDSSMesoAnnotSettings *wdssMesoAnnotSettings;
  drawWDSSTvsAnnotSettings *wdssTvsAnnotSettings;
  void drawTrackRadarMark(float lat, float lng, char *markStr,
			  time_t rendertime,
			  renderProperties *renderProps);
  void applySettingsAllWins(); // apply the wdssSettings from this window to all drawWDSS objects

  wdssTracksRadarEventHandler *getWdssTracksHandler() 
    { return latestWDSSClient; };

  bool thisDistIsNearest(float thisdist, float other1, float other2);
  wdssCellMap renderTimeCells;
  wdssStormCellData *nearestCell(time_t rendertime, 
				 float curlat, float curlng,
				 float &dist);  // 
  wdssStormCellData *lastNearestCell;  // 
  wdssCellTrack *getNearestCellTrack(wdssStormCellData *nearestcell = NULL);
  wdssMesoMap renderTimeMesos;
  wdssMesoData* nearestMeso(time_t rendertime, 
			    float curlat, float curlng,
			    float &dist);  // 
  wdssMesoData *lastNearestMeso;  // 
  wdssMesoTrack *getNearestMesoTrack(wdssMesoData* nearestmeso);
  wdssTvsMap renderTimeTvss;
  wdssTvsData* nearestTvs(time_t rendertime, 
			  float curlat, float curlng,
			  float &dist);  // 
  wdssTvsData *lastNearestTvs;  // 
  wdssTvsTrack *getNearestTvsTrack(wdssTvsData* nearesttvs);
  wdssBaseData *lastNearestItem;
  float maxNearDist;
  virtual time_t nearestCellTime()
    {
      if (lastNearestItem)
	return lastNearestItem->tm;
      else
	return 0;
    };
  wdssCellType nearestCellType()
    {
      if (lastNearestItem)
	return lastNearestItem->cellType;
      else
	return WCT_BASE;
    };
  
  bool newCursorPosRedrawReqd(float lat, float lng);  // check for nearest annot cell change
  void newCellSelected(wdssBaseData *selcell);
 private:
 protected:
  void renderTracks(time_t rendertime,
		    renderProperties *renderProps,
		    bool renderCurrentOnly = false,
		    wdssTracksRadarEventHandler *wdssclient = 0);
  void renderCellTrack(wdssCellTrack *celltrack, 
		       time_t rendertime, time_t latestknowntime,
		    renderProperties *renderProps,
		    bool renderCurrentOnly = false);
  bool shouldRenderMesoData(wdssMesoData *mesoentry);
  void renderMesoTrack(wdssMesoTrack *mesotrack, 
		    time_t rendertime, time_t latestime,
		    renderProperties *renderProps,
		    bool renderCurrentOnly = false);
  void renderTvsTrack(wdssTvsTrack *tvstrack, 
		    time_t rendertime, time_t latestime,
		    renderProperties *renderProps,
		    bool renderCurrentOnly = false);
  void renderCellEllipse(wdssBaseData *cellentry, 
			 float cellradius,
			 time_t rendertime,
			 char * label,
			 renderProperties *renderProps);
  void renderStormCellEllipse(wdssStormCellData *cellentry, 
			 float cellradius,
			 time_t rendertime, 
			 renderProperties *renderProps);
  void renderTVSSymbol(wdssBaseData *cellentry, 
		       float cellradius,
		       time_t rendertime, 
		       char * label,
		       renderProperties *renderProps,
		       bool centred = true);
  void renderMesoArrows(wdssBaseData *cellentry, 
		       float cellradius,
		       time_t rendertime, 
		       renderProperties *renderProps);
  void renderDDArrows(wdssBaseData *cellentry, 
		       float cellradius,
		       time_t rendertime, 
		       renderProperties *renderProps);
  void renderMesoCell(wdssMesoData *cellentry, 
		      float cellradius,
		      time_t rendertime, 
		      renderProperties *renderProps);
  void renderTvsCell(wdssTvsData *cellentry, 
		     float cellradius,
		     time_t rendertime, 
		     renderProperties *renderProps);
  void renderCellFcstEllipse(wdssBaseData *cellentry,
			     float cellradius,
			     time_t fcstSecs,
			     renderProperties *renderProps);
  void renderTrackPath(wdssBaseData *prev_cellentry, 
		       wdssBaseData *cellentry, 
		       renderProperties *renderProps);
  void renderCellText(wdssStormCellData *cellentry,
		      renderProperties *renderProps,
		      drawWDSSCellAnnotSettings *annotsettings = NULL);
  void renderMesoText(wdssMesoData *cellentry,
		      renderProperties *renderProps,
		      drawWDSSMesoAnnotSettings *annotsettings = NULL);
  void renderTvsText(wdssTvsData *cellentry,
		      renderProperties *renderProps,
		      drawWDSSTvsAnnotSettings *annotsettings = NULL);
  void setCurrentProps(float VIL, 
		       float *linethickness,
		       RGBA *currentcolor); // set up current color and thickness based on VIL vs maxVIL
  float getMaxCellTextPrefixWidth(renderProperties *renderProps,
				  drawWDSSCellAnnotSettings *annotsettings);
  float getMaxMesoTextPrefixWidth(renderProperties *renderProps,
				  drawWDSSMesoAnnotSettings *annotsettings);
  float getMaxTvsTextPrefixWidth(renderProperties *renderProps,
				  drawWDSSTvsAnnotSettings *annotsettings);

  wdssTracksRadarEventHandler* latestWDSSClient;
};

#endif
