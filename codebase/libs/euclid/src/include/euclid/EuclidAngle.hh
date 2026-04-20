#ifndef EUCLID_ANGLE_HH
#define EUCLID_ANGLE_HH

#include <iosfwd>
#include <string>

/**
 * @class EuclidAngle
 *
 * @brief Simple planar (Euclidean) angle class.
 *
 * Stores angle internally in radians using double precision.
 *
 * Designed for clarity and robustness in 2D geometry:
 *  - Explicit construction from degrees or radians
 *  - Simple arithmetic operators
 *  - Normalization utilities
 *  - Trig helpers
 *
 * This class intentionally avoids more complex features such as:
 *  - DMS formatting
 *  - user-defined literals
 *  - spherical/geodetic semantics
 *
 * Use this class for planar angle work (e.g. radar beam geometry,
 * Cartesian coordinate transforms).
 */

namespace euclid
{

  class EuclidAngle
  {

  public:

    //=======================================================================
    // Constants
    //=======================================================================

    static const double PI;
    static const double TWO_PI;
    static const double DEG_TO_RAD;
    static const double RAD_TO_DEG;

    //=======================================================================
    // Construction / destruction
    //=======================================================================

    /// Default constructor - initializes to 0 radians.
    EuclidAngle();

    /// Destructor.
    ~EuclidAngle();

    /// Copy constructor.
    EuclidAngle(const EuclidAngle &rhs);

    /// Assignment operator.
    EuclidAngle &operator=(const EuclidAngle &rhs);

    //=======================================================================
    // Named constructors
    //=======================================================================

    /// Construct from radians.
    static EuclidAngle fromRadians(double radians);

    /// Construct from degrees.
    static EuclidAngle fromDegrees(double degrees);

    //=======================================================================
    // Accessors
    //=======================================================================

    /// Get angle in radians.
    double radians() const;

    /// Get angle in degrees.
    double degrees() const;

    /// Check for finite value.
    bool isFinite() const;

    //=======================================================================
    // Utilities
    //=======================================================================

    /// Absolute value.
    EuclidAngle abs() const;

    /// Normalize to range [0, 2*pi).
    EuclidAngle normalize0To2Pi() const;

    /// Normalize to range [-pi, pi).
    EuclidAngle normalizeMinusPiToPi() const;

    /// String representation in degrees.
    std::string toStringDegrees(const std::string &format = "%.6f") const;

    //=======================================================================
    // Operators
    //=======================================================================

    EuclidAngle &operator+=(const EuclidAngle &rhs);
    EuclidAngle &operator-=(const EuclidAngle &rhs);
    EuclidAngle &operator*=(double scalar);
    EuclidAngle &operator/=(double scalar);

    bool operator==(const EuclidAngle &rhs) const;
    bool operator!=(const EuclidAngle &rhs) const;
    bool operator<(const EuclidAngle &rhs) const;
    bool operator<=(const EuclidAngle &rhs) const;
    bool operator>(const EuclidAngle &rhs) const;
    bool operator>=(const EuclidAngle &rhs) const;

  private:

    /// Private constructor from radians.
    explicit EuclidAngle(double radians);

    double _radians;

  };

  //=======================================================================
  // Free operators
  //=======================================================================

  EuclidAngle operator-(const EuclidAngle &angle);

  EuclidAngle operator+(const EuclidAngle &lhs, const EuclidAngle &rhs);
  EuclidAngle operator-(const EuclidAngle &lhs, const EuclidAngle &rhs);

  EuclidAngle operator*(const EuclidAngle &angle, double scalar);
  EuclidAngle operator*(double scalar, const EuclidAngle &angle);

  EuclidAngle operator/(const EuclidAngle &angle, double scalar);
  double operator/(const EuclidAngle &lhs, const EuclidAngle &rhs);

  //=======================================================================
  // Trig helpers
  //=======================================================================

  double sin(const EuclidAngle &angle);
  double cos(const EuclidAngle &angle);
  double tan(const EuclidAngle &angle);

  EuclidAngle arcSin(double x);
  EuclidAngle arcCos(double x);
  EuclidAngle arcTtan(double x);
  EuclidAngle ArcTan2(double y, double x);

  //=======================================================================
  // Stream output
  //=======================================================================

  std::ostream &operator<<(std::ostream &out, const EuclidAngle &angle);

} // namespace euclid

#endif
