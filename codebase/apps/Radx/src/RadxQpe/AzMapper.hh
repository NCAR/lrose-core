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
#include <toolsa/copyright.h>
/**
 * @file AzMapper.hh
 * @brief Mapping that connects azimuth values to RadxRay indices
 * @class AzMapper
 * @brief Mapping that connects azimuth values to RadxRay indices
 */

# ifndef    AZ_MAPPER_HH
# define    AZ_MAPPER_HH


#include <vector>

class RadxSweep;
class RadxRay;
class AzMapper1;


//----------------------------------------------------------------
class AzMapper
{
public:

  /**
   * Empty constructor
   */
  AzMapper(void);

  /**
   * Constructor builds up state for a sweep
   * @param[in] sweep The Radx sweep
   * @param[in] rays  The rays pointers from the RadxVol
   */
  AzMapper(const RadxSweep &sweep, const std::vector<RadxRay *> &rays);

  /**
   *  Destructor
   */
  virtual ~AzMapper(void);

  /**
   * @return the ray index that closest matches an azimuth, within a tolerance
   *         or -1 if there are no azimuths within the tolerance
   * @param[in] a  The azimuth to look for  (degrees)
   * @param[in] maxDelta  The tolerance (degrees)
   */
  int closestRayIndex(double a, double maxDelta) const;

  /**
   *@ return number of items in the mapping
   */
  inline std::size_t size(void) const            { return _map.size(); }

  /**
   * @return reference to the i'th map
   * @param[in] i
   */
  inline AzMapper1 & operator[](std::size_t i) { return _map[i]; }

  /**
   * @return const reference to the i'th map
   * @param[in] i
   */
  inline const AzMapper1 & operator[](std::size_t i) const { return _map[i]; }

protected:
private:  


  std::vector<AzMapper1> _map;  /**< The mapping */

};

/**
 * @class AzMapper1
 * @brief  an azimuth (degrees) and associated ray index 
 *
 * This is a 'struct class'
 */
class AzMapper1
{
public:

  /**
   * @param[in] rayIndex
   * @param[in] azDegrees
   */
  inline AzMapper1(int rayIndex, double azDegrees) :
    _rayIndex(rayIndex), _azimuthDeg(azDegrees)
  {
  }

  /**
   * Destructor
   */
  inline ~AzMapper1(void) {}

  int _rayIndex;      /**< index */
  double _azimuthDeg; /**< az */
};

# endif 
