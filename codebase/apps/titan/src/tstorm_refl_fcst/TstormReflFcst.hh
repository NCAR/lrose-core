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
// TstormReflFcst.h
//
// TstormReflFcst object
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1998
//
///////////////////////////////////////////////////////////////

#ifndef TstormReflFcst_H
#define TstormReflFcst_H

#include <cstdio>
#include <vector>

#include <toolsa/os_config.h>
#include <dsserver/DsLdataInfo.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxPjg.hh>
#include <Spdb/DsSpdb.hh>
#include <tdrp/tdrp.h>
#include <rapformats/tstorm_spdb.h>
#include <toolsa/ldata_info.h>
#include <toolsa/pmu.h>

#include "Args.hh"
#include "Params.hh"
using namespace std;


class TstormReflFcst
{
  
public:

  // constructor

  TstormReflFcst(int argc, char **argv);

  // destructor
  
  ~TstormReflFcst();

  // run 

  void run();

  // data members

  int okay;

protected:
  
private:

  char *_programName;
  Args *_args;
  Params *_params;
  
  time_t _archiveStartTime;
  time_t _archiveEndTime;
  
  DsSpdb                         _stormSpdb;
  DsLdataInfo                    _stormLdata;
  vector< Spdb::chunk_t >  _stormChunks;
  
  DsMdvx                         _griddedMdvx;
  MdvxPjg                        _griddedProj;
  
  unsigned char                 *_polygonGrid;
  int                            _polygonGridAlloc;
  
  time_t _getStormData(void);
  int    _getGriddedData(time_t storm_data_time);
  
  time_t _retrieveArchiveStormData(time_t start_time,
				   time_t end_time);
  
  bool _retrieveClosestGriddedData(const time_t data_time,
				   const int time_margin);
  
  int _retrieveStormData(time_t data_time);
  
  void _generateForecast(void);
  
  bool _thresholdedForecast(MdvxField &output_field,
			    const tstorm_spdb_header_t *header,
			    const tstorm_spdb_entry_t *entries,
			    const int forecast_secs);
  bool _unthresholdedForecast(MdvxField &output_field,
			      const tstorm_spdb_header_t *header,
			      const tstorm_spdb_entry_t *entries,
			      const int forecast_secs);
  
  void _setMasterHeader(DsMdvx &output_file,
			const time_t file_time);
  
  MdvxField *_createOutputField(const time_t field_time,
				const int forecast_duration);
  
};

#endif
