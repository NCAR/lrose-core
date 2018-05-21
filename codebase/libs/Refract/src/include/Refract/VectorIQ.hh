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
 * @file VectorIQ.hh
 *
 * @file VectorIQ.hh
 * Class for holding a vector of IQ values
 * @class VectorIQ
 * Class for holding a vector of IQ values
 */

#ifndef VectorIQ_HH
#define VectorIQ_HH

#include <Refract/IQ.hh>
#include <dataport/port_types.h>
#include <rapmath/math_macros.h>
#include <string>
#include <cstring>
#include <vector>

class FieldWithData;

class VectorIQ
{
public:

  /**
   * Empty constructor, vector is empty
   */
  inline VectorIQ(void) : _scanSize(0) {}

  /**
   * Constructor in which data is allocated and set to a value
   * @param[in] scanSize  # of points
   * @param[in] value  at each point I=Q=value
   */
  VectorIQ(int scanSize, double value);
    
  /**
   * Constructor with I and Q passed in as fl32 arrays
   * @param[in] i
   * @param[in] q
   * @param[in] scanSize  Number of points in i and q
   */
  VectorIQ(fl32 *i, fl32 *q, int scanSize);

  /**
   * Destructor
   */
  inline ~VectorIQ(void) {}

  inline int num(void) const {return _scanSize;}
  
  /**
   * Initialize using inputs
   * @param[in] i   Array of I
   * @param[in] q   Array of Q
   * @param[in] scanSize  Length of array
   */
  void initialize(fl32 *i, fl32 *q, int scanSize);
  
  /**
   * Copy local state into input arrays
   * @param[in,out]  i  Array of length scanSize
   * @param[in,out]  q  Array of length scanSize
   * @param[in] scanSize  Length of i,q inputs
   * @return true if length is correct and data copied
   */
  bool copyToArrays(fl32 *i, fl32 *q, int scanSize) const;

  /**
   * operator=
   * @param[in] v
   */
  inline VectorIQ &operator=(const VectorIQ &v)
  {
    if (&v == this)
    {
      return *this;
    }
    _scanSize = v._scanSize;
    _iq = v._iq;
    return *this;
  }
  
  /**
   * operator[]  
   * @param[in] i  Index
   * @return reference to i'th IQ
   */
  inline const IQ& operator[](size_t i) const {return _iq[i];}

  /**
   * operator[]  
   * @param[in] i  Index
   * @return reference to i'th IQ
   */
  inline IQ& operator[](size_t i) {return _iq[i];}
  
  /**
   * operator+
   * Add input to local state
   * @param[in] d
   */
  inline VectorIQ &operator+=(const VectorIQ &d)
  {
    for (int i=0; i<_scanSize; ++i)
    {
      _iq[i] += d[i];
    }
    return *this;
  }

  /**
   * Debug phase create
   * @return vector
   */
  std::vector<double> createDebugPhaseVector(void) const;

  /**
   * Normsquaured create
   * @return vector
   */
  std::vector<double> createNormSquaredVector(void) const;

  /**
   * Norm create
   * @return vector
   */
  std::vector<double> createNormVector(void) const;

  /**
   * Set to (0,0) everywhere
   */
  void setAllZero(void);

  /**
   * Set to (0,0) in a range [i0,i1]
   */
  void setRangeZero(int i0, int i1);

  /**
   * For all points where r >= rMin set data to input 
   * at all other points, don't change local data
   *
   * @param[in] rMin
   * @param[in] numRange
   * @param[in] numAz
   * @param[in] inp
   */
  void copyLargeR(int rMin, int numRange, int numAz, const VectorIQ &inp);

  /**
   * Normalize each IQ to mag=1
   */
  void normalize(void);

  /**
   * Take a 3 point IQ average and normalize, return new object
   * @param[in] icenter  Center point for normalization
   */
  IQ normalized3ptAverage(int icenter) const;

  /**
   * @return refractivity measure for IQ centered at a point
   * @param[in] icenter  center index
   * @param[in] slope
   */
  double refractivity(int icenter, double slope) const;

  /**
   * Copy input into local
   * @param[in] f
   */
  void copyIQ(const VectorIQ &f);

  /**
   * Copy input into local.  Where bad or missing, set IQ to 0,0
   * @param[in] inp
   * @param[in] iBad  bad I
   * @param[in] iMissing  missing I
   * @param[in] qBad  bad Q
   * @param[in] qMissing  missing Q
   */
  void copyIQFilterBadOrMissing(const VectorIQ &inp, double iBad,
				double iMissing, double qBad,
				double qMissing);
  
  /**
   * Modify local object to be phase diff with input
   * @param[in] f
   */
  void phaseDiffV(const VectorIQ &f);

  /**
   * Set local to b.phaseDiffC(c);
   * @param[in] b
   * @param[in] c
   */
  void phaseDiffV(const VectorIQ &b, const VectorIQ &c);

  /**
   * Modify local object to be phase diff '2' with input
   * @param[in] b
   */
  void phaseDiff2V(const VectorIQ &b);

  /**
   * @normalize using input SNR data
   * @param[in] SNR
   */
  void normalizeUsingSNR(const FieldWithData &SNR);

  /**
   * Divide each IQ by corresponding norm data
   * @param[in] norm 
   */
  void normalizeWithVector(const std::vector<double> &norm);


  /**
   * Multiply each IQ by quality[i]/norm()
   */
  void normalizeWithQuality(const std::vector<double> quality);

  /**
   * Smooth close to radar
   * @param[in] rMin  Min radius index
   * @param[in] rMax  Max radius index
   * @param[in] numAzimuth
   * @param[in] numGates
   * @param[in] difPrevScan  Difference IQ
   */
  void smoothClose(int rMin, int rMax, int numAzimuth, int numGates,
		   const VectorIQ &difPrevScan);

  /**
   * Smooth far from radar
   * @param[in] rMax  Max radius index for not far
   * @param[in] numAzimuth
   * @param[in] numGates
   * @param[in] difPrevScan  Difference IQ
   */
  void smoothFar(int rMax, int numAzimuth, int numGates,
		 const VectorIQ &difPrevScan);


  void shiftDown(int i0, int i1);

  void setSlopes(int numBeams, int smoothRange);

protected:
  std::vector<IQ> _iq;   /**< The vector of IQ */
  int _scanSize;         /**< The size of the vector */
private:


};

#endif
