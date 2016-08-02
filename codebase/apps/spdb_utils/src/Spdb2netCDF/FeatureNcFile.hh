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
 * @file FeatureNcFile.hh
 *
 * @class FeatureNcFile
 *
 * Class controlling access to a Feature netCDF file.
 *  
 * @date 8/12/2014
 *
 */

#ifndef FeatureNcFile_HH
#define FeatureNcFile_HH

#include <map>
#include <string>

#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>

#include "NetcdfClassic.hh"

using namespace std;

/** 
 * @class FeatureNcFile
 */

class FeatureNcFile : public NetcdfClassic
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

  FeatureNcFile(const string &file_format_string,
		const bool debug_flag = false);
  

  /**
   * @brief Destructor
   */

  virtual ~FeatureNcFile(void);
  

  ///////////////////////////////
  // File manipulation methods //
  ///////////////////////////////

  /**
   * @brief Open the file for writing.
   *
   * @param[in] output_dir Output directory.
   * @param[in] interval_start The start time for the data.
   * @param[in] interval_end The end time for the data.
   *
   * @return Returns true on success, false on failure.
   */

  bool openWrite(const string &output_dir,
		 const DateTime &interval_start, const DateTime &interval_end);
  
  /**
   * @brief Get the number of records currently in the file.
   *
   * @return Returns the number of records currently in the file.
   */

  size_t getNumRecords() const
  {
    return _recordDim.getSize();
  }
  


  /**
   * @brief Add a 1-D variable to the file.  The variable will have an unlimited
   *        dimension of the record number.
   *
   * @param[in] nc_var The netCDF variable object to use.
   * @param[in] name The name of the variable.
   * @param[in] standardName The standard name of the variable.
   * @param[in] longName The long name of the variable.
   * @param[in] ncType The netCDF type of the variable.
   * @param[in] units The units for the variable.
   * @param[in] coordinates The coordinates for the variable.
   *
   * @return Returns true on success, false on failure.
   */

  bool addVar(const string &name, 
	      const string &standardName,
	      const string &longName,
	      NcType ncType, 
	      const string &units = "",
	      const string &coordinates = "")
  {
    NcVar *nc_var = new NcVar();
    
    if (NetcdfClassic::addVar(*nc_var, name, standardName, longName,
			      ncType, _recordDim, units) != 0)
      return false;
    
    if (coordinates.length() > 0)
      addAttr(*nc_var, "coordinates", coordinates);
    
    _varList[name] = nc_var;
    
    return true;
  }
  
  /**
   * @brief Add a 1-D altitude variable to the file.  The variable will have
   *        an unlimited dimension of the record number.
   *
   * @param[in] nc_var The netCDF variable object to use.
   * @param[in] name The name of the variable.
   * @param[in] standardName The standard name of the variable.
   * @param[in] longName The long name of the variable.
   * @param[in] ncType The netCDF type of the variable.
   * @param[in] units The units for the variable.
   *
   * @return Returns true on success, false on failure.
   */

  bool addAltVar(const string &name, 
		 const string &standardName,
		 const string &longName,
		 NcType ncType, 
		 const string &units = "")
  {
    NcVar *nc_var = new NcVar();
    
    if (NetcdfClassic::addVar(*nc_var, name, standardName, longName,
			      ncType, _recordDim, units) != 0)
      return false;
    
    addAttr(*nc_var, "positive", "up");
    addAttr(*nc_var, "axis", "Z");
    
    _varList[name] = nc_var;
    
    return true;
  }
  
  /**
   * @brief Set the value of the variable list initialized flag.  This flag
   *        should be set to true after addVar() or addAltVar() has been called
   *        for each of the variables in the file.
   *
   * @param[in] initialized Flag indicating whether the list has been
   *                        initialized.
   */

  void setVarListInitialized(const bool initialized)
  {
    _varListInitialized = initialized;
  }
  
  /**
   * @brief Query the value of the variable list initialized flag.
   *
   * @return Returns true if the list has been initialized, false otherwise.
   */

  bool isVarListInitialized() const
  {
    return _varListInitialized;
  }
  
  /**
   * @brief Append a record value to the given variable.
   *
   * @param[in] name The name of the variable in the netCDF file.
   * @param[in] value The data value.
   * @param[in] record_num The record number for the new value.
   *
   * @return Returns true on success, false on failure.
   */

  bool appendValue(const string &name, const double value, const size_t record_num);
  bool appendValue(const string &name, const int value, const size_t record_num);
  bool appendValue(const string &name, const bool value, const size_t record_num);
  bool appendValue(const string &name, const string &value, const size_t record_num);
  
  /**
   * @brief Get the name of the current output file.
   *
   * @return Returns the name of the current output file.
   */

  string getFileName() const
  {
    return _filePath.getFile();
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
  
  /**
   * @brief The path to the current output file.
   */

  Path _filePath;

  /**
   * @brief The file format string.  This string identifies the type of data
   *        stored in the file.
   */

  string _fileFormat;
  
  /**
   * @brief The record dimension.  This is the unlimited dimension.
   */

  NcDim _recordDim;
  
  /**
   * @brief Flag indicating whether the variable list has been initialized.
   *        The value of this flag is controlled by the client.
   */
  
  bool _varListInitialized;
  

  /**
   * @brief Pointers to the variables in the netCDF file.
   */

  map< string, NcVar* > _varList;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Construct the file path based on the given information.
   *
   * @param[in] output_dir Output directory.
   * @param[in] interval_start The start time for the data.
   * @param[in] interval_end The end time for the data.
   *
   * @return Returns the constructed file path.
   */

  Path _constructFilePath(const string &output_dir,
			  const DateTime &interval_start,
			  const DateTime &interval_end) const;
  
};


#endif
