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
// InfoStore.hh
//
// InfoStore object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 1999
//
///////////////////////////////////////////////////////////////
//
// InfoStore provides access to the DataMapper store for the
// DataMapper application.
//
// Notes:
//
// 1. access to the store by clients is done through the
// DmapStore class.
//
// 2. The InfoStore and DmapStore classes coordinate access to
// the store using file locking. InfoStore write-locks the file
// for writing, and DmapStore read-locks the file for reading.
//
////////////////////////////////////////////////////////////////

#ifndef InfoStore_H
#define InfoStore_H

#include <dsserver/DmapMessage.hh>
#include <string>
#include <vector>
#include <map>
#include "Params.hh"
using namespace std;

typedef pair<string, DMAP_info_t> infoPair_t;
typedef map<string, DMAP_info_t, less<string> > infoSet_t;

class InfoStore {
  
public:

  // Constructor
  
  InfoStore(const Params &params,
	    const vector<string> &dataTypes);

  // destructor
  
  ~InfoStore();

  // data access

  const infoSet_t &infoSet() { return _infoSet; }

  // register the latest data info
  // returns 0 on success, -1 on failure

  void regLatestDataInfo(const DMAP_info_t &info);

  // register the status info
  // returns 0 on success, -1 on failure

  void regStatusInfo(const DMAP_info_t &info);

  // register the data set info
  // returns 0 on success, -1 on failure

  void regDataSetInfo(const DMAP_info_t &info);

  // register full info
  // returns 0 on success, -1 on failure

  void regFullInfo(const vector<DMAP_info_t> &infoArray);

  // delete the specified info
  // returns 0 on success, -1 on failure

  void deleteInfo(const DMAP_info_t &info);

  // Load up selected information

  void loadSelected(const char *datatype, const char *dir,
		    infoSet_t &selectedSet);

  // Save the state to  disk file

  int saveState();

  // clear out store

  void clear();

  // Purge the store of old items
  
  void purge();

  // print status of object

  void print(ostream &out);

protected:
  
private:

  const Params &_params;
  infoSet_t _infoSet;
  const vector<string> _dataTypes;

  int _readState();
  void _fillDataType(DMAP_info_t &info);
  infoSet_t::iterator _insertInfo(const DMAP_info_t &info);
  infoSet_t::iterator _findInfo(const DMAP_info_t &info);
  infoSet_t::iterator _findInfoWild(const DMAP_info_t &info);
  string _computeKey(const DMAP_info_t &info);
  void _printInfo(const DMAP_info_t &info, ostream &out);

};

#endif
