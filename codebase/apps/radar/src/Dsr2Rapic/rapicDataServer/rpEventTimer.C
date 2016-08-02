/*

  simple event timer class
  Use start/stop calls to time something
  Keeps avg, max/min, and last times

*/

#include "rpEventTimer.h"

rpEventTimer::rpEventTimer()
{
  reset();
}

void rpEventTimer::reset() // reset min/max, total etc.
{
  timeval_set(_startTime);
  timeval_set(_endTime);
  _minTime =           // min event time
    _maxTime =         // max event time
    _lastTime = -1;     // last event time
  _totalTime = 0;   // total times
  _events = 0;       // number of events
  _started = false;
  _lastFailed = false;
  _lastFailedStr = "Undefined";
  _lastSuccessTime = 0;
  setLabel();
}

void rpEventTimer::start(char *_str) // start timer
{
  if (_started)
    {
      fprintf(stderr, "rpEventTimer::start %s Error - already started\n",
	      label.c_str());
      if (startstr.size())
	fprintf(stderr, "  - Prev str=%s\n", startstr.c_str());
      if (_str)
	fprintf(stderr, "  - New str=%s\n", _str);
    }
  gettimeofday(&_startTime, NULL);
  _started = true;
  if (_str)
    startstr = _str;
  else
    startstr.clear();    
}

void rpEventTimer::start(timeval starttimeval, 
			 char *_str) // start timer - pass start timeval
{
  if (_started)
    {
      fprintf(stderr, "rpEventTimer::start %s Error - already started\n",
	      label.c_str());
      if (startstr.size())
	fprintf(stderr, "  - Prev str=%s\n", startstr.c_str());
      if (_str)
	fprintf(stderr, "  - New str=%s\n", _str);
    }
  _startTime = starttimeval;
  _started = true;
  if (_str)
    startstr = _str;
  else
    startstr.clear();    
}

void rpEventTimer::start(time_t starttime_t, 
			 char *_str) // start timer - pass start time_t
{
  if (_started)
    {
      fprintf(stderr, "rpEventTimer::start %s Error - already started\n",
	      label.c_str());
      if (startstr.size())
	fprintf(stderr, "  - Prev str=%s\n", startstr.c_str());
      if (_str)
	fprintf(stderr, "  - New str=%s\n", _str);
    }
  timeval_set(_startTime, starttime_t);
  _started = true;
  if (_str)
    startstr = _str;
  else
    startstr.clear();    
}

void rpEventTimer::stop(bool failed, char *failstr)  // stop timer - use real time
{
  timeval temptime;

  gettimeofday(&temptime, NULL);
  stop(temptime, failed, failstr);
}

void rpEventTimer::stop(time_t stoptime_t,
			bool failed, char *failstr)  // stop timer - pass stop time_t
{
  timeval temptime;
  timeval_set(temptime, stoptime_t);
  stop(temptime, failed, failstr);
}  

void rpEventTimer::stop(timeval stoptimeval,
			bool failed, char *failstr)  // stop timer - pass stop timev
{
  if (_started)
    {
      _endTime = stoptimeval;
      _lastTime = timeval_diff(_startTime, _endTime);
      addTime(_lastTime);
      if (!failed)
	_lastSuccessTime = _endTime.tv_sec;
      _lastFailed = failed;
      if (failstr) 
	_lastFailedStr = failstr;
    }
  else
    fprintf(stderr, "rpEventTimer::stop Error - called while not started\n");
  _started = false;
}

void rpEventTimer::addTime(double lasttime)
{
  _lastTime = lasttime;
  if (!_events || (_lastTime < _minTime))
    _minTime = _lastTime;
  if (!_events || (_lastTime > _maxTime))
    _maxTime = _lastTime;
  _totalTime += _lastTime;
  _events++;
} 

void rpEventTimer::dumpStatus(FILE *statusfile, char *prefixstring)
{
  if (!statusfile)
    return;
  char timestr[128];
  char nullprefix[] = "";
  if (!prefixstring)
    prefixstring = nullprefix;
  if (events())
    fprintf(statusfile, "%slastEvent=%s totalEvents=%d\n"
	    "%slast=%0.3f min=%0.3f max=%0.3f avg=%0.3f total=%0.3f\n",
	    prefixstring,
	    TimeString(lastStart(), timestr, true, true), int(events()),
	    prefixstring,
	    lastTime(), minTime(), maxTime(), avgTime(), totalTime());
  else
    fprintf(statusfile, "%s - rpEventTimer not used\n", prefixstring);
  if (_lastFailed)
    {
      fprintf(statusfile, "%s Last Event FAILED - %s\n",
	    prefixstring, _lastFailedStr.c_str());
      fprintf(statusfile, "%s Last Success Time - %s\n",
	    prefixstring, TimeString(_lastSuccessTime, timestr, true, true));
      
    }
    
}
