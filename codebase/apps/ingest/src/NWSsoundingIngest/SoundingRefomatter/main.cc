
#include <cstdio>
#include <cstring>
#include <ctype.h>
#include <dirent.h>
#include <cstdlib>
#include <cstring>

//
// This is a small program that reformats sounding data into a format
// that NWSsoundingIngest can read. This is needed because sometimes
// the input file format is not quite right - Eric Nelson is aware
// of the details. Niles Oien March 2006.
//

int processFile(char *inFilename, char *outFilename);

int main(int argc, char *argv[]){

  if (argc < 3){
    fprintf(stderr,"USAGE : indir outdir\n");
    fprintf(stderr,"All *.UPP files in indir will be processed into outdir\n");
    fprintf(stderr,"Niles Oien March 2006\n");
    return -1;
  }

  //
  // Open the input directory for scanning.
  //  
  DIR *dirp;
  if ((dirp = opendir( argv[1] )) == NULL)
  {
    fprintf(stderr,"Failed to open %s for reading.\n", argv[1] );
    perror( argv[1] );
    return -1;
  }

 struct dirent *dp;
 for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)){

   //
   // Only process those files that end in .UPP
   //
   char *p = dp->d_name + strlen(dp->d_name) - strlen(".UPP");

   if (!(strcmp(p, ".UPP"))){

     char in[1024], out[1024];

	 fprintf(stdout,"Looking at %s\n", dp->d_name );
     sprintf(in, "%s/%s", argv[1], dp->d_name );
     sprintf(out,"%s/%s", argv[2], dp->d_name );

     if (processFile(in, out)){
       fprintf(stderr, "Error processing %s into %s\n",
	       in, out);
     }

   }

 }
 return 0;
}

  
int processFile(char *inFilename, char *outFilename){


  FILE *ifp = fopen (inFilename,"r");
  if (ifp == NULL){
    fprintf(stderr,"Unable to open %s\n", inFilename);
    return -1;
  }

  FILE *ofp = fopen (outFilename,"w");
  if (ifp == NULL){
    fprintf(stderr,"Unable to create %s\n", outFilename);
    return -1;
  }


  char Line[2048];
  int mode = 0; // 0 == nothing, 1 = TTAA, 2 = TTBB, 3 = PPBB

  while (NULL != fgets(Line, 2048, ifp)){
    //
    // Skip short lines (pointless)
    //
    if (strlen(Line) < 4) continue;
    //
    // Take the linefeed off the end since it's a PITA
    //
    Line[ strlen(Line) - 1 ] = char(0);
    //
    // Ditto the trailing space
    //
    if ( Line[ strlen(Line) - 1 ] == ' '){
       Line[ strlen(Line) - 1 ] = char(0);
    }

    //
    // Skip lines that are not format compliant.
    //

    int allBad = 0;
    for (unsigned i=0; i < strlen(Line); i++){
      if (
	  ( !(isdigit( (int)Line[i] ) ) ) &&
	  ( Line[i] != ' ') &&
	  ( Line[i] != '/')
	  ){
	allBad = 1;
	break;
      }
    }


    if (
	(NULL != strstr(Line, "TTAA")) ||
	(NULL != strstr(Line, "TTBB")) ||
	(NULL != strstr(Line, "PPBB"))
	){
      allBad = 0;
    }

    if (allBad) continue;

    //
    // If this line has a TTAA, TTBB or PPBB message, get the
    // index of the first T or P. You need this - trust me.
    //

    int offsetIndex = 0;
    
    if (
	(NULL != strstr(Line, "TTAA")) ||
	(NULL != strstr(Line, "TTBB")) ||
	(NULL != strstr(Line, "PPBB"))
	){
      for (unsigned i=0; i < strlen(Line); i++){
	if (
	    (Line[i] == 'T') ||
	    (Line[i] == 'P')
	    ){
	  offsetIndex = i;
	  break;
	}
      }
    }


    //    fprintf(stdout,"Mode at start : %d\n", mode);
    //    fprintf(stdout,"Line : %s \n", Line);

    //
    // Process according to mode.
    //

    if (mode == 0){ // Waiting for TTAA line
      if (NULL != strstr(Line, "TTAA")){
	//
	// Got TTAA message, set mode and process this line.
	//
	mode = 1;
	fprintf(ofp,"%s", Line + offsetIndex);
	continue;
      }
    }
    


   if (mode == 1){ // Processing TTAA line.
      if (NULL != strstr(Line, "TTBB")){
	//
	// Got TTBB message, set mode and process this line.
	//
	mode = 2;
	fprintf(ofp,"=\n\n\n%s", Line + offsetIndex);
	continue;
      } else {
	fprintf(ofp," %s", Line);
	continue;
      }
    }


    if (mode == 2 ){ // Processing TTBB line.
      if (NULL != strstr(Line, "PPBB")){
	//
	// Got PPBB message, set mode and process this line.
	//
	mode = 3;
	fprintf(ofp,"=\n\n\n%s", Line);
	continue;
      } else {
	fprintf(ofp," %s", Line);
	continue;
      }
    }


    if (mode == 3 ){ // Processing TTBB line.
      if (NULL != strstr(Line, "TTAA")){
	//
	// Got TTAA message, set mode and process this line.
	//
	mode = 1;
	fprintf(ofp,"=\n\n\n%s", Line + offsetIndex);
	continue;
      } else {
	fprintf(ofp," %s", Line);
	continue;
      }
    }

    //    fprintf(stdout,"Mode at end : %d\n\n", mode);
	  
  }

  fclose (ifp); 

  fprintf(ofp,"=\n\n\n");
  

  fclose(ofp);

  return 0;

}
