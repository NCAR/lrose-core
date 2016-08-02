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

#include <Mdv/MdvxField.hh>
#include <toolsa/umisc.h>
#include <toolsa/pjg_flat.h>
#include <Mdv/DsMdvx.hh>
#include <cstdio>

#include <stdlib.h> // For system calls to move output file around.

#include "Mdv2Axf.hh"
using namespace std;

//
// Constructor. Does nothing.
//
Mdv2Axf::Mdv2Axf(){
  AxfFp = NULL;
}

//
// Destructor. Does nothing.
//
Mdv2Axf::~Mdv2Axf(){

}

//
// Open the output file and write the header.
//
int Mdv2Axf::Begin(Mdvx::field_header_t Fhdr, time_t t, Params *P){

  //
  // Make a copy of the Latest File Name - each file, when
  // written, is copied to this file name. This is done in 
  // the destructor.
  //
  sprintf(_LatestFileName,"%s",P->LatestFileName);
  _NumLatestFiles = P->NumLatestFiles;

  //
  // Do some checks before we open the output file.
  //
  if (Fhdr.grid_dx != Fhdr.grid_dy){
    fprintf(stderr,"Grid spacing different in X and Y. I cannot cope.\n");
    exit(-1);
  }

  //
  // Construe the output filename and open it.
  // The output file name is declared in Mdv2Axf.hh so that
  // it can be used in the destructor to copy the file
  // to its latest version.
  //
  date_time_t dataTime;

  dataTime.unix_time = t;
  uconvert_from_utime( &dataTime );

  sprintf(_OutFileName,"%s/%s_%s_%02d_%4d%02d%02d%02d%02d%02d.axf",
	  P->OutDir, P->BaseAxfName,
	  P->VarName,P->PlaneNumber,
	  dataTime.year, dataTime.month, dataTime.day,
	  dataTime.hour, dataTime.min, dataTime.sec);
  AxfFp = fopen(_OutFileName,"wt");
  if (AxfFp == NULL){
    cerr << "Failed to open " << _OutFileName << endl;
    exit(-1);
  }


  //
  // Write the descriptive block.
  //
  fprintf(AxfFp,"%s\n","[DESCRIPTION]");
  fprintf(AxfFp,"%s\n","system[30]=\"ANC\"");
  fprintf(AxfFp,"product[%d]=\"%s\"\n",
	  strlen(P->VarName),P->VarName);
  fprintf(AxfFp,"%s\n","radar[30]=\"Kurnell\"");

  fprintf(AxfFp,"aifstime[30]=\"%4d%02d%02d%02d%02d%02d\"\n",
	  dataTime.year, dataTime.month, dataTime.day,
	  dataTime.hour, dataTime.min, dataTime.sec);
  //
  // Get the lat,lon of the NW corner of the grid.
  // Also get the grid spacing units while we're at it.
  //
  double nw_lat, nw_lon;
  char gridSpacingUnits[30];

  switch(Fhdr.proj_type){

  case Mdvx::PROJ_LATLON :
    sprintf(gridSpacingUnits,"%s","deg");
    nw_lat = Fhdr.grid_miny + Fhdr.ny * Fhdr.grid_dy;
    nw_lon = Fhdr.grid_minx;
    break;

  case Mdvx::PROJ_FLAT :
    double Dx, Dy;
    sprintf(gridSpacingUnits,"%s","km");
    Dy = Fhdr.grid_miny + Fhdr.ny * Fhdr.grid_dy;
    Dx = Fhdr.grid_minx;
    PJGLatLonPlusDxDy(Fhdr.proj_origin_lat, Fhdr.proj_origin_lon,
		      Dx, Dy, &nw_lat, &nw_lon); 

    break;


  default :

    fprintf(stderr,"Unknown projection.\n");
    exit(-1);
    break;

  }

  fprintf(AxfFp,"nw_corner_lat=%g\n",nw_lat);
  fprintf(AxfFp,"nw_corner_lon=%g\n",nw_lon);
  fprintf(AxfFp,"grid_spacing_units=\"%s\"\n",gridSpacingUnits);
  fprintf(AxfFp,"grid_spacing=%g\n",Fhdr.grid_dx);
  fprintf(AxfFp,"x_dimension=%d\n",Fhdr.nx);
  fprintf(AxfFp,"y_dimension=%d\n",Fhdr.ny);
  //
  // The units are specified in the parameter file rather than
  // in the MDV file. This is unfortunate since it means they are
  // specified in two places - however, given that multiple fields
  // are being processed it is unclear as to which field should
  // be used to specify the units.
  //
  fprintf(AxfFp,"units=\"%s\"\n",P->Units);
  //
  fprintf(AxfFp,"[$]\n\n");


  //
  // End of descriptive section - Add the Grid section.
  //
  fprintf(AxfFp,"[GRID]\ngrid[30]\n");
  for (int i=0; i < P->FieldNames_n; i++){
    fprintf(AxfFp,"\"GRID%d\"\n",i);
  }

  fprintf(AxfFp,"[$]\n\n");

  return 0;
}


//
// Process data. Just loop through, writing output file.
// Missing/bad data are replaced with the value 
// specified in the runtime parameters.
//
int Mdv2Axf::Process(time_t t, MdvxField *field, 
		     int GridNum, int PlaneNum, float MissingVal, 
		     float ConversionFactor, char *FieldName){

  Mdvx::field_header_t Fhdr = field->getFieldHeader(); 
  float *data = (float *) field->getVol();
  float  value;

  fprintf(AxfFp,"[GRID%d]\n",GridNum);
  fprintf(AxfFp,"%s[%d]\n",FieldName, Fhdr.nx);

  int NumGood=0, NumTot=0;
  float min,max;
  int first=1;
  for(int j=Fhdr.ny-1; j > -1; j--){
    fprintf(AxfFp,"[");
    for(int i=0; i < Fhdr.nx; i++){

      int index = PlaneNum*Fhdr.ny*Fhdr.nx +
	j*Fhdr.nx + i;

      NumTot++;
      value = data[index];

      if ((value == Fhdr.bad_data_value) ||
	  (value == Fhdr.missing_data_value)){
	fprintf(AxfFp,"%g",MissingVal);
      } else {
	NumGood++;
        value *= ConversionFactor;
	fprintf(AxfFp,"%g",value);
	if (first){
	  first=0;
	  min=value; max=min;
	} else {
	  if (value < min) min = value;
	  if (value > max) max = value;
	}
      }
      if (i != Fhdr.nx-1) fprintf(AxfFp,",\t");
    }
    fprintf(AxfFp,"]\n");
  }
  fprintf(AxfFp,"[$]\n\n");
  if (NumGood > 0){
    fprintf(stdout,"%s range from %g to %g (percent good %g)\n",
	    Fhdr.field_name, min, max, 100.0*float(NumGood)/float(NumTot));
  } else {
    fprintf(stdout,"All data missing for field %s\n",Fhdr.field_name);
  }

  return 0;
}

//
// Close the output file and copy it to the latest file name.
//

int Mdv2Axf::CloseOutput(){


  fclose(AxfFp);


  //
  // Do a sequence of file moves to get ready for the latest output.
  //

  char command[2*MAX_PATH_LEN+10];
  for(int k= _NumLatestFiles; k > 1  ;k--){
    char FNameA[MAX_PATH_LEN], FNameB[MAX_PATH_LEN];

    sprintf(FNameA,"%s_%d.axf",_LatestFileName,k-1);
    sprintf(FNameB,"%s_%d.axf",_LatestFileName,k);
    sprintf(command,"\\mv  %s %s",FNameA, FNameB);
 
    system(command);

  }

  //
  // Copy the file into the latest output file name
  // specified. This is done with calls to system which
  // is not ideal.
  //
  char TempFileName[MAX_PATH_LEN];

  sprintf(TempFileName,"%s.tmp",_LatestFileName);
  //
  // Copy the file across to a temporary file and then
  // move that file into place as the latest file (a move
  // being faster than a copy, this may avoid file lock issues).
  //
  // This is done with system calls, which is not great, but not
  // (I think) awful either.
  //
  sprintf(command,"\\cp %s %s", _OutFileName, TempFileName);
  if (system(command)){
    cerr << "Command " << command << " failed." << endl;
    exit(-1);
  }                          

  char Target[MAX_PATH_LEN];
  sprintf(Target,"%s_%d.axf",_LatestFileName,1);
 
 sprintf(command,"\\mv %s %s", TempFileName, Target);
  if (system(command)){
    cerr << "Command " << command << " failed." << endl;
    exit(-1);
  }
     

  return 0;
}



