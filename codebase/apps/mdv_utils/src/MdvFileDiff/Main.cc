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
#include "FieldDataDiff.hh"
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <cstdio>
#include <algorithm>
#include <vector>
#include <string>

//------------------------------------------------------------------
static void _int_compare(const std::string &name, int i0, int i1,
			 bool &diff, std::string &s)
{
  if (i0 != i1)
  {
    diff = true;
    char buf[1000];
    sprintf(buf, "%d %d\n", i0, i1);
    s += "  " + name + ":";
    s += buf;
  }
}

//------------------------------------------------------------------
static void _double_compare(const std::string &name, double i0, double i1,
			    bool &diff, std::string &s)
{
  if (i0 != i1)
  {
    diff = true;
    char buf[1000];
    sprintf(buf, "%lf %lf\n", i0, i1);
    s += "  " + name + ":";
    s += buf;
  }
}

//------------------------------------------------------------------
static void _string_compare(const std::string &name, const char *i0,
			    const char *i1, const int maxlen,
			    bool &diff, std::string &s)
{
  string s0 = i0;
  string s1 = i1;
  if (static_cast<int>(s0.length()) < maxlen && 
      static_cast<int>(s1.length()) < maxlen)
  {
    if (s0 != s1)
    {
      diff = true;
      s += "  " + name + ":";
      s += "'" + s0 + "'";
      s += ",";
      s += "'" + s1 + "'";
      s += "\n";
    }
  }
  else
  {
    if (strncmp(i0, i1, maxlen) != 0)
    {
      diff = true;
      char *c0 = new char [maxlen+1];
      char *c1 = new char [maxlen+1];
      for (int i=0; i<maxlen; ++i)
      {
	c0[i] = i0[i];
	c1[i] = i1[i];
      }
      c0[maxlen] = 0;
      c1[maxlen] = 0;
      s += "  " + name + ":";
      s += "'";
      s += c0;
      s += "',";
      s += "'";
      s += c1;
      s += "'\n";

      delete [] c0;
      delete [] c1;
    }
  }
}

//------------------------------------------------------------------
static void _compare_mhdr(const Mdvx::master_header_t &m0,
			  const Mdvx::master_header_t &m1)
			  // const bool allfields)
{
  bool is_diff = false;
  std::string s = "";
  _int_compare("time_gen", m0.time_gen, m1.time_gen, is_diff, s);
  _int_compare("time_begin", m0.time_begin, m1.time_begin, is_diff, s);
  _int_compare("time_end", m0.time_end, m1.time_end, is_diff, s);
  _int_compare("time_centroid", m0.time_centroid, m1.time_centroid, is_diff, s);
  _int_compare("time_expire", m0.time_expire, m1.time_expire, is_diff, s);
  _int_compare("num_data_times", m0.num_data_times, m1.num_data_times, is_diff,
	       s);
  _int_compare("data_dimension", m0.data_dimension, m1.data_dimension, is_diff,
	       s);
  _int_compare("data_collection_type", m0.data_collection_type,
	       m1.data_collection_type, is_diff, s);
  _int_compare("native_vlevel_type", m0.native_vlevel_type,
	       m1.native_vlevel_type, is_diff, s);
  _int_compare("vlevel_type", m0.vlevel_type, m1.vlevel_type, is_diff, s);
  _int_compare("vlevel_included", m0.vlevel_included, m1.vlevel_included,
	       is_diff, s);
  _int_compare("grid_orientation", m0.grid_orientation, m1.grid_orientation,
	       is_diff, s);
  _int_compare("data_ordering", m0.data_ordering, m1.data_ordering,
	       is_diff, s);
  // if (allfields)
  // {
    _int_compare("n_fields", m0.n_fields, m1.n_fields, is_diff, s);
  // }
  _int_compare("max_nx", m0.max_nx, m1.max_nx, is_diff, s);
  _int_compare("max_ny", m0.max_ny, m1.max_ny, is_diff, s);
  _int_compare("max_nz", m0.max_nz, m1.max_nz, is_diff, s);
  _int_compare("n_chunks", m0.n_chunks, m1.n_chunks, is_diff, s);
  _int_compare("forecast_time", m0.forecast_time, m1.forecast_time, is_diff, s);
  _int_compare("forecast_delta", m0.forecast_delta, m1.forecast_delta,
	       is_diff, s);
  _double_compare("sensor_lon", m0.sensor_lon, m1.sensor_lon, is_diff, s);
  _double_compare("sensor_lat", m0.sensor_lat, m1.sensor_lat, is_diff, s);
  _double_compare("sensor_alt", m0.sensor_alt, m1.sensor_alt, is_diff, s);

  // not yet dealing with data_set_info, data_set_name, data_set_source
  if (is_diff)
  {
    printf("Master header differences:\n");
    printf("%s\n", s.c_str());
    // _diff.masterHdrDiff(s);
  }
}

static void _compare_fhdr(const std::string &name,
			  const Mdvx::field_header_t &f0,
			  const Mdvx::field_header_t &f1)
{
  bool is_diff = false;
  std::string s = "";
  _int_compare("forecast_delta", f0.forecast_delta, f1.forecast_delta, is_diff,
	       s);
  _int_compare("forecast_time", f0.forecast_time, f1.forecast_time, is_diff, s);
  _int_compare("nx", f0.nx, f1.nx, is_diff, s);
  _int_compare("ny", f0.ny, f1.ny, is_diff, s);
  _int_compare("nz", f0.nz, f1.nz, is_diff, s);
  _int_compare("proj_type", f0.proj_type, f1.proj_type, is_diff, s);
  _int_compare("encoding_type", f0.encoding_type, f1.encoding_type, is_diff, s);
  _int_compare("data_element_nbytes", f0.data_element_nbytes,
	       f1.data_element_nbytes, is_diff, s);
  _int_compare("compression_type", f0.compression_type, f1.compression_type,
	       is_diff, s);
  _int_compare("transform_type", f0.transform_type, f1.transform_type,
	       is_diff, s);
  _int_compare("scaling_type", f0.scaling_type, f1.scaling_type,
	       is_diff, s);
  _int_compare("native_vlevel_type", f0.native_vlevel_type,
	       f1.native_vlevel_type, is_diff, s);
  _int_compare("vlevel_type", f0.vlevel_type, f1.vlevel_type, is_diff, s);
  _int_compare("dz_constant", f0.dz_constant, f1.dz_constant, is_diff, s);
  _int_compare("data_dimension", f0.data_dimension, f1.data_dimension, is_diff,
	       s);
  _int_compare("zoom_clipped", f0.zoom_clipped, f1.zoom_clipped, is_diff, s);
  _int_compare("zoom_no_overlap", f0.zoom_no_overlap, f1.zoom_no_overlap,
	       is_diff, s);
  _double_compare("proj_origin_lat", f0.proj_origin_lat, f1.proj_origin_lat,
		  is_diff, s);
  _double_compare("proj_origin_lon", f0.proj_origin_lon, f1.proj_origin_lon,
		  is_diff, s);
  _double_compare("vert_reference", f0.vert_reference, f1.vert_reference,
		  is_diff, s);
  _double_compare("grid_dx", f0.grid_dx, f1.grid_dx, is_diff, s);
  _double_compare("grid_dy", f0.grid_dy, f1.grid_dy, is_diff, s);
  _double_compare("grid_dz", f0.grid_dz, f1.grid_dz, is_diff, s);
  _double_compare("grid_minx", f0.grid_minx, f1.grid_minx, is_diff, s);
  _double_compare("grid_miny", f0.grid_miny, f1.grid_miny, is_diff, s);
  _double_compare("grid_minz", f0.grid_minz, f1.grid_minz, is_diff, s);

  // not doing scale, bias, bad_data_value, missing_data_value

  _double_compare("proj_rotation", f0.proj_rotation, f1.proj_rotation, is_diff,
		  s);
  
  // not doing min_value, max_value, min_value_orig_vol, max_value_orig_vol

  _string_compare("field_name_long", f0.field_name_long, f1.field_name_long,
		  MDV_LONG_FIELD_LEN, is_diff, s);
  _string_compare("field_name", f0.field_name, f1.field_name,
		  MDV_SHORT_FIELD_LEN, is_diff, s);
  _string_compare("units", f0.units, f1.units, MDV_UNITS_LEN, is_diff, s);
  _string_compare("transform", f0.transform, f1.transform, MDV_TRANSFORM_LEN,
		  is_diff, s);
  if (is_diff)
  {
    printf("Field header differences:%s\n", name.c_str());
    printf("%s\n", s.c_str());
  }
}

static void _compare_data(const std::string &name, fl32 *data0, fl32 *data1, 
			  const Mdvx::field_header_t &f0,
			  const Mdvx::field_header_t &f1)
{
  if (f0.nx != f1.nx || f0.ny != f1.ny || f0.nz != f1.nz)
  {
    printf("Data differences:%s\n", name.c_str());
    printf("  No comparison due to unequal dimensions\n");
    return;
  }
  FieldDataDiff diff(name);
  for (int z=0; z<f1.nz; ++z)
  {
    for (int y=0; y<f1.ny; ++y)
    {
      for (int x=0; x<f1.nx; ++x)
      {
	fl32 d0 = data0[z*f1.nx*f1.ny + y*f1.nx + x];
	fl32 d1 = data1[z*f1.nx*f1.ny + y*f1.nx + x];
	bool bad0 = (d0 == f0.missing_data_value || d0 == f0.bad_data_value);
	bool bad1 = (d1 == f1.missing_data_value || d1 == f1.bad_data_value);
	if (bad0 && bad1)
	{
	  diff.dataSame();
	}
	else if (bad0 && !bad1)
	{
	  diff.dataOnlyOne();
	}
	else if ((bad0 && !bad1) || (bad1 && !bad0))
	{
	  diff.dataOnlyOne();
	}
	else
	{
	  double d = fabs(d0-d1);
	  if (d > 0.0)
	  {
	    diff.dataDiff(d, x, y, z);
	  }
	  else
	  {
	    diff.dataSame();
	  }
	}
      }
    }
  }
  if (diff.isDifferent(0.0))
  {
    printf("Data differences:\n");
    diff.print(0.0);
  }
}

int main(int argc, char **argv)
{
  if (argc != 3)
  {
    printf("Usage:  MdvFileDiff   <file1> <file2>\n");
    exit(1);
  }

  DsMdvx d0;
  d0.setReadPath(argv[1]);

  DsMdvx d1;
  d1.setReadPath(argv[2]);


  if (d0.readVolume())
  {
    printf("ERROR reading %s\n", argv[1]);
    exit(1);
  }
  if (d1.readVolume())
  {
    printf("ERROR reading %s\n", argv[2]);
    exit(1);
  }

  const Mdvx::master_header_t m0 = d0.getMasterHeaderFile();
  const Mdvx::master_header_t m1 = d1.getMasterHeaderFile();
  _compare_mhdr(m0, m1);

  vector<string> allFields;
  for (int i=0; i<m0.n_fields; ++i)
  {
    Mdvx::field_header_t f = d0.getFieldHeaderFile(i);
    string s = f.field_name_long;
    if (find(allFields.begin(), allFields.end(), s) == allFields.end())
    {
      allFields.push_back(s);
    }
  }
  for (int i=0; i<m1.n_fields; ++i)
  {
    Mdvx::field_header_t f = d1.getFieldHeaderFile(i);
    string s = f.field_name_long;
    if (find(allFields.begin(), allFields.end(), s) == allFields.end())
    {
      allFields.push_back(s);
    }
  }
  for (size_t i=0; i<allFields.size(); ++i)
  {
    MdvxField *f0 = d0.getFieldByName(allFields[i]);
    MdvxField *f1 = d1.getFieldByName(allFields[i]);
    if (f0 == NULL && f1 == NULL)
    {
      // weird but ok
    }
    else if (f0 == NULL && f1 != NULL)
    {
      printf("Field %s  in %s  but not in %s\n",
	     allFields[i].c_str(), argv[2], argv[1]);
    }
    else if (f0 != NULL && f1 == NULL)
    {
      printf("Field %s  in %s  but not in %s\n",
	     allFields[i].c_str(), argv[1], argv[2]);
    }
    else
    {
      f0->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
      f1->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
      _compare_fhdr(allFields[i], f0->getFieldHeader(), f1->getFieldHeader());
      fl32 *data0 = (fl32 *)f0->getVol();
      fl32 *data1 = (fl32 *)f1->getVol();
      _compare_data(allFields[i], data0, data1, f0->getFieldHeader(),
		    f1->getFieldHeader());
    }
  }
  
  // info.init(_parms.setWantedFields(D0, D1));
  // bool first = true;
  // for (int i=0; i<info.nField(); ++i)
  // {
  //   _process_field(t, lt, i, first, info);
  // }
	 
  // _thread.lockForIO();
  // info.output(_parms, t, lt);
  // _thread.unlockAfterIO();



  exit(0);
}
