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
// DataSet.hh
//
// DataSet object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2006
//
///////////////////////////////////////////////////////////////

#ifndef DataSet_H
#define DataSet_H

#include <string>
#include <map>
#include <didss/LdataInfo.hh>
#include "Params.hh"
using namespace std;

// typedefs for map of active children

typedef pair<pid_t, time_t> activePair_t;
typedef map <pid_t, time_t, less<pid_t> > activeMap_t;

////////////////////////
// This class

class DataSet {
  
public:

  // constructor

  DataSet (const Params &params, int data_set_num);
  
  // destructor
  
  ~DataSet();

  // constructor OK check

  bool isOK;

  // check for new data
  // returns true if new data found, false otherwise

  bool check();

  // kill remaining children
  
  void killRemainingChildren();

protected:
  
private:

  Params _params;
  activeMap_t _active;

  int _dataSetNum;
  string _inputDir;
  bool _callNewDataScript;
  string _newDataScriptName;
  bool _callLateDataScript;
  string _lateDataScriptName;
  int _lateDataSecs;
  vector<string> _trailingArgs;
  
  LdataInfo _ldata;
  LdataInfo _ldataPrev;

  time_t _timeLastAction;
  bool _lateDataScriptCalled;

  void _callScript(bool run_in_background,
		   const LdataInfo &ldata,
		   const char *script_to_call,
		   bool include_data_late_secs);

  void _execScriptAllArgs(const LdataInfo &ldata,
                          const char *script_to_call,
                          bool include_data_late_secs);
  
  void _execScriptSpecifiedArgs(const LdataInfo &ldata,
                                const char *script_to_call);
  
  void _addCommand(const string &command,
                   vector<string> &argVec);

  void _reapChildren();
  
  void _killAsRequired(pid_t pid, time_t terminate_time);

};

#endif

