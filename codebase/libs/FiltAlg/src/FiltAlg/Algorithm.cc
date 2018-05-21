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
 * @file Algorithm.cc
 */

#include <FiltAlg/Algorithm.hh>
#include <FiltAlg/FiltCreate.hh>
#include <FiltAlg/Statics.hh>
#include <FiltAlg/Filter.hh>
#include <FiltAlg/FiltInfo.hh>

#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
#include <toolsa/LogStream.hh>
#include <euclid/LineList.hh>
#include <toolsa/TaThreadSimple.hh>
#include <toolsa/pmu.h>
#include <unistd.h>
#include <algorithm>

//------------------------------------------------------------------
static GridAlgs _createGrid(const char *name, const Mdvx::field_header_t &hdr)
{
  // create a 2d grid, make bad and missing the same value
  GridAlgs g2d(name, hdr.nx, hdr.ny, static_cast<double>(hdr.bad_data_value));
  return g2d;
}

//------------------------------------------------------------------
static GridAlgs _createGrid(const char *name, const Mdvx::field_header_t &hdr,
			    fl32 *data)
{
  // create a 2d grid, make bad and missing the same value
  GridAlgs g2d(name, hdr.nx, hdr.ny, static_cast<double>(hdr.bad_data_value));

  for (int j=0; j<hdr.nx*hdr.ny; ++j)
  {
    fl32 v = data[j];
    if (v  == hdr.missing_data_value)
    {
      v = hdr.bad_data_value;
    }
    g2d.setValue(j, static_cast<double>(v));
  }
  return g2d;
}

//------------------------------------------------------------------
TaThread *Algorithm::AlgThreads::clone(const int index)
{
  // it is a simple thread that uses the Algorithm::compute() as method
  TaThreadSimple *t = new TaThreadSimple(index);
  t->setThreadMethod(Algorithm::compute);
  t->setThreadContext(this);
  return (TaThread *)t;
}

//------------------------------------------------------------------
Algorithm::Algorithm(const FiltAlgParms &p, const FiltCreate *create)
{
  _ok = true;
  _last_time = -1;
  _info = NULL;
  _input_info = NULL;
  for (int i=0; i<p.filter_n; ++i)
  {
    if (!_create_filter(create, p, p._filter[i]))
    {
      _ok = false;
    }
  }

  _thread.init(p.num_threads, p.thread_debug);
}

//------------------------------------------------------------------
Algorithm::~Algorithm(void)
{
  for (int i=0; i<static_cast<int>(_filters.size()); ++i)
  {
    delete _filters[i];
  }
  _filters.clear();
  if (_info != NULL)
  {
    delete _info;
  }
  if (_input_info != NULL)
  {
    delete _input_info;
  }
}

//------------------------------------------------------------------
bool Algorithm::update(const time_t &t, const FiltAlgParms &p)
{
  DsMdvx dout;

  bool vlevelChange;
  if (!_update_init(t, p, dout, vlevelChange))
  {
    return false;
  }

  bool stat = true;

  // start from front and do filters.
  for (int i=0; i<static_cast<int>(_filters.size()); ++i)
  {
    if (!_filter(t, vlevelChange, p.vlevel_tolerance, _filters[i], dout))
    {
      stat = false;
    }
  }

  if (stat)
  {
    stat = _do_output(p, dout);
  }
  _last_time = t;
  return stat;
}

//------------------------------------------------------------------
void Algorithm::compute(void *ti)
{
  FiltInfo *algInfo = static_cast<FiltInfo *>(ti);

  algInfo->doFilter(false);

  // IN THIS CASE WE DO NOT DELETE THE INFO
  // delete algInfo;
}

//------------------------------------------------------------------
bool Algorithm::write_lines(const std::string &spdbUrl, const LineList &l, 
			    const GridProj &gp)
{
  return true;
}

//------------------------------------------------------------------
bool Algorithm::_create_filter(const FiltCreate *create,
			       const FiltAlgParms &ap,
			       const FiltAlgParams::data_filter_t &p)
{
  Filter *f = create->create(p, ap);
  if (f == NULL)
  {
    return false;
  }
  if (!f->ok())
  {
    delete f;
    return false;
  }

  // Set info if info is used for this filter
  f->set_info(&_info);
  f->set_input_info(&_input_info);

  _filters.push_back(f);
  return true;
}

//------------------------------------------------------------------
bool Algorithm::_update_init(const time_t &t, const FiltAlgParms &p,
			     DsMdvx &dout, bool &vlevelChange)
{
  Mdvx::master_header_t mhdr;

  // read all the inputs
  if (!_read(t, p, mhdr))
  {
    return false;
  }

  // set output master header using input
  dout.setMasterHeader(mhdr);

  // initialize 'info. using input'
  if (_info != NULL)
  {
    if (mhdr.sensor_alt != 0 && mhdr.sensor_lat != 0 && mhdr.sensor_lon != 0)
    {
      _info->init(t, mhdr.sensor_lat, mhdr.sensor_lon, mhdr.sensor_alt,
		  mhdr.data_set_name, mhdr.data_set_source);
    }
    else
    {
      _info->init(t, _hdr.proj_origin_lat, _hdr.proj_origin_lon,
		  _hdr.grid_minz, mhdr.data_set_name, mhdr.data_set_source);
    }
  }

  // prepare to produce output
  _output.clear();

  if (_vlevel  == _last_vlevel)
  {
    vlevelChange = false;
  }
  else
  {
    vlevelChange = true;
    _last_vlevel = _vlevel;
    LOG(WARNING) << "Scanning vertical levels have changed, or first scan";
  }
    
  return true;
}

//------------------------------------------------------------------
bool Algorithm::_do_output(const FiltAlgParms &p, DsMdvx &dout)
{
  if (_info != NULL)
  {
    // write out the info to both mdv and spdb
    _info->info_print(LogStream::DEBUG_VERBOSE);
    _info->store(dout);
    _info->output(p.output_spdb);
  }
  // write the mdv output, which should now have all output data in it.
  if (dout.writeToDir(p.output_url))
  {
    LOG(ERROR) << "Unable to write MDV";
    return false;
  }
  return true;
}

//------------------------------------------------------------------
bool Algorithm::_read(const time_t &t, const FiltAlgParms &p, 
		      Mdvx::master_header_t &mhdr)
{
  LOG(DEBUG) << "Reading input data";

  // clear any previous input data
  _input.clear();

  int wait_seconds = 0;

  vector<string> done_urls;
  vector<string> done_feedback_urls;

  // its assumed all inputs are MDV
  // for each input (param)
  for (int i=0; i<p.input_n; ++i)
  {
    // has this url already been read in? (there can be more than one input
    // with the same url (different fields from one url)
    string url = p._input[i].url;
    if (find(done_urls.begin(), done_urls.end(), url) == done_urls.end())
    {
      // it has not been read in, we read it in now.
      done_urls.push_back(url);
      if (!_read_1(t, p, i, url, mhdr, wait_seconds))
      {
	return false;
      }
    }
  }

  // if get here assume things have been read in, so the field header _hdr
  // is set, as is the vertical level header _vhdr
  for (int i=0; i<p.feedback_input_n; ++i)
  {
    // has this url already been read in? (there can be more than one input
    // with the same url (different fields from one url)
    string url = p._feedback_input[i].url;
    if (find(done_feedback_urls.begin(), done_feedback_urls.end(), url)
	== done_feedback_urls.end())
    {
      // it has not been read in, we read it in now.
      done_feedback_urls.push_back(url);
      if (!_feedback_read_1(t, p, i, url))
      {
	return false;
      }
    }
  }
  return true;
}

//------------------------------------------------------------------
bool Algorithm::_read_1(const time_t &t, const FiltAlgParms &p, const int i0, 
			const string &url, Mdvx::master_header_t &mhdr,
			int &wait_seconds)
{
  DsMdvx d;

  // set to read this url.
  // d.setReadTime(Mdvx::READ_FIRST_BEFORE, url, 0, t);
  d.setReadTime(Mdvx::READ_CLOSEST, url, p.read_tolerance_seconds, t);
  d.clearReadFields();

  // add in every input field that is configured to be in this url.
  // start at i0, the first index for this url
  for (int i=i0; i<p.input_n; ++i)
  {
    string urli = p._input[i].url;
    if (urli == url)
    {
      d.addReadField(p._input[i].field);
    }
  }

  // restrict vertical levels if doing so.
  if (p._vlevel[0] >= 0 && p._vlevel[1] >= 0)
  {
    d.setReadVlevelLimits(p._vlevel[0], p._vlevel[1]);
  }

  while (true)
  {
    // read now.
    if (d.readVolume())
    {
      if (wait_seconds < static_cast<int>(p.max_wait_minutes*60.0))
      {
	sleep(5);
	wait_seconds += 5;
      }
      else
      {
        LOG(ERROR) << "MDV READ ERROR - check field list";
        d.printReadRequest(cerr);
	LOG(ERROR) << "cant read volume url=" << url;
	return false;
      }
    }
    else
    {
      break;
    }
  }

  bool stat = true;
  // find all the inputs for this url
  for (int i=i0; i<p.input_n; ++i)
  {
    string urli = p._input[i].url;
    if (urli == url)
    {
      // add each input that is from this url to the internal state
      if (!_add_input(p._input[i].field, p._input[i].name, p, d))
      {
	stat = false;
      }
    }
  }

  // grab the master header
  mhdr = d.getMasterHeader();

  // set the projection object
  _proj = MdvxProj(mhdr, _hdr);

  Statics::_gproj = MdvxProj(mhdr, _hdr);

  // if this is a url from which info is input, get that from the chunk
  if (url == p.input_info_url && _input_info != NULL)
  {
    if (!_input_info->retrieve(d))
    {
      stat = false;
    }
  }
  return stat;
}

//------------------------------------------------------------------
bool Algorithm::_feedback_read_1(const time_t &t,
				 const FiltAlgParms &p, const int i0, 
				 const string &url)
{
  bool fake = false;
  DsMdvx d;

  if (_last_time == -1 && p.feedback_immediately)
  {
    d.setTimeListModeValid(url, t-p.max_feedback_seconds_back, t-1);
    d.compileTimeList();
    vector<time_t> dt = d.getTimeList();
    if (!dt.empty())
    {
      _last_time = *dt.rbegin();
      LOG(DEBUG) << "Setting last time for URL to "
		 << DateTime::strn(_last_time) 
		 << "  -   " << url;
    }
  }
  if (_last_time == -1)
  {
    fake = true;
  }
  else
  {
    // set to read this url.
    d.setReadTime(Mdvx::READ_CLOSEST, url, 0, _last_time);
    d.clearReadFields();

    // add in every input field that is configured to be in this url.
    // start at i0, the first index for this url
    for (int i=i0; i<p.feedback_input_n; ++i)
    {
      string urli = p._feedback_input[i].url;
      if (urli == url)
      {
	d.addReadField(p._feedback_input[i].field);
      }
    }

    // restrict vertical levels if doing so.
    if (p._vlevel[0] >= 0 && p._vlevel[1] >= 0)
    {
      d.setReadVlevelLimits(p._vlevel[0], p._vlevel[1]);
    }

    // read now.
    if (d.readVolume())
    {
      LOG(ERROR) << "Feedback MDV READ ERROR - missing field(s)";
      fake = true;
    }
  }

  bool stat = true;
  // find all the inputs for this url
  for (int i=i0; i<p.feedback_input_n; ++i)
  {
    string urli = p._feedback_input[i].url;
    if (urli == url)
    {
      // add each input that is from this url to the internal state
      if (!_add_feedback_input(p._feedback_input[i].field,
			       p._feedback_input[i].name,
			       p, d, fake))
      {
	stat = false;
      }
    }
  }

  // if this is a url from which info is input, get that from the chunk
  if (url == p.input_info_url && _input_info != NULL)
  {
    if (fake)
    {
      LOG(WARNING) << "Information from feedback URL not implemented";
      stat = false;
    }
    else
    {
      if (!_input_info->retrieve(d))
      {
	stat = false;
      }
    }
  }
  return stat;
}

//------------------------------------------------------------------
// inputs assumed MDV 3d grids
bool Algorithm::_add_input(const char *input_field, const char *name,
			   const FiltAlgParms &p, DsMdvx &d)
{
  LOG(DEBUG_VERBOSE) << "Reading FIELD " << name << "=" << input_field;
  MdvxField *f;
  f = d.getFieldByName(input_field);
  if (f == NULL)
  {
    LOG(ERROR) << "cant read field " << input_field;
    return false;
  }
  string s = name;

  // initialize a 3d grid object
  Data g(s, Data::GRID3D, false);

  // make sure the data is good to go
  f->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);

  // fill in headers state using this data, (which can be overwritten later)
  // therefore, next 2 lines assumes all inputs have same grid(nx,ny,proj)!
  _hdr = f->getFieldHeader();
  _vhdr = f->getVlevelHeader();

  // get the data
  fl32 *data = (fl32 *)f->getVol();

  // for each vertical level
  int num = _hdr.nz;

  // store the vertical levels per field, NOTE that not checking for
  // consistency between fields, which could be problematic
  _vlevel.clear();  
  for (int i=0; i<num; ++i)
  {
    _vlevel.push_back(_vhdr.level[i]);
    LOG(DEBUG_VERBOSE) << "..Creating elev " << _vhdr.level[i];

    GridAlgs g2d = _createGrid(name, _hdr, &data[i*_hdr.nx*_hdr.ny]);
    if (p.min_gate_index >= 0)
    {
      g2d.adjust(p.min_gate_index, -1);
    }

    // add that 2d grid to the 3d grid object
    GridProj gp(_hdr);
    g.add(g2d, _vhdr.level[i], i, gp);
  }

  // store to state
  _input.push_back(g);
  return true;
}

//------------------------------------------------------------------
// inputs assumed MDV 3d grids
bool Algorithm::_add_feedback_input(const char *input_field, const char *name,
				    const FiltAlgParms &p, DsMdvx &d,
				    const bool &fake)
{
  string s = name;
  fl32 *data = NULL;

  // initialize a 3d grid object
  Data g(s, Data::GRID3D, false);
  GridAlgs g2d;
  bool lfake = fake;

  if (!lfake)
  {
    LOG(DEBUG_VERBOSE) << "Reading FIELD " << name << "=" << input_field;
    MdvxField *f;
    f = d.getFieldByName(input_field);
    if (f == NULL)
    {
      LOG(ERROR) << "cant read field " << input_field << ", set all missing";
      lfake = true;
    }
    else
    {
      // make sure the data is good to go
      f->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);

      // get the data
      data = (fl32 *)f->getVol();
    }
  }

  // for each vertical level, NOTE assume all inputs have same dimensions
  // including the feedback ones
  int num = _hdr.nz;
  for (int i=0; i<num; ++i)
  {
    LOG(DEBUG_VERBOSE) << "..Creating elev " << _vhdr.level[i];
    // create a 2d grid
    if (lfake)
    {
      g2d = _createGrid(name, _hdr);
    }
    else 
    {
      g2d = _createGrid(name, _hdr, &data[i*_hdr.nx*_hdr.ny]);
    }

    if (p.min_gate_index >= 0)
    {
      g2d.adjust(p.min_gate_index, -1);
    }

    // add that 2d grid to the 3d grid object
    GridProj gp(_hdr);
    g.add(g2d, _vhdr.level[i], i, gp);
  }

  // store to state
  _input.push_back(g);
  return true;
}

//------------------------------------------------------------------
bool Algorithm::_filter(const time_t &t, bool vlevelChange,
			const double vlevel_tolerance,
			Filter *f, DsMdvx &dout)
{
  PMU_auto_register(f->sprintInputOutput().c_str());

  // let the filter know if vertical levels change
  if (vlevelChange)
  {
    f->vertical_level_change();
  }

  // create a vector of outputs by applying the filter
  vector<Data> new_output;
  if (!_filter_1(t, vlevel_tolerance, f, new_output))
  {
    return false;
  }

  // for each such new output
  for (int j=0; j<static_cast<int>(new_output.size()); ++j)
  {
    // store it into state
    _output.push_back(new_output[j]);

    // add this output to the DsMdv object, if it is to be written
    if (new_output[j].is_output() && new_output[j].is_grid3d())
    {
      _add_field(t, new_output[j], dout);
    }
  }
  return true;
}

//------------------------------------------------------------------
bool Algorithm::_filter_1(const time_t &t, const double vlevel_tolerance,
			  Filter *f, vector<Data> &out)

{
  const Data *gin;
  Data gout;

  // debug printing
  f->printInputOutput();

  // create pointer to existing main input (gin) and new main output (gout)
  gin = f->create_input_output(_input, _output, gout);
  if (gin == NULL)
  {
    return false;
  }

  // create internal filter inputs for this filter
  if (!f->create_inputs(t, _input, _output))
  {
    LOG(ERROR) << "Filter not performed, no output";
    return false;
  }

  bool status = true;
  if (gin->is_data1d())
  {
    status = _filter_1d(f, gin, &gout);
  }
  else
  {
    status = _filter_2d(f, gin, &gout);
  }
  if (!status)
  {
    return false;
  }

  // store gout (results) to returned output, which frees memory for extra
  return f->store_outputs(gout, _info, _filtInfo, out);
}

//------------------------------------------------------------------
bool Algorithm::_filter_1d(const Filter *f, const Data *gin, Data *gout)
{
  _filtInfo.clear();
  _filtInfo.resize(1);
  f->create_extra(_filtInfo[0]);

  FiltInfoInput inputs(gin, gout);
  _filtInfo[0].setInput(inputs);

  f->filter_print();
  if (!f->filter(_filtInfo[0].getInput(), _filtInfo[0].getOutput()))
  {
    return false;
  }
  else
  {
    return _filtInfo[0].store1dValue(*gout);
  }
}

//------------------------------------------------------------------
bool Algorithm::_filter_2d(const Filter *f, const Data *gin, Data *gout)
{
  time_t t0 = time(0);

  // initialize the FiltInfoThreaded to a bunch of empties
  _filtInfo.clear();
  _filtInfo.resize(gin->num_vlevel());
  for (int i=0; i<gin->num_vlevel(); ++i)
  {
    f->create_extra(_filtInfo[i]);
  }

  for (int i=0; i<gin->num_vlevel(); ++i)
  {
    _compute(*gin, i, f, gout);
  }

  _thread.waitForThreads();

  time_t t1 = time(0);
  LOG(DEBUG_VERBOSE) << "------filter elapsed time = " << t1-t0 << " seconds";

  bool status= true;
  for (int i=0; i<gin->num_vlevel(); ++i)
  {
    if (!_filtInfo[i].storeSlice(*gout))
    {
      status = false;
    }
  }
  return status;
}

//------------------------------------------------------------------
bool Algorithm::_add_field(const time_t &t, const Data &G, DsMdvx &dout) const
{
  if (G.dim_bad(_hdr.nx, _hdr.ny, _hdr.nz, true))
  {
    LOG(ERROR) << "mismatch in grids";
    return false;
  }

  // lots of Mdv stuff here.
  const char *fname = G.get_name().c_str(); 
  Mdvx::field_header_t loc_hdr = _hdr;

  // create a 3d data volume
  fl32 *out = G.volume(loc_hdr.bad_data_value, loc_hdr.missing_data_value);
  if (out == NULL)
  {
    LOG(ERROR) << "ERROR creating volume data";
    return false;
  }
  strncpy(loc_hdr.field_name_long, fname, MDV_LONG_FIELD_LEN-1);
  strncpy(loc_hdr.field_name, fname, MDV_SHORT_FIELD_LEN-1);
  MdvxField *fout = new MdvxField(loc_hdr, _vhdr, (void *)0,true,false);
  fl32 *fo = (fl32 *)fout->getVol();
  memcpy(fo, out, _hdr.nx*_hdr.ny*_hdr.nz*sizeof(fl32));
  // fout->convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_GZIP);
  fout->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_GZIP);
  dout.addField(fout);
  delete [] out;
  return true;
}

//------------------------------------------------------------------
void Algorithm::_compute(const Data &gin, const int i, 
			 const Filter *f, const Data *gout)
{
  FiltInfo *info = _setupInfo(gin, i, f, gout);
  _thread.thread(i, info);
}

//------------------------------------------------------------------
FiltInfo *Algorithm::_setupInfo(const Data &gin, const int i,
				const Filter *f, const Data *gout)
{
  FiltInfo *info = &_filtInfo[i];
  const VlevelSlice *gi = gin.ith_vlevel_slice(i);
  double vlevel;
  GridProj gp;
  gi->get_grid_info(vlevel, gp);
  FiltInfoInput inputs(gi, _vlevel, f, gout, i, vlevel, gp);
  info->setInput(inputs);
  return info;
}
