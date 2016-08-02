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
 * @file CombineData.hh 
 * @brief Information about one data field that is combined with others.
 * @class CombineData
 * @brief Information about one data field that is combined with others.
 * 
 */

#ifndef COMBINE_DATA_H
#define COMBINE_DATA_H
#include <string>
#include <FiltAlg/Data.hh>

//------------------------------------------------------------------
class CombineData 
{
public:
  /**
   * Constructor
   * Empty
   */
  CombineData(void);

  /**
   * Constructor
   * @param[in] name  Name for data
   * @param[in] is_input True if data is an input data 
   * @param[in] weight  Weight to give this data when combining
   */
  CombineData(const string &name, const bool is_input, const double weight);

  /**
   * Constructor with confidence input
   * @param[in] name  Name for data
   * @param[in] is_input True if data is an input data 
   * @param[in] conf_name  Name for confidence data
   * @param[in] conf_is_input True if confidence data is an input data 
   * @param[in] weight  Weight to give this data when combining
   */
  CombineData(const string &name, const bool is_input, 
	      const string &conf_name, const bool conf_is_input, 
	      const double weight);

  /**
   * Destructor
   */
  virtual ~CombineData(void);
  
  /**
   * Search inputs or outputs for match to name, set internal pointer _g to 
   * point to the matching data
   * @param[in] input  Input data 
   * @param[in] output  Output data 
   * 
   * @return false if nothing found.
   */
  bool create_comb_data(const vector<Data> &input, const vector<Data> &output);

  /**
   * @return pointer to a 2d slice for input vlevel
   * @param[in] vlevel  Vertical level (degrees)
   * @param[in] tolerance  Allowed error in vlevel compared to internal values
   * 
   * Returns NULL if no match
   */
  const VlevelSlice *matching_vlevel(const double vlevel,
				     const double tolerance) const;

  /**
   * @return pointer to a 2d slice confidence data for input vlevel
   * @param[in] vlevel  Vertical level (degrees)
   * @param[in] tolerance  Allowed error in vlevel compared to internal values
   * 
   * Returns NULL if no match
   */
  const VlevelSlice *matching_conf_vlevel(const double vlevel,
					  const double tolerance) const;

  /**
   * @return pointer to the data, might be NULL
   */
  inline const Data *get_data(void) const {return _g;}

  /**
   * @return pointer to the confidence data, might be NULL
   */
  inline const Data *get_conf_data(void) const {return _c;}

  /**
   * @return true if data has same type as input
   * @param[in] type  The type to compare against
   */
  bool type_equals(const Data::Data_t type) const;

  /**
   * @return the weight value
   */
  inline double get_weight(void) const {return _weight;}

  /**
   * @return true if input name same as local name
   * @param[in] name  To compare against
   */
  inline bool name_equals(const std::string &name) const
  {
    return name == _name;
  }

protected:
private:

  string _name;    /**< name of the data */
  bool _is_input;  /**< True if it is an input data */
  const Data * _g; /**< Pointer to the data */

  string _conf_name; /**< Name of optional confidence data */
  bool _is_conf_input; /**< True if confidence data is an input data */
  const Data *_c;  /**< Pointer to optional confidence data */

  double _weight;  /**< Weight for the data */
};

#endif

