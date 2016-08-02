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
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Params.hh"

//
// See the READ.ME file - Niles Oien April 2006.
//

void updateParams(char *appName, char *paramFilename, Params *params);

int main(int argc, char *argv[]){

  //
  // Get the TDRP params.
  //
  Params params;
  if (params.loadFromArgs(argc,argv,NULL,NULL)){
    fprintf(stderr, "Specify params file with -params option.\n");
    return(-1);
  }     

  //
  // Parse out the input params file, specified with -f
  //
  char *paramFilename = NULL;

  for (int i=1; i < argc; i++){

    if ( 
	(!(strcmp("-h", argv[i]))) ||
	(!(strcmp("-help", argv[i]))) ||
	(!(strcmp("--help", argv[i]))) 
	){
      fprintf(stderr,"See the READ.ME file in CVS\n");
      exit(-1);
    }

    if (!(strcmp("-f", argv[i]))){
      if (i == argc-1) break;
      paramFilename = argv[i+1];
      break;
    }

  }

  if (paramFilename == NULL){
    fprintf(stderr,"ERROR : no param file specified.\n");
    exit(-1);
  }

  //
  // Print the parameter file name on stderr. We will be following
  // this up by printing a status, so no return afterwards.
  //
  fprintf(stderr,"%s : ", paramFilename);

  //
  // Find the last "/" in the parameter file name.
  //
  int pathOffset = 0;
  for (int i = strlen(paramFilename) - 1; i > 0; i--){

    if ( *(paramFilename + i) == '/' ){
      pathOffset = i + 1;
      break;
    }
  }

  //
  // Then go forward past leading underscores
  //
  int go = 1;
  do {

    if ( *(paramFilename + pathOffset) == '_' )
      pathOffset++;
    else 
      go = 0;

  } while ( go );

  //
  // Now see if we can extract the app name.
  //
  int k = 0;
  char appName[1024];
  go = 1;
  do {
    if (
	(*(paramFilename + pathOffset + k) == '.' ) ||
	(*(paramFilename + pathOffset + k) == char(0) )
	) {
      go = 0;
      appName[k] = char(0);
    } else {
      appName[k] = *(paramFilename + pathOffset + k);
      k++;
    }
  } while (go);

  //
  // Special case - core files - 
  //
  if (!(strncmp(appName, "core", 4))){

    if ( params.deleteCoreFiles ){
      int problemDeleting = unlink( paramFilename );
      if (problemDeleting){
	fprintf(stderr,"CORE FILE, UNABLE TO DELETE\n");
      } else {
	fprintf(stderr,"CORE FILE, DELETED.\n");
      }
    } else {
      fprintf(stderr,"CORE FILE, IGNORED.\n" );
    }
    exit(0);
  }

  //
  // Print the application name.
  //
  fprintf(stderr, "APPLICATION : %s : ", appName);


  //
  // See if the appication exists.
  //
  char com[1024];
  sprintf(com,"csh -c \"which %s >& /dev/null\"", appName);
  if (system(com)){
    if (params.substitutions_n == 0){
      fprintf(stderr,"CHECK PARAMS BY HAND : Cannot find app\n" );
      exit(0);
    } else {
      //
      // Did not find app, have non-empty substitute list. Try substitution.
      //
      int gotSubName = 0;
      for (int l=0; l < params.substitutions_n; l++){
	if (!(strncmp(params._substitutions[l].matchName,
		      appName,
		      strlen(params._substitutions[l].matchName)))){
	  gotSubName = 1;
	  sprintf(appName,"%s", params._substitutions[l].appName);
	  break;
	}
      }
      if (gotSubName){
	//
	// Try again with new appName.
	//
	sprintf(com,"csh -c \"which %s >& /dev/null\"", appName);
	if (system(com)){
	  fprintf(stderr," CHECK PARAMS BY HAND : Cannot find app or substitute %s\n",
		  appName);
	  exit(0);
	} else {
	  fprintf(stderr," SUBSTITUTE APP %s FOUND : ", appName);
	}
      } else {
	//
	// No substitution found.
	//
	fprintf(stderr,"CHECK PARAMS BY HAND : Cannot find app\n" );
	exit(0);
      }
    }
  }
 
  //
  // It does exist - see if it is one of the apps we leave alone.
  //
  for (int k=0; k < params.leaveTheseAppsParamFilesAlone_n; k++){
    if (!(strcmp( params._leaveTheseAppsParamFilesAlone[k], appName))){
      fprintf(stderr,"CHECK PARAMS BY HAND : app is on the exclusion list.\n" );
      exit(0);
    }
  }

  //
  // Run the params check. Run the app with the parameter file and the -check_params option.
  // If there are missing parameters, this will cause lines like :
  //
  // TDRP_WARNING: parameter not set, using default - 'debug'
  // TDRP_WARNING: parameter not set, using default - 'instance'
  // TDRP_WARNING: parameter not set, using default - 'input_fmq_url'
  // 
  // To be written to the output. Pipe this output to a temporary file.
  //
  unlink("/tmp/tdrpCheck.tmp");
  sprintf(com,"csh -c \"%s -params %s -check_params >& /tmp/tdrpCheck.tmp\"", appName, paramFilename);
  system (com);

  //
  // Grep for the word 'default' in that temporary file. If it is found,
  // then the parameter file needs updating.
  //
  sprintf(com,"csh -c \"grep default /tmp/tdrpCheck.tmp >& /dev/null\"");
  // fprintf(stderr,"Executing %s\n",com);
  int retVal = system(com);
  unlink("/tmp/tdrpCheck.tmp");

  if (retVal != 0){
    //
    // No new parameters. Only update if UPDATE_ALL was selected.
    //
    if (params.updateOption == Params::UPDATE_ALL){
      fprintf(stderr,"ALL PARAMETERS PRESENT : UPDATING\n");
      updateParams(appName, paramFilename, &params);
    } else {
      fprintf(stderr,"ALL PARAMETERS PRESENT : LEAVING\n");
    }
  } else {
    //
    // The app now has new parameters. If UPDATE_NONE, leave it
    // alone, otherwise update.
    //
    if (params.updateOption == Params::UPDATE_NONE){
      fprintf(stderr,"CHECK PARAMS BY HAND - APP HAS NEW PARAMETERS\n");
    } else {
      fprintf(stderr,"APP HAS NEW PARAMETERS - UPDATING\n");
      updateParams(appName, paramFilename, &params);
    }
  }

  return 0;

}


void updateParams(char *appName, char *paramFilename, Params *params){

  //
  // Make a backup copy, if requested. Only do this if the backup
  // file does not already exist.
  //
  char com[1024];
  if (params->makeBackupCopies){
    char backupFilename[1024];
    sprintf(backupFilename,"%s.backUp", paramFilename);

    struct stat buf;
    int statVal = stat(backupFilename, &buf);
    
    if (statVal){
      //
      // The stat failed, assume the file does not exist.
      // and make a backup copy.
      //
      sprintf(com,"/bin/cp -f %s %s 1> /dev/null 2> /dev/null",
	      paramFilename, backupFilename );
      system(com);
    }
  }
  //
  // Run the app with -print_params option. Pipe stdout to /tmp/new.params
  // and stderr to /dev/null.
  //
  unlink ("/tmp/new.params");
  sprintf(com, "%s -params %s -print_params %s 1> /tmp/new.params 2> /dev/null", 
	  appName, paramFilename, params->printParamsOption);
  system(com);
  //
  // Move the new param file into place.
  //
  sprintf(com, "/bin/mv -f /tmp/new.params %s", paramFilename);
  int retVal = system( com );
  if (retVal){
    fprintf(stderr, " ----- UPDATE FAILED FOR THIS PARAM FILE!\n\n");
  }

  unlink ("/tmp/new.params"); // Should have been moved but let's try anyway.
  
  return;

}
