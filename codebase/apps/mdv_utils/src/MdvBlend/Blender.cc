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
//////////////////////////////////////////////////////////
// $Id: Blender.cc,v 1.9 2016/03/04 02:22:10 dixon Exp $
//
// Blends two Mdv files into one
//
// Yan Chen, RAL, NCAR
//
// Dec. 2007
//
//////////////////////////////////////////////////////////

#include <cmath>
#include <sstream>
#include <toolsa/pmu.h>
#include "Blender.hh"
using namespace std;

// constructor

Blender::Blender(const string &prog_name, const Params &params) :
  _progName(prog_name), _params(params)
{

}

// destructor

Blender::~Blender()
{
  
}

//////////////////////////////////////////////////
// Blend the files 

int Blender::blendFiles(DsMdvx &shMdvx, DsMdvx &nhMdvx) {

  // register with procmap
  PMU_auto_register("Blender::blendFiles");

  shMdvx.setReadEncodingType(Mdvx::ENCODING_ASIS);
  shMdvx.setReadCompressionType(Mdvx::COMPRESSION_ASIS);
  nhMdvx.setReadEncodingType(Mdvx::ENCODING_ASIS);
  nhMdvx.setReadCompressionType(Mdvx::COMPRESSION_ASIS);

  if (_params.field_names_n > 0) {
    for (int i = 0; i < _params.field_names_n; i++) {
      shMdvx.addReadField(_params._field_names[i]);
      nhMdvx.addReadField(_params._field_names[i]);
    }
  }

  bool read_sh_success = true;
  bool read_nh_success = true;

  string dataSource = "Data blended from the following files:\n";

  if (_params.debug) {
    cerr << "Reading SH file..." << endl;
  }

  if (shMdvx.readVolume()) {
    cerr << "ERROR: readVolume() " << endl;
    cerr << shMdvx.getErrStr() << endl;
    read_sh_success = false;
  }

  if (_params.debug) {
    cerr << shMdvx.getPathInUse() << endl;
  }

  if (_params.debug) {
    cerr << "Reading NH file..." << endl;
  }

  if (nhMdvx.readVolume()) {
    cerr << "ERROR: readVolume() " << endl;
    cerr << nhMdvx.getErrStr() << endl;
    read_nh_success = false;
  }

  if (_params.debug) {
    cerr << nhMdvx.getPathInUse() << endl;
  }

  if (!read_sh_success && !read_nh_success) {
    return -1;
  }

  if (read_sh_success && !read_nh_success) {
    if (_params.mode == Params::REALTIME) {
      return -1;
    } else {
      DsMdvx outMdvx(shMdvx);
      dataSource += shMdvx.getPathInUse();
      dataSource += "\n";
      _write_output(outMdvx, dataSource);
      return 0;
    }
  }

  if (!read_sh_success && read_nh_success) {
    if (_params.mode == Params::REALTIME) {
      return -1;
    } else {
      DsMdvx outMdvx(nhMdvx);
      dataSource += nhMdvx.getPathInUse();
      dataSource += "\n";
      _write_output(outMdvx, dataSource);
      return 0;
    }
  }

  // check if files match

  Mdvx::master_header_t sh_master_hdr = shMdvx.getMasterHeader();
  Mdvx::master_header_t nh_master_hdr = nhMdvx.getMasterHeader();
  string err_str = "";
  if (!_files_match(sh_master_hdr, nh_master_hdr, err_str)) {
    cerr << "ERROR: Files do not match: " << endl;
    cerr << shMdvx.getPathInUse() << endl;
    cerr << nhMdvx.getPathInUse() << endl;
    cerr << err_str;
    cerr << "Blending failed." << endl;
    return -1;
  }

  // check if fields match

  int nFields = shMdvx.getNFields();
  for (int field_index = 0; field_index < nFields; field_index++) {
    MdvxField *shField = shMdvx.getFieldByNum(field_index);
    MdvxField *nhField = nhMdvx.getField(shField->getFieldName());
    if (nhField == NULL) {
      cerr << "ERROR: Field: "<< shField->getFieldName() << endl;
      cerr << "Could not be found in file:" << endl;
      cerr << "  " << nhMdvx.getPathInUse() << endl;
      cerr << "Blending failed." << endl;
      return -1;
    }

    err_str = "";
    if (!_fields_match(shField, nhField, err_str)) {
      cerr << "ERROR: Fields do not match: " << shField->getFieldName()
           << endl;
      cerr << err_str << endl;
      cerr << "Blending failed." << endl;
      return -1;
    }
  }

  // all match, blend two files

  if (_params.debug)
    cerr << "Blending..." << endl;

  // copy entire sh file to out file
  DsMdvx outMdvx(shMdvx);

  double southern_limit = _params.blending_zone.southern_limit;
  double northern_limit = _params.blending_zone.northern_limit;

  for (int field_index = 0; field_index < nFields; field_index++) {
    MdvxField *shField = outMdvx.getFieldByNum(field_index);
    MdvxField *nhField = nhMdvx.getFieldByName(shField->getFieldName());
    const Mdvx::field_header_t &sh_fhdr = shField->getFieldHeader();
    const Mdvx::field_header_t &nh_fhdr = nhField->getFieldHeader();

    // save important information for output purpose

    Mdvx::encoding_type_t encoding_type =
      (Mdvx::encoding_type_t)sh_fhdr.encoding_type;
    Mdvx::compression_type_t compression_type =
      (Mdvx::compression_type_t)sh_fhdr.compression_type;
    Mdvx::transform_type_t transform_type =
      (Mdvx::transform_type_t)sh_fhdr.transform_type;
    Mdvx::scaling_type_t scaling_type =
      (Mdvx::scaling_type_t)sh_fhdr.scaling_type;
    double scale = sh_fhdr.scale;
    double bias = sh_fhdr.bias;

    // convert input to float32, none-compression, linear

    shField->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
    shField->transform2Linear();
    shField->setPlanePtrs();

    nhField->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
    nhField->transform2Linear();
    nhField->setPlanePtrs();

    fl32 *sh_data;
    fl32 *sh_data_ptr;
    fl32 *nh_data;
    fl32 *nh_data_ptr;

    double miny = sh_fhdr.grid_miny;
    double dy = sh_fhdr.grid_dy;
    int nx = sh_fhdr.nx;
    int ny = sh_fhdr.ny;
    int nz = sh_fhdr.nz;

    int y_index_at_sh_limit = (int) round((southern_limit - miny) / dy);
    int y_index_at_nh_limit = (int) ceil((northern_limit - miny) / dy);

    if (
      southern_limit == northern_limit ||
      y_index_at_sh_limit == y_index_at_nh_limit
    ) {
      // no blending, just combine two files.

      for (int plane = 0; plane < nz; plane++) {
        // HMMM Work-around
        if (nz > 1) {
          sh_data = (fl32 *) shField->getPlane(plane);
          nh_data = (fl32 *) nhField->getPlane(plane);
        } else {
          sh_data = (fl32 *) shField->getVol();
          nh_data = (fl32 *) nhField->getVol();
        }

        for (int y = y_index_at_sh_limit + 1; y < ny; y++) {
          sh_data_ptr = sh_data + (y * nx);
          nh_data_ptr = nh_data + (y * nx);

          for (int x = 0; x < nx; x++) {
            *sh_data_ptr = *nh_data_ptr;
            sh_data_ptr++;
            nh_data_ptr++;
	  }

	}

      }

    } else {

      vector<double> weights; // in the order of 0.0 to max_weight
      double n_weights = y_index_at_nh_limit - y_index_at_sh_limit + 1;

      if (_params.weight_method == Params::WEIGHT_LINEAR) {
        double increment = _params.max_blending_weight / (n_weights - 1);

        weights.push_back(0.0);
        for (int w = 1; w < n_weights - 1; w++) {
          weights.push_back(w * increment);
        }
        weights.push_back(_params.max_blending_weight);

      } else { // LOG

      }

      for (int plane = 0; plane < nz; plane++) {
        // HMMM Work-around
        if (nz > 1) {
          sh_data = (fl32 *) shField->getPlane(plane);
          nh_data = (fl32 *) nhField->getPlane(plane);
        } else {
          sh_data = (fl32 *) shField->getVol();
          nh_data = (fl32 *) nhField->getVol();
        }
        // Blending from south to north

        for (int y = y_index_at_sh_limit; y <= y_index_at_nh_limit; y++) {
          sh_data_ptr = sh_data + (y * nx);
          nh_data_ptr = nh_data + (y * nx);

          for (int x = 0; x < nx; x++) {

	    // ignore it if data from NH is bad or missing

            if (
              *nh_data_ptr == nh_fhdr.bad_data_value ||
              *nh_data_ptr == nh_fhdr.missing_data_value
	    ) {

              sh_data_ptr++;
              nh_data_ptr++;
              continue;
	    }

	    // data from NH is good
	    // replace it in output object if data from SH is bad or missing

            if (
              *sh_data_ptr == sh_fhdr.bad_data_value ||
              *sh_data_ptr == sh_fhdr.missing_data_value
	    ) {

              *sh_data_ptr = *nh_data_ptr;
              sh_data_ptr++;
              nh_data_ptr++;
              continue;
	    }

	    // both data from NH and SH are good, blend them

            int weight_index = y - y_index_at_sh_limit;

            double blended_value =
              *sh_data_ptr *
              (_params.max_blending_weight - weights[weight_index]) +
              *nh_data_ptr * weights[weight_index];

	    // replace the output value with the blended value
            *sh_data_ptr = blended_value;

            sh_data_ptr++;
            nh_data_ptr++;

	  } // end of x
        }// end of y

        // for the rest of northern part, copy them to the output

        for (int y = y_index_at_nh_limit + 1; y < ny; y++) {
          sh_data_ptr = sh_data + (y * nx);
          nh_data_ptr = nh_data + (y * nx);

          for (int x = 0; x < nx; x++) {
            *sh_data_ptr = *nh_data_ptr;
            sh_data_ptr++;
            nh_data_ptr++;
	  }
        }

      } // end of plane
    } // if southern_limit = northern_limit

    // set min & max to 0, so they will be re-calculated.

    Mdvx::field_header_t out_fhdr = sh_fhdr;

    out_fhdr.min_value = 0.0;
    out_fhdr.max_value = 0.0;
    out_fhdr.min_value_orig_vol = 0.0;
    out_fhdr.max_value_orig_vol = 0.0;
    out_fhdr.bad_data_value = -9999;
    out_fhdr.missing_data_value = -9999;
    shField->setFieldHeader(out_fhdr);

    // convert back to the type as the input

    shField->convertType(
      encoding_type,
      compression_type,
      scaling_type,
      scale,
      bias
    );
    if (transform_type == Mdvx::DATA_TRANSFORM_LOG)
      shField->transform2Log();

  } // end of field

  if (_params.debug)
    cerr << "Writing output..." << endl << endl;

  dataSource += shMdvx.getPathInUse();
  dataSource += "\n";
  dataSource += nhMdvx.getPathInUse();
  dataSource += "\n";

  _write_output(outMdvx, dataSource);

  return 0;
}

//////////////////////////////////////////////////////
// check if master headers from two files match or not

bool Blender::_files_match(
  Mdvx::master_header_t &mhdr1,
  Mdvx::master_header_t &mhdr2,
  string &err_str
) {

  bool iret = true;

  if (mhdr1.n_fields != mhdr2.n_fields) {
    stringstream ss;
    ss << "Two input files have different fields:";
    ss << mhdr1.n_fields << "," << mhdr2.n_fields << endl << endl;
    err_str += ss.str();

    iret = false;
  }

  if (mhdr1.time_begin != mhdr2.time_begin) {
    stringstream ss;
    ss << "Begin times do not match:" << endl;
    ss << "  " << _timeStr(mhdr1.time_begin) << endl;
    ss << "  " << _timeStr(mhdr2.time_begin) << endl << endl;
    err_str += ss.str();

    iret = false;
  }

  if (mhdr1.time_end != mhdr2.time_end) {
    stringstream ss;
    ss << "End times do not match:" << endl;
    ss << "  " << _timeStr(mhdr1.time_end) << endl;
    ss << "  " << _timeStr(mhdr2.time_end) << endl << endl;
    err_str += ss.str();

    iret = false;
  }

  if (mhdr1.data_dimension != mhdr2.data_dimension) {
    stringstream ss;
    ss << "Data dimensions do not match:";
    ss << mhdr1.data_dimension << "," << mhdr2.data_dimension << endl << endl;
    err_str += ss.str();

    iret = false;
  }

  if (mhdr1.grid_orientation != mhdr2.grid_orientation) {
    stringstream ss;
    ss << "Grid orientations do not match:" << endl;
    ss << "  " << Mdvx::orientType2Str(mhdr1.grid_orientation);
    ss << ", " << Mdvx::orientType2Str(mhdr2.grid_orientation) << endl << endl;
    err_str += ss.str();

    iret = false;
  }

  if (mhdr1.data_ordering != mhdr2.data_ordering) {
    stringstream ss;
    ss << "Data orderings do not match:" << endl;
    ss << "  " << Mdvx::orderType2Str(mhdr1.data_ordering);
    ss << ", " << Mdvx::orderType2Str(mhdr2.data_ordering) << endl << endl;
    err_str += ss.str();

    iret = false;
  }

  return(iret);
}

//////////////////////////////////////////////////////
// check if field headers from two files match or not

bool Blender::_fields_match(
  MdvxField *field1,
  MdvxField *field2,
  string &err_str
) {

  bool iret = true;

  const Mdvx::field_header_t &fhdr1 = field1->getFieldHeader();
  const Mdvx::field_header_t &fhdr2 = field2->getFieldHeader();

  MdvxProj proj1(fhdr1);
  MdvxProj proj2(fhdr2);
  if (proj1 != proj2) {
    err_str += "Field projections do not match:\n";
    err_str += fhdr1.field_name_long;
    err_str += ":\n";
    ostringstream oss;
    proj2.print(oss);
    err_str += oss.str();
    err_str += fhdr2.field_name_long;
    err_str += ":\n";
    oss.flush();
    proj2.print(oss);
    err_str += oss.str();

    iret = false;
  }

  if (fhdr1.vlevel_type != fhdr2.vlevel_type) {
    stringstream ss;
    ss << "Field vlevel types do not match:" << endl;
    ss << fhdr1.field_name_long << endl;
    ss << "   vert_type = "
       << Mdvx::vertType2Str(fhdr1.vlevel_type) << endl;
    ss << fhdr2.field_name_long << endl;
    ss << "   vert_type = "
       << Mdvx::vertType2Str(fhdr2.vlevel_type) << endl;
    err_str += ss.str();

    iret = false;
  }

  // Only check the vertical levels if they aren't constant

  if (!fhdr1.dz_constant || !fhdr2.dz_constant) {
    const Mdvx::vlevel_header_t &vlevel_hdr1 = field1->getVlevelHeader();
    const Mdvx::vlevel_header_t &vlevel_hdr2 = field2->getVlevelHeader();

    for (int z = 0; z < fhdr1.nz; ++z) {
      // Make sure all of the v levels match.
      //  I'm assuming that if the type is set to 0,
      // then it really wasn't set at all and the check should be ignored.

      if (
        vlevel_hdr1.type[z] != 0 &&
        vlevel_hdr2.type[z] != 0 &&
        vlevel_hdr1.type[z] != vlevel_hdr2.type[z] ||
        vlevel_hdr1.level[z] != vlevel_hdr2.level[z]
      ) {
        stringstream ss;
        ss << "Field vertical levels do not match:" << endl;
        ss << fhdr1.field_name_long << endl;
        ss << "   type[" << z << "] = "
           << Mdvx::vertType2Str(vlevel_hdr1.type[z]) << endl;
        ss << "   level[" << z << "] = " << vlevel_hdr1.level[z] << endl;
        ss << fhdr2.field_name_long << endl;
        ss << "   type[" << z << "] = "
           << Mdvx::vertType2Str(vlevel_hdr2.type[z]) << endl;
        ss << "   level[" << z << "] = " << vlevel_hdr2.level[z] << endl;
        err_str += ss.str();

        iret = false;
      }
    } /* endfor - z */
  }

  return(iret);
}


/////////////////////////////
// string for rendering time

char *Blender::_timeStr(const time_t ttime)

{
  if (ttime == -1 || ttime == 0) {
    return "not set";
  } else {
    return (utimstr(ttime));
  }
}

//////////////////////////////////////////////////
// Output

int Blender::_write_output(DsMdvx& outMdvx, string &dataSource) {

  outMdvx.setDataSetInfo(dataSource.c_str());
  outMdvx.setDataSetName(_params.output_data_set_info);

  // write out the file

  outMdvx.setAppName(_progName);
  outMdvx.setWriteLdataInfo();

  if (
    _params.mode == Params::ARCHIVE ||
    _params.mode == Params::REALTIME
  ) {
    if (outMdvx.writeToDir(_params.output_url_dir)) {
      cerr << "ERROR: Blender::_write_output()" << endl;
      cerr << "  Cannot write file, url: "
           << _params.output_url_dir << endl;
      cerr << outMdvx.getErrStr() << endl;
      return -1;
    }

  } else if (_params.mode == Params::FILELIST) {
    string output_path(_params.output_url_dir);
    output_path += string(_params.output_file_name);
    if(outMdvx.writeToPath(output_path.c_str())) {
      cerr << "ERROR: Blender::_write_output()" << endl;
      cerr << "  Cannot write file, path:"
           << output_path << endl;
      cerr << outMdvx.getErrStr() << endl;
      return -1;
    }
  }

  return 0;
}



