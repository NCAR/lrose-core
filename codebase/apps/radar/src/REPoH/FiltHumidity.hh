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
 * @file FiltHumidity.hh 
 * @brief Humidity filter
 * @class FiltHumidity
 * @brief Humidity filter
 */

#ifndef FiltHumidity_H
#define FiltHumidity_H

#include "KernelGrids.hh"
#include "Kernels.hh"
#include <FiltAlg/Filter.hh>
#include "Params.hh"
#include <string>

class HumidityExtra;

/*----------------------------------------------------------------*/
class FiltHumidity : public Filter
{
public:
  /**
   * Constructor
   */
  FiltHumidity(const FiltAlgParams::data_filter_t f, const FiltAlgParms &P);
  /**
   * Destructor
   */
  virtual ~FiltHumidity();
  #include <FiltAlg/FilterVirtualFunctions.hh>

protected:
private:

  // copy of params
  Params _params;

  // output data objects (main output (Not part of this class) is the
  // humidity data)
  Data _clump; // clumps
  Data _clump_nosmall; // remove small clumps

  Data _pid_clump; // clumps
  Data _pid_clump_nosmall; // remove small clumps

  Data _kernel_cp;     // kernel center points
  Data _fkernel_cp;    // filtered kernel center points

  Data _edge;          // points near edge of clumps
  Data _fedge;         // filtered points near edge of clumps (far enough
                       // from others)

  Data _outside;       // points near edge but outside clumps
  Data _foutside;      // filt. points outside clumps 

  Data _unfiltered_attenuation;   // attenuation grid
  Data _attenuation;   // attenuation grid

  Data _unfiltered_humidity;      // humidity for unfiltered kernels
  // filtered humidity is the main output

  Data _dbz_diff;      // Sdbz - Kdbz

  // data pointers for input mdv fields
  const Data *_snoise;
  const Data *_knoise;
  const Data *_sdbz;
  const Data *_kdbz;
  const Data *_szdr;
  const Data *_srhohv;

  // // the 2d grids for the inputs
  // KernelGrids _grids;

  // current time
  time_t _time;
  
  bool _set_inputs(const FiltInfoInput &inp, HumidityExtra &e) const;



  void _output(const HumidityExtra &e, const Kernels &vk,
	       const std::string &asciiOutStr,
	       const std::string &spdb_url,
	       const std::string &outside_spdb_url,
	       const std::string ascii_path) const;
};

/**
 * Class for extra information needed in humidity filtering
 */
class HumidityExtra
{
public:
  /**
   * Constructor
   */
  inline HumidityExtra() {}

  /**
   * Destructor
   */
  inline ~HumidityExtra() {}

  time_t _time;
  KernelGrids _grids;     // pointers to a lot of stuff
  Grid2d _dbz_diff;      // Sdbz - Kdbz
  Grid2d _clump;
  Grid2d _clump_nosmall;
  Grid2d _pid_clump;
  Grid2d _pid_clump_nosmall;
  Grid2d _edge;
  Grid2d _outside;
  Grid2d _fedge;
  Grid2d _foutside;

  Grid2d _kernel_cp;
  Grid2d _unfiltered_attenuation;
  Grid2d _unfiltered_humidity;
  Grid2d _fkernel_cp;
  Grid2d _attenuation;

  Kernels _vk;
  Kernels _vkf;
  std::string _ascii_output_string;
  std::string _ascii_output_string_f;

  double _vlevel;
  int _nx, _ny;
  int _vlevel_index;
  GridProj _gp;
};

#endif

