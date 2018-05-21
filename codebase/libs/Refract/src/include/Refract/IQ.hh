// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/**
 *
 * @file IQ.hh
 *
 * @file IQ.hh
 * Class for holding two values, I and Q
 * @class IQ
 * Class for holding two values, I and Q
 */

#ifndef IQ_HH
#define IQ_HH

#include <Refract/RefractConstants.hh>
#include <Refract/RefractUtil.hh>
#include <rapmath/math_macros.h>
#include <dataport/port_types.h>
#include <cmath>

class IQ
{
public:

  /**
   * Empty constructor, I = Q = 0
   */
  inline IQ(void) : _i(0.0), _q(0.0) {}

  /**
   * Constructor with values passed in
   *
   * @param[in] i
   * @param[in] q
   */
  IQ(fl32 i, fl32 q) : _i(i), _q(q) {}

  /**
   * Constructor with values passed in
   *
   * @param[in] i
   * @param[in] q
   */
  IQ(double i, double q) : _i(i), _q(q) {}

  /**
   * The angle is in degrees..._i = cos(angle), _q=sin(angle)
   * @param[in] angle
   */
  IQ(double angle) :
    _i(cos(angle*DEG_TO_RAD)), _q(sin(angle*DEG_TO_RAD)) {}

  /**
   * Operator==
   *
   * @param[in] other  
   */
  inline bool operator==(const IQ& other) const
  {
    return _i == other._i && _q == other._q;
  }

  /**
   * Set I and Q to inputs
   * @param[in] i
   * @param[in] q
   */
  inline void set(double i, double q) {_i = i; _q = q;}

  /**
   * Operator+
   *
   * @param[in] other  To be added to this
   * @return new object with sum
   */
  inline IQ operator+(const IQ& other) const
  {
    double i = _i + other._i;
    double q = _q + other._q;
    return IQ(i, q);
  }

  /**
   * Operator-
   *
   * @param[in] other  To be subtracted from this
   * @return new object with difference
   */
  inline IQ operator-(const IQ& other) const
  {
    double i = _i - other._i;
    double q = _q - other._q;
    return IQ(i, q);
  }

  /**
   * Operator/
   *
   * @param[in] v I and Q are divided by input value
   * @return new object with ratio
   */
  inline IQ operator/(const double v) const
  {
    double i = _i/v;
    double q = _q/v;
    return IQ(i, q);
  }

  /**
   * Operator+=
   * @param[in] b  Added to local I and Q
   * @return reference to local object
   */
  inline IQ &operator+=(const IQ& b)
  {
    _i += b._i;
    _q += b._q;
    return *this;
  }

  /**
   * Operator-=
   * @param[in] b  Subtracted from local I&Q
   * @return reference to local object
   */
  inline IQ &operator-=(const IQ& b)
  {
    _i -= b._i;
    _q -= b._q;
    return *this;
  }
  
  /**
   * Operator/=
   * @param[in] b  Divide local I and Q by b
   * @return reference to local object
   */
  inline IQ &operator/=(double b)
  {
    _i /= b;
    _q /= b;
    return *this;
  }

  /**
   * Operator*=
   * @param[in] b  multiply local I and Q by b
   * @return reference to local object
   */
  inline IQ &operator*=(double b)
  {
    _i *= b;
    _q *= b;
    return *this;
  }

  /**
   * @return i*inp.i + q*inp.q
   * @param[in] inp
   */
  inline double iqProduct(const IQ &inp) const
  {
    return _i*inp._i + _q*inp._q;
  }

  /**
   * @return q*inpi - i*inpq
   * @param[in] inp
   */
  inline double iqCorrelation(const IQ &inp) const
  {
    return _q*inp._i -_i*inp._q;
  }
  
  /**
   * Destructor
   */
  inline ~IQ(void) {}

  /**
   * @return i
   */
  inline double i(void) const {return _i;}

  /**
   * @return q
   */
  inline double q(void) const {return _q;}

  /**
   * @return true if i=mi or q = mq
   * @param[in] mi  Missing i
   * @param[in] mq  Missing q
   */
  inline bool missingIorQ(double mi, double mq) const
  {
    return _i == mi || _q == mq;
  }

  /**
   * @return true if missing or bad i or q
   * @param[in] iBad  bad I
   * @param[in] iMissing  missing I
   * @param[in] qBad  bad Q
   * @param[in] qMissing  missing Q
   */
  inline bool isBadOrMissing(double iBad, double iMissing,
			     double qBad, double qMissing) const
  {
    return _i == iBad || _i == iMissing || _q == qBad || _q == qMissing;
  }

  /**
   * @return square of magnitude of I,Q vector
   */
  inline double normSquared(void) const
  {
    return RefractUtil::SQR(_i) + RefractUtil::SQR(_q);
  }  

  /**
   * @return the phase of IQ  whichis arctangent(Q/I)
   */
  inline double phase(void) const
  {
    if (_q == 0 && _i == 0)
    {
      return refract::INVALID;
    }
    else
    {
      return atan2(_q, _i)/DEG_TO_RAD;
    }
  }

  /**
   * Convert local IQ to phase difference between input and local
   * @param[in] iq
   */
  inline void phaseDiffV(const IQ &iq)
  {
    double tmp_a = _i;
    double tmp_b = _q;
    _i = tmp_a*iq._i  + tmp_b*iq._q;
    _q = tmp_a*iq._q  - tmp_b*iq._i;
  }

  /**
   * Convert local IQ to phase difference between input and local
   * @param[in] iq
   */
  inline void phaseDiff2V(const IQ &iq)
  {
    double tmp_a = _i;
    double tmp_b = _q;
    _i = tmp_a*iq._i + tmp_b*iq._q;
    _q = tmp_b*iq._i - tmp_a*iq._q;     
  }
  
  /**
   * @return magnitude of I,Q vector
   */
  inline double norm(void) const
  {
    return sqrt(RefractUtil::SQR(_i)+ RefractUtil::SQR(_q));
  }

  /**
   * Normalize the I,Q vector so it has magnitude 1.0
   */
  inline void normalize(void)
  {
    double n = norm();
    if (n != 0.0)
    {
      *this /= n;
    }
  }
  
  /**
   * @return a new oject that is the phase diff of local and input
   * local is unchanged.
   * @param[in] iq
   */
  inline IQ phaseDiffC(const IQ &iq) const
  {
    return IQ(_i*iq._i + _q*iq._q, _i*iq._q - _q*iq._i);
  }

  /**
   * @return a new oject that is the phase diff 'fit' of local and input
   * local is unchanged.
   * @param[in] iq
   */
  inline IQ phaseDiffFitC(const IQ &iq) const
  {
    return IQ(_i*iq._i - _q*iq._q, _q*iq._i + _i*iq._q);
  }


  /**
   * @return true if I or Q are zero
   */
  inline bool hasZero(void) const
  {
    return _i == 0 || _q == 0;
  }


  /**
   * @return true if I and Q are both zero
   */
  inline bool isZero(void) const
  {
    return _i == 0 && _q == 0;
  }
  
protected:
private:

  double _i;  /**< I */
  double _q;  /**< Q */

};

#endif
