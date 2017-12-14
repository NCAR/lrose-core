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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2017/12/14 15:47:01 $
//   $Id: DsMultipleTrigger.cc,v 1.18 2017/12/14 15:47:01 dixon Exp $
//   $Revision: 1.18 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
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
 *********************************************************************/

#include <ctime>
#include <dsdata/DsMultipleTrigger.hh>
#include <toolsa/DateTime.hh>

using namespace std;


/**********************************************************************
 * Real time multiple triggering.
 */
DsMultipleTrigger::DsMultipleTrigger(int max_valid_age,
				     const heartbeat_func_t heartbeat_func,
				     const int delay_msec) :
  DsTrigger(TYPE_TIME_TRIGGER),
  __realtime(true),
  __max_valid_age(max_valid_age),
  __heartbeat_func(heartbeat_func),
  __delay_msec(delay_msec),
  __t0(-1),
  __t1(-1),
  __triggering_checked(false),
  __need_first(false),
  __need(false),
  __optional(false),
  __sleep_seconds(1),
  __trigger_type(TRIGGER_ALL),
  __last_trigger_time(-1),
  __debug(false),
  __debug_output_rate(60),
  __debug_count(0),
  __debug_now(false)
{
}

/**********************************************************************
 * Archive mode multiple triggering.
 */
DsMultipleTrigger::DsMultipleTrigger(time_t start_time, time_t end_time) :
  DsTrigger(TYPE_TIME_TRIGGER),
  __realtime(false),
  __max_valid_age(0),
  __heartbeat_func(NULL),
  __delay_msec(0),
  __t0(start_time),
  __t1(end_time),
  __triggering_checked(false),
  __need_first(false),
  __need(false),
  __optional(false),
  __sleep_seconds(1),
  __trigger_type(TRIGGER_ALL),
  __last_trigger_time(-1),
  __debug(false),
  __debug_output_rate(60),
  __debug_count(0),
  __debug_now(false)
{
}

/**********************************************************************
 * Destructor
 */
DsMultipleTrigger::~DsMultipleTrigger()
{
    __elem.clear();
}

/**********************************************************************
 * Real time multiple triggering.
 */
bool DsMultipleTrigger::initRealtime(int max_valid_age,
				     const heartbeat_func_t heartbeat_func,
				     const int delay_msec)
{
    __realtime = true;
    __max_valid_age = max_valid_age;
    __heartbeat_func = heartbeat_func;
    __delay_msec = delay_msec;
    __t0 = -1;
    __t1 = -1;
    __triggering_checked = false;
    __need_first = __need = __optional = false;
    __trigger_type = TRIGGER_ALL;
    __last_trigger_time = -1;
    __debug = false;
    __debug_output_rate = 60;
    __debug_count = 0;
    __debug_now = false;
    __sleep_seconds = 1;

    return true;
}

/**********************************************************************
 * Archive mode multiple triggering.
 */
bool DsMultipleTrigger::initArchive(time_t start_time, time_t end_time)
{
    __realtime = false;
    __max_valid_age = 0;
    __heartbeat_func = NULL;
    __delay_msec = 0;
    __t0 = start_time;
    __t1 = end_time;
    __triggering_checked = false;
    __need_first = __need = __optional = false;
    __trigger_type = TRIGGER_ALL;
    __last_trigger_time = -1;
    __debug = false;
    __debug_output_rate = 60;
    __debug_count = 0;
    __debug_now = false;
    __sleep_seconds = 1;

    return true;
}

////////////////////
// Access methods //
////////////////////

/**********************************************************************
 * next() - Get the next trigger and set the triggerInfo accordingly
 *
 * Returns 0 upon success, -1 upon failure.
 */

int DsMultipleTrigger::next()
{
  static const string method_name = "DsMultipleTrigger::next()";
  
  // Clear out the old stuff

  _clearErrStr();
  _triggerInfo.clear();
  
  // Check for end of trigger data

  if (endOfData())
    return -1;
  
  // Get the next trigger time

  time_t trigger_time = 0;
  TriggerInfo trigger_info;
  
  switch (__trigger_type)
  {
  case TRIGGER_ALL :
    trigger_time = trigger(trigger_info);
    break;
    
  case TRIGGER_ANY_ONE :
    trigger_time = trigger_any_one(trigger_info);
    break;
  }

  // Check for a triggering error

  if (trigger_time == -1)
  {
    _errStr = "ERROR - " + method_name + "\n";
    _errStr += "Error getting next trigger time\n";
    
    return -1;
  }
  
  // Set the trigger info

  _triggerInfo = trigger_info;
  
  return 0;
}


/**********************************************************************
 * endOfData() - Check whether we are at the end of the data.
 */

bool DsMultipleTrigger::endOfData() const
{
  // First, check to see if we are "done"

  if (done())
    return true;
  
  // Otherwise, if we are in archive mode and none of our triggers have
  // any more data, then return true

  if (__realtime)
    return false;
  
  vector<DsMultTrigElem>::const_iterator i;

  for (i=__elem.begin(); i!=__elem.end(); ++i)
  {
    if (!i->endOfData())
      return false;
  }
  
  return true;
}


/**********************************************************************
 * reset() - Reset to start of data list
 */

void DsMultipleTrigger::reset()
{
  // I don't know how to reset this data, so I'll leave this as a
  // noop.  We need this method for the DsTrigger mode.
}


/**********************************************************************
   * add() - Add a new url to trigger off of...realtime mode.
   */
void DsMultipleTrigger::
add(const string url,
    DsMultTrigElem::e_trigger_t type /* = DsMultTrigElem::NEEDED*/,
    int max_seconds_try/* =10*/ )
{
    DsMultTrigElem *e;
  
    if (__realtime)
        e = new DsMultTrigElem(url, type, max_seconds_try,__max_valid_age,
			       __heartbeat_func, __delay_msec);
    else
        e = new DsMultTrigElem(type, url, __t0, __t1);
    e->set_debug(__debug);
    __elem.push_back(*e);
    delete e;
    __triggering_checked = false;
}
  
/**********************************************************************
 * trigger() - Do triggering on all urls.  Return time or -1.
 *             Does not return with a valid time unless all NEEDED
 *             and NEEDED_FIRST url's have triggered at the return time.
 */
time_t DsMultipleTrigger::trigger(TriggerInfo &trigger_info)
{
    time_t t0;
    if (!_trigger_init())
        return -1;
    
    if (__need_first)
        t0 = _trigger_when_need_first(trigger_info);
    else
        t0 = _trigger_when_no_need_first(trigger_info);
    __last_trigger_time = t0;
    return t0;
}

/**********************************************************************
 * trigger() - Do triggering on any one url.  Return time or -1.
 *             Does not return with a valid time until at least one
 *             url has triggered at the return time.
 *
 *             NEEDED_FIRST urls are forbidden with this call...a -1
 *             will be returned if any URL is NEEDED_FIRST.

 *             The first url that triggers causes a return.
 *
 *             This is a realtime only triggering mechanism currently.
 *             -1 is returned when called in archive mode.
 */
time_t DsMultipleTrigger::trigger_any_one(TriggerInfo &trigger_info)
{
  static const string method = "DsMultipleTrigger::trigger_any_one()";
    time_t t0;
  
    if (!__realtime)
    {
      cerr << method << " ERROR. Only works for realtime" << endl;
      return -1;
    }
    
    if (!_trigger_init())
        return -1;

    if (__need_first)
    {
      cerr << method << " ERROR..NEEDED_FIRST is forbidden" << endl;
      return -1;
    }

    // is there anything else?
    if (__need || __optional)
        t0 = _trigger_any_one(trigger_info);
    else
        t0 = -1;
    
    // t0 is it..
    __last_trigger_time = t0;
    return t0;
}

/**********************************************************************
 * done() - Done if it is archive mode and at least one NEEDED or
 *          NEEDED_FIRST url has no more data in the time range.
 */
bool DsMultipleTrigger::done(void) const
{
    vector<DsMultTrigElem>::const_iterator i;

    // must be archive mode for this to be possible..
    if (__realtime)
        return false;
    
    // if any one url is done, that's it.
    for (i=__elem.begin(); i!=__elem.end(); ++i)
    {
        if (i->done())
	    return true;
    }
    return false;
}

  
/**********************************************************************
 * is_triggered() - return true if input url has triggered at input time.
 */
bool DsMultipleTrigger::is_triggered(const string url, time_t time) const
{
    const DsMultTrigElem *i;

    if ((i = _matching_url(url)) == NULL)
        return false;
    return i->is_triggered(time);
}

/**********************************************************************
 * set_debug()
 */
void DsMultipleTrigger::set_debug(bool flag)
{
    vector<DsMultTrigElem>::iterator i;

    __debug = flag;
    // set each url's debug state also.
    for (i=__elem.begin(); i!=__elem.end(); ++i)
        i->set_debug(flag);
}

/**********************************************************************
 * set_debug_output_frequency()
 */
void DsMultipleTrigger::set_debug_output_frequencey(int rate)
{
    __debug_output_rate = rate;
}

/**********************************************************************
 * exists()
 */
bool DsMultipleTrigger::exists(const string url, 
			       DsMultTrigElem::e_trigger_t mode) const
{
    const DsMultTrigElem *i;

    if ((i = _matching_url(url)) == NULL)
        return false;
    return i->mode_equals(mode);
}

/**********************************************************************
 * is_empty
 */
bool DsMultipleTrigger::is_empty(void) const
{
    return ((int)__elem.size() == 0);
}

/**********************************************************************
 * change sleep between attempts to trigger (default is one second)
 */
void DsMultipleTrigger::change_sleep(int seconds)
{
    __sleep_seconds = seconds;
}


/////////////////////
// Private methods //
/////////////////////

/*----------------------------------------------------------------*/
bool DsMultipleTrigger::_trigger_init(void)
{
  static const string method = "DsMultipleTrigger::trigger_init()";

    // make sure everything is o.k.
    if (!__triggering_checked)
    {
	if (!_check_triggering())
	    return false;
    }

    // update debug output stuff to force output.
    __debug_count = __debug_output_rate;
    if (__debug)
      cerr << method << endl;
    
    // clear out old triggers.
    if (__last_trigger_time > 0)
        _clear_triggers_less_or_equal_time(__last_trigger_time);

    // see if all done..if so return -1.
    if (done())
    {
	if (__debug)
	  cerr << method << " NO MORE DATA TO TRIGGER OFF OF" << endl;
	return false;
    }
    return true;
}

/*----------------------------------------------------------------*/
bool DsMultipleTrigger::_check_triggering(void)
{
  static const string method = "DsMultipleTrigger::_check_triggering()";

    vector<DsMultTrigElem>::iterator i;
    int need_first, need, optional;

    __triggering_checked = true;
    need = need_first = optional = 0;
    for (i=__elem.begin(); i!=__elem.end(); ++i)
    {
        if (i->mode_equals(DsMultTrigElem::NEEDED_FIRST))
	    ++need_first;
        if (i->mode_equals(DsMultTrigElem::NEEDED))
	    ++need;
        if (i->mode_equals(DsMultTrigElem::OPTIONAL))
	    ++optional;
    }
    __need = (need > 0);
    __need_first = (need_first > 0);
    __optional = (optional > 0);
    if (need_first > 1)
    {
      cerr << method << " ERROR can have at most one NEEDED_FIRST" << endl;
      return false;
    }
    return true;
}

/*----------------------------------------------------------------*/
void DsMultipleTrigger::_clear_triggers_less_or_equal_time(time_t t0)
{
    vector<DsMultTrigElem>::iterator i;

    for (i=__elem.begin(); i!=__elem.end(); ++i)
        i->clear_trigger_less_or_equal(t0);
}

/*----------------------------------------------------------------*/
// trigger, when there is a NEEDED_FIRST url.
time_t DsMultipleTrigger::_trigger_when_need_first(TriggerInfo &trigger_info)
{
    time_t tneed, tother;
    tother = tneed = -1;
    
    // keep trying:
    while (true)
    {
        // trigger needed first at time >= tother.
        if ((tneed = _trigger_need_first(tother,
					 trigger_info)) == -1)
	    break;

	// get other stuff.
	tother = _trigger_after_first(tneed);
	if (tother <= tneed || tother == -1)
	    break;

	// the other stuff was newer than the NEEDED_FIRST
	// try again.
    }
    
    return tneed;
}

/*----------------------------------------------------------------*/
// trigger when there is not a NEEDED_FIRST url.
time_t DsMultipleTrigger::_trigger_when_no_need_first(TriggerInfo &trigger_info)
{
    time_t t0=-1;
    int seconds;
    
    // is there anything to do ?
    if (!__need && !__optional)
        return -1;

    // loop through trying to get stuff from each url.
    for (seconds=0;;seconds+=__sleep_seconds)
    {
        if (_trigger_no_first_one_pass(&t0, &seconds, trigger_info))
	{
	    _print_status(seconds, true);
	    break;
	}
	if (__heartbeat_func != NULL)
	    __heartbeat_func("trigger_when_no_first");
	_print_status(seconds);
	sleep(__sleep_seconds);

	// do we need anything?
	if (!_need_triggering(seconds))
	    break;
    }

    // done..
    return t0;
}

/*----------------------------------------------------------------*/
// trigger NEEDED FIRST url at time >= min_time.
time_t DsMultipleTrigger::_trigger_need_first(time_t min_time,
					      TriggerInfo &trigger_info)
{
    DsMultTrigElem *f;
    int seconds;
    
    // find the needed first element.
    if ((f = _needed_first_elem()) == NULL)
        return -1;
    
    // keep trying to trigger until trigger at time >= min_time:
    seconds = 0;
    while (!_trigger_needed_first(*f, min_time, trigger_info))
        _print_status(seconds++);
    _print_status(seconds, true);
    return trigger_info.getIssueTime();
}

/*----------------------------------------------------------------*/
bool DsMultipleTrigger::_trigger_needed_first(DsMultTrigElem &f,
					      time_t min_time,
					      TriggerInfo &trigger_info)
{
  static const string method = "DsMultipleTrigger::trigger_needed_first()";
    // trigger this needed first thingy, should hang on this call.
    if (__debug)
    {
      cerr << method << " url=" << f.getUrl() << endl;
      cerr << method << " Should hang want >= "
	   << DateTime::str(trigger_info.getIssueTime()) << endl;
    }
    time_t trigger_time;
    if ((trigger_time = f.trigger()) > 0)
    {
        if (__debug)
	{
	  cerr << method << " did trigger url=" << f.getUrl() << " at "
	       << DateTime::str(trigger_time) << endl;
	}
	trigger_info.setIssueTime(trigger_time);
	trigger_info.setFilePath(f.getUrl());
    }
    else
    {
      cerr << method << " ERROR. url=" << f.getUrl() << " trigger failed"
	   << endl;
      return true;
    }
    return (trigger_time >= min_time);
}

/*----------------------------------------------------------------*/
// trigger after NEEDED_FIRST has triggered at tfirst.
time_t DsMultipleTrigger::_trigger_after_first(time_t tfirst) 
{
    time_t t1;
    int seconds;

    // is there anything to do ?
    if (!__need && !__optional)
        return -1;
    
    // loop through a number of times trying to get stuff.
    for (t1=tfirst,seconds=0;;seconds+=__sleep_seconds)
    {
        if (_trigger_after_first_one_pass(&t1, seconds))
	{
	    _print_status(seconds, true);
	    break;
	}
	if (__heartbeat_func != NULL)
	    __heartbeat_func("trigger_after_first");
	_print_status(seconds);
	sleep(__sleep_seconds);
    }
    return t1;
}

/*----------------------------------------------------------------*/
// returns true to stop doing stuff.
bool DsMultipleTrigger::_trigger_after_first_one_pass(time_t *t0,
						      int seconds)
{
    time_t t;
    vector<DsMultTrigElem>::iterator i;

    for (i=__elem.begin(); i != __elem.end(); ++i)
    {
        if ((t = i->trigger()) == -1)
	    continue;
	if (t < *t0)
	    i->clear_trigger_less_or_equal(t);
	if (t > *t0)
	{
	    // clear all triggers less or equal t0
	    _clear_triggers_less_or_equal_time(*t0);
	    *t0 = t;
	    return true;
	}

	// if nothing more needed, return time.
	if (!_need_triggering(seconds))
	    return true;
    }
    return false;
}
  
/*----------------------------------------------------------------*/
bool DsMultipleTrigger::_trigger_no_first_one_pass(time_t *t0,
						   int *seconds,
						   TriggerInfo &trigger_info)
{
    time_t t;
    vector<DsMultTrigElem>::iterator i;
    
    for (i=__elem.begin(); i != __elem.end(); ++i)
    {
        // trigger if need to, return trigger time.
        if ((t = i->trigger()) == -1)
	    continue;
	if (t < *t0)
	    // too old, clear this one.
	    i->clear_trigger_less_or_equal(*t0);
	else if (t > *t0)
	{
	    // too new, clear all triggers less or equal t0
	    // and reset t0...i.e. move forward in time.
	    _clear_triggers_less_or_equal_time(*t0);
	    *t0 = t;
	    *seconds = 0;
	}

	// stop trying if nothing more needed.
	if (!_need_triggering(*seconds))
	{
	  trigger_info.setFilePath(i->getUrl());
	  trigger_info.setIssueTime(*t0);
	  return true;
	}
    }
    return false;
}

/*----------------------------------------------------------------*/
time_t DsMultipleTrigger::_trigger_any_one(TriggerInfo &trigger_info)
{
    time_t t1;
    int seconds;
    
    // loop through forever trying to get anything.
    for (seconds=0;;seconds+=__sleep_seconds)
    {
        if ((t1 = _trigger_any(trigger_info)) != -1)
	{
	    _print_status(seconds, true);
	    break;
	}
	if (!__realtime)
	    // no time loops in archive mode.
	    break;
	if (__heartbeat_func != NULL)
	    __heartbeat_func("trigger_any_one");
	_print_status(seconds);
	sleep(__sleep_seconds);
    }
    return t1;
}

/*----------------------------------------------------------------*/
time_t DsMultipleTrigger::_trigger_any(TriggerInfo &trigger_info)
{
    vector<DsMultTrigElem>::iterator i;
    time_t t;
    
    // set t1 by triggering things that need triggering:
    for (i=__elem.begin(); i != __elem.end(); ++i)
    {
        if ((t = i->trigger()) > 0)
	{
	  trigger_info.setFilePath(i->getUrl());
	  trigger_info.setIssueTime(t);
	  return t;
	}
    }
    return -1;
}

/*----------------------------------------------------------------*/
bool DsMultipleTrigger::_need_triggering(int seconds)
{
    vector<DsMultTrigElem>::iterator i;
    bool stat = false;
    
    for (i=__elem.begin(); i != __elem.end(); ++i)
    {
        if (i->need_trigger(seconds))
	    stat = true;
    }
    return stat;
}

/*----------------------------------------------------------------*/
const DsMultTrigElem *
DsMultipleTrigger::_matching_url(const string url) const
{
    vector<DsMultTrigElem>::const_iterator i;
    
    for (i=__elem.begin(); i != __elem.end(); ++i)
      if (i->url_equals(url))
  	return &(*i);
    return NULL;
}

/*----------------------------------------------------------------*/
DsMultTrigElem *
DsMultipleTrigger::_needed_first_elem(void)
{
  static const string method = "DsMultipleTrigger::_needed_first_elem()";
    vector<DsMultTrigElem>::iterator i;
    
    for (i=__elem.begin(); i!=__elem.end(); ++i)
        if (i->mode_equals(DsMultTrigElem::NEEDED_FIRST))
	    return &(*i);

    cerr << method << " ERROR no NEEDED_FIRST" << endl;
    return NULL;
}

/*----------------------------------------------------------------*/
void DsMultipleTrigger::_check_debug(void)
{
    ++__debug_count;
    if ((__debug_now = (__debug_count >= __debug_output_rate)))
        __debug_count = 0;
    __debug_now = (__debug_now && __debug);
}

/*----------------------------------------------------------------*/
void DsMultipleTrigger::_print_status(int seconds, bool force /*=false*/)
{
    vector<DsMultTrigElem>::iterator i;
    
    if (force)
        __debug_count = __debug_output_rate;
    _check_debug();
    if (!__debug_now)
        return;

    cerr << "DsMultipleTrigger::_print_status():" << endl;

    for (i=__elem.begin(); i != __elem.end(); ++i)
      i->print_trigger_status(seconds);
}

