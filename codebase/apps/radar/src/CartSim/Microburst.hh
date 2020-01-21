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
 * @file  Microburst.hh
 * @brief  
 * @class  Microburst
 * @brief  
 */

# ifndef    MICROBURST_H
# define    MICROBURST_H

#include "Params.hh"
#include "Thing.hh"
#include "Xyz.hh"
#include <rapmath/FuzzyF.hh>
class Data;
class LineAndPoint;

//----------------------------------------------------------------
class Microburst : public Thing
{
public:

  /**
   * Default constructor
   */
  Microburst(const Params &P, const Params::Microburst_t &mP);
  
  /**
   * Destructor
   */
  virtual ~Microburst(void);

protected:

  virtual void _addToData(const Xyz &loc, Data &data) const;

private:  

  double     _mag;     /* peak mag. of microburst wind vectors (m/s) */
  double     _r;       /* radius of mb to peak outflow(meters) */
  double     _z;       /* height of mb to peak outflow(meters) */
  double     _core_dbz;/* reflectivity of descending core */
  bool       _line;    /**< True for microburst line */
  std::vector<Xyz> _loc;     /* center point location (meters), or line pts*/
  bool       _gain;    /* true for a "gain" shape*/
  Xyz        _motion;  /* motion vector (m/s) */

  FuzzyF _Utilda;
  FuzzyF _Stilda;
  FuzzyF _Wtilda;
  FuzzyF _Wrmax;

  void _mbLine(const Xyz &loc, Data &data) const;
  void _mbPoint(const Xyz &loc, Data &data) const;
  bool _windLine(const Xyz &loc, double &r, double &z, Xyz &wind) const;
  bool _windPoint(const Xyz &loc, double &r, double &z, Xyz &wind) const;
  void _add(const Xyz &wind, double r, double z, Data &data) const;
  bool _windMag(double r, double z, double &vert, double &horiz) const;

  /**
   * Return scale value [0,1] as a function of radius and height
   */
  double _radialCoreScale(double r, double z, double scale,
			  const FuzzyF &mapping) const;

  Xyz _wind(const Xyz &v, double r, double vert, double horiz) const;
  double _windZ(double r, double z, double rscale, double size_scale) const;
  double _windU(double r, double z, double size_scale) const;
  bool _lineWindDirection(const LineAndPoint &info, Xyz &wind) const;



  // Xyz _wind(const Xyz &loc, const Xyz &current0, const Xyz &v,
  // 	    double r, double rscale, double size_scale);
  // void _windMag(double rscale, double size_scale, double r, double z,
  // 		 double &vert_wind, double &horiz_wind);
  // double _windZ(double r, double z, double rscale, double size_scale);
};

# endif 
