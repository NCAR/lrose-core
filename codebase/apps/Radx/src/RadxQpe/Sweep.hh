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
 * @file Sweep.hh
 * @brief One sweep of data
 * @class Sweep
 * @brief One sweep of data
 */

# ifndef    SWEEP_DATA_H
# define    SWEEP_DATA_H

#include "Parms.hh"
#include "Geom.hh"
#include "Field.hh"
#include <vector>
#include <radar/BeamHeight.hh>

//----------------------------------------------------------------
class Sweep
{
public:

  /**
   * Empty constructor, only elevation filled in
   */
  Sweep(double elev);

  /**
   * Construct from Radx information
   */
  Sweep(double elv, const RadxSweep &sweep, const std::vector<RadxRay *> &rays,
	    const Geom &geom);

  /**
   * Construct from parameters
   */
  Sweep(double elv, const Geom &geom, const Parms &parms);

  /**
   *  Destructor
   */
  virtual ~Sweep(void);

  /**
   * @return elevation angle
   */
  inline double elev(void) const {return _elev;}

  /**
   * @return beam ht in km msl
   * @param[in] igt  Index to gate
   * @param[in] geom  Geometry of sweep
   * @param[out] range to gate in km
   */
  double getBeamHtKmMsl(int igt, const Geom &geom, double &rangeKm) const;

  /**
   * Debug print
   */
  void print(void) const;

  /**
   * Fill in using Radx information, checking elevation of sweep against
   * local value
   * @return true if successful
   */
  bool fill(const RadxSweep &sweep, const std::vector<RadxRay *> &rays,
	    const Geom &geom);

  /**
   * Extract the value at a grid point for a named field
   * @param[in] igt  Gate index
   * @param[in] iaz  Azimuth index
   * @param[in] fieldName   Name of grid to use
   * @param[out] v  Value returned
   *
   * @return true if data is not missing at the point
   */
  bool value(int igt, int iaz, const std::string &fieldName, double &v) const;

  /**
   * Set value at a grid point for a named field
   *
   * @param[in] fieldName   Name of grid to write to
   * @param[in] igt  Gate index
   * @param[in] iaz  Azimuth index
   * @param[out] v  Value written
   *
   * @return true if able to write, false if fieldName isn't present.
   */
  bool setValue(const std::string &fieldName, int igt, int iaz, double v);

  /**
   * Set radar ht MSL in km
   */
  void setRadarHtKm(const Params &params, double val);

  typedef std::vector<Field>::iterator iterator;
  typedef std::vector<Field>::const_iterator const_iterator;
  typedef std::vector<Field>::reverse_iterator reverse_iterator;
  typedef std::vector<Field>::const_reverse_iterator const_reverse_iterator;
  iterator       begin()                       { return _grids.begin(); }
  const_iterator begin() const                 { return _grids.begin(); }
  size_t size() const                          { return _grids.size(); }
  iterator end()                               { return _grids.end(); }
  const_iterator end() const                   { return _grids.end(); }
  Field& operator[](size_t i)             {return _grids[i];}
  const Field& operator[](size_t i) const {return _grids[i];}
  Field& operator[](const std::string &name);
  const Field& operator[](const std::string &name) const;
  bool _isOK;

protected:

private:  

  double _elev;              /**< Elevation angle */
  std::vector<Field> _grids; /**< The fields in the sweep stored as grids */
  Field _bad; /**< this is small so each Sweep can have one */
  BeamHeight _beamHt;

};

# endif 
