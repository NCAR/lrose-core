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
///////////////////////////////////////////////////////////////
// OutputFile.cc
//
// Handles output to MDV files
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////

#include "OutputFile.hh"

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <Mdv/MdvxField.hh>
using namespace std;

//////////////
// Constructor

OutputFile::OutputFile(const string &prog_name, const Params &params) :
  _progName(prog_name), _params(params)

{

  _fracFieldNum = 0;
  _meanFieldNum = 1;
  _sdevFieldNum = 2;
  _corrFieldNum = 3;
  _nFields = 4;
  
}

/////////////
// destructor

OutputFile::~OutputFile()

{
  
}

////////////////////////////////////////
// write()
//
// Write out the precip data
//

int OutputFile::write(time_t start_time,
		      time_t end_time,
		      time_t centroid_time,
		      const Mdvx::master_header_t &mhdrIn,
		      const Mdvx::field_header_t &fhdrIn,
		      const Mdvx::vlevel_header_t &vhdrIn,
		      const fl32 *frac,
		      const fl32 *mean,
		      const fl32 *sdev,
		      const fl32 *corr)
  
{
  
  DsMdvx mdvx;
  
  // set the master header
  
  Mdvx::master_header_t mhdr = mhdrIn;
  
  mhdr.time_begin = start_time;
  mhdr.time_end = end_time;
  mhdr.time_centroid = centroid_time;
  mhdr.time_expire = end_time;
  
  mhdr.n_chunks = 0;
  mhdr.field_grids_differ = FALSE;

  // info
  
  char info[1024];
  sprintf(info, "Clutter statistics.\n");
  STRncopy(mhdr.data_set_info, info, MDV_INFO_LEN);
  STRncopy(mhdr.data_set_name, _params.data_set_name, MDV_NAME_LEN);
  STRncopy(mhdr.data_set_source, _params.data_set_source, MDV_NAME_LEN);
  
  mdvx.setMasterHeader(mhdr);
  
  // fill in field headers and vlevel headers
  
  mdvx.clearFields();
  
  for (int oField = 0; oField < _nFields; oField++) {
    
    Mdvx::field_header_t fhdr = fhdrIn;
    Mdvx::vlevel_header_t vhdr = vhdrIn;

    fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
    fhdr.data_element_nbytes = sizeof(fl32);
    fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);
    fhdr.compression_type = Mdvx::COMPRESSION_NONE;
    fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
    fhdr.scaling_type = Mdvx::SCALING_NONE;

    fhdr.bad_data_value = 0.0;
    fhdr.missing_data_value = 0.0;
    
    // create field
    
    MdvxField *field;
    if (oField == _fracFieldNum) {
      field = new MdvxField(fhdr, vhdr, frac);
    } else if (oField == _meanFieldNum) {
      field = new MdvxField(fhdr, vhdr, mean);
    } else if (oField == _sdevFieldNum) {
      field = new MdvxField(fhdr, vhdr, sdev);
    } else if (oField == _corrFieldNum) {
      field = new MdvxField(fhdr, vhdr, corr);
    }
    
    field->convertRounded((Mdvx::encoding_type_t) _params.output_encoding_type,
			  Mdvx::COMPRESSION_ZLIB);
    
    if (oField == _fracFieldNum) {
      field->setFieldName("fracUsed");
      field->setFieldNameLong("Fraction data used");
      field->setUnits("val");
      field->setTransform("none");
    } else if (oField == _meanFieldNum) {
      field->setFieldName("mean");
      field->setFieldNameLong("Mean");
      field->setUnits("DBZ");
      field->setTransform("DBZ");
    } else if (oField == _sdevFieldNum) {
      field->setFieldName("sdev");
      field->setFieldNameLong("Standard Deviation");
      field->setUnits("DBZ");
      field->setTransform("DBZ");
    } else if (oField == _corrFieldNum) {
      field->setFieldName("autoCorr");
      field->setFieldNameLong("Auto Correlation");
      field->setUnits("val");
      field->setTransform("none");
    }
    
    // add field to mdvx object
    
    mdvx.addField(field);
    
  } // oField
  
  mdvx.setWriteLdataInfo();
  
  if (_params.debug) {
    cerr << "Writing data, for time: "
	 << DateTime::str(centroid_time) << " to url:" 
	 << _params.output_url << endl;
  }
  
  if (mdvx.writeToDir(_params.output_url)) {
    cerr << "ERROR - OutputFile::write()" << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }

  return 0;
  
}



