/*

  drawFlightTrack.h

*/

#ifndef __DRAW_FLIGHTTRACK_H
#define __DRAW_FLIGHTTRACK_H

#include "drawTrack.h"

/* 36   1001                              {NLHEAD  FFI} */
/* Belyaev Gennady */
/* MDB */
/* UCSE */
/* Troccinox */
/* 1   1                                  {IVOL   NVOL} */
/* 2005 02 04   2005 02 05                {DATE  RDATE} */
/* 0                                      {DX(1)} */
/* TIME (UT SECONDS) FROM 00 HRS ON DATE GIVEN IN LINE 7 */
/*  21                                    {NV} */
/*  1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 */
/* 999 999 999 99999 999 9999 99 99 999 999 9999 9999 9999 9999 999 999999 */

/* 99999 999999999 9999999999999999 99999 99999999  */

/* Tout (C)            {Out-boarding air temperature} */
/* Pstat (hPa)         {Static pressure} */
/* Vtrue (km/h)        {True air speed} */
/* Halt (meter)        {True altitude} */
/* Dr_ang (degree)     {Drift angle} */
/* Head (degree)       {True heading} */
/* Pitch (degree)      {Pitch angle} */
/* Attack (degree)     {Attack angle} */
/* Sl_ang (degree)     {Slip angle} */
/* Uwind (m/s)         {Absolute value of wind velocity} */
/* W_dir (degree)      {Wind direction} */
/* Roll (degree)       {Roll angle} */
/* Lat (degree)        {Latitude} */
/* Long (degree)       {Longitude} */
/* Gr_sp (km/h)        {Ground speed} */
/* Tr_ang (degree)     {Track angle} */
/* Alt (m)             {Altitude up MSL} */
/* SS1 (NOUNIT)        {State word #1} */
/* SS2 (NOUNIT)        {State word #2} */
/* SRK (NOUNIT)        {Word of one-time instruction} */
/* SYS_TIME (seconds)  {System Time} */
/* 0                                      {NSCOML} */
/* 1                                      {NNCOML} */

/* TIME    Tout Pstat  Vtrue  Halt  Dr_ang Head    Pitch Attac Sl_ang Uwind W_dir   Roll    Lat       Long      Gr_sp Tr_ang Alt   SS1       SS2      SRK   SYS_TIME */

/* 59977 32.81  965.59 64.63  392   -22.59 0.00    2.81  0.00  0.00   1.97  211.97  1.11    -21.1437  -50.4279  0.1   181    413   111111111 1100100111101111 11110 33825161 */

/* 59978 32.81  965.59 60.96  392   -22.59 0.00    2.80  0.00  0.00   2.13  211.99  1.11    -21.1437  -50.4279  0.1   178    413   111111111 1100100111101111 11110 33825162 */

/* 59979 32.81  965.59 64.04  392   -22.59 0.00    2.81  0.00  0.00   2.20  211.97  1.11    -21.1437  -50.4279  0.1   179    414   111111111 1100100111101111 11110 33825163 */

/* 59980 32.81  965.59 64.67  392   -22.59 0.00    2.80  0.00  0.00   2.07  211.99  1.11    -21.1437  -50.4279  0.1   179    414   111111111 1100100111101111 11110 33825164 */

class flightTrackDataNode
{
 public:
  time_t time;
  float lat, lng, alt, heading, roll, pitch;
  float coord[3];   // global earth xyz(km)
  //  float wind_vel, wind_dir;
  flightTrackDataNode()
    {
      time = 0; 
      lat = lng = alt = roll = pitch = 0.0;
      heading = -1.0;
      coord[0] = coord[1] = coord[2] = 0.0;
    };
  flightTrackDataNode(time_t _time, float _lat, float _lng, 
		      float _alt, float _heading = -1.0, 
		      float _roll = 0.0, float _pitch = 0.0)
    {
      set(_time, _lat, _lng, _alt, _heading, _roll, _pitch);
    };
  void set(time_t _time, float _lat, float _lng, 
	   float _alt, float _heading = -1.0, 
	   float _roll = 0.0, float _pitch = 0.0)
    {
      time = _time; 
      lat = _lat;
      lng = _lng;
      alt = _alt;
      heading = _heading;
      roll = _roll;
      pitch = _pitch;
      projectPos();
    };
  void projectPos();  //project lat/long/ht to global xyz
};

class flightTrackData
{
  spinlock *lock;
 public:
  string filename;
  string label;
  RGBA trackColor;
  RGBA fwdTrackColor;
  RGBA iconColor;
  void init();
  int readFile(char *fname = NULL, int _interval = 0); // return number of nodes read  
  int readFile2(char *fname = NULL, int _interval = 0); // return number of nodes read  
  int refresh();
  std::map<time_t, flightTrackDataNode> flightData;
  time_t startTime, endTime, interval;
  void dump(char *fname = NULL);
  flightTrackData();
  flightTrackData(const flightTrackData &ftrack);
};

class drawFlightTrack : public drawTrack
{
 public:
  drawFlightTrack(char *fname = NULL);
  virtual ~drawFlightTrack();
  virtual void doRender(time_t rendertime, renderProperties *renderProps, 
			flightTrackData &trackdata);
  virtual void doRender(time_t rendertime, renderProperties *renderProps, 
			flightTrackData &trackdata, 
			float curlat, float curlng);
  virtual void renderTracks(time_t rendertime, 
			    renderProperties *renderProps, 
			    flightTrackData &trackdata,  
		    bool renderCurrentOnly = false);
  void renderFlightPath(time_t rendertime, renderProperties *renderProps, 
			flightTrackData &trackdata);
  void renderCurrentPosIcon(time_t rendertime, renderProperties *renderProps, 
			    flightTrackDataNode &currentNode,
			    flightTrackData &trackdata);
};

class flightTrackManager
{
  spinlock *lock;
 public:
  string inifile;
  flightTrackManager(char *_inifile = NULL);
  ~flightTrackManager();
  map<string, flightTrackData> flightTracks;
  drawFlightTrack flightTrackRenderer;
  void init(char *_inifile = NULL);
  int addFlightTrack(char *flight_fname, int interval = 0);
  int addFlightTrackSimple(char *initstr, int interval = 0);
  void refresh();
  void checkRefresh();
  void dump();
  bool getLock();
  void relLock();
  void doRender(time_t rendertime, renderProperties *renderProps, DrawGLFText *textrenderer);
  void doRender(time_t rendertime, renderProperties *renderProps, DrawGLFText *textrenderer, 
		float curlat, float curlng);
};

extern flightTrackManager *flightManager;
extern bool refreshFlightTrackRequest;
#endif
