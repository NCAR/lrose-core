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
// GribFile - Main class for manipulating GRIB files.
//
////////////////////////////////////////////

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>

#include <grib/constants.h>
#include <grib/GribFile.hh>
#include <toolsa/file_io.h>

using namespace std;

GribFile::GribFile() :
  _filePath(""),
  _filePtr(0),
  _fileContentsRead(false),
  _fileContentsInventoried(false)
{
}

GribFile::~GribFile() 
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

void GribFile::setFilePath(const string &new_file_path)
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
  _fileContentsInventoried = false;
  
  // Now set the new path

  _filePath = new_file_path;
}

//int GribFile::createInventory(const string &file_path)
//{
//  static const string method_name = "GribFile::createInventory()";
//  
//  // Determine the input file path
//
//  if (file_path != "")
//    setFilePath(file_path);
//  
//  if (_filePath == "")
//  {
//    cerr << "ERROR: " << method_name << endl;
//    cerr << "No input file path specified" << endl;
//    
//    return GRIB_FAILURE;
//  }
//  
//  // Don't reinventory the file
//
//  if (_fileContentsInventoried)
//    return GRIB_SUCCESS;
//  
//  // Clear out the current inventory so we can create a new one
//
//  _clearInventory();
//  
//  // Determine the input file size
//
//  struct stat file_stat;
//  
//  if (stat(_filePath.c_str(), &file_stat) != 0)
//  {
//    cerr << "ERROR: " << method_name << endl;
//    cerr << "Error stat'ing input GRIB file." << endl;
//    perror(_filePath.c_str());
//    
//    return GRIB_FAILURE;
//  }
//  
//  int file_size = file_stat.st_size;
//  
//  // Open the input file
//
//  if ((_filePtr = ta_fopen_uncompress(_filePath.c_str(), "r")) == 0)
//  {
//    cerr << "ERROR: " << method_name << endl;
//    cerr << "Error opening input GRIB file: " << _filePath << endl;
//    perror(_filePath.c_str());
//    
//    return GRIB_FAILURE;
//  }
//  
//  // Read the input file into a local buffer
//
//  ui08 *grib_contents = new ui08[file_size];
//  int bytes_read;
//  
//  if ((bytes_read = fread(grib_contents, sizeof(ui08), file_size, _filePtr))
//      != file_size)
//  {
//    cerr << "ERROR: " << method_name << endl;
//    cerr << "Error reading contents of GRIB file: " << _filePath << endl;
//    cerr << "Expected " << file_size << " bytes" << endl;
//    cerr << "Read " << bytes_read << " bytes." << endl;
//    
//    delete [] grib_contents;
//    fclose(_filePtr);
//    _filePtr = 0;
//    
//    return GRIB_FAILURE;
//  }
//  
//  fclose(_filePtr);
//  _filePtr = 0;
//
//  // Inventory the file
//  ui08 *grib_ptr = grib_contents;
//  
//  while (grib_ptr < grib_contents + file_size)
//  {
//    // Find the beginning of the grib record
//
//    bool record_found = false;
//    
//    while (grib_ptr < grib_contents + file_size &&
//	   !record_found)
//    {
//      if (grib_ptr[0] == 'G' &&
//	  grib_ptr[1] == 'R' &&
//	  grib_ptr[2] == 'I' &&
//	  grib_ptr[3] == 'B')
//      {
//	record_found = true;
//	break;
//      }
//      
//      ++grib_ptr;
//    }
//    
//    if (!record_found)
//      break;
//    
//    file_inventory_t inventory;
//    
//    inventory.record_start = grib_ptr - grib_contents;
//    inventory.record = 0;
//
//    if (GribRecord::inventory(&grib_ptr) != GRIB_SUCCESS)
//    {
//      cerr << "ERROR: " << method_name << endl;
//      cerr << "Error inventorying record in grib file" << endl;
//      
//      return GRIB_FAILURE;
//    }
//    
//    inventory.record_len = inventory.record->getRecordSize();
//    
//    _inventory.push_back(inventory);
//  }
//  
//  delete [] grib_contents;
//  
//  _fileContentsInventoried = true;
//  
//  return GRIB_SUCCESS;
//}

int GribFile::read(const string &file_path)
{
  static const string method_name = "GribFile::read()";
  
  // Determine the input file path

  if (file_path != "")
    setFilePath(file_path);
  
  if (_filePath == "")
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "No input file path specified" << endl;
    
    return GRIB_FAILURE;
  }
  
  // Don't reread the file

  if (_fileContentsRead)
    return GRIB_SUCCESS;
  
  // Clear out the current inventory so we can create a new one

  _clearInventory();
  
  // Determine the input file size

  struct stat file_stat;
  
  if (stat(_filePath.c_str(), &file_stat) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error stat'ing input GRIB file." << endl;
    perror(_filePath.c_str());
    
    return GRIB_FAILURE;
  }
  
  int file_size = file_stat.st_size;
  
  // Open the input file

  if ((_filePtr = ta_fopen_uncompress(_filePath.c_str(), "r")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening input GRIB file: " << _filePath << endl;
    perror(_filePath.c_str());
    
    return GRIB_FAILURE;
  }
  
  // Read the input file into a local buffer

  ui08 *grib_contents = new ui08[file_size];
  int bytes_read;
  
  if ((bytes_read = fread(grib_contents, sizeof(ui08), file_size, _filePtr))
      != file_size)
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
  
  while (grib_ptr < grib_contents + file_size)
  {
    // Find the beginning of the grib record

    bool record_found = false;
    
    while (grib_ptr < grib_contents + file_size &&
	   !record_found)
    {
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
    
    inventory.record_start = grib_ptr - grib_contents;
    inventory.record = new GribRecord();
    
    if (inventory.record->unpack(&grib_ptr) != GRIB_SUCCESS)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error unpacking record in grib file" << endl;
      
      return GRIB_FAILURE;
    }
    
    inventory.parameter_id = inventory.record->getParameterId();
    inventory.level_id = inventory.record->getLevelId();
    
    inventory.record_len = inventory.record->getRecordSize();
    
    _inventory.push_back(inventory);
  }
  
  delete [] grib_contents;
  
  _fileContentsRead = true;
  _fileContentsInventoried = true;
  
  return GRIB_SUCCESS;
}

void GribFile::printInventory(ostream &stream) const
{
  stream << "Grib file inventory:" << endl;
  
  vector< file_inventory_t >::const_iterator inventory_record;
  
  for (inventory_record = _inventory.begin();
       inventory_record != _inventory.end(); ++inventory_record)
    stream << "   start = " << inventory_record->record_start
	   << ", length = " << inventory_record->record_len
	   << ", parameter id = " << inventory_record->parameter_id
	   << ", level_id = " << inventory_record->level_id << endl;
}

int GribFile::write(const string &file_path)
{
  static const string method_name = "GribFile::write()";
  
  // Determine the output file path

  if (file_path != "")
    setFilePath(file_path);
  
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
  
  return GRIB_SUCCESS;
}

void GribFile::print(FILE *stream, const bool print_bitmap_flag,
		     const bool print_data_flag,
		     const bool print_min_datum_flag,
		     const bool print_max_datum_flag) const
{
  vector< file_inventory_t >::const_iterator inventory;
  
  for (inventory = _inventory.begin(); inventory != _inventory.end();
       ++inventory)
    inventory->record->print(stream, print_bitmap_flag, print_data_flag,
			     print_min_datum_flag, print_max_datum_flag);
}


void GribFile::print(ostream &stream, const bool print_bitmap_flag,
		     const bool print_data_flag,
		     const bool print_min_datum_flag,
		     const bool print_max_datum_flag) const
{
  vector< file_inventory_t >::const_iterator inventory;
  
  for (inventory = _inventory.begin(); inventory != _inventory.end();
       ++inventory)
    inventory->record->print(stream, print_bitmap_flag, print_data_flag,
			     print_min_datum_flag, print_max_datum_flag);
}


void GribFile::_clearInventory()
{
  // First reclaim memory

  vector< file_inventory_t >::iterator inventory;
  
  for (inventory = _inventory.begin(); inventory != _inventory.end();
       ++inventory)
    delete inventory->record;
  
  // The clear out the vector

  _inventory.erase(_inventory.begin(), _inventory.end());
}
