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

#include <string>
#include <toolsa/pmu.h>
#include <euclid/boundary.h>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>

#include "Params.hh"
#include "FileHandler.hh"
#include "Region.hh"
#include "OutputFile.hh"
using namespace std;

///////////////////////////////////////////////////////

FileHandler::FileHandler()
{
  truthMdv=NULL; forecastMdv=NULL; outMdv=NULL;
  num_non=0; num_fail=0; num_false=0; num_success=0;

}

///////////////////////////////////////////////////////

FileHandler::~FileHandler()
{
  delete truthMdv; // The truth is no longer operative.
  delete forecastMdv; delete outMdv;
}

///////////////////////////////////////////////////////

int FileHandler::ProcessPair(time_t ForecastTime,
			     time_t DataTime,
			     Params *P)
{


  truthMdv = new DsMdvx();
  forecastMdv = new DsMdvx();
  outMdv = new DsMdvx();

  //
  // Set the remapping.
  //
  switch (P->grid_projection){

  case Params::FLAT:
    truthMdv->setReadRemapFlat(P->grid_nx, P->grid_ny,
			       P->grid_minx, P->grid_miny,
			       P->grid_dx, P->grid_dy,
			       P->grid_origin_lat, P->grid_origin_lon,
			       P->grid_rotation);
    forecastMdv->setReadRemapFlat(P->grid_nx, P->grid_ny,
				  P->grid_minx, P->grid_miny,
				  P->grid_dx, P->grid_dy,
				  P->grid_origin_lat, P->grid_origin_lon,
				  P->grid_rotation);
    break;

  case Params::LATLON:
    truthMdv->setReadRemapLatlon(P->grid_nx, P->grid_ny,
				 P->grid_minx, P->grid_miny,
				 P->grid_dx, P->grid_dy);
    forecastMdv->setReadRemapLatlon(P->grid_nx, P->grid_ny,
				    P->grid_minx, P->grid_miny,
				    P->grid_dx, P->grid_dy);
    break;

  case Params::LAMBERT:
    truthMdv->setReadRemapLc2(P->grid_nx, P->grid_ny,
			      P->grid_minx, P->grid_miny,
			      P->grid_dx, P->grid_dy,
			      P->grid_origin_lat, P->grid_origin_lon,
			      P->grid_lat1,  P->grid_lat2);
    forecastMdv->setReadRemapLc2(P->grid_nx, P->grid_ny,
				 P->grid_minx, P->grid_miny,
				 P->grid_dx, P->grid_dy,
				 P->grid_origin_lat, P->grid_origin_lon,
				 P->grid_lat1,  P->grid_lat2);
    break;

  default:
    fprintf(stderr,"Unsupported projection.\n");
    return -1;
    break;

  }

  //
  // Get the appropriate fields
  //
  if (P->UseFieldNumbers || P->FieldNumbersMustMatch)
  {
    truthMdv->addReadField(P->TruthFieldNum);
    forecastMdv->addReadField(P->ForecastFieldNum);
  }
  else
  {
    truthMdv->addReadField(P->TruthFieldName);
    forecastMdv->addReadField(P->ForecastFieldName);
  }
  
  //
  // Set up compositing on the truth field if desired.
  //
  if (P->composite_truth){
    truthMdv->setReadComposite();
    truthMdv->setReadVlevelLimits(P->composite_min,
				  P->composite_max);
  }
  //
  // Set up compositing on the forecast field if desired.
  //
  if (P->composite_forecast){
    forecastMdv->setReadComposite();
    forecastMdv->setReadVlevelLimits(P->composite_min,
				     P->composite_max);
  }
  //
  // Do the reads.
  //

  forecastMdv->setReadTime(Mdvx::READ_CLOSEST,
		   P->ForecastDir,
		   0, // Search margin - set to 0 as t is exact file time.
		   ForecastTime );

  forecastMdv->setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  forecastMdv->setReadCompressionType(Mdvx::COMPRESSION_NONE);

  if (forecastMdv->readVolume()) {
    string Error(forecastMdv->getErrStr());
    fprintf(stderr,"%s\n", Error.c_str());
    return -1;
  }          

  if(P->Debug)
  {
      cerr << "ForecastTime = " << utimstr(ForecastTime) << endl;
      cerr << "Using forecast file <" << forecastMdv->getPathInUse() << ">\n";
  }

  truthMdv->setReadTime(Mdvx::READ_CLOSEST,
		   P->TruthDir,
		   P->Slop,
		   DataTime );

  truthMdv->setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  truthMdv->setReadCompressionType(Mdvx::COMPRESSION_NONE);

  if (truthMdv->readVolume()) {
    string Error(truthMdv->getErrStr());
    fprintf(stderr,"%s\n", Error.c_str());
    return(-1);
  }          

  if(P->Debug)
  {
      cerr << "TruthTime = " << utimstr(DataTime) << endl;
      cerr << "Using truth  file <" << truthMdv->getPathInUse() << ">\n";
  }

  //
  // Get the appropriate fields.
  //

  MdvxField *truthField, *forecastField;

  truthField = truthMdv->getFieldByNum(0);

  if (truthField == NULL){
    fprintf(stderr,"No truth field name %s\n",
	    P->TruthFieldName);
    return -1;
  }


  forecastField = forecastMdv->getFieldByNum(0);

  if (forecastField == NULL){
    fprintf(stderr,"No forecast field name %s\n",
	    P->ForecastFieldName);
    return -1;
  }


  //
  // Detect storms, and clump the truth field.
  //

  Mdvx::field_header_t truthFhdr = truthField->getFieldHeader();
  Mdvx::field_header_t forecastFhdr = forecastField->getFieldHeader();

  Mdvx::master_header_t truthMhdr = truthMdv->getMasterHeader();
  Mdvx::master_header_t forecastMhdr = forecastMdv->getMasterHeader();

  if (P->Debug){
    fprintf(stderr,"Truth data at %s\n",utimstr(truthMhdr.time_centroid));
  }


  if (P->FieldNumbersMustMatch &&
      strcmp(P->TruthFieldName, truthFhdr.field_name)){
    fprintf(stderr, "WARNING! Truth field %s in not field number %d\n",
	    P->TruthFieldName, P->TruthFieldNum);
    fprintf(stderr,"Instead I have %s\n",truthFhdr.field_name);
    return -1;
  }

  if (P->FieldNumbersMustMatch &&
      strcmp(P->ForecastFieldName, forecastFhdr.field_name)){
    fprintf(stderr, "WARNING! Forecast field %s in not field number %d\n",
	    P->ForecastFieldName, P->ForecastFieldNum);
    fprintf(stderr,"Instead I have %s\n",forecastFhdr.field_name);
    return -1;
  }

  // Remove clumps from the raw data, if requested.  The clumps are
  // removed in place so that the MdvxField object can still be used
  // in the thresholding, as before.  This should be okay since the
  // MdvxField object doesn't seem to be used anywhere after the
  // thresholding.

  if (P->ClumpTruthRaw)
  {
    // First, we have to convert the raw data to bytes since that's
    // what the clumping algorithm uses.

    truthField->convertType(Mdvx::ENCODING_INT8,
			    Mdvx::COMPRESSION_NONE,
			    Mdvx::SCALING_DYNAMIC);
    
    // Now, loop through the planes and remove the clumps

    Mdvx::field_header_t truth_field_hdr = truthField->getFieldHeader();
    int threshold_byte =
      (int)(((P->ClumpRawThreshold - truth_field_hdr.bias)
	      / truth_field_hdr.scale) + 0.5);
    
    truthField->setPlanePtrs();
    for (int z = 0; z < truth_field_hdr.nz; ++z)
      clump_plane((ui08 *)truthField->getPlane(z),
		  truth_field_hdr.nx, truth_field_hdr.ny,
		  threshold_byte, P->MinTruthClumpRawPixels, P->Debug);
    
    // Finally, convert the truth field back to floats

    truthField->convertType(Mdvx::ENCODING_FLOAT32,
			    Mdvx::COMPRESSION_NONE,
			    Mdvx::SCALING_NONE);
  }

  float *forecastDetected = Threshold((float *) forecastField->getVol(),
				      P,1,
				      forecastFhdr.bad_data_value,
				      forecastFhdr.missing_data_value);

  float *truthDetected = Threshold((float *) truthField->getVol(),
				   P,0,
				   truthFhdr.bad_data_value,
				   truthFhdr.missing_data_value);

  if (forecastDetected == 0 || truthDetected == 0)
    return -1;
  
  if (P->ClumpTruth)
  {
    if (Clumper(truthDetected,P) != 0)
      return -1;
  }
  
  //

  //
  // Blank out all but region of interest, if desired.
  //
  if (P->RegionSpecified){
    Region(P,
	   forecastDetected,
	   forecastFhdr,
	   forecastMhdr);

    Region(P,
	   truthDetected,
	   truthFhdr,
	   truthMhdr);

  }

   //
   // Generate contingency table.
   //
   float *contingencyTable = Contingency(truthDetected,
					 forecastDetected,
					 forecastFhdr.nx * forecastFhdr.ny * forecastFhdr.nz,
					 P);


   if (P->SuperImpose){
     for (int k=0; k < forecastFhdr.nx*forecastFhdr.ny*forecastFhdr.nz; k++){
       if ((forecastDetected[k] == ContingencyTrue) ||
	   (forecastDetected[k] == ContingencyRelaxed))
	 contingencyTable[k] = contingencyTable[k] + 10.0;


     }


   }


   //
   // Output it, if requested.
   //

    if (P->IntermediateGrids){

      Mdvx::master_header_t inMhdr = truthMdv->getMasterHeader();

      OutputFile *O = new OutputFile(DataTime,
 				    inMhdr,
 				    P,
 				    truthDetected,
 				    forecastDetected,
 				    contingencyTable,
				    ContingencyMissing); 

     delete O;

   }

  free(truthDetected); free(forecastDetected);
  free(contingencyTable);

   return 0;

}

/////////////////////////////////////////////////////

float *FileHandler::Threshold(float *Grid, Params *P, int IsForecast,
			      float Bad, float Missing){

  float lower_limit, upper_limit;
  int Relax, NumRelax, WindowSize;

  if (IsForecast){
    lower_limit=P->forecast_lower_limit; 
    upper_limit=P->forecast_upper_limit;
    Relax = P->ForecastRelax;
    NumRelax = P->ForecastNumRelax;
  } else {
    lower_limit=P->truth_lower_limit; 
    upper_limit=P->truth_upper_limit; 
    Relax = P->TruthRelax;
    NumRelax = P->TruthNumRelax;
  }

  WindowSize = (2*Relax+1)*(2*Relax+1);

  int nx = P->grid_nx;
  int ny = P->grid_ny;
  int nz = P->grid_nz;
  

  int Size = nx * ny * nz;

  float *t = (float *)malloc(Size*sizeof(float));
  if (t==NULL){
    fprintf(stderr,"Malloc failed in thresholding.\n");
    return 0;
  }

  float *d = Grid;

  for (int iz=0; iz < nz; iz++){
    for (int iy=0; iy < ny; iy++){
      for (int ix=0; ix < nx; ix++){

	int i;
	i = iz*ny*nx + iy*nx + ix;

	int NumFound = 0;
	int NumMissing = 0;


	  for (int ixx = ix - Relax; ixx <= ix + Relax; ixx++){
	    for (int iyy = iy - Relax; iyy <= iy + Relax; iyy++){
	      if ((iyy > -1) && (iyy < ny) &&
		  (ixx > -1) && (ixx < nx)) {

		int k =  iz*ny*nx + iyy*nx + ixx;

		if ((d[k] == Bad) || (d[k] == Missing)){
		  NumMissing++;
		  continue;
		}
	

		if ((d[k] >= lower_limit) && (d[k] <= upper_limit)){
		  NumFound++;
		}

	      } else { // Point is outside of grid, count as missing.
		NumMissing++;
	      }
	    }
	  }

	  t[i]=ContingencyFalse;

	  if (NumFound == WindowSize) {
	    t[i]=ContingencyTrue;
	  } else {
	    if (NumFound >= NumRelax){
	      t[i]=ContingencyRelaxed;
	    }
	  }
	  if (NumMissing == WindowSize){
	    // All data in the region were missing, so
	    // forget about it.
	    t[i]=ContingencyMissing;
	  }

	  //
	  // If a grid point is true, but the box
	  // is only part full, the user may
	  // want to count this as true, not relaxed.
	  //
	  if ((P->NotRelaxedIfTrue) &&
	      (d[i] >=lower_limit) && (d[i] <=upper_limit)){
	    t[i]=ContingencyTrue;
	  }

      }
    }
  }

  return t;

}

////////////////////////////////////////////////////////

float *FileHandler::Contingency(float *truthDetected, float *forecastDetected,
		   int Size, Params *P){


  float *c = (float *)malloc(Size*sizeof(float));
  if (c==NULL){
    fprintf(stderr,"Malloc failed in contingency table.\n");
    return 0;
  }


  for (int i=0; i<Size; i++){

    float truthDetectedVal, forecastDetectedVal;

    truthDetectedVal   =truthDetected[i];
    forecastDetectedVal=forecastDetected[i];

    //
    // Deal with detected clumps in the truth field.
    //
    if (truthDetectedVal == ContingencyClumped)
      truthDetectedVal = ContingencyFalse;

    //
    // Count missings as non-events if so specified.
    //
    if (P->CountMissing){

      if (truthDetectedVal == ContingencyMissing)
	truthDetectedVal = ContingencyFalse;

      if (forecastDetectedVal == ContingencyMissing)
	forecastDetectedVal = ContingencyFalse;
    }
    //
    // Check for missing data.
    //
    if ((truthDetectedVal == ContingencyMissing) ||
	(forecastDetectedVal == ContingencyMissing)){
      c[i]=Missing;
      continue;
    }

    if ((truthDetectedVal == ContingencyRelaxed) &&
	(!(P->ExtendedContingency))) {
      truthDetectedVal = ContingencyTrue;
      // Treat storms detected in relaxed mode simply as storms
      // in this case.
    }

    if ((forecastDetectedVal == ContingencyRelaxed) &&
	(!(P->ExtendedContingency))) {
      forecastDetectedVal = ContingencyTrue;
      // Treat storms detected in relaxed mode simply as storms
      // in this case.
    }


    if ((int)truthDetectedVal == (int)ContingencyFalse)
    {
      ///////// There was no storm, what was the forecast?
      if ((int)forecastDetectedVal == (int)ContingencyFalse)
      {
	num_non++;
	c[i]=NonEvent;
      }
      else if ((int)forecastDetectedVal == (int)ContingencyRelaxed)
      {
	num_non++;
	c[i]=NonEvent;
      }
      else if ((int)forecastDetectedVal == (int)ContingencyTrue)
      {
	num_false++;
	c[i]=FalseAlarm;
      }
      else
      {
	fprintf(stderr,"Weird forecast value!\n");
	return 0;
      }
    }
    else if ((int)truthDetectedVal == (int)ContingencyRelaxed)
    {
      ///////// There was a storm detected in relaxed mode
      if ((int)forecastDetectedVal == (int)ContingencyFalse)
      {
	num_non++;
	c[i]=NonEvent;
      }
      else if ((int)forecastDetectedVal == (int)ContingencyRelaxed)
      {
	num_success++;
	c[i]=Success;
      }
      else if ((int)forecastDetectedVal == (int)ContingencyTrue)
      {
	num_success++;
	c[i]=Success;
      }
      else
      {
	fprintf(stderr,"Weird forecast value!\n");
	return 0;
      }
    }
    else if ((int)truthDetectedVal == (int)ContingencyTrue)
    {
      /////////// There was a storm.
      //      switch ( (int) forecastDetectedVal) {

      if ((int)forecastDetectedVal == (int)ContingencyFalse)
      {
	num_fail++;
	c[i]=Failure;
      }
      else if ((int)forecastDetectedVal == (int)ContingencyRelaxed)
      {
	num_success++;
	c[i]=Success;
      }
      else if ((int)forecastDetectedVal == (int)ContingencyTrue)
      {
	num_success++;
	c[i]=Success;
      }
      else
      {
	fprintf(stderr,"Weird forecast value!\n");
	return 0;
      }
    }
    else
    {
      fprintf(stderr,"Weird truth value: %f \nAt i= %d of %d\n",
	      truthDetectedVal,i,Size);
      return 0;
    }

  }

  return c;
}

/////////////////////////////////////////////////////////////////

int FileHandler::Clumper(float *t, Params *P)
{
  //
  // Have to loop through each plane, dumping to
  // a byte array.
  //

  ui08 *b = (ui08 *)malloc(P->grid_nx * P->grid_ny * sizeof(ui08));
  if (b==NULL){
    fprintf(stderr, "Malloc fail on clump.\n");
    return -1;
  }
  int PlaneSize = P->grid_nx * P->grid_ny;

  for (int iz=0; iz < P->grid_nz; iz++){
    float *f = t + iz*PlaneSize;
    for (int i=0; i < PlaneSize; i++){

      b[i]=255;
      if (f[i] == ContingencyMissing) b[i] = 0;
      if (f[i] == ContingencyFalse  ) b[i] = 1;
      if (f[i] == ContingencyRelaxed) b[i] = 3;
      if (f[i] == ContingencyTrue   ) b[i] = 4;

      if (b[i] == 255){
	fprintf(stderr,"Unrecognised code in float array : %g\n",
		f[i]);
	return -1;
      }
    }
    //
    // OK - have byte array b - get rid of clumps.
    //
    clump_plane( b, P->grid_nx, P->grid_ny,
		 2, P->MinTruthClumpPixels, P->Debug);

    //
    // Unfold back into float.
    //

    for (int i=0; i < PlaneSize; i++){

      f[i]=100.0;
      if (b[i] == 0) f[i] = ContingencyMissing;
      if (b[i] == 1) f[i] = ContingencyFalse;
      if (b[i] == 2) f[i] = ContingencyClumped;
      if (b[i] == 3) f[i] = ContingencyRelaxed;
      if (b[i] == 4) f[i] = ContingencyTrue;

      if (f[i] > 99.0){
	fprintf(stderr,"Unrecognised code in byte array : %d\n",
		(int)b[i]);
	return -1;
      }
    }


  }

  free(b);

  return 0;
}


/////////////////////////////////////////////////////


/*********************************************************************
 * clump()
 *
 * clump the data in the plane, setting
 * values for small clumps to just below the threshold.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 1997
 *
 * Mike Dixon
 *
 *********************************************************************/


void FileHandler::clump_plane( ui08 *plane, int nx, int ny,
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














