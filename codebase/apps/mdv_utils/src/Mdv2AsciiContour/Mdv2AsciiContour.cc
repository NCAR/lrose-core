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
#include <rapformats/GenPoly.hh>

#include <cstdlib>
#include <Spdb/DsSpdb.hh>

#include "Mdv2AsciiContour.hh"

#include "Params.hh"
using namespace std;

void clump_plane( ui08 *plane, int nx, int ny,
		  int threshold_byte, int min_npoints,
		  int Debug);

//
// Constructor. Does nothing.
//
Mdv2AsciiContour::Mdv2AsciiContour(){

}
 
//
// Destructor. Does nothing.
//
Mdv2AsciiContour::~Mdv2AsciiContour(){

}


//
// Process a field.
//
int Mdv2AsciiContour::Process(Mdvx::master_header_t Mhdr,
			 Mdvx::field_header_t Fhdr,
			 MdvxField *field, Params *params,
			 time_t t){

  if (Fhdr.nz > 1){
    cerr << "3D data - I cannot cope." << endl;
    return 0;
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
      if ((params->ThreshAbove) && (data[i] >= params->Thresh)){
	madeIt = true;
      }

      if ((!(params->ThreshAbove)) && (data[i] <= params->Thresh)){
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
  if (params->Clump){
    clump_plane( d, Fhdr.nx, Fhdr.ny,
		 32, params->MinClumpPixels, 0);
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
  // Open the output file.
  //
  char OutFile[1024];
  char TimeStr[64];
  date_time_t T;
  T.unix_time =  t;
  uconvert_from_utime( &T );
  
  sprintf(TimeStr,"%4d%02d%02d_%02d%02d%02d",
	  T.year, T.month, T.day, T.hour, T.min, T.sec);    
  
  if (ta_makedir_recurse(params->OutDir)){
    cerr << "Failed to create " << params->OutDir << endl;
    exit(-1);
  }

  sprintf(OutFile,"%s/CONT_%s_%s.dat",
	  params->OutDir,TimeStr,params->VarName);
  
  
  FILE *ofp = fopen(OutFile,"w");
  if (ofp == NULL){
    cerr << "Failed to open " << OutFile << endl;
    exit(-1);
  }
  //
  // Put the number of contours in the file.
  //
  fprintf(ofp,"NUM_CONTOURS : %d\n", num_clumps);


  //
  // Special case - if the number of contours is zero, and we
  // are outputting SPDB, then output a "nothing" struct so that
  // the data set updates.
  //
  if ((params->doSpdb) && (num_clumps == 0)){
    GenPoly nothing;
    GenPoly::vertex_t northPole;
    northPole.lat = 90; northPole.lon = 180.0;
    //
    // Add twice - must have at least two verticies.
    //
    nothing.addVertex( northPole );
    nothing.addVertex( northPole );
    nothing.setTime( t );
    nothing.setExpireTime( t + params->spdbExpiry );

    if (!(nothing.assemble())){
      cerr << nothing.getErrStr() << endl;
      exit(-1);
    }
    DsSpdb spdbMgr;
    //
    if (spdbMgr.put( params->spdbUrl,
		     SPDB_GENERIC_POLYLINE_ID,
		     SPDB_GENERIC_POLYLINE_LABEL,
		     1, t, t + params->spdbExpiry,
		     nothing.getBufLen(), nothing.getBufPtr(),
		     0 )){

                cerr << "Failed to write to SPDB." << endl;
                exit(-1);
              }

  }



  // Loop through all Clumps
     
  for(i=0; i < num_clumps; i++, clump_ptr++) {
    
     num_nodes = 4 * num_intervals;

    EG_alloc_nodes(num_nodes, &Nnodes_allocated,
		   &Bdry_list, &Bdry_pts, &Nodes);

    if (EG_bdry_graph(Rowh, Fhdr.nx, Fhdr.ny, Nodes, num_nodes, i+1)) {
      cerr << "ERROR - Mdv2AsciiContour::compute_bdry" << endl;
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

     fprintf(ofp,"CONTOUR %d SIZE %d\n", i+1, bdry_size);
     for  (int k=0; k< bdry_size; k++){
       fprintf(ofp,"%f\t%f\n",lat[k],lon[k]);
     }
     //
     // Output SPDB, if desired.
     //
     if (params->doSpdb){
       GenPoly G;
       GenPoly::vertex_t point;
       for  (int ik=0; ik< bdry_size; ik++){
	 point.lat = lat[ik];  point.lon = lon[ik]; 
	 G.addVertex( point );
       }
       //
       // Add the first point to the end to close the contour
       //
       point.lat = lat[0];  point.lon = lon[0]; 
       G.addVertex( point );
       //
       G.setTime( t );
       G.setExpireTime( t + params->spdbExpiry );
       //
       if (!(G.assemble())){
	 cerr << "Problem " << G.getErrStr() << endl;
	 exit(-1);
       }
       DsSpdb spdbMgr;
       //
       if (spdbMgr.put( params->spdbUrl,
			SPDB_GENERIC_POLYLINE_ID,
			SPDB_GENERIC_POLYLINE_LABEL,
			i+1, t, t + params->spdbExpiry,
			G.getBufLen(), G.getBufPtr(),
			0 )){

	 cerr << "Failed to write to Spdb." << endl;
	 exit(-1);
       }
     }
     free(lat); free(lon);
  }
  
  free(d); fclose(ofp);
     
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








