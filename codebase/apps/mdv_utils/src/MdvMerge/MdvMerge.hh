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
// MdvMerge.hh
//
// MdvMerge object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 1998
//
///////////////////////////////////////////////////////////////

#ifndef MdvMerge_H
#define MdvMerge_H

#include <string>
#include <vector>

#include <toolsa/umisc.h>
#include <tdrp/tdrp.h>
#include <Fmq/NowcastQueue.hh> 

using namespace std;

#define VGRID_MISSING -9999.999

/////////////////////////
// Forward declarations
class Args;
class Params;
class MdvxProj;
class Trigger;
class InputFile;
class Merger;

class MdvMerge {
  
public:

  // constructor

  MdvMerge (int argc, char **argv);

  // destructor
  
  ~MdvMerge();

  // run 

  int Run();

  // data members

  bool OK;
  bool Done;

protected:
  
private:

  string _progName;
  Args *_args;
  Params *_params;
  MdvxProj *_outGrid;
  //
  // A nowcast FMQ to send trigger messages to, if desired.
  //
  NowcastQueue _nowcastQueue;

  // Methods

  void _getFieldList(const int& idx, vector<int>& field_list);
  void _getFieldList(const int& idx, vector<string>& field_list);

  void _loadGrid();

  Trigger *_createTrigger();

  Merger *_createMerger();

  int _computeScaleAndBias(const int& out_field, const int& nInput, 
			   const vector<InputFile*>& inFiles,
			   double& scale, double& bias);

};

#endif
