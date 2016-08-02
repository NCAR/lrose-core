/*

  drawTitan.h

*/

#ifndef __DRAW_TITAN_H
#define __DRAW_TITAN_H

#include "bool.h"
#include <time.h>
#include "rapicTitanClient.h"
#include "drawTrack.h"
#include <map>

class drawTitanAnnotSettings : public drawTrackAnnotSettings
{
 public:
  trackAnnotProps
    topP,            // top (km)
    baseP,           // base (km)
    massP,           // mass (ktons)
    tilt_angleP,     // deg
    tilt_dirnP,      // degT
    dbZmaxP,         // dbZ
    dbZmaxHtP,       // km
    vilP,            //
    dirP,            // degrees
    spdP,            // km/hr
    hailP,           // 
    hail2P,          // 
    volP,            // volume (km^3)
    areaP;           // area_mean (km^2)
  drawTitanAnnotSettings(char *fname = NULL);
  virtual ~drawTitanAnnotSettings();
  void init(char *fname = NULL);
  int propCountRng(TitanTrackEntry *trackentry,
		   float rng);  // return number of annotProps visible at rng
};

class drawTitanSettings : public drawTrackSettings
{
 public:
  drawTitanSettings(char *fname = NULL);
//  drawTitanSettings(drawTitanSettings *copyfrom);
  virtual ~drawTitanSettings() {};
  void init(char *fname = NULL);
  void setDefaults();
  void setSettings(drawTitanSettings *copyfrom);
  float maxVILHighlightVal; // VIL value for currentShapeColMax, all VIL > this will be max color
  float minVIL;             // min track VIL to display. Track VIL is MAX value found in track
  int titanThresh;          // select which titan thresh to use 1=thresh[0], etc
  float thresh[3];           // titan threshold dBZ levels 
  int  startTimeOfs;  // KLUDGE: titan time is end of vol, subtract this for start
  float getThresh(int threshlevel = -1);
  int getThreshIdx();
  void  setThresh(int threshlevel);
};

class drawTitan : public drawTrack
{
 public:
  drawTitan(char *initfile = NULL);
  virtual ~drawTitan();
  virtual bool setEnabled(bool state = true);  // return true if state changed
  void setThresh(int threshidx);
  virtual int  getStn();
  virtual int  getArea();
  int getThreshIdx();
  float getThresh(int threshidx = -1);
  bool setTitanClientPtrLatest(int stn, bool area_flag = false);  // point to the most up to date client for stn , return true if found
  void setTitanClientPtr(rapicTitanClient *titanclient);
  void init(char *initfile = NULL);
  void copySettings(drawTitanSettings *copyfrom);
  void copySettings(drawTitan *copyfrom);
  /*
    render assumes the appropriate GLwDrawingAreaMakeCurrent
    call has been made to set the rendering widget and context.
    The rendering will be done with respect to the passed rendertime
  */
  void doRender(time_t rendertime, renderProperties *renderProps, int stn, bool area_flag);
  virtual void doRender(time_t rendertime, renderProperties *renderProps, int stn, 
			float curlat, float curlng, bool area_flag);
  virtual void doRenderAnnot(time_t rendertime, renderProperties *renderProps, int stn, 
			float curlat, float curlng, bool area_flag);
  virtual bool newCursorPosRedrawReqd(float lat, float lng); // return true if cursor pos requires redraw

  drawTitanSettings *titanSettings;
  drawTitanAnnotSettings *titanAnnotSettings;
  void drawTrackRadarMark(float lat, float lng, char *markStr,
			  time_t rendertime,
			  renderProperties *renderProps);
  virtual drawTrackShapeMode getShapeMode();
  virtual drawTrackShapeMode nextShapeMode();

  TitanServer *lastRenderTitanServer;

  titanCellMap renderTimeCells;
  TitanTrackEntry *_nearestCell;   
  TitanTrackEntry *nearestCell();  // return currently defined nearest cell
  TitanTrackEntry *nearestCell(float curlat, float curlng,
			       float &dist);
  virtual time_t nearestCellTime()
    {
      if (_nearestCell)
	return _nearestCell->entry().time;
      else
	return 0;
    };
  
  TitanSimpleTrack *_nearestTrack;
  TitanSimpleTrack *nearestTrack(TitanTrackEntry *nearestcell = NULL);
  void newCellSelected(TitanTrackEntry *selcell);
  
  float maxNearDist;

 private:
 protected:
  void renderTracks(time_t rendertime, 
		    renderProperties *renderProps, 
		    bool renderCurrentOnly = false,
		    TitanServer *titanserver = 0);
  void renderComplexTrack(TitanComplexTrack *complextrack, 
			  time_t rendertime, time_t latestscantime,
			  renderProperties *renderProps,
			 int trackNum, bool renderCurrentOnly = false);
  float getMaxVilInSimpleTrack(TitanSimpleTrack *simpletrack);
  void renderSimpleTrack(TitanSimpleTrack *simpletrack, 
			 time_t rendertime, time_t latestscantime,
			 renderProperties *renderProps,
			 int trackNum,
			 bool renderCurrentOnly = false);
  void renderTrackEntry(TitanTrackEntry *trackentry,
			time_t rendertime, 
			renderProperties *renderProps,
			drawTrackShapeMode shapemode);
  void renderPolygon(TitanTrackEntry *trackentry, 
		     time_t rendertime, 
		     renderProperties *renderProps);
  void renderEllipse(TitanTrackEntry *trackentry, 
		     time_t rendertime, 
		     renderProperties *renderProps);
  void renderFcstEllipse(TitanTrackEntry *trackentry, 
			 time_t fcstSecs,
			 renderProperties *renderProps);
  void renderTrackPath(TitanTrackEntry *prev_trackentry, 
		       TitanTrackEntry *trackentry, 
		       renderProperties *renderProps);
  void renderCellText(TitanTrackEntry *trackentry, 
		      renderProperties *renderProps,
		      drawTitanAnnotSettings *annotsettings = NULL);
  void setCurrentProps(float VIL, 
		       float *linethickness,
		       RGBA *currentcolor); // set up current color and thickness based on VIL vs maxVIL
  float getMaxCellTextPrefixWidth(TitanTrackEntry *trackentry,
				  renderProperties *renderProps,
				  drawTitanAnnotSettings *annotsettings);

  rapicTitanClient* latestTitanClient;
  si32 grid_type; /* TITAN_PROJ_FLAT or TITAN_PROJ_LATLON */
  fl32 origin_lat, origin_long;
};

#endif
