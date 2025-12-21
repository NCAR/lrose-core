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
 * @brief The non-precip data at one location, all elevations
 * @class VertData
 * @brief The non-precip data at one location, all elevations
 */

#ifndef VERT_DATA_HH 
#define VERT_DATA_HH 

#include <string>
#include <vector>

class VertData1;
class Data;
class BeamBlock;
class Parms;

class VertData
{
public:

  /**
   * Construct at a gate/azimuth using inputs
   * @param[in] data  Input fields except beam blockage
   * @param[in] block  Beam blockage input
   * @param[in] igt  Gate index
   * @param[in] iaz  Azimuth index
   * @param[in] params
   */
  VertData (const Data &data, const BeamBlock &block, 
	    int igt, int iaz, const Parms &params);

  /**
   * Destructor
   */
  ~VertData(void);

  typedef std::vector<VertData1>::iterator iterator;
  typedef std::vector<VertData1>::const_iterator const_iterator;
  typedef std::vector<VertData1>::reverse_iterator reverse_iterator;
  typedef std::vector<VertData1>::const_reverse_iterator const_reverse_iterator;
  iterator       begin()                            { return _data.begin(); }
  const_iterator begin() const                      { return _data.begin(); }
  std::size_t size() const                          { return _data.size(); }
  iterator end()                                    { return _data.end(); }
  const_iterator end() const                        { return _data.end(); }
  VertData1& operator[](std::size_t i)              { return _data[i]; }
  const VertData1& operator[](std::size_t i) const  { return _data[i]; }

protected:
private:

  std::vector<VertData1> _data;
};


/**
 * @class VertData1
 * @brief  One elevation's non-precip data, as a 'struct class'
 */
class VertData1
{
public:
  /**
   * Has SNR
   *
   * @param[in] pid  PID type
   * @param[in] beamBlockage  Percent blockage
   * @param[in] terrainheight
   * @param[in] height  Height above radar meters
   * @param[in] snr  Signal to noise ratio
   */
  inline VertData1(double pid, double beamBlockage, double terrainheight,
                   double elev, double height, double range, double snr) :
          _pid(pid),
          _hasSnr(true),
          _snr(snr),
          _beamE(beamBlockage),
	  _peak(terrainheight),
          _elevDeg(elev),
          _heightKm(height),
          _rangeKm(range)
  {
  }

  /**
   * Does not have SNR
   *
   * @param[in] pid  PID type
   * @param[in] beamBlockage Percent blockage
   * @param[in] terrainheight
   * @param[in] height  Height above radar meters
   */
  inline VertData1(double pid, double beamBlockage, double terrainheight,
                   double elev, double height, double range) :
          _pid(pid),
          _hasSnr(false),
          _snr(0.0),
          _beamE(beamBlockage),
          _peak(terrainheight),
          _elevDeg(elev),
          _heightKm(height),
          _rangeKm(range)
  {
  }
	    
  double _pid;        /**< PID */
  bool   _hasSnr;     /**< True if has signal to noise ratio */
  double _snr;        /**< SNR when _hasSnr */
  double _beamE;      /**< Cumulative beam blockage fraction */
  double _peak;       /**< Maximum terrain height */
  double _elevDeg;    /**< elevation angle (deg) */
  double _heightKm;   /**< height above radar (km) */
  double _rangeKm;    /**< range from radar (km) */

protected:
private:

};

#endif
