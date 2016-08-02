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
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/Path.hh>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
   
#include "GrowingFile.hh"
using namespace std;

GrowingFile::GrowingFile(const Params &params,
                         string fileName, 
                         time_t fileTime):
        _params(params),
        pFilePtr(NULL),
        pFileName(fileName),
        pFileTime(fileTime),
        pOffset(0)
{
  
}

GrowingFile::~GrowingFile()
{
  if (pFilePtr != NULL)
  {
    fclose(pFilePtr);

    pFilePtr = NULL;
  }
}

int GrowingFile::open() 
{
  //
  // Open the file or return error indicator
  // 
  if ( (pFilePtr = fopen(pFileName.c_str(), "rb")) == NULL)
  {
    fprintf(stderr," GrowingFile::open(): Failed to open %s\n", 
	    pFileName.c_str());
   
    return 1;
  }
  else
  {
    //
    // Success
    // 
    return 0;
  }
}

bool GrowingFile::process( bool &deleteFlag)
{
  //
  // See if the file has grown.
  //

  if (stat(pFileName.c_str(),&pStats))
  {
    fprintf(stderr,"GrowingFile::process(): Failed to stat %s\n", 
	    pFileName.c_str());

    //
    // Set delete indicator for file
    //
    deleteFlag = true;

    //
    // Return no processing indicator
    //
    return false;
  } 
  else
  {
    if (pOffset < pStats.st_size)
    { 
      //
      // File has grown. Write the new data to a file.
      //
      if (_params.Debug)
      {
	fprintf(stderr,"GrowingFile::process(): %s: File has grown.\n", 
		pFileName.c_str());
      }

      pWriteData();
      
      pOffset = pStats.st_size;

      deleteFlag = false;

      return true;
    } 
    else 
    {
      if (_params.Debug) 
      {
	fprintf(stderr,"GrowingFile::process(): %s: File has not grown.\n", 
		pFileName.c_str());
      }
      //
      // Check the time since last mod. If > pModTimeThresh
      // Set delete flag to true, otherwise set to false
      //
      time_t now = time(0);

      if ( (float) (now - pFileTime)/3600 > _params.MaxGap)
      {
	deleteFlag = true;

	fprintf(stderr,"GrowingFile::process(): %s: Filetime is older than %d "
	       "hours. Closing file.\n", pFileName.c_str(),
                _params.MaxGap);
	
	pClose();
      }
      return false;
    }
  }
}

void GrowingFile::pClose() 
{
  if (pFilePtr)
  {
    fclose(pFilePtr);
   
    pFilePtr = NULL;
  }
}

int GrowingFile::pWriteData() 
{

  //
  // Seek to the first byte we have not processed.
  //
  long begin, length;

  if (pOffset == 0)
  { 
    //
    // It points to an unprocessed byte.
    //
    begin = 0;

    length = pStats.st_size - pOffset + 1;
  } 
  else 
  {
    //
    // The byte pointed at by offset has already been processed.
    //
    begin = pOffset + 1;
    
    length = pStats.st_size - pOffset;
  }

  if (fseek(pFilePtr, begin, SEEK_SET))
  {
    perror("GrowingFile::pWriteData(): Seek (and write) failed ");
    
    return 1;
  }

  //
  // Allocate memory for new contents of file.
  //
  unsigned char *buffer;

  buffer = new unsigned char[length];

  if (buffer == NULL)
  {
    fprintf(stderr,"New failed.\n");
    
    return 1;
  }
  
  //
  // Read in data.
  //
  fread((void*)buffer, sizeof(unsigned char), length, pFilePtr);

  //
  // Put together the output file name.
  //
  char outputPath[MAX_PATH_LEN];
  date_time_t t;
  ugmtime(&t);
  
  if (_params.OutHasDayDir) {
    sprintf(outputPath,              
            "%s%s%.d%.2d%.2d%s%s.%d%02d%02d%02d%02d%02d",
            _params.OutDir, PATH_DELIM,
            t.year, t.month, t.day, PATH_DELIM,
            _params.OutPrecursor,
            t.year,t.month,t.day,
            t.hour, t.min, t.sec);
  } else {
    sprintf(outputPath,              
            "%s%s%s.%d%02d%02d%02d%02d%02d",
            _params.OutDir, PATH_DELIM, _params.OutPrecursor,
            t.year,t.month,t.day,
            t.hour, t.min, t.sec);
  }
    
  // make directory for output path
  Path path(outputPath);
  if (path.makeDirRecurse()) {
    cerr << "ERROR - GrowingFile::pWriteData()" << endl;
    cerr << "  Cannot make dir for path: " << outputPath << endl;
  }

  FILE *outFile;

  //
  // Open append in case the file already exists
  //
  if ( (outFile = fopen(outputPath,"ab")) == NULL)
  {
    fprintf(stderr,"GrowingFile::pWriteData(): Failed to create %s\n",outputPath);
   
    return 1;
  }

  if (_params.Debug)
  {
    fprintf(stderr,"GrowingFile::pWriteData(): Appending to file %s\n", 
	    outputPath);
  }

  fwrite(buffer, sizeof(unsigned char), length, outFile);

  delete(buffer); 

  fclose(outFile);

  if (_params.WriteLdataFile) {

    string relPath;
    Path::stripDir(_params.OutDir, outputPath, relPath);
    
    DsLdataInfo dsLdataInfo;
    dsLdataInfo.setDebug(_params.Debug);
    dsLdataInfo.setDir(_params.OutDir);
    dsLdataInfo.setDataFileExt(path.getExt());
    dsLdataInfo.setWriter("LdmDynamic2Static");
    dsLdataInfo.setRelDataPath(relPath); 
    dsLdataInfo.setUserInfo1(_params.OutPrecursor); 
    dsLdataInfo.setUserInfo2(relPath); 
    dsLdataInfo.write(t.unix_time);

  }

  sleep(2); // no files will be growing in output

  return 0;

}


