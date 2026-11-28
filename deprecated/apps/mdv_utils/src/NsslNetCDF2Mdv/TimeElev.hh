/**
 * @file TimeElev.hh
 * @brief  Time,Elevation pair
 * @class TimeElev
 * @brief  Time,Elevation pair
 */

#ifndef TimeElev_HH
#define TimeElev_HH

#include "NsslData.hh"
#include <toolsa/DateTime.hh>
#include <string>
#include <cstdio>
class TimeElev
{
public:

  /**
   * Constructor
   */
  inline TimeElev(const NsslData &data) : _time(data.getTime()),
					  _elev(data.getElev())
  {}
  
  /**
   * Destructor
   */
  inline virtual ~TimeElev(void) {}
  
  inline bool operator==(const TimeElev &e) const
  {
    return _time == e._time && _elev == e._elev;
  }

  inline void print(void) const
  {
    printf("%s  %.2lf\n", DateTime::strn(_time).c_str(), _elev);
  }

  inline bool operator <(const TimeElev &e) const
  {
    return (_time < e._time || (_time == e._time && _elev < e._elev));
  }

  time_t _time;
  double _elev;

protected:
private:


};


#endif
