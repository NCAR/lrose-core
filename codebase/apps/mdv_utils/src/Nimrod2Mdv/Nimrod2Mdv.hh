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
// Nimrod2Mdv.hh
//
// Nimrod2Mdv object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2011
//
///////////////////////////////////////////////////////////////
//
// Nimrod2Mdv converts a UK met office NIMROD file to MDV.
//
////////////////////////////////////////////////////////////////

#ifndef Nimrod2Mdv_hh
#define Nimrod2Mdv_hh

#include <string>
#include <vector>
#include <didss/DsInputPath.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxRemapLut.hh>
#include "Args.hh"
#include "Params.hh"
using namespace std;

////////////////////////
// This class

class Nimrod2Mdv {
  
public:

  typedef struct {

    si16 vt_year; /* valid time */
    si16 vt_month;
    si16 vt_day;
    si16 vt_hour;
    si16 vt_min;
    si16 vt_sec;
    si16 dt_year; /*  data time */
    si16 dt_month;
    si16 dt_day;
    si16 dt_hour;
    si16 dt_min;
    si16 data_type; /* 0:float 1:integer 2:byte */
    si16 byte_width;
    si16 experiment_number; // 14

    si16 grid_type; /* 0:NG, 1:lat/lon 2:space 3:polar_stereo 4:xy 5:other */
    si16 nrows;
    si16 ncols;
    si16 release_num;
    si16 field_code;
    si16 vert_type; /* 0:agl 1:msl 2:pres 3:sigma 4:eta 5:radar_beam_num
                     * 6:temp 7:theta 8:thetae 9:wetbulbtemp
                     * 10:vort 11:cloudboundary */
    si16 vert_ref_level; // 21

    si16 n_elements_from_60;
    si16 n_elements_from_109;
    si16 origin_loc; /* 0:topL 1:botL 2:topR 3:botR */
    si16 missing_int;
    si16 accum_period_min;
    si16 n_model_levels;
    si16 ellipsoid; /* 0:airy 1: modUTM32 2:GRS80 */
    si16 spare29;
    si16 spare30;
    si16 spare31;

  } nimrod_hdr_int16_start_t;
  
  typedef struct {

    fl32 vlevel;
    fl32 ref_vlevel;
    fl32 ypos_of_first_point;
    fl32 delta_y;
    fl32 xpos_of_first_point;
    fl32 delta_x;
    fl32 missing_float;
    fl32 mks_scaling_factor;
    fl32 data_offset;
    fl32 x_offset_of_model_from_grid;
    fl32 y_offset_of_model_from_grid;
    fl32 origin_lat;
    fl32 origin_lon;
    fl32 origin_easting_meters;
    fl32 origin_northing_meters;
    fl32 scale_factor_central_meridian;
    fl32 spare48;
    fl32 spare49;
    fl32 spare50;
    fl32 spare51;
    fl32 spare52;
    fl32 spare53;
    fl32 spare54;
    fl32 spare55;
    fl32 spare56;
    fl32 spare57;
    fl32 spare58;
    fl32 spare59;

    fl32 ypos_of_top_left;
    fl32 xpos_of_top_left;
    fl32 ypos_of_top_right;
    fl32 xpos_of_top_right;
    fl32 ypos_of_bot_right;
    fl32 xpos_of_bot_right;
    fl32 ypos_of_bot_left;
    fl32 xpos_of_bot_left;
    fl32 sat_calib_coeff;
    fl32 space_count;
    fl32 ducting_index;
    fl32 elevation_angle;

    fl32 spare[33];

  } nimrod_hdr_fl32_t;

  typedef struct {

    char units[8];
    char data_source[24];
    char field_title[24];

  } nimrod_hdr_char_t;

  typedef struct {

    si16 radar_num;
    si16 radar_sites;
    si16 radar_sites2;
    si16 clut_map_num;
    si16 cal_type; /* 0:uncal 1:frontal 2:showers 3:rainshadow 4:brightband */
    si16 bright_band_height;
    si16 bright_band_intensity;
    si16 bright_band_test_param_1;
    si16 bright_band_test_param_2;
    si16 infill_flag;
    si16 stop_elevation;
    si16 spare1[13];
    si16 spare2[8];
    si16 sat_indent;
    si16 meteosat_ident;
    si16 synop_availability;
    si16 spare3[16];

  } nimrod_hdr_int16_end_t;
  
  // constructor

  Nimrod2Mdv (int argc, char **argv);

  // destructor
  
  ~Nimrod2Mdv();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  static const fl32 _missingFloat;

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  DsInputPath *_input;
  MdvxRemapLut _remapLut;

  int _processFile(const char *input_path);

  void _loadFieldData(char *inBuf,
                      int nx, int ny,
                      int byteWidth,
                      int nBytes,
                      bool startsTopLeft,
                      fl32 *outBuf,
                      int missingInt,
                      fl32 missingFloat);
  
  void _initMasterHeader(DsMdvx &mdvx, time_t dataTime,
                         const string &dataSetSource,
                         int nx, int ny);

  void _initFieldHeaders(Mdvx::field_header_t &fhdr,
                         Mdvx::vlevel_header_t &vhdr,
                         nimrod_hdr_int16_start_t &hdrIntStart,
                         nimrod_hdr_fl32_t &hdrFloat,
                         nimrod_hdr_char_t &hdrChar,
                         nimrod_hdr_int16_end_t &hdrIntEnd);

  int _writeOutput(DsMdvx &mdvx);

  void _printHdrInt16Start(ostream &out,
                           nimrod_hdr_int16_start_t &hdr);
  void _printHdrFL32(ostream &out,
                     nimrod_hdr_fl32_t &hdr);
  void _printHdrChar(ostream &out,
                     nimrod_hdr_char_t &hdr);
  void _printHdrInt16End(ostream &out,
                         nimrod_hdr_int16_end_t &hdr);
  
  void _remap(DsMdvx &mdvx);
  void _autoRemapToLatLon(DsMdvx &mdvx);

};

#endif

