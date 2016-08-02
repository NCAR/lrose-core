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
 * @file AzElev.hh
 * @brief Azimuth elevation pair, used as key for mapping
 * @class AzElev
 * @brief Azimuth elevation pair, used as key for mapping
 */

# ifndef    RADX_AZ_ELEV_HH
# define    RADX_AZ_ELEV_HH

#include <string>

//------------------------------------------------------------------
class RadxAzElev
{
public:

  /**
   * constructor
   */
  RadxAzElev(void);

  /**
   * constructor
   * @param[in] az  Angle degrees
   * @param[in] elev  Elevation angle degrees 
   */
  RadxAzElev(const double az, const double elev);

  /**
   *  destructor
   */
  virtual ~RadxAzElev(void);
  
  /**
   * @return true if a1 is at a higher elevation
   * than local object, or the same elevation angle
   * and a larger azimuth value
   *
   * @param[in] a1
   */
  bool operator<(const RadxAzElev &a1) const;

  /**
   * @return true if a1 has same azimuth and elevation
   * as local object.
   *
   * @param[in] a
   */
  bool operator==(const RadxAzElev &a) const;

  /**
   * @return true if object is good
   */
  inline bool ok(void) const {return _ok;}

  /**
   * Print contents to a string
   */
  std::string sprint(void) const;

  /**
   * @return azimuth
   */
  inline double getAz(void) const {return _az; }

  /**
   * @return elevation
   */
  inline double getElev(void) const {return _elev; }

protected:
private:  
  
  double _az;   /**< Az degrees */
  double _elev; /**< Elev angle degrees */
  bool _ok;     /**< Object is set flag */
};

# endif
