/**
 * @file EnhanceInfo.hh 
 * @brief 
 * @class EnhanceInfo
 * @brief 
 */

#ifndef ENHANCE_INFO_H
#define ENHANCE_INFO_H

#include "EnhancementD.hh"
#include "EnhancementOffsetD.hh"

class Enhance;
class Grid2d;

//------------------------------------------------------------------
class EnhanceInfo
{
public:
  /**
   * Constructor
   *
   */
  inline EnhanceInfo(const Enhance *alg, int y, int nx, int ny,
		     const Grid2d *inData,
		     EnhancementD *e,
		     EnhancementOffsetsD *o,
		     Grid2d *outData,
		     bool interest) :
    _alg(alg), _y(y), _nx(nx), _ny(ny), _inData(inData),
    _e(e), _o(o), _outData(outData), _interest(interest) {}
  
  /**
   * Destructor
   */
  inline virtual ~EnhanceInfo(void)
  {
  }

  const Enhance *_alg;
  int _y;
  int _nx;
  int _ny;
  const Grid2d *_inData;
  EnhancementD *_e;        
  EnhancementOffsetsD *_o; 
  Grid2d *_outData;
  bool _interest;   /**< False for direction as output */

protected:
private:

};

#endif
