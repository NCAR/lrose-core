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
// This is a small utility that calls wget to get the
// files from a web directory.
//
// Niles Oien September 2006
//

#include <cstdio>
#include <cstring>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <toolsa/file_io.h>

#include <cstdlib>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>

#include <didss/LdataInfo.hh>

#include <vector>

#include "Params.hh"

int _fileExists(const char *dirName,
		const char *fileName,
		char *foundFilename,
		char *foundExt,
		int debug);

int parseLine(char *Line,
	      char *fileName, 
	      char *filenameNoExt, 
	      time_t *fileTime,
	      char *matchString,
	      bool isFtp);

int getMonth(char *monStr);

int main(int argc, char *argv[]){

  //
  // Get the TDRP paramters
  //
  Params P;
  if (P.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }                       

  //
  // See if "wget" is installed.
  //
  if (P.debug) fprintf(stdout, "Checking for wget....\n");
  char com[1024];

  if (P.debug){
    sprintf(com, "which wget");
  } else {
    sprintf(com, "which wget 2>&1 > /dev/null");
  }

  int i = system(com);
  if (i != 0){
    fprintf(stderr,"wget is not installed - I cannot cope.\n");
    exit(-1);
  }
  
  PMU_auto_init("httpFileFetch", P.instance,
                PROCMAP_REGISTER_INTERVAL);     

  //
  // Make sure the output directory exists.
  //
  if (ta_makedir_recurse(P.outDir)){
    fprintf(stderr,"Error - could not create %s\n",
	    P.outDir);
    exit(-1);
  }
  //
  // Make sure the index directory exists.
  //
  if (ta_makedir_recurse(P.indexDir)){
    fprintf(stderr,"Error - could not create %s\n",
	    P.indexDir);
    exit(-1);
  }
  //
  // Go into the main loop, which is eternal (unless the user has
  // elected to run once only).
  //
  do {

    //
    // Download the index.html file (delete it first)
    //
    char indexFile[1024];
    sprintf(indexFile, "%s/index.html", P.indexDir);
    unlink( indexFile );
    
    sprintf(com,"wget %s -P %s %s",
	    P.wgetOptions, P.indexDir, P.url);
    if (P.debug){
      fprintf(stderr,"Executing : %s\n", com);
    }
    system(com);

    //
    // Parse index.html for file names, times, push back
    // into vectors. Keep a note of the most recent file time.
    //
    FILE *fp = fopen(indexFile, "r");
    if (fp == NULL){
      fprintf(stderr, "Unable to open %s\n", indexFile);
      exit(-1);
    }

    vector <time_t> fileTimes;
    vector <string> fileNames;
    vector <string> filenamesNoExt;

    fileTimes.clear(); fileNames.clear(); filenamesNoExt.clear();

    time_t mostRecentFileTime = 0L;
    char Line[1024];
    while(NULL != fgets(Line, 1024, fp)){
      char fileName[1024];
      char filenameNoExt[1024];
      time_t fileTime;
      if (0==parseLine(Line, fileName, filenameNoExt, 
		       &fileTime, P.filenameSubString,
		       P.isFtp)){
	if (P.debug){
	  fprintf(stderr,"Have %s (%s) at %s\n", fileName, filenameNoExt,
		  utimstr(fileTime));
	}


	//
	// See if it contains substrings we want it to.
	//
	bool containsAll = true;
	for (int is=0; is < P.requiredSubstrings_n; is++){
	  if (NULL == strstr(fileName, P._requiredSubstrings[is])){
	    if (P.debug){
	      fprintf(stderr,"Skipping %s since it does not contain substring %s\n",
		      fileName, P._requiredSubstrings[is]);
	    }
	    containsAll = false;
	    break;
	  }
	}

	if (containsAll){
	  if (mostRecentFileTime < fileTime) mostRecentFileTime = fileTime;
	  fileTimes.push_back( fileTime );
	  fileNames.push_back( fileName );
	  filenamesNoExt.push_back( filenameNoExt );
	}
      }

    }
    fclose(fp);


    //
    // Unless the user wants to leave it, delete the index file.
    //
    if (!(P.leaveIndexFileWhenDone)){
      unlink( indexFile );
    }

    //
    // Loop through the files, doing downloads if they pass the criteria.
    //
    vector< time_t >::iterator i1;
    vector< string >::iterator i2 = fileNames.begin();
    vector< string >::iterator i3 = filenamesNoExt.begin();

    for( i1 = fileTimes.begin(); i1 != fileTimes.end(); i1++ ) {

      long absAge = time(NULL) - *i1;
      if (P.ignoreTimes) absAge = P.maxAbsoluteFileAge-1;
      if (absAge > P.maxAbsoluteFileAge){
	if (P.debug){
	  fprintf(stderr,"Skipping %s due to absolute age check (%ld > %ld).\n",
		  i2->c_str(), absAge, P.maxAbsoluteFileAge);
	}
	i2++; i3++;
	continue;
      }

      long relAge = mostRecentFileTime - *i1;
      if (P.ignoreTimes) relAge = P.maxRelativeFileAge - 1;
      if (relAge > P.maxRelativeFileAge){
	if (P.debug){
	  fprintf(stderr,"Skipping %s due to relative age check (%ld > %ld).\n",
		  i2->c_str(), relAge, P.maxRelativeFileAge);
	}
	i2++; i3++;
	continue;
      }

      if (strlen(P.filenameSubString) > 0){
	if (NULL==strstr(i2->c_str(), P.filenameSubString)){
	  if (P.debug){
	    fprintf(stderr,"Skipping %s due to substring check (%s).\n",
		    i2->c_str(), P.filenameSubString);
	  }
	  i2++; i3++;
	  continue;
	}
      }

      //
      // See if the file already exists on disk. This is actually
      // reasonably tricky, depending as it does on if the file,
      // or a compressed version thereof, or an uncompressed version
      // thereof, is on disk.
      //
      int localDebug = 0;
      if (P.debug) localDebug = 1;

      if (_fileExists(P.outDir, i2->c_str(), NULL, NULL, localDebug)){
	if (P.debug){
	  fprintf(stderr,"Skipping %s since it already exists.\n",
		  i2->c_str());
	}
	i2++; i3++;
	continue;
      }

      //
      // OK to download.
      //
      sprintf(com,"wget %s -P %s %s/%s",
	      P.wgetOptions, P.outDir, P.url, i2->c_str());
      if (P.debug){
	fprintf(stderr,"Executing : %s\n", com);
      }
      system(com);

      //
      // Run a post download command on the file, if desired.
      //
      if (P.doPostDownloadCommand){
	sprintf(com, "%s %s/%s", P.postDownloadCommand, P.outDir, i2->c_str());
	if (P.debug) fprintf(stderr, "Executing %s\n", com);
	system(com);
      }
      
      //
      // Write an ldatainfo file, if requested.
      //
      if (P.writeLdata){
	//
	// Test the file to be sure it's there. Also check its
	// age, since we only want to write an ldatainfo file if the
	// file is new.
	//
	char foundFilename[1024];
	char foundExt[64];

	if (_fileExists(P.outDir, i2->c_str(), foundFilename, foundExt, localDebug)){

	  char totalFoundFilename[1024];
	  if (strlen(foundExt) > 1) {
	    sprintf(totalFoundFilename,"%s/%s.%s", P.outDir, foundFilename, foundExt);
	  } else {
	    sprintf(totalFoundFilename,"%s/%s", P.outDir, foundFilename);
	  }
	  
	  struct stat buf;
	  if (0 == stat(totalFoundFilename, &buf)){
	    long fileAge = time(NULL) - buf.st_ctime;
	    if (fileAge > P.maxAgeForNewFile){
	      if (P.debug){
                           fprintf(stderr, "File %s is too old (%d seconds), no _ldataInfo written.\n", 
                                   totalFoundFilename, (int)fileAge);
                           fprintf(stderr, "File time : %s\n", utimstr(buf.st_ctime));
                           fprintf(stderr, "Current time : %s\n", utimstr(time(NULL)));
              }
	    } else {
	      //
	      // New file is there - write out ldatainfo.
	      //
	      LdataInfo L;
	      L.setWriter( "httpFileFetch" );
	      L.setDir(P.outDir);
	      L.setDataFileExt( foundExt );
	      L.setUserInfo1( foundFilename );
	      char tb[1024];
	      if (strlen( foundExt) > 1){
		sprintf(tb,"%s.%s", foundFilename, foundExt);
	      } else {
		sprintf(tb,"%s", foundFilename);
	      }
	      L.setUserInfo2( tb );
	      L.setRelDataPath( tb );
	      L.setDataType( P.dataType );
	      //
	      int retVal = 0;
	      if (P.useRemoteFileTimes){
		retVal = L.write( *i1 ); // Use file time for time in ldatainfo
	      } else {
		retVal = L.write( time(NULL) );
	      }
	      if (retVal){
		fprintf(stderr,"LdataInfo write failed.\n");
	      } else {
		if (P.debug){
		  fprintf(stderr,"LDATA write :\n");
		  L.printFull( cerr );
		}
	      }
	    }
	  }
	}
      }
      //
      // Sleep between file downloads.
      //
      if (P.debug){
	fprintf(stderr, "Waiting for %d seconds before next file download.\n\n\n",
		P.fileDownloadDelay);
      }
      for (i=0; i < P.fileDownloadDelay; i++){
	PMU_auto_register("Downloading");
	sleep(1);
      }

      i2++; i3++;

    }

    //
    // Sleep between directory checks.
    //
    if (!(P.runOnce)){
      if (P.debug){
	fprintf(stderr, "Waiting for %d seconds before next run....\n\n\n",
		P.sleepBetweenChecks);
      }
      for (i=0; i < P.sleepBetweenChecks; i++){
	PMU_auto_register("Waiting...");
	sleep(1);
      }
    }

  } while (!(P.runOnce)); // End of main loop.

  return 0;

}


int parseLine(char *Line,
	      char *fileName, 
	      char *filenameNoExt, 
	      time_t *fileTime,
	      char *matchString,
	      bool isFtp){
  //
  // If the line does not contain "a href", it is not for us.
  //
  if (strcasestr(Line, "a href") == NULL) return -1;
  if (strcasestr(Line, "</a>") == NULL) return -1;

  //
  // If the line contains the words "Parent Directory", it's not for us.
  //
  if (strstr(Line, "Parent Directory") != NULL) return -1;



  //
  // If a match string is specified and it's not in the line, return.
  //
  if (strlen(matchString) > 0){
    if (NULL == strstr(Line, matchString)){
      return -1;
    }
  }

  //
  // Deal with FTP protocol format, which is slightly different.
  //
  if (isFtp){

    date_time_t T;
    char monStr[5];
    if (3 == sscanf(Line, "%d %3s %d", &T.year, monStr, &T.day)){
      T.hour = 0; T.min = 0; T.sec = 0;
      T.month = getMonth(monStr);

      // Sometimes hour and minute are not set
      sscanf(Line+strlen("  2007 Jun 05"), "%d:%d", &T.hour, &T.min);
      uconvert_to_utime( &T );
      *fileTime = T.unix_time;

      // Filename starts after ">" character and ends with "<"
      char *p1 = strstr(Line, ">"); // Pointer to first ">"
      if (p1 ==NULL) return -1;
      char *p2 = strstr(p1, "<"); // Subsequent "<" character
      if (p1 ==NULL) return -1;

      char *start = p1 + 1;
      int len = p2 - p1 - 1;
      memcpy(fileName, start, len);
      fileName[len] = char(0);

      int i = len -1;

      do {
	if (fileName[i] == '.') break;
	i--;
      } while (i > 0);

      if (i==0){
	// No dot found
	memcpy(filenameNoExt, start, len);
      } else {
	// Dot was found
	memcpy(filenameNoExt, start, i);
      }
      
      return 0;

    }

    return -1;

  } // End of FTP protocol.

  //
  // Line looks plausible - start parsing it -
  //
  char *p = strcasestr(Line, "a href=");
  p = p + strlen("a href=") + 1;
  int i=-1;

  do{
    i++;
    fileName[i]=p[i];
    filenameNoExt[i]=p[i];
    //
    if (fileName[i]== '"')
      fileName[i]=0;
    //
    if (filenameNoExt[i]== '.')
      filenameNoExt[i]=char(0);
    //
  } while (fileName[i] != char(0));
  
  //
  // Should have the name, now parse out the date (harder).
  //
  *fileTime = 0L;

  p = strcasestr(Line, "</a>");
  p = p + strlen("</a>") + 1;
  i=-1;

  date_time_t T;
  T.sec = 0;
  char monStr[5];

  if (5 == sscanf(p, " %d-%3s-%d %d:%d",
		  &T.day, monStr, &T.year, &T.hour, &T.min)){
    int month = getMonth(monStr);
    if (month != -1){
      T.month = month;
      uconvert_to_utime( &T );
      *fileTime = T.unix_time;
    }
  }

  return 0;

}

int getMonth(char *monStr){

  if (strcmp(monStr, "Jan") == 0) return 1;
  if (strcmp(monStr, "Feb") == 0) return 2;
  if (strcmp(monStr, "Mar") == 0) return 3;
  if (strcmp(monStr, "Apr") == 0) return 4;
  if (strcmp(monStr, "May") == 0) return 5;
  if (strcmp(monStr, "Jun") == 0) return 6;
  if (strcmp(monStr, "Jul") == 0) return 7;
  if (strcmp(monStr, "Aug") == 0) return 8;
  if (strcmp(monStr, "Sep") == 0) return 9;
  if (strcmp(monStr, "Oct") == 0) return 10;
  if (strcmp(monStr, "Nov") == 0) return 11;
  if (strcmp(monStr, "Dec") == 0) return 12;

  return -1;

}

//
// Returns 1 if the file is already exists. Complicated by the fact
// that either the filename we pass in, or the file on disk, may be
// compressed. The filname that was actually out there on disk that
// makes us think we found the file is returned in foundFilename, if
// it is not NULL (this allows the writing of an ldata info file).
// Similarly the compression extension is returned in foundExt if it
// is not null.
//
int _fileExists(const char *dirName, const char *fileName, 
		char *foundFilename, char *foundExt, int debug){


  const int nExts = 8; // I can think of 8 ways to compress a file, and they are ....

  char *exts[nExts] = {(char *) ".gz", (char *) ".Z", (char *) ".bz", (char *) ".bz2", (char *) ".tgz", (char *) ".tar", (char *) ".tbz2", (char *) ".tbz" };

  //
  // Make a working copy of the filename.
  //
  char fnBuffer[1024];
  sprintf(fnBuffer, "%s", fileName);

  //
  // If the filename that was passed in was compressed, strip
  // off the compression extension.
  //
  for (int i=0; i < nExts; i++){
    int extLen = strlen(exts[i]);
    if (!(strcmp(fnBuffer + strlen(fnBuffer) - extLen, exts[i]))){
      fnBuffer[strlen(fnBuffer) - extLen] = char(0);
      break;
    }
  }

  if (debug) fprintf(stderr, "Testing if file %s exists in %s\n", fnBuffer, dirName);


  //
  // See if the uncompressed file exists on disk.
  //
  struct stat buf;

  char testFilename[1024];
  sprintf(testFilename, "%s/%s", dirName, fnBuffer);
  if (debug) fprintf(stderr, "  Looking for %s\n", testFilename);
  if (0==stat(testFilename, &buf)){
    if (debug) fprintf(stderr, "     Found %s\n", testFilename);
    if (NULL != foundFilename) sprintf(foundFilename,"%s", fnBuffer);
    if (NULL != foundExt) sprintf(foundExt,"%s", "");
    return 1;
  }
  //
  // Try the compressed versions.
  //
  for (int i=0; i < nExts; i++){
    sprintf(testFilename, "%s/%s%s", dirName, fnBuffer, exts[i]);
    if (debug) fprintf(stderr, "  Looking for %s\n", testFilename);
    if (0==stat(testFilename, &buf)){
      if (debug) fprintf(stderr, "     Found %s\n", testFilename);
      if (NULL != foundFilename) sprintf(foundFilename,"%s", fnBuffer);
      if (NULL != foundExt) sprintf(foundExt,"%s", exts[i] + 1);
      return 1;
    }
  }

  //
  // Failed to find it, or a compressed version thereof.
  //
  if (debug) fprintf(stderr,"Did not find %s\n", fnBuffer);
  return 0;
}
