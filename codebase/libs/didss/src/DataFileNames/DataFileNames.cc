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
/////////////////////////////////////////////////////////////
// DataFileNames.cc - implementation of DataFilenames.hh
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// December 1998
//
/////////////////////////////////////////////////////////////

#include <toolsa/file_io.h>
#include <cctype>
#include <unistd.h>

//
// The way the different OS's stat the file systems
// requires some fiddling here.
//
#if defined(__linux) || defined(LINUX_ALPHA)
 #include <sys/vfs.h>
#elif defined(__APPLE__)
 #include <sys/mount.h>
#else
 #include <sys/types.h>
 #include <sys/statvfs.h>
#endif

#include <didss/DataFileNames.hh>
using namespace std;

//
// Find out how full the file system is for a file.
// Return -1 if failed, else 0..100
//
int DataFileNames::PercentFullDisk(const string FileName) const
{
  return PercentFullDisk(FileName.c_str());
}


int DataFileNames::PercentFullDisk(const char *FileName) const
{

#if defined(__linux) || defined(LINUX_ALPHA) || defined(__APPLE__)

  struct statfs fs;

  if (ta_statfs(FileName, &fs)) return -1;
 
  float frac = 1.0 - (float(fs.f_bavail)/float(fs.f_blocks));
  int percent = int(100.0 * frac + 0.5);
  /* Watch roundoff. */

  if (percent < 0) percent=0;
  if (percent > 100) percent=100;

  return percent;

#else
  
  int percent;
  struct statvfs B;
  float frac;

  if (statvfs(FileName,&B)) return -1;
  frac = float(B.f_bfree) / float(B.f_blocks); 
  frac = 1.0 - frac;

  percent=int(100.0*frac);
  /* Watch roundoff. */

  if (percent < 0) percent=0;
  if (percent > 100) percent=100;

  return percent;

#endif

}


///////////////////////////////////////////
// Print the result of looking at a file.
// By default print to stdout

void DataFileNames::PrintFileFacts(FILE *out /* = NULL */) const
{

  if (out == NULL) {
    out = stdout;
  }

  fprintf(out,"\tCompressed :\t\t\t");
  if (Compressed) {
    fprintf(out,"TRUE\n");
  } else {
    fprintf(out,"FALSE\n");
  }

  fprintf(out,"\tDate in filename :\t\t");
  if (!(NameDateValid)) {
    fprintf(out,"FALSE\n");
  } else {
    fprintf(out,"TRUE\t");
    fprintf(out,"%04d/%02d/%02d %02d:%02d:%02d\n",
	    NameDate.year,NameDate.month,NameDate.day,
	    NameDate.hour,NameDate.min,NameDate.sec);
  }

  fprintf(out,"\tTime implied (date only in filename) :\t\t\t");
  if (DateOnly) {
    fprintf(out,"TRUE\n");
  } else {
    fprintf(out,"FALSE\n");
  }

  fprintf(out,"\tFile exists :\t\t\t");
  if (Exists) {
    fprintf(out,"TRUE\n");
  } else {
    fprintf(out,"FALSE\n");
    return; // Don't print other fields.
  }

  fprintf(out,"\tTime since modification (secs) :\t%ld\n", long(ModTime));
  fprintf(out,"\tTime since access (secs) :\t\t%ld\n", long(AccessTime));


  fprintf(out,"\tIs a directory :\t\t\t");
  if (Directory) {
    fprintf(out,"TRUE\n");
  } else {
    fprintf(out,"FALSE\n");
  }


}


////////////////////////////////////
// Main routine - look at a file and set the
// public variables appropriately.
//
void DataFileNames::GetFileFacts(const string FileName,
				 const string CompressedExt,
				 const string MatchExt, const string FilePattern)
{
    GetFileFacts(FileName.c_str(), CompressedExt.c_str(), MatchExt.c_str(),
                 FilePattern.c_str());
}

void DataFileNames::GetFileFacts(const char *FileName,
				 const char *CompressedExt,
				 const char *MatchExt, const char *FilePattern)
{

  char *ext; // To see if file's extention matches the compressor's.
  char s[MAX_PATH_LEN]; // For internal workings.
  char *end; // End of the filename - used to get rid of the path.
  struct stat FileStat; // To get file details with stat.

  DateOnly = false; // set to true if filename has no hour, min, sec

  // Get the things that don't matter if the file
  // actually exists first - compressed, and date/time.

  // Is this filename that of a compressed file?

  if (strlen(CompressedExt) >= strlen(FileName)){ // name too short
    Compressed=false;
  } else { // We'll have to look at it.
    ext= (char *)FileName + strlen(FileName) - strlen(CompressedExt);
    if (!strcmp(CompressedExt,ext)){
      Compressed=true;
    } else {
      Compressed=false;
    }
  }


  // Copy the filename for internal doings.
  sprintf(s,"%s",FileName);

  // First, strip the compressed extention from the filename, if compressed.
  if (Compressed){
    s[strlen(FileName)-strlen(CompressedExt)]=char(0);
  }

  // See if the filename matches with the
  // specified extension.
  if (strlen(s) >= strlen(MatchExt)) {
    end= s + strlen(s)-strlen(MatchExt);
    if (!strcmp(end,MatchExt)){
      ExtMatches=true;
    } else {
      ExtMatches=false;
    }
  }

  // Get the date and time from the filename.
  NameDateValid=false; // Default.
  time_t dataTime = 0;
  
  // If an additional file pattern was not provided, 
  // then try to parse using the built-in patterns
  if( FilePattern == NULL || strlen( FilePattern ) == 0 )
  {
    if (getDataTime(FileName, dataTime, DateOnly) == 0) {
      NameDateValid=true;
      NameDate.unix_time = dataTime;
      uconvert_from_utime(&NameDate);
    }
  }
  else
  {
    if (getDataTime(FileName, FilePattern, dataTime, DateOnly) == 0) {
      NameDateValid=true;
      NameDate.unix_time = dataTime;
      uconvert_from_utime(&NameDate);
    }
  }


  // See if the file exists. Failure to stat
  // is taken as a no.
  
  if (ta_stat(FileName,&FileStat)!=0){
    Exists=false;
    return;
  }
  Exists=true;


  // See if it is a regular file.
  if (S_ISREG(FileStat.st_mode)){
    Regular=true;
  } else {
    Regular=false;
  }
  
  // See if it is a Soft Link file.
  if (S_ISREG(FileStat.st_mode)){
    Regular=true;
  } else {
    Regular=false;
  }


  // Make note of the times.
  ModTime=    time(NULL) - FileStat.st_mtime; 
  AccessTime= time(NULL) - FileStat.st_atime;

  // And the file size.
  FileSize = (unsigned long)(FileStat.st_size);

  // See if it is a directory.
  if (S_ISDIR(FileStat.st_mode)){
    Directory=true;
	
    // See if it is a Soft Link 
    if (ta_lstat(FileName,&FileStat)!=0){
        Exists=false;
        return;
	} else {  // ta_lstat valid return.
      if (S_ISLNK(FileStat.st_mode)) {
		  char  buf[MAX_PATH_LEN];
		  int len;
		  IsSoftLinkDir = true;

		  if((len = readlink(FileName,buf,MAX_PATH_LEN)) > 0) {
			  if((strncmp(buf,"./",2) == 0) ||
				 (strncmp(buf,"../",3) == 0 )) {
				 IsPathRelative = true;
			  } else {
				 IsPathRelative = false;
			  }
		  } else {
            Exists=false;
            return;
		  }
	  } else { // is not a Soft Linked Dir
		  IsSoftLinkDir = false;
		  IsPathRelative = false;
	  }
	}

  } else { // Is not a Dir
    Directory=false;
	IsSoftLinkDir = false;
	IsPathRelative = false;
  }
}

/////////////////////////////////////////////////////////////////////
// Get the data time information from the given file path.
//
// The following formats are supported.
//    (* indicates any sequence of non-digits)
//    (? indicates a single non-digit)
//
//     */yyyymmdd/g_hhmmss/f_llllllll.ext
//     */yyyymmdd/hhmmss.ext
//     */swp.yyymmddhhmmss (dorade) 2000 and later
//     */swp.yymmddhhmmss (dorade) 1999 and before
//     */*yyyymmdd?hhmmss*//
//     */*yyyymmddhhmmss*
//     */*yyyymmddhhmm*
//     */*yyyymmddhh.tmhhmm (mm5 forecast)
//     */*yyyymmddhh*
//     */*yyyyjjjhhmm*
//     */*yyyyjjjhh*
//     */*yyyymmdd?hhmm*
//     */*yyyymmdd?hh*
//     */*yyyymmdd*
//     */*yyjjj*
//
// Returns 0 on success, -1 on error.
// On success, sets file_time.

int DataFileNames::getDataTime(const string& file_path,
             time_t &data_time,
             bool &date_only,
             bool gen_time)
{

  data_time = 0;
  date_only = false;
  
  date_time_t ftime;
  int lead_time;

  // Copy the filename

  char fcopy[MAX_PATH_LEN];
  STRcopy(fcopy, file_path.c_str(), MAX_PATH_LEN);

  // find the last 3 path delimiters in the path

  int delim_len = strlen(PATH_DELIM);
  char *delim1 = NULL;
  char *delim2 = NULL;
  char *delim3 = NULL;
  char *delim4 = strstr(fcopy, PATH_DELIM);

  while (delim4 != NULL) {
    delim1 = delim2;
    delim2 = delim3;
    delim3 = delim4;
    delim4 = strstr(delim3 + delim_len, PATH_DELIM);
  }

  // find the starting characters of the last 3 directories in the path

  char *start1 = NULL;
  char *start2 = NULL;
  char *start3 = NULL;

  if (delim3 == NULL) {
    start3 = fcopy;
  } else if (delim2 == NULL) {
    start3 = delim3 + delim_len;
    start2 = fcopy;
  } else if (delim1 == NULL) {
    start3 = delim3 + delim_len;
    start2 = delim2 + delim_len;
    start1 = fcopy;
  } else {
    start3 = delim3 + delim_len;
    start2 = delim2 + delim_len;
    start1 = delim1 + delim_len;
  }


  // Extract the filename from the path

  if (start1 != NULL) {

    // try */yyyymmdd/g_hhmmss/f_llllllll*

    if (sscanf(start1, "%4d%2d%2d",
	       &ftime.year, &ftime.month, &ftime.day) == 3) {
      if (sscanf(start2, "g_%2d%2d%2d",
		 &ftime.hour, &ftime.min, &ftime.sec) == 3) {
	if (sscanf(start3, "f_%8d", &lead_time) == 1) {
	  uconvert_to_utime(&ftime);
          if (gen_time)
             data_time = ftime.unix_time;
          else
	     data_time = ftime.unix_time + lead_time;
	  return 0;
	}
      }
    }

  } // if (start1 != NULL)
	       
  // try */yyyymmdd/hhmmss*
  
  if (start2 != NULL) {
    if (sscanf(start2, "%4d%2d%2d",
	       &ftime.year, &ftime.month, &ftime.day) == 3) {
      if (sscanf(start3, "%2d%2d%2d",
		 &ftime.hour, &ftime.min, &ftime.sec) == 3) {
	// make sure we have an hour and not a date
	if (strlen(start3) == 6 || !isdigit(start3[6])) {
	  uconvert_to_utime(&ftime);
	  data_time = ftime.unix_time;
	  return 0;
	}
      }
    }
  }

  // check for DORADE (polar radar) files
  // for 2000 or after DORADE has 3 digits for year

  if (sscanf(start3, "swp.%3d%2d%2d%2d%2d%2d.",
             &ftime.year, &ftime.month, &ftime.day,
             &ftime.hour, &ftime.min, &ftime.sec) == 6) {
    ftime.year += 1900;
    uconvert_to_utime(&ftime);
    data_time = ftime.unix_time;
    return 0;
  }

  // DORADE for 1999 or before has 2 digits for year

  if (sscanf(start3, "swp.%2d%2d%2d%2d%2d%2d.",
             &ftime.year, &ftime.month, &ftime.day,
             &ftime.hour, &ftime.min, &ftime.sec) == 6) {
    ftime.year += 1900;
    uconvert_to_utime(&ftime);
    data_time = ftime.unix_time;
    return 0;
  }

  // move start3 forward until we get a digit

  char *firstDigit = start3;
  while (!isdigit(*firstDigit)) {
    firstDigit++;
  }
  if (strlen(firstDigit) < 6) {
    return -1;
  }

  // find the number of digits before the first non-digit

  char *ccc = firstDigit;
  while (isdigit(*ccc)) {
    ccc++;
  }
  int digitsLen1 = ccc - firstDigit;
  
  // find the number of digits before the second non-digit

  int digitsLen2 = 0;
  if (*ccc != '\0') {
    ccc++;
    while (isdigit(*ccc)) {
      ccc++;
    }
    digitsLen2 = ccc - firstDigit - digitsLen1 - 1;
  }

  // try *yyyymmdd?hhmmss* (? is single non-digit)
  
  if (digitsLen1 == 8 && digitsLen2 == 6) {
    char cc;
    if (sscanf(firstDigit, "%4d%2d%2d%1c%2d%2d%2d",
	       &ftime.year, &ftime.month, &ftime.day, &cc,
	       &ftime.hour, &ftime.min, &ftime.sec) == 7) {
      if (!isdigit(cc)) {
	uconvert_to_utime(&ftime);
	data_time = ftime.unix_time;
	return 0;
      }
    }
  }

  // try *yymmdd?hhmmss* (? is single non-digit)
  
  if (digitsLen1 == 6 && digitsLen2 == 6) {
    char cc;
    if (sscanf(firstDigit, "%2d%2d%2d%1c%2d%2d%2d",
	       &ftime.year, &ftime.month, &ftime.day, &cc,
	       &ftime.hour, &ftime.min, &ftime.sec) == 7) {

      ftime.year = ftime.year + 2000;

      if (!isdigit(cc)) {
	uconvert_to_utime(&ftime);
	data_time = ftime.unix_time;
	return 0;
      }
    }
  }

  // try *yyyymmddhhmmss*

  if (digitsLen1 == 14) {
    if (sscanf(firstDigit, "%4d%2d%2d%2d%2d%2d",
	       &ftime.year, &ftime.month, &ftime.day,
	       &ftime.hour, &ftime.min, &ftime.sec) == 6) {
      uconvert_to_utime(&ftime);
      data_time = ftime.unix_time;
      return 0;
    }
  }

  // try *yyyymmddhhmm*

  if (digitsLen1 == 12) {
    if (sscanf(firstDigit, "%4d%2d%2d%2d%2d",
	       &ftime.year, &ftime.month, &ftime.day,
	       &ftime.hour, &ftime.min) == 5) {
      ftime.sec = 0;
      uconvert_to_utime(&ftime);
      data_time = ftime.unix_time;
      return 0;
    }
  }
  
  // try *yyyymmddhh.tmhhmm
  
  if (digitsLen1 == 10) {
    int lead_hr;
    int lead_min;
    if (sscanf(firstDigit, "%4d%2d%2d%2d.tm%2d%2d",
	       &ftime.year, &ftime.month, &ftime.day,
	       &ftime.hour, &lead_hr, &lead_min) == 6) {
      ftime.min = 0;
      ftime.sec = 0;
      uconvert_to_utime(&ftime);
      data_time = ftime.unix_time + lead_hr * 3600 + lead_min * 60;
      return 0;
    }
  }
  
  // try *yyyymmddhh*
  
  if (digitsLen1 == 10) {
    if (sscanf(firstDigit, "%4d%2d%2d%2d",
	       &ftime.year, &ftime.month, &ftime.day,
	       &ftime.hour) == 4) {
      ftime.min = 0;
      ftime.sec = 0;
      uconvert_to_utime(&ftime);
      data_time = ftime.unix_time;
      return 0;
    }
  }

  // try *yyyyjjjhhmm*
  
  if (digitsLen1 == 11) {
    int julday;
    if (sscanf(firstDigit, "%4d%3d%2d%2d",
	       &ftime.year, &julday, &ftime.hour, &ftime.min) == 4) {
      ftime.month = 1;
      ftime.day = 1;
      ftime.sec = 0;
      uconvert_to_utime(&ftime);
      data_time = ftime.unix_time + (julday - 1) * SECS_IN_DAY;
      return 0;
    }
  }
  
  // try *yyyyjjjhh*
  
  if (digitsLen1 == 9) {
    int julday;
    if (sscanf(firstDigit, "%4d%3d%2d",
	       &ftime.year, &julday, &ftime.hour) == 3) {
      ftime.month = 1;
      ftime.day = 1;
      ftime.min = 0;
      ftime.sec = 0;
      uconvert_to_utime(&ftime);
      data_time = ftime.unix_time + (julday - 1) * SECS_IN_DAY;
      return 0;
    }
  }
  
  // try *yyyymmdd?hhmm* (? is single non-digit)

  if (digitsLen1 == 8 && digitsLen2 == 4) {
    char cc;
    if (sscanf(firstDigit, "%4d%2d%2d%1c%2d%2d",
	       &ftime.year, &ftime.month, &ftime.day,
	       &cc, &ftime.hour, &ftime.min) == 6) {
      ftime.sec = 0;
      uconvert_to_utime(&ftime);
      data_time = ftime.unix_time;
      return 0;
    }
  }

  // try *yyyymmdd?hh* (? is single non-digit)

  if (digitsLen1 == 8 && digitsLen2 == 2) {
    char cc;
    if (sscanf(firstDigit, "%4d%2d%2d%1c%2d",
	       &ftime.year, &ftime.month, &ftime.day,
	       &cc, &ftime.hour) == 5) {
      ftime.min = 0;
      ftime.sec = 0;
      uconvert_to_utime(&ftime);
      data_time = ftime.unix_time;
      return 0;
    }
  }

  // try *yyyymmdd*

  if (digitsLen1 == 8) {
    if (sscanf(firstDigit, "%4d%2d%2d",
	       &ftime.year, &ftime.month, &ftime.day) == 3) {
      ftime.hour = 12;
      ftime.min = 0;
      ftime.sec = 0;
      date_only = true;
      uconvert_to_utime(&ftime);
      data_time = ftime.unix_time;
      return 0;
    }
  }

  // try *yyjjj*
  
  if (digitsLen1 == 5) {
    int yy;
    int julday;
    if (sscanf(firstDigit, "%2d%3d",
	       &yy, &julday) == 2) {
      if (yy >= 90) {
	ftime.year = yy + 1900;
      } else {
	ftime.year = yy + 2000;
      }
      ftime.month = 1;
      ftime.day = 1;
      ftime.hour = 12;
      ftime.min = 0;
      ftime.sec = 0;
      date_only = true;
      uconvert_to_utime(&ftime);
      data_time = ftime.unix_time + (julday  - 1) * SECS_IN_DAY;
      return 0;
    }
  }
  
  // no luck

  return -1;

}

// Returns 0 on success, -1 on error.
// On success, sets file_time.

int DataFileNames::getDataTime(const string& file_path,
             const string &file_pattern,
             time_t &data_time,
             bool &date_only,
             bool gen_time)
{
  // store the file time here
  date_time_t ftime;

  const char *filePath    = file_path.c_str();
  const char *filePattern = file_pattern.c_str();
  int        lastIndex    = -1;
  int        parseStatus  = 0;

  // find the last directory separator in the filePath
  for( int i = 0; i < (int) strlen( filePath ); i++ )
  {
    if( filePath[i] == '/' )
    {
      lastIndex = i;
    }
  }

  // initialize the date to zero
  ftime.unix_time = 0; ftime.year = 1970; ftime.month = 1;
  ftime.day = 1; ftime.hour = 0; ftime.min = 0; ftime.sec = 0;
  
  // set a pointer to the beginning of the file name
  // fileName and filePattern should match at this point
  // e.g., fileName    -> 20101118.gfs.t18z.1bmhs.tm00.bufr_d
  //       filePattern -> YYYYMMDD.gfs.tHHz.1bmhs.tm00.bufr_d
  const char *fileName = ( filePath + lastIndex + 1 );
  const char *ptr;

  /* get the Year (YYYY) */
  ptr = strstr( filePattern, "YYYY" );
  if( ptr != NULL )
  {
    parseStatus = sscanf( fileName + ( ptr - filePattern ), "%4d", &ftime.year );
    if( parseStatus == 0 ) return -1; /* error parsing file */
  }

  /* get the Month (MM) */
  ptr = strstr( filePattern, "MM" );
  if( ptr != NULL )
  {
    parseStatus = sscanf( fileName + ( ptr - filePattern ), "%2d", &ftime.month );
    if( parseStatus == 0 ) return -1; /* error parsing file */
  }

  /* get the Day (DD) */
  ptr = strstr( filePattern, "DD" );
  if( ptr != NULL )
  {
    parseStatus = sscanf( fileName + ( ptr - filePattern ), "%2d", &ftime.day );
    if( parseStatus == 0 ) return -1; /* error parsing file */
  }

  /* get the Hour (HH) */
  ptr = strstr( filePattern, "HH" );
  if( ptr != NULL )
  {
    parseStatus = sscanf( fileName + ( ptr - filePattern ), "%2d", &ftime.hour );
    if( parseStatus == 0 ) return -1; /* error parsing file */
  }

  /* get the Minute (MN) */
  ptr = strstr( filePattern, "MN" );
  if( ptr != NULL )
  {
    parseStatus = sscanf( fileName + ( ptr - filePattern ), "%2d", &ftime.min );
    if( parseStatus == 0 ) return -1; /* error parsing file */
  }

  /* get the Second (SS) */
  ptr = strstr( filePattern, "SS" );
  if( ptr != NULL )
  {
    parseStatus = sscanf( fileName + ( ptr - filePattern ), "%2d", &ftime.sec );
    if( parseStatus == 0 ) return -1; /* error parsing file */
  }

  // convert to unix time and return success
  uconvert_to_utime(&ftime);
  data_time = ftime.unix_time;
  return 0;
}
