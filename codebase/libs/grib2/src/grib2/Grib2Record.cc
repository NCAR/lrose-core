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
/////////////////////////////////////////////
// Grib2Record - Main class for manipulating GRIB records.
////////////////////////////////////////////

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <grib2/constants.h>
#include <grib2/Grib2Record.hh>
#include <grib2/LocalUseSec.hh>
#include <grib2/DRS.hh>
#include <grib2/GDS.hh>
#include <grib2/BMS.hh>
#include <grib2/PDS.hh>
#include <grib2/DS.hh>

using namespace std;

namespace Grib2 {

Grib2Record::Grib2Record()
{

}

Grib2Record::Grib2Record(g2_si32 disciplineNumber, time_t referenceTime, g2_si32 referenceTimeType, 
			 g2_si32 typeOfData, g2_si32 generatingSubCentreID, g2_si32 generatingCentreID, 
			 g2_si32 productionStatus, g2_si32 localTablesVersion, g2_si32 masterTablesVersion)
{
  _is.setEditionNum(2);
  _is.setDisciplineNum(disciplineNumber);
  _ids.setCenterId(generatingCentreID);
  _ids.setSubCenterId(generatingSubCentreID);
  _ids.setGenerateTime(referenceTime);
  _ids.setGenerateTimeType(referenceTimeType);
  _ids.setProductionStatus(productionStatus);
  _ids.setProccesedDataType(typeOfData);
  _ids.setLocalTableVersion(localTablesVersion);
  _ids.setMasterTableVersion(masterTablesVersion);
}

Grib2Record::~Grib2Record() 
{
  // Reclaim memory from sequences of GRIB sections 2 to 7, sections 
  // 3 to 7 or sections 4 to 7 may be repeated within a single GRIB message. 
  list <LocalUseSec *> lus;
  list <GDS  *> gds;
  list <PDS *> pds;
  list <DRS *> drs;
  list <BMS *> bms;
  list <DS *> ds;
  vector < repeatSections_t >::iterator RS;

  for (RS = _repeatSec.begin(); RS != _repeatSec.end(); ++RS) {
    lus.push_back(RS->lus);
    gds.push_back(RS->gds);
    pds.push_back(RS->pds);
    drs.push_back(RS->drs);
    bms.push_back(RS->bms);
    ds.push_back(RS->ds);
  }
  lus.sort();
  lus.unique();
  list <LocalUseSec *>::const_iterator lus_iter;
  for (lus_iter = lus.begin(); lus_iter != lus.end(); ++lus_iter) {
    if(*(lus_iter) != NULL)
      delete *(lus_iter);
  }

  gds.sort();
  gds.unique();
  list <GDS *>::const_iterator gds_iter;
  for (gds_iter = gds.begin(); gds_iter != gds.end(); ++gds_iter) {
    if(*(gds_iter) != NULL)
      delete *(gds_iter);
  }

  pds.sort();
  pds.unique();
  list <PDS *>::const_iterator pds_iter;
  for (pds_iter = pds.begin(); pds_iter != pds.end(); ++pds_iter) {
    if(*(pds_iter) != NULL)
      delete *(pds_iter);
  }

  drs.sort();
  drs.unique();
  list <DRS *>::const_iterator drs_iter;
  for (drs_iter = drs.begin(); drs_iter != drs.end(); ++drs_iter) {
    if(*(drs_iter) != NULL)
      delete *(drs_iter);
  }

  bms.sort();
  bms.unique();
  list <BMS *>::const_iterator bms_iter;
  for (bms_iter = bms.begin(); bms_iter != bms.end(); ++bms_iter) {
    if(*(bms_iter) != NULL)
      delete *(bms_iter);
  }

  ds.sort();
  ds.unique();
  list <DS *>::const_iterator ds_iter;
  for (ds_iter = ds.begin(); ds_iter != ds.end(); ++ds_iter) {
    if(*(ds_iter) != NULL)
      delete *(ds_iter);
  }
}


int Grib2Record::unpack(g2_ui08 **file_ptr, g2_ui64 file_size)
{
  g2_ui08 *section_ptr = *file_ptr;

  int return_value;
  g2_ui64 current_len = 0;

  // Unpack the Indicator Section
  if ((return_value = _is.unpack(section_ptr))
      != GRIB_SUCCESS)
  {
    cerr << "ERROR: Grib2Record::unpack()" << endl;
    cerr << "Cannot unpack Indicator Section" << endl;
    return return_value;
  }

  if(_is.getTotalSize() > file_size)
  {
    cerr << "ERROR: Grib2Record::unpack()" << endl;
    cerr << "Indicated file size is bigger than actual file, message may be incomplete." << endl;
    cerr << "Indicated Size: " << _is.getTotalSize() << " Actual size: " << file_size << endl;
    return GRIB_FAILURE;
  }

  section_ptr += _is.getSize();
  current_len += _is.getSize();

  // unpack the Identification Section
  if ((return_value = _ids.unpack(section_ptr)) != GRIB_SUCCESS) {
    cerr << "ERROR: Grib2Record::unpack()" << endl;
    cerr << "Cannot unpack Identification Section" << endl;
    return return_value;
  }
  
  section_ptr += _ids.getSize();
  current_len += _ids.getSize();

  g2_si32 lus_size = 0;
  g2_si32 gds_size = 0;
  g2_si32 pds_size = 0;
  g2_si32 drs_size = 0;
  g2_si32 bms_size = 0;
  g2_si32 ds_size = 0;

  g2_si32 *prevBitMap = NULL;
  g2_si32 prevBitMapSize = 0;

  repeatSections_t last_RS;
  last_RS.lus = NULL;
  last_RS.gds = NULL;
  last_RS.pds = NULL;
  last_RS.drs = NULL;
  last_RS.bms = NULL;
  last_RS.ds = NULL;

  Grib2Sections_t sectionsPtr;
  sectionsPtr.is = &_is;
  sectionsPtr.ids = &_ids;
  sectionsPtr.lus = NULL;
  sectionsPtr.gds = NULL;
  sectionsPtr.pds = NULL;
  sectionsPtr.drs = NULL;
  sectionsPtr.bms = NULL;
  sectionsPtr.ds = NULL;
  sectionsPtr.es = &_es;
  sectionsPtr.summary = NULL;

  while (current_len < _is.getTotalSize() - 4) {

    // RS contains LocalUseSec, GDS, PDS, DRS, BMS and DS
    repeatSections_t RS = last_RS;

    if ((g2_si32) section_ptr[4] == 2) {
      RS.lus = new LocalUseSec();
      // unpack the Local Use Section  -> optional
      if ((return_value = RS.lus->unpack(section_ptr)) != GRIB_SUCCESS) {
	cerr << "ERROR: Grib2Record::unpack()" << endl;
        cerr << "Cannot unpack Local Use Section" << endl;
        return return_value;
      }
      sectionsPtr.lus = RS.lus;
      section_ptr += RS.lus->getSize();
      current_len += RS.lus->getSize();
      lus_size++;

    } 

    if ((g2_si32) section_ptr[4] == 3) {
      RS.gds = new GDS();
      // unpack the Grid Definition Section
      if ((return_value = RS.gds->unpack(section_ptr)) != GRIB_SUCCESS) {
	cerr << "ERROR: Grib2Record::unpack()" << endl;
        cerr << "Cannot unpack Grid Definition Section" << endl;
        return return_value;
      }
      sectionsPtr.gds = RS.gds;
      section_ptr += RS.gds->getSize();
      current_len += RS.gds->getSize();

      gds_size++;
    }
  
    if ((g2_si32) section_ptr[4] == 4) {
      RS.pds = new PDS(sectionsPtr);
      // Unpack the Product Definition Section
      if ((return_value = RS.pds->unpack(section_ptr)) != GRIB_SUCCESS) {
	cerr << "ERROR: Grib2Record::unpack()" << endl;
        cerr << "Cannot unpack Product Definition section" << endl;
        return return_value;
      }
      sectionsPtr.pds = RS.pds;
      section_ptr += RS.pds->getSize();
      current_len += RS.pds->getSize();

      pds_size++;
    }

    if ((g2_si32) section_ptr[4] == 5) {
      RS.drs = new DRS(sectionsPtr);
      // Unpack the Data Representation Section
      if ((return_value = RS.drs->unpack(section_ptr)) != GRIB_SUCCESS) {
	cerr << "ERROR: Grib2Record::unpack()" << endl;
        cerr << "Cannot unpack Data Representation Section" << endl;
        return return_value;
      }
      sectionsPtr.drs = RS.drs;
      section_ptr += RS.drs->getSize();
      current_len += RS.drs->getSize();
  
      drs_size++;

    }

    if ((g2_si32) section_ptr[4] == 6) {
      RS.bms = new BMS(254, prevBitMapSize, prevBitMap);
      // Unpack the Bit-map section
      if ((return_value = RS.bms->unpack(section_ptr)) != GRIB_SUCCESS) {   
	cerr << "ERROR: Grib2Record::unpack()" << endl;
        cerr << "Cannot unpack Bit-map Section" << endl;
        return return_value;
      }
      sectionsPtr.bms = RS.bms;
      section_ptr += RS.bms->getSize();
      current_len += RS.bms->getSize();

      bms_size++;
    }

    if ((g2_si32) section_ptr[4] == 7) {
      RS.ds = new DS(sectionsPtr);
      // Unpack the data section
      if ((return_value = RS.ds->unpack(section_ptr)) != GRIB_SUCCESS) {   
	cerr << "ERROR: Grib2Record::unpack()" << endl;
        cerr << "Cannot unpack Data Section" << endl;
        return return_value;
      }
      sectionsPtr.ds = RS.ds;
      section_ptr += RS.ds->getSize();
      current_len += RS.ds->getSize();

      ds_size++;
    }

    rec_summary_t summary;
    RS.pds->getRecSummary(&summary);
    RS.summary = summary;
    sectionsPtr.summary = &(RS.summary);

    _repeatSec.push_back(RS);
    // it is possible in repeat sections to use a bit map from the previous section
    prevBitMap = RS.bms->getBitMap();
    prevBitMapSize = RS.bms->getBitMapSize();
    last_RS = RS;

  }
  // Unpack the End Section
  if ((return_value = _es.unpack(section_ptr)) != GRIB_SUCCESS) {
    cerr << "ERROR: Grib2Record::unpack()" << endl;
    cerr << "Cannot unpack End Section" << endl;
    return return_value;
  }
  
  section_ptr += _es.getSize();
  current_len += _es.getSize();
  
  *file_ptr = section_ptr;

  return GRIB_SUCCESS;
}

g2_ui08 *Grib2Record::pack()
// The caller is responsible for freeing the memory allocated here.
{
  g2_ui64 total_len = 0;
  total_len += _is.getSize();
  total_len += _ids.getSize();

  vector < repeatSections_t >::iterator RS;
  for (RS = _repeatSec.begin(); RS != _repeatSec.end(); ++RS) {
    if(RS == _repeatSec.begin() || (RS-1)->lus != RS->lus)
      if(RS->lus != NULL)
	total_len += RS->lus->getSize();
    if(RS == _repeatSec.begin() || (RS-1)->gds != RS->gds)
      total_len += RS->gds->getSize();
    total_len += RS->pds->getSize();
    total_len += RS->drs->getSize();
    total_len += RS->bms->getSize();
    total_len += RS->ds->getSize();
  }
  total_len += _es.getSize();


  g2_ui08 *grib_contents = new g2_ui08[total_len];
  g2_ui08 *section_ptr = grib_contents;
  g2_ui64 current_len = 0;

  _is.setTotalSize(total_len);
  if(_is.pack(section_ptr) != GRIB_SUCCESS) {
    delete[] grib_contents;
    return NULL;
  }
  section_ptr += _is.getSize();
  current_len += _is.getSize();

  if(_ids.pack(section_ptr) != GRIB_SUCCESS) {
    delete[] grib_contents;
    return NULL;
  }
  section_ptr += _ids.getSize();
  current_len += _ids.getSize();

  for (RS = _repeatSec.begin(); RS != _repeatSec.end(); ++RS) {

    if(RS == _repeatSec.begin() || (RS-1)->lus != RS->lus) {
      if(RS->lus != NULL) {
	if(RS->lus->pack(section_ptr) != GRIB_SUCCESS) {
	  delete[] grib_contents;
	  return NULL;
	}
	section_ptr += RS->lus->getSize();
	current_len += RS->lus->getSize();
      }
    }

    if(RS == _repeatSec.begin() || (RS-1)->gds != RS->gds) {
      if(RS->gds->pack(section_ptr) != GRIB_SUCCESS) {
	delete[] grib_contents;
	return NULL;
      }
      section_ptr += RS->gds->getSize();
      current_len += RS->gds->getSize();
    }

    if(RS->pds->pack(section_ptr) != GRIB_SUCCESS) {
      delete[] grib_contents;
      return NULL;
    }
    section_ptr += RS->pds->getSize();
    current_len += RS->pds->getSize();

    if(RS->drs->pack(section_ptr) != GRIB_SUCCESS) {
      delete[] grib_contents;
      return NULL;
    }
    section_ptr += RS->drs->getSize();
    current_len += RS->drs->getSize();

    if(RS->bms->pack(section_ptr) != GRIB_SUCCESS) {
      delete[] grib_contents;
      return NULL;
    }
    section_ptr += RS->bms->getSize();
    current_len += RS->bms->getSize();

    if(RS->ds->pack(section_ptr) != GRIB_SUCCESS) {
      delete[] grib_contents;
      return NULL;
    }
    section_ptr += RS->ds->getSize();
    current_len += RS->ds->getSize();
  }
  if(current_len + 4 != total_len) {
    cerr << "ERROR: Grib2Record::pack()" << endl;
    cerr << "Cannot pack Grib2Record, estimated size != packed size" << endl;
    delete[] grib_contents;
    return NULL;
  }

  _es.pack(section_ptr);

  return grib_contents;
}


// print the indicator section only
void Grib2Record::printIs(FILE *stream) {
   _is.print(stream);
}

// print the Identification section only
void Grib2Record::printIds(FILE *stream) {
   _ids.print(stream);
}

// print the local use section only
void Grib2Record::printLus(FILE *stream) {

  vector < repeatSections_t >::iterator RS;
  
  for (RS = _repeatSec.begin(); RS != _repeatSec.end(); ++RS) {
    if(RS->lus != NULL)
      RS->lus->print(stream);
  }
}

// print the grid definition section only
void Grib2Record::printGds(FILE *stream) {

  vector < repeatSections_t >::iterator RS;
  
  for (RS = _repeatSec.begin(); RS != _repeatSec.end(); ++RS) {
    if(RS->gds != NULL)
      RS->gds->print(stream);
  }
}

// print the Product definition section only
void Grib2Record::printPds(FILE *stream) {

  vector < repeatSections_t >::iterator RS;
  
  for (RS = _repeatSec.begin(); RS != _repeatSec.end(); ++RS) {
    if(RS->pds != NULL)
      RS->pds->print(stream);
  }
}

// print the data representation section only
void Grib2Record::printDrs(FILE *stream) {

  vector < repeatSections_t >::iterator RS;
  
  for (RS = _repeatSec.begin(); RS != _repeatSec.end(); ++RS) {
    if(RS->drs != NULL)
      RS->drs->print(stream);
  }
}

// print the bit map section only
void Grib2Record::printBms(FILE *stream) {

  vector < repeatSections_t >::iterator RS;
  
  for (RS = _repeatSec.begin(); RS != _repeatSec.end(); ++RS) {
    if(RS->bms != NULL)
      RS->bms->print(stream);
  }
}


// print the data section only
void Grib2Record::printDs(FILE *stream) {

  vector < repeatSections_t >::iterator RS;
  
  for (RS = _repeatSec.begin(); RS != _repeatSec.end(); ++RS) {
    if(RS->ds != NULL)
      RS->ds->print(stream);
  }
}

list <string> Grib2Record::getFieldList()
{
  list <string> fields;
  vector < repeatSections_t >::iterator RS;

  for (RS = _repeatSec.begin(); RS != _repeatSec.end(); ++RS) {
    string field(RS->summary.name);
    fields.push_back(field);
  }
  fields.sort();
  fields.unique();
  return fields;
}

list <string> Grib2Record::getFieldLevels(const string &fieldName)
{
  list <string> levels;
  vector < repeatSections_t >::iterator RS;

  for (RS = _repeatSec.begin(); RS != _repeatSec.end(); ++RS)
    if(fieldName.compare(RS->summary.name) == 0) {
      string level(RS->summary.levelType);
      levels.push_back(level);
    }
  
  levels.sort();
  levels.unique();
  return levels;
}

list <long int> Grib2Record::getForecastList()
{
  list <long int> times;
  vector < repeatSections_t >::iterator RS;

  for (RS = _repeatSec.begin(); RS != _repeatSec.end(); ++RS) {
    long int time(RS->pds->getForecastTime());
    times.push_back(time);
  }
  times.sort();
  times.unique();
  return times;
}

// get records matching those with attributes in the argument list
vector <Grib2Record::Grib2Sections_t> Grib2Record::getRecords(const string &fieldName, const string &level,
							      const long int &leadTime)
{
  vector < repeatSections_t >::iterator RS;
  vector <Grib2Sections_t> matches;

  for (RS = _repeatSec.begin(); RS != _repeatSec.end(); ++RS) {

    if ((fieldName.compare(RS->summary.name) == 0) && 
	(level.compare(RS->summary.levelType) == 0) &&
	(leadTime == -99 || RS->pds->getForecastTime() == leadTime)) {
      Grib2Sections_t match;
      match.is = &_is;
      match.ids = &_ids;
      match.lus = RS->lus;
      match.gds = RS->gds;
      match.pds = RS->pds;
      match.drs = RS->drs;
      match.bms = RS->bms;
      match.ds = RS->ds;
      match.summary = &(RS->summary);
      match.es = &_es;
      matches.push_back(match);
    }
  }

  return matches;
}

// Determine if there are any records matching those with attributes in the argument list
bool Grib2Record::recordMatches (const string &fieldName, const string &level) {

  vector < repeatSections_t >::iterator RS;
  
  for (RS = _repeatSec.begin(); RS != _repeatSec.end(); ++RS) {

    if ((fieldName.compare(RS->summary.name) == 0) && 
                  (level.compare(RS->summary.levelType) == 0)) {
      return (true);
    }
  }
  return (false);

}

// print record summary (Product information)
int Grib2Record::printSummary(FILE *stream, int debug) {

  int count = 0;
  vector < repeatSections_t >::iterator RS;

  fprintf(stream, " %04d%02d%02d", _ids.getYear(),_ids.getMonth(), _ids.getDay());
  if( _ids.getMin() == 0 && _ids.getSec() == 0)
      fprintf(stream, " %02dZ", _ids.getHour());
    else
      fprintf(stream, " %02d%02d%02d",_ids.getHour(),_ids.getMin(), _ids.getSec());

  for (RS = _repeatSec.begin(); RS != _repeatSec.end(); ++RS) {
    if (count > 0) {
      fprintf(stream, "\n\t\t\t+%s  %s ", RS->summary.forecastTime.c_str(), RS->summary.name.c_str());
    }
    else
      fprintf(stream, "\t+%s  %s ", RS->summary.forecastTime.c_str(), RS->summary.name.c_str());

    if(debug > 0)
      fprintf(stream, "[%d,%d,%d]", RS->summary.discipline,
    	      RS->summary.category, RS->summary.paramNumber);

    fprintf(stream, " '%s'", RS->summary.longName.c_str());
    fprintf(stream, " (%s) ", RS->summary.units.c_str());

    fprintf(stream, "  %s", RS->summary.levelType.c_str());
    if(RS->summary.levelUnits.compare("-") != 0) {
      if(ceil(RS->summary.levelVal) == RS->summary.levelVal)
	fprintf(stream, " %d", (int)RS->summary.levelVal);
      else
	fprintf(stream, " %f", RS->summary.levelVal);
      if(RS->summary.levelVal2 != -999)
      {
	if(ceil(RS->summary.levelVal2) == RS->summary.levelVal2)
	  fprintf(stream, "-%d", (int)RS->summary.levelVal2);
	else
	  fprintf(stream, "-%f", RS->summary.levelVal2);
      }
    }
    fprintf(stream, " '%s'", RS->summary.levelTypeLong.c_str());
    if(RS->summary.levelUnits.compare("-") != 0)
      fprintf(stream, " (%s)", RS->summary.levelUnits.c_str());

    fprintf(stream, " %s", RS->summary.additional.c_str());

    count++;
  }

  fprintf(stream, "\n");
  return count;
}


void Grib2Record::print(FILE *stream)
{
  vector < repeatSections_t >::iterator RS;
  
  _is.print(stream);           // Indicator Section
  _ids.print(stream);          // Identification Section
  for (RS = _repeatSec.begin(); RS != _repeatSec.end(); ++RS) {
    if(RS->lus != NULL)
      RS->lus->print(stream);      // Local Use Section (optional)
    if(RS->gds != NULL)
      RS->gds->print(stream);      // Grid Definition Section
    if(RS->pds != NULL)
      RS->pds->print(stream);      // Product Definition Section
    if(RS->drs != NULL)
      RS->drs->print(stream);      // Data Representation Section
    if(RS->bms != NULL)
      RS->bms->print(stream);      // Bit-map Section
    if(RS->ds != NULL)
      RS->ds->print(stream);       // Data Section
  }
}

int Grib2Record::print(FILE *stream, print_sections_t printSec)
{
  vector < repeatSections_t >::iterator RS;
  int fields_num = 0;
  if (printSec.is)
    _is.print(stream);
  if (printSec.ids)
    _ids.print(stream);
  for (RS = _repeatSec.begin(); RS != _repeatSec.end(); ++RS) {
    if (printSec.lus && RS->lus != NULL)
      if(RS == _repeatSec.begin() || (RS-1)->lus != RS->lus)
	RS->lus->print(stream);
    if (printSec.gds && RS->gds != NULL)
      if(RS == _repeatSec.begin() || (RS-1)->gds != RS->gds)
	RS->gds->print(stream);
    if (printSec.pds && RS->pds != NULL)
      RS->pds->print(stream);
    if (printSec.drs && RS->drs != NULL)
      RS->drs->print(stream);
    if (printSec.bms && RS->bms != NULL)
      RS->bms->print(stream);
    if (printSec.ds && RS->ds != NULL)
      RS->ds->print(stream);
    fields_num ++;
  }
  fprintf(stream, "total number of fields in this record %d \n", fields_num);
  return fields_num;
}

void Grib2Record::setLocalUse(g2_si32 dataSize, g2_ui08 *localUseData)
{
  repeatSections_t RS;
  RS.gds = NULL;
  RS.pds = NULL;
  RS.drs = NULL;
  RS.bms = NULL;
  RS.ds = NULL; 
  RS.lus = new LocalUseSec(dataSize, localUseData);
  _repeatSec.push_back(RS);

}

void Grib2Record::setGrid(g2_si32 numberDataPoints, g2_si32 gridDefNum, GribProj *projectionTemplate)
{
  if(!_repeatSec.empty()) {
    repeatSections_t &RS = _repeatSec[_repeatSec.size()-1];
    if(RS.gds == NULL) {
      RS.gds = new GDS(numberDataPoints, gridDefNum, projectionTemplate);
      return;
    }
  }
  repeatSections_t RS;
  RS.lus = NULL;
  RS.gds = new GDS(numberDataPoints, gridDefNum, projectionTemplate);
  RS.pds = NULL;
  RS.drs = NULL;
  RS.bms = NULL;
  RS.ds = NULL; 
  _repeatSec.push_back(RS);
}

int Grib2Record::addField(g2_si32 prodDefNum, ProdDefTemp *productTemplate, 
			   g2_si32 dataRepNum, DataRepTemp *dataRepTemplate,
			   g2_fl32 *data, g2_si32 bitMapType, g2_si32 *bitMap)
{
  // If the repeatSec vector is empty caller didnt call setGrid first.
  if(!_repeatSec.empty()) 
  {
    // If the gds doesn't exist caller didnt call setGrid first.
    if(_repeatSec[_repeatSec.size()-1].gds != NULL) 
    {
      // If the pds already exists this is not the first field
      // for this gds.  Create a new repeat Sections for this field.
      if(_repeatSec[_repeatSec.size()-1].pds != NULL)
      {
	repeatSections_t RS_;
	RS_.lus = _repeatSec[_repeatSec.size()-1].lus;
	RS_.gds = _repeatSec[_repeatSec.size()-1].gds;
	RS_.pds = NULL;
	RS_.drs = NULL;
	RS_.bms = NULL;
	RS_.ds = NULL; 
	_repeatSec.push_back(RS_);

      }

      repeatSections_t &RS = _repeatSec[_repeatSec.size()-1];

      Grib2Sections_t sectionsPtr;
      sectionsPtr.is = &_is;
      sectionsPtr.ids = &_ids;
      sectionsPtr.lus = RS.lus;
      sectionsPtr.gds = RS.gds;
      sectionsPtr.pds = NULL;
      sectionsPtr.drs = NULL;
      sectionsPtr.bms = NULL;
      sectionsPtr.ds = NULL;
      sectionsPtr.es = &_es;
      sectionsPtr.summary = NULL;

      g2_si32 numDataPoints = RS.gds->getNumDataPoints();

      RS.pds = new PDS(sectionsPtr, prodDefNum, productTemplate);
      productTemplate->setParamStrings();
      sectionsPtr.pds = RS.pds;

      RS.drs = new DRS(sectionsPtr, dataRepNum, dataRepTemplate);
      sectionsPtr.drs = RS.drs;

      if(bitMapType == 254) 
      {
	if(_repeatSec.size() > 1)
	{
	  repeatSections_t &prev_RS = _repeatSec[_repeatSec.size()-2];
	  g2_si32 *prevBitMap = prev_RS.bms->getBitMap();
	  g2_si32  prevBitMapSize = prev_RS.bms->getBitMapSize();
	  if(prevBitMapSize == numDataPoints) 
	  {
	    RS.bms = new BMS(bitMapType, numDataPoints, prevBitMap);
	  } else {
	    cerr << "ERROR: Grib2Record::addField()" << endl;
	    cerr << "Previously defined bit map code but previously defined bit map " <<
	      "is not the same size." << endl;
	    RS.bms = new BMS(255, numDataPoints, NULL);
	  }
	} else {
	  cerr << "ERROR: Grib2Record::addField()" << endl;
	  cerr << "Previously defined bit map code but no previously defined bit map." << endl;
	  RS.bms = new BMS(255, numDataPoints, NULL);
	}
      } else
	RS.bms = new BMS(bitMapType, numDataPoints, bitMap);
      sectionsPtr.bms = RS.bms;

      RS.ds = new DS(sectionsPtr);
      if(RS.ds->encode(data) == GRIB_FAILURE)
	return GRIB_FAILURE;
      sectionsPtr.ds = RS.ds;

      rec_summary_t summary;
      RS.pds->getRecSummary(&summary);
      RS.summary = summary;
      sectionsPtr.summary = &(RS.summary);
      return GRIB_SUCCESS;
    } else {
      cerr << "ERROR: Grib2Record::addField()" << endl;
      cerr << "Must call addGrid before addField." << endl;
      return GRIB_FAILURE;
    }
  } else {
    cerr << "ERROR: Grib2Record::addField()" << endl;
    cerr << "Must call addGrid before addField." << endl;
    return GRIB_FAILURE;
  }

}

} // namespace Grib2

