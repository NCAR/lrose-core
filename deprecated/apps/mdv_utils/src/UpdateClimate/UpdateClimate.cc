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

#include <cstdio>
#include <ctime>
#include <cstdlib> 
#include <didss/LdataInfo.hh>

#include <cstdlib>
#include <string.h>
#include <toolsa/umisc.h>
#include <math.h>

#include <mdv/mdv_write.h>
#include <mdv/mdv_utils.h>
#include <mdv/mdv_user.h>   
#include <mdv/mdv_read.h>
#include <toolsa/pmu.h>

#include "Params.hh"
using namespace std;

// File scope.
void HB(char *s){ // Heartbeat.
  PMU_auto_register(s);
}

LdataInfo *L; // Global so that we can delete it in tidy_and_exit

static void tidy_and_exit (int sig);

main(int argc, char *argv[])
{

  Params *P = new Params();

  if (P->loadFromArgs(argc,argv,NULL,NULL)){
    fprintf(stderr,"Specify params file with -params option.\n");
    exit(-1);
  }


  if (getenv("CLEAR_SKY_CLIMATOLOGY_DIR")==NULL){
    fprintf(stderr,"Please define CLEAR_SKY_CLIMATOLOGY_DIR\n");
    exit(-1);
  }


  // Trap.
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);


  PMU_auto_init("UpdateClimate",
		P->Instance,PROCMAP_REGISTER_INTERVAL);

  L = new LdataInfo(P->InDir);
                                                                
  if (!(L->isOK())){
    cerr << "Failed to make LdataInfo object - check input directory?\n";
    exit(-1);
  }

  while(1){
    L->readBlocking(-1,P->Strobe*1000, HB );

    MDV_handle_t InHandle;
    MDV_init_handle(&InHandle);

    // Put together the input file name.
    char InputFile[MAX_PATH_LEN];

    date_time_t MdvTime;
    MdvTime=L->getLatestTimeStruct();

    if (P->InDir[strlen(P->InDir)-1] == '/'){
      sprintf(InputFile,"%s%d%02d%02d/%02d%02d%02d.mdv",
	      P->InDir,MdvTime.year,MdvTime.month,
	      MdvTime.day, MdvTime.hour, MdvTime.min,
	      MdvTime.sec);
    } else {
      sprintf(InputFile,"%s/%d%02d%02d/%02d%02d%02d.mdv",
	      P->InDir,MdvTime.year,MdvTime.month,
	      MdvTime.day, MdvTime.hour, MdvTime.min,
	      MdvTime.sec);
    }
  
    // Read the file.
    if(MDV_read_all(&InHandle,InputFile,MDV_INT8)){
      fprintf(stderr,"Failed to read %s.\n",InputFile);
      exit(-1);
    }  

    //
    // Futz around with the data time to get
    // the climate time.
    //
    date_time_t DataTime, ClimateTime;

    DataTime.unix_time=InHandle.master_hdr.time_centroid;
    uconvert_from_utime(&DataTime);

    ClimateTime=DataTime;
    ClimateTime.year=1990; ClimateTime.day=15;
    ClimateTime.min=30; ClimateTime.sec=0;
    ClimateTime.unix_time=uconvert_to_utime(&ClimateTime);

    //
    // Read the climate data.
    //
    char ClimateDir[MAX_PATH_LEN], ClimateFile[MAX_PATH_LEN];
    sprintf(ClimateDir,"%s",getenv("CLEAR_SKY_CLIMATOLOGY_DIR"));
    if (ClimateDir[strlen(ClimateDir)-1] == '/'){
      sprintf(ClimateFile,"%s%d%02d%02d/%02d%02d%02d.mdv",
	      ClimateDir,ClimateTime.year,ClimateTime.month,
	      ClimateTime.day, ClimateTime.hour, ClimateTime.min,
	      ClimateTime.sec);
    } else {
      sprintf(ClimateFile,"%s/%d%02d%02d/%02d%02d%02d.mdv",
	      ClimateDir,ClimateTime.year,ClimateTime.month,
	      ClimateTime.day, ClimateTime.hour, ClimateTime.min,
	      ClimateTime.sec);
    }


    fprintf(stdout,"Climate file is %s\n",ClimateFile);

    MDV_handle_t CliHandle;
    MDV_init_handle(&CliHandle);
  
    // Read the file.
    if(MDV_read_all(&CliHandle,ClimateFile,MDV_INT8)){
      fprintf(stderr,"Failed to read climate file %s.\n",ClimateFile);
      exit(-1);
    }  


    //
    // Go through all the fields and update them.
    //

    int MaxSize;
    MaxSize=InHandle.master_hdr.max_nx*InHandle.master_hdr.max_ny;

    float *InField, *CliField;
    float BadData=10000.0;

    InField= (float *)malloc(MaxSize*sizeof(float));
    CliField=(float *)malloc(MaxSize*sizeof(float));


    MDV_field_header_t *Fhdr = InHandle.fld_hdrs;
    MDV_field_header_t *CFhdr = CliHandle.fld_hdrs;


    for (int f=0; f<InHandle.master_hdr.n_fields; f++){

      int Size = (Fhdr+f)->nx * (Fhdr+f)->ny;
      int Updates=0;

      // 
      // See if we are looking at albedo or temperature.
      //
      float val;
      float Scale,Bias;

      /* Now, use the field code.
      if (!(strcmp((Fhdr+f)->field_name,"temperature"))){
	val=1.0; // default temperature for init.
	Bias=-63.5; Scale=0.5;
      } else {
	val=-1.0;   // default albedo for init.
	Bias=0.0; Scale=0.4;
      }
      */


      switch ((Fhdr+f)->field_code){

      case 11 : // Temperature
	val=1.0;
	Bias=-63.5; Scale=0.5;
	break;

      case 84 : // Albedo
	val=-1.0;
	Bias=0.0; Scale=0.4;
	break;

      default :
	fprintf(stderr,"Unrecognised field code %d\n", 
		(Fhdr+f)->field_code );
	exit(-1);
	break;


      }


      unsigned char *b,*c;
      b=(unsigned char *)InHandle.field_plane[f][0];
      c=(unsigned char *)CliHandle.field_plane[f][0];


      // Unwrap into floating point numbers.

      for (int q=0; q<Size; q++){
	if ((b[q]!=(Fhdr+f)->bad_data_value) &&
	    (b[q]!=(Fhdr+f)->missing_data_value)){
	  InField[q]=(Fhdr+f)->scale*b[q] + (Fhdr+f)->bias;
	} else {
	  InField[q]=BadData;
	}	

	if ((c[q]!=(CFhdr+f)->bad_data_value) &&
	    (c[q]!=(CFhdr+f)->missing_data_value)){
	  CliField[q]=(CFhdr+f)->scale*c[q] + (CFhdr+f)->bias;
	} else {
	  CliField[q]=BadData;
	}
      
      }

      // In the input field, replace albedos greater 
      // than 70% and temperatures with
      // absolute values greater than 55 Celcius with a bad value.

      for (int q=0; q<Size; q++){

	switch ((Fhdr+f)->field_code){

	case 11 : // Temperature
	  if (fabs(InField[q]) > 55.0) InField[q]=BadData;
	  break;

	case 84 : // Albedo
	  if ((InField[q] < 0.0) || (InField[q] > 70.0)) InField[q]=BadData;
	  break;

	default :
	  fprintf(stderr,"Unrecognised field code %d\n", 
		  (Fhdr+f)->field_code );
	  exit(-1);
	  break;

	}

      }

      // Do the comparison.
      for(int q=0;q<Size;q++){
	  if ((CliField[q]*val < InField[q]*val) || (CliField[q]==BadData)){
	    if (InField[q]!=BadData){
	      CliField[q]=InField[q];
	      Updates++;
	    }
	  }

      }

      //
      // Scale the field back into bytes.
      //



      fprintf(stdout,"%d updates for field %d : %s %s (%d percent)\n",
	      Updates,f,(Fhdr+f)->field_name_long,
	      (Fhdr+f)->field_name,
	      int(100.0*float(Updates)/float(Size)));


      (CFhdr+f)->scale=Scale;
      (CFhdr+f)->bias=Bias;
      (CFhdr+f)->bad_data_value=255;
      (CFhdr+f)->missing_data_value=255;

      for (int k=0; k<Size; k++){
	int qw;
	qw=(int)rint(((CliField[k]- (CFhdr+f)->bias) / (CFhdr+f)->scale));
	if ((qw > -1) || (qw < 255)){
	 c[k]=(unsigned char)qw;
	} else {
	  c[k]=(unsigned char)(CFhdr+f)->bad_data_value;
	}
      }
    

    } // end of loop through fields.

    free(InField); free(CliField);

    int i = MDV_write_to_dir(&CliHandle, ClimateDir, MDV_PLANE_RLE8, TRUE);

    if (i!= MDV_SUCCESS){
      fprintf(stderr,"Failed to write data!\n");
    }

    MDV_free_handle(&InHandle);
    MDV_free_handle(&CliHandle);

  } // End of read blocking loop 

}

///////////////////////////////////////////////////////

static void tidy_and_exit (int sig){
  PMU_auto_unregister();
  exit(sig);
  delete L;
}
 









