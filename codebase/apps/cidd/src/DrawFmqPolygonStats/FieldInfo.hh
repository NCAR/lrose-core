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
 * @file FieldInfo.hh
 *
 * @class FieldInfo
 *
 * Information about a gridded field that will be used to generate statistics.
 *  
 * @date 9/1/2011
 *
 */

#ifndef FieldInfo_HH
#define FieldInfo_HH

#include <string>

#include <Mdv/MdvxField.hh>

using namespace std;


/** 
 * @class FieldInfo
 */

class FieldInfo
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

  FieldInfo();
  

  /**
   * @brief Constructor
   *
   * @param[in] field_name   The name of the field in the MDV file.
   * @param[in] field_num    The index of the field in the MDV file.
   * @param[in] is_log       Flag indicating whether this is a logarithmic
   *                           field.
   */

  FieldInfo(const string &field_name,
	    const int field_num,
	    const bool is_log = false);
  

  /**
   * @brief Destructor
   */

  virtual ~FieldInfo(void);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**
   * @brief Get a pointer to the field in the MDV file.
   *
   * @return Returns a pointer to the field in the file on success, 0 on
   *         failure.
   */

  inline const MdvxField *getField() const
  {
    return _field;
  }
  

  /**
   * @brief Get the field name.
   *
   * @return Returns the field name.
   */

  inline string getFieldName() const
  {
    return _fieldName;
  }
  

  /**
   * @brief Get the field number.
   *
   * @return Returns the field number.
   */

  inline size_t getFieldNum() const
  {
    return _fieldNum;
  }
  

  /**
   * @brief Get a flag indicating whether this field is stored on a logarithmic
   *        scale.
   *
   * @return Returns true if the field is stored on a logarithmic scale, false
   *         otherwise.
   */

  inline bool isLogField() const
  {
    return _isLog;
  }
  

  /**
   * @brief Set the field pointer.
   *
   * @param[in] field    The pointer to the MDV field.
   */

  inline void setField(MdvxField *field)
  {
    _field = field;
  }
  

  /**
   * @brief Set the field name.
   *
   * @param[in] field_name   The field name.
   */

  inline void setFieldName(const string &field_name)
  {
    _fieldName = field_name;
  }
  

  /**
   * @brief Set the field number.
   *
   * @param[in] field_num    The field number.
   */

  inline void setFieldNum(const size_t field_num)
  {
    _fieldNum = field_num;
  }
  

  /**
   * @brief Set the logarithmic flag.
   *
   * @param[in] is_log     The new logarithmic flag.
   */

  inline void setIsLog(const bool is_log)
  {
    _isLog = is_log;
  }
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief The name of the field, used to request the field from the MDV file.
   */

  string _fieldName;
  

  /**
   * @brief The index of the field, used to request the field from the MDV file.
   */

  size_t _fieldNum;
  

  /**
   * @brief Flag indicating that this field is stored on a logarithmic scale
   *        and so must be changed to a linear scale for some statistics.
   */

  bool _isLog;
  

  /**
   * @brief Pointer to the field in the MDV file.  The actual field object is
   *        owned by another object so will not be reclaimed here.
   */

  MdvxField *_field;
  

};


#endif
