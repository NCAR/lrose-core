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
////////////////////////////////////////////////////////////////////
// <titan/Titan2Xml.hh>
//
// Convert titan objects to XML
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2021
//
////////////////////////////////////////////////////////////////
//
// Static methods that return strings
//
////////////////////////////////////////////////////////////////

#ifndef Titan2Xml_HH
#define Titan2Xml_HH

#include <string>
#include <titan/storm.h>
#include <titan/track.h>
#include <rapformats/titan_grid.h>

using namespace std;

class Titan2Xml
{

public:
  
  // storm file header
  
  static string stormFileHeader(int level,
                                const storm_file_header_t &header);

  // storm file params

  static string stormFileParams(int level,
                                const storm_file_params_t &params);
  
  // storm file scan header
  
  static string stormScanHeader(int level,
                                const storm_file_scan_header_t &header);
  
  // storm global props
  
  static string stormGlobalProps(int level,
                                 const storm_file_params_t &params,
                                 const storm_file_global_props_t &gprops);

  // storm layer props
  // NOTE: layerNum is relative to grid minz
  
  static string stormLayerProps(int level,
                                int layerNum,
                                const titan_grid_t &grid,
                                const storm_file_layer_props_t &lprops);

  // track file header
  
  static string trackFileHeader(int level,
                                const track_file_header_t &header);

  // track file params
  
  static string trackFileParams(int level,
                                const track_file_params_t &params);
  
  // simple params
  
  static string simpleTrackParams(int level,
                                  const simple_track_params_t &params);
  
  // complex params
  
  static string complexTrackParams(int level,
                                   const complex_track_params_t &params);

  // track entry
  // if entry num is not supplied, it will not be included
  
  static string trackEntry(int level,
                           const track_file_entry_t &entry,
                           int entry_num = -1);

  // forecast props
  
  static string forecastProps(const string &tag,
                              int level,
                              const track_file_forecast_props_t &props);
  
  static string titanGrid(const string &tag,
                          int level,
                          const titan_grid_t &grid);
  
  static string trackVerify(const string &tag,
                            int level,
                            const track_file_verify_t &verify);

  // contingency data
  
  static string contingencyData(const string &tag,
                                int level,
                                const track_file_contingency_data_t &cont);
  
  static string gridType(int grid_type);

  static string forecastType(int forecast_type);

  // precip mode

  static string precipMode(int precip_mode);

  // global props hail union type
  
  static string gpropsHailUnion(int gprops_union_type);

protected:
private:

};

#endif


