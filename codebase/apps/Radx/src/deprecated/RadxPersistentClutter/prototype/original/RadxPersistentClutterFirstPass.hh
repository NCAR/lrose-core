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
 * @file RadxPersistentClutterFirstPass.hh
 * @brief The first pass algorithm
 * @class RadxPersistentClutterFirstPass
 * @brief The first pass algorithm
 *
 * The algorithm builds up the internal state and goes for convergence into
 * a stable clutter state
 */


#ifndef RADXPERSISTENTCLUTTERFIRSTPASS_H
#define RADXPERSISTENTCLUTTERFIRSTPASS_H

#include "RadxPersistentClutter.hh"

class RadxPersistentClutterFirstPass : public RadxPersistentClutter
{
  
public:

  /**
   * Constructor
   * @param[in] argc  Args count
   * @param[in] argv  Args
   * @param[in] cleanup  Method to call on exit
   * @param[in] outOfStore  Method to call  when not enough memory
   */
  RadxPersistentClutterFirstPass(int argc, char **argv, void cleanup(int),
				 void outOfStore(void));

  /**
   * Destructor
   */
  virtual ~RadxPersistentClutterFirstPass(void);

  #include "RadxPersistentClutterVirtualMethods.hh"

protected:
private:

  double _nvolume;       /**< The number of volumes processed so far */
  double _total_pixels;  /**< Number of gates (pixels) in the volume */

  std::string _ascii_fname;  /**< Name of ascii file to write to */
  std::string _freq_fname;   /**< Name of other ascii file to write to */
  std::vector<std::string> _ascii_output;  /**< The ascii output */
  std::vector<std::string> _freq_output;  /**< More ascii output */

  std::vector<double> _threshold;  /**< The threshold for each volume,
				    *   in increasing volume order */
  std::vector<double> _change;     /**< percent of points that change status 
				    *   in increasing volume order */
  int _kstar;                      /**< K* value from paper */

  /**
   * Initializes using a ray, updating _az, and _store members
   *
   * @param[in] ray  The ray
   * @param[in] az  Azimuth of some data
   * @param[in] elev  Elevation of some data
   * @param[in] x0  Ray smallest value
   * @param[in] dx  Ray delta value
   * @param[in] nx  Ray number of elements
   */
  bool _processFirstRay(const RadxRay &ray, const double az, const double elev,
			const double x0, const double dx, const int nx);

  /**
   *
   * Compute and return kstar value
   */
  int _computeHistoCutoff(void) const;

  /**
   * Return true if clutter detection has converged
   * @param[in] t  Time value
   */
  bool _output_for_graphics(const time_t &t);

  /**
   * Return true if clutter detection has converged
   */
  bool _check_convergence(void);

};

#endif
