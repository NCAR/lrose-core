#ifndef DATA_STATUS_H
#define DATA_STATUS_H
/**
 * @file DataStatus.hh
 * @brief  Named data with a status and possibly a value
 * @class DataStatus
 * @brief  Named data with a status and possibly a value
 */

#include <string>

class DataStatus
{
public:
  inline DataStatus(void) : _status(false), _name("unknown"),
			    _value(0) {}

  /**
   * Construct so false status
   */
  inline DataStatus(const std::string &name) : _status(false), _name(name),
					       _value(0) {}
  inline ~DataStatus(void) {}
  inline std::string sprintIfBad(void) const
  {
    if (!_status)
    {
      return _name;
    }
    else
    {
      return "";
    }
  }
  
  bool _status;
  std::string _name;
  double _value;

protected:
private:

};

#endif
