/*

  rpRenderTitanOGL.h

*/

#include "bool.h"
#include <time.h>
#include "rpRender.h"
#include "rapicTitanClient.h"
#include "rdr.h"
#include "draw.h"

enum rpRenderTitanShapeMode
{
  rpTitan_NoShape,
  rpTitan_ShapeEllipse,
  rpTitan_ShapePolygon,
};

enum rpRenderTitanTrackPath
{
  rpTitan_NoTrackPath,
  rpTitan_PathLineOnly,
  rpTitan_PathLineArrow
};

enum rpRenderAnnotMode
{
  rpTitan_NoAnnot,
  rpTitan_Annot_Vel,
  rpTitan_Annot_TrackNo,
  rpTitan_Annot_VIL,
  rpTitan_Annot_MaxdBZ
};

class rpRenderTitanSettings
{
 public:
  rpRenderTitanSettings();
  rpRenderTitanSettings(rpRenderTitanSettings *copyfrom);
  rpRenderTitanShapeMode shapeMode;
  float shapeThickness;
  bool fillShape;
  time_t timeBefore;  // seconds of "history" to render. if 0 render whole track
  time_t timeAfter;
  rpRenderTitanTrackPath trackLineMode;
  float trackThickness;
  rpRenderAnnotMode annotMode;
  
  void setDefaults();
  void setSettings(rpRenderTitanSettings *copyfrom);
  

  RGBA currentShapeCol;
  RGBA pastShapeCol;
  RGBA fcstShapeCol;
  RGBA trackCol;
  RGBA fcastTrackCol;

};


class rpRenderTitanOGL : public rpRender
{
 public:
  rpRenderTitanOGL(rapicTitanClient *titanclient = 0);  
  ~rpRenderTitanOGL();
  void setTitanClient(rapicTitanClient *titanclient);
  void init();
  void copySettings(rpRenderTitanSettings *copyfrom);
  void copySettings(rpRenderTitanOGL *copyfrom);
  /*
    render assumes the appropriate GLwDrawingAreaMakeCurrent
    call has been made to set the rendering widget and context.
    The rendering will be done with respect to the passed rendertime
  */
  void render(time_t rendertime, renderProperties *renderProps);
  rpRenderTitanSettings *settings;

 private:
 protected:
  void renderTracks(time_t rendertime, renderProperties *renderProps, TitanServer *titanserver = 0);
  void renderComplexTrack(TitanComplexTrack *complextrack, time_t rendertime, 
			  renderProperties *renderProps);
  void renderSimpleTrack(TitanSimpleTrack *simpletrack, time_t rendertime, 
			 renderProperties *renderProps);
  void renderTrackEntry(TitanTrackEntry *trackentry, time_t rendertime, 
			renderProperties *renderProps);
  void renderPolygon(TitanTrackEntry *trackentry, time_t rendertime, 
		     renderProperties *renderProps);
  void renderEllipse(TitanTrackEntry *trackentry, time_t rendertime, 
		     renderProperties *renderProps);
  void renderTrackPath(TitanTrackEntry *prev_trackentry, TitanTrackEntry *trackentry, 
		       renderProperties *renderProps);

  rapicTitanClient *titanClient;
  si32 grid_type; /* TITAN_PROJ_FLAT or TITAN_PROJ_LATLON */
  fl32 origin_lat, origin_long;
};
