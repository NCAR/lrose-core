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
// <titan/TitanTrackEntry.hh>
//
// Track entry object for TitanServer
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, 80305-3000, USA
//
// Jan 2001
//
////////////////////////////////////////////////////////////////////

#ifndef TitanTrackEntry_HH
#define TitanTrackEntry_HH


#include <toolsa/MemBuf.hh>
#include <titan/storm.h>
#include <titan/track.h>
#include <vector>
#include <cstdio>
#include <iostream>
using namespace std;

class TitanTrackEntry
{

  friend class TitanServer;

public:
  
  // constructor
  
  TitanTrackEntry();
  
  // destructor
  
  virtual ~TitanTrackEntry();

  // clear the object

  void clear();

  // data access

  const track_file_entry_t &entry() const { return _entry; }
  const storm_file_scan_header_t &scan() const { return _scan; }
  const storm_file_global_props_t &gprops() const { return _gprops; }
  const vector<storm_file_layer_props_t> &lprops() const { return _lprops; }
  const vector<storm_file_dbz_hist_t> &hist() const { return _hist; }
  const vector<storm_file_run_t> &runs() const { return _runs; }
  const vector<storm_file_run_t> &proj_runs() const { return _proj_runs; }

  // assemble into buffer

  void assemble(MemBuf &buf, bool clear_buffer = false) const;

  // disassemble from buffer
  // Sets len_used to the length of the buffer used while disassembling.
  // Returns 0 on success, -1 on failure.
  
  int disassemble(const void *buf, int buf_len, int &len_used);

  // Print

  void print(FILE *out,
             int entry_num,
             const storm_file_params_t &sparams,
             const track_file_params_t &tparams);

  void printXML(FILE *out,
                int entry_num,
                const storm_file_params_t &sparams,
                const track_file_params_t &tparams);
protected:

  track_file_entry_t _entry;
  storm_file_scan_header_t _scan;
  storm_file_global_props_t _gprops;
  vector<storm_file_layer_props_t> _lprops;
  vector<storm_file_dbz_hist_t> _hist;
  vector<storm_file_run_t> _runs;
  vector<storm_file_run_t> _proj_runs;

};

#endif


