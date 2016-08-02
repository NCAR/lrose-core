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
 * @file MdvHandler.hh
 *
 * @class MdvHandler
 *
 * Class for handling MDV file input.
 *  
 * @date 3/18/2009
 *
 */

#ifndef MdvHandler_HH
#define MdvHandler_HH

#include <string>
#include <vector>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>

#include "FieldInfo.hh"

using namespace std;


/** 
 * @class MdvHandler
 */

class MdvHandler
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
   */

  MdvHandler(const bool debug_flag = false);
  

  /**
   * @brief Destructor
   */

  virtual ~MdvHandler(void);
  

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  /**
   * @brief Initialize the object.
   *
   * @param[in] url URL of the MDV database.
   * @param[in] search_margin_secs Search margin, in seconds, to use when
   *                               retrieving files from the database.
   * @param[in] use_field_names Flag indicating whether to identify fields
   *                            in the MDV file by name or by number.
   * @param[in] debug_flag Debug flag.
   *
   * @return Returns true on success, false on failure.
   */

  bool init(const string &url, const int search_margin_secs,
	    const bool use_field_names,
	    const bool debug_flag);
  

  /**
   * @brief Set the DBZ field by name.
   *
   * @param[in] field_name DBZ field name.
   * @param[in] field_num DBZ field number.
   *
   * @return Returns true on success, false on failure.
   */

  bool setDbzField(const string &field_name,
		   const int field_num);
  

  /**
   * @brief Set the ZDR field by name.
   *
   * @param[in] field_name ZDR field name.
   * @param[in] field_num ZDR field number.
   *
   * @return Returns true on success, false on failure.
   */

  bool setZdrField(const string &field_name,
		   const int field_num);
  

  /**
   * @brief Add a statistic field to the list.
   *
   * @param[in] field_name Statistic field name.
   * @param[in] field_num  Statistic field number.
   * @param[in] is_log     Flag inidicating whether this is a log field.
   *
   * @return Returns true on success, false on failure.
   */

  bool addStatisticField(const string &field_name,
			 const int field_num,
			 const bool is_log);
  

  /**
   * @brief Add a discrete statistic field to the list.
   *
   * @param[in] field_name Statistic field name.
   * @param[in] field_num  Statistic field number.
   *
   * @return Returns true on success, false on failure.
   */

  bool addDiscreteStatisticField(const string &field_name,
				 const int field_num);
  

  /**
   * @brief Add a threshold field to the list.
   *
   * @param[in] field_name Threshold field name.
   * @param[in] field_num Threshold field number.
   *
   * @return Returns true on success, false on failure.
   */

  bool addThresholdField(const string &field_name,
			 const int field_num);
  

  /**
   * @brief Add a dropsize threshold field to the list.
   *
   * @param[in] field_name Threshold field name.
   * @param[in] field_num Threshold field number.
   *
   * @return Returns true on success, false on failure.
   */

  bool addDropsizeThreshField(const string &field_name,
			      const int field_num);
  

  //////////////////////////
  // Input/output methods //
  //////////////////////////

  /**
   * @brief Get the MDV data for the given data time.
   *
   * @param[in] data_time Desired data time.
   *
   * @return Returns true on success, false on failure.
   */

  bool getData(const time_t data_time);


  ////////////////////
  // Access methods //
  ////////////////////

  /**
   * @brief Get the master header from the input file.
   *
   * @return Returns the master header.
   */

  inline Mdvx::master_header_t getMasterHeader()
  {
    return _mdvx.getMasterHeader();
  }
  

  /**
   * @brief Get the DBZ field pointer.
   *
   * @return Returns the pointer to the DBZ field.
   */

  inline const MdvxField *getDbzField() const
  {
    return _dbzFieldInfo.getField();
  }
  
    
  /**
   * @brief Get the ZDR field pointer.
   *
   * @return Returns the pointer to the ZDR field.
   */

  inline const MdvxField *getZdrField() const
  {
    return _zdrFieldInfo.getField();
  }
  
    
  /**
   * @brief Get the indicated statistic field pointer.
   *
   * @param field_index The index of the field in the list of fields.
   *
   * @return Returns the pointer to the indicated statistic field
   *         on success, 0 on failure.
   */

  inline const MdvxField *getStatField(const size_t field_index) const
  {
    if (field_index >= _statFieldInfo.size())
      return 0;
    
    return _statFieldInfo[field_index].getField();
  }
  
    
  /**
   * @brief Get the indicated discrete statistic field pointer.
   *
   * @param field_index The index of the field in the list of fields.
   *
   * @return Returns the pointer to the indicated discrete statistic field
   *         on success, 0 on failure.
   */

  inline const MdvxField *getDiscreteStatField(const size_t field_index) const
  {
    if (field_index >= _discreteFieldInfo.size())
      return 0;
    
    return _discreteFieldInfo[field_index].getField();
  }
  
    
  /**
   * @brief Get the number of threshold fields in the input.
   *
   * @return Returns the number of threshold fields.
   */

  inline size_t getNumThresholdFields() const
  {
    return _thresholdFieldInfo.size();
  }
  

  /**
   * @brief Get the indicated threshold field pointer.
   *
   * @param field_index The index of the field in the list of fields.
   *
   * @return Returns the pointer to the indicated threshold field
   *         on success, 0 on failure.
   */

  inline const MdvxField *getThresholdField(const size_t field_index) const
  {
    if (field_index >= _thresholdFieldInfo.size())
      return 0;
    
    return _thresholdFieldInfo[field_index].getField();
  }
  
    
  /**
   * @brief Get the number of dropsize threshold fields in the input.
   *
   * @return Returns the number of dropsize threshold fields.
   */

  inline size_t getNumDropsizeThreshFields() const
  {
    return _dropsizeThreshFieldInfo.size();
  }
  

  /**
   * @brief Get the indicated dropsize threshold field pointer.
   *
   * @param field_index The index of the field in the list of fields.
   *
   * @return Returns the pointer to the indicated dropsize threshold field
   *         on success, 0 on failure.
   */

  inline const MdvxField *getDropsizeThreshField(const size_t field_index) const
  {
    if (field_index >= _dropsizeThreshFieldInfo.size())
      return 0;
    
    return _dropsizeThreshFieldInfo[field_index].getField();
  }
  
    
  /**
   * @brief Get the scan mode from the MDV file.
   *
   * @return Returns the scan mode for this file.
   */

  int getScanMode() const;
  

  /**
   * @brief Get the list of statistic fields.
   *
   * @return Returns a list of statistic fields.
   */

  inline vector< FieldInfo >getStatFieldList() const
  {
    return _statFieldInfo;
  }
  
    
  /**
   * @brief Get the list of discrete statistic fields.
   *
   * @return Returns a list of discrete statistic fields.
   */

  inline vector< FieldInfo >getDiscreteStatFieldList() const
  {
    return _discreteFieldInfo;
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
   * @brief The current MDV file.
   */

  DsMdvx _mdvx;
  
  /**
   * @brief URL for the MDV files.
   */

  string _url;
  
  /**
   * @brief Search margin in seconds.
   */

  int _searchMarginSecs;
  
  /**
   * @brief The time of the latest MDV file request.
   */
  
  time_t _latestRequestTime;
  
  /**
   * @brief Flag indicating whether to use field names or field numbers
   *        to identify the MDV fields.
   */

  bool _useFieldNames;
  
  /**
   * @brief Flag indicating whether to read a threshold field from the
   *        MDV file.
   */

  bool _readThresholdField;
  
  /**
   * @brief Information about the statistics fields.
   */

  vector< FieldInfo > _statFieldInfo;
  
  /**
   * @brief Information about the discrete statistics fields.
   */

  vector< FieldInfo > _discreteFieldInfo;
  
  /**
   * @brief List of field names of all of the fields that need to be read
   *        from the MDV file.  Used only if _useFieldNames is true.
   */

  set< string > _readFieldNames;

  /**
   * @brief List of field indexes of all of the fields that need to be read
   *        from the MDV file.  Used only if _useFieldNames is false.
   */

  set< int > _readFieldNums;
  
  /**
   * @brief The DBZ field information.
   */

  FieldInfo _dbzFieldInfo;
  
  /**
   * @brief The ZDR field information.
   */

  FieldInfo _zdrFieldInfo;
  
  /**
   * @brief List of the threshold fields.
   */

  vector< FieldInfo > _thresholdFieldInfo;
  
  /**
   * @brief List of the dropsize threshold fields.
   */

  vector< FieldInfo > _dropsizeThreshFieldInfo;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  MdvxField *_findField(const string &field_name, const int field_num) const
  {
    int field_index = 0;
    bool field_found;
    
    if (_useFieldNames)
    {
      for (set< string >::const_iterator read_field_name =
	     _readFieldNames.begin();
	   read_field_name != _readFieldNames.end();
	   ++read_field_name, ++field_index)
      {
	if (*read_field_name == field_name)
	{
	  field_found = true;
	  break;
	}
      } /* endfor - read_field_name */
    }
    else
    {
      for (set< int >::const_iterator read_field_num =
	     _readFieldNums.begin();
	   read_field_num != _readFieldNums.end();
	   ++read_field_num, ++field_index)
      {
	if (*read_field_num == field_num)
	{
	  field_found = true;
	  break;
	}
      } /* endfor - read_field_name */
    }

    // If we didn't find the field, return 0

    if (!field_found)
      return 0;
    
    return _mdvx.getField(field_index);
  }
  
  bool _readFile(const time_t data_time);
  

};


#endif
