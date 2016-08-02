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
 * @file FiltInfoOutput.hh 
 * @brief The information needed to put filter results into output
 * @class FiltInfoOutput
 * @brief The information needed to put filter results into output
 */

#ifndef FILT_INFO_OUTPUT_HH
#define FILT_INFO_OUTPUT_HH
#include <string>
#include <cstdio>
#include <euclid/GridAlgs.hh>

class Data;
class GridProj;

//------------------------------------------------------------------
class FiltInfoOutput : public GridAlgs
{
public:

  typedef enum
  {
    GRID=0,
    VALUE=1,
    NONE=2
  } Info_t;

  /** 
   * Constructor for NONE
   * @param[in] extra   Pointer to extra information (if needed) or NULL
   */
  FiltInfoOutput(void *extra);

  /** 
   * Constructor for GRID output
   * @param[in] g  Grid to copy in
   * @param[in] extra   Pointer to extra information (if needed) or NULL
   */
  FiltInfoOutput(const Grid2d &g, void *extra);

  /**
   * Constructor for VALUE output
   * @param[in] v  Value to use
   * @param[in] extra   Pointer to extra information (if needed) or NULL
   */
  FiltInfoOutput(const double v, void *extra);

  /**
   * Destructor
   */
  virtual ~FiltInfoOutput(void);

  /**
   * @return true if type = GRID
   */
  inline bool isGrid(void) const {return _type == GRID;}

  /**
   * Return value if type = VALUE
   * @param[out] v  Value
   * @return true if type was = VALUE
   */
  bool getFiltInfoValue(double &v) const;

  /**
   * Store local state to Data if it makes sense to do so, assuming the
   * output is for data at some vertical level
   * @param[in] vlevel  Vertical level
   * @param[in] vlevel_index  Vertical level index
   * @param[in] gp  Grid projection
   * @param[in,out] out  Where to store to
   *
   * @return true if storing happened false if some mismatch in local type 
   *              and Data type
   */
  bool storeSlice(const double vlevel, const int vlevel_index,
		  const GridProj &gp, Data &out) const;

  /**
   * Store local state to Data if it makes sense to do so, assuming the
   * output is for single valued data
   * @param[in,out] out  Where to store to
   *
   * @return true if storing happened false if some mismatch in local type 
   *              and Data type
   */
  bool store1dValue(Data &out) const;

  /**
   * Set extra pointer member to input
   * @param[in] e
   */
  inline void storeExtra(void *e) { _extra = e;}

  /**
   * Return extra pointer, which can be NULL
   */
  inline void *getExtra(void) { return _extra;}

  /**
   * Set status to false
   */
  void setBad(void);

  /**
   * Set type to NO OUTPUT
   */
  void setNoOutput(void);

  /**
   * @return string for input type
   * @param[in] type
   */
  static std::string sType(FiltInfoOutput::Info_t type);

protected:
private:

  double _value;  /**< Value when _type = VALUE */
  Info_t _type;   /**< The type */
  bool _status;   /**< True if contents are ok */
  void *_extra;   /**< Extra information pointer, can be NULL */

};

#endif
