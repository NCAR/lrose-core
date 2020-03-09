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
//
//
// Program to print out what percentage of an MDV file has bad or missing
// data, and give the data min and max if there are any good data.
//
// Use it like this :
//
// MdvxHowBad blah.mdv
//
// This is a simplistic diagnostic so it's not coded all that neatly.
// It is a reasonable example of how MDV data are read, however.
//
// Niles Oien

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>  
#include <cstdio>
#include <cstdlib>
#include <math.h>
using namespace std;

int main(int argc, char *argv[])
{

  //
  // Get the input MDV filename from the command line.
  //
  char FileName[1024];
  float val,min,max;
  double data_total,mean,var,sumsqr;
  int i,j,k,pbad,first;
  long numbad,numgood,total; 

  // See if we got a filename on the input line.
  // if not, ask for one.
  // 
  if (argc > 1){
    sprintf(FileName,"%s", argv[1]);
  } else {
    fprintf(stdout,"Input file name --->");
    fscanf(stdin,"%s",FileName);
  }

  //
  // Instantiate an MDV object.
  //
  DsMdvx In;

  //
  // Set the read path. This is a rather unusual way to read
  // MDV data, more typically a client will specify a URL to
  // read the data from and the time for which we want to read
  // data, with a temporal tolerance, and read the data most
  // appropriate for that time. But that's not what we're doing
  // in this little diagnostic.
  //
  In.setReadPath(FileName);

  //
  // Set the encoding type to be floating point numbers.
  // Other options include scaled bytes, in which case the
  // phsical value is opbtained by multiplying the bytes by a scale and
  // adding an offset. Scaled integers are also an option.
  //
  In.setReadEncodingType(Mdvx::ENCODING_FLOAT32);

  //
  // Make sure we get uncompressed data.
  //
  In.setReadCompressionType(Mdvx::COMPRESSION_NONE);
         
  //
  // OK - we're ready to do the read. Whatever data is in the
  // MDV file on disk, it will now be converted to floating point
  // uncompressed data in the DsMdvx object in memory.
  //
  if (In.readVolume()){
    fprintf(stderr,"Failed to open %s\n",FileName);
    exit(-1);
  }

  //
  // An MDV file consists of one master header followed by
  // several fields. Each field has a field header and a vlevel
  // header. The field header contains information pertinent to
  // that field and the vlevel header (which I don't think we
  // even consider here) has information about the vertical heights
  // of the planes. 
  //
  // Get the master header from the DsMdvx object so we can loop through
  // the fields.
  //
  Mdvx::master_header_t InMhdr = In.getMasterHeader();     

  for (i=0; i < InMhdr.n_fields; i++){

    MdvxField *InField;

    //
    // Get the field by its field number. It's more common to get
    // fields by their name, but that's not what we're doing here.
    //
    InField = In.getFieldByNum( i );

    if (InField == NULL){
      fprintf(stderr,"Field %d not there.\n",i);
      exit(-1);
    }

    //
    // Get the field header from this field object. The vlevel header
    // could be obtained in a similar way.
    //
    Mdvx::field_header_t InFhdr = InField->getFieldHeader();   

    //
    // Start printing statistics about the data. Not much more
    // after this point pertains to MDV data reading.
    //

    fprintf(stdout,"Field %d : %s (Units %s ) %ld by %ld by %ld\n",
	    i+1,InFhdr.field_name,InFhdr.units,
	    InFhdr.nx,InFhdr.ny,InFhdr.nz);

    // Look at the actual data. MDV data is stored as
    //  a series of two dimensional planes (very often there
    // is only one plane). Loop through the 
    // planes (the z direction) and then
    // for each plane, loop through the data (the x and y
    // directions). Record minimum, max and percent bad
    // for each plane, and print out. 

    for (j=0; j< InFhdr.nz; j++){ // Loop through planes.

      total = InFhdr.nx * InFhdr.ny; // Total number of data points.
      numbad = 0; numgood=0;
      data_total=0.0;
      first=1;

      //
      // Get access to the floating point data.
      //
      float *data = (float *) InField->getVol();

      for(k=0; k<total;k++){ // Loop through the data in x and y 

	int index = k + j * total;

	if ((data[index] ==  InFhdr.missing_data_value ) ||
	    (data[index] ==  InFhdr.bad_data_value )){

	  // Bad data.
	  numbad ++;

	} else {

	  numgood++;

	  // Good data - decode the byte into a physical value,
	  // using the scale and bias. 

	  val = data[index];
	  data_total = data_total + val;
	  if (first) {
	    min=val; max=val; first=0;
	  } else {
	    if (val < min) min = val;
	    if (val > max) max = val;
	  }
	}
      }

      // Calculate the percent bad and print it.
      pbad=(int)(100.0*((double)(numbad) / (double)(total)));
      fprintf(stdout,"\tPlane %d : %ld of %ld are bad or missing (%d percent)\n",
	      j+1,numbad,total,pbad);

      // If the field was not 100% bad, print the min and max.
      if (!(first)){
	fprintf(stdout,"\tData values range from %g to %g\n",
		min,max);

	mean = data_total/numgood;



	// Take another hit at the data to get the variance. 

	sumsqr=0.0;
	for(k=0; k<total;k++){ /* Loop through the data in x and y */

	  int index = k + j * total;

	  if ((data[index] != InFhdr.missing_data_value ) &&
	      (data[index] != InFhdr.bad_data_value )){

	    val = data[index];
	    sumsqr = sumsqr + (val-mean)*(val-mean);

	  }
	}

	var = sumsqr / numgood;
	
	fprintf(stdout,"\tMean : %g Variance : %g Standard Deviation : %g\n",
		mean,var,sqrt(var));
      }


    } // End of loop through data for a plane.

  } // End of loop through planes. 

  // All done. 

  return (0);

}








