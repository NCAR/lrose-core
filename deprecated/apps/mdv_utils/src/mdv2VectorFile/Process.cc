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


#include "Process.hh"

#include <iostream>
#include <Mdv/MdvxField.hh>
#include <math.h>
#include <toolsa/str.h>
#include <toolsa/pjg_flat.h>
#include <Mdv/MdvxProj.hh>
using namespace std;

//
// Constructor
//
Process::Process(){
  return;
}

////////////////////////////////////////////////////
//
// Main method - process data at a given time.
//
int Process::Derive(Params *P, time_t T){


  if (P->Debug){
    cerr << "Data at " << utimstr(T) << endl;
  }

  if (ta_makedir_recurse(P->OutDir )){
    cerr << "Failed to create output directory " << P->OutDir << endl;
    return -1;
  }

  //
  // Set up for the new data.
  //
  DsMdvx New;

  New.setReadTime(Mdvx::READ_FIRST_BEFORE, P->TriggerUrl, 0, T);
  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  for (int i=0; i< P->fieldNames_n; i++){
    New.addReadField(P->_fieldNames[i]);
  }

  if (New.readVolume()){
    cerr << "Read failed at " << utimstr(T) << " from ";
    cerr << P->TriggerUrl  << endl;
    return -1;
  }     

  //
  //
  Mdvx::master_header_t InMhdr = New.getMasterHeader();
  date_time_t ft, vt;
  vt.unix_time = InMhdr.time_centroid; uconvert_from_utime( &vt );
  ft.unix_time = InMhdr.time_gen; uconvert_from_utime( &ft );

  for (int ifield=0; ifield < InMhdr.n_fields; ifield++){

    //
    // Get the desired field.
    //
    MdvxField *InField = New.getFieldByNum(ifield);

    if (InField == NULL){
      cerr << "New field number " << ifield << " not found." << endl;
      continue;
    }

    // Open the output file
    Mdvx::field_header_t InFhdr = InField->getFieldHeader();
    if (P->Debug) cerr << "Processing field " << InFhdr.field_name << " in " << New.getPathInUse() << endl;
    char outfileName[1024];
    sprintf(outfileName,"%s/%4d%02d%02d_%02d%02d%02d__%4d%02d%02d_%02d%02d%02d_%s.vec",
	    P->OutDir, 
	    vt.year, vt.month, vt.day, vt.hour, vt.min, vt.sec,
	    ft.year, ft.month, ft.day, ft.hour, ft.min, ft.sec,
	    InFhdr.field_name);

    FILE *fp = fopen(outfileName,"w");
    if (fp == NULL){
      cerr << "Failed to create " << outfileName << endl;
      continue;
    }

    // Set up indices to increase or decrease as requested.
    int xStart=0;
    int xEnd=InFhdr.nx;
    int xInc = 1;

    if (P->indexDecrease.xIndexDecreases){
      xStart=InFhdr.nx-1;
      xEnd=-1;
      xInc = -1;
    }

    int yStart=0;
    int yEnd=InFhdr.ny;
    int yInc = 1;

    if (P->indexDecrease.yIndexDecreases){
      yStart=InFhdr.ny-1;
      yEnd=-1;
      yInc = -1;
    }

    int zStart=0;
    int zEnd=InFhdr.nz;
    int zInc = 1;

    if (P->indexDecrease.zIndexDecreases){
      zStart=InFhdr.nz-1;
      zEnd=-1;
      zInc = -1;
    }

 
    fl32 *InData = (fl32 *) InField->getVol();
    
    switch (P->outputOrder){

    case Params::ORDER_XYZ :

      for (int ix=xStart; ix != xEnd; ix+=xInc){
	for (int iy=yStart; iy != yEnd; iy+=yInc){
	  for (int iz=zStart; iz != zEnd; iz+=zInc){
	    int index = iz*InFhdr.nx*InFhdr.ny + iy*InFhdr.nx + ix;
	    double printVal = InData[index];
	    if ((printVal == InFhdr.bad_data_value) || (printVal == InFhdr.missing_data_value))
	      printVal = P->badVal;
	    fprintf(fp, "%lf\n", printVal);
	  }
	}
      }
      
      break;
      
      
    case Params::ORDER_XZY :
      for (int ix=xStart; ix != xEnd; ix+=xInc){
	for (int iz=zStart; iz != zEnd; iz+=zInc){
	  for (int iy=yStart; iy != yEnd; iy+=yInc){
	    int index = iz*InFhdr.nx*InFhdr.ny + iy*InFhdr.nx + ix;
	    double printVal = InData[index];
	    if ((printVal == InFhdr.bad_data_value) || (printVal == InFhdr.missing_data_value))
	      printVal = P->badVal;
	    fprintf(fp, "%lf\n", printVal);
	  }
	}
      }

      break;


    case Params::ORDER_YXZ :
      
      for (int iy=yStart; iy != yEnd; iy+=yInc){
	for (int ix=xStart; ix != xEnd; ix+=xInc){
	  for (int iz=zStart; iz != zEnd; iz+=zInc){
	    int index = iz*InFhdr.nx*InFhdr.ny + iy*InFhdr.nx + ix;
	    double printVal = InData[index];
	    if ((printVal == InFhdr.bad_data_value) || (printVal == InFhdr.missing_data_value))
	      printVal = P->badVal;
	    fprintf(fp, "%lf\n", printVal);
	  }
	}
      }
      break;
      

    case Params::ORDER_ZXY :

      for (int iz=zStart; iz != zEnd; iz+=zInc){
	for (int ix=xStart; ix != xEnd; ix+=xInc){
	  for (int iy=yStart; iy != yEnd; iy+=yInc){
	    int index = iz*InFhdr.nx*InFhdr.ny + iy*InFhdr.nx + ix;
	    double printVal = InData[index];
	    if ((printVal == InFhdr.bad_data_value) || (printVal == InFhdr.missing_data_value))
	      printVal = P->badVal;
	    fprintf(fp, "%lf\n", printVal);
	  }
	}
      }
      break;
      
    case Params::ORDER_ZYX :

      for (int iz=zStart; iz != zEnd; iz+=zInc){
	for (int iy=yStart; iy != yEnd; iy+=yInc){
	  for (int ix=xStart; ix != xEnd; ix+=xInc){
	    int index = iz*InFhdr.nx*InFhdr.ny + iy*InFhdr.nx + ix;
	    double printVal = InData[index];
	    if ((printVal == InFhdr.bad_data_value) || (printVal == InFhdr.missing_data_value))
	      printVal = P->badVal;
	    fprintf(fp, "%lf\n", printVal);
	  }
	}
      }
      break;
      

    case Params::ORDER_YZX :

      for (int iy=yStart; iy != yEnd; iy+=yInc){
	for (int iz=zStart; iz != zEnd; iz+=zInc){
	   for (int ix=xStart; ix != xEnd; ix+=xInc){
	    int index = iz*InFhdr.nx*InFhdr.ny + iy*InFhdr.nx + ix;
	    double printVal = InData[index];
	    if ((printVal == InFhdr.bad_data_value) || (printVal == InFhdr.missing_data_value))
	      printVal = P->badVal;
	    fprintf(fp, "%lf\n", printVal);
	  }
	}
      }
      break;

    default :
      cerr << "Odd ordering specified!" << endl;
      exit(-1);
      break;
   
    }

    fclose(fp);

  } // End of loop through the fields.

  if (P->Debug){
    cerr << "Finished data processing for this time." << endl << endl;
  }

  return 0;

}

////////////////////////////////////////////////////
//
// Destructor
//
Process::~Process(){
  return;
}










