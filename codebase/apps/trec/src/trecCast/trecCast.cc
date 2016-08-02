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

#include <mdv/mdv_read.h>
#include <mdv/mdv_write.h>
#include <stdio.h>
#include <math.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/pjg_flat.h>

#include "trecCast.hh"
#include "Params.hh"

trecCast::trecCast(char *FileName, Params *P)
{

  OK=1;
  MDV_handle_t Handle; 
  MDV_init_handle(&Handle);


  if(MDV_read_all(&Handle,FileName,MDV_INT8)){
    fprintf(stderr,"Failed to read %s.\n",FileName);
    OK=0;
  }

  //
  // Do some checks on the input MDV file.
  //

  if (Handle.master_hdr.n_fields != 3){
    fprintf(stderr,"WARNING : %s has %d fields, expected 3\n",
	    FileName, Handle.master_hdr.n_fields);
  }

  if (Handle.master_hdr.max_nz > 1){
    fprintf(stderr,"WARNING : %s has 3D data, I cannot cope.\n",
	    FileName);
    exit(-1);
  }

  if (Handle.master_hdr.field_grids_differ){
    fprintf(stderr,"WARNING : %s has differing field grids, I cannot cope.\n",
	    FileName);
    exit(-1);
  }

  if (Handle.master_hdr.grid_order_direction!=MDV_ORIENT_SN_WE){
    fprintf(stderr,"WARNING: %s has unsupported data order, I cannot cope.\n",
	    FileName);
    exit(-1);
  }
    

  MDV_field_header_t *Fhdr;
  Fhdr = Handle.fld_hdrs;
  

  MDV_handle_t& hr = Handle;
  const float Bad = -1000.0;

  float *F = GetField(hr ,0, Bad);
  float *U = GetField(hr ,1, Bad);
  float *V = GetField(hr ,2, Bad);

  Fhdr->forecast_delta = P->Time;
  Fhdr->forecast_time = Handle.master_hdr.time_centroid + P->Time;

  int num;
  if (P->Debug){
    num=0;
    for(int i=0; i< Fhdr->nx * Fhdr->ny; i++){
      if (U[i]==Bad) num++;
    }
    fprintf(stderr,"Interpolating U : Before : %d bad values\n",num);
  }
  Interp(U,Fhdr->nx, Fhdr->ny,P->SquareSize,Bad,P->WeightAtEdge);

  if (P->Debug){
    num=0;
    for(int i=0; i< Fhdr->nx * Fhdr->ny; i++){
      if (U[i]==Bad) num++;
    }
    fprintf(stderr,"After : %d bad values\n",num);

    num=0;
    for(int i=0; i< Fhdr->nx * Fhdr->ny; i++){
      if (V[i]==Bad) num++;
    }
    fprintf(stderr,"Interpolating V : Before : %d bad values\n",num);
  }
  Interp(V,Fhdr->nx, Fhdr->ny,P->SquareSize,Bad,P->WeightAtEdge);

  if (P->Debug){
    num=0;
    for(int i=0; i< Fhdr->nx * Fhdr->ny; i++){
      if (V[i]==Bad) num++;
    }
    fprintf(stderr,"After : %d bad values\n",num);
  }
 
  float *NewF = (float *)malloc(Fhdr->nx * Fhdr->ny *sizeof(float));
  if (NewF==NULL){
    fprintf(stderr,"Malloc failed.\n");
    exit(-1);
  }

  for (int k=0; k<Fhdr->nx * Fhdr->ny; k++) NewF[k]=Bad;

  for (int i=0; i<Fhdr->nx; i++){

    PMU_auto_register("Advecting");

    for(int j=0; j<Fhdr->ny; j++){
      if ((U[j*Fhdr->nx+i]!= Bad) &&
	  (V[j*Fhdr->nx+i]!= Bad)){

	const double MetersPerKm = 1000.0;

	double Dx = -1.0*U[j*Fhdr->nx+i] * P->Time / MetersPerKm;
	double Dy = -1.0*V[j*Fhdr->nx+i] * P->Time / MetersPerKm;
	
	int iDx,iDy,NewI,NewJ;
	switch (Fhdr->proj_type) {

	case MDV_PROJ_FLAT :
	  iDx = (int)rint(Dx/Fhdr->grid_dx);
	  iDy = (int)rint(Dy/Fhdr->grid_dy);
	  NewI = i +iDx;
	  NewJ = j +iDy;
	  break;

	case MDV_PROJ_LATLON :
	  double lat2,lon2;
          PJGLatLonPlusDxDy(j*Fhdr->grid_dy + Fhdr->grid_miny,
			    i*Fhdr->grid_dx + Fhdr->grid_minx,
			    Dx, Dy,
			    &lat2, &lon2);
	  NewI = (int)rint((lon2-Fhdr->grid_minx)/Fhdr->grid_dx);
	  NewJ = (int)rint((lat2-Fhdr->grid_miny)/Fhdr->grid_dy);
	  break; 

	default :
	  fprintf(stderr,"Usupported projection type.\n");
	  exit(-1);
	  break;

	}


	if ((NewI > -1)       && (NewJ > -1)        &&
	    (NewI < Fhdr->nx) && (NewJ < Fhdr->ny)  &&
	    (F[NewJ*Fhdr->nx+NewI]!=Bad)            &&
	    (F[NewJ*Fhdr->nx+NewI] >= P->MinThresh) &&
	    (F[NewJ*Fhdr->nx+NewI] <= P->MaxThresh)){
	  
	  NewF[j*Fhdr->nx + i]=F[NewJ*Fhdr->nx+NewI];
	}
      }
    }
  }

  PutField(hr, 0, Bad, NewF);
  PutField(hr, 1, Bad, U);
  PutField(hr, 2, Bad, V);

  free(F); free(U); free(V); free(NewF);

  if(MDV_write_to_dir(&Handle,P->OutputDir,MDV_INT8,1)!=MDV_SUCCESS){
    fprintf(stderr,"Failed to write output.\n");
    exit(-1);
  }


}

/*----------------------------------*/

float *trecCast::GetField(const MDV_handle_t& Handle, int fnum, float bad)
{

  MDV_field_header_t *Fhdr;
  Fhdr = Handle.fld_hdrs + fnum;

  float *field = (float *)malloc(Fhdr->nx * Fhdr->ny *sizeof(float));
  if (field==NULL){
    fprintf(stderr,"Malloc failed.\n");
    exit(-1);
  }

  unsigned char *databytes=(unsigned char *)(Handle.field_plane[fnum][0]);
   for (int i=0; i< Fhdr->nx * Fhdr->ny; i++){

     if ((databytes[i] == (unsigned char) Fhdr->missing_data_value ) ||
	 (databytes[i] == (unsigned char) Fhdr->bad_data_value )){    
       field[i]=bad;
     } else {
       field[i]=Fhdr->bias + databytes[i]*Fhdr->scale;
     }

  }

  return field;

}

/*---------------------------------*/

void trecCast::Interp(float *field, int nx, int ny, 
		      int InterpSquare, float Bad, double WeightAtEdge){

  
  // Make a local copy of the array.
   
  float *lfield = (float *)malloc(nx*ny*sizeof(float));
  if (lfield==NULL){
    fprintf(stderr,"Local malloc fail.\n");
    exit(-1);
  }

  for (int k=0; k<nx*ny; k++) lfield[k]=field[k];

  double k = -1.0*log(WeightAtEdge);
  for (int i=0;i<nx;i++){
    PMU_auto_register("Interpolating");
    for(int j=0; j<ny; j++){

      if (lfield[j*nx+i]==Bad){
	float total = 0.0, sumw=0.0;
	int num=0;
	for (int ii=i-InterpSquare; ii <= i+InterpSquare; ii++){
	  for (int jj=j-InterpSquare; jj <= j+InterpSquare; jj++){
	    if ((ii > -1) && (jj > -1) &&
		(ii < nx) && (jj < ny)){
	      if (lfield[jj*nx+ii]!=Bad){
		float d = sqrt((ii-i)*(ii-i) + (jj-j)*(jj-j));
		d=d/double(InterpSquare); // Normalize to 1 at edge
		float weight = exp(-d*k);
		sumw=sumw+weight;
		total=total + weight*lfield[jj*nx+ii];
		num++;
	      }
	    }
	  }
	}
	if (num > 0) field[j*nx+i]=total/sumw;
      }
    }
  }
  free(lfield);

}

/*--------------------------------*/

void trecCast::PutField(const MDV_handle_t& Handle, int fnum, 
	      float bad, float *field)
{

  MDV_field_header_t *Fhdr;
  Fhdr = Handle.fld_hdrs + fnum;


  unsigned char *databytes=(unsigned char *)(Handle.field_plane[fnum][0]);

   for (int i=0; i< Fhdr->nx * Fhdr->ny; i++){

     if (field[i]==bad){
       databytes[i] = (unsigned char) Fhdr->missing_data_value;
     } else {
       databytes[i] = (unsigned char) ((field[i]-Fhdr->bias)/Fhdr->scale);
     }

  }


}







