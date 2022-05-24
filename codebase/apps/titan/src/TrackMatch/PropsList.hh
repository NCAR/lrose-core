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
// PropsList.hh
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 1998
//
//////////////////////////////////////////////////////////

#ifndef PropsList_h
#define PropsList_h

#include <toolsa/umisc.h>
#include "Params.hh"
using namespace std;

typedef struct {

  time_t time;
  int year, month, day, hour, min, sec;

  int n_simple_tracks;
  int complex_track_num;
  int simple_track_num;

  double range;
  double xx, yy;
  double area, volume, mass, precip_flux;
  double da_dt, dv_dt, dm_dt, dp_dt;

  double delta_prop;

} initial_props_t;

class PropsList {
  
public:

  // constructor
  
  PropsList(const char *prog_name,
            const Params &params);
  
  // Destructor
  
  virtual ~PropsList();

  // scan an entry

  int scan(char *line, initial_props_t *props);

  // add entry

  int update(char *line, initial_props_t *case_props);

  // print

  void print(FILE *out);
  void print(FILE *out, initial_props_t *props);

protected:
  
private:

  char *_progName;
  int _debug;
  const Params &_params;
  int _nList;
  initial_props_t *_list;

  // Sort

  void _sort();

};

#endif
