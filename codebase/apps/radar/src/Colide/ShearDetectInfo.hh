/**
 * @file ShearDetectInfo.hh 
 * @brief 
 * @class ShearDetectInfo
 * @brief 
 */

#ifndef SHEAR_DETECT_INFO_H
#define SHEAR_DETECT_INFO_H

class Grid2d;
class ShearDetection;
class ShearDetect;
class ShearDetectOffsets;


//------------------------------------------------------------------
class ShearDetectInfo
{
public:
  /**
   * Constructor
   *
   */
  inline ShearDetectInfo(const ShearDetection *alg, int y, int nx, int ny,
			 const Grid2d *inData,
			 const Grid2d *secondary,
			 const Grid2d *direction,
			 const ShearDetect *sd,
			 const ShearDetectOffsets *off,
			 Grid2d *out) :
    _alg(alg), _y(y), _nx(nx), _ny(ny), _inData(inData), _secondary(secondary),
    _direction(direction), _sd(sd), _off(off), _out(out)
  {}

  
  /**
   * Destructor
   */
  inline virtual ~ShearDetectInfo(void) {}

  const ShearDetection *_alg;
  int _y;
  int _nx;
  int _ny;
  const Grid2d *_inData;
  const Grid2d *_secondary;
  const Grid2d *_direction;
  const ShearDetect *_sd;
  const ShearDetectOffsets *_off;
  double _dirMissing;
  Grid2d *_out;
protected:
private:

};

#endif
