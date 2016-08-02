/*

  fileReaderRapic.C

*/

#include "fileReader.h"

void fileReaderRapic::init(char *initstr)
{
  fileReader::init(initstr);
}

void fileReaderRapic::readFile(char *fname)
{
  if (!fname)
    return;
  rdr_scan *newscan = new rdr_scan(this, "fileReaderRapic");
  newscan->readRapicFile(fname);
  // treat same as COMM source, default is DB which will be ignored by DBMngr
  newscan->data_source = COMM;   
  if (newscan->HeaderValid() &&
      newscan->Finished() &&
      ScanMng)
    {
      ScanMng->NewDataAvail(newscan);
      ScanMng->FinishedDataAvail(newscan);
    }
  if (newscan->ShouldDelete(this, "fileReaderRapic"))
    delete newscan;
}

char fileReaderRapicTitle[] = "fileReaderRapic::dumpStatus";

void fileReaderRapic::dumpStatus(FILE *dumpfile, char *title)
{
  if (!title)
    title = fileReaderRapicTitle;
  fileReader::dumpStatus(dumpfile, title);
}
