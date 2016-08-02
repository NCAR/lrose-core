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
//   $Date: 2016/03/03 18:06:33 $
//   $Id: DsMultTrigElem.cc,v 1.11 2016/03/03 18:06:33 dixon Exp $
//   $Revision: 1.11 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * DsMultTrigElem.hh: One element in a DsMultipleTrigger.
 *                    Controls the triggering for one url.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2002
 *
 * Dave Albo
 * 
 * Triggers inputs from either spdb or mdv
 * triggering off of DsLdataInfo or DsTimeList triggers.
 *
 * This class is used by DsMultipleTrigger, probably not useful
 * otherwise.
 *
 *********************************************************************/

#include <dsdata/DsMultTrigElem.hh>
#include <dsserver/DsLocator.hh>
#include <toolsa/DateTime.hh>

using namespace std;

/**********************************************************************
 * Constructors
 */

/**********************************************************************
 * Real time constructor.
 */
DsMultTrigElem::
DsMultTrigElem(const string url,
	       e_trigger_t mode,
	       int seconds_to_try,
	       int max_valid_age,
	       const heartbeat_func_t heartbeat_func /* = NULL*/,
	       const int delay_msec /* = 5000*/ )
{
    __url = url;
    _form_rt_trigger_url(url);
    __realtime = true;
    __mode = mode;
    __t0 = __t1 = -1;
    __max_valid_age = max_valid_age;
    __seconds_to_try = seconds_to_try;
    __heartbeat_func = heartbeat_func;
    if (__mode == NEEDED_FIRST)
        __delay_msec = delay_msec;
    else
        __delay_msec = -1;
    __triggered = false;
    __data_time = -1;
    __debug = false;
    _check_local();
    _init_triggers();
}

/**********************************************************************
 * Archive mode constructor.
 */
DsMultTrigElem::
DsMultTrigElem(e_trigger_t mode, const string url, 
	       time_t start_time, time_t end_time)
{
    __url = url;
    __trigger_url = url;
    __realtime = false;
    __mode = mode;
    __t0 = start_time;
    __t1 = end_time;
    __max_valid_age = 0;
    __seconds_to_try = 0;
    __heartbeat_func = NULL;
    __delay_msec = 0;
    __triggered = false;
    __data_time = -1;
    __debug = false;
    _check_local();
    _init_triggers();
}

/**********************************************************************
 * Copy constructor.
 */
DsMultTrigElem::
DsMultTrigElem(const DsMultTrigElem &d)
{
    _copy(d);
}

/**********************************************************************
 * Destructor
 */
DsMultTrigElem::~DsMultTrigElem()
{
}

  
/**********************************************************************
 * Operator overloading.
 */
void DsMultTrigElem::operator=(const DsMultTrigElem &d)
{
    _copy(d);
}

/**********************************************************************
 * Operator overloading.
 */
bool DsMultTrigElem::operator==(const DsMultTrigElem &d) const
{
   return (__url == d.__url &&
	   __trigger_url == d.__trigger_url &&
	   __realtime == d.__realtime &&
	   __mode == d.__mode &&
	   __t0 == d.__t0 &&
	   __t1 == d.__t1 &&
	   __max_valid_age == d.__max_valid_age &&
	   __seconds_to_try == d.__seconds_to_try &&
	   __heartbeat_func == d.__heartbeat_func &&
	   __delay_msec == d.__delay_msec);
}

////////////////////
// Access methods //
////////////////////

/**********************************************************************
 * trigger return -1 or time.
 */
time_t DsMultTrigElem::trigger(void)
{
  static const string method = "DsMultTrigElem::trigger()";

    TriggerInfo i;
    int stat;
    
    if (__mode == NONE)
        return -1;
    
    if (__triggered)
    {
//         printf("     Already triggered %s\n", __url.c_str());
	return __data_time;
    }
    
    // 3 cases..realtime local, realtime remote, archive.
    if (__realtime && __isLocal)
    {
	  if (__mode == NEEDED_FIRST)
	  {
	      __loctrigger.readBlocking(-1, 1000, __heartbeat_func);
	      __data_time = __loctrigger.getLatestTime();
	  }
	  else
	  {
	      if (__loctrigger.read(-1) == -1)
		  __data_time = -1;
	     else
	         __data_time = __loctrigger.getLatestTime();
	  }
    }
    else if (__realtime && !__isLocal)
    {
        stat = __ltrigger.DsTrigger::next(i);
	if (stat != 0)
	{
	    if (__debug)
	      cerr << method << " Triggering error for url=" << __url << endl;
	    
	    return -1;
	}
	__data_time = i.getIssueTime();
    }
    else
    {
        stat = __ttrigger.DsTrigger::next(i);
	if (stat != 0)
	{
	    if (__debug)
	      cerr << method << " Triggering error for url=" << __url << endl;
	    
	    return -1;
	}
	__data_time = i.getIssueTime();
    }
    if (__data_time <= 0)
    {
//         if (__debug)
// 	    printf("    None yet %s\n", __url.c_str());
	return -1;
    }
    __triggered = true;

    if (__debug)
    {
      cerr << method << " Triggered time=" << DateTime::str(__data_time) 
	    << " url=" << __url << endl;
       
    }
    return __data_time;
}


/**********************************************************************
 * done() - If it is archive mode and mode= NEEDED or 
 *          NEEDED_FIRST and no more data in the time range.
 * returns true if this is so.
 */
bool DsMultTrigElem::done(void) const
{
    if (__realtime)
        return false;
    if (__mode == OPTIONAL)
        return false;
    if (__mode == NONE)
        return true;
    return __ttrigger.endOfData();
}

/**********************************************************************
 * true if input mode = internal mode.
 */
bool DsMultTrigElem::mode_equals(e_trigger_t m) const
{
    return (m == __mode);
}

/**********************************************************************
 * true if input url = internal url.
 */
bool DsMultTrigElem::url_equals(const string url) const
{
    return (url == __url);
}

/**********************************************************************
 * if triggered at this time or earlier time, clear it out.
 */
void DsMultTrigElem::clear_trigger_less_or_equal(time_t time)
{
  if (__triggered && __data_time <= time)
  {
      __triggered = false;
      __data_time = -1;
  }
}

/**********************************************************************
 * return true if need triggering now.
 */
bool DsMultTrigElem::need_trigger(int seconds_tried) const
{
    bool stat;
  
    if (__mode == NONE)
        return false;
  
    if (__triggered)
        return false;
    
    switch (__mode)
    {
    case NEEDED:
	stat = true;
	break;
    case NEEDED_FIRST:
	stat = true;
	break;
    case OPTIONAL:
    default:
        stat = (seconds_tried < __seconds_to_try);
	break;
    }
    return stat;
}

/**********************************************************************
 * print status of triggering..
 */
void DsMultTrigElem::print_trigger_status(int seconds_tried) const
{
  static const string method = "DsMultTrigElem::print_trigger_status()";
    if (!__debug)
        return;

    cerr << method << ":";

    if (__mode == NONE)
    {
      cerr << "Done...No trigger " << __url << endl;
      return;
    }
    
    if (__triggered)
    {
      cerr << "Done...Triggered " << __url << endl;
      return;
    }
    switch (__mode)
    {
    case NEEDED:
      cerr << "Need " << __url << endl;
      break;
    case NEEDED_FIRST:
      cerr << "Need First " << __url << endl;
	break;
    case OPTIONAL:
    default:
        if (seconds_tried < __seconds_to_try)
	  cerr << "Optional Need " << __url << " ("
	       << seconds_tried << " of " << __seconds_to_try <<")" << endl;
	else
	  cerr << "Optional Don't Need " << __url << " ("
	       << seconds_tried << " of " << __seconds_to_try << ")" << endl;
	break;
    }
}

/**********************************************************************
 * return true if triggered and internal time = time.
 */
bool DsMultTrigElem::is_triggered(time_t time) const
{
    return (__triggered && (time == __data_time));
}

/**********************************************************************
 * set debug state.
 */
void DsMultTrigElem::set_debug(bool flag)
{
    __debug = flag;
}

////////////////////
// Private methods //
////////////////////

/*----------------------------------------------------------------*/
// major hack follows..replace spdbp with mdvp to get triggering to work.
void DsMultTrigElem::_form_rt_trigger_url(const string url)
{

    const char *c= url.c_str();
    if (strstr(c, "spdbp") != NULL)
    {
	c += strlen("spdbp");
	__trigger_url = "mdvp";
	__trigger_url += c;
    }
    else
        __trigger_url = url;
}

/*----------------------------------------------------------------*/
void DsMultTrigElem::_copy(const DsMultTrigElem &d)
{
    __url = d.__url;
    __trigger_url = d.__trigger_url;
    __realtime = d.__realtime;
    __mode = d.__mode;
    __t0 = d.__t0;
    __t1 = d.__t1;
    __max_valid_age = d.__max_valid_age;
    __seconds_to_try = d.__seconds_to_try;
    __heartbeat_func = d.__heartbeat_func;
    __delay_msec = d.__delay_msec;
    __triggered = false;
    __data_time = -1;
    __debug = d.__debug;
    __isLocal = d.__isLocal;
    _init_triggers();
}

/*----------------------------------------------------------------*/
void DsMultTrigElem::_check_local(void)
{
  static const string method = "DsMultTrigElem::_check_local()";

    bool contactServer;
    DsURL d(__url);

    // determine if host is local
    if ( DsLocator.resolve( d, &contactServer, false ) != 0 )
    {
      cerr << method << " ERROR cannot resolve url " << __url << endl;
      __isLocal = false;
    }
    else
      __isLocal = !contactServer;
    if (__isLocal)
      cerr << method << " " << __url << " is local" << endl;
    else
      cerr << method << " " << __url << " is remote" << endl;
}

/*----------------------------------------------------------------*/
void DsMultTrigElem::_init_triggers(void)
{
    if (__mode == NONE)
        return;
    if (__realtime)
    {
        if (__isLocal)
	{
	    // local uses the actual url, not the trigger url.
	    __loctrigger.setDirFromUrl(__url);
	    __loctrigger.read(-1);
	}
	else
        {
	    __ltrigger.init(__trigger_url, __max_valid_age,
			    __heartbeat_func, __delay_msec);
	    if (__delay_msec == -1)
	        // do one trigger just in case need to clear something out.
	        __ltrigger.next();
	}
    }
    else
    {
        __ttrigger.init(__url, __t0, __t1);
	__ttrigger.reset();
    }
}
