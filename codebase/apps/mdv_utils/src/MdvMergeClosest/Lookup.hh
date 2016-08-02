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
 * @file Lookup.hh
 * @brief  Lookup, contains info as to which input to use at each gridpoint
 * @class Lookup
 * @brief  Lookup, contains info as to which input to use at each gridpoint
 */

# ifndef    LOOKUP_H
# define    LOOKUP_H

#include "Parms.hh"
#include <Mdv/MdvxProj.hh>
#include <vector>

//----------------------------------------------------------------
class Lookup
{
public:

  /**
   * Empty constructor
   */
  Lookup(void);

  /**
   *  Destructor
   */
  virtual ~Lookup(void);

  /**
   * Initialize local state
   * @param[in] parms  Contains lat/lon information for each input
   * @param[in] proj  The project, contains everything else that is needed
   */
  void init(const Parms &parms, const MdvxProj &proj);

  /**
   * Change state to indicate the input is available
   * @param[in] index  Index to input
   */
  void update(int index);

  /**
   * Change state to indicate the input is unavailable
   * @param[in] index  Index to input
   */
  void timeout(int index);

  /**
   * @return index to which input is best at a point, -1 for none
   * @param[in] x
   * @param[in] y
   */
  int bestInput(int x, int y) const;

  inline const int &nx(void) const {return _nx;}
  inline const int &ny(void) const {return _ny;}
  inline const int &ninput(void) const {return _ninput;}

protected:
private:  

  bool *_available;  /**< availabity of each input, yes or no */
  double *_values;   /**< The distances, (nx,ny,ninput). */
  int *_lookup;      /**< The lookups, (nx,ny,ninput). Gives index to input,
		      * with _lookup(x,y,0) the best, (x,y,1) next best, etc. */
  int _nx;
  int _ny;
  int _ninput;
};

# endif 
