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
 * @file Info.hh 
 * @brief Information from alg., can be written to XML in MDV and SPDB
 * @class Info
 * @brief Information from alg. can be written to XML in MDV and SPDB
 */

#ifndef INFO_H
#define INFO_H

#include <string>
#include <FiltAlg/Data2d.hh>
#include <FiltAlg/Data1d.hh>
#include <toolsa/MemBuf.hh>
#include <toolsa/LogStream.hh>

class DsMdvx;

//------------------------------------------------------------------
class Info
{
public:
  /**
   * Empty
   */
  Info(void);

  /**
   * Destructor
   */
  virtual ~Info(void);

  /**
   * Print to a logging object
   * @param[in] e  Logging type
   * @note calls virtual method dprint()
   */
  void info_print(const LogStream::Log_t e) const;

  /**
   * Clear to empty
   * @note calls virtual method dclear()
   */
  void clear(void);

  /**
   * Return the altitude information content (km msl)
   */
  inline double get_radar_alt_km(void) const {return _radar_alt_km;}

  /**
   * Action taken once per radar volume - store inputs to state
   * @param[in] t  Scan time
   * @param[in] lat  Radar lat.
   * @param[in] lon  Radar lon.
   * @param[in] alt  Radar altitude (km msl)
   * @param[in] radar_name  Name
   * @param[in] radar_loc  Description
   * @note calls virtual method dinit()
   */
  void init(const time_t &t, const double lat, const double lon, 
	    const double alt, const string &radar_name,
	    const string &radar_loc);
  /**
   * @return the fixed official name for 2d vertical level data
   */
  inline static string data2d_vlevel_name(void) {return "Vlevel_degrees";}

  /**
   * add input as 2d data 
   * @param[in] d the data
   * @param[in] vlevels the vertical levels (degrees) assocated with d
   * @note if vlevels already stored internally, not stored again.
   */
  void add_data2d(const Data2d &d, const Data2d &vlevels);

  /**
   * add input as 1d data 
   * @param[in] d the data
   * @return true if ok, false if input is not good Data1d
   */
  bool add_data1d(const Data1d &d);

  /**
   * Store entire content as an MDV chunk
   *
   * @param[in,out] m  Object in which to put the chunk
   */
  void store(DsMdvx &m) const;

  /**
   * Retrieve entire content as an MDV chunk
   *
   * @param[in,out] m  Object from which to get the chunk
   * @return true for success
   */
  bool retrieve(DsMdvx &m);

  /**
   * Store entire content as an SPDB product, write to input url.
   *
   * @param[in] spdb_url  Where to write SPDB
   */
  void output(const string &spdb_url);

  /**
   * Write internal state as an XML output string, written as a file
   * @param[in] path  Name of the file to write
   */
  void write_xml(const string &path) const;

  /**
   * Append internal state as an XML output string to a file
   * @param[in] path  Name of the file to append to
   */
  void append_xml(const string &path) const;

  /** 
   * Write internal state as an XML string
   * @return The string with XML
   * @note calls virtual method dwrite_xml()
   */
  string write_xml(void) const;

  /**
   * Read state from input XML string, including derived class
   * @param[in] s  String with XML in it.
   * @return true if successful.
   * @note calls virtual method dset_from_xml();
   */
  bool set_from_xml(const string &s);

  /**
   * assemble as XML
   * Load up an XML string from the state, and add to _memBuf
   */
  void assemble(void);

  /**
   * Get the assembled buffer info
   * @return the pointer to the buffer
   */
  inline const void *getBufPtr(void) const { return _memBuf.getPtr(); }

  /**
   * Get the assembled buffer length
   * @return the length (bytes)
   */
  inline int getBufLen(void) const { return _memBuf.getLen(); }

  /**
   * Disassemble buffer, sets the values into the object. Handles byte swapping.
   * @Return 0 on success, -1 on failure
   * @param[in] buf  The buffer
   * @param[in] len  The buffer length
   */
  int disassemble(const void *buf, int len);

  /**
   * virtual function implemented in derived class
   */
  virtual void dclear(void) = 0;

  /**
   * virtual function implemented in derived class
   * @param[in] e Severity indicating logging or not
   */
  virtual void dprint(const LogStream::Log_t e) const = 0;

  /**
   * virtual function implemented in derived class
   */
  virtual void dinit(void) = 0;

  /**
   * virtual function implemented in derived class
   * @return XML string for derived information.
   */
  virtual string dwrite_xml(void) const = 0;

  /**
   * virtual function implemented in derived class
   * @param[in] s  String from which to get XML
   * @return true if successful
   */
  virtual bool dset_from_xml(const string &s) = 0;

protected:

  time_t _time;           /**< Data time */
  double _radar_alt_km;   /**< Altitude (km) of radar */

private:

  bool _well_formed;      /**< True if object well formed */

  string _radar_name;     /**< Name of radar */
  string _radar_loc;      /**< descrition of where radar is */
  double _radar_lat;      /**< Latitude of radar */
  double _radar_lon;      /**< Longitude of radar */

  vector<Data2d> _data2;  /**< Generic Two dimensional data
			   *   (one value per height) */
  vector<Data1d> _data1;  /**< Generic one dimensional data (one value) */

  
  MemBuf _memBuf;         /**< buffer for assemble / disassemble */
};

#endif
