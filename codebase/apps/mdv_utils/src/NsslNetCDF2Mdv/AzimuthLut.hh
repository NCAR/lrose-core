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
 * @file AzimuthLut.hh
 * @brief Class for handling the azimuth lookup table.
 * @class AzimuthLut
 * @brief Class for handling the azimuth lookup table.
 */

#ifndef AzimuthLut_H
#define AzimuthLut_H

#include <iostream>
#include <string>
#include <vector>
#include <toolsa/LogStream.hh>

class AzimuthLut
{
  
public:

  /**
   * Constructor
   * @param[in] beamwidth
   */
  AzimuthLut(double beamwidth = 1.0);
  

  /**
   * Destructor
   */
  virtual ~AzimuthLut(void);


  /**
   * Clear out the current lookup table values.  This method
   * resets all of the azimuth indices to -1;
   */
  void clear(void);
  

  /**
   * Set the beam width to the given value.  This method
   * clears out all of the old index values and reinitializes
   * the lookup table at the new size.
   *
   * @param[in] beamwidth
   */
  void setBeamWidth(double beamwidth);
  

  /**
   * Add the given azimuth index to the lookup table.
   * @param[in] azimuth  The degrees
   * @param[in] azimuth_index  The index
   */
  inline void addAzIndex(double azimuth, int azimuth_index)
  {
    _azimuthIndices[(int)((azimuth / _beamWidth) + (_beamWidth / 2.0))] =
      azimuth_index;
  }
  

  /**
   * @return the azimuth index for the given beam index.
   * @param[in] beam_index
   *
   * Returns the azimuth index on success, -1 on failure.
   */
  inline int getAzIndex(int beam_index) const
  {
    if (beam_index < 0 || beam_index >= (int)_azimuthIndices.size())
      return -1;
    return _azimuthIndices[beam_index];
  }
  

  /**
   * Print the lookup table contents to the logger
   */
  inline void print() const
  {
    for (size_t i = 0; i < _azimuthIndices.size(); ++i)
      LOG(PRINT) << "   " << i << "   " << _azimuthIndices[i];
  }
  
protected:
  
  double _beamWidth;   /**< The beam width */
  std::vector<int> _azimuthIndices;  /**< The indices */
  
};

#endif
