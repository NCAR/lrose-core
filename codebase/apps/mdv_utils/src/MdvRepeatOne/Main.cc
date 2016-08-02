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
#include "Params.hh"
#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxChunk.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <cstdio>
#include <iostream>
#include <unistd.h>


int main( int argc, char *argv[] )
{
  //
  // Get params and check in.
  //  
  Params params;

  if (params.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }

  PMU_auto_init("MdvRepeat", params.instance, PROCMAP_REGISTER_INTERVAL);

  DateTime dt(params.date.year, params.date.month, params.date.day,
	      params.date.hour, params.date.min, params.date.sec);
  time_t t = dt.utime();

  DsMdvx D;
  D.setReadTime(Mdvx::READ_FIRST_BEFORE, params.InUrl, 0, t);
  if (D.readVolume()){
    cerr << "Read failed at " << DateTime::strn(t) << " from ";
    cerr << params.InUrl  << endl;
    return(-1);
  }
  Mdvx::master_header_t mIn = D.getMasterHeader();

  for (;;)
  {
    DsMdvx Dout(D);
    Mdvx::master_header_t Mhdr = Dout.getMasterHeader();

    time_t tnow = time(0);
    t = tnow - params.realtimeDelay;
    Mhdr.time_gen = t;
    Mhdr.time_begin = Mhdr.time_end = Mhdr.time_centroid = Mhdr.time_expire = t;
    // Change the name so it's clear that these are replayed data.
    sprintf(Mhdr.data_set_info, "MdvRepeatOne");
    sprintf(Mhdr.data_set_name, "MdvRepeatOne");
    sprintf(Mhdr.data_set_source, "MdvRepeatOne");

    Mhdr.max_nx = Mhdr.max_ny = Mhdr.max_nz = 0;

    Dout.setMasterHeader(Mhdr);

    Dout.clearFields();
    //
    // Loop throught the fields, adjust the time in the
    // field header (only one entry) and add it to the output
    // object.
    //
    for (int ifld=0; ifld < mIn.n_fields; ifld++)
    {
      MdvxField *InField = D.getFieldByNum( ifld );

      if (InField == NULL){
	cerr << "Field " << ifld << " not found." << endl;
	continue;
      }

      // convert to float32
      InField->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);


      Mdvx::field_header_t Fhdr = InField->getFieldHeader();
      Fhdr.forecast_time = t;
      Mdvx::vlevel_header_t Vhdr = InField->getVlevelHeader();
      fl32 *data = (fl32 *)InField->getVol();
      
      Mdvx::field_header_t FhdrOut(Fhdr);
      if(params.do_resample == true) {
        FhdrOut.nx = params.resample_proj_info.nx;
        FhdrOut.ny = params.resample_proj_info.ny;
        FhdrOut.grid_minx = params.resample_proj_info.minx;
        FhdrOut.grid_miny = params.resample_proj_info.miny;
      }
      FhdrOut.volume_size = FhdrOut.nx *  FhdrOut.ny * 
	FhdrOut.nz * FhdrOut.data_element_nbytes;

      MdvxField *fld = new MdvxField(FhdrOut, Vhdr, (void *)0, true, false);
      fl32 *fo = (fl32 *)fld->getVol();
      for (int iz=0; iz<FhdrOut.nz; ++iz)
      {
	for (int iy=0; iy<FhdrOut.ny; ++iy)
	{
	  double y = FhdrOut.grid_miny + FhdrOut.grid_dy*iy;
	  int in_y = (y - Fhdr.grid_miny)/Fhdr.grid_dy;
	  if (in_y < 0 || in_y >= Fhdr.ny)
	  {
	    cerr << "Y=" << y << " is out of range" << endl;
	    continue;
	  }
	  for (int ix=0; ix<FhdrOut.nx; ++ix)
	  {
	    double x = FhdrOut.grid_minx + FhdrOut.grid_dx*ix;
	    int in_x = (x - Fhdr.grid_minx)/Fhdr.grid_dx;
	    if (in_x < 0 || in_x >= Fhdr.nx)
	    {
	      cerr << "X=" << x << " is out of range" << endl;
	      continue;
	    }

	    int iout = iz*FhdrOut.nx*FhdrOut.ny + iy*FhdrOut.nx + ix;
	    if (iout < 0 || iout >= FhdrOut.volume_size )
	    {
	      cerr << "Out of range " << iz << endl;
	      continue;
	    }
	    int iin = iz*Fhdr.nx*Fhdr.ny + in_y*Fhdr.nx + in_x;
	    if (iin < 0 || iin >= Fhdr.volume_size )
	    {
	      cerr << "Out of range " << iz << endl;
	      continue;
	    }

	    fo[iout] = data[iin];
	  }
	}
      }
      Dout.addField(fld);
    }

    //
    // Add any chunks from the input.
    //
    for (int ic=0; ic < D.getNChunks(); ic++){
      MdvxChunk *inChunk = D.getChunkByNum( ic );

      //
      // The chunk must be created using 'new'.
      // The chunk object becomes 'owned' by the Mdvx object, and will be
      // freed by it.
      //
      MdvxChunk *outChunk = new MdvxChunk( *inChunk );
      Dout.addChunk( outChunk );
    }

    //
    // Write it out.
    //
    if (Dout.writeToDir(params.OutUrl))
    {
      cerr << "Failed to wite to " << params.OutUrl << endl;
    }

    sleep(params.Delay);
  }
  return 0;
}
