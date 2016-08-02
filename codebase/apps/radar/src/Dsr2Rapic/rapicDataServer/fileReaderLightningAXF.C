/*

  fileReaderLightningAXF.C

*/

#include "fileReader.h"

void fileReaderLightningAXF::init(char *initstr)
{
  fileReader::init(initstr);
}

void fileReaderLightningAXF::readFile(char *fname)
{
  if (!fname)
    return;
  if (!lightningAXFReader)
    {
      lightningAXFReader = new readLightningAXF();
    }
  if (lightningAXFReader)
    {
      lightningAXFReader->debug = debug;
      lightningAXFReader->readFile(fname);
    }
}

char fileReaderLightningAXFTitle[] = "fileReaderLightningAXF::dumpStatus";

void fileReaderLightningAXF::dumpStatus(FILE *dumpfile, char *title)
{
  if (!title)
    title = fileReaderLightningAXFTitle;
  fileReader::dumpStatus(dumpfile, title);
  if (lightningAXFReader)
    lightningAXFReader->dumpStatus(dumpfile);
}
