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
 * @file Field.hh
 * @brief One field of data, represented as a Grid2d
 * @class Field
 * @brief One field of data, represented as a Grid2d
 */

# ifndef    FIELD_DATA_H
# define    FIELD_DATA_H

#include "Params.hh"
#include <euclid/Grid2d.hh>
#include <Radx/RadxField.hh>
#include <dataport/port_types.h>

class RadxRay;
class RadxSweep;
class Geom;
class GateMapper;

//----------------------------------------------------------------
class Field : public Grid2d
{
public:

  /**
   * Empty constructor
   */
  Field(void);

  /**
   * Default constructor
   * @param[in] name  Field name
   * @param[in] sweep Sweep with data for name
   * @param[in] rays  The rays pointed to by field
   * @param[in] geom  Local grid geometry
   *
   * This constructor fills in the local members using information from the
   * rays and sweep.
   */
  Field(const std::string &name, const RadxSweep &sweep,
	const std::vector<RadxRay *> &rays, const Geom &geom);

  /**
   * Default constructor
   *
   * @param[in] field Parameters used to fill in state
   * @param[in] geom  Local grid geometry
   *
   * This constructor sets all data values to missing
   */
  Field(const Params::output_field_t &field, const Geom &geom);

  /**
   * Default constructor
   *
   * @param[in] field Parameters used to fill in state
   * @param[in] geom  Local grid geometry
   *
   * This constructor sets all data values to missing
   */
  Field(const Params::rainrate_field_t &field, const Geom &geom);

  /**
   *  Destructor
   */
  virtual ~Field(void);

  /**
   * @return units
   */
  inline std::string units(void) const {return _units;}

  /**
   * Debug print
   */
  void print(void) const;

  /**
   * Add data for one azimuth to the local grid using inputs
   * @param[in] gateIndexMap Connection between radx gates and grid gates
   * @param[in] ia  azimuth index
   * @param[in] data  Data from Radx at all gates
   */
  void addDataAtAzimuth(const GateMapper &map, int iy, const fl32 *data);

  /**
   * Create data from local Grid2d for one azimuthal beam.
   * Checks that the input gates dimension is same as local value
   * Allocates memory for returned array
   *
   * @param[in] iaz  Azimuth index
   * @param[in] nr   Number of gates
   *
   * If the dimensions are inconsistent returns NULL
   */
  Radx::fl32 *createData(int iaz, int nr) const;

  /**
   * @return true if input field is the fixed 'bad' field
   * @param[in] f  Field to check
   */
  static bool isBad(const Field &f);


  bool _isOK; /**< True if this object is good */

protected:
private:  

  std::string _units;   /**< Each field has units */

};

# endif 
