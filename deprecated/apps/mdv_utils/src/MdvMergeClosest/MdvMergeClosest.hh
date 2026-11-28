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
 * @file MdvMergeClosest.hh
 * @brief The algorithm
 * @class MdvMergeClosest
 * @brief The algorithm
 */

# ifndef    MdvMergeClosest_hh
# define    MdvMergeClosest_hh

#include "Parms.hh"
#include "InputData.hh"
#include "OutputData.hh"
#include "Lookup.hh"
#include <vector>

//----------------------------------------------------------------
class MdvMergeClosest
{
public:

  /**
   * Default constructor
   *
   * @param[in] p  The parameters to put into state
   */
  MdvMergeClosest(const Parms &p);
  
  /**
   *  Destructor
   */
  virtual ~MdvMergeClosest(void);


  void update(const time_t &t);

protected:
private:  


  /**
   * The Algorithm parameters, kept as internal state
   */
  Parms _parms;


  /**
   * The input data, one per source
   */
  std::vector<InputData> _data;

  bool _first;  /**< True until the first data is set */

  /**
   * The output data
   */
  OutputData _output;

  /**
   * Projection that should be common to all inputs
   */
  MdvxProj _proj;


  /**
   * Lookup object
   */
  Lookup _lookup;

  bool _add(Data &d, int index);
  void _update(Data &d, int index);
  void _timeout(Data &d, int index);
};

# endif 
