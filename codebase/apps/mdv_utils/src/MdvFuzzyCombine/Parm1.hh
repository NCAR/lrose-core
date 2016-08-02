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
#include <copyright.h>

/**
 * @file Parm1.hh
 * @brief All the algorithm parameters for one input
 * @class Parm1
 * @brief All the algorithm parameters for one input
 *
 * The parameters are intentionally public as it is a stateless 'struct-like'
 * class.
 */

# ifndef    PARM_1_HH
# define    PARM_1_HH

#include <rapmath/FuzzyF.hh>
#include <string>

//------------------------------------------------------------------
class Parm1
{
public:

  /**
   * Default constructor (sets members to default values)
   */
  inline Parm1(const std::string &name, const double weight, const FuzzyF &f) :
    _name(name), _weight(weight), _mapping(f) {}
    

  /**
   * Destructor
   */
  inline virtual ~Parm1(void) {}

  std::string _name;  /**< Field name */
  double _weight;     /**< Weight to give it in combining */
  FuzzyF _mapping;    /**< Fuzzy function to transform values prior to combine*/

protected:
private:  

};

# endif
