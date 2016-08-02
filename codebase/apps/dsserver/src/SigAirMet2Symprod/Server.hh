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
#include <rapformats/SigAirMet.hh>

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

  virtual ~Server(){};
  
protected:

  // members containing rendering information

  Symprod::vert_align_t _symprodVertAlign;
  Symprod::horiz_align_t _symprodHorizAlign;
  Symprod::font_style_t _symprodFontStyle;
  
  // methods invoked from the base class for managing 
  // local override of parameter file
  
  int loadLocalParams( const string &paramFile, void **params);
  void freeLocalParams( void *params ) { delete (Params*)params; }
  
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

  int _WildCard(char *WildSpec, char *String, int debug);

  void _computeCentroid(const vector <double> &lats, 
                        const vector <double> &lons,
                        double &centroidLat,
                        double &centroidLon);

  void _conditionLongitudes(vector <double> &lons);
  
  void _drawForecasts(const vector <sigairmet_forecast_t> &forecasts,
                      const Params *serverParams,
                      const char *colorToUse,
                      const double centerLat,
                      const double centerLon,
                      const int renderIcon,
                      const char *iconName,
                      const float iconScale,
                      const int allowClientScaling,
                      Symprod &prod,
                      double &dirn,
                      bool &labelsRendered);

  void _drawOutlooks(const vector <sigairmet_forecast_t> &outlooks,
                     const Params *serverParams,
                     const char *colorToUse,
                     const double centerLat,
                     const double centerLon,
                     const int renderIcon,
                     const char *iconName,
                     const float iconScale,
                     const int allowClientScaling,
                     Symprod &prod,
                     double &dirn,
                     bool &labelsRendered);
  
  bool _drawIcon(const Params *serverParams,
                 const char *iconName,
                 const char *colorToUse,
                 const double centerLat,
                 const double centerLon,
                 const float iconScale,
                 const int renderIcon,
                 const int allowClientScaling,
                 Symprod &prod);
  
  void _computeMovementTextOffsets(const Params *serverParams,
                                   double dirn,
                                   int &offsetX,
                                   int &offsetY);

};

#endif
