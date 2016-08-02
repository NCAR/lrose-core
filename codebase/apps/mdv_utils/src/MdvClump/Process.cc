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
#include <Mdv/MdvxChunk.hh>
#include <math.h>
#include <toolsa/str.h>
#include <toolsa/pjg_flat.h>
#include <euclid/boundary.h>
using namespace std;


void clump_plane( ui08 *plane, int nx, int ny,
		  int threshold_byte, int min_npoints,
		  int max_npoints, int Debug);
//
// Constructor
//
Process::Process(){
  OutputUrl = (char *)NULL;
}

////////////////////////////////////////////////////
//
// Main method - process data at a given time.
//
int Process::Derive(Params *P, time_t T){

  OutputUrl = STRdup(P->OutUrl);

  if (P->Debug){
    cerr << "Data at " << utimstr(T) << endl;
  }


  //
  // Set up for the new data.
  //
  DsMdvx New;


  New.setDebug( P->Debug);

  New.setReadTime(Mdvx::READ_FIRST_BEFORE, P->TriggerUrl, 0, T);
  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);


  if (P->RemapGrid){
    switch ( P->grid_projection){

    case Params::FLAT:
      New.setReadRemapFlat(P->grid_nx, P->grid_ny,
			 P->grid_minx, P->grid_miny,
			 P->grid_dx, P->grid_dy,
 			 P->grid_origin_lat, P->grid_origin_lon,
			 P->grid_rotation);

      break;
                                   
    case Params::LATLON:
      New.setReadRemapLatlon(P->grid_nx, P->grid_ny,
			   P->grid_minx, P->grid_miny,
			   P->grid_dx, P->grid_dy);

      break;            

    case Params::LAMBERT:
      New.setReadRemapLc2(P->grid_nx, P->grid_ny,
			P->grid_minx, P->grid_miny,
			P->grid_dx, P->grid_dy,
			P->grid_origin_lat, 
			P->grid_origin_lon,
			P->grid_lat1,  P->grid_lat2);
      
      break;
      
    default:
      cerr << "Unsupported projection." << endl;
      return -1;
      break;
      
    }               
  }

  if (New.readVolume()){
    cerr << "Read failed at " << utimstr(T) << " from ";
    cerr << P->TriggerUrl  << endl;
    return -1;
  }     

  //
  // Set up the existing data - which we will have to write if
  // it does not exist.
  //
  

  //
  // Set up the output.
  //
  Mdvx::master_header_t InMhdr = New.getMasterHeader();
  Mdvx::master_header_t OutMhdr = InMhdr;


  Out.setMasterHeader(OutMhdr);
  Out.clearFields();     

  //
  // Loop through the chunks
  //
  int nChunks = New.getNChunks();
     
  for( int i = 0; i < nChunks; i++ ) {
        
     //
     // Get the chunk from the input file
     //
     MdvxChunk *inChunk = New.getChunkByNum( i );
        

     //
     // Copy the chunk
     //
     MdvxChunk *outChunk = new MdvxChunk( *inChunk );
        

    //
    // Add it to the output file
    //
    Out.addChunk( outChunk );
        
  }
     

  //
  // Commence loop through fields.
  //
  for (int i=0; i< P->InFieldName_n; i++){
    //
    // Get the desired field.
    //
    MdvxField *InField;

    if (!(strncmp(P->_InFieldName[i],"#",1))){
      int Fnum = atoi(P->_InFieldName[i] + 1);
      InField = New.getFieldByNum( Fnum ); 
    } else {
      InField = New.getFieldByName( P->_InFieldName[i] );
    }

    if (InField == NULL){
      cerr << "New field " << P->_InFieldName[i] << " not found." << endl;
      return -1;
    }
    Mdvx::field_header_t InFhdr = InField->getFieldHeader();
    Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();

    fl32 *InData = (fl32 *)InField->getVol();

    //
    // If we have a polar projection, we need to add data wrapping
    // to handle clumps that appear on the 0/360 line.  This wrapping
    // needs to be big enough to detect the largest clump that is necessary
    // for removal
    //
    int wrapSize = 0;
    if( InFhdr.proj_type == Mdvx::PROJ_POLAR_RADAR ) {
       wrapSize = MAX( P->MinNumPoints, P->MaxNumPoints );
    }

    int numPlanePoints = InFhdr.nx * ( InFhdr.ny + 2*wrapSize );
    ui08 *b = (ui08 *)malloc(sizeof(ui08) * numPlanePoints);

    if (b == NULL){
      cerr << "Malloc failed." << endl;
      exit(-1);
    }

    //
    // Loop through the planes.
    //
    for (int iz=0; iz <InFhdr.nz; iz++){

      //
      // Put the plane into a byte array.
      // Missing/bad values are assigned byte 0, non-missing
      // are assigned 64 (arbitrary).
      //
      for (int ixy=0; ixy <InFhdr.ny*InFhdr.nx; ixy++){
	int index = iz*InFhdr.ny*InFhdr.nx + ixy;
	if (
	    (InData[index] == InFhdr.bad_data_value) ||
	    (InData[index] == InFhdr.missing_data_value)
	    ){
	  b[ixy + wrapSize*InFhdr.nx]=0;
	} else {
	  b[ixy + wrapSize*InFhdr.nx]=64;
	}
      }	     

      //
      // Do the data wrapping
      //
      for ( int iy = 0; iy < wrapSize; iy++ ) {
         for( int ix = 0; ix < InFhdr.nx; ix++ ) {
            b[iy*InFhdr.nx+ix] = b[(InFhdr.ny+iy)*InFhdr.nx+ix];
            b[(iy+InFhdr.ny+wrapSize)*InFhdr.nx+ix] = 
               b[(wrapSize+iy)*InFhdr.nx+ix];
         }
      }
      
      //
      // Clump on this.
      //   
      clump_plane( b, InFhdr.nx, InFhdr.ny + 2*wrapSize,
      	           32, P->MinNumPoints, P->MaxNumPoints, 
      	           P->Debug);    

      //
      // Put this back into the plane.
      //
      for (int ixy=0; ixy <InFhdr.nx * InFhdr.ny; ixy++){
	int index = iz*InFhdr.nx*InFhdr.ny + ixy;
	if (b[ixy+wrapSize*InFhdr.nx] < 64) {
           InData[index]=InFhdr.bad_data_value;
	}
      }
    } // End of loop through the planes.

    free(b);

    //
    // Do a loop to get the min,max. Useful as a diagnostic,
    // and allows us to set the bad and missing values of the
    // output.
    //
    int first = 1;
    double min=0.0,max=0.0,mean=0.0,total=0.0;
    long Numgood = 0;
    for(int k=0; k < InFhdr.nx*InFhdr.ny*InFhdr.nz; k++){
      if (
	  (InData[k] == InFhdr.missing_data_value) ||
	  (InData[k] == InFhdr.bad_data_value)
	  ) {
	continue;
      } else {
	Numgood++;
	total = total + InData[k];
	if (first){
	  min = InData[k];
	  max = min;
	  first = 0;
	} else {
	  if (InData[k] < min) min = InData[k];
	  if (InData[k] > max) max = InData[k];
	}
      }
    }
    
    if (Numgood == 0){
      if (P->Debug){
	cerr << "All output data are missing." << endl;
      }
    } else {
      mean = total / double(Numgood);
      fl32 newBad = max + 1.0;
      for(int k=0; k < InFhdr.nx*InFhdr.ny*InFhdr.nz; k++){
	if (
	    (InData[k] == InFhdr.missing_data_value) ||
	    (InData[k] == InFhdr.bad_data_value)
	    ) {   
	  InData[k] = newBad;
	}
      }

      InFhdr.missing_data_value = newBad;
      InFhdr.bad_data_value = newBad;

      if (P->Debug){
	cerr << "Data range from " << min << " to " << max;
	cerr << " with mean " << mean << endl;
      }
    }

    MdvxField *fld = new MdvxField(InFhdr, InVhdr, (void *)InData);
    
    fld->setFieldName(P->_OutFieldName[i]);
    fld->setFieldNameLong(P->_OutFieldName[i]);
    
    if (fld->convertRounded(Mdvx::ENCODING_INT8,
    			    Mdvx::COMPRESSION_ZLIB)){
      cerr << "convertRounded failed - I cannot go on." << endl;
      exit(-1);
    }  
    
    Out.addField(fld);
    
  } // End of loop through the fields.


  if (P->Debug){
    cerr << "Finished data processing for this time." << endl << endl;
  }

  return 0;

}



///////////////////////////////////////////////////
void clump_plane( ui08 *plane, int nx, int ny,
		  int threshold_byte, int min_npoints,
		  int max_npoints, int Debug)

{

  int i,j;		/* counters */
  int offset;		/* offset into data planes */
  int num_intervals = 0;
  int num_clumps = 0;
  Interval *intv_ptr = NULL;
  Clump_order *clump_ptr = NULL;

  static Row_hdr *Rowh = NULL;
  static int Nrows_alloc = 0;
  static Interval *Intervals = NULL;
  static int N_intervals_alloc = 0;

  static int N_ints_alloc_clump = 0;
  static Clump_order *Clumps = NULL;	/* a set of clumps */
  static Interval **Interval_order = NULL; 
  
  /*
   * Allocate Space for Row structs
   */

  EG_alloc_rowh(ny, &Nrows_alloc, &Rowh);
  
  /*
   * Find all intervals in each row
   */
  
  num_intervals = EG_find_intervals(ny,nx,plane,&Intervals,
				    &N_intervals_alloc,Rowh,
				    threshold_byte);
  
  /*
   * Allocate space for clump structs & make sure ID's are set to NULL_ID 
   */
  
  EG_alloc_clumps(num_intervals, &N_ints_alloc_clump,
		  &Clumps, &Interval_order);
  EG_reset_clump_id(Intervals, num_intervals);

  /*
   * Find all Clumps using the Intervals previously found
   */
  
  num_clumps = EG_rclump_2d(Rowh,ny,TRUE,1,Interval_order, Clumps);

  /*
   * 0th Clump is not used and Clumps array is actually num_clumps+1 big
   */

  clump_ptr = &(Clumps[1]);

  /*
   * Loop through all Clumps
   */
  
  int num_gone=0;
  for(i=0; i < num_clumps; i++, clump_ptr++) {
    
    if (
	(clump_ptr->pts < min_npoints) ||
	((max_npoints > 0) && (clump_ptr->pts > max_npoints))
	){
      num_gone++;
      /*
       * For small clumps set grid value to 
       * the threshold_byte
       */
      
      /*
       * Loop through each interval in clump
       */
      
      for(j=0; j < clump_ptr->size; j++) { 
	intv_ptr = clump_ptr->ptr[j];
	offset = (intv_ptr->row_in_plane * nx) + intv_ptr->begin;
	memset(plane + offset, threshold_byte, intv_ptr->len);
      }
    }

  } /* i */

  if (Debug) fprintf(stderr,"%d clumps removed.\n",num_gone);
  
}

////////////////////////////////////////////////////
//
// Destructor
//
Process::~Process(){
  //
  // Only do the write if fields were added.
  //

  Mdvx::master_header_t Mhdr = Out.getMasterHeader();

  if (Mhdr.n_fields > 0){
    if (Out.writeToDir(OutputUrl)) {
      cerr << "Failed to wite to " << OutputUrl << endl;
      exit(-1);
    }      
  }
  free(OutputUrl);
}










