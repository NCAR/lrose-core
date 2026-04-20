#include <cmath>
#include <cstdio>
#include <ostream>
#include <euclid/EuclidAngle.hh>

//=======================================================================
// Constants
//=======================================================================

namespace euclid {
  
  const double EuclidAngle::PI = 3.14159265358979323846;
  const double EuclidAngle::TWO_PI = 2.0 * EuclidAngle::PI;
  const double EuclidAngle::DEG_TO_RAD = EuclidAngle::PI / 180.0;
  const double EuclidAngle::RAD_TO_DEG = 180.0 / EuclidAngle::PI;

  //=======================================================================
  // Construction
  //=======================================================================

  EuclidAngle::EuclidAngle()
          : _radians(0.0)
  {
  }

  EuclidAngle::~EuclidAngle()
  {
  }

  EuclidAngle::EuclidAngle(const EuclidAngle &rhs)
          : _radians(rhs._radians)
  {
  }

  EuclidAngle &EuclidAngle::operator=(const EuclidAngle &rhs)
  {
    if (&rhs != this) {
      _radians = rhs._radians;
    }
    return *this;
  }

  EuclidAngle::EuclidAngle(double radians)
          : _radians(radians)
  {
  }

  //=======================================================================
  // Named constructors
  //=======================================================================

  EuclidAngle EuclidAngle::fromRadians(double radians)
  {
    return EuclidAngle(radians);
  }

  EuclidAngle EuclidAngle::fromDegrees(double degrees)
  {
    return EuclidAngle(degrees * DEG_TO_RAD);
  }

  //=======================================================================
  // Accessors
  //=======================================================================

  double EuclidAngle::radians() const
  {
    return _radians;
  }

  double EuclidAngle::degrees() const
  {
    return _radians * RAD_TO_DEG;
  }

  bool EuclidAngle::isFinite() const
  {
    return std::isfinite(_radians);
  }

  //=======================================================================
  // Utilities
  //=======================================================================

  EuclidAngle EuclidAngle::abs() const
  {
    return EuclidAngle::fromRadians(std::fabs(_radians));
  }

  EuclidAngle EuclidAngle::normalize0To2Pi() const
  {
    double val = std::fmod(_radians, TWO_PI);
    if (val < 0.0) {
      val += TWO_PI;
    }
    return EuclidAngle::fromRadians(val);
  }

  EuclidAngle EuclidAngle::normalizeMinusPiToPi() const
  {
    double val = std::fmod(_radians + PI, TWO_PI);
    if (val < 0.0) {
      val += TWO_PI;
    }
    return EuclidAngle::fromRadians(val - PI);
  }

  std::string EuclidAngle::toStringDegrees(const std::string &format) const
  {
    char text[128];
    std::snprintf(text, sizeof(text), (format + " deg").c_str(), degrees());
    return std::string(text);
  }

  //=======================================================================
  // Operators
  //=======================================================================

  EuclidAngle &EuclidAngle::operator+=(const EuclidAngle &rhs)
  {
    _radians += rhs._radians;
    return *this;
  }

  EuclidAngle &EuclidAngle::operator-=(const EuclidAngle &rhs)
  {
    _radians -= rhs._radians;
    return *this;
  }

  EuclidAngle &EuclidAngle::operator*=(double scalar)
  {
    _radians *= scalar;
    return *this;
  }

  EuclidAngle &EuclidAngle::operator/=(double scalar)
  {
    _radians /= scalar;
    return *this;
  }

  bool EuclidAngle::operator==(const EuclidAngle &rhs) const
  {
    return _radians == rhs._radians;
  }

  bool EuclidAngle::operator!=(const EuclidAngle &rhs) const
  {
    return _radians != rhs._radians;
  }

  bool EuclidAngle::operator<(const EuclidAngle &rhs) const
  {
    return _radians < rhs._radians;
  }

  bool EuclidAngle::operator<=(const EuclidAngle &rhs) const
  {
    return _radians <= rhs._radians;
  }

  bool EuclidAngle::operator>(const EuclidAngle &rhs) const
  {
    return _radians > rhs._radians;
  }

  bool EuclidAngle::operator>=(const EuclidAngle &rhs) const
  {
    return _radians >= rhs._radians;
  }

  //=======================================================================
  // Free operators
  //=======================================================================

  EuclidAngle operator-(const EuclidAngle &angle)
  {
    return EuclidAngle::fromRadians(-angle.radians());
  }

  EuclidAngle operator+(const EuclidAngle &lhs, const EuclidAngle &rhs)
  {
    return EuclidAngle::fromRadians(lhs.radians() + rhs.radians());
  }

  EuclidAngle operator-(const EuclidAngle &lhs, const EuclidAngle &rhs)
  {
    return EuclidAngle::fromRadians(lhs.radians() - rhs.radians());
  }

  EuclidAngle operator*(const EuclidAngle &angle, double scalar)
  {
    return EuclidAngle::fromRadians(angle.radians() * scalar);
  }

  EuclidAngle operator*(double scalar, const EuclidAngle &angle)
  {
    return EuclidAngle::fromRadians(scalar * angle.radians());
  }

  EuclidAngle operator/(const EuclidAngle &angle, double scalar)
  {
    return EuclidAngle::fromRadians(angle.radians() / scalar);
  }

  double operator/(const EuclidAngle &lhs, const EuclidAngle &rhs)
  {
    return lhs.radians() / rhs.radians();
  }

  //=======================================================================
  // Trig helpers
  //=======================================================================

  double sin(const EuclidAngle &angle)
  {
    return std::sin(angle.radians());
  }

  double cos(const EuclidAngle &angle)
  {
    return std::cos(angle.radians());
  }

  double tan(const EuclidAngle &angle)
  {
    return std::tan(angle.radians());
  }

  EuclidAngle arcSin(double x)
  {
    return EuclidAngle::fromRadians(std::asin(x));
  }

  EuclidAngle arcCos(double x)
  {
    return EuclidAngle::fromRadians(std::acos(x));
  }

  EuclidAngle arcTan(double x)
  {
    return EuclidAngle::fromRadians(std::atan(x));
  }

  EuclidAngle arcTan2(double y, double x)
  {
    return EuclidAngle::fromRadians(std::atan2(y, x));
  }

  //=======================================================================
  // Stream output
  //=======================================================================

  std::ostream &operator<<(std::ostream &out, const EuclidAngle &angle)
  {
    out << angle.degrees();
    return out;
  }

} // namespace euclid
