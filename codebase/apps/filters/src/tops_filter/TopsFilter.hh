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
// TopsFilter.h
//
// TopsFilter object
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 1998
//
///////////////////////////////////////////////////////////////

#ifndef TopsFilter_HH
#define TopsFilter_HH

#include <iostream>

#include <toolsa/os_config.h>
#include <euclid/CircularTemplate.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/DsMdvxInput.hh>
#include <Mdv/MdvxField.hh>
#include <tdrp/tdrp.h>
#include <toolsa/pmu.h>

#include "Args.hh"
#include "Params.hh"


class TopsFilter
{
  
public:

  // destructor
  
  ~TopsFilter();

  // Get TopsFilter singleton instance

  static TopsFilter *Inst(int argc, char **argv);
  static TopsFilter *Inst();
  
  // run 

  void run();

  // data members

  bool okay;

protected:
  
private:

  // constructor -- private because this is a singleton object

  TopsFilter(int argc, char **argv);

  // Singleton instance pointer

  static TopsFilter *_instance;
  
  // Program parameters.

  char *_programName;
  Args *_args;
  Params *_params;
  
  time_t _archiveStartTime;
  time_t _archiveEndTime;
  
  DsMdvxInput _radarRetriever;
  
  DsMdvx _radarFile;
  DsMdvx _topsFile;

  MdvxField *_outputField;
  
  // Template for calculating the tops value for a grid point

  CircularTemplate *_template;
  
  time_t _getRadarData(void);
  bool   _getTopsData(time_t radar_data_time);
  
  double _getTopsValue(int x, int y);
  static int _qsortValueArray(const void *value1, const void *value2);
  
  bool _filterTops(void);
  
  void _writeOutputFile(void);
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("TopsFilter");
  }
  
};

#endif
