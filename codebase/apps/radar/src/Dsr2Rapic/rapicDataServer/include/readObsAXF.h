/*
  readObsAXF.h
  Class to parse axf format obs data into obsData class containers
  Passes fixedObsData instances to the obsDataMng singleton

  Reads Field declarations section to get fields available, names and 
  positions to locate required pre-defined field names

  This is a somewhat incomplete implementation, e.g. assumes "," delimiter
  and doesn't parse all of the meta data
*/

#ifndef __READAXFOBS_H
#define __READAXFOBS_H

#include <string>
#include <vector>
#include "obsData.h"

enum axfFieldTypes { axf_undefined, axf_string, axf_int, axf_float };

class axfFieldData
{
 public:
  axfFieldData();
  axfFieldData(char *fieldname, axfFieldTypes fieldtype, 
	       int fieldpos = -1);
  void set(char *fieldname, axfFieldTypes fieldtype, 
	   int fieldpos = -1);
  void init();
  std::string fieldName;
  axfFieldTypes fieldType;
  int fieldPos;
  bool valid;
  bool readFieldData(char *fieldstr, int pos); // read description line
                                           // pos is line no./field pos
  bool fieldNameMatch(char *matchstr);
};
  

class obsFieldOfsWid
{
 public:
  int ofs, wid;
};

class readObsAXF
{
 public:
  readObsAXF(char *axfname = NULL);
  virtual ~readObsAXF();
  void init();
  void setFName(char *axfname);
  std::string axfName;
  FILE *axfFile;
  std::vector<axfFieldData> fieldData;
  int nameF, stationF, stnF, siteF, timeF, latF, longF, htF, 
    wdF, wsF, wgF, spF, tmpF, dpF, r9F, r10F;
  int readFile(char *axfname = NULL,
	       time_t start_time = 0,
	       time_t end_time = 0); // rd file return # of obs, -1 on err
  void resetFPos();  // reset field positions
  bool readDescr();  // get field posns from AXF Description
  int readObs(time_t start_time = 0,
	      time_t end_time = 0);     // read AXF Surface obs entries
  bool readObsString(char *obsstr,
		     time_t start_time = 0,
		     time_t end_time = 0);
  bool readObsStringBase(std::string &obsString,
			 baseObsData *obs);
  bool readObsStringFixed(std::string &obsString,
			  fixedObsData *obs);
  bool readObsStringMoving(std::string &obsString,
			   movingObsData *obs);
  bool checkFPos(char *obsstring);  // check 1st obs line for field pos 
  std::vector<obsFieldOfsWid> fieldOfs; // char offsets of each field in an Obs line
  std::string delimiter;
  int fieldCount;
  time_t lastReadStartTime, thisReadStartTime,
    lastReadEndTime, thisReadEndTime;
  int lastReadObsCount,
    lastReadAddedCount;
  int debug;
  int getFieldOfs(std::string &obsString);  // get offsets for each field of obs str
                                     // results in fieldsOfs
  int getFieldInt(std::string &obsString, 
		  int fieldnum);     // returns int val for field
  float getFieldFloat(std::string &obsString, 
		      int fieldnum); // returns flaot value for field
  void getFieldString(std::string &obsString, 
		      int fieldnum, std::string &str); // copy string
  time_t getFieldTime(std::string &obsString, 
		    int fieldnum); // copy string
  void dumpStatus(FILE *dumpfile);
};

#endif
