/*

  file reader base class
  Allows file reader to use fam file event notification to
  trigger read

*/

#ifndef __FILEREADER__H
#define __FILEREADER__H

#include "rdrscan.h"
#include "readNexRad.h"
#include "readObsAXF.h"
#include "readLightningAXF.h"

enum fileReaderType {
  frt_Undefined, frt_Rapic, frt_Nexrad, frt_ObsAXF, frt_LightningAXF, frt_LAST
};
// if new fileReaderType added - MUST add string to 
// fileReader.C fileReaderTypeStrings
extern char *fileReaderTypeStrings[];

class fileReader {
 public:
  fileReader(char *initstr = 0)
    {
      readerType = frt_Undefined;
    };      
  virtual ~fileReader() {};
  fileReaderType readerType;
  int debug;  
  virtual char* typeStr()
    {
      if ((readerType < frt_Undefined) ||
	  (readerType >= frt_LAST))
	readerType = frt_Undefined;
      return fileReaderTypeStrings[readerType];
    };
  virtual void init(char *initstr);
  virtual void readFile(char *fname) 
    {};
  virtual void readData(void *ptr, size_t size, size_t nmemb, void *stream)
    {};
  virtual void dumpStatus(FILE *dumpfile, char *title = NULL);
  
};

class fileReaderRapic : public fileReader {
 public:
  fileReaderRapic(char *initstr = 0) :
    fileReader(initstr)
    {
      readerType = frt_Rapic;
      init(initstr);
    };
  virtual void init(char *initstr);
  // do the event operations
  virtual void readFile(char *fname);
  virtual void dumpStatus(FILE *dumpfile, char *title = NULL);
};

class fileReaderNexrad : public fileReader {
 public:
  fileReaderNexrad(char *initstr = 0) :
    fileReader(initstr)
    {
      readerType = frt_Nexrad;
      nexradReader = NULL;
      radarType = "CINRAD";
      elTolerance = 0.4;
      stnid = 0;
      init(initstr);
    };
  virtual ~fileReaderNexrad()
    {
      if (nexradReader)
	delete nexradReader;
    };
  virtual void init(char *initstr);
  // do the event operations
  virtual void readFile(char *fname);
  virtual void dumpStatus(FILE *dumpfile, char *title);
  readNexRad *nexradReader;
  int stnid;
  string radarType;
  float elTolerance;
};

class fileReaderObsAXF : public fileReader {
 public:
  fileReaderObsAXF(char *initstr = 0) :
    fileReader(initstr)
    {
      readerType = frt_ObsAXF;
      obsAXFReader = NULL;
      init(initstr);
    };
  virtual ~fileReaderObsAXF()
    {
      if (obsAXFReader)
	delete obsAXFReader;
    };
  virtual void init(char *initstr);
  // do the event operations
  virtual void readFile(char *fname);
  virtual void dumpStatus(FILE *dumpfile, char *title = NULL);
  readObsAXF *obsAXFReader;
};

class fileReaderLightningAXF : public fileReader {
 public:
  fileReaderLightningAXF(char *initstr = 0) :
    fileReader(initstr)
    {
      readerType = frt_LightningAXF;
      lightningAXFReader = NULL;
      init(initstr);
    };
  virtual ~fileReaderLightningAXF()
    {
      if (lightningAXFReader)
	delete lightningAXFReader;
    };
  virtual void init(char *initstr);
  // do the event operations
  virtual void readFile(char *fname);
  virtual void dumpStatus(FILE *dumpfile, char *title = NULL);
  readLightningAXF *lightningAXFReader;
};


#endif
