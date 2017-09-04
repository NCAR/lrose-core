/**
 * @file RefractUtil.hh
 * @brief Commonly used constants 
 * @class RefractUtil
 * @brief utility methods, static
 */

#ifndef REFRACT_UTIL_HH
#define REFRACT_UTIL_HH

#include <ctime>
#include <string>

class RefractUtil
{
public:
  /**
   * @return  value*value
   * @param[in] value
   */
  static double SQR(double value);

  /**
   * @brief Set the reference N value to the value calculated from the
   *        given values.
   *
   * @param[in] pressure Pressure value in mb.
   * @param[in] temperature Temperature value in C.
   * @param[in] dewpoint_temperature Dewpoint temperature value in C.
   * @return N
   */
  static double deriveNValue(double pressure, double temperature,
			     double dewpoint_temperature);

protected:
private:
};
  
#endif
 
