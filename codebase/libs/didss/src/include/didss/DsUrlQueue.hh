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
////////////////////////////////////////////////////////////////////////////////
// $Id: DsUrlQueue.hh,v 1.4 2016/03/03 19:20:47 dixon Exp $
////////////////////////////////////////////////////////////////////////////////

#ifndef _DSURL_QUEUE_INC_
#define _DSURL_QUEUE_INC_

#include <map>
using namespace std;

//
// Forward references
//
class DsURL;

typedef  map< int, DsURL*, less<int> > urlByPriority;


class DsUrlQueue
{
public:
   DsUrlQueue();
   DsUrlQueue( const DsURL &url );
   DsUrlQueue( const DsUrlQueue &urlQueue );

  ~DsUrlQueue();

   static const int   INITIAL_PRIORITY;
   static const int   NEXT_AVAIL_PRIORITY;

   DsUrlQueue&  operator= ( const DsURL &url );
   DsUrlQueue&  operator= ( const DsUrlQueue &urlQueue );
   void         copy( const DsUrlQueue &urlQueue );
   void         clear();
   size_t       size(){ return urls.size(); }

   void         addURL( const DsURL &url, int priority = NEXT_AVAIL_PRIORITY );

   //
   // Iterating through the prioritized URL's
   //
   DsURL*       getURL(){ return getURL( top ); }
   DsURL*       getURL( int priority );
   DsURL*       nextURL( int *currentPriority );
   int          topPriority() const { return top; }

private:

   //
   // Prioritized list of DsURL's
   //
   int            top;
   urlByPriority  urls;
};

#endif
