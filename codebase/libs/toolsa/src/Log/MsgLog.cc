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
//
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// January 1998
//
// $Id: MsgLog.cc,v 1.30 2016/03/03 18:00:25 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <cstdarg>
#include <cstdio>
#include <cassert>
#include <toolsa/MsgLog.hh>
#include <toolsa/str.h>
using namespace std;

MsgLog::MsgLog() 
       :Log()
{
   init();
}

MsgLog::MsgLog( const string &appName,
                const char *instance /* = NULL */ )
       :Log( appName, instance )
{
   init();
}
    
MsgLog::~MsgLog()
{
   categoriesBySeverity::iterator i;

   //
   // Clear out any category lists
   //
   if ( categories ) {
      for( i=categories->begin(); i != categories->end(); i++ ) {
         delete( (*i).second );
      }
      delete( categories );
   }
}

void
MsgLog::init()
{
   suffix       = "msgLog";
   doDebug      = false;
   doInfo       = false;
   guiFcn       = NULL;
   linePos      = NULL;
   lineBreak    = NULL;
   categories   = NULL;
}

void
MsgLog::postMsg( msgId id )
{
   postMsg( getMsg( id ) );
}

void
MsgLog::postMsg( severity level, msgId id )
{
   postMsg( level, getMsg( id ) );
}

void                          
MsgLog::postMsg( severity level, int category, msgId id )
{
   postMsg( level, category, getMsg( id ) );        
}

const char*
MsgLog::getMsg( msgId id )
{
  const char *msg = NULL;

   switch( id ) {
      case BANNER:
           msg = "*********************************************************";
           break;
      case SEPARATOR:
           msg = "---------------------------------------------------------";
           break;
   }
   return( msg );
}

void MsgLog::postLine( const char* line )
{
  addLine( line );
}

void MsgLog::postMsg( const char* format, ... )
{
   va_list args;
   va_start( args, format );
   vsprintf( formattedMsg, format, args );
   va_end( args );

   doMsg( NOTE, formattedMsg );
}

void MsgLog::postMsg( severity level, const char* format, ... )
{
   va_list args;
   //
   // Filter out the unwanted messages
   //
   if ( !isEnabled( level ) ) {
      return;
   }
   va_start( args, format );
   vsprintf( formattedMsg, format, args );
   va_end( args );

   doMsg( level, formattedMsg );
}

void MsgLog::postMsg( severity level, int catId, const char* format, ... )
{
   va_list args;
   //
   // Filter out the unwanted messages
   //
   if ( !isEnabled( level, catId ) ) {
      return;
   }

   va_start( args, format );
   vsprintf( formattedMsg, format, args );
   va_end( args );

   doMsg( level, formattedMsg );
}

void MsgLog::doMsg( severity level, const char* pformattedMsg )
{
   string  finalMsg;

   //
   // Say something more, if we're in a crisis
   //
   if ( level == FATAL ) {
      finalMsg = "Fatal Error!\n"
                 "Application may exit with unpredictable results.\n";
   }
   else if ( level == INTERNAL ) {
      finalMsg = "Internal Error!\n"
                 "Please, immediately report the following message...\n";
   }
   finalMsg += pformattedMsg;

   msg2txt( level, finalMsg.c_str() );
   if ( guiFcn ) {
      (*guiFcn)( level, finalMsg.c_str() );
   }
}

void MsgLog::enableMsg( severity level, bool state )
{
   //
   // Enable/disable a severity level without respect to categories
   //
   switch( level ) {
      case DEBUG:
           doDebug = state;
           break;
      case INFO:
           doInfo = state;
           break;
      default:
           //
           // can't enable/disable anything else
           //
           break;
   }
}

bool MsgLog::enableMsg( severity level, int catId, bool state )
{
   stateByCategory *categoryList;

   pair< categoriesBySeverity::iterator, bool > listStat;
   categoryStat catStat;

   //
   // Create category map, if we don't have one
   //
   if ( !categories ) {
      categories   = new categoriesBySeverity;
      categoryList = NULL;
   }
   else {
      categoryList = getCategoriesBySeverity( level );
   }

   //
   // Create a new list of categories for this message severity, 
   // if we don't have one
   //
   if ( categoryList == NULL ) {
      categoryList = new stateByCategory;
      listStat = categories->insert( pair< const int,
                                           stateByCategory* >
                                   ( level, categoryList ) );

      //
      // Bail out if the insert failed
      //
      if ( listStat.second == false ) {
         delete categoryList;
         return false;
      }
   }
   else {
      catStat = getCategory( categoryList, catId );
   }

   //
   // Create a new category if we don't have one
   //
   if ( catStat.second == false ) {
      catStat = categoryList->insert( pair< const int, bool >( catId, state ) );
      if ( catStat.second == false ) {
         return false;
      }
   }
   else {
      //
      // We got the category, set its state
      //
      (*(catStat.first)).second = state;
   }

   return true;
}

stateByCategory *
MsgLog::getCategoriesBySeverity( severity level ) const
{
   //
   // Degenerate case
   //
   if ( categories == NULL )
      return( NULL );

   //
   // Grab the categoryList for the specified severity
   //
   categoriesBySeverity::iterator i = categories->find( level );

   if ( i != categories->end() ) {
      return( (*i).second );
   }
   else {
      return( NULL );
   }
}

categoryStat
MsgLog::getCategory( stateByCategory* categoryList, int catId ) const
{

   //
   // Look up a category in the specified list
   //
   stateByCategory::iterator i = categoryList->find( catId );

   if ( i != categoryList->end() ) {
      return categoryStat( i, true );
   }
   else {
      return categoryStat( i, false );
   }
}

bool MsgLog::isEnabled( severity level, int catId ) const
{
   stateByCategory* categoryList = getCategoriesBySeverity( level );
   if ( categoryList ) {
      categoryStat catStat;
      catStat = getCategory( categoryList, catId );
      if ( catStat.second == true )
         return (*(catStat.first)).second;
      else
         return false;
   }
   else {
      return false;
   }                                                
}

bool MsgLog::areAnyEnabled( severity level ) const
{
   stateByCategory* categoryList = getCategoriesBySeverity( level );

   if ( categoryList ) {
      stateByCategory::iterator item;

      for( item=categoryList->begin(); item != categoryList->end(); ++item ) {
         if ( (*item).second ) {
            return true;
         }
      }
   }

   return false;
}

bool MsgLog::isEnabled( severity level ) const
{
   //
   // Degenerate case 
   // Can't disable anything other than DEBUG and INFO
   // For example, FATAL, ERROR, WARNING, are always enabled
   //
   if ( level != DEBUG  &&  level != INFO ) {
      return true;
   }

   if ( categories ) {
      //
      // If any category at this severity level is enabled, 
      // then the severity level is considered enabled
      //
      return( areAnyEnabled( level ));
   }
   else {
      //
      // The simple case where no categories have been specified
      //
     if (level == DEBUG) {
       return doDebug;                   
     } else {
       return doInfo;
     }

   }
}

void MsgLog::msg2txt( severity level, const char *message )
{

   // reopen file if dayMode and day has changed

   openFile();
   _lock();

   string  prefix;

   switch( level ) {
      case DEBUG:
           prefix = "DEBUG:    ";
           break;
      case INFO:
           prefix = "INFO:     ";
           break;
      case WARN:
           prefix = "WARNING:  ";
           break;
      case ERROR:
           prefix = "ERROR:    ";
           break;
      case FATAL:
           prefix = "FATAL:    ";
           break;
      case INTERNAL:
           prefix = "INTERNAL: ";
           break;
      case NOTE:
           break;
   }

   // put the text

   char   *line;
   string  fullLine;
   while ( (line = nextLine( message )) != NULL ) {
      fullLine = prefix + line;
      addLine( fullLine );
   }

   _unlock();

}

void MsgLog::addLine( const char *line )
{
  if ( isOutputToFile() ) {
      logFile << line << endl;
  } else if ( !guiFcn ) {
    if ( saysWho.size() ) {
      cerr << saysWho << ": " << line << endl;
    }
    else {
      cerr << line << endl;
    }
  }
}

char* MsgLog::nextLine( const char *message )
{
   //
   // Break the message text, if necessary
   //
   if ( linePos == NULL ) {
      //
      // Starting on a new message
      //
      STRncopy( line, message, 1024 );
      linePos = line;
   }
   else if ( lineBreak == NULL ) {
      //
      // No more lines
      //
      lineBreak = linePos = NULL;
      return NULL;
   }
   else if ( lineBreak != NULL ) {
      //
      // Continuing from the last message
      //
      linePos = ++lineBreak;
   }

   //
   // Search for a new line \n
   //
   if( (lineBreak = strchr(linePos, '\n')) != NULL ) {
      *lineBreak = '\0';
      //
      // Check for the off chance that the message ends with a \n
      //
      if ( *(lineBreak+1) == '\0' )
         lineBreak = NULL;

      return( linePos );
   }

   //
   // Return the last line
   //
   if ( *linePos != '\0' ) {
      return( linePos );
   }

   //
   // Shouldn't get to this point
   //
   return( NULL );
}
