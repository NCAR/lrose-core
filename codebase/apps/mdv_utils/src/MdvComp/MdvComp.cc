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
// MdvComp.cc
//
// MdvComp object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// September 1999
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include "MdvComp.hh"
#include <set>
using namespace std;

// Constructor

MdvComp::MdvComp(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "MdvComp";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = const_cast<char*>(string("unknown").c_str());
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
    return;
  }

  // check that start and end time is set in archive mode

  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR - must specify start and end dates." << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
      return;
    }
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // initialize the data input object

  if (_params.mode == Params::REALTIME) {
    if (_input.setRealtime(_params.input_url, 600,
			   PMU_auto_register)) {
      isOK = false;
    }
  } else if (_params.mode == Params::ARCHIVE) {
    if (_input.setArchive(_params.input_url,
			  _args.startTime,
			  _args.endTime)) {
      isOK = false;
    }
  } else if (_params.mode == Params::FILELIST) {
    if (_input.setFilelist(_args.inputFileList)) {
      isOK = false;
    }
  }

  return;

}

// destructor

MdvComp::~MdvComp()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int MdvComp::Run()
{
  
  int iret = 0;

  // register with procmap
  
  PMU_auto_register("Run");

  // loop until end of data
  
  _input.reset();
  while (!_input.endOfData()) {
    
    PMU_auto_register("In main loop");
    
    // create input DsMdvx object
    
    DsMdvx inMdvx;
    if (_params.debug) {
      inMdvx.setDebug(_params.debug >= Params::DEBUG_VERBOSE);
    }
    
    // do the read

    if (_doRead(inMdvx)) {
      continue;
    }
    
    PMU_auto_register("Before write");
    
    // create output DsMdvx object
    
    DsMdvx outMdvx;
    if (_params.debug) {
      outMdvx.setDebug(_params.debug >= Params::DEBUG_VERBOSE);
    }
    if (_params.output_path_in_forecast_format) {
      outMdvx.setWriteAsForecast();
    }

    outMdvx.setAppName(_progName);
    outMdvx.setWriteLdataInfo();
    Mdvx::master_header_t mhdr = inMdvx.getMasterHeader();
    outMdvx.setMasterHeader(mhdr);
    string info = inMdvx.getMasterHeader().data_set_info;
    info += " : Composites computed using MdvComp";
    outMdvx.setDataSetInfo(info.c_str());

    // compute the composite, adding the fields to the output object
  
    outMdvx.clearFields();
    
    if (_params.field_select == Params::ALL_FIELDS ||
	_params.field_select == Params::SOME_FIELDS) {
      
      if (_params.comp_method == Params::COMP_FULL) {
	_computeFullComp(inMdvx, outMdvx);
      } else {
	_computeLayeredComp(inMdvx, outMdvx);
      }
      
    } else if (_params.field_select == Params::COMBO_FIELDS) {
      
      _computeComboComp(inMdvx, outMdvx);
      
    }
    
    PMU_auto_register("Before write");
    
    // set output compression
    
    for (int i = 0; i < outMdvx.getNFields(); i++) {
      MdvxField *field = outMdvx.getFieldByNum(i);
      field->requestCompression(_params.output_compression_type);
    }

    // add any chunks

    outMdvx.clearChunks();
    for (int i = 0; i < inMdvx.getNChunks(); i++) {
      MdvxChunk *chunk = new MdvxChunk(*inMdvx.getChunkByNum(i));
      outMdvx.addChunk(chunk);
    }
    
    // write out
    
    if(outMdvx.writeToDir(_params.output_url)) {
      cerr << "ERROR - MdvComp::Run" << endl;
      cerr << "  Cannot write data set." << endl;
      cerr << outMdvx.getErrStr() << endl;
      iret = -1;
    }
    
  } // while
  
  return iret;

}

/////////////////////////////////////////////////////////
// perform the read
//
// Returns 0 on success, -1 on failure.

int MdvComp::_doRead(DsMdvx &inMdvx)
  
{
  
  // create unique list of fields to read
  
  set<string, less<string> > readFields;
  if (_params.field_select == Params::SOME_FIELDS) {
    for (int i = 0; i < _params.field_names_n; i++) {
      readFields.insert(readFields.begin(), _params._field_names[i]);
    }
  } else if (_params.field_select == Params::COMBO_FIELDS) {
    for (int i = 0; i < _params.field_vlevel_combo_n; i++) {
      readFields.insert(readFields.begin(),
			(_params._field_vlevel_combo[i].in_fld_name));
    }
  }
  
  inMdvx.clearRead();
  
  // specify the fields to be read in
  
  if (_params.field_select == Params::SOME_FIELDS ||
      _params.field_select == Params::COMBO_FIELDS) {
    set<string, less<string> >::iterator ii;
    for (ii = readFields.begin(); ii != readFields.end(); ii++) {
      inMdvx.addReadField(*ii);
      if (_params.debug) {
	cerr << "Adding field to request: " << *ii << endl;
      }
    }
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    inMdvx.printReadRequest(cerr);
  }
  
  // read in
  
  PMU_auto_register("Before read");
  
  if (_input.readVolumeNext(inMdvx)) {
    cerr << "ERROR - MdvComp::_doRead" << endl;
    cerr << "  Cannot read in data." << endl;
    cerr << _input.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Read in file: " << inMdvx.getPathInUse() << endl;
  }

  return 0;

}
  
/////////////////////////////////////////////////////////
// compute the full composite for all the fields read in

void MdvComp::_computeFullComp(const DsMdvx &inMdvx,
			       DsMdvx &outMdvx)
  
{


  for (int i = 0; i < inMdvx.getNFields(); i++) {
    MdvxField *field = new MdvxField(*inMdvx.getFieldByNum(i));
    if (_params.debug) {
      cerr << "Creating full composite for field: " <<
	field->getFieldName() << endl;
    }
    field->convert2Composite();
    outMdvx.addField(field);
  }

}
  
/////////////////////////////////////////////////////////
// compute the layered composite for all the fields read in

void MdvComp::_computeLayeredComp(const DsMdvx &inMdvx,
				  DsMdvx &outMdvx)
  
{

  for (int ifield = 0; ifield < inMdvx.getNFields(); ifield++) {

    MdvxField inField(*inMdvx.getFieldByNum(ifield));
    inField.decompress();
    
    if (_params.debug) {
      cerr << "Creating layered composite for field: " <<
	inField.getFieldName() << endl;
    }

    // data buffer for layer composite
    MemBuf dataBuf;
    int nzOut = _params.layer_vlevels_n;

    // get headers

    Mdvx::field_header_t fhdr = inField.getFieldHeader();
    Mdvx::vlevel_header_t vhdr = inField.getVlevelHeader();
    for (int iz = 0; iz < fhdr.nz; iz++) {
      vhdr.level[iz] = 0.0;
    }

    for (int iz = 0; iz < nzOut; iz++) {
      
      MdvxField fld(inField);
      double lower = _params._layer_vlevels[iz].lower_vlevel;
      double mid = _params._layer_vlevels[iz].mid_vlevel;
      double upper = _params._layer_vlevels[iz].upper_vlevel;
      
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "  Lower vlevel: " << lower << endl;
	cerr << "  Mid vlevel: " << mid << endl;
	cerr << "  Upper vlevel: " << upper << endl;
      }

      if (iz == 0) {
	fhdr.grid_minz = mid;
      }
      vhdr.level[iz] = mid;
      fld.convert2Composite(lower, upper);
      dataBuf.add(fld.getVol(), fld.getVolLen());
      
    }

    fhdr.nz = nzOut;
    fhdr.volume_size = dataBuf.getLen();
    fhdr.dz_constant = false;
    
    MdvxField *field = new MdvxField(fhdr, vhdr, dataBuf.getPtr());
    outMdvx.addField(field);

  } // i

}
  
/////////////////////////////////////////////////////////
// compute the composite for all the combination of
// fields and vlevels

void MdvComp::_computeComboComp(const DsMdvx &inMdvx,
				DsMdvx &outMdvx)
  
{
  
  outMdvx.clearFields();
  
  // go through the specified fields, computing the composites
  
  for (int i = 0; i < _params.field_vlevel_combo_n; i++) {

    char *fldName = _params._field_vlevel_combo[i].in_fld_name;
    MdvxField *inField = inMdvx.getFieldByName(fldName);

    if (inField != NULL) {
      
      // copy the field

      MdvxField *outField = new MdvxField(*inField);
      
      // compute composite
      
      double lowerVlevel = _params._field_vlevel_combo[i].lower_vlevel;
      double upperVlevel = _params._field_vlevel_combo[i].upper_vlevel;

      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "MdvComp::_computeComposites" << endl;
	cerr << "  Cannot compute composite." << endl;
	cerr << "  Field name: " << fldName << endl;
	cerr << "  Lower vlevel: " << lowerVlevel << endl;
	cerr << "  Upper vlevel: " << upperVlevel << endl;
      }

      if (outField->convert2Composite(lowerVlevel, upperVlevel)) {

	cerr << "ERROR - MdvComp::_computeComposites" << endl;
	cerr << "  Cannot compute composite." << endl;
	cerr << "  Field name: " << fldName << endl;
	cerr << "  Lower vlevel: " << lowerVlevel << endl;
	cerr << "  Upper vlevel: " << upperVlevel << endl;

	delete outField;

      } else {

	// set the names
	
	outField->setFieldName(_params._field_vlevel_combo[i].out_fld_name);
	string longName = _params._field_vlevel_combo[i].in_fld_name;
	char tmpStr[128];
	sprintf(tmpStr, " - composite vlevel %g to %g",
		_params._field_vlevel_combo[i].lower_vlevel,
		_params._field_vlevel_combo[i].upper_vlevel);
	longName += tmpStr;
	outField->setFieldNameLong(longName.c_str());
	
	// add the field
	
	outMdvx.addField(outField);

      }
	
    } // if (inField != NULL) {

  } // i

}
