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
// EventList.hh - class to deal with event lists.
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// December 1998
//
/////////////////////////////////////////////////////////////

/**
 * @class EventList
 *
 * Class to handle event lists.
 *
 * @author Niles Oien
 * @version $Id: EventList.hh,v 1.9 2016/03/06 23:53:41 dixon Exp $
 * @see something
 */

#ifndef EVENTLIST_HH
#define EVENTLIST_HH

#include <string>
#include <vector>

#include <toolsa/udatetime.h> // For time structure date_time_t

using namespace std;

/**
 * @addtogroup Janitor
 */
/*@{*/

class EventList
{
  
public:

  //////////////////
  // Public types //
  //////////////////

  // public data type - used to set verbosity.
  /** enum for verbosity of debug output. */
  enum Verbosity { Silent,		/**< silent - no output.	*/
		   ReportErrors,	/**< report errors only.	*/
		   Full };		/**< full output.		*/

  // constructor

  /**
   * Default constructor.
   */
  EventList();

  // Destructor

  /**
   * Destructor.
   */
  ~EventList();


  ////////////////////
  // Public methods //
  ////////////////////

  // Read events from eventlist file.  Returns either the number of events
  // read or -1 (no file).
  /**
   * Read events from an eventlist file.
   *
   * @param[in] file_name - file to read from.
   *
   * @returns number of events read on success; -1 otherwise.
   */
  int  ReadEventList(const string file_name); 

  // Write an eventlist file. Returns 1 if OK, 0 if the file cannot be
  // created.
  /**
   * Write an eventlist file.
   *
   * @param[in] FileName - name of file to create.
   *
   * @returns 1 on success; 0 if the file cannot be created.
   */
  int  WriteEventList(char *FileName);

  // Set the level of output.
  /**
   * Accessor method to set the level of output.
   *
   * @param[in] Level - desired level of output.
   */
  void SetVerbosity(Verbosity Level);

  // See if a time is "in" an event list.
  /**
   * Method to inquire whether a time is in an event list.
   *
   * @param[in] event_time - time to check.
   *
   * @returns true if event_time is in the event list; false otherwise.
   */
  bool TimeInEventList(const date_time_t event_time);

  // See if a day is "in" an event list.
  // This is useful for file names that have
  // the date, but not the time, specified,
  // like YYYYMMDD.spdb - if the
  // eventlist covers any time in the day YYYYMMDD,
  // return true, else false.
  /**
   * Method to inquire whether a day in an event list.
   *
   * @param[in] event_time - time to check.
   *
   * @returns true if event list covers any time during the day; false otherwise.
   */
  bool DayInEventList(date_time_t event_time);

  // Sets up an eventlist without reading from a file.
  // May be useful in conjunction with WriteEventList
  // to create event files. Num is the number of
  // events, et is an array of event times, size 2*Num,
  // delineating start and stop times for each event.
  // returns 1 if OK, 0 if Num exceeds allocation.
  /**
   * Accessor method to set an event list.
   *
   * @param[in] Num - number of events.
   * @param[in] et - array of event times.
   *
   * @returns 1 on success; 0 if Num exceeds allocation.
   */
  int SetEventList(int Num, date_time_t *et);


private:

  ///////////////////
  // Private types //
  ///////////////////

  /** struct for the time limits of an event. */
  struct Event
  {
    date_time_t Start;		/**< start time of event.	*/
    date_time_t Finish;		/**< end time of event.		*/
  };

  /////////////////////
  // Private members //
  /////////////////////

  vector< Event > _eventList;	/**< the event list.		*/
  
  Verbosity  Verbose; 		/**< debugging level.		*/
};

/*@}*/

#endif







