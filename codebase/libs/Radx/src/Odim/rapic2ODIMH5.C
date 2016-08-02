/*
  NOTE - suggest using following to build stand alone rapic2ODIMH5 app to
  avoid conflicts with library version in librapic
  i.e. compile main version to rapic2ODIMH5.o instead of rapicODIMH5.o
g++ -c -g -DSTDCPPHEADERS -I../include -I../include/hdf5 rapic2ODIMH5.C
g++ -o rapic2ODIMH5 rapic2ODIMH5.o -L. -lrapicUtils -L../lib -lhdf5 -L/usr/lib -lpthread -lz
*/

#include "rdrscan.h"
#include "odimH5FileWriter.h"
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
std::string rapic2ODIMH5Version("1.5");
std::string rapicSuffix(".rapic");

int RP_ARGC = 0;
char **RP_ARGV = NULL;

odimH5FileWriter odimh5writer;

void build_rainfields_filename(rdr_scan* rpscan, std::string& hdfname)
{
  char buf[128];
  time_t scantime = rpscan->ScanTime();
  struct tm* utc = gmtime(&scantime);
  sprintf(
        buf
      , "%d%s_%04d%02d%02d_%02d%02d%02d.vol.h5"
      , rpscan->StnID()
      , "02" // this is the 'radar volume' product suffix used by rainfields 3
      , utc->tm_year + 1900
      , utc->tm_mon + 1
      , utc->tm_mday
      , utc->tm_hour
      , utc->tm_min
      , utc->tm_sec
      );
  hdfname.assign(buf);
}

bool appClosing()
{
  return true;
}

int convertRapicDB(char *rapicdbname)
{
  int fd = open(rapicdbname,O_RDONLY);

  if (fd < 0)
    {
      printf("Failed to open Rapic database %s - %s\n",
	     rapicdbname, strerror(errno));
      return -1;
    }

  int convertCount = 0;
  rdr_scan *rpscan = NULL;
  off64_t offset = 0, 
    endoffset = 0;
  long long size = -1;
  bool odimh5_ok = false;
  long long fsize = lseek(fd,0,SEEK_END);
  int readresult = 0;
  bool finished = false;
  char scanstring[128];
  

  while (!finished)
    {
      if (rpscan == NULL)
	rpscan = new rdr_scan(NULL, "rapicODIMH5File_convertRapicDB");
      readresult = rpscan->readRapicFile(fd, offset, endoffset, size);
      if ((readresult > 0) && rpscan->dataValid())
	{
          string hdfname;
          build_rainfields_filename(rpscan, hdfname);
	  odimh5_ok = odimh5writer.writeFile((char *)hdfname.c_str(), rpscan);
	  if (odimh5_ok)
	    {
	      printf("  Convert RapicFile to ODIM_H5File=%s succeeded\n",
		     hdfname.c_str());
	      convertCount++;
	    }
	  else
	    printf("  RapicFile=%s read OK, ODIM_H5File=%s write FAILED\n",
		   rpscan->scanFilename(scanstring), hdfname.c_str());
	}
      else
	printf("  Rapic database %s read FAILED at offset = %lld\n", 
	       rapicdbname, (long long)offset);
      offset = endoffset;    // get current offset
      size = -1;
 
      if (ShouldDelete(rpscan, NULL, "rapicODIMH5File_convertDirectory"))
	delete rpscan;
      rpscan = NULL;
      finished = off_t(endoffset) >= fsize-1;
    }
  close(fd);
  return convertCount;
}

int convertDirectory(char *dirname, bool floatFlag = false);

int convertDirectory(char *dirname, bool floatFlag)
{
  DIR *dir = NULL;

  struct dirent *ent = NULL;
  dir = opendir(dirname) ;
  if (!dir)
    {
      printf("Failed to open directory %s - %s\n",
	     dirname, strerror(errno));
      return -1;
    }

  //   for ( int ia = 0; ia < 2; ia++ ) {
  //     ent = readdir(dir) ;
  //   }
  
  string dirEntString;
  int convertCount = 0;
  rdr_scan *rpscan = NULL;
  bool odimh5_ok = false;
  while ((ent = readdir(dir)) != NULL)
    {
      dirEntString = dirname;
      if (dirEntString[dirEntString.size()-1] != '/')
	dirEntString += '/';
      dirEntString += ent->d_name;
      string::size_type suffixpos = dirEntString.find(rapicSuffix);
      if (suffixpos != string::npos)
	{
	  rpscan = new rdr_scan(NULL, "rapicODIMH5File_convertDirectory");
	  if (rpscan->readRapicFile((char *)dirEntString.c_str()) &&
	      rpscan->dataValid())
	    {
              string hdfname;
              build_rainfields_filename(rpscan, hdfname);
	      odimh5_ok = odimh5writer.writeFile((char *)hdfname.c_str(), rpscan);
	      if (odimh5_ok)
		{
		  printf("  Convert RapicFile=%s to ODIM_H5File=%s succeeded\n",
			 dirEntString.c_str(), hdfname.c_str());
		  convertCount++;
		}
	      else
		printf("  RapicFile=%s read OK, ODIM_H5File=%s write FAILED\n",
		       dirEntString.c_str(), hdfname.c_str());
	    }
	  else
	    printf("  RapicFile=%s read FAILED\n",
		   dirEntString.c_str());

	  if (ShouldDelete(rpscan, NULL, "rapicODIMH5File_convertDirectory"))
	    delete rpscan;
	  rpscan = NULL;
	}
    }
  closedir(dir);
  return convertCount;
}

int main(int argc, char *argv[])
{
  rdr_scan rpscan(NULL, "rapicODIMH5File_main");

  RP_ARGC = argc;
  RP_ARGV = argv;

  if (argc < 3)
    {
      printf("%s Version %s\n"
	     "Usage options:\n"
	     "%s rapic_filename odimh5_filename [-float]\n"
	     "%s -d rapic_dir_name [-float]\n"
	     "%s -rapicdb rapic_database_name [-float]\n"
	     "Other options may also be used in the last argument"
	     " separated by commas\n"
	     "  compressFactor=7 (range 1 to 9)\n"
	     "  compressChunkSize=10,100 (default 45,80)\n"
	     "  noSzip\n"
	     "  useSzip\n"
	     "  szipPixelsPerBlock=32 (default 16)\n\n",
	     argv[0], rapic2ODIMH5Version.c_str(), argv[0], argv[0], argv[0]);
      exit(1);
    }
  bool odimh5_ok = false;
  int odimFilesConverted = 0;
  
  if (argc > 3)
    {
      if (strstr(argv[3], "-float"))
	odimh5writer.writeFloat = true;
      char *tempstrptr = NULL;
      if ((tempstrptr = strstr(argv[3], "compressFactor=")))
	{
	  int compress;
	  if (sscanf(tempstrptr, "compressFactor=%d", &compress)
	       == 1)
	    odimh5writer.compressFactor = compress;
	}
      if ((tempstrptr = strstr(argv[3], "compressChunkSize=")))
	{
	  int cdim0, cdim1;
	  if (sscanf(tempstrptr, "compressChunkSize=%d,%d", &cdim0, &cdim1)
	      == 2)
	    {
	      odimh5writer.compressChunkSize[0] = cdim0;
	      odimh5writer.compressChunkSize[1] = cdim1;
	    }
	}
      if ((tempstrptr = strstr(argv[3], "useSzip")))
	odimh5writer.useSzip = true;
      if ((tempstrptr = strstr(argv[3], "noSzip")))
	odimh5writer.useSzip = false;
      if ((tempstrptr = strstr(argv[3], "szipPixelsPerBlock=")))
	{
	  int szippixels;
	  if (sscanf(tempstrptr, "szipPixelsPerBlock=%d", &szippixels)
	      == 1)
	    {
	      odimh5writer.szipPixelsPerBlock = szippixels;
	    }
	  odimh5writer.useSzip = true;
	}
    }
  if (strcmp(argv[1], "-rapicdb") == 0)
    { 
      odimFilesConverted = convertRapicDB(argv[2]);
      odimh5_ok = odimFilesConverted > 0;
    }
  if (strcmp(argv[1], "-d") == 0)
    { 
      odimFilesConverted = convertDirectory(argv[2]);
      odimh5_ok = odimFilesConverted > 0;
    }
  else
    {
      string rapicfname = argv[1];
      string odimh5fname = argv[2];
      if (rpscan.readRapicFile(argv[1]))
	{
	  odimh5_ok = odimh5writer.writeFile(argv[2], &rpscan);
	  if (odimh5_ok)
	    {
	      printf("%s convert RapicFile=%s to ODIM_H5File=%s succeeded\n",
		     argv[0], argv[1], argv[2]);
	      odimFilesConverted++;
	    }
	  else
	    printf("%s RapicFile=%s read OK, ODIM_H5File=%s write FAILED\n",
		   argv[0], argv[1], argv[2]);
	}
      else
	printf("%s RapicFile=%s read FAILED\n",
	       argv[0], argv[1]);
    }
  printf("%s Rapic files converted count = %d\n",
	 argv[0], odimFilesConverted);
  exit(!odimh5_ok);
}
