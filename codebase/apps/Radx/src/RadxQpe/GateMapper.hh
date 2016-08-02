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
 * @file GateMapper.hh
 * @brief Mapping that connects Radx gate indices to Grid2d gate indices
 * @class GateMapper
 * @brief Mapping that connects Radx gate indices to Grid2d gate indices
 */

# ifndef    GATE_MAPPER_HH
# define    GATE_MAPPER_HH

#include <vector>
class Geom;
class GateMapper1;

//----------------------------------------------------------------
class GateMapper
{
public:
  /**
   * Empty constructor
   */
  GateMapper(void);

  /**
   * @param[in] radx_r0  Range to closest gate in radx space (meters)
   * @param[in] radx_nr  Number of gates in radx space
   * @param[in] grid_r0  Range to closest gate in grid space (meters)
   * @param[in] geom  Grid2d geometry
   */
  GateMapper(double radx_r0, int radx_nr, double grid_r0, const Geom &geom);

  /**
   *  Destructor
   */
  virtual ~GateMapper(void);

  /**
   * @return number of associated mappings between radx and grid
   */
  inline std::size_t size(void) const            { return _map.size(); }

  typedef std::vector<GateMapper1>::iterator iterator;
  typedef std::vector<GateMapper1>::const_iterator const_iterator;
  typedef std::vector<GateMapper1>::reverse_iterator reverse_iterator;
  typedef std::vector<GateMapper1>::const_reverse_iterator const_reverse_iterator;

  std::vector<GateMapper1>::iterator begin()             { return _map.begin(); }
  std::vector<GateMapper1>::const_iterator begin() const { return _map.begin(); }
  std::vector<GateMapper1>::iterator end()               { return _map.end(); }
  std::vector<GateMapper1>::const_iterator end() const   { return _map.end(); }

  /**
   * @return i'th mapping reference
   * @param[in] i
   */
  inline GateMapper1 & operator[](std::size_t i) { return _map[i]; }

  /**
   * @return i'th mapping const reference
   * @param[in] i
   */
  inline const GateMapper1 & operator[](std::size_t i) const { return _map[i]; }

  bool _isOK;  /**< True if object is good */

protected:
private:  

  std::vector<GateMapper1> _map;  /**< the associated index mappings */

};

/**
 * @class GateMapper1
 * @brief  pair of matching gate indices for radx and for grid
 *
 * This is a 'struct class'
 */
class GateMapper1
{
public:

  /**
   * @param[in] radIndex
   * @param[in] griIndex
   */
  inline GateMapper1(int radIndex, int gridIndex) : _radxIndex(radIndex),
						    _grid2dIndex(gridIndex)
  {
  }

  /**
   * Destructor 
   */
  inline ~GateMapper1(void) {}

  int _radxIndex;       /**< Radx index */
  int _grid2dIndex;     /**< Grid2d index */
};


# endif 
