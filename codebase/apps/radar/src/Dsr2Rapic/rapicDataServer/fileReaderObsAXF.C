/*

  fileReaderObsAXF.C

*/

#include "fileReader.h"

void fileReaderObsAXF::init(char *initstr)
{
  fileReader::init(initstr);
}

void fileReaderObsAXF::readFile(char *fname)
{
  if (!fname)
    return;
  if (!obsAXFReader)
    {
      obsAXFReader = new readObsAXF();
    }
  if (obsAXFReader)
    {
      obsAXFReader->debug = debug;
      obsAXFReader->readFile(fname);
    }
}

char fileReaderObsAXFTitle[] = "fileReaderObsAXF::dumpStatus";

void fileReaderObsAXF::dumpStatus(FILE *dumpfile, char *title)
{
  if (!title)
    title = fileReaderObsAXFTitle;
  fileReader::dumpStatus(dumpfile, title);
  if (obsAXFReader)
    obsAXFReader->dumpStatus(dumpfile);
}
