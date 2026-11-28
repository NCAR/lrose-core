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
 * @file MdvFuzzyCombine.hh
 * @brief The MdvFuzzyCombine algorithm.
 * @class MdvFuzzyCombine
 * @brief The MdvFuzzyCombine algorithm.
 */

# ifndef    MDV_FUZZY_COMBINE_HH
# define    MDV_FUZZY_COMBINE_HH

#include "ParmsMdvFuzzyCombine.hh"
#include <euclid/Grid2d.hh>
#include <euclid/Grid2dDistToNonMissing.hh>
#include <map>

class DsMdvx;
class Grid2d;
class MdvxProj;

//----------------------------------------------------------------
class MdvFuzzyCombine
{
public:

  /**
   * Default constructor
   *
   * @param[in] p  The parameters to put into state
   * @param[in] tidyAndExit  Cleanup function to call at exit
   *
   * Prepare the class for a call to the run() method
   */
  MdvFuzzyCombine(const ParmsMdvFuzzyCombine &p, void tidyAndExit(int));
  
  /**
   *  Destructor
   */
  virtual ~MdvFuzzyCombine(void);

  /**
   * Run the app
   */
  void run(void);


protected:
private:  

  /**
   * The Algorithm parameters, kept as internal state
   */
  ParmsMdvFuzzyCombine _parms;

  /**
   * Map from field name to the Grid2d Data which is read in
   */
  std::map<std::string, Grid2d> _grid;

  /**
   * Optional mask input
   */
  Grid2d _mask;
  Grid2d _erodedMask;
  Grid2d _unSmoothed;
  Grid2d _smoothed;
  Grid2d _nonTapered;

  /**
   * True if mask data is set
   */
  bool _hasMask;

  /**
   * True if first time through
   */
  bool _first;

  /**
   * missing data fill in object, used only when _hasMask
   */
  Grid2dDistToNonMissing _distFill;

  /**
   * mask missing data expansion object, used only when _hasMask
   */
  Grid2dDistToNonMissing _eroder;


  void _process(const time_t &gt, const int lt);
  void _process(const time_t &t);
  bool _read(DsMdvx &D);
  bool _readMask(DsMdvx &M, DsMdvx &D);
  bool _loadStaticMask(const time_t &t, DsMdvx &D);
  void _combine(Grid2d &g);
  void _write(DsMdvx &D, const Grid2d &g, const bool isFcst);
};

# endif 
