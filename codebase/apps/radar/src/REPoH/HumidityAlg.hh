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
 * @file HumidityAlg.hh 
 * @brief the algorihms for humidity estimation
 * @class HumidityAlg
 * @brief the algorihms for humidity estimation
 */

#ifndef HumidityAlg_H
#define HumidityAlg_H

#include <FiltAlg/GridProj.hh>
#include <euclid/Grid2d.hh>
#include <euclid/Grid2dClump.hh> // needed for region_t definition below

#include <vector>

class FiltInfoInput;
class Params;
class ClumpAssociate;
class CloudGaps;
class CloudGap;
class Kernels;
class KernelGrids;

/*----------------------------------------------------------------*/
class HumidityAlg
{
public:
  HumidityAlg(const FiltInfoInput &inp);
  ~HumidityAlg();

  /**
   * add bias to g, return biased grid
   */
  Grid2d add_bias(const Grid2d &g, const double bias) const;

  /**
   * take g1 - g2 and add results to d
   */
  void diff_grid(const Grid2d &g1, const Grid2d &g2, Grid2d &d) const;

  /**
   * Clumping of weather 'clouds', filtering out small clumps
   * @return the clumped data each clump has its own color
   * Results put into clump and clump_nosmall objects
   */
  Grid2d clumping(const Grid2d &pid, const Grid2d &snoise,
		  const Grid2d &knoise, const Params &params, 
		  Grid2d &clump, Grid2d &clump_nosmall);

  /**
   * Clumping of weather 'clouds', based on PID, filtering out small clumps
   * @return the clumped data each clump has its own color
   * Results put into clump and clump_nosmall objects
   */
   Grid2d clumping(const Grid2d &pid, const Params &params, Grid2d &clump,
		   Grid2d &clump_nosmall);

  /**
   * Associate each clump with a 'pid_clump' value or values.
   */
  ClumpAssociate associate_clumps(const Grid2d &clumps,
				  const Grid2d &pid_clumps);

  /**
   * Construction of CloudGaps object (gaps between clouds).
   * Results put into edge, outside, fedge, foutside objects.
   */
  CloudGaps build_gaps(const Grid2d &clumps, const Grid2d &pid_clumps,
		       ClumpAssociate &ca, const Params &params,
		       Grid2d &edge, Grid2d &outside, Grid2d &fedge,
		       Grid2d &foutside) const;

  /**
   * Build actual kenels from inputs and local state
   * Results put info into kernel_cp object
   */
  Kernels kernel_build(const time_t &time, const Grid2d &clumps,
		       const CloudGaps &gaps, const KernelGrids &grids,
		       const Params &params, Grid2d &kernel_cp) const;

  /**
   * Output the kernel information to both SPDB genpoly and ascii one line
   * summaries
   * Put data for attenuation and humidity to those Data objects
   */
  void kernel_output(const Kernels &vk, Grid2d &attenuation,
		     Grid2d &humidity, std::string &ascii_output_string);

  /**
   * Filter kernels to 'good' ones only. returns the smaller set.
   * Puts results to fkernel_cp object
   */
  Kernels kernel_filter(const Kernels &vk, Grid2d &fkernel_cp) const;

  /**
   * Clump index 0,1,2,...   maps to colors 1,2,3,...
   */
  inline static double index_to_color(const int index)
  {
    return (double)(index+1);
  }

  /**
   * colors 1,2,3,... map to clump index 0,1,2,...
   */
  inline static int color_to_index(const double color)
  {
    return (int)(color-1.0);
  }

protected:
private:

  /**
   *  grid dim.
   */
  int _nx, _ny;

  /**
   * elevation value
   */
  double _vlevel;

  /**
   * Elevation index value
   */
  int _vlevel_index;

  /**
   * Grid information
   */
  GridProj _gp;

  /**
   * a vector of clumps, each a vector of points (see Clump.hh)
   */
  std::vector<clump::Region_t> _r;

  /**
   * same thing for pid clumps
   */
  vector<clump::Region_t> _pid_r;

  bool _is_weather(const int x,  const int y, const Grid2d &pid,
		   const Grid2d &snoise, const Grid2d &knoise,
		   const Params &params) const;
  bool _is_pid_clump(const int x,  const int y, const Grid2d &pid,
		     const Params &params) const;
  void _filter_clump(Grid2d &g);
  void _filter_pid_clump(Grid2d &g);
  void _filter_small_clumps(Grid2d &c, const int minpt) const;
  void _filter_small_pid_clumps(Grid2d &c, const int minpt) const;
  Grid2d _build_outside_mask(const Grid2d &clump) const;
  bool _kernel_mask(const CloudGap &gap, const Grid2d &clump,
		    const bool is_far, Grid2d &mask) const;
};

#endif
 
