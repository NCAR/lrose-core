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
 *   $Id: DsMultTrigElem.hh,v 1.8 2016/03/03 18:06:33 dixon Exp $
 *   $Revision: 1.8 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
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
 ************************************************************************/

#ifndef DsMultTrigElem_HH
#define DsMultTrigElem_HH

/*
 **************************** includes **********************************
 */

#include <string>
#include <vector>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
using namespace std;

typedef void (*heartbeat_func_t)(const char *label);

/*
 ************************* class definitions ****************************
 */

class DsMultTrigElem
{
  friend class DsMultipleTrigger;

public:

  //////////////////
  // Public enums //
  //////////////////

  // you can either:
  //
  //   1. hang waiting for something to trigger, (only one of these allowed).
  //      This is the data that is 'NEEDED_FIRST'
  //   2. try over and over to trigger, but don't give up ever.
  //      This is data that is 'NEEDED'.
  //   3. try over and over to trigger, finally give up.
  //      THis is dat that is 'OPTIONAL'.
  //
  typedef enum
  {
      NONE = -1,
      NEEDED_FIRST = 0,   
      NEEDED = 1,
      OPTIONAL = 2
  } e_trigger_t;
  
  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  /**********************************************************************
   * Constructors
   */

  /**********************************************************************
   * Real time constructor.
   *
   *   url = MDV or SPDB protocol url.
   *   mode = from above.
   *   seconds_to_try = # of seconds to try to trigger.
   *   max_valid_age, heartbeat_func, delay_msecs
   *       These have same meaning as for DsLdataTrigger.
   *
   * Note that delay_msecs is used only if mode = NEEDED_FIRST.
   * Note that seconds_to_try is used only if mode = OPTIONAL.
   */
  DsMultTrigElem(const string url,
		 e_trigger_t mode,
		 int seconds_to_try,
		 int max_valid_age,
		 const heartbeat_func_t heartbeat_func = NULL,
		 const int delay_msec = 5000);

  /**********************************************************************
   * Archive mode multiple triggering constructor.
   *   between the range of times specified.
   */
  DsMultTrigElem(e_trigger_t mode, const string url, 
		 time_t start_time, time_t end_time);

  /**********************************************************************
   * Copy constructor.
   */
  DsMultTrigElem(const DsMultTrigElem &d);
  

  /**********************************************************************
   * Destructor
   */
  virtual ~DsMultTrigElem();
  
  /**********************************************************************
   * Operator overloading.
   */
  void operator=(const DsMultTrigElem &d);
  bool operator==(const DsMultTrigElem &d) const;

  ////////////////////
  // Access methods //
  ////////////////////

  // trigger, return next time or -1.
  time_t trigger(void);
  string getUrl(void) const { return __url; }
  
protected:

  /**********************************************************************
   * misc. little things not public, used by DsMultipleTrigger .
   */
  bool done(void) const;
  bool endOfData(void) const
  {
    if (__realtime) return false;
    return __ttrigger.endOfData();
  }
  bool mode_equals(e_trigger_t m) const;
  bool url_equals(const string url) const;
  void clear_trigger_less_or_equal(time_t time);
  bool need_trigger(int seconds_tried) const;
  void print_trigger_status(int seconds_tried) const;
  bool is_triggered(time_t time) const;
  void set_debug(bool flag);
  
private:

  string            __url;            // the url
  string            __trigger_url;    // the url for triggering (not alwaysame)
  bool              __realtime;       // realtime flag
  e_trigger_t       __mode;           // type of triggering.
  time_t            __t0, __t1;       // range of times, archive mode
  int               __max_valid_age;  // realtime param.
  int               __seconds_to_try; // realtime param.
  heartbeat_func_t  __heartbeat_func; // realtime param.
  int               __delay_msec;     // realtime param.
  bool              __debug;          // flag for debug output.
  bool              __isLocal;        // true if url is for local machine.
  
  DsLdataTrigger    __ltrigger;  // triggering mechanism, realtime.
  DsTimeListTrigger __ttrigger;  // triggering mechanism, archive.
  DsLdataInfo       __loctrigger;// triggering mechanism, realtime local.
  
  bool   __triggered;  // true if currently triggered.
  time_t __data_time;  // time of data when currently triggered.

  // private methods.
  void _form_rt_trigger_url(const string url);
  void _copy(const DsMultTrigElem &d);
  void _check_local(void);
  void _init_triggers(void);
};

#endif /* DsMultipleTrigger_HH */


