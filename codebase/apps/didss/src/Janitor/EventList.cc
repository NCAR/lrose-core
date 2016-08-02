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
// EventList.cc - Implementation of class to deal with event lists.
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// December 1998
//
/////////////////////////////////////////////////////////////

/**
 * @file EventList.cc
 *
 * Class to handle event lists.
 *
 * @author Niles Oien
 * @see something
 */

#include <iostream>
#include <cstdio>
#include <cassert>
#include <toolsa/udatetime.h> // date_time_t struct
#include <toolsa/umisc.h> // Forward declaration of uconvert_to_utime
#include <time.h> // Need a dummy of type time_t.

#include "EventList.hh"

using namespace std;


////////////// Constructor. ////////////

EventList::EventList() :
  Verbose(Silent)
{
  // Do nothing
}

//
//////////////// Destructor. //////////
//

EventList::~EventList()
{
  // Do nothing
}


/////////// Set the verbosity. ////////////////

void EventList::SetVerbosity(Verbosity Level)
{
  Verbose = Level;
  if (Verbose == Full)
    cerr << "Full eventlist debugging turned on." << endl;;
}

///////////////////////////////////////////////////////
// Read an event list file.
//
// Returns either the number of events read (0..N) or
//   -1 : No file found
//

int EventList::ReadEventList(const string file_name )
{

  const int MaxLineLen = 512;    // Buffer to read eventlist line by line.
  const unsigned MinEventLength = 35; // Minimum characters in valid line with event.
  // If there are fewer characters than this,
  // the line is treated as a comment.

  FILE *elp;
  char line[MaxLineLen+1];
  int j;

  if (Verbose==Full)
    cerr << "Looking for event list file " << file_name << endl;

  elp = fopen(file_name.c_str(), "ra");
  if (elp == NULL)
  {
    if (Verbose == Full) 
      cerr << file_name << " : event list file not found." << endl;
    return -1;
  }

  while (fgets(line,MaxLineLen,elp) != NULL)
  {
    if (Verbose == Full)
      cerr << "Eventlist read :" << line << endl;

    /* Is it a comment or a line too short to hold an event? */ 
    if ((line[0]=='#') || (strlen(line) < MinEventLength))
      continue;

    /* Try to read the line - 3 possible formats. */

    Event event;
    
    /* Try for the slash and colon format first. */

    if ((j = sscanf(line,
		    " start %d/%d/%d %d:%d:%d end %d/%d/%d %d:%d:%d",
		    &event.Start.year, &event.Start.month, &event.Start.day,
		    &event.Start.hour, &event.Start.min, &event.Start.sec,
		    &event.Finish.year, &event.Finish.month, &event.Finish.day,
		    &event.Finish.hour, &event.Finish.min, &event.Finish.sec))
	== 12)
    {
      if (Verbose == Full) 
	cerr << "Accepted the line in slash colon format." << endl;
    }
    /* Try the other formats. One read statement works if there are
       white spaces in there or not. */
    else if ((j = sscanf(line,
			 " start %4d%2d%2d%2d%2d%2d end %4d%2d%2d%2d%2d%2d",
			 &event.Start.year, &event.Start.month, &event.Start.day,
			 &event.Start.hour, &event.Start.min, &event.Start.sec,
			 &event.Finish.year, &event.Finish.month, &event.Finish.day,
			 &event.Finish.hour, &event.Finish.min, &event.Finish.sec))
	     == 12)
    {
      if (Verbose == Full)
	cerr << "Accepted the line." << endl; 
    }
    else
    {
      if (Verbose >= ReportErrors)
	cerr << "ERROR : Error processing line: " << line << endl;
      
      continue;
    }
    
    
    // Fill in the UNIX tiem field in the date_time_t structure

    uconvert_to_utime(&event.Start);
    uconvert_to_utime(&event.Finish);
    
    if (event.Start.unix_time > event.Finish.unix_time)
    {
      if (Verbose != Silent)
      {
	cerr << "WARNING : ReadEventList " << file_name <<
	  " : Start time succeeds end time." << endl;
	cerr << "In line " << line << endl;
      }
    }

    // Add the event to the list

    _eventList.push_back(event);
  }


  fclose(elp);

  return _eventList.size();

}



///////////////////////////////////////////////////////
// Write an event list file.
// returns 1 if OK, 0 if can't open file.
//

int EventList::WriteEventList(char *FileName)
{
  FILE *elp;

  elp = fopen(FileName, "wa");
  if (elp == NULL)
  {
    if (Verbose != Silent)
      cerr << FileName << " : event list file cannot be created." << endl;
    return 0;
  }

  fprintf(elp,"#\n# Event list with %d entries.\n#\n", (int) _eventList.size());

  vector< Event >::iterator event;
  
  for (event = _eventList.begin(); event != _eventList.end(); ++event)
  {
    fprintf(elp," start %04d/%02d/%02d %02d:%02d:%02d\t",
	    event->Start.year, event->Start.month, event->Start.day,
	    event->Start.hour, event->Start.min, event->Start.sec);

    fprintf(elp,"end %04d/%02d/%02d %02d:%02d:%02d\n",
	    event->Finish.year, event->Finish.month, event->Finish.day,
	    event->Finish.hour, event->Finish.min, event->Finish.sec);

  }


  fclose(elp);

  if (Verbose == Full)
    cerr << "Eventlist file " << FileName << " written with " <<
      _eventList.size() << " events." << endl;

  return 1;
}





///////////////////////////////////////////////////////
// See if a time is on the event list.
// Returns 1 if it is, else 0.
//

bool EventList::TimeInEventList(const date_time_t event_time)
{
  // Make sure the unix time field of the event time structure
  // is filled in.

  date_time_t full_event_time = event_time;
  uconvert_to_utime(&full_event_time);

  // Then use it to check times.

  vector< Event >::iterator event;
  
  for (event = _eventList.begin(); event != _eventList.end(); ++event)
  {
    if ((event->Start.unix_time <= event_time.unix_time) && 
	(event->Finish.unix_time >= event_time.unix_time))
      return true;
  }

  return false;
}

///////////////////////////////////////////////////////


bool EventList::DayInEventList(const date_time_t event_time)

{
  vector< Event >::iterator event;
  
  for (event = _eventList.begin(); event != _eventList.end(); ++event)
  {
    //    if ((E[i].Start.unix_time<=event_time.unix_time) && 
    //	(E[i].Finish.unix_time>=event_time.unix_time)) return 1;

    date_time_t day_begin = event->Start;

    day_begin.hour = 0;
    day_begin.min = 0;
    day_begin.sec = 1;

    uconvert_to_utime(&day_begin);

    do {
      if ((day_begin.day == event_time.day) &&
	  (day_begin.month == event_time.month) &&
	  (day_begin.year == event_time.year))
	return 1;

      day_begin.unix_time += SECS_IN_DAY;
      uconvert_from_utime(&day_begin);

    } while (day_begin.unix_time <= event->Finish.unix_time);

  }

  return 0;

}


///////////////////////////////////////////////////////
// Set eventlist up in memory.

int EventList::SetEventList(int Num, date_time_t *et)
{
  for (int i = 0; i < Num; ++i)
  {
    Event event;
    
    event.Start = et[2*i];
    event.Finish = et[2*i+1];

    // Make sure the unix_time field is filled out.

    uconvert_to_utime(&event.Start);
    uconvert_to_utime(&event.Finish);

    if (event.Start.unix_time > event.Finish.unix_time)
    {
      if (Verbose != Silent)
	cerr <<
	  "WARNING : SetEventList : A start time succeeds an end time." <<
	  endl;
    }

    // Add the event to the list

    _eventList.push_back(event);
  }

  return 1;
}
