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
// $Id: DsUrlQueue.cc,v 1.5 2016/03/03 18:03:31 dixon Exp $
////////////////////////////////////////////////////////////////////////////////

#include <didss/DsURL.hh>
#include <didss/DsUrlQueue.hh>
using namespace std;

const int    DsUrlQueue::INITIAL_PRIORITY    = 0;
const int    DsUrlQueue::NEXT_AVAIL_PRIORITY = -2147483647;

DsUrlQueue::DsUrlQueue()
{
   top = INITIAL_PRIORITY;
}

DsUrlQueue::DsUrlQueue( const DsURL &url )
{
   //
   // Add a single url of top priority
   //
   top = INITIAL_PRIORITY;
   addURL( url, top );
}

DsUrlQueue::DsUrlQueue( const DsUrlQueue &urlQueue )
{
   copy( urlQueue );
}

DsUrlQueue::~DsUrlQueue()
{
   clear();
}

DsUrlQueue&
DsUrlQueue::operator= ( const DsURL &url )
{
   //
   // Clear out the old url's before setting the new one with top priority
   //
   clear();
   top = INITIAL_PRIORITY;
   addURL( url, top );

   return *this;
}

DsUrlQueue&
DsUrlQueue::operator= ( const DsUrlQueue &urlQueue )
{
   copy( urlQueue );
   return *this;
}

void
DsUrlQueue::copy( const DsUrlQueue &urlQueue )
{
   int    priority;
   DsURL *url;

   //
   // Clear out the old url's before setting the new ones
   //
   clear();

   //
   // Make a copy of all the url's from the source urlQueue
   //
   priority = urlQueue.topPriority();
   url = getURL( priority );
   while( url != NULL ) {
      addURL( *url, priority );
      //
      // The following weird syntax is to get around the
      // compiler warning (sometimes error) about nextURL() not
      // being a const method.  Making nextURL a const method
      // which is what should happen, results in some strange
      // stl conversion errors
      //
      url = ((DsUrlQueue*)&urlQueue)->nextURL( &priority );
   }
}

void
DsUrlQueue::clear()
{
   urlByPriority::iterator i;

   top = INITIAL_PRIORITY;
   for( i=urls.begin(); i != urls.end(); i++ ) {
      delete (*i).second;
   }
   urls.erase( urls.begin(), urls.end() );
}

void
DsUrlQueue::addURL( const DsURL &url, int priority )
{
   //
   // See if we need to determine the priority
   //
   if ( priority == NEXT_AVAIL_PRIORITY ) {
      urlByPriority::reverse_iterator i = urls.rbegin();
      if ( i != urls.rend() ) {
         //
         // Get the last used priority and increment it
         //
         priority = (*i).first + 1;
      }
      else {
         priority = INITIAL_PRIORITY;
      }
   }

   //
   // Hang onto the current priority level if it's the top
   //
   if ( priority < top ) {
      top = priority;
   }

   //
   // Make a copy of the url and hang onto it
   //
   DsURL *newUrl = new DsURL( url );
   urls[priority] = newUrl;
}

DsURL*
DsUrlQueue::getURL( int priority )
{
   urlByPriority::iterator i = urls.find( priority );

   if ( i != urls.end() ) {
      return( (*i).second );
   }
   else {
      return( NULL );
   }
}

DsURL*
DsUrlQueue::nextURL( int *priorityPtr )
{
   int currentPriority;

   //
   // Find the current priority URL
   //
   if ( priorityPtr )
      currentPriority = *priorityPtr;
   else
      currentPriority = top;

   urlByPriority::iterator i = urls.find( currentPriority );

   if ( i != urls.end() ) {
      //
      // Now get the next priority URL
      //
      i++;
      if ( i != urls.end() ) {
         if ( priorityPtr ) {
            *priorityPtr = (*i).first;
         }
         return( (*i).second );
      }
      else {
         return( NULL );
      }
   }
   else {
      //
      // If this happens, the caller has done something really wierd
      // 'cause we can't even find the current priority
      //
      return( NULL );
   }
}


