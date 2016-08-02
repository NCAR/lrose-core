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
 * @file Geom.hh
 * @brief Simple geometry of 2d polar data
 * @class Geom
 * @brief Simple geometry of 2d polar data
 */

# ifndef    GEOM_H
# define    GEOM_H

#include <vector>

//----------------------------------------------------------------
class Geom
{
public:
  /**
   * Empty constructor
   */
  Geom(void);

  /**
   * Default constructor
   *
   * @param[in] isIndexed True if azimuths are equally spaced
   * @param[in] angleResDeg  az spacing if isIndexed=true
   * @param[in] az  azimuth degrees, ordered, when isIndexed=false
   * @param[in] nr  Number of gates
   * @param[in] r0  Minimum gate (meters)
   * @param[in] dr  Distance between gates (meters)
   * @param[in] outputAngleResDeg  the output angle resolution (output angles
   *                               are always indexed 0 to 360)
   */
  Geom(bool isIndexed, double angleResDeg, const std::vector<double> &az,
       int nr, double r0, double dr, double outputAngleResDeg);
  
  /**
   *  Destructor
   */
  virtual ~Geom(void);

  bool closestGateIndex(double gate, int &ig) const;
  bool closestOutputAzimuthIndex(double az, int &iaz) const;
  double ithGtMeters(int i) const {return _r0 + i*_dr;}
  double r0(void) const {return _r0; }
  double dr(void) const {return _dr; }
  int nGate() const {return _nr;}
  double ithOutputAz(int i) const {return _output_da*i;}
  int nOutputAz() const {return _output_na;}
  double deltaOutputAz(void) const {return _output_da;}

protected:
private:  

  bool _isIndexed; /**< azimuthal data indexing */
  double _da;      /**< delta degrees When isIndexed */
  double _r0;      /**< Smallest range meters */
  double _dr;      /**< Delta gate meters */
  int _nr;         /**< Number of gate grid points */
  std::vector<double> _az;  /**< The azimuth degrees, in order (actual) */

  double _output_da; /**< Azimuthal resolution, output */
  int    _output_na; /**< Azimuthal number of output rays, assume 0-360 */

};

# endif 
