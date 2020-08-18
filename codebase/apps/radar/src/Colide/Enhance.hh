/**
 * @file Enhance.hh
 * @brief Used for Enhance filter algorithm
 * @class Enhance
 * @brief Used for Enhance filter algorithm
 */
# ifndef    Enhance_H
# define    Enhance_H

#include "ParmEnhance.hh"
#include "EnhancementD.hh"
#include "Bad.hh"
#include <euclid/Grid2d.hh>
#include <toolsa/TaThreadDoubleQue.hh>

class FiltEnhanceExtra;
class EnhancementOffsetsD;

class Enhance
{
public:
  Enhance(const ParmEnhance &p);
  virtual ~Enhance();
  
  void line_enhance_direction(const Grid2d *input, Grid2d *output);
  void line_enhance(const Grid2d *input, Grid2d *output);

  /**
   * Compute method needed for threading
   * @param[in] i  Information
   */
  static void compute(void *i);

private:  

  /**
   * @class EnhanceThreads
   * @brief Simple class to instantiate TaThreadDoubleQue by implementing
   * the clone() method.
   */
  class EnhanceThreads : public TaThreadDoubleQue
  {
  public:
    /**
     * Empty constructor
     */
    inline EnhanceThreads() : TaThreadDoubleQue() {}
    /**
     * Empty destructor
     */
    inline virtual ~EnhanceThreads() {}
    /**
     * Clone a thread and return pointer to base class
     * @param[in] index 
     */
    TaThread *clone(const int index);
  };

  ParmEnhance _parm;
  EnhanceThreads _thread;

  inline double _enhancement_xy(const Grid2d &input, int x, int y,
				int nx, int ny, const EnhancementD &e,
				const EnhancementOffsetsD &o) const
  {
    double det, ang;
    double iv;
    if (!input.getValue(x,  y, iv))
      return 0.0;
    if (iv == 0.0)
      return 0.0;
    if (!e.best_fit(input, x, y, nx, ny, o, det, ang))
      return 0.0;
    det *= iv;
    return det;
  }


  inline double _enhancement_angle_xy(const Grid2d &input, int x, int y,
				      int nx, int ny, const EnhancementD &e,
				      const EnhancementOffsetsD &o) const
  {
    double det;
    double ang = 0;
    double iv;
    if (!input.getValue(x,  y, iv))
      return badvalue::ANGLE;
    if (iv == 0.0)
      return badvalue::ANGLE;
    if (!e.best_fit(input, x, y, nx, ny, o, det, ang))
      return badvalue::ANGLE;
    return ang;
  }
};

# endif     /* CLDLINEIMG_H */
