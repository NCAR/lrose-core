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
///////////////////////////////////////////////////////////////
// Server.hh
//
// FileServerobject
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2000
//
///////////////////////////////////////////////////////////////


#ifndef _Server_HH
#define _Server_HH

#include <string>
#include <map>

#include <Spdb/DsSymprodServer.hh>
#include <Spdb/Symprod.hh>
#include <toolsa/MemBuf.hh>
#include <rapformats/Taf.hh>

#include "Filter.hh"
#include "Params.hh"
#include "IconDef.hh"

using namespace std;

class Server : public DsSymprodServer
  
{

public:
    
  // Constructor:                       
  //   o Registers with procmap
  //   o Opens socket on specified port              
  
  Server(const string &prog_name,
	 Params *initialParams);
  
  // destructor

  virtual ~Server();
  
protected:

  // parameters

  const Params *_params;

  // reference time of the reqest from the client

  time_t _requestTime;

  // TAF and its periods

  Taf _taf;
  Taf::ForecastPeriod _normalPeriod;
  Taf::ForecastPeriod _tempoPeriod;
  bool _tempoActive;

  // lookup table for spatial decimation

  bool **_locOccupied;

  // color filtering

  Filter *_filter;

  // members containing rendering information

  Symprod::vert_align_t _symprodVertAlign;
  Symprod::horiz_align_t _symprodHorizAlign;
  Symprod::font_style_t _symprodFontStyle;
  
  Symprod::vert_align_t _hiddenTextVertAlign;
  Symprod::horiz_align_t _hiddenTextHorizAlign;
  Symprod::font_style_t _hiddenTextFontStyle;
  
  // methods invoked from the base class for managing 
  // local override of parameter file
  
  int loadLocalParams( const string &paramFile, void **params);
  void freeLocalParams( void *params ) { delete (Params*)params; }
  
  // overload transformData()
  
  void transformData(const void *params,
                     const string &dir_path,
                     const int prod_id,
                     const string &prod_label,
                     const int n_chunks_in,
                     const Spdb::chunk_ref_t *chunk_refs_in,
                     const Spdb::aux_ref_t *aux_refs_in,
                     const void *chunk_data_in,
                     int &n_chunks_out,
                     MemBuf &refBufOut,
                     MemBuf &auxBufOut,
                     MemBuf &dataBufOut);

  // Convert the given data chunk from the SPDB database to symprod format.
  
  int convertToSymprod(const void *params,
		       const string &dir_path,
		       const int prod_id,
		       const string &prod_label,
		       const Spdb::chunk_ref_t &chunk_ref,
		       const Spdb::aux_ref_t &aux_ref,
		       const void *spdb_data,
		       const int spdb_len,
		       MemBuf &symprod_buf);

private:
  
  map< string, IconDef* > _iconDefList;
  
  Symprod::capstyle_t _convertCapstyle(int capstyle);
  Symprod::joinstyle_t _convertJoinstyle(int joinstyle);
  Symprod::linetype_t _convertLineType(int line_type);

  int _wildCard(const char *WildSpec, const char *String, bool debug);

  void _addForPeriod(Symprod &prod, const Taf::ForecastPeriod &period);

  int _findFlightCatIndex(const Taf::ForecastPeriod &period);

  void _addWindBarb(Symprod &prod,
                    const Taf::ForecastPeriod &period,
                    bool plotGust,
                    int fcatIndex);

  void _addFlightCat(Symprod &prod,
                     const Taf::ForecastPeriod &period,
                     int fcatIndex);

  void _addHiddenText(Symprod &prod,
                      const Taf::ForecastPeriod &period,
                      int fcatIndex);
  
  void _addLocationIcon(Symprod &prod);

  void _addLabels(Symprod &prod,
                  const Taf::ForecastPeriod &period,
                  int fcatIndex);
  
  void _addIcon(Symprod &prod,
                const Taf::ForecastPeriod &period);
  
  void _drawIcon(const char *iconName,
                 const char *colorToUse,
                 double centerLat,
                 double centerLon,
                 double iconScale,
                 bool allowClientScaling,
                 Symprod &prod);
  
  Symprod::vert_align_t _vertAlign(Params::vert_align_t align);

  Symprod::horiz_align_t _horizAlign(Params::horiz_align_t align);

  Symprod::font_style_t _fontStyle(Params::font_style_t style);

  static double _nearest(double target, double delta);
  
};

#endif
