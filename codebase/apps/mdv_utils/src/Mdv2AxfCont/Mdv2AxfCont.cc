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

#include <iostream>
#include <euclid/boundary.h>
#include <euclid/geometry.h> 

#include <stdlib.h> // For system calls to move output file around.
 
#include "Mdv2AxfCont.hh"

#include "Params.hh"
using namespace std;

void clump_plane( ui08 *plane, int nx, int ny,
		  int threshold_byte, int min_npoints,
		  int Debug);

//
// Constructor. Does nothing.
//
Mdv2AxfCont::Mdv2AxfCont(){

}
 
//
// Destructor. Does nothing.
//
Mdv2AxfCont::~Mdv2AxfCont(){

}


//
// Process a field.
//
int Mdv2AxfCont::Process(Mdvx::master_header_t Mhdr,
			 Mdvx::field_header_t Fhdr,
			 MdvxField *field, Params *P,
			 time_t t){

  if (Fhdr.nz > 1){
    cerr << "3D data - I cannot cope." << endl;
    exit(-1);
  }

  float *data = (float *)field->getVol();

  //
  // Threshold the float data into a byte array. This is necessary
  // in order to call the Euclian Geometry (EG) routines to get
  // a contour.
  //
  unsigned char *d = (unsigned char *)malloc(Fhdr.nx * Fhdr.ny);
  if (d == NULL){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }

  float Missing = Fhdr.missing_data_value ;
  float Bad     = Fhdr.bad_data_value ;

  for (int i=0; i < Fhdr.nx * Fhdr.ny; i++){
    bool madeIt = false;    

    if ((data[i] != Missing) && (data[i] != Bad)){
      if ((P->ThreshAbove) && (data[i] >= P->Thresh)){
	madeIt = true;
      }

      if ((!(P->ThreshAbove)) && (data[i] <= P->Thresh)){
	madeIt = true;
      }
    }

    if (madeIt){
      d[i]=63; // Arbitrary value.
    } else {
      d[i]=0;
    }
  }

  //
  // Do some clumping on the plane, if desired.
  //
  if (P->Clump){
    clump_plane( d, Fhdr.nx, Fhdr.ny,
		 32, P->MinClumpPixels, 0);
  }

  //
  // Now that we have a byte array, call the EG routines to
  // get the contour in indicie space.
  //

  int i;  		/* counters */
  int num_intervals = 0;
  int num_clumps = 0;
  Clump_order *clump_ptr = NULL;
  
  static Row_hdr *Rowh = NULL;
  static int Nrows_alloc = 0;
  static Interval *Intervals = NULL;
  static int N_intervals_alloc = 0;

  static int N_ints_alloc_clump = 0;
  static Clump_order *Clumps = NULL;	/* a set of clumps */
  static Interval **Interval_order = NULL; 

  static int *Bdry_list = NULL;
  static Point_d *Bdry_pts = NULL;
  static Node *Nodes = NULL;
  static int Nnodes_allocated = 0;
  
  int num_nodes;
  int bdry_size;
  
  
  
  // Allocate Space for Row structs
  
  EG_alloc_rowh(Fhdr.ny, &Nrows_alloc, &Rowh);

  
  // Find all intervals in each row
  num_intervals = EG_find_intervals(Fhdr.nx, Fhdr.ny, d, &Intervals,
				    &N_intervals_alloc,Rowh,
				    1 );

  // Allocate space for clump structs & make sure ID's are set to NULL_ID 
     
  EG_alloc_clumps(num_intervals, &N_ints_alloc_clump,
		  &Clumps, &Interval_order);
  EG_reset_clump_id(Intervals, num_intervals);

  
  // Find all Clumps using the Intervals previously found
  
  num_clumps = EG_rclump_2d(Rowh,Fhdr.ny,1,1,Interval_order, Clumps);

  
  // 0th Clump is not used and Clumps array is actually num_clumps+1 big
  
  clump_ptr = &(Clumps[1]);

  //
  // If no clumps found, go on and write an empty file.
  // The program used to return in this event but an empty
  // file is needed to get the data to update.
  //
  // if (num_clumps == 0) return 0;
  
  //
  // Open the AXF file.
  //
  char OutFile[1024];
  char TimeStr[64];
  date_time_t T;
  T.unix_time =  t;
  uconvert_from_utime( &T );
  
  sprintf(TimeStr,"%4d%02d%02d%02d%02d%02d",
	  T.year, T.month, T.day, T.hour, T.min, T.sec);    
  
  sprintf(OutFile,"%s/ANC_CONT_%s_%s.axf",
	  P->OutDir,TimeStr,P->VarName);
  
  
  FILE *ofp = fopen(OutFile,"w");
  if (ofp == NULL){
    cerr << "Failed to open " << OutFile << endl;
    exit(-1);
  }
  
  //
  // Put the AXF header on the file.
  //
  fprintf(ofp,"[DESCRIPTION]\nsystem[30]=\"anc\"\n");
  fprintf(ofp,"product[30]=\"ANC contours of %s\"\n",P->VarName);
  fprintf(ofp,"radar[30]=\"Kurnell\"\n");
  fprintf(ofp,"aifstime[30]=\"%s\"\n",TimeStr);
  fprintf(ofp,"[$]\n\n");                    

  //
  // Put the descriptive header on the file.
  //
  fprintf(ofp,"[LINE]\nline[30]\n");
  for(int k=0; k < num_clumps; k++){
    fprintf(ofp,"\"CONTOUR_LINE_%d\"\n",k);
  }
  fprintf(ofp,"[$]\n\n");                    

  // Loop through all Clumps
     
  for(i=0; i < num_clumps; i++, clump_ptr++) {
    
 
    num_nodes = 4 * num_intervals;
      

    EG_alloc_nodes(num_nodes, &Nnodes_allocated,
		   &Bdry_list, &Bdry_pts, &Nodes);



    if (EG_bdry_graph(Rowh, Fhdr.nx, Fhdr.ny, Nodes, num_nodes, i+1)) {
      cerr << "ERROR - Mdv2AxfCont::compute_bdry" << endl;
      cerr << "Cannot compute boundary" << endl;
      exit(-1);
    }
    
    bdry_size = EG_traverse_bdry_graph(Nodes, 2L, Bdry_list);

      
    // generate the array of boundary points from the array of
    // boundary nodes and a bdry_list
 
     EG_gen_bdry(Bdry_pts, Nodes, Bdry_list, bdry_size);

     //
     // OK - indicies are now in Bdry_pts[k].x
     // and Bdry_pts[k].y as floats, for  k=0; k< bdry_size; k++
     //
     // Translate these to lat, lon space.
     //


     double *lon = (double *)malloc(sizeof(double)*bdry_size);
     double *lat = (double *)malloc(sizeof(double)*bdry_size);

     if ((lat == NULL) || (lon == NULL)){
       cerr << "Malloc failed on latlon points." << endl;
       exit(-1);
     }

     MdvxProj P(Mhdr, Fhdr);
     for  (int k=0; k< bdry_size; k++){

       P.xyIndex2latlon((int)rint(Bdry_pts[k].x),
			(int)rint(Bdry_pts[k].y),
			lat[k],lon[k]);

     }

     fprintf(ofp,"[CONTOUR_LINE_%d]\n",i);
     fprintf(ofp,"lat lon\n");
     for  (int k=0; k< bdry_size; k++){
       fprintf(ofp,"%lf\t%lf\n",lat[k],lon[k]);
     }
     fprintf(ofp,"[$]\n\n");

     free(lat); free(lon);
  }
  
  free(d); fclose(ofp);

  //
  // Do a sequence of file moves to get ready for the latest output.
  //

  char command[2*MAX_PATH_LEN+10];
  for(int k=P->NumLatestFiles; k > 1  ;k--){
    char FNameA[MAX_PATH_LEN], FNameB[MAX_PATH_LEN];

    sprintf(FNameA,"%s_%d.axf",P->LatestFileName,k-1);
    sprintf(FNameB,"%s_%d.axf",P->LatestFileName,k);
    sprintf(command,"\\mv  %s %s",FNameA, FNameB);
 
    system(command);

  }

  //
  // Copy the file into the latest output file name
  // specified. This is done with calls to system which
  // is not ideal.
  //
  char TempFileName[MAX_PATH_LEN];

  sprintf(TempFileName,"%s.tmp",P->LatestFileName);
  //
  // Copy the file across to a temporary file and then
  // move that file into place as the latest file (a move
  // being faster than a copy, this may avoid file lock issues).
  //
  // This is done with system calls, which is not great, but not
  // (I think) awful either.
  //
  sprintf(command,"\\cp %s %s", OutFile, TempFileName);
  if (system(command)){
    cerr << "Command " << command << " failed." << endl;
    exit(-1);
  }                          

  char Target[MAX_PATH_LEN];
  sprintf(Target,"%s_%d.axf",P->LatestFileName,1);
 
 sprintf(command,"\\mv %s %s", TempFileName, Target);
  if (system(command)){
    cerr << "Command " << command << " failed." << endl;
    exit(-1);
  }
     
  return 0;
  
}
////////////////////////////////////////////////////////////
//
//
void clump_plane( ui08 *plane, int nx, int ny,
		  int threshold_byte, int min_npoints,
		  int Debug)
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
    
    if (clump_ptr->pts < min_npoints) {
      num_gone++;
      /*
       * For small clumps set grid value to 0
       */
      
      /*
       * Loop through each interval in clump
       */
      
      for(j=0; j < clump_ptr->size; j++) { 
	intv_ptr = clump_ptr->ptr[j];
	offset = (intv_ptr->row_in_plane * nx) + intv_ptr->begin;
	memset(plane + offset, 0, intv_ptr->len);
      }
    }

  } /* i */

  if (Debug) fprintf(stderr,"%d clumps removed.\n",num_gone);
  
}








