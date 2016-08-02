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
 * @file FiltHumidity.cc
 */
#include "FiltHumidity.hh"
#include "AsciiOutput.hh"
#include "HumidityAlg.hh"
#include "CloudGaps.hh"
#include "CloudGap.hh"
#include "ClumpAssociate.hh"
#include <toolsa/LogStream.hh>
#include <vector>

using std::string;
using std::vector;

/*----------------------------------------------------------------*/
static bool _create_input1(const string &name, const vector<Data> &input,
			   const vector<Data> &output, const Data **ret)
{
  *ret = Filter::set_data(true, name.c_str(), input, output);
  if (*ret == NULL)
  {
    LOG(ERROR) << name << " never found";
    return false;
  }
  else
    return true;
}

/*----------------------------------------------------------------*/
FiltHumidity::FiltHumidity(const FiltAlgParams::data_filter_t f,
			   const FiltAlgParms &P) : Filter(f, P)
{
  if (!ok())
    return;

  const void *vap = P.app_parms();
  const Params *ap = (const Params *)vap;
  _params = *ap;

  // set the output mdv data objects
  _clump = Data("clump", Data::GRID3D, true);
  _clump_nosmall = Data("clump_nosmall", Data::GRID3D, true);
  _pid_clump = Data("pid_clump", Data::GRID3D, true);
  _pid_clump_nosmall = Data("pid_clump_nosmall", Data::GRID3D, true);
  _edge = Data("edge", Data::GRID3D, true);
  _outside = Data("outside", Data::GRID3D, true);
  _fedge = Data("fedge", Data::GRID3D, true);
  _foutside = Data("foutside", Data::GRID3D, true);
  _kernel_cp = Data("kernel_cp", Data::GRID3D, true);
  _fkernel_cp = Data("fkernel_cp", Data::GRID3D, true);
  _attenuation = Data("attenuation", Data::GRID3D, true);
  _unfiltered_attenuation = Data("unfiltered_attenuation", Data::GRID3D, true);
  _unfiltered_humidity = Data("unfiltered_humidity", Data::GRID3D, true);
  _dbz_diff = Data("dbz_diff", Data::GRID3D, true);
}

/*----------------------------------------------------------------*/
FiltHumidity::~FiltHumidity()
{
}

/*----------------------------------------------------------------*/
bool FiltHumidity::create_inputs(const time_t &t, 
				 const vector<Data> &input,
				 const vector<Data> &output)
{
  // set time
  _time = t;

  // set all the input pointers
  bool stat = true;
  if (!_create_input1(_params.s_noise_name, input, output, &_snoise))
    stat = false;
  if (!_create_input1(_params.k_noise_name, input, output, &_knoise))
    stat = false;
  if (!_create_input1(_params.s_dbz_name, input, output, &_sdbz))
    stat = false;
  if (!_create_input1(_params.k_dbz_name, input, output, &_kdbz))
    stat = false;
  if (!_create_input1(_params.s_zdr_name, input, output, &_szdr))
    stat = false;
  if (!_create_input1(_params.s_rhohv_name, input, output, &_srhohv))
    stat = false;
  if (!stat)
    return false;

  // clear out the outputs
  _clump.clear();
  _clump_nosmall.clear();
  _pid_clump.clear();
  _pid_clump_nosmall.clear();
  _kernel_cp.clear();
  _fkernel_cp.clear();
  _edge.clear();
  _outside.clear();
  _fedge.clear();
  _foutside.clear();
  _attenuation.clear();
  _unfiltered_attenuation.clear();
  _unfiltered_humidity.clear();
  _dbz_diff.clear();

  // clear out ascii output in case this is a re-run
  AsciiOutput A(_params.unfiltered_kernel_ascii_path, _time);
  A.clear();

  AsciiOutput A2(_params.filtered_kernel_ascii_path, _time);
  A2.clear();
  
  return true;
}

//------------------------------------------------------------------
void FiltHumidity::create_extra(FiltInfo &info) const
{
  HumidityExtra *e = new HumidityExtra();
  info.getOutput().storeExtra((void *)e);
}

/*----------------------------------------------------------------*/
void FiltHumidity::initialize_output(const Data &inp,
				     const FiltAlgParams::data_filter_t f,
				     Data &g)
{
  g = Data(f.output_field, Data::GRID3D, f.write_output_field);
}

/*----------------------------------------------------------------*/
void FiltHumidity::filter_print(void) const
{
  LOG(DEBUG) << "Filtering";
}

/*----------------------------------------------------------------*/
void FiltHumidity::filter_print(const double vlevel) const
{
  LOG(DEBUG) << "Filtering vlevel=" << vlevel;
}

/*----------------------------------------------------------------*/
bool FiltHumidity::filter(const FiltInfoInput &inp, FiltInfoOutput &o) const
{
  HumidityExtra *e = (HumidityExtra *)o.getExtra();

  // create pointers to all needed inputs
  if (!_set_inputs(inp, *e))
  {
    return false;
  }

  // create the algorithm objecct
  HumidityAlg H(inp);

  // shift the kdbz values due to a bias and point to that instead.
  e->_grids._kdbzAdjusted = H.add_bias(*e->_grids._kdbzUnadjusted,
				       _params.k_dbz_offset);

  // _grids._kdbz = &g;

  // make a dbz diff grid  and store
  H.diff_grid(*e->_grids._sdbz, e->_grids._kdbzAdjusted, e->_dbz_diff);

  e->_grids._dbz_diff = &e->_dbz_diff;

  // do clumping related filtering:
  Grid2d clumps = H.clumping(*e->_grids._pid, *e->_grids._snoise, 
			      *e->_grids._knoise,
			      _params, e->_clump, e->_clump_nosmall);
  
  // do PID clumping related filtering:
  Grid2d pid_clumps = H.clumping(*e->_grids._pid, _params, e->_pid_clump,
				  e->_pid_clump_nosmall);
  
  // associate the PID clumping with the other clumping (map each clump
  // id value to one or more PID clump values
  ClumpAssociate ca = H.associate_clumps(clumps, pid_clumps);

  // Build the gaps between clumps
  CloudGaps gaps = H.build_gaps(clumps, pid_clumps, ca, _params, e->_edge,
				e->_outside, e->_fedge, e->_foutside);

  // do kernel building and output results
  e->_vk = H.kernel_build(_time, clumps, gaps, e->_grids, _params,
			  e->_kernel_cp);

  H.kernel_output(e->_vk, e->_unfiltered_attenuation, e->_unfiltered_humidity,
		  e->_ascii_output_string);

  // filter kernels  down to the actual ones and output those results
  // last one
  e->_vkf = H.kernel_filter(e->_vk, e->_fkernel_cp);

  Grid2d gout;
  H.kernel_output(e->_vkf, e->_attenuation, gout, e->_ascii_output_string_f);
  o = FiltInfoOutput(gout, (void *)e);
  return true;
}

/*----------------------------------------------------------------*/
bool FiltHumidity::store_outputs(const Data &o, Info *info,
				 vector<FiltInfo> &extra,
				 vector<Data> &output)
{
  // clear out the outputs
  _clump.clear();
  _clump_nosmall.clear();
  _pid_clump.clear();
  _pid_clump_nosmall.clear();
  _kernel_cp.clear();
  _fkernel_cp.clear();
  _edge.clear();
  _outside.clear();
  _fedge.clear();
  _foutside.clear();
  _attenuation.clear();
  _unfiltered_attenuation.clear();
  _unfiltered_humidity.clear();
  _dbz_diff.clear();
  for (size_t i = 0; i<extra.size(); ++i)
  {
    HumidityExtra *e = (HumidityExtra *)extra[i].getOutput().getExtra();
    _dbz_diff.add(e->_dbz_diff, e->_vlevel, e->_vlevel_index, e->_gp);
    _clump.add(e->_clump, e->_vlevel, e->_vlevel_index, e->_gp);
    _clump_nosmall.add(e->_clump_nosmall, e->_vlevel, e->_vlevel_index, e->_gp);
    _pid_clump.add(e->_pid_clump, e->_vlevel, e->_vlevel_index, e->_gp);
    _pid_clump_nosmall.add(e->_pid_clump_nosmall, e->_vlevel,
			   e->_vlevel_index, e->_gp);
    _edge.add(e->_edge, e->_vlevel, e->_vlevel_index, e->_gp);
    _outside.add(e->_outside, e->_vlevel, e->_vlevel_index, e->_gp);
    _fedge.add(e->_fedge, e->_vlevel, e->_vlevel_index, e->_gp);
    _foutside.add(e->_foutside, e->_vlevel, e->_vlevel_index, e->_gp);
    _kernel_cp.add(e->_kernel_cp, e->_vlevel, e->_vlevel_index, e->_gp);
    _unfiltered_attenuation.add(e->_unfiltered_attenuation, e->_vlevel,
				e->_vlevel_index, e->_gp);
    _unfiltered_humidity.add(e->_unfiltered_humidity, e->_vlevel,
			     e->_vlevel_index, e->_gp);
    _fkernel_cp.add(e->_fkernel_cp, e->_vlevel, e->_vlevel_index, e->_gp);
    _attenuation.add(e->_attenuation, e->_vlevel, e->_vlevel_index, e->_gp);

    // append the ascii strings to the input path
    _output(*e, e->_vk, e->_ascii_output_string, _params.kernel_spdb_url,
	    _params.kernel_outside_spdb_url,
	    _params.unfiltered_kernel_ascii_path);
    _output(*e, e->_vkf, e->_ascii_output_string_f,
	    _params.filtered_kernel_spdb_url,
	    _params.filtered_kernel_outside_spdb_url,
	    _params.filtered_kernel_ascii_path);
    delete e;
  }
  
  // always output these 3 things:
  output.push_back(o);
  output.push_back(_attenuation);
  output.push_back(_dbz_diff);

  // output more depending on debugging
  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG))
  {
    output.push_back(_clump_nosmall);
    output.push_back(_pid_clump_nosmall);
    output.push_back(_fedge);
    output.push_back(_foutside);
    output.push_back(_fkernel_cp);
  }
  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE))
  {
    output.push_back(_clump);
    output.push_back(_pid_clump);
    output.push_back(_edge);
    output.push_back(_outside);
    output.push_back(_kernel_cp);
    output.push_back(_unfiltered_attenuation);
    output.push_back(_unfiltered_humidity);
  }
  return true;
}

/*----------------------------------------------------------------*/
void FiltHumidity::set_info(Info **info) const
{
}

/*----------------------------------------------------------------*/
void FiltHumidity::set_input_info(Info **info) const
{
}

/*----------------------------------------------------------------*/
bool FiltHumidity::_set_inputs(const FiltInfoInput &inp,
			       HumidityExtra &e) const
{
  if (!inp.hasVlevels())
  {
    LOG(ERROR) << "input not vlevels";
    return false;
  }
  const VlevelSlice *vs = inp.getSlice();
  if (!vs->is_grid())
  {
    LOG(ERROR) << "input not gridded";
    return false;
  }    
  
  e._time = _time;
  e._nx = vs->getNx();
  e._ny = vs->getNy();

  double vlevel = inp.getVlevel();

  // point to all the input slices
  const Grid2d *pid, *snoise, *knoise, *sdbz, *kdbz, *szdr, *srhohv;
  pid = vs;
  snoise = _snoise->matching_vlevel(vlevel, _vlevel_tolerance);
  knoise = _knoise->matching_vlevel(vlevel, _vlevel_tolerance);
  sdbz = _sdbz->matching_vlevel(vlevel, _vlevel_tolerance);
  kdbz = _kdbz->matching_vlevel(vlevel, _vlevel_tolerance);
  szdr = _szdr->matching_vlevel(vlevel, _vlevel_tolerance);
  srhohv = _srhohv->matching_vlevel(vlevel, _vlevel_tolerance);
  if (snoise == NULL || knoise == NULL || sdbz == NULL || kdbz == NULL || 
      szdr == NULL || srhohv == NULL)
  {
    LOG(ERROR) << "cannot proceed in filter for humidity. Missing inputs";
    return false;
  }
  else
  {
    e._grids = KernelGrids(&sdbz, &kdbz, &szdr, &pid, &snoise, &knoise,
			   &srhohv);
    e._vlevel = inp.getVlevel();
    e._vlevel_index = inp.getVlevelIndex();
    e._gp = inp.getGridProj();
    return true;
  }
}

void FiltHumidity::_output(const HumidityExtra &e, const Kernels &vk,
			   const std::string &asciiOutStr,
			   const string &url,
			   const string &outside_url,
			   const string ascii_path) const
{
  // append the ascii strings to the input path
  AsciiOutput A(ascii_path, e._time);
  A.append_nocr(asciiOutStr);

  if (url.size() != 0)
  {
    // write out the genpoly representation of the kernels
    char name[1000];
    sprintf(name, "%s/vlevel_%d", url.c_str(), e._vlevel_index);
    vk.write_genpoly(name, e._time, e._nx, e._ny, false);
  }

  if (outside_url.size() != 0)
  {
    // write out the genpoly representation of 'just outside' the kernels
    char name[1000];
    sprintf(name, "%s/vlevel_%d", outside_url.c_str(), e._vlevel_index);
    vk.write_genpoly(name, e._time, e._nx, e._ny, true);
  }
}

