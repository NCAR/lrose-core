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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/03 18:06:33 $
 *   $Id: DsMultipleTrigger.hh,v 1.7 2016/03/03 18:06:33 dixon Exp $
 *   $Revision: 1.7 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * DsMultipleTrigger.hh: Triggers multiple url's, real-time or archive
 *                       mode, spdb or mdv protocol.
 * RAP, NCAR, Boulder CO
 *
 * May 2002
 *
 * Dave Albo
 * 
 * Triggers a number of inputs from either spdb or mdv data files, 
 * triggering off of DsMultTrigElem objects.
 *
 * Its assumed the data flow for every url specified is somewhat
 * similar as to rate of change.
 *
 ************************************************************************/

#ifndef DsMultipleTrigger_HH
#define DsMultipleTrigger_HH

/*
 **************************** includes **********************************
 */

#include <string>
#include <vector>
#include <dsdata/DsMultTrigElem.hh>
#include <dsdata/DsTrigger.hh>

using namespace std;

/*
 ************************* class definitions ****************************
 */

class DsMultipleTrigger : public DsTrigger
{

public:

  /////////////////
  // Public type //
  /////////////////

  typedef enum
  {
    TRIGGER_ALL,
    TRIGGER_ANY_ONE
  } trigger_type_t;
  
  
  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  /**********************************************************************
   * Constructors
   */

  /**********************************************************************
   * Real time multiple triggering.
   * Input arguments have the same meaning as for DsLdataTrigger.
   */
  DsMultipleTrigger(int max_valid_age = -1,
		    const heartbeat_func_t heartbeat_func = NULL,
		    const int delay_msec = 5000);

  /**********************************************************************
   * Archive mode multiple triggering between the range of times specified.
   */
  DsMultipleTrigger(time_t start_time, time_t end_time);


  /**********************************************************************
   * Destructor
   */
  virtual ~DsMultipleTrigger();
  
  /**********************************************************************
   * Initializers
   */

  /**********************************************************************
   * Real time multiple triggering.
   * Input arguments have the same meaning as for DsLdataTrigger.
   */
  bool initRealtime(int max_valid_age = -1,
		    const heartbeat_func_t heartbeat_func = NULL,
		    const int delay_msec = 5000);

  /**********************************************************************
   * Archive mode multiple triggering between the range of times specified.
   */
  bool initArchive(time_t start_time, time_t end_time);


  /**********************************************************************
   * add() - Add a new url to trigger off of...realtime mode.
   *         url: URL to watch...can be mdv or spdb protocol.
   *
   * Note that only one url can be added with type = NEEDED_FIRST.
   * Note tha the max_seconds_try argument is used only when type = OPTIONAL.
   */
  void add(const string url,   
	   DsMultTrigElem::e_trigger_t type = DsMultTrigElem::NEEDED, 
	   int max_seconds_try = 10); 


  ////////////////////
  // Access methods //
  ////////////////////


  /**********************************************************************
   * next() - Get the next trigger and set the triggerInfo accordingly
   *
   * Returns 0 upon success, -1 upon failure.
   */

  int next();
  

  /**********************************************************************
   * endOfData() - Check whether we are at the end of the data.
   */

  bool endOfData() const;
  

  /**********************************************************************
   * reset() - Reset to start of data list
   */

  void reset();
  

  /**********************************************************************
   * setTriggerType() - Set the trigger type for this object.  The default
   *                    trigger type is TRIGGER_ALL.
   */

  void setTriggerType(const trigger_type_t trigger_type)
  {
    __trigger_type = trigger_type;
  }
  

  /**********************************************************************
   * trigger() - Do triggering on all urls.  Return time or -1.
   *             Does not return with a valid time unless all NEEDED
   *             and NEEDED_FIRST url's have triggered at the return time.
   */
  time_t trigger(TriggerInfo &trigger_info);
  
  /**********************************************************************
   * trigger() - Do triggering on any one url.  Return time or -1.
   *             Does not return with a valid time until at least one
   *             url has triggered at the return time.
   *             NEEDED_FIRST urls are forbidden with this call...a -1
   *             will be returned if any URL is NEEDED_FIRST.
   *
   *             The first url that triggers causes a return.
   *
   *             This is a realtime only triggering mechanism currently.
   *             -1 is returned when called in archive mode.
   */
  time_t trigger_any_one(TriggerInfo &trigger_info);
  

  /**********************************************************************
   * done() - Done if it is archive mode and at least one NEEDED or
   *          NEEDED_FIRST url has no more data in the time range.
   *          Returns true if this is so.
   */
  bool done(void) const;
  

  /**********************************************************************
   * is_triggered() - return true if input url has triggered at input time.
   */
  bool is_triggered(const string url, time_t time) const;
  
  /**********************************************************************
   * Toggle debugging state.  Debugging goes to stdout.
   */
  void set_debug(bool stat);
  
  /**********************************************************************
   * Change Debug output rate to input value, has effect when debug is true.
   */
  void set_debug_output_frequencey(int rate);
  
  /**********************************************************************
   * True if inputs exist internally.
   */
  bool exists(const string url, DsMultTrigElem::e_trigger_t mode) const;

  /**********************************************************************
   * is_empty
   */
  bool is_empty(void) const;

  /**********************************************************************
   * change sleep between attempts to trigger (default is one second)
   */
  void change_sleep(int seconds);
  
private:

  // fixed variables after creation
  bool __realtime;                    
  int __max_valid_age;                // realtime parameter used by all
  heartbeat_func_t __heartbeat_func;  // realtime parameter used by all
  int __delay_msec;                   // realtime parameter used by all
  time_t __t0, __t1;                  // archive parameter used by all.
  bool __triggering_checked;          // true if consistency check is done.
  bool __need_first;                  // true if there is one NEED_FIRST url
  bool __need;                        // true if there is at least one NEED
  bool __optional;                    // true if there is at least one OPTIONAL
  int __sleep_seconds;                // seconds to sleep between attempts.
  
  // state variables that change.
  trigger_type_t __trigger_type;      // type of triggering to do when using
                                      //     the DsTrigger methods
  vector<DsMultTrigElem> __elem;      // the individual triggering urls.
  time_t __last_trigger_time;         // last returned trigger time.
  bool __debug;                       // debug status.
  int __debug_output_rate;            // rate of debug output <=1=all,2=half,3=third,..
  int __debug_count;                  // current debug count.
  bool __debug_now;                   // true for debug output now.
  
  // private methods.
  bool _trigger_init(void);
  bool _check_triggering(void);
  void _clear_triggers_less_or_equal_time(time_t t0);
  time_t _trigger_when_need_first(TriggerInfo &trigger_info);
  time_t _trigger_when_no_need_first(TriggerInfo &trigger_info);
  time_t _trigger_need_first(time_t min_time, TriggerInfo &trigger_info);
  bool _trigger_needed_first(DsMultTrigElem &f, time_t min_time,
			     TriggerInfo &trigger_info);
  time_t _trigger_after_first(time_t tfirst);
  bool _trigger_after_first_one_pass(time_t *t0, int seconds);
  bool _trigger_no_first_one_pass(time_t *t, int *seconds,
				  TriggerInfo &trigger_info);
  time_t _trigger_any_one(TriggerInfo &trigger_info);
  time_t _trigger_any(TriggerInfo &trigger_info);
  bool _need_triggering(int seconds);
  const DsMultTrigElem *_matching_url(const string url) const;
  DsMultTrigElem *_needed_first_elem(void);
  void _check_debug(void);
  void _print_status(int seconds, bool force=false);
};

#endif /* DsMultipleTrigger_HH */
