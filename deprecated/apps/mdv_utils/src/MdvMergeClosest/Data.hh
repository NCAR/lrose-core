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
#include <toolsa/copyright.h>
/**
 * @file Data.hh
 * @brief Base class for input and output data
 * @class Data
 * @brief Base class for input and output data
 */

# ifndef    DATA_H
# define    DATA_H

#include "Parms.hh"
#include <euclid/Grid2d.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxProj.hh>

#include <map>
#include <string>

//----------------------------------------------------------------
class Data
{
public:

  /**
   * Constructor
   * @param[in] dparm  Params for this data 
   * @param[in] parm   Overall params
   */
  Data(const ParmInput &dparm, const Parms &parm);

  /**
   *  Destructor
   */
  virtual ~Data(void);

  /**
   * Debug print
   */
  void print(void) const;

  inline string getUrl(void) const {return _dataParm._url;}
  inline MdvxProj getProjection(void) const {return _proj;}

  /**
   * Read data at _time into the input DsMdvx object, and fill in local
   * state
   * @param[in] in  Object for reading MDV
   * @return true for success
   */
  bool read(DsMdvx &in);

  /**
   * Set all data in local state to missing
   */
  void setAllMissing(void);

  /**
   * Set the values for the various fields at a point to the input
   * values at that point
   * @param[in] d  Data to copy in
   * @param[in] x  Index
   * @param[in] y  Index
   */
  void setValues(const Data &d, int x, int y);

protected:

  ParmInput _dataParm;  /**< Params for this data */
  Parms _parm;          /**< Overall params */

  /**
   * mapping from field name to grid data
   */
  std::map<std::string, Grid2d> _grid;

  MdvxProj _proj;          /**< grid projection as gotten from MDV read */
  bool _projectionIsSet;   /**< True once _proj is set */

  time_t _time; /**< valid time of current grid data */

private:  

  bool _setField(DsMdvx &d, const Mdvx::master_header_t &mhdr,
		 const std::string &name);
};

# endif 
