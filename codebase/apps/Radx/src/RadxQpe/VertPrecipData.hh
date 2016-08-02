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
 * @file VertData.hh
 * @brief The precip data at one location for one precip type, all elevations
 * @class VertData
 * @brief The precip data at one location for one precip type, all elevations
 */

#ifndef VERT_PRECIP_DATA_HH 
#define VERT_PRECIP_DATA_HH 

#include <string>
#include <vector>

class Data;

class VertPrecipData
{
public:

  /**
   * Construct at a gate/azimuth using inputs
   * @param[in] data  Input fields
   * @param[in] igt  Gate index
   * @param[in] iaz  Azimuth index
   * @param[in] name  Field name
   */
  VertPrecipData (const Data &data, int igt, int iaz, const std::string &name,
		  int index);

  /**
   * Destructor
   */
  ~VertPrecipData(void);

  typedef std::vector<double>::iterator iterator;
  typedef std::vector<double>::const_iterator const_iterator;
  typedef std::vector<double>::reverse_iterator reverse_iterator;
  typedef std::vector<double>::const_reverse_iterator const_reverse_iterator;
  iterator       begin()                            { return _data.begin(); }
  const_iterator begin() const                      { return _data.begin(); }
  std::size_t size() const                          { return _data.size(); }
  iterator end()                                    { return _data.end(); }
  const_iterator end() const                        { return _data.end(); }
  double& operator[](std::size_t i)        { return _data[i]; }
  const double& operator[](std::size_t i) const { return _data[i]; }

  int getIndex(void) const { return _index;}

protected:
private:

  std::vector<double> _data;
  int _index;  /**< Index into precip types */
};

#endif
