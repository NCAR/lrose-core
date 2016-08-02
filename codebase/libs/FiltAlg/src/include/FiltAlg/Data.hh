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
 * @brief Data is either VlevelData (data at each vlevel) or Data1d (one number)
 * @class Data
 * @brief Data is either VlevelData (data at each vlevel) or Data1d (one number)
 */

#ifndef DATA_H
#define DATA_H
#include <string>
#include <FiltAlg/VlevelData.hh>
#include <FiltAlg/Data1d.hh>
#include <FiltAlg/Info.hh>

//------------------------------------------------------------------
class Data : public VlevelData, public Data1d
{
public:
  /**
   * @enum data_t 
   *
   * The types of data
   */
  typedef enum
  {
    GRID3D=0,  /**< 2d grid at each vlevel */
    DATA2D=1,  /**< one number at each vlevel */
    DATA1D=2,  /**< one number total */
    UNKNOWN=3  /**< Not set or not known */
  } Data_t;
    

  /**
   * Empty constructor, UNKNOWN
   */
  Data(void);

  /**
   * Constructor
   * @param[in] name   Name for the objecct
   * @param[in] type   Type of data
   * @param[in] is_output  True if this data is for output
   */
  Data(const string &name, const Data_t type, const bool is_output);

  /**
   * Destructor
   */
  virtual ~Data(void);

  /**
   * Debug print to stdout
   */
  void print(void) const;


  /**
   * @return object name
   */
  inline string get_name(void) const {return _name;}

  /** 
   * @return true if this data is for output
   */
  inline bool is_output(void) const {return _is_output;}

  /**
   * @return true if this is a 3d grid
   */
  inline bool is_grid3d(void) const {return _type == GRID3D;}

  /**
   * @return true if this is a single value
   */
  inline bool is_data1d(void) const {return _type == DATA1D;}

  /**
   * @return the type
   */
  inline Data_t get_type(void) const {return _type;}

  /**
   * @return true if input name same as local name
   * @param[in] name
   */
  inline bool name_equals(const char *name) const
  {
    string s = name;
    return s == _name;
  }

  /**
   * @return string name for a particular type
   * @param[in] type  A type
   */
  static string print_type(const Data_t type);

protected:
private:

  string _name;      /**< Name of object */
  Data_t _type;      /**< Type */
  bool _is_output;   /**< True if this data is for output */
};

#endif
