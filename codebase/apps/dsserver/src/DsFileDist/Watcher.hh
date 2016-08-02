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
// Watcher.hh
//
// Class for watching directories for new data files.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2010
//
/////////////////////////////////////////////////////////////

#ifndef Watcher_HH
#define Watcher_HH

#include <string>
#include <list>

#include <didss/DsInputPath.hh>
#include <dsserver/DestUrlArray.hh>

#include "Params.hh"
#include "PutArgs.hh"

using namespace std;

class Watcher {
  
public:

  // constructor
  
  Watcher (const string &prog_name,
           const Params &params,
           const string &ldata_top_dir,
           bool based_on_params,
           bool ldata_avail,
           const string &dir_path,
           list<PutArgs *> &put_list,
           const string &error_fmq_path);

  // Destructor
  
  virtual ~Watcher();

  // public data
  
  bool isOK;

  // check if directory is still active
  
  bool isStillActive();

  // find any new files, add to put list

  int findFiles();
  
private:
  
  string _progName;
  Params _params;
  string _dirPath;
  bool _basedOnParams;
  bool _ldataAvail;
  string _ldataTopDir;
  list<PutArgs *> &_putList;
  string _errorFmqPath;
  time_t _paramFileMtime;

  DsInputPath _input;

  int _getNextFile();
  bool _processThisFile();

  int _loadUrlList(DestUrlArray &destUrls);

  void _addPut(PutArgs *put_args);
  
  bool _paramsStillActive();
  bool _ldataStillActive();
  
private:

};

#endif



