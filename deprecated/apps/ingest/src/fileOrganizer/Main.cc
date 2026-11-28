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
// This small utility takes a directory filled with files named
// in the YYYYMMDDblah convention, ie. 20030811_0352.dat
// and creates a date-named subdirectory and moves the file into
// it, so that
//
// Some/Dir/20030811_0352.dat
//
//    Becomes :
//
// Some/Dir/20030811/20030811_0352.dat
//
// This happens to be quite handy sometimes.
//
// The path delimiter is assumed to be '/' and generally accepted UNIX
// commands are used in system() calls.
//
// Niles Oien April 2004
//
#include <cstdio>
#include <sys/stat.h>
#include <toolsa/file_io.h>
#include <toolsa/umisc.h>
#include <stdlib.h>
#include <cstring>

int main(int argc, char * argv[]){

  int debug = 0;
  int zip = 0;
  float minAgeHours = 0.0;
  int resortToFiletimes = 0;
  //
  // Parse the arguments to see if zip or debug are set.
  //
  for (int i=1; i < argc; i++){
    if (!strcmp("-debug", argv[i])) debug = 1;
    if (!strcmp("-resortToFiletimes", argv[i])) resortToFiletimes = 1;
    if (!strcmp("-zip", argv[i])) zip = 1;
    if (!strncmp("-minAge=", argv[i], strlen("-minAge="))){
      if (1 != sscanf( argv[i],"-minAge=%f", &minAgeHours)){
	fprintf(stderr,"Failed to interpret argument %s\n", argv[i]);
	exit(-1);
      }
    }
  }


  if (argc < 2){
    fprintf(stderr,"USAGE : fileOrganizer <dir_name> [-zip] [-debug] [-minAge=x] [-resortToFiletimes]\n\n");
    fprintf(stderr,"Files named /Some/Dir/YYYYMMDDblah.dat are moved to\n");
    fprintf(stderr,"/Some/Dir/YYYYMMDD/YYYYMMDDblah.dat - ie. they are moved into\n");
    fprintf(stderr,"dated subdirectories. Filenames with underscores between the\n");
    fprintf(stderr,"date fields, ie. YYYY_MM_DDBlah.dat, are also moved.\n\n");

    fprintf(stderr,"Age, x, is in hours, defaults to 0. Files not older than this\n");
    fprintf(stderr,"are not moved into subdirectories. This is specified with\n");
    fprintf(stderr,"the -minAge= option, ie. -minAge=2.5\n\n");

    fprintf(stderr,"The -debug option causes some messages to be printed\n\n");

    fprintf(stderr,"The -resortToFiletimes option will cause the program to use\n");
    fprintf(stderr,"the time the file last changed if the time cannot be parsed\n");
    fprintf(stderr,"from the filename.\n\n");

    fprintf(stderr,"The -zip option causes the files to be zipped after they are moved.\n");
    fprintf(stderr,"Niles Oien April 2004\n");

    return -1;
  }
  //
  // Open the directory.
  //
  DIR *dirp;
  dirp = opendir(argv[1]);
  if (dirp == NULL)
    {
      fprintf(stderr,"Failed to open directory %s\n", argv[1]);
      perror(argv[1]);      
      return -1;
    }
  
  // Loop thru directory looking for the data file names
  
  struct dirent *dp;
  int count=0;
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
  {
    //
    // Leave file starting with a dot or underscore.
    //
    if (dp->d_name[0]=='.' || dp->d_name[0]== '_')
      {
	if (debug){
	  fprintf(stderr,"Skipping %s, it is hidden.\n", dp->d_name);
	}
	continue;
      }
    //
    // See if it is a directory - leave these alone (no recursion).
    //
    char FileName[MAX_PATH_LEN];
    sprintf(FileName,"%s/%s", argv[1], dp->d_name);
    struct stat FileStat;
    //
    if (ta_stat(FileName,&FileStat)!=0){
      continue;
    }
    //
    // Continue if it is a directory.
    //
    if (S_ISDIR(FileStat.st_mode)){
      if (debug){
	fprintf(stderr,"Skipping %s, it is a directory.\n", dp->d_name);
      }
      continue;
    } 
    //
    // Continue if it is not yet old enough.
    //
    time_t now = time(NULL);
    long age = now - FileStat.st_mtime;
    float ageInHours = age/3600.0;
    if (ageInHours < minAgeHours){
      if (debug){
	fprintf(stderr,"Skipping %s, it is too new (%f hours).\n", 
		dp->d_name, ageInHours);
      }
      continue;
    }
    //
    // Skip it if we can't decode the date.
    //
    int year, month, day;
    //
    if (
	(3 != sscanf(dp->d_name,"%4d%2d%2d", &year, &month, &day)) &&
	(3 != sscanf(dp->d_name,"%4d_%2d_%2d", &year, &month, &day))
	){
      //
      if (debug){
	fprintf(stderr,"For %s, can't decode the name.\n", dp->d_name);
      }
      if (!(resortToFiletimes)){
	continue;
      } else {
	date_time_t T;
	T.unix_time = FileStat.st_mtime;
	uconvert_from_utime( &T );
	year = T.year; month = T.month; day = T.day;
	if (debug){
	  fprintf(stderr,"For %s, taking date as %d%02d%02d from stat\n", 
		  dp->d_name, year, month, day);
	}

      }
    }
    //
    // Do a rough check on the date.
    //
    if (
	(year < 1975) ||
	(year > 2500) ||
	(month < 1) ||
	(month > 12) ||
	(day < 1) ||
	(day > 31) 
	){
      continue;
    }

    //
    // Make the target directory. System call.
    //
    char dirPath[MAX_PATH_LEN];
    sprintf(dirPath,"%s/%04d%02d%02d", argv[1], year, month, day);
    char com[MAX_PATH_LEN];
    sprintf(com,"/bin/mkdir -p %s", dirPath);
    int ret = system( com );
    if (debug){
      fprintf(stderr,"%s\n", com);
    }
    if (ret){
      continue;
    }
    //
    // Move the file into the target directory. Done
    // with a system call.
    //
    sprintf(com,"/bin/mv -f %s %s", FileName, dirPath);
    if (debug){
      fprintf(stderr,"%s\n", com);
    }
    system(com);
    //
    // Gzip if specified.
    //
    if (zip){
      char movedFilename[1024];
      sprintf(movedFilename, "%s/%s", dirPath, dp->d_name);
      sprintf(com,"gzip -f %s", movedFilename);
      if (debug){
	fprintf(stderr,"%s\n", com);
      }
      system(com);
    }

    count++;
    if (count > 59){
      count = 0;
      sleep(1);
    }

  }
  //
  // Done looping through files - exit.
  //
  return 0;

}
