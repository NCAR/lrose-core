#include "CalibDayNight.hh"
#include <toolsa/LogStream.hh>
#include <toolsa/DateTime.hh>

//------------------------------------------------------------------
static double _getTimeFraction(int h, int m, int s)
{
  return static_cast<double>(h) + static_cast<double>(m)/60.0 +
    static_cast<double>(s)/3600.0;
}

//------------------------------------------------------------------
static double _getTimeFraction(const int *hms)
{
  return _getTimeFraction(hms[0], hms[1], hms[2]);
}

CalibDayNight::CalibDayNight()
{
}

CalibDayNight::~CalibDayNight()
{
}

bool CalibDayNight::initialize(const std::string &ref_file_name_day,
			       const std::string &ref_file_name_night,
			       const int *hms_night, const int *hms_day,
			       int day_night_transition_delta_seconds)
{
  for (int i=0; i<3; ++i)
  {
    _hms_day[i] = hms_day[i];
    _hms_night[i] = hms_night[i];
  }
  _hourDay = _getTimeFraction(_hms_day);
  _hourNight = _getTimeFraction(_hms_night);

  _transition_delta_seconds = day_night_transition_delta_seconds;

  bool ret = true;
  if (!_calibDay.initialize(ref_file_name_day))
  {
    ret = false;
  }
  if (!_calibNight.initialize(ref_file_name_night))
  {
    ret = false;
  }
  return ret;
}

FieldDataPair CalibDayNight::avIqPtr(const time_t &t) const
{
  double wDay, wNight;
  _weights(t, wDay, wNight);
  return FieldDataPair(*_calibDay.avIqPtr(), wDay,
		       *_calibNight.avIqPtr(), wNight);
}

FieldWithData CalibDayNight::phaseErPtr(const time_t &t) const
{
  double wDay, wNight;
  _weights(t, wDay, wNight);
  return FieldWithData(*_calibDay.phaseErPtr(), wDay,
		       *_calibNight.phaseErPtr(), wNight);
}

void CalibDayNight::_weights(const time_t &t, double &wDay,
			     double &wNight) const
{
  DateTime dt(t);
  int h = dt.getHour();
  int m = dt.getMin();
  int s = dt.getSec();
  double tHour = _getTimeFraction(h, m, s);

  // figure out if it is day or night, and how close to the boundary
  bool day;
  double delta, delta2;

  if (_hourDay < _hourNight)
  {
    // daytime goes from _hourDay to _hourNight, night goes from _hourNight
    // to _hourDay+24
    day = (tHour >= _hourDay && tHour < _hourNight);
    if (day)
    {
      // see if day is closer to sunrise or sunset
      delta = tHour - _hourDay;
      delta2 = _hourNight - tHour;
    }
    else
    {
      if (tHour >= _hourNight)
      {
	delta = tHour - _hourNight;
	delta2 = 24.0 - tHour + _hourDay;
      }
      else
      {
	delta = _hourDay - tHour;
	delta2 = tHour + 24.0 - _hourNight;
      }
    }
  }
  else
  {
    // daytime goes from _hourDay to _hourNight+24
    // nighttime goes from _hourNight to _hourDay
    day = !(tHour >= _hourNight && tHour < _hourDay);
    if (day)
    {
      // see if day is closer to sunrise or sunset
      if (tHour >= _hourDay)
      {
	delta = tHour - _hourDay;
	delta2 = _hourNight + 24.0 - tHour;
      }
      else
      {
	// tHour < hourNight
	delta = _hourNight - tHour;
	delta2 = tHour + 24.0 - _hourDay;
      }
    }
    else
    {
      delta = tHour - _hourNight;
      delta2 = _hourDay - tHour;
    }
  }
  if (delta2 < delta)
  {
    delta = delta2;
  }
  double idelta = delta*3600.0;

  if (day)
  {
    if ((int)idelta < _transition_delta_seconds)
    {
      wDay = 0.5*idelta/(double)_transition_delta_seconds + 0.5;
      wNight = 1.0 - wDay;
    }
    else
    {
      wDay = 1.0;
      wNight = 0.0;
    }
  }
  else
  {
    if ((int)idelta < _transition_delta_seconds)
    {
      wNight = 0.5*idelta/(double)_transition_delta_seconds + 0.5;
      wDay = 1.0 - wNight;
    }
    else
    {
      wNight = 1.0;
      wDay = 0.0;
    }
  }
}  
