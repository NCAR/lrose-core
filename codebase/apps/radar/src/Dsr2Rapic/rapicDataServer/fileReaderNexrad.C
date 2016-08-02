/*

  fileReaderNexrad.C

*/

#include "fileReader.h"

void fileReaderNexrad::init(char *initstr)
{
  char *tempstr;
  char stnstr[512];
  float tempf;

  if ((tempstr = strstr(initstr, "radartype=CINRAD")))
    {
      radarType = "CINRAD";
      elTolerance = 0.4;
    }
  if ((tempstr = strstr(initstr, "radartype=NEXRAD")))
    {
      radarType = "NEXRAD";
      elTolerance = 0.1;
    }
  if ((tempstr = strstr(initstr, "station=")))
    {
      if (sscanf(tempstr, "station=%s", stnstr) == 1)
	stnid = decode_stnstr(stnstr);
    }
  if ((tempstr = strstr(initstr, "el_tolerance=")))
    {
      if (sscanf(tempstr, "el_tolerance=%f", &tempf) == 1)
	elTolerance = tempf;
    }
  fileReader::init(initstr);
}

void fileReaderNexrad::readFile(char *fname)
{
  if (!fname)
    return;
  char filestringflag[1024];
  strcpy(filestringflag, fname);
  strcat(filestringflag, ".done");
	
  if (!nexradReader)
    {
      nexradReader = new readNexRad();
      if (nexradReader)
	{
	  nexradReader->debug = debug;
	  nexradReader->elTolerance = elTolerance;
	}
    }
  if (nexradReader)
    {
      if (!FileExists(filestringflag))
	{
	  if (debug)
	    fprintf(stdout, "fileReaderNexrad::doEvent - converting %s\n",
		    fname);
	  nexradReader->ReadNexradData(fname, (char *)radarType.c_str(),
				       stnid, true);
	  if (nexradReader->createdScan &&
	      nexradReader->createdScan->HeaderValid() &&
	      nexradReader->createdScan->Finished() &&
	      ScanMng)
	    {
	      ScanMng->NewDataAvail(nexradReader->createdScan);
	      ScanMng->FinishedDataAvail(nexradReader->createdScan);
	    }
	  nexradReader->derefCreatedScan();
	  symlink(fname, filestringflag);
	}
      else
	if (debug)
	  fprintf(stdout, "fileReaderNexrad::doEvent - Skipping %s - previously converted\n",
		  fname);	
    }
}

char fileReaderNexradTitle[] = "fileReaderNexrad::dumpStatus";

void fileReaderNexrad::dumpStatus(FILE *dumpfile, char *title)
{
  if (!title)
    title = fileReaderNexradTitle;
  fileReader::dumpStatus(dumpfile, title);
  if (dumpfile)
    fprintf(dumpfile, "station=%s type=%s el_tolerance=%1.3f\n",
	    stn_name(stnid), radarType.c_str(), elTolerance);
}
