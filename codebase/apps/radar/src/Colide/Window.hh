/**
 * @file Window.hh
 * @brief a template window
 *
 * @class Window
 * @brief a template window
 *
 * template window..windows are used for rotating boxes around a point.
 */

# ifndef   WINDOW_H
# define   WINDOW_H

#ifdef WINDOW_MAIN
namespace window
{
  int MAX_NANG = 16;
  int MAX_NP = 32;
}
#else
namespace window
{
  extern int MAX_NANG;
  extern int MAX_NP;
}
#endif

class Window
{
public:

  /**
   * default constructor..empty
   */
  Window(void);

  /**
   * Window for a box length by width, (number of angles is computed from
   * box size.)
   * @param[in] length  Box length (npt)
   * @param[in] width Gox width (npt)
   */
  Window(double length, double width);
  

  /**
   * Window for a box length by width, rotated through nangles total
   * evenly spaced in [0,180).
   * 
   * @param[in] length  Box length (npt)
   * @param[in] width Gox width (npt)
   * @param[in] nangles  Total number of rotation angles
   */
  Window(double length, double width, int nangles);

  /**
   * Destructor
   */
  virtual ~Window(void);


  /**
   * @return number of rotation angles.
   */
  inline int num_angles(void) const {return _nang;}

  /**
   * @return ith rotation angle, i=0,1,..num_angles()-1
   * @param[in] i  Index
   */
  inline double ith_angle(int i) const
  {
    if (_nang > 0)
    {
      return static_cast<double>(i)*180.0/static_cast<double>(_nang);
    }
    else
    {
      return 0.0;
    }
  }

  /**
   * @return distance between template gridpoints, which is roughly
   * length/(MAX_NP-1).
   */
  inline double step_size(void) const {return _step;}

  /**
   * @return number of template gridpoints lengthwise, which is roughly 
   * length/step
   */
  inline int num_points(void) const {return _npt;}

  double _length;  /**< Window length (npt) */
  double  _width;  /**< Window width (npt) */

protected:
private:  
  
  double _step;  /**< Step size within the window (npt) */
  int _nang;   /**< Total number of rotation angles */
  int _npt;    /**< Number of steps within the window */
};


# endif 

