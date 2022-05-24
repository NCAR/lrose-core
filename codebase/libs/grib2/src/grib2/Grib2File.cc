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
// Grib2File - Main class for manipulating GRIB files.
////////////////////////////////////////////

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <grib2/Grib2File.hh>
#include <toolsa/file_io.h>
#include <toolsa/str.h>

#define EDITION_LOCATION 7
#define GRIB2 2

using namespace std;

namespace Grib2 {

Grib2File::Grib2File()
{
  _filePath = "";
  _filePtr = NULL;
  _fileContentsRead = false;
  _last_file_action = CONSTRUCT;
}

Grib2File::~Grib2File() 
{
  // Close any open files

  if (_filePtr != 0)
    fclose(_filePtr);

  // Reclaim memory

  vector< file_inventory_t >::iterator inventory;
  
  for (inventory = _inventory.begin(); inventory != _inventory.end();
       ++inventory)
    delete inventory->record;
  
}

void Grib2File::_setFilePath(const string &new_file_path)
{
  // If the file path really isn't changing, don't do anything

  if (new_file_path == _filePath)
    return;
  
  // Close the old file if it was open

  if (_filePtr != 0)
  {
    fclose(_filePtr);
    _filePtr = 0;
  }
  
  // Indicate that we haven't read the contents of this file

  _fileContentsRead = false;
  
  // Now set the new path

  _filePath = new_file_path;
}


void Grib2File::clearInventory()
{
  // First reclaim memory

  vector< file_inventory_t >::iterator inventory;
  
  for (inventory = _inventory.begin(); inventory != _inventory.end(); ++inventory)
    delete inventory->record;
  
  // The clear out the vector

  _inventory.erase(_inventory.begin(), _inventory.end());

  _filePath = "";
  _last_file_action = CLEAR;
}

int Grib2File::read(const string &file_path)
{
  static const string method_name = "Grib2File::read()";
  
  // Clear out the current inventory so we can create a new one

  clearInventory();
  
  // Determine the input file path

  if (file_path != "")
    _setFilePath(file_path);
  
  if (_filePath == "")
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "No input file path specified" << endl;
    
    return GRIB_FAILURE;
  }
  
  // Don't reread the file

  if (_fileContentsRead)
    return GRIB_SUCCESS;
  
  
  // Open the input file

  if ((_filePtr = ta_fopen_uncompress(_filePath.c_str(), "r")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening input GRIB file: " << _filePath << endl;
    perror(_filePath.c_str());
    
    return GRIB_FAILURE;
  }

  char *uncompressPath = STRdup(_filePath.c_str());
  char *ext = uncompressPath + strlen(uncompressPath) - 2;
  if (!strncmp(ext, ".Z", 2)) {
    *ext = '\0';
  }

  ext = uncompressPath + strlen(uncompressPath) - 3; 
  if (!strncmp(ext, ".gz", 3)) {
    *ext = '\0';
  }

  ext = uncompressPath + strlen(uncompressPath) - 4;
  if (!strncmp(ext, ".bz2", 4)) {
    *ext = '\0';
  }

  // Determine the input file size
  struct stat file_stat;
  if (stat(uncompressPath, &file_stat) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error stat'ing input GRIB file." << endl;
    perror(_filePath.c_str());
    
    return GRIB_FAILURE;
  }
  
  ui64 file_size = file_stat.st_size;

  // Read the input file into a local buffer

  ui08 *grib_contents = new ui08[file_size];
  int bytes_read;
  
  if ((bytes_read = fread(grib_contents, sizeof(ui08), file_size, _filePtr))
      != (int)file_size)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading contents of GRIB file: " << _filePath << endl;
    cerr << "Expected " << file_size << " bytes" << endl;
    cerr << "Read " << bytes_read << " bytes." << endl;
    
    delete [] grib_contents;
    fclose(_filePtr);
    _filePtr = 0;
    
    return GRIB_FAILURE;
  }
  
  fclose(_filePtr);
  _filePtr = 0;
  
  // Unpack the contents of the GRIB file

  ui08 *grib_ptr = grib_contents;
  int rec_num = 0;

  while (grib_ptr < grib_contents + file_size)
  {
    bool record_found = false;
    
    // some non-standard grib2 records have WMO headers
    while (grib_ptr < grib_contents + file_size && !record_found) {
    if (grib_ptr[0] == 'G' &&
          grib_ptr[1] == 'R' &&
          grib_ptr[2] == 'I' &&
          grib_ptr[3] == 'B')
      {
        record_found = true;
        break;
      }
     
      ++grib_ptr;
    }
 
    if (!record_found)
      break;

    file_inventory_t inventory;

    ui08 edition_num = grib_ptr[EDITION_LOCATION];

    if (edition_num != GRIB2) {
      cerr << "ERROR: reading edition number " << endl;
      cerr << "       Illegal number is " << (int) edition_num << endl;
      cerr << "       Not a GRIB2 record, exiting " << endl;
      delete[] grib_contents;
      return GRIB_FAILURE;
    }
    else
      inventory.record = new Grib2Record();
    
    if (inventory.record->unpack(&grib_ptr, file_size - (grib_ptr - grib_contents)) != GRIB_SUCCESS)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error unpacking record in grib file" << endl;
      delete[] grib_contents;
      return GRIB_FAILURE;
    }
    
    
    _inventory.push_back(inventory);

    rec_num++;

  }
  
  delete [] grib_contents;
  
  _fileContentsRead = true;
  _last_file_action = READ;

  return GRIB_SUCCESS;
}

void Grib2File::printSummary(FILE *stream, int debug) const
{
  vector< file_inventory_t >::const_iterator inventory;
  int rec_num = 1, fields_num = 0;
  
  for (inventory = _inventory.begin(); inventory != _inventory.end();
       ++inventory) {
    if(rec_num < 10)
      fprintf(stream, "%d  ", rec_num);
    else
      if(rec_num < 100)
	fprintf(stream, "%d ", rec_num);
      else
	fprintf(stream, "%d", rec_num);

    fields_num += inventory->record->printSummary(stream, debug);
    
    rec_num++;
  }
  fprintf(stream, "total number of records processed %d\n", rec_num-1);
  fprintf(stream, "total number of fields processed %d \n", fields_num);
}

list <string> Grib2File::getFieldList()
{
  vector< file_inventory_t >::const_iterator inventory;
  list <string> fields;
  for (inventory = _inventory.begin(); inventory != _inventory.end(); ++inventory) {
    list <string> recordFields = inventory->record->getFieldList();
    list <string>::const_iterator field;
    for (field = recordFields.begin(); field != recordFields.end(); ++field) {
      fields.push_back(*(field));
    }
  }
  fields.sort();
  fields.unique();
  return fields;
}

list <string> Grib2File::getFieldLevels(const string &fieldName)
{
  vector< file_inventory_t >::const_iterator inventory;
  list <string> levels;
  for (inventory = _inventory.begin(); inventory != _inventory.end(); ++inventory) {
    list <string> recordLevels = inventory->record->getFieldLevels(fieldName);
    list <string>::const_iterator level;
    for (level = recordLevels.begin(); level != recordLevels.end(); ++level) {
      levels.push_back(*(level));
    }
  }
  levels.sort();
  levels.unique();
  return levels;
}

list <long int> Grib2File::getForecastList()
{
  vector< file_inventory_t >::const_iterator inventory;
  list <long int> times;
  for (inventory = _inventory.begin(); inventory != _inventory.end(); ++inventory) {
    list <long int> recordTimes = inventory->record->getForecastList();
    list <long int>::const_iterator time;
    for (time = recordTimes.begin(); time != recordTimes.end(); ++time) {
      times.push_back(*(time));
    }
  }
  times.sort();
  times.unique();
  return times;
}

vector <Grib2Record::Grib2Sections_t>  Grib2File::getRecords(const string &fieldName, 
                                                  const string &level, const long int &leadTime)
{
  vector <Grib2Record::Grib2Sections_t> recordsFound;

  vector< file_inventory_t >::const_iterator inventory;

  for (inventory = _inventory.begin(); inventory != _inventory.end(); ++inventory) {
    if (inventory->record->recordMatches (fieldName, level)) {
      vector <Grib2Record::Grib2Sections_t> foundRecords = inventory->record->getRecords (fieldName, level, leadTime);

      for(size_t a = 0;  a < foundRecords.size(); a++) {
	recordsFound.push_back(foundRecords[a]);
      }
    }
  }

  return recordsFound;
}

void Grib2File::printContents(FILE *stream, Grib2Record::print_sections_t printSec) const
{
  vector< file_inventory_t >::const_iterator inventory;
  int rec_num = 0, fields_num = 0;
  
  for (inventory = _inventory.begin(); inventory != _inventory.end();
       ++inventory) {
    fields_num += inventory->record->print(stream, printSec);
    rec_num++;
  }
  fprintf(stream, "total number of records processed %d \n", rec_num);
  fprintf(stream, "total number of fields processed %d \n", fields_num);
}

void Grib2File::print(FILE *stream)
{
  vector< file_inventory_t >::const_iterator inventory;
  
  for (inventory = _inventory.begin(); inventory != _inventory.end();
       ++inventory)
    inventory->record->print(stream);
}

int Grib2File::create(si32 disciplineNumber, time_t referenceTime, si32 referenceTimeType, 
		      si32 typeOfData, si32 generatingSubCentreID, si32 generatingCentreID, 
		      si32 productionStatus, si32 localTablesVersion, si32 masterTablesVersion)
{
  if(_last_file_action == CREATE ||
     _last_file_action == ADDLOCALUSE ||
     _last_file_action == ADDGRID)
  {
    cerr << "ERROR: Grib2File::create()" << endl;
    cerr << "Cannot create new Grib2Record untill previous record is finished." << endl;
    return GRIB_FAILURE;
  }

  file_inventory_t inventory;
  inventory.record = new Grib2Record(disciplineNumber, referenceTime, referenceTimeType, 
				     typeOfData, generatingSubCentreID, generatingCentreID, 
				     productionStatus, localTablesVersion, masterTablesVersion);

  _inventory.push_back(inventory);

  _last_file_action = CREATE;
  return GRIB_SUCCESS;
}

int Grib2File::addLocalUse(si32 dataSize, ui08 *localUseData)
{
  if(_last_file_action == CREATE && !_inventory.empty())
  {
    file_inventory_t inventory = _inventory[_inventory.size()-1];
    inventory.record->setLocalUse(dataSize, localUseData);
    
    _last_file_action = ADDLOCALUSE;
    return GRIB_SUCCESS;
  }
  cerr << "ERROR: Grib2File::addLocalUse()" << endl;
  cerr << "Can only add a local use section after creating a new Grib2Record." << endl;
  return GRIB_FAILURE;
}

int Grib2File::addGrid(si32 numberDataPoints, si32 gridDefNum, GribProj *projectionTemplate)
{
  if((_last_file_action == CREATE || _last_file_action == ADDLOCALUSE ||
     _last_file_action == ADDFIELD)  && !_inventory.empty())
  {
    file_inventory_t inventory = _inventory[_inventory.size()-1];
    inventory.record->setGrid(numberDataPoints, gridDefNum, projectionTemplate);
    
    _last_file_action = ADDGRID;
    return GRIB_SUCCESS;
  }
  cerr << "ERROR: Grib2File::addGrid()" << endl;
  cerr << "Can only add a grid section after creating a new Grib2Record, " <<
    "adding a local use section or adding a data field." << endl;
  return GRIB_FAILURE;
}

int Grib2File::addField(si32 prodDefNum, ProdDefTemp *productTemplate, 
			si32 dataRepNum, DataRepTemp *dataRepTemplate,
			fl32 *data, si32 bitMapType, si32 *bitMap)
{
  if((_last_file_action == ADDFIELD || _last_file_action == ADDGRID)
     && !_inventory.empty())
  {
    file_inventory_t inventory = _inventory[_inventory.size()-1];
    if(inventory.record->addField(prodDefNum, productTemplate, dataRepNum, dataRepTemplate,
			       data, bitMapType, bitMap) == GRIB_FAILURE)
      return GRIB_FAILURE;

    _last_file_action = ADDFIELD;
    return GRIB_SUCCESS;
  }
  cerr << "ERROR: Grib2File::addField()" << endl;
  cerr << "Can only add a field after adding another field or  " <<
    "after adding the grid section." << endl;
  return GRIB_FAILURE;
}

int Grib2File::write(const string &file_path)
{
  static const string method_name = "Grib2File::write()";

  if((_last_file_action == ADDFIELD || _last_file_action == READ ||
      _last_file_action == WRITE) && !_inventory.empty())
  {
    // Determine the output file path

    if (file_path != "")
      _setFilePath(file_path);
    
    if (_filePath == "")
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "No output file path specified" << endl;
      
      return GRIB_FAILURE;
    }
    
    // Open the output file
    
    if ((_filePtr = fopen(_filePath.c_str(), "w")) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error opening output GRIB file." << endl;
      perror(_filePath.c_str());
      
      return GRIB_FAILURE;
    }
  
    // Write each of the records to the output file
    
    vector< file_inventory_t >::iterator inventory;
    
    for (inventory = _inventory.begin(); inventory != _inventory.end();
	 ++inventory)
    {
      // Construct the buffer to be written to the output file
      
      ui08 *grib_contents;
      
      if ((grib_contents = inventory->record->pack()) == 0)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error packing GRIB contents for output file: "
	     << _filePath << endl;
	
	return GRIB_FAILURE;
      }
      
      // Write the GRIB contents to the output file
      
      int bytes_written;
      int record_size = inventory->record->getRecordSize();
      
      if ((bytes_written = fwrite((void *)grib_contents, sizeof(ui08),
				  record_size, _filePtr))
	  != record_size)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error writing GRIB contents to output file." << endl;
	cerr << "Tried to write " << record_size << " bytes." << endl;
	cerr << "Really wrote " << bytes_written << " bytes." << endl;
	
	perror(_filePath.c_str());
	
	delete [] grib_contents;
	fclose(_filePtr);
	_filePtr = 0;
	
	return GRIB_FAILURE;
      }
      
      delete [] grib_contents;
    }
    
    fclose(_filePtr);
    _filePtr = 0;
    _last_file_action = WRITE;

    return GRIB_SUCCESS;
  }
  cerr << "ERROR: " << method_name << endl;
  cerr << "Cannot write out a uncompleted file." << endl;
  return GRIB_FAILURE;
}

} // namespace Grib2

