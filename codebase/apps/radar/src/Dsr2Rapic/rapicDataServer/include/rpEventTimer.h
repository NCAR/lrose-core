/*

  simple event timer class
  Use start/stop calls to time something
  Keeps avg, max/min, and last times

*/

#ifndef __RPEVENTTIMER_H__
#define __RPEVENTTIMER_H__

#include "utils.h"
#include <string>

class rpEventTimer
{
 private:
  timeval _startTime;    // time start called
  timeval _endTime;    // time start called
  double  _minTime;      // min event time
  double  _maxTime;      // max event time
  double  _lastTime;     // last event time
  double  _totalTime;    // total times
  int     _events;       // number of events
  bool    _started;
  bool    _lastFailed;   // state when stopped
  std::string  _lastFailedStr;// optional fail string
  time_t  _lastSuccessTime; // time of last successful stop
  std::string  label;    // label for this timer
  std::string  startstr; // string passed with start call
 public:
  virtual void    setLabel(char *labelstr = NULL)
    {
      if (labelstr)
	label = labelstr;
      else
	label = "Anonymous";
    };
  virtual void    start(char *_str = NULL); // start timer
  virtual void    start(timeval starttimeval,
			char *_str = NULL); // start timer - pass start timeval
  virtual void    start(time_t starttime_t,
			char *_str = NULL); // start timer - pass start timeval
  virtual void    stop(bool failed = false, char *failstr = NULL);  // stop timer
  virtual void    stop(timeval stoptimeval, 
		       bool failed = false, char *failstr = NULL);  // stop timer - pass stop timev
  virtual void    stop(time_t stoptime_t,
		       bool failed = false, char *failstr = NULL);  // stop timer - pass stop timev
  virtual void    reset(); // reset min/max, total etc.
  rpEventTimer();
  virtual ~rpEventTimer() {};
  double   minTime() { return _minTime; };
  void     setMinTime(double mintime) { _minTime = mintime; };
  double   maxTime() { return _maxTime; };
  void     setMaxTime(double maxtime) { _maxTime = maxtime; };
  double   lastTime() { return _lastTime; };
  void     setLastTime(double lasttime) { addTime(lasttime); };
  void     addTime(double lasttime);
  double   avgTime() 
    { 
      if (_events) return _totalTime / _events; 
      else return 0.0;
    };
  double   totalTime() { return _totalTime; };
  void     setTotalTime(double totaltime) { _totalTime = totaltime; };
  time_t   lastStart() { return _startTime.tv_sec; };
  time_t   lastEnd() { return _endTime.tv_sec; };
  timeval  lastStartVal() { return _startTime; };
  timeval  lastEndVal() { return _endTime; };
  bool     started() { return _started; };
  long int events() { return _events; };
  void     setEvents(int events) { _events = events; };
  void     dumpStatus(FILE *statusfile, char *prefixstring = NULL);
};

#endif
