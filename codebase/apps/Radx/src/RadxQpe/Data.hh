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
 * @file Data.hh
 * @brief Handles any kind of volume data
 * @class Data
 * @brief Handles any kind of volume data
 */

#ifndef DATA_HH 
#define DATA_HH 

#include "Parms.hh"
#include "Geom.hh"
#include "Sweep.hh"
#include <Radx/RadxVol.hh>

class Data : public Geom
{
public:

  /**
   * @param[in] params
   */
  Data (const Parms &params);

  /**
   * Destructor
   */
  ~Data(void);

  /**
   * @return true if input data and local data do not match up
   * @param[in] data
   */
  bool mismatch(const Data &data) const;


  /**
   * Read in a volume, but do not set local _sweeps
   *
   * @param[in] path  File name
   * @param[in] fields  The fields to attempt to read in
   * @return true if successful
   */
  bool readLite(const std::string &path,
		const std::vector<std::string> &fields);
  /**
   * Read fields from a path, filling in all member values
   * @param[in] path  
   * @param[in] fields
   * @return true for success
   */
  bool read(const std::string &path, const std::vector<std::string> &fields);

  /**
   * Read fields for a triggered time, filling in all member values
   * @param[in] t  Trigger time
   * @param[in] fields
   * @return true for success
   */
  bool read(const time_t &t, const std::vector<std::string> &fields);

  /**
   * @return the elevation angles as a vector
   */
  std::vector<double> getElev(void) const;

  typedef std::vector<Sweep>::iterator iterator;
  typedef std::vector<Sweep>::const_iterator const_iterator;
  typedef std::vector<Sweep>::reverse_iterator reverse_iterator;
  typedef std::vector<Sweep>::const_reverse_iterator const_reverse_iterator;
  iterator       begin()                       { return _sweeps.begin(); }
  const_iterator begin() const                 { return _sweeps.begin(); }
  size_t size() const                          { return _sweeps.size(); }
  iterator end()                               { return _sweeps.end(); }
  const_iterator end() const                   { return _sweeps.end(); }
  Sweep& operator[](size_t i)                  { return _sweeps[i]; }
  const Sweep& operator[](size_t i) const      { return _sweeps[i]; }

  size_t index(const Sweep &s) const
  {
    for (size_t i=0; i<size(); ++i)
    {
      if (_sweeps[i].elev() == s.elev())
      {
	return i;
      }
    }
    return -1;
  }

  const RadxVol &getVol(void) const {return _vol;}


  bool _dataIsOK;   /**< Object ok yes or no */
  
protected:

  RadxVol _vol;       /**< Used for input */
  Parms _params;      /**< App params */

  /**
   * Sweeps for all fields at each elevation
   */
  std::vector<Sweep> _sweeps;

private:

  double _lat;        /**< Radar location */
  double _lon;        /**< Radar location */

  bool _readLite(RadxFile &primaryFile, const std::string &path);
};

#endif
