/**
 * @file RaySubset.hh
 * @brief A range of gate indices along a radar ray
 * @class RaySubset
 * @brief A range of gate indices along a radar ray
 *
 * Extremely simple class
 */
#ifndef RAY_SUBSET_HH
#define RAY_SUBSET_HH

#include <euclid/Grid2d.hh>
#include <string>
#include <cstdio>

class RaySubset
{
public:

  /**
   * At the radar so only a y value. 
   *
   * @param[in] y  Azimuth index
   * @param[in] outside True if this is to be the 'outside cloud' subset
   *
   * If it is outside, x0=x1=-1   If it is in cloud, x0=x1=0
   */
  inline RaySubset(int y, bool outside) : _y(y)
  {
    if (outside)
    {
      _x0 = _x1 = -1;
    }
    else
    {
      _x0 = _x1 = 0;
    }
  }

  /**
   * Normal constructor, not at radar
   *
   * @param[in] x0  Closest gate index 
   * @param[in] x1  Farthest gate index
   * @param[in] y   Azimuth index
   */
  inline RaySubset(int x0, int x1, int y) : _x0(x0), _x1(x1), _y(y) {}

  /**
   * Destructor
   */
  inline ~RaySubset(void) {}

  /**
   * @return _x0
   */
  inline int getX0(void) const {return _x0;}

  /**
   * @return _x1
   */
  inline int getX1(void) const {return _x1;}

  /**
   * @return true if x settings indicate at the radar inside a cloud
   */
  inline bool isCloudAtRadar(void) const {return _x0 == 0 && _x1 == 0;}

  /**
   * @return true if x settings indicate at the radar outside a cloud
   */
  inline bool isOutsideAtRadar(void) const {return _x0 == -1 && _x1 == -1;}

  /**
   * @return debug description of object
   * @param[in] label  To put into description
   */
  inline std::string sprint(const std::string &label) const
  {
    char buf[1000];
    sprintf(buf, "%s:x[%d,%d],y:%d", label.c_str(), _x0, _x1, _y);
    std::string s = buf;
    return s;
  }

  /**
   * Along the ray subset write a value to the data grid
   * @param[in,out] data   Grid
   * @param[in] v  Value
   */
  void toGrid(Grid2d &data, double v) const
  {
    for (int x=_x0; x<=_x1; ++x)
    {
      if (x >= 0)
      {
	data.setValue(x, _y, v);
      }
    }
  }

protected:
private:

  int _x0;  /**< Closest gate index */
  int _x1;  /**< Farthest gate index */
  int _y;   /**< Azimuth index */
};

#endif
