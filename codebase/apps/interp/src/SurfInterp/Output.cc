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
/////////////////////////////////////////////////////////////
// Output.cc
//
// Output class - handles the output to MDV files
//
// Niles Oien
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan  1999
//
///////////////////////////////////////////////////////////////

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Output.hh"

using namespace std;
  
/*********************************************************************
 * Constructors
 */

Output::Output(const DateTime &data_time, const int data_duration,
	       const Pjg &proj, const float altitude,
	       const string &dataset_info, const string &dataset_name,
	       const string &dataset_source, const string &prog_name) :
  _progName(prog_name),
  _outputProj(proj)
{
  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = time(0);
  master_hdr.user_time = data_time.utime();
  master_hdr.time_begin = data_time.utime() - data_duration;
  master_hdr.time_end = data_time.utime();
  master_hdr.time_centroid = data_time.utime();

  master_hdr.time_expire = data_time.utime() + data_duration;

  master_hdr.num_data_times = 1;                
  master_hdr.index_number = 1;

  if (_outputProj.getNx() == 1)
    master_hdr.data_dimension = 2;
  else
    master_hdr.data_dimension = 3;

  master_hdr.data_collection_type = Mdvx::DATA_MEASURED;
  
  master_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  master_hdr.vlevel_included = 1;
               
  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;  

  master_hdr.n_fields = 0;
  master_hdr.max_nx = 0;
  master_hdr.max_ny = 0;
  master_hdr.max_nz = 0; 

  master_hdr.n_chunks = 0;

  master_hdr.field_grids_differ= 0;

  master_hdr.sensor_lon = _outputProj.getOriginLon();
  master_hdr.sensor_lat = _outputProj.getOriginLat();
  master_hdr.sensor_alt = altitude;
      
  sprintf(master_hdr.data_set_info, "%s", dataset_info.c_str());
  sprintf(master_hdr.data_set_name, "%s", dataset_name.c_str());
  sprintf(master_hdr.data_set_source, "%s", dataset_source.c_str());

  _mdvx.setMasterHeader(master_hdr); 
}


/*********************************************************************
 * Destructor
 */

Output::~Output()
{
}
  

/*********************************************************************
 * addField() - Add the given field to the output file.
 */

void Output::addField(const char *var_name, const char *short_var_name,
		      const char *var_units,
		      const float *data,
		      const float bad, const float missing,
		      const Mdvx::encoding_type_t encoding_type,
		      const Mdvx::scaling_type_t scaling_type,
		      const double scale,
		      const double bias)
{
  Mdvx::master_header_t master_hdr = _mdvx.getMasterHeader();

  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);               

  date_time_t Now; ulocaltime (&Now);

  fhdr.user_time1 =     Now.unix_time;
  fhdr.forecast_delta = 0;
  fhdr.user_time2 =     master_hdr.time_begin;
  fhdr.user_time3 =     master_hdr.time_end;
  fhdr.forecast_time =  master_hdr.time_centroid;
  fhdr.user_time4 =     master_hdr.time_expire; 

  // Then reals
  fhdr.proj_origin_lat =  master_hdr.sensor_lat;
  fhdr.proj_origin_lon =  master_hdr.sensor_lon;
  fhdr.vert_reference = 0.0;

  fhdr.bad_data_value = bad;
  fhdr.missing_data_value = bad;
  fhdr.proj_rotation = 0.0;

  // Then characters
  sprintf(fhdr.field_name_long,"%s",var_name);
  sprintf(fhdr.field_name,"%s",short_var_name);
  sprintf(fhdr.units,"%s",var_units);
  sprintf(fhdr.transform,"%s"," ");

  _outputProj.syncToFieldHdr(fhdr);
  
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;     
  
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vhdr.level[0] = fhdr.grid_minz;
  
  // create field
  
  MdvxField *field = new MdvxField(fhdr, vhdr, data);    

  field->convertType(encoding_type,
		     Mdvx::COMPRESSION_GZIP,
		     scaling_type, scale, bias);

  // add field to mdvx object

  _mdvx.addField(field);                        
}


/*********************************************************************
 * write() - Write the output file to disk.
 *
 * Returns true on success, false on failure.
 */

bool Output::write(char *output_url)
{
  _mdvx.setWriteLdataInfo();

  if (_mdvx.writeToDir(output_url)) 
  {
    cerr << _progName << ": Output::write(): Cannot write to url: "
	 << output_url << endl;
    cerr << _mdvx.getErrStr() << endl;
    return false;
  }
          
  return true;
}
