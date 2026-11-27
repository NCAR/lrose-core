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
// September 2011
//
///////////////////////////////////////////////////////////////


#ifndef _Server_HH
#define _Server_HH

#include <string>

#include <DeTect/DeTectHorizontalSpdb.hh>
#include <Spdb/DsSpdb.hh>
#include <Spdb/Symprod.hh>
#include <Spdb/DsSymprodServer.hh>
#include <toolsa/MemBuf.hh>

#include "Params.hh"
#include "TrackInfo.hh"

using namespace std;

class Server : public DsSymprodServer
{

public:
    
  // Constructor:                       
  //   o Registers with procmap
  //   o Opens socket on specified port              

  Server(const string &prog_name,
	 Params       *initialParams);

  // destructor

  virtual ~Server(){};
  
protected:

  // methods invoked from the base class for managing 
  // local override of parameter file

  int      loadLocalParams( const string &paramFile, void **params);
  void     freeLocalParams( void *params )
                          { delete (Params*)params; }

  // Convert the given data chunk from the SPDB database to symprod format.

  int convertToSymprod(const void *params,
                       const string &dir_path,
                       const int prod_id,
                       const string &prod_label,
                       const Spdb::chunk_ref_t &chunk_ref,
                       const Spdb::aux_ref_t &aux_ref,
                       const void *spdb_data,
                       const int spdb_len,
                       MemBuf &symprod_buf) { return 0; };

  int convertToSymprod(const void *params,
		       const string &dir_path,
		       const TrackInfo &track_info,
		       MemBuf &symprod_buf);

  void transformData(const void *localParams,
		     const string &dir_path,
		     const DateTime &request_time,
		     int prod_id,
		     const string &prod_label,
		     int n_chunks_in,
		     const Spdb::chunk_ref_t *chunk_refs_in,
		     const Spdb::aux_ref_t *aux_refs_in,
		     const void *chunk_data_in,
		     int &n_chunks_out,
		     MemBuf &refBufOut,
		     MemBuf &auxBufOut,
		     MemBuf &dataBufOut);
  

private:

  MemBuf _coastedIcon;
  
//  // Add an arrow to show forecast movement vector
//
//  void _addForecastVector(Params *serverParams,
//			  Symprod &prod,
//			  DeTectHorizontalSpdb &polyline);

  // Add a SYMPROD text object for the given value to the product buffer.

  template<class T> void _addText(const T value,
				  const char *value_format,
				  Symprod &prod,
				  double lat, double lon,
				  const char *color, const char *background_color,
				  const int x_offset, const int y_offset,
				  const int font_size, const char *font_name,
				  Symprod::vert_align_t vert_align,
				  Symprod::horiz_align_t horiz_align)
  {
    // Assemble the sting.
    char labelString[256];
    sprintf(labelString, value_format, value);

    // Add this text to the prod.
    prod.addText(labelString,
		 lat, lon, color, background_color,
		 x_offset, y_offset,
		 vert_align, horiz_align,
		 font_size, Symprod::TEXT_NORM, font_name);
  }

  // Add the ellipse to the Symprod product

  void _addEllipse(const Params *serverParams, 
		   Symprod &prod,
		   const TrackInfo &track_info) const;

  // Add the track to the Symprod product

  void _addTrack(const Params *serverParams, 
		 Symprod &prod,
		 const TrackInfo &track_info) const;

  // Add the vector to the Symprod product

  void _addVector(const Params *serverParams, 
		  Symprod &prod,
		  const TrackInfo &track_info) const;

  // Convert the TDRP capstyle parameter to the matching symprod value.

  static Symprod::capstyle_t _convertCapstyleParam(int capstyle);
  
  // Convert the TDRP horizontal alignment parameter to the matching
  // symprod value.

  static Symprod::horiz_align_t _convertHorizAlignParam(int horiz_align);
  
  // Convert the TDRP joinstyle parameter to the matching symprod value.

  static Symprod::joinstyle_t _convertJoinstyleParam(int joinstyle);
  
  // Convert the TDRP line type parameter to the matching symprod value.

  static Symprod::linetype_t _convertLineTypeParam(int line_type);
  
  // Convert the TDRP vertical alignment parameter to the matching
  // symprod value.

  static Symprod::vert_align_t _convertVertAlignParam(int vert_align);
  
  // Get the appropriate data from the database

  virtual int _doGet(const Params &params,
		     DsSpdb &spdb,
		     const string &dir_path,
		     int get_mode,
		     const DsSpdbMsg::info_t &info,
		     DsSpdbMsg::info_t *get_info,
		     string &errStr,
		     time_t &requestTime,
		     time_t &startTime,
		     time_t &endTime);
  
  // Handle the get request.  This method overrides the base class method.

  virtual int _handleGet(const void *localParams,
			 const DsSpdbMsg &inMsg, 
			 const string &dirPath,
			 Socket &socket);
  

};

#endif
