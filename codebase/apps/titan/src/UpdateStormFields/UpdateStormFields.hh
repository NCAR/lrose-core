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
// UpdateStormFields.hh
//
// UpdateStormFields object
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// January 1998
//
///////////////////////////////////////////////////////////////

#ifndef UpdateStormFields_HH
#define UpdateStormFields_HH

#include <stdio.h>

#include <toolsa/os_config.h>
#include <dsdata/DsTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Spdb/DsSpdb.hh>
#include <tdrp/tdrp.h>
#include <rapformats/bdry.h>
#include <rapformats/tstorm_spdb.h>
#include <toolsa/pmu.h>

#include "Args.hh"
#include "Params.hh"


class UpdateStormFields
{
  
public:

  // constructor

  UpdateStormFields(int argc, char **argv);

  // destructor
  
  ~UpdateStormFields();

  // initialization

  bool init();
  
  // run 

  void run();

  // data members

  int okay;
  int done;

protected:
  
  typedef struct
  {
    double speed;
    double weight;
    double u_sum_added;
    double v_sum_added;
  } std_dev_info_t;

private:

  char *_programName;
  Args *_args;
  Params *_params;
  
  DsTrigger *_dataTrigger;
  
  time_t _archiveStartTime;
  time_t _archiveEndTime;
  
  DsSpdb _stormSpdb;
  vector< Spdb::chunk_t > _stormChunks;
  
  string _fieldUrl;
  DsSpdb _fieldSpdb;
  vector< Spdb::chunk_t > _fieldChunks;

  double _radiusOfInfl;

  double _calculateStormDistance(tstorm_spdb_header_t *storm_hdr,
				 tstorm_spdb_entry_t *storm,
				 tstorm_spdb_header_t *base_hdr,
				 tstorm_spdb_entry_t *base);
  
  bool _processData(const time_t trigger_time);
  
  bool _retrieveStormData(const time_t data_time,
			  const int max_time_offset);
  bool _retrieveFieldData(const time_t data_time,
			  const int max_time_offset);
  
  void _updateStormFields(void);
  
  void _writeOutputDatabase(void);
  
};

#endif
