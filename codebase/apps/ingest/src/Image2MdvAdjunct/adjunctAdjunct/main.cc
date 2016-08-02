
#include <cstdio>


/**
 *
 * @file main.cc
 *
 *
 * Small adjunct to Image2MdvAdjunct that generates some parts
 * of TDRP paramtere files.
 *
 *
 * @author Niles Oien
 *
 */

#include "Params.hh"
#include <cmath>
#include <cstdio>
#include <toolsa/file_io.h>
#include <cstdlib>

int main(int argc, char *argv[]){

  //
  // Get RAP_DATA_DIR variable or default to local.
  //
  char rapDir[1024];
  if (NULL == getenv("RAP_DATA_DIR")){
    sprintf(rapDir,"%s", "./");
  } else {
    sprintf(rapDir,"%s", getenv("RAP_DATA_DIR"));
  }
  
  //
  // Read the command-line specified param file.
  //
  Params params;

  if (params.loadFromArgs(argc,argv,NULL,NULL)){
    fprintf(stderr, "Specify params file with -params option.\n");
    return(-1);
  }  

  //
  // Make mdv output directory.
  //
  char dir[1024];
  sprintf(dir,"%s/%s", rapDir, params.mdvOutDir);
  if (ta_makedir_recurse( dir )){
    fprintf(stderr, "Failed to create directory %s\n", dir);
    return -1;
  }

  //
  // Open the output files.
  //
  char outName[1024];
  sprintf(outName,"%s/%s/_DsMdvServer.tiled", rapDir, params.mdvOutDir );

  FILE *dfp = fopen (outName, "w");
  if (dfp == NULL){
    fprintf(stderr, "Failed to create %s\n", outName);
    return -1;
  }

  FILE *ifp = fopen ("Image2MdvAdjunct.template", "w");
  if (ifp == NULL){
    fprintf(stderr, "Failed to create Image2MdvAdjunct.template\n");
    return -1;
  }

  FILE *cfp = fopen ("copyScript", "w");
  if (cfp == NULL){
    fprintf(stderr, "Failed to create file copyScript\n");
    return -1;
  }

  fprintf(cfp, "#!/bin/csh\n\n# Automatically generated copy script, see Niles Oien.\n\n");

  fprintf(dfp, "// This file automatically generated, see Niles Oien\n\n");
  fprintf(dfp,"\nserve_multiple_domains = TRUE;\n\n");
  fprintf(dfp,"domains = {\n");

  fprintf(ifp,"// This file automatically generated, see Niles Oien.\n\n");
  fprintf(ifp,"\noutDimensions = {\n");

  int rowNum = 0, colNum = 0;

  //
  // Loop hrough the zoom states.
  //
  for (int izoom=0; izoom < params.tile_n; izoom++){

    //
    // Loop through the tiles in this zoom state.
    //
    for (double latCenter = params.region.minLat;
	 latCenter <= params.region.maxLat;
	 latCenter += params._tile[izoom].latitudeExtent / 2.0){

      for (double lonCenter = params.region.minLon;
	   lonCenter <= params.region.maxLon;
	   lonCenter += params._tile[izoom].longitudeExtent / 2.0){
      
	if (lonCenter == params.region.minLon) colNum = 0;
	if (latCenter == params.region.minLat) rowNum = 0;

	//
	// Give this tile a name.
	//
	char name[32];
	sprintf(name,"row_%03d_col_%03d", rowNum, colNum);
	fprintf(stdout,"Processing for %s in %s\n", 
		name, params._tile[izoom].name);


	//
	// Write to the top level DsMdvServer param file and
	// in the process decide if we're global.
	//
	double latStart = 0.0;
	double latEnd = 0.0;
	double lonStart = 0.0;
	double lonEnd = 0.0;

	if (params._tile[izoom].isGlobal){

	  fprintf(dfp, " { %lf, %lf, %lf, %lf, \"mdvp::%s//localhost:0:%s/%s/%s\" },\n",
		  -90.0, -180.0, 90.0, 180.0,
		  name, params.mdvOutDir, params._tile[izoom].name ,name);

	  latStart = params.region.minLat;  latEnd = params.region.maxLat;  
	  lonStart = params.region.minLon;  lonEnd = params.region.maxLon;  

	  //
	  // In the global case make sure we exit both nested "for" loops.
	  //
	  lonCenter = params.region.maxLon + 1.0;
	  latCenter = params.region.maxLat + 1.0;

	} else {

	  latStart = latCenter - params._tile[izoom].latitudeExtent / 2.0;
	  lonStart = lonCenter - params._tile[izoom].longitudeExtent / 2.0;
	  latEnd = latCenter + params._tile[izoom].latitudeExtent / 2.0;
	  lonEnd = lonCenter + params._tile[izoom].longitudeExtent / 2.0;

	  if (!(params.allowTilesOutsideRegion)){
	    if (
		(latStart < params.region.minLat) ||
		(lonStart < params.region.minLon) ||
		(latEnd > params.region.maxLat) ||
		(lonEnd > params.region.maxLon)
		){
	      fprintf(stderr,"Skipping %s in %s, outside region.\n", 
		      name, params._tile[izoom].name);
	      continue; // Skip this tile.
	    }
	  }

	  fprintf(dfp, " { %lf, %lf, %lf, %lf, \"mdvp::%s//localhost:0:%s/%s/%s\" },\n",
		  latStart, lonStart, latEnd, lonEnd,
		  name, params.mdvOutDir, params._tile[izoom].name ,name);
	  
	}
 

	//
	// Write to the Image2MdvAdjunct param file.
	//
	fprintf(ifp, " { \"%s_%s\", %lf, %lf, %lf, %lf, %d, 0.0, 0.0 },\n",
		params._tile[izoom].name, name, latStart, lonStart, latEnd, lonEnd,
		params._tile[izoom].nx);
	
	//
	// Make the target directory under RAP_DATA_DIR and write
	// the little local DsMdvServer param file there.
	//
	sprintf(dir,"%s/%s/%s/%s", rapDir, params.mdvOutDir,  
		params._tile[izoom].name, name);
	if (ta_makedir_recurse( dir )){
	  fprintf(stderr, "Failed to create directory %s\n", dir);
	  return -1;
	}

	char subFilename[1024];
	sprintf(subFilename, "%s/_DsMdvServer.%s", dir, name);
	FILE *sdfp = fopen(subFilename, "w" );
	if (sdfp == NULL){
	  fprintf(stderr,"Failed to create %s\n", subFilename);
	  return -1;
	}
     
	fprintf(sdfp, "// This file automatically generated by Niles Oien\n\n");
	fprintf(sdfp, "\nuse_static_file = TRUE;\n\n");
	fprintf(sdfp, "static_file_url = \"%s/%s/%s/%s_%s.mdv\";\n\n", 
		params.mdvOutDir, params._tile[izoom].name,
		name, params._tile[izoom].name, name);
	fclose(sdfp);

	//
	// Add an entry to the copy script that will populate the directories with
	// MDV files after Image2MdvAdjunct has driven Image2Mdv to create the
	// MDV files.
	//
	fprintf(cfp, "cp -v %s_%s.mdv %s/%s/%s/%s/\n",
		params._tile[izoom].name,
		name, rapDir, params.mdvOutDir, 
		params._tile[izoom].name, name);
 
	colNum++;
      }
      rowNum++;
    }
  }

  //
  // Back up to erase trailing newline and comma.
  //
  fseek(dfp, -2, SEEK_CUR);
  fseek(ifp, -2, SEEK_CUR);

  fprintf(dfp,"\n};\n\n");
  fprintf(ifp,"\n};\n\n");
  fprintf(cfp,"#\n");

  fclose(dfp); fclose(ifp); fclose(cfp);

  //
  // Set protection on the copy script to open.
  //
  system("chmod 777 copyScript");

  return 0;

}
