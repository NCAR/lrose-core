/**
 * @file FloatUserData.cc
 */
#include <rapmath/FloatUserData.hh>

bool FloatUserData::getFloat(double &v) const
{
  v = _value;
  return true;
}
