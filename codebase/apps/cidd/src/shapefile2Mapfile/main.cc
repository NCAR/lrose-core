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

#include <shapelib/shapefil.h>
#include <toolsa/pjg_flat.h>
#include <Mdv/MdvxProj.hh>
#include <toolsa/file_io.h>
#include <toolsa/umisc.h>

#include "Params.hh"

int main(int argc, char *argv[]){

  //
  // Get the TDRP params.
  //
  Params P;

  if (P.loadFromArgs(argc,argv,NULL,NULL)){
    fprintf(stderr, "Specify params file with -params option.");
    return(-1);
  }  

  //
  // Make the output directory.
  //
  if (ta_makedir_recurse( P.outDir )){
    fprintf(stderr,"Failed to create directory %s\n", P.outDir);
    return -1;
  }

  //
  // Set up the projection object for labert conformal use.
  //
  MdvxProj Proj;

  Proj.initLc2(P.SPCS_parameters.originLat, 
	       P.SPCS_parameters.originLon, 
	       P.SPCS_parameters.trueLat1, 
	       P.SPCS_parameters.trueLat2);

  //
  // Loop thgrough the list of specified maps.
  //
  for (int imap=0; imap < P.mapFiles_n; imap++){

    fprintf(stderr,"Processing for %s\n",  P._mapFiles[imap]);

    //
    // Open the input and the output. If we fail to do
    // either of these, skip this map and continue to the next one.
    //
    SHPHandle SH;
    SHPObject *SO;

    char inFile[1024];
    sprintf(inFile,"%s/%s", P.inDir, P._mapFiles[imap]);

    if((SH = SHPOpen(inFile,"rb")) == NULL) {
      fprintf(stderr,"Failed to find %s\n", inFile);
      continue;
    }

    char outFile[1024];
    sprintf(outFile,"%s/%s%s.map", P.outDir,
	    P.generalDescriptor, P._mapFiles[imap]);

    FILE *ofp = fopen(outFile, "w");
    if (ofp == NULL){
      fprintf(stderr, "Failed to create %s\n", outFile);
      continue;
    }

    //
    // Define an icon at the start of the map file.
    //
    fprintf(ofp,"MAP_NAME %s%s\n#\n#\n", P.generalDescriptor, P._mapFiles[imap]);
    fprintf(ofp,"# Converted from shapefile %s by shapefile2Mapfile\n# %s UTC oien@ucar.edu\n#\n#\n", 
	    P._mapFiles[imap], utimstr(time(NULL)));
    fprintf(ofp,"ICONDEF CROSS 6\n0 -5\n0 5\n32767 32767\n-5 0\n5 0\n32767 32767\n#\n");

    int n_objects;
    int shape_type;

    SHPGetInfo(SH, &n_objects, &shape_type, NULL, NULL);

    //
    // Loop through the objects, which can be either lines or points (icons).
    //
    for(int i=0; i < n_objects; i++ ) {
   
      SO = SHPReadObject(SH,i);    // Load the shape object
   

      // One U.S. Survey Foot which is related to the meter by 1 m = 3.280833333333.... ft.
      // as opposed to 1m = 3.280839895 ft which is referred to as the International Foot

      const double metersToFeet = 3.280833333333;
      const double feetToStatuteMiles = 5280.0;
      const double statuteMilesToKm = 1.609;
      double xKm, yKm, lat, lon;

      switch(SO->nSHPType) {
     
      case SHPT_POLYGON:  // Polyline
      case SHPT_ARC:
      case SHPT_ARCM:
      case SHPT_ARCZ:

	fprintf(ofp,"POLYLINE fromShapeObject_%d %d\n", i+1, SO->nVertices);

	for (int j=0; j < SO->nVertices; j++) {
	  //
	  // Note that while the origin Easting and Northing are in meters, the
	  // co-ordinates themselves are in feet, and our internal xy2latlon
	  // systems need kilometers. So there are some conversions needed.
	  // All this took quite a while to work out.
	  //
	  //
	  xKm = statuteMilesToKm*(SO->padfX[j] - metersToFeet*P.SPCS_parameters.originEastingMeters)/feetToStatuteMiles;
	  yKm = statuteMilesToKm*(SO->padfY[j] - metersToFeet*P.SPCS_parameters.originNorthingMeters)/feetToStatuteMiles;

	  xKm += P.xOffset/1000.0;
	  yKm += P.yOffset/1000.0;

	  Proj.xy2latlon(xKm, yKm, lat, lon);
 
	  fprintf(ofp,"%lf\t%lf\n", lat, lon);

	}
	fprintf(ofp,"#\n");
	break;

      case SHPT_POINT :  // Icon Instance
      case SHPT_POINTZ:
      case SHPT_POINTM:

	  xKm = statuteMilesToKm*(SO->padfX[0] - metersToFeet*P.SPCS_parameters.originEastingMeters)/feetToStatuteMiles;
	  yKm = statuteMilesToKm*(SO->padfY[0] - metersToFeet*P.SPCS_parameters.originNorthingMeters)/feetToStatuteMiles;
     
	  Proj.xy2latlon(xKm, yKm, lat, lon);
     
	  fprintf(ofp,"ICON CROSS %lf\t%lf\n", lat, lon);

	  fprintf(ofp,"#\n");
	  break;

      default : // Don't know what this object is. Skip it.
	fprintf(stderr, "WARNING : object %d is undefined.\n", i);
	break;
	
      } // End of switch statement about line/point/unknown object type.

      if(SO != NULL) SHPDestroyObject(SO);

    } // End of loop thgrough objects.

    fclose(ofp);

  } // End of loop through maps.

 return 0;

}

