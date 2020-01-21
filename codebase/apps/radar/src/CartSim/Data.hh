// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// © University Corporation for Atmospheric Research (UCAR) 2009-2010. 
// All rights reserved.  The Government's right to use this data and/or 
// software (the "Work") is restricted, per the terms of Cooperative 
// Agreement (ATM (AGS)-0753581 10/1/08) between UCAR and the National 
// Science Foundation, to a "nonexclusive, nontransferable, irrevocable, 
// royalty-free license to exercise or have exercised for or on behalf of 
// the U.S. throughout the world all the exclusive rights provided by 
// copyrights.  Such license, however, does not include the right to sell 
// copies or phonorecords of the copyrighted works to the public."   The 
// Work is provided "AS IS" and without warranty of any kind.  UCAR 
// EXPRESSLY DISCLAIMS ALL OTHER WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
// ANY IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE.  
//  
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include <toolsa/copyright.h>
/**
 * @file  Data.hh
 * @brief  data fields at a point
 * @class  Data
 * @brief  data fields at a point
 */

# ifndef    DATA_HH
# define    DATA_HH

#include "Params.hh"
#include "Xyz.hh"

//----------------------------------------------------------------
class Data
{
public:

  /**
   * Default constructor
   */
  Data(void);
  
  /**
   * Set data to ambient values using params
   */
  Data(const Params &p);
  
  /**
   * Destructor
   */
  virtual ~Data(void);

  /**
   * @return velocity vector
   */
  inline Xyz getVel(void) const {return _vel;}

  /**
   * Set velocity vector to input
   * @param[in] v
   */
  inline void setVel(const Xyz &v) { _vel = v;}

  /**
   * Set dbz value to input
   * @param[in] dbz
   */
  inline void setDbz(double dbz) {_ref = dbz;}

  /**
   * Set dbz value to max of input and current value
   * @parma[in] dbz
   */
  inline void setMaxDbz(double dbz)
  {
    if (_ref < dbz)
    {
      _ref = dbz;
    }
  }

  /**
   * Set spectrum width to max of input and local value
   * @param[in] sw
   */
  inline void setMaxSpectrumWidth(const double sw) 
  {
    if (sw > _sw)
    {
      _sw = sw;
    }
  }

  /**
   * Add input to current turbulent noise value
   * @param[in] noise
   */
  inline void addTurbNoise(const double noise)
  {
    _turb_noise += noise;
  }

  /**
   * @return the value associated with a type
   * @param[in] f  The type
   */
  double value(Params::Field_t f) const;

  /**
   * @return the magnitude of the velocity
   */
  inline double windSpeed(void) const { return _vel.magnitude();}

  /**
   * Add local noise to the local fields
   */
  void addNoise(void);

  /**
   * Set clutter to input value
   */
  inline void setClutter(const double c) {_clutter = c;}

protected:
private:  

  Xyz     _vel;       /**< velocity  (m/s) */
  double  _ref;       /**< reflectivity (dbz) */
  double  _snr;       /**< signal to noise ratio */
  double  _sw;        /**< spectrum width */
  double  _clutter;   /**< clutter value at a point */
  double  _noise;     /**< noise to add to fields */
  double  _turb_noise; /**< noise to add to fields due to turbulence*/
};

# endif 
