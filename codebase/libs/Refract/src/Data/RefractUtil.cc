/**
 * @file RefractUtil.cc
 */
#include <Refract/RefractUtil.hh>
#include <cmath>

double RefractUtil::SQR(double value)
{
  return value*value;
}
  
double  RefractUtil::deriveNValue(double pressure, double temperature,
				  double dewpoint_temperature)
{
  double vapor_pres = 6.112 * exp(17.67 * dewpoint_temperature /
				  (dewpoint_temperature + 243.5));
  double nValue = 77.6 * pressure / (temperature + 273.16) + 373250 *
    vapor_pres / SQR(temperature + 273.16);
  return nValue;
}
  
