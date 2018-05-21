/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1997
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1997/9/26 14:18:54
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/////////////////////////////////////////////////////////////
// WinsRadar2Mdv.hh
//
// WinsRadar2Mdv object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2000
//
///////////////////////////////////////////////////////////////

#ifndef WinsRadar2Mdv_H
#define WinsRadar2Mdv_H

#include <toolsa/umisc.h>
#include <string>
#include <iostream>

#include "Args.hh"
#include "Params.hh"
#include <Mdv/DsMdvx.hh>
using namespace std;

class DsInputPath;

class WinsRadar2Mdv {
  
public:

  typedef struct {
    char volume_info[20];
    si32 lat;
    si32 lon;
    si16 ht;
    si16 year;
    si16 month;
    si16 day;
    si16 hour;
    si16 min_start;
    si16 min_end;
    char field_name[4];
    char field_units[6];
    si16 nx;
    si16 ny;
    si16 dx;
    si16 dy;
    si16 scale;
    si16 data_for_byte_0;
    si16 data_for_byte_254;
    si16 log_flag;
    char color_table_name[10];
    si32 min_lat;
    si32 min_lon;
    si32 max_lat;
    si32 max_lon;
    si32 spare[104];
  } wins_radar_header_t;

  // constructor

  WinsRadar2Mdv (int argc, char **argv);

  // destructor
  
  ~WinsRadar2Mdv();

  // run 

  int Run();

  // data members

  int OK;

protected:
  
private:

  string _progName;
  Args _args;
  Params _params;
  char *_paramsPath;
  DsInputPath *_input;

  int _processFile (const char *file_path);
  int _swapHeader(wins_radar_header_t &header);
  void _printHeader(const wins_radar_header_t &header,
		    ostream &out);
  

};

#endif
