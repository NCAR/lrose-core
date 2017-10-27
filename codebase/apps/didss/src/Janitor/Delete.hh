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
// Delete.hh: handle file deletion
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2016
//
/////////////////////////////////////////////////////////////

#ifndef DELETE_HH
#define DELETE_HH

#include <map>
#include <string>
#include <vector>
#include "Delete.hh"

using namespace std;
class Params;

// typedefs for map of active children

typedef pair<pid_t, time_t> activePair_t;
typedef map <pid_t, time_t, less<pid_t> > activeMap_t;

class Delete
{
  
public:
  
  // constructor
  
  /**
   * @param[in] progName - program name.
   * @param[in] params - parameters
   */

  Delete (const string &progName,
          const Params *params);
    
  // Destructor
    
  virtual ~Delete();

  // remove specified file
  // Returns 0 on success, -1 on failure.
  
  int removeFile(const string &filePath);

protected:
private:

  // members

  string _progName;
  const Params *_params;
  activeMap_t _active;  /**< active children from delete scripts */

  // methods

  void _callScriptOnFileDeletion(const string &filePath,
                                 time_t modTime);

  void _callScript(bool run_in_background,
                   const vector<string> &args,
                   const string &script_to_call);

  void _execScript(const vector<string> &args,
                   const string &script_to_call);
    
  void _reapChildren();
  
  void _killAsRequired(pid_t pid, time_t terminate_time);

  void _killRemainingChildren();

};

#endif



