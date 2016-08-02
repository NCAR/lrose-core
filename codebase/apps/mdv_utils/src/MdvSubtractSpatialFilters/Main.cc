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


#include <Mdv/DsMdvx.hh>
#include <iostream>
#include <stdlib.h>
#include <toolsa/umisc.h>
#include <cstdio>
#include <Mdv/MdvxField.hh>


int main(int argc, char *argv[]){

  if (argc < 4){
    cerr << "USAGE : MdvSubtractSpatialFilters A B factor, A-B*factor is written back to A" << endl;
    exit(-1);
  }

  double factor = atof(argv[3]);

  DsMdvx A;
  A.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  A.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  A.setReadPath(argv[1]);
  if (A.readVolume()){
    cerr << "Read failed from " << argv[1] << endl;
    exit(-1);
  }

  MdvxField *AField = A.getFieldByNum( 0 );
  Mdvx::field_header_t AFhdr = AField->getFieldHeader();

  //
  // Read B, first set remap so grid matches A
  //
  DsMdvx B;
  B.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  B.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  switch (AFhdr.proj_type){

  case Mdvx::PROJ_FLAT :
    B.setReadRemapFlat(AFhdr.nx, AFhdr.ny,
		       AFhdr.grid_minx, AFhdr.grid_miny,
		       AFhdr.grid_dx, AFhdr.grid_dy,
		       AFhdr.proj_origin_lat, AFhdr.proj_origin_lon,
		       AFhdr.proj_rotation);
    break;

  case Mdvx::PROJ_LATLON :
    B.setReadRemapLatlon(AFhdr.nx, AFhdr.ny,
			 AFhdr.grid_minx, AFhdr.grid_miny,
			 AFhdr.grid_dx, AFhdr.grid_dy);
    break;
    
  case Mdvx::PROJ_LAMBERT_CONF :
    B.setReadRemapLc2(AFhdr.nx, AFhdr.ny,
		      AFhdr.grid_minx, AFhdr.grid_miny,
		      AFhdr.grid_dx, AFhdr.grid_dy,
		      AFhdr.proj_origin_lat, AFhdr.proj_origin_lon,
		      AFhdr.proj_param[0], AFhdr.proj_param[1]);
    break;
     
  default :
    cerr << "What projection type is " << argv[1] << " !?" << endl;
    exit(-1);
    break;

  }

  B.setReadPath(argv[2]);
  if (B.readVolume()){
    cerr << "Read failed from " << argv[2] << endl;
    exit(-1);
  }

  MdvxField *BField = B.getFieldByNum( 0 );
  Mdvx::field_header_t BFhdr = BField->getFieldHeader();

  float *Adata = (float *) AField->getVol();
  float *Bdata = (float *) BField->getVol();

  for (int ik=0; ik < AFhdr.nx * AFhdr.ny; ik++){

    if (
	(Adata[ik] == AFhdr.bad_data_value) ||
	(Adata[ik] == AFhdr.missing_data_value) ||
	(Bdata[ik] == BFhdr.bad_data_value) ||
	(Bdata[ik] == BFhdr.missing_data_value)
	){
      continue;
    }
    Adata[ik] = Adata[ik] - Bdata[ik] * factor;
  }
  
  Mdvx::vlevel_header_t AVhdr = AField->getVlevelHeader();
  MdvxField *fld = new MdvxField(AFhdr, AVhdr, Adata);
  Mdvx::master_header_t AMhdr = A.getMasterHeader();

  DsMdvx C;
  C.setMasterHeader( AMhdr);
  C.clearFields();
  C.addField( fld );

  C.setWritePath( argv[1] );
  if (C.writeToPath()) {
    cerr << "ERROR - Output::write" << endl;
    cerr << "  Cannot write to path : " << argv[1] << endl;
    cerr << C.getErrStr() << endl;
  }

  return 0;

}
