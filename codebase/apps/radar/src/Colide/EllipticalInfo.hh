/**
 * @file EllipticalInfo.hh 
 * @brief 
 * @class EllipticalInfo
 * @brief 
 */

#ifndef ELLIPTICAL_INFO_H
#define ELLIPTICAL_INFO_H

class Elliptical;
class Grid2d;


//------------------------------------------------------------------
class EllipticalInfo
{
public:
  typedef enum {ORIENTATION, CONFIDENCE, INTEREST} Info_t;

  /**
   * Constructor
   *
   */
  inline EllipticalInfo(const Elliptical *alg, int y, int nx, 
			Info_t info,
			const Grid2d *input,
			const Grid2d *orientation,
			Grid2d *output) :
    _alg(alg), _y(y), _nx(nx), _info(info), _input(input),
    _orientation(orientation), _output(output) {}

  
  /**
   * Destructor
   */
  inline virtual ~EllipticalInfo(void) {}

  const Elliptical *_alg;
  int _y;
  int _nx;
  Info_t _info;
  const Grid2d *_input;
  const Grid2d *_orientation;
  Grid2d *_output;

protected:
private:

};

#endif
