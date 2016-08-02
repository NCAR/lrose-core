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
 *
 * @file Feature.hh
 *
 * @class Feature
 *
 * Base class for feature data.
 *  
 * @date 8/11/2014
 *
 */

#ifndef Feature_HH
#define Feature_HH

#include <string>

#include <Spdb/DsSpdb.hh>

#include "FeatureNcFile.hh"

using namespace std;

/** 
 * @class Feature
 */

class Feature
{
 public:

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /**
   * @brief Constructor
   *
   * @param[in] debug_flag Debug flag.
   * @param[in] verbose_flag Verbose debug flag.
   */

  Feature(const bool debug_flag = false, const bool verbose_flag = false);
  

  /**
   * @brief Destructor
   */

  virtual ~Feature(void);
  

  ///////////////////
  // Print Methods //
  ///////////////////

  /**
   * @brief Convert the object to a simple string for printing.
   */

  string toString() const
  {
    return "*** toString() not yet implemented for this feature ***";
  }
  
  /**
   * @brief Write the feature information to the given output stream.
   *
   * @param[in,out] out The output stream.
   * @param[in] spacer String to prepend to the output lines.
   */

  void print(ostream &out, string spacer = "") const;


  ///////////////////////////////
  // SPDB Input/Output Methods //
  ///////////////////////////////

  /**
   * @brief Write the feature to the given SPDB database.
   *
   * @param[in,out] spdb The SPDB database object.
   * @param[in] spdb_url The database URL.
   * @param[in] prod_id The product identifier.
   * @param[in] prod_label The product label.
   * @param[in] data_type The data type.
   * @param[in] data_type2 The second data type.
   *
   * @return Returns true on success, false on failure.
   */

  virtual bool toSpdb(DsSpdb &spdb,
		      const string &spdb_url,
		      const int prod_id,
		      const string &prod_label,
		      const int data_type,
		      const int data_type2) const
  {
    cerr << "*** toSpdb() not yet implemented for this feature ***" << endl;
    return false;
  }
  
  /**
   * @brief Write the feature to the given SPDB database buffer.  You must remember to call
   *        Spdb::put() after you have written all of the desired features to the buffer.
   *
   * @param[in,out] spdb The SPDB database object.
   * @param[in] data_type The data type.
   * @param[in] data_type2 The second data type.
   *
   * @return Returns true on success, false on failure.
   */

  virtual bool toSpdbBuffer(Spdb &spdb,
			    const int data_type,
			    const int data_type2) const
  {
    cerr << "*** toSpdbBuffer() not yet implemented for this feature ***" << endl;
    return false;
  }
  
  /**
   * @brief Set the feature information based on the data in the given SPDB chunk.
   *
   * @param[in] chunk The SPDB chunk.
   *
   * @return Returns true on success, false on failure.
   */

  virtual bool fromSpdbChunk(const Spdb::chunk_t &chunk)
  {
    cerr << "*** fromSpdbChunk() not yet implemented for this feature ***" << endl;
    return false;
  }
  

  //////////////////////////////
  // XML Input/Output Methods //
  //////////////////////////////

  /**
   * @brief Convert the feature information to XML.
   *
   * @return Returns a string with the XML information.
   */

  virtual string toXML() const
  {
    cerr << "*** toXML() not yet implemented for this feature ***" << endl;
    return "";
  }
  
  /**
   * @brief Set the feature information based on the given XML data.
   *
   * @param[in] xml_string The XML information.
   *
   * @return Returns true on success, false on failure.
   */

  virtual bool fromXML(const string &xml_string)
  {
    cerr << "*** fromXML() not yet implemented for this feature ***" << endl;
    return false;
  }


  /////////////////////////////////
  // netCDF Input/Output Methods //
  /////////////////////////////////

  /**
   * @brief Write the feature information to the given feature netCDF file.
   *
   * @param[in,out] nc_file The feature netCDF file.
   *
   * @return Returns true on success, false on failure.
   */

  virtual bool toNetcdf(FeatureNcFile &nc_file) const
  {
    cerr << "*** toNetcdf() not yet implemented for this feature ***" << endl;
    return false;
  }
  
  /**
   * @brief Set the feature information based on the information in the given
   *        netCDF file.
   *
   * @param[in] nc_file The feature netCDF file.
   * @param[in] record The record number of the feature.
   *
   * @return Returns true on success, false on failure.
   */

  virtual bool fromNetcdf(FeatureNcFile &nc_file, const int record)
  {
    cerr << "*** fromNetcdf() not yet implemented for this feature ***" << endl;
    return false;
  }
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Debug flag.
   */

  bool _debug;
  
  /**
   * @brief Verbose debug flag.
   */

  bool _verbose;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

};


#endif
