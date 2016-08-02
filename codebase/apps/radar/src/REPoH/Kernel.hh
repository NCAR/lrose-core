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
 * @file Kernel.hh 
 * @brief kernel around an interesting point
 * @class Kernel
 * @brief kernel around an interesting point
 */

#ifndef KERNEL_H
#define KERNEL_H

#include <string>
#include <vector>
#include "Params.hh"

class GridProj;
class Grid2d;
class CloudGap;
class DsSpdb;
class KernelGrids;

//*------------------------------------------------------------------
class Kernel
{
public:

  Kernel(const double vlevel, const bool is_far, const Grid2d &mask,
	 const Grid2d &omask, const CloudGap &g, const Grid2d &clumps,
	 const Params &params, const GridProj &gp);

  ~Kernel(void);


  /**
   * Good means passed all tests
   */
  inline bool is_good(void) const {return  _is_good;}

  /**
   * Ok means well formed with enough points
   */
  inline bool is_ok(void) const {return _ok;}

  inline int npt(void) const {return (int)_pts.size();}
  inline int npt_outside(void) const {return (int)_out_pts.size();}

  /**
   * Add a point to the kernel
   */
  inline void add(const int x, const int y)
  {
    _total_pts.push_back(pair<int,int>(x,y));
    _pts.push_back(pair<int,int>(x,y));
  }

  /**
   * Add a point to the 'outside' kernel points
   */
  inline void add_outside(const int x, const int y)
  {
    _total_out_pts.push_back(pair<int,int>(x,y));
    _out_pts.push_back(pair<int,int>(x,y));
  }

  /**
   * Put centerpoint to grid with value = _color
   */
  void cp_to_grid(Grid2d &g) const;

  /**
   * Put centerpoint to grid with value = input
   */
  void cp_to_grid(Grid2d &g, const double value) const;

  /**
   * Print out a one line status summary to stdout
   */
  void print_status(void) const;

  /**
   * Print out all the kernel points
   */
  void print(void) const;

  /**
   * Debugging: Print out kernel contents to a file for use by Scott Ellis
   * in a format he likes.
   */
  void print_debug(const std::string dir, const int id, const double vlevel,
		   const KernelGrids &grids, const double km_per_gate);

  /**
   * Finish processing a kernel of points using inputs.
   * Set i.d. to input i.
   * Decide if its good or bad using equations
   * Print summary status
   */
  void finish_processing(const time_t &time, const int i, const double vlevel,
			 const KernelGrids &grids, const Params &P,
			 const double km_per_gate);

  /**
   * Write a genpoly surrounding the kernel points to Spdb,
   * outside = true for points just outside the kernel
   * does not write when at origin
   */
  bool write_genpoly(const time_t &t, const int nx, const int ny,
		     const bool outside, DsSpdb &D) const;

  /**
   * @return the average point x and y, and the total attenuation
   */
  void get_attenuation(int &x, int &y, double &atten) const;

  /**
   * cubic equation for humidity using attenuation
   */
  static double humidity_from_attenuation(const double a);

protected:
private:

  bool _ok;
  bool _is_good;
  int _color;
  double _vlevel;
  int _center_x, _center_y;
  std::vector<std::pair<int,int> > _pts;
  std::vector<std::pair<int,int> > _out_pts;
  std::vector<std::pair<int,int> > _total_pts;
  std::vector<std::pair<int,int> > _total_out_pts;

  bool _pid_ok, _sdbz_ok, _sdbz_out_ok, _D0_ok, _corr_ok, _sdbz_diff_ok;
  bool _dbz_diff_ok;
  double _sdbz_ave, _sdbz_ave_out, _kdbz_ave, _D0, _z_ave, _ZDR_ave, _Ag, _q;
  double _corr, _sdbz_diff;
  int _dbz_diff_npt_removed;

  void _set_good(const KernelGrids &grids, const Params &P,
		 const double km_per_gate);
  bool _set_genpoly(const time_t &time, const int id, const int nx,
		    const int ny);
  void _print(FILE *fp, const Grid2d &g) const;
  int _x_ave(void) const;
  void _to_grid0(Grid2d &g, const double value) const;
  void _to_grid1(Grid2d &g, const double value) const;
  double _gaseous_attenuation(const double km_per_gate) const;
  bool _filter_dbz_diff(const Grid2d &dbz_diff, const Params &P);
  bool _filter_dbz_diff_1(const Grid2d &dbz_diff, const Params &P);
  bool _is_debug(const Params::mask_t &mask, const double range,
		 const double az, const double vlevel) const;
};

#endif
 
