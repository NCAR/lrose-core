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

//   File: $RCSfile: Stats.hh,v $
//   Version: $Revision: 1.9 $  Dated: $Date: 2016/03/04 02:22:11 $

/**
 * @file Stats.hh
 * @class Stats
 */

# ifndef    Stats_HH
# define    Stats_HH

#include <Mdv/MdvxProj.hh>
#include <string>
#include <vector>
#include "Stat.hh"

/*----------------------------------------------------------------*/
class Stats
{
public:

  Stats(void);
  Stats(const time_t &t, const int lt, const std::string &name,
	const double min_diff_thresh, const bool show_all,
	const bool show_info);
  Stats(const time_t &t, const std::string &name, const double min_diff_thresh,
	const bool show_all, const bool show_info);

  /**
   *  destructor
   */
  virtual ~Stats();

  void setField(const std::string &fname);

  void fieldMissingInOne(void);
  void fieldHdrError(void);
  void fieldReadError(void);
  void inc_one_missing(void);
  bool process_diff(const int x, const int y, const double diff);
  void addInfo(const std::string &format, ...);

  void print_final(void) const;
  void print_one(const MdvxProj &proj) const;

  /**
   * Public members
   */
  int _nread_error;
  int _nmhdr_error;
  int _nmetadata_diff;
  std::string _name;
  Stat *_current;

protected:
private:  

  time_t _t;
  int _lt;
  std::vector<Stat> _stat;

  double _min_diff_threshold;
  bool _show_all;
  bool _show_info;
  std::vector<std::string> _info;
};

# endif     /* Stats_HH */
