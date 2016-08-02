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

//   File: $RCSfile: Stat.hh,v $
//   Version: $Revision: 1.6 $  Dated: $Date: 2016/03/04 02:22:11 $

/**
 * @file Stats.hh
 * @class Stats
 */

# ifndef    Stat_HH
# define    Stat_HH

#include <Mdv/MdvxProj.hh>
#include <string>
#include <vector>
#include "Stat.hh"

/*----------------------------------------------------------------*/
class Stat
{
public:

  Stat(const std::string &name);

  /**
   *  destructor
   */
  virtual ~Stat();

  void process_diff(const int x, const int y, const double diff);

  void print_final(const int nc) const;
  void print_one(const MdvxProj &proj, const bool show_all,
		 const bool show_info, const int nc) const;
  void inc_one_missing(void);
  void addInfo(const std::string &format, ...);

  /**
   * Public members
   */
  std::string _name;

  double _min_diff;
  double _max_diff;
  double _ave_diff;

  int _nread_error;
  int _nfieldhdr_error;
  int _nmissing_field;

  double _ncompare;
  int _ndiff;
  int _nmissing_only_one;
  int _ntotal;
  int _max_x, _max_y;


protected:
private:  

  std::vector<std::string> _info;
  std::string _printNameWithPadding(const int nc) const;
};

# endif     /* Stat_HH */
