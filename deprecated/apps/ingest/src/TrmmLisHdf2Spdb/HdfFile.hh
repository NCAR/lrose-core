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
 * @file HdfFile.hh
 *
 * @class HdfFile
 *
 * Class controlling access to a TRMM HDF file.
 *  
 * @date 4/9/2009
 *
 */

#ifndef HdfFile_HH
#define HdfFile_HH

#include <string>
#include <vector>

#include <hdf/hdf.h>
#include <hdf/mfhdf.h>

#include <dataport/port_types.h>
#include <rapformats/ltg.h>
#include <rapformats/LtgGroup.hh>
#include <toolsa/DateTime.hh>

using namespace std;

/** 
 * @class HdfFile
 */

class HdfFile
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
   * @param[in] file_path Path for HDF file to process.
   * @param[in] debug_flag Debug flag.
   * @param[in] verbose_flag Verbose flag.
   */

  HdfFile(const string &file_path,
	  const bool debug_flag = false,
	  const bool verbose_flag = false);
  

  /**
   * @brief Destructor
   */

  virtual ~HdfFile(void);
  

  /**
   * @brief Initialize the HDF file.
   *
   * @return Returns true on success, false on failure.
   */

  bool init();
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**
   * @brief Get the flashes from the file.
   *
   * @param[out] flashes The flashes retrieved from the file.
   *
   * @return Returns true on success, false on failure.
   */

  bool getFlashes(vector< LTG_strike_t > &flashes);
  

  /**
   * @brief Get the groups from the file.
   *
   * @param[out] groups The groups retrieved from the file.
   *
   * @return Returns true on success, false on failure.
   */

  bool getGroups(vector< LtgGroup > &groups);
  

protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * @brief The name of the flash VDATA in the HDF files
   */

  static const string FLASH_VDATA_NAME;
  
  /**
   * @brief The list of fields to retrieve from the flash VDATA, in
   *        the order desired.
   */

  static const string FLASH_VDATA_FIELD_LIST;

  /**
   * @brief The number of bytes in each record read from the flash
   *        VDATA.  This is the number of bytes in the fields listed
   *        in FLASH_VDATA_FIELD_LIST.
   */

  static const size_t FLASH_VDATA_RECORD_SIZE;
  
  /**
   * @brief The name of the group VDATA in the HDF files
   */

  static const string GROUP_VDATA_NAME;
  
  /**
   * @brief The list of fields to retrieve from the group VDATA, in
   *        the order desired.
   */

  static const string GROUP_VDATA_FIELD_LIST;

  /**
   * @brief The number of bytes in each record read from the group
   *        VDATA.  This is the number of bytes in the fields listed
   *        in GROUP_VDATA_FIELD_LIST.
   */

  static const size_t GROUP_VDATA_RECORD_SIZE;
  
  /**
   * @brief Table used in converting TAI93 times into UTC.
   */

  static const double LEAPSECONDS_TAI93_OFFSET[];

  /**
   * @brief Table used in converting TAI93 times into UTC.
   */

  static const long LEAPSECONDS_DAYNUM[];

  /**
   * @brief Table used in converting TAI93 times into UTC.
   */

  static const int MONTH_DAYS[];
  

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
   * @brief The path to the TRMM HDF file.
   */

  string _filePath;
  

  /**
   * @brief HDF file identifier.
   */

  int32 _fileId;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Get the UTC date associated with the given day number.
   *
   * @param[in] day_num TAI93 day number.
   *
   * @return Returns a DateTime object containing the UTC date associated
   *         with the given TAI93 date.
   */

  static DateTime _getDate(const long day_num);
  

  /**
   * @brief Get the flash data from the given flash VDATA.
   *
   * @param[out] flashs The lightning flashes.
   * @param[in] vdata_id The identifier for the flash VDATA section.
   * @param[in] n_records The number of records in the flash VDATA.
   * @param[in] interlace_mode The interlace mode used in the flash VDATA.
   *
   * @return Returns true on success, false on failure.
   */

  bool _setFlashData(vector< LTG_strike_t > &flashes,
		     const int32 vdata_id,
		     const int32 n_records, const int32 interlace_mode);
  
  
  /**
   * @brief Get the group data from the given flash VDATA.
   *
   * @param[out] groups The lightning groups.
   * @param[in] vdata_id The identifier for the group VDATA section.
   * @param[in] n_records The number of records in the group VDATA.
   * @param[in] interlace_mode The interlace mode used in the group VDATA.
   *
   * @return Returns true on success, false on failure.
   */

  bool _setGroupData(vector< LtgGroup > &groups,
		     const int32 vdata_id,
		     const int32 n_records, const int32 interlace_mode);
  
  
  /**
   * @brief Convert the given TAI93 time to UTC.
   *
   * @param[in] tai93 The TAI93 time.
   *
   * @return Returns the associated UTC time.
   */

  DateTime _tai93toUtc(const float64 tai93);
  

};


#endif
