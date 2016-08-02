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
 * @file BeamBlock.hh
 * @brief Handles the beam block data set.
 * @class BeamBlock
 * @brief Handles the beam block data set.
 */

#ifndef BEAM_BLOCK_HH
#define BEAM_BLOCK_HH

#include "Data.hh"

class BeamBlock : public Data
{
public:

  /**
   * @param[in] params
   */
  BeamBlock (const Parms &params);

  /**
   * Destructor
   */
  ~BeamBlock(void);

  /**
   * Figure out which elevations most closely match those of the input 
   * data, and create grids for just those ones
   * @param[in] data  Data that is input
   * 
   * @return true if able to 'line up' input data and beam block data
   */
  bool align(const Data &data);

  // /**
  //  * @return the blockage percent at each elevation's grid at the closest
  //  *         gridpoint to the input values
  //  * @param[in] gate  Meters from radar
  //  * @param[in] az  Azimuth degrees
  //  * @param[in] name  Field name of blockage
  //  */
  // std::vector<double> blockVertValues(double gate, double az,
  // 				      const std::string &name) const;

  /**
   * @return index to the elevation that is closest to the input elevation,
   *         or -1 for nothing
   * @param[in] elev  Degrees
   */
  int closestElevIndex(double elev) const;


protected:
private:

  /**
   * The indices into the beam sweeps to use 
   */
  std::vector<int> _elevIndex;  

  /**
   *  The elevation angles found in input
   */
  std::vector<double> _beamElevations; 

};

#endif
