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
// $Id: Benchmark.cc,v 1.10 2016/03/03 18:00:25 dixon Exp $ 
//
////////////////////////////////////////////////////////////////////////////////

#include <toolsa/Benchmark.hh>
using namespace std;

Benchmark::Benchmark()
          :Log()
{
   init();
}

Benchmark::Benchmark( const string &appName,
                      const char *instance /* = NULL */ )
          :Log( appName, instance )
{
   init();
}

Benchmark::~Benchmark()
{
   delete startTime;
}

void
Benchmark::init()
{
   suffix = "benchmark";

   //
   // Allocate the tms structure
   // This is done here instead of declaring it in the header file
   // 'cause the <sys/times.h> file has a name conflict (times)
   // with the C++ STL <function.h> file
   //
   startTime = new (struct tms);

   //
   // Get number of clock ticks per second
   //
   clktck  = sysconf(_SC_CLK_TCK);
   marking = false;
}

void
Benchmark::benchmark( char *description )
{
   //
   // Degenerate case
   //
   if ( !isOutputToFile() )
      return;

   if ( description == NULL )
      stopMarking();
   else
      startMarking( description );
}

void
Benchmark::startMarking( char *description ) 
{
   //
   // First, see if this is an implicit stop
   //
   if ( marking ) {
      stopMarking();
   }

   //
   // Start an interval
   //
   marking = true;
   if ( description )
      logFile << description << endl;
   else
      logFile << "NEW BENCHMARK INTERVAL" << endl;
   start = times( startTime );
}

void
Benchmark::stopMarking()
{
   struct tms endTime;
   clock_t    end;

   //
   // Degenerate case
   //
   if ( !marking )
      return;

   //
   // Stop an interval and print the results
   //
   marking = false;
   end = times( &endTime );
   logFile << "  wall: "
           << (end - start)/(double)clktck << endl;
   logFile << "  user: "
           << (endTime.tms_utime - startTime->tms_utime)/
              (double)clktck
           << endl;
   logFile << "  syst: "
           << (endTime.tms_stime - startTime->tms_stime)/
              (double)clktck
           << endl;
   logFile << "  chdu: "
           << (endTime.tms_cutime - startTime->tms_cutime)/
              (double)clktck
           << endl;
   logFile << "  chds: "
           << (endTime.tms_cstime - startTime->tms_cstime)/
              (double)clktck
           << endl;
}
