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
// $Id: MsgLog.hh,v 1.24 2016/03/03 18:00:26 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef	_MSG_LOG_INC_
#define	_MSG_LOG_INC_


#include <string>
#include <functional>
#include <utility>
#include <map>
#include <toolsa/str.h>
#include <toolsa/Log.hh>
using namespace std;

//
// Forward declarations
//
class MsgCategory;
class MsgLog;

typedef   map< int, bool, less<int> > stateByCategory;
typedef   map< int, stateByCategory*, less<int> > categoriesBySeverity;
typedef   pair< stateByCategory::iterator, bool > categoryStat;


class MsgLog : public Log {
public:
   MsgLog();
   MsgLog( const string &appName,
           const char *instance = NULL );
   virtual ~MsgLog();

   enum severity { debug,
                   info,
                   warn,
                   error,
                   fatal,
                   internal,
                   note };

   enum msgId { banner,
                separator };

   // post line without interpretation
   // line should not have line-feed or carriage-return at end
   void postLine( const char* line );

   // post messages with interpretation - messsage will end at a
  // blank line

   virtual void        postMsg( const char *format, ... );
   virtual void        postMsg( severity level, const char *format, ... );
   virtual void        postMsg( severity level, int category, const char *format, ... );

   virtual void        postMsg( msgId id );
   virtual void        postMsg( severity level, msgId id );
   virtual void        postMsg( severity level, int category, msgId id );

   char*       nextLine( const char *message );
   const char* getMsg( msgId id );

   virtual void enableMsg( severity level, bool state=true );
   virtual bool enableMsg( severity level, int category, bool state=true );
   virtual bool isEnabled( severity level ) const;
   virtual bool isEnabled( severity level, int category ) const;

   void        setGui( void (*fcn)( severity, const char *message ) )
                     { guiFcn = fcn; }

  // virtual operations for the composite - default to noop
  virtual void 		addLog( MsgLog* aLog ) {};
  virtual MsgLog* 	getLogByName( const char* name ) 
  { return ( STRequal_exact( name, myName.c_str() ) ) ? this : NULL; };
  MsgLog* 		getLogByName( const string& name ) 
  { return getLogByName( name.c_str() ); }
  const string&		getName( void ) const { return myName; }
  void 			setName( const string& name ) { myName = name; }
  void 			setName( const char* name ) { myName.assign(name); }
  
protected:

  string       myName;			// identifying string for composite
  char	       formattedMsg[4096];	// working space
  
   bool        doDebug;
   bool        doInfo;

private:

   char        line[1024];
   char*       lineBreak;
   char*       linePos;

   void        init();

   void        doMsg( severity level, const char* formattedMsg );
   void        msg2txt( severity level, const char *message );
   void        (*guiFcn)( severity level, const char *message );

   void        addLine( const char *line );
   void        addLine( string &line )
                      { addLine( line.c_str() ); }

   //
   // Category management
   //
   categoriesBySeverity  *categories;
   
   stateByCategory* getCategoriesBySeverity( severity level ) const;
   categoryStat     getCategory( stateByCategory* categoryList, 
                                 int catId ) const;
   bool             areAnyEnabled( severity level ) const;

};

#define DEBUG         MsgLog::debug
#define INFO          MsgLog::info
#define WARN          MsgLog::warn
#define WARNING       MsgLog::warn
#define ERROR         MsgLog::error
#define FATAL         MsgLog::fatal
#define INTERNAL      MsgLog::internal
#define NOTE          MsgLog::note

#define SEPARATOR     MsgLog::separator
#define BANNER        MsgLog::banner

#endif
