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
 * @file OutputData.hh
 * @brief Handles data that is output 
 * @class OutputData
 * @brief Handles data  that is output
 */

# ifndef    OUTPUT_DATA_H
# define    OUTPUT_DATA_H

#include "Data.hh"
#include <Mdv/DsMdvx.hh>

//----------------------------------------------------------------
class OutputData : public Data
{
public:

  /**
   * @param[in] parm
   */
  OutputData(const Parms &parm);

  /**
   *  Destructor
   */
  virtual ~OutputData(void);

  /**
   * Initialize state using example input data
   * @param[in] data  Example data
   */
  void init(const Data &data);

  /**
   * Write contents to MDV at time
   * @param[in] t  Time
   */
  void write(const time_t &t);

protected:
private:  

  /**
   * Object used for writing
   */
  DsMdvx _mdv;

};

# endif 
