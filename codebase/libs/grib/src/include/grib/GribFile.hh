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

#ifndef GribFile_HH
#define GribFile_HH

#include <string>
#include <vector>

#include <grib/GribRecord.hh>

using namespace std;

class GribFile
{

public:

  GribFile();
  ~GribFile();
  
//  int createInventory(const string &file_path = "");
//  
  int read(const string &file_path = "");
  int write(const string &file_path = "");
  
  void print(FILE *, const bool print_bitmap_flag = false,
	     const bool print_data_flag = false,
	     const bool print_min_datum_flag = false,
	     const bool print_max_datum_flag = false) const;
  
  void print(ostream &stream, const bool print_bitmap_flag = false,
	     const bool print_data_flag = false,
	     const bool print_min_datum_flag = false,
	     const bool print_max_datum_flag = false) const;
  
  void setFilePath(const string &new_file_path);

  void printInventory(ostream &stream) const;
    
  vector< const GribRecord* > getRecords(const int parameter_id,
					 const int level_id)
  {
    vector< const GribRecord* > record_list;
    
    vector< file_inventory_t >::const_iterator inventory_record;
    
    for (inventory_record = _inventory.begin();
	 inventory_record != _inventory.end(); ++inventory_record)
    {
      if (inventory_record->parameter_id == parameter_id &&
	  inventory_record->level_id == level_id)
	record_list.push_back(inventory_record->record);
    } /* endfor - inventory_record */
    
    return record_list;
  }
  

private:
  
  typedef struct
  {
    int record_start;
    int record_len;
    int parameter_id;
    int level_id;
    GribRecord *record;
  } file_inventory_t;
  
  vector< file_inventory_t > _inventory;
  
  string _filePath;
  FILE *_filePtr;
  bool _fileContentsRead;
  bool _fileContentsInventoried;

  void _clearInventory();
  
};

#endif
