// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/////////////////////////////////////////////////////////////
// SigAirMet.hh
//
// C++ class for dealing with SIGMETs and AIRMETs.
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// Jan 2003
//////////////////////////////////////////////////////////////

#ifndef _SigAirMet_hh
#define _SigAirMet_hh

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <toolsa/MemBuf.hh>
#include <dataport/port_types.h>

using namespace std;

#define SIGAIRMET_HDR_NBYTES_32 96
#define SIGAIRMET_ID_NBYTES 16
#define SIGAIRMET_SOURCE_NBYTES 16
#define SIGAIRMET_QUALIFIER_NBYTES 32
#define SIGAIRMET_WX_NBYTES 32
#define SIGAIRMET_FIR_NBYTES 16

typedef enum {
  AIRMET_GROUP = 1,
  SIGMET_GROUP = 2
} sigairmet_group_t;

typedef enum {
  SIGAIRMET_NEW = 1,
  SIGAIRMET_AMENDMENT = 2
} sigairmet_action_t;

typedef struct {
  fl32 lat;
  fl32 lon;
} sigairmet_vertex_t;

typedef struct {
  si32 time;
  fl32 lat;
  fl32 lon;
  si32 id;
} sigairmet_forecast_t;

typedef struct {
  si32 group;       // sigairmet_group_t
  si32 action;      // sigairmet_action_t
  ti32 issue_time;
  ti32 start_time;
  ti32 end_time;
  si32 cancel_flag; // 1 if cancelled
  ti32 cancel_time; // time cancelled
  si32 flevels_set;
  fl32 bottom_flevel;
  fl32 top_flevel;
  si32 centroid_set;
  fl32 centroid_lat;
  fl32 centroid_lon;
  si32 fir_set;
  si32 text_len;
  fl32 movement_speed; // km/hr, -1 if not set
  fl32 movement_dirn;  // deg T, -1 if not set
  si32 n_vertices;
  si32 n_forecasts;
  si32 n_outlooks;
  si32 polygon_is_fir_bdry;
  si32 obs_and_or_fcst;  // 1=obs, 2=fcst, 3=obs and fcst
  ti32 obs_time; // observed time if available
  ti32 fcast_time; // forecast time if available (from body, i.e. in place of obs time - 2010 Sigmet changes)
  si32 spare[8];
  char id[SIGAIRMET_ID_NBYTES];
  char source[SIGAIRMET_SOURCE_NBYTES];
  char qualifier[SIGAIRMET_QUALIFIER_NBYTES];
  char wx[SIGAIRMET_WX_NBYTES];
  char fir[SIGAIRMET_FIR_NBYTES];
} sigairmet_hdr_t;

class SigAirMet {

public:

  // constructor

  SigAirMet();

  // destructor

  ~SigAirMet();

  // clear the object

  void clear();

  // set methods

  void setId(const string &id);
  void setSource(const string &source);
  void setQualifier(const string &qualifier);
  void setWx(const string &wx);
  void setFir(const string &fir);
  void setText(const string &text);
  void setObsAndOrFcst(int obsfcst);

  void setGroup(sigairmet_group_t group);
  void setAction(sigairmet_action_t action);

  void setIssueTime(time_t issue_time);
  void setStartTime(time_t start_time);
  void setEndTime(time_t end_time);
  void setObsTime(time_t obs_time);
  void setFcastTime(time_t fcast_time);

  void setCancelFlag(bool cancel_flag);
  void setCancelTime(time_t cancel_time);

  void setPolygonIsFirBdry();

  void setFlightLevels(double bottom_flevel, double top_flevel);
  void setCentroid(double lat, double lon);
  
  void setMovementSpeed(double speed);
  void setMovementDirn(double dirn);

  // add vertex
  
  void addVertex(double lat, double lon);

  // add forecast
  
  void addForecast(time_t time, double lat, double lon, int id = 0);

  // add outlook
  
  void addOutlook(time_t time, double lat, double lon, int id = 0);

  // clear vertices, forecasts and outlooks

  void clearVertices();
  void clearForecasts();
  void clearOutlooks();

  // compute the centroid from the vertices
  // add the vertices first

  int computeCentroid();

  // data get methods

  const char *getId() const { return _hdr.id; }
  const char *getSource() const { return _hdr.source; }
  const char *getQualifier() const { return _hdr.qualifier; }
  const char *getWx() const { return _hdr.wx; }
  const char *getFir() const { return _hdr.fir; }
  const string &getText() const { return _text; }

  sigairmet_group_t getGroup() const { return (sigairmet_group_t) _hdr.group; }
  static sigairmet_group_t getGroup(const string &groupStr);

  sigairmet_action_t getAction() const { return (sigairmet_action_t) _hdr.action; }
  static sigairmet_action_t getAction(const string &actionStr);

  time_t getIssueTime() const { return _hdr.issue_time; };
  time_t getStartTime() const { return _hdr.start_time; };
  time_t getEndTime() const { return _hdr.end_time; }
  time_t getObsTime() const { return _hdr.obs_time; }
  time_t getFcastTime() const { return _hdr.fcast_time; }

  bool getCancelFlag() const { return _hdr.cancel_flag; }
  time_t getCancelTime() const { return _hdr.cancel_time; }

  bool flightLevelsSet() const { return _hdr.flevels_set; }
  double getBottomFlightLevel() const { return _hdr.bottom_flevel; }
  double getTopFlightLevel() const { return _hdr.top_flevel; }
  
  bool centroidSet() const { return _hdr.centroid_set; }
  double getCentroidLat() const { return _hdr.centroid_lat; }
  double getCentroidLon() const { return _hdr.centroid_lon; }
  
  bool firSet() const { return _hdr.fir_set; }
  bool polygonIsFirBdry() const { return _hdr.polygon_is_fir_bdry; }

  double getMovementSpeed() const { return _hdr.movement_speed; }
  double getMovementDirn() const { return _hdr.movement_dirn; }

  size_t getNVertices() const { return _vertices.size(); }
  const vector<sigairmet_vertex_t> &getVertices() const { return _vertices; }

  size_t getNForecasts() const { return _forecasts.size(); }
  const vector<sigairmet_forecast_t> &getForecasts() const { return _forecasts; }

  size_t getNOutlooks() const { return _outlooks.size(); }
  const vector<sigairmet_forecast_t> &getOutlooks() const { return _outlooks; }

  ///////////////////////////////////////////
  // assemble()
  // Load up the buffer from the object.
  // Handles byte swapping.
  
  void assemble();

  ///////////////////////////////////////////////////////////
  // disassemble()
  // Disassembles a buffer, sets the values in the object.
  // Handles byte swapping.
  // Returns 0 on success, -1 on failure

  int disassemble(const void *buf, int len);

  // get the assembled buffer info

  void *getBufPtr() const { return _memBuf.getPtr(); }
  int getBufLen() const { return _memBuf.getLen(); }
  
  ///////////////////////////////////////////
  // load XML from object
  
  void loadXml(string &xml, int startIndentLevel = 0);
  
  /////////////////////////
  // printing
  
  void print(ostream &out, string spacer = "") const;
  const char *group2String() const;
  static const char *group2String(sigairmet_group_t group);
  const char *action2String() const;
  static const char *action2String(sigairmet_action_t action);

  // Print an XML representation of the object.
  
	void printAsXml(ostream &out, int startIndentLevel = 0);

  // available weather types

protected:

private:

  sigairmet_hdr_t _hdr;
  string _text;
	string _text_without_header;
	string _wmo_header;
  mutable string _xml;
  MemBuf _memBuf;

  vector<sigairmet_vertex_t> _vertices;
  vector<sigairmet_forecast_t> _forecasts;
  vector<sigairmet_forecast_t> _outlooks;

  void _printHdr(ostream &out, string spacer = "") const;
  void _hdrToBE(sigairmet_hdr_t &hdr);
  void _hdrFromBE(sigairmet_hdr_t &hdr);
  void _vertexToBE(sigairmet_vertex_t &vertex);
  void _vertexFromBE(sigairmet_vertex_t &vertex);
  void _forecastToBE(sigairmet_forecast_t &forecast);
  void _forecastFromBE(sigairmet_forecast_t &forecast);
  void _conditionLongitudes();

	// prep for XML
	void _splitTextByHeader();
	string _trimString(const string& str, const string& whitespace = " \t\r\n");
};


#endif
