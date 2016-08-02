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
///////////////////////////////////////////////////////////////
// TstormMgr class
//   Reads and interprets tstorm data from spdb
//
// $Id: TstormMgr.hh,v 1.8 2016/03/03 18:06:34 dixon Exp $
//////////////////////////////////////////////////////////////
#ifndef _TSTORM_MGR_HH_
#define _TSTORM_MGR_HH_

#include <string>
#include <vector>
#include <rapformats/tstorm_spdb.h>
#include <Spdb/DsSpdb.hh>
#include <dsdata/TstormGroup.hh>
using namespace std;

class TstormMgr {

 public:

   TstormMgr();
   TstormMgr(const string& spdbUrl, const time_t margin = 0 );
   ~TstormMgr();

   void clearData();

   // Read the Tstorms from an SPDB database.  If the URL and
   // time margin are specified, the associated object members
   // are updated to these values.
   //
   // Returns the number of chunks read from the database on
   // success, -1 on error

   int readTstorms(const time_t when );
   int readTstorms(const string& spdbUrl,
		   const time_t when,
		   const time_t margin = 0 );

   // Write the Tstorms to an SPDB database.  If a URL is
   // specified, the associated object members are updated to
   // these values.
   //
   // Returns true on success, false on error

   bool writeTstorms(const string& spdb_url = "");
  
   int getNumGroups() const { return( _groups.size() ); }
   vector< TstormGroup* >& getGroups() { return( _groups ); }
   
   // Add the given group to the storm mgr.  After this method call,
   // the TstormMgr object retains control of the TstormGroup object,
   // so this object shouldn't be modified or freed by the calling method.

   void addGroup(TstormGroup *group)
   {
     _groups.push_back(group);
   }
  

 private:
  
   //
   // Spdb information
   //
   DsSpdb                 _spdbMgr;
   string                 _url;
   time_t                 _timeMargin;

   vector< TstormGroup* > _groups;
   
};

#endif


    
   
