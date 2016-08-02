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
/**
 *
 * @file HdfFile.cc
 *
 * @class HdfFile
 *
 * Class controlling access to a TRMM HDF file.
 *  
 * @date 4/9/2009
 *
 */

#include <iostream>

#include <toolsa/Path.hh>

#include "HdfFile.hh"

using namespace std;


// Global constants

const string HdfFile::FLASH_VDATA_NAME = "flash";
const string HdfFile::FLASH_VDATA_FIELD_LIST = "TAI93_time,location";
const size_t HdfFile::FLASH_VDATA_RECORD_SIZE =
    sizeof(float64) + (2 * sizeof(float32));

const string HdfFile::GROUP_VDATA_NAME = "group";
const string HdfFile::GROUP_VDATA_FIELD_LIST =
  "TAI93_time,location,observe_time,radiance,footprint,child_count,approx_threshold,alert_flag,cluster_index,density_index,noise_index,glint_index,oblong_index,grouping_sequence,grouping_status";
const size_t HdfFile::GROUP_VDATA_RECORD_SIZE =
  sizeof(float64) + (2 * sizeof(float32)) + sizeof(int16) +
  (2 * sizeof(float32)) + sizeof(int32) + (5 * sizeof(char)) +
  (2 * sizeof(float32)) + sizeof(int32) + sizeof(char);

const double HdfFile::LEAPSECONDS_TAI93_OFFSET[23] =
{ -662774417.000, 
  -647049616.000, 
  -631152015.000, 
  -599616014.000, 
  -568080013.000, 
  -536544012.000, 
  -504921611.000, 
  -473385610.000, 
  -441849619.000, 
  -410313608.000, 
  -363052807.000, 
  -331516806.000, 
  -299980805.000, 
  -236822404.000, 
  -157852803.000, 
  -94694402.000, 
  -63158401.000, 
  -15897600.000, 
  15638401.000, 
  47174402.000, 
  94608003.000, 
  141868804.000,
  189109605.000 };
	 
const long HdfFile::LEAPSECONDS_DAYNUM[23] = 
{ 26298, 
  26480, 
  26664, 
  27029, 
  27394, 
  27759, 
  28125, 
  28490, 
  28855, 
  29220, 
  29767, 
  30132, 
  30497, 
  31228, 
  32142,
  32873, 
  33238, 
  33785, 
  34150, 
  34515, 
  35064, 
  35611,
  36160 };

const int HdfFile::MONTH_DAYS[] =
{ 0,
  31,
  59,
  90,
  120,
  151,
  181,
  212,
  243,
  273,
  304,
  334,
  365 };



/*********************************************************************
 * Constructors
 */

HdfFile::HdfFile(const string &file_path,
		 const bool debug_flag, const bool verbose_flag) :
  _debug(debug_flag),
  _verbose(verbose_flag),
  _filePath(file_path),
  _fileId(-1)
{
}


/*********************************************************************
 * Destructor
 */

HdfFile::~HdfFile()
{
  // Close the FDATA interface

  Vend(_fileId);
  
  // Close the file if it was successfully opened

  if (_fileId != -1)
    Hclose(_fileId);
}


/*********************************************************************
 * getFlashes()
 */

bool HdfFile::getFlashes(vector< LTG_strike_t > &flashes)
{
  static const string method_name = "HdfFile::getFlashes()";
  
  // Attach to the flashes Vdata

  int32 vdata_ref;
  
  if ((vdata_ref = VSfind(_fileId, FLASH_VDATA_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error finding <" << FLASH_VDATA_NAME
	 << "> Vdata in HDF file: " << _filePath << endl;
    
    return false;
  }
  
  int32 vdata_id;
  
  if ((vdata_id = VSattach(_fileId, vdata_ref, "r")) == FAIL)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error attaching to <" << FLASH_VDATA_NAME
	 << "> Vdata in HDF file: " << _filePath << endl;
    
    return false;
  }

  // Get the needed information about the Vdata

  int32 n_records;
  int32 interlace_mode;
  
  if (VSinquire(vdata_id, &n_records, &interlace_mode, 0, 0, 0) != SUCCEED)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting information about flashes Vdata" << endl;
    
    VSdetach(vdata_id);
    return false;
  }
  
  if (n_records == 0)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "No flashes in file: " << _filePath << endl;
    
    return true;
  }
  
  // Initialize the flash data

  flashes.clear();
  flashes.reserve(n_records);
  
  LTG_strike_t flash;
  LTG_init(&flash);
  
  for (int i = 0; i < n_records; ++i)
  {
    flashes.push_back(flash);
  }
  
  // Set the flash data

  if (!_setFlashData(flashes, vdata_id, n_records, interlace_mode))
  {
    VSdetach(vdata_id);
    
    return false;
  }
  
  VSdetach(vdata_id);
  
  return true;
}


/*********************************************************************
 * getGroups()
 */

bool HdfFile::getGroups(vector< LtgGroup > &groups)
{
  static const string method_name = "HdfFile::getGroups()";
  
  // Attach to the groups Vdata

  int32 vdata_ref;
  
  if ((vdata_ref = VSfind(_fileId, GROUP_VDATA_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error finding <" << GROUP_VDATA_NAME
	 << "> Vdata in HDF file: " << _filePath << endl;
    
    return false;
  }
  
  int32 vdata_id;
  
  if ((vdata_id = VSattach(_fileId, vdata_ref, "r")) == FAIL)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error attaching to <" << GROUP_VDATA_NAME
	 << "> Vdata in HDF file: " << _filePath << endl;
    
    return false;
  }

  // Get the needed information about the Vdata

  int32 n_records;
  int32 interlace_mode;
  
  if (VSinquire(vdata_id, &n_records, &interlace_mode, 0, 0, 0) != SUCCEED)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting information about groups Vdata" << endl;
    
    VSdetach(vdata_id);
    return false;
  }
  
  if (n_records == 0)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "No groups in file: " << _filePath << endl;
    
    return true;
  }
  
  // Initialize the group data

  groups.clear();
  groups.reserve(n_records);
  
  LtgGroup group;
  
  for (int i = 0; i < n_records; ++i)
  {
    groups.push_back(group);
  }
  
  // Set the group data

  if (!_setGroupData(groups, vdata_id, n_records, interlace_mode))
  {
    VSdetach(vdata_id);
    
    return false;
  }
  
  VSdetach(vdata_id);
  
  return true;
}


/*********************************************************************
 * init()
 */

bool HdfFile::init()
{
  static const string method_name = "HdfFile::init()";
  
  if (_verbose)
    cerr << "Checking file to make sure it is an HDF file" << endl;
  
  if (Hishdf(_filePath.c_str()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << _filePath << " is not a valid HDF file, or file not found" << endl;
    
    return false;
  } 

  // Open connections to the HDF file

  if (_verbose)
    cerr << "Opening connection to file" << endl;
  
  if ((_fileId = Hopen(_filePath.c_str(), DFACC_READ, 0)) == -1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening input file : " << _filePath << endl;

    return false;
  }

  // Start HDF Vdata Interface.  All of the data in the TRMM LIS files
  // is stored in the Vdata.

  if (_verbose)
    cerr << "Starting VDATA interface" << endl;
  
  if (Vstart(_fileId) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening VDATA interface for HDF file: " << _filePath << endl;
    
    return false;
  }

  // If in verbose mode, display the Vdata fields in the file so the user
  // can see what's available.

  if (_verbose)
  {
    int32 prev_vdata_ref = -1;
    int32 vdata_ref;
    
    while ((vdata_ref = VSgetid(_fileId, prev_vdata_ref)) != FAIL)
    {
      int32 vdata_id;
      
      if ((vdata_id = VSattach(_fileId, vdata_ref, "r")) == FAIL)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error attaching to Vdata reference number: "
	     << vdata_ref << endl;
	cerr << "Skipping Vdata for verbose output" << endl;
	
	prev_vdata_ref = vdata_ref;
	continue;
      }
      
      int32 n_records;
      int32 interlace_mode;
      char field_name_list[VSNAMELENMAX * 20];
      int32 vdata_size;
      char vdata_name[VSNAMELENMAX];
      
      if (VSinquire(vdata_id, &n_records, &interlace_mode, field_name_list,
		    &vdata_size, vdata_name) != SUCCEED)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error retrieving information about Vdata id: "
	     << vdata_id << endl;
	cerr << "Skipping Vdata for verbose output" << endl;
	
	VSdetach(vdata_id);
	prev_vdata_ref = vdata_ref;
	continue;
      }
      
      cerr << "Vdata information:" << endl;
      cerr << "   name: " << vdata_name << endl;
      cerr << "   size: " << vdata_size << " bytes" << endl;
      cerr << "   n_records: " << n_records << endl;
      cerr << "   interlace_mode: " << interlace_mode << endl;
      cerr << "   field_name_list: " << field_name_list << endl;
      
      VSdetach(vdata_id);
      prev_vdata_ref = vdata_ref;
    } /* endwhile - vdata_ref */
    
  }
  
  return true;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _getDate()
 */

DateTime HdfFile::_getDate(const long day_num)
{
  static const string method_name = "HdfFile::_getDate()";
  
  int year = (int)((double)(4 * day_num) / 1461.0);
  int day_of_year = (int)(day_num - (long)(365 + (((year - 1) * 1461) >> 2)));
  year += 1900;
  int leap_days = ((year % 4 == 0) & (day_of_year > MONTH_DAYS[2])) ? 1 : 0;
  int month = 0;
        
  do 
  {
    month++;
  }
  while (day_of_year > MONTH_DAYS[month] + leap_days);
        
  int day = day_of_year - MONTH_DAYS[month-1];

  if (month > 2)
    day -= leap_days;

  return DateTime(year, month, day);
}


/*********************************************************************
 * _setFlashData()
 */

bool HdfFile::_setFlashData(vector< LTG_strike_t > &strikes,
			    const int32 vdata_id,
			    const int32 n_records,
			    const int32 interlace_mode)
{
  static const string method_name = "HdfFile::_setFlashData()";
  
  // Get the required data

  if (VSfexist(vdata_id, (char *)FLASH_VDATA_FIELD_LIST.c_str()) != 1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Couldn't find fields <" << FLASH_VDATA_FIELD_LIST
	 << "> in flash Vdata" << endl;
    
    return false;
  }
  
  if (VSsetfields(vdata_id, FLASH_VDATA_FIELD_LIST.c_str())
      != SUCCEED)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error setting Vdata fields to <"
	 << FLASH_VDATA_FIELD_LIST << ">" << endl;
    
    return false;
  }
  
  if (_verbose)
  {
    cerr << "---> Reading <" << FLASH_VDATA_FIELD_LIST << "> fields" << endl;
    cerr << "     Num records = " << n_records << endl;
    cerr << "     Interlace mode = " << interlace_mode << endl;
  }
  
  uint8 *field_buf = new uint8[n_records * FLASH_VDATA_RECORD_SIZE];
  int32 n_records_read;
  
  if ((n_records_read = VSread(vdata_id, field_buf, n_records,
			       interlace_mode)) != n_records)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading flash Vdata fields from HDF file" << endl;
    cerr << "Expected " << n_records << " records" << endl;
    cerr << "Got " << n_records_read << " records" << endl;
    
    delete [] field_buf;
    return false;
  }
  
  // Unpack the data fields

  float64 *flash_times = new float64[n_records];
  float32 *locations = new float32[n_records * 2];
  VOIDP buf_ptrs[2];
  buf_ptrs[0] = flash_times;
  buf_ptrs[1] = locations;
  
  if (VSfpack(vdata_id, _HDF_VSUNPACK, FLASH_VDATA_FIELD_LIST.c_str(),
	      field_buf, FLASH_VDATA_RECORD_SIZE * n_records, n_records,
	      FLASH_VDATA_FIELD_LIST.c_str(), buf_ptrs) != SUCCEED)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error unpacking flash Vdata fields" << endl;
    
    delete [] field_buf;
    delete [] flash_times;
    delete [] locations;
    
    return false;
  }
  
  delete [] field_buf;
  
  // Set the values in the strike records

  for (int i = 0; i < n_records; ++i)
  {
    strikes[i].time = _tai93toUtc(flash_times[i]).utime();
    strikes[i].latitude = locations[i * 2];
    strikes[i].longitude = locations[(i * 2) + 1];
  } /* endfor - i */
  
  // Clean up and leave

  delete [] flash_times;
  delete [] locations;
  
  return true;
}


/*********************************************************************
 * _setGroupData()
 */

bool HdfFile::_setGroupData(vector< LtgGroup > &groups,
			    const int32 vdata_id,
			    const int32 n_records,
			    const int32 interlace_mode)
{
  static const string method_name = "HdfFile::_setGroupData()";
  
  // Get the required data

  if (VSfexist(vdata_id, (char *)GROUP_VDATA_FIELD_LIST.c_str()) != 1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Couldn't find fields <" << GROUP_VDATA_FIELD_LIST
	 << "> in group Vdata" << endl;
    
    return false;
  }
  
  if (VSsetfields(vdata_id, GROUP_VDATA_FIELD_LIST.c_str())
      != SUCCEED)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error setting Vdata fields to <"
	 << GROUP_VDATA_FIELD_LIST << ">" << endl;
    
    return false;
  }
  
  if (_verbose)
  {
    cerr << "---> Reading <" << GROUP_VDATA_FIELD_LIST << "> fields" << endl;
    cerr << "     Num records = " << n_records << endl;
    cerr << "     Interlace mode = " << interlace_mode << endl;
  }
  
  uint8 *field_buf = new uint8[n_records * GROUP_VDATA_RECORD_SIZE];
  int32 n_records_read;
  
  if ((n_records_read = VSread(vdata_id, field_buf, n_records,
			       interlace_mode)) != n_records)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading group Vdata fields from HDF file" << endl;
    cerr << "Expected " << n_records << " records" << endl;
    cerr << "Got " << n_records_read << " records" << endl;
    
    delete [] field_buf;
    return false;
  }
  
  // Unpack the data fields

  float64 *group_times = new float64[n_records];
  float32 *locations = new float32[n_records * 2];
  int16 *observe_times = new int16[n_records];
  float32 *radiances = new float32[n_records];
  float32 *footprints = new float32[n_records];
  int32 *child_counts = new int32[n_records];
  char *approx_thresholds = new char[n_records];
  char *alert_flags = new char[n_records];
  char *cluster_indices = new char[n_records];
  char *density_indices = new char[n_records];
  char *noise_indices = new char[n_records];
  float32 *glint_indices = new float32[n_records];
  float32 *oblong_indices = new float32[n_records];
  int32 *grouping_sequences = new int32[n_records];
  char *grouping_statuses = new char[n_records];
  
  VOIDP buf_ptrs[15];
  buf_ptrs[0] = group_times;
  buf_ptrs[1] = locations;
  buf_ptrs[2] = observe_times;
  buf_ptrs[3] = radiances;
  buf_ptrs[4] = footprints;
  buf_ptrs[5] = child_counts;
  buf_ptrs[6] = approx_thresholds;
  buf_ptrs[7] = alert_flags;
  buf_ptrs[8] = cluster_indices;
  buf_ptrs[9] = density_indices;
  buf_ptrs[10] = noise_indices;
  buf_ptrs[11] = glint_indices;
  buf_ptrs[12] = oblong_indices;
  buf_ptrs[13] = grouping_sequences;
  buf_ptrs[14] = grouping_statuses;
  
  if (VSfpack(vdata_id, _HDF_VSUNPACK, GROUP_VDATA_FIELD_LIST.c_str(),
	      field_buf, GROUP_VDATA_RECORD_SIZE * n_records, n_records,
	      GROUP_VDATA_FIELD_LIST.c_str(), buf_ptrs) != SUCCEED)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error unpacking group Vdata fields" << endl;
    
    delete [] field_buf;
    delete [] group_times;
    delete [] locations;
    delete [] observe_times;
    delete [] radiances;
    delete [] footprints;
    delete [] child_counts;
    delete [] approx_thresholds;
    delete [] alert_flags;
    delete [] cluster_indices;
    delete [] density_indices;
    delete [] noise_indices;
    delete [] glint_indices;
    delete [] oblong_indices;
    delete [] grouping_sequences;
    delete [] grouping_statuses;
    
    return false;
  }
  
  delete [] field_buf;
  
  // Set the values in the strike records

  for (int i = 0; i < n_records; ++i)
  {
    groups[i].setTime(_tai93toUtc(group_times[i]).utime());
    groups[i].setLocation(locations[i * 2], locations[(i * 2) + 1]);
    groups[i].setObserveTime(observe_times[i]);
    groups[i].setNetRadiance(radiances[i]);
    groups[i].setFootprint(footprints[i]);
    groups[i].setChildCount(child_counts[i]);
    groups[i].setApproxThreshold(approx_thresholds[i]);
    groups[i].setAlertFlag(alert_flags[i]);
    groups[i].setClusterIndex(cluster_indices[i]);
    groups[i].setDensityIndex(density_indices[i]);
    groups[i].setNoiseIndex(noise_indices[i]);
    groups[i].setGlintIndex(glint_indices[i]);
    groups[i].setOblongIndex(oblong_indices[i]);
    groups[i].setGroupingSequence(grouping_sequences[i]);
    groups[i].setGroupingStatus(grouping_statuses[i]);
  } /* endfor - i */
  
  // Clean up and leave

  delete [] group_times;
  delete [] locations;
  delete [] observe_times;
  delete [] radiances;
  delete [] footprints;
  delete [] child_counts;
  delete [] approx_thresholds;
  delete [] alert_flags;
  delete [] cluster_indices;
  delete [] density_indices;
  delete [] noise_indices;
  delete [] glint_indices;
  delete [] oblong_indices;
  delete [] grouping_sequences;
  delete [] grouping_statuses;
  
  return true;
}


/*********************************************************************
 * _tai93toUtc()
 */

DateTime HdfFile::_tai93toUtc(const float64 tai93)
{
  static const string method_name = "HdfFile::_tai93toUtc()";
  
  long idx = 0;
  while (LEAPSECONDS_TAI93_OFFSET[idx] <= tai93 && idx < 21)
    idx++;
  idx--;
  long ref_daynum = LEAPSECONDS_DAYNUM[idx];
  double sec_since_ref = tai93 - LEAPSECONDS_TAI93_OFFSET[idx];
  double days_since_ref = sec_since_ref/86400.0;
  long daynums_since_ref = (long)(((double)days_since_ref));
  long this_daynum = ref_daynum + daynums_since_ref;
    
  DateTime utc = _getDate(this_daynum);
    
  sec_since_ref -= (double)daynums_since_ref*86400.0;
    
  utc.setHour((int)(((double)sec_since_ref/3600.0)));
  sec_since_ref -= (double)utc.getHour()*3600.0;
  utc.setMin((int)(((double)sec_since_ref/60.0)));
  sec_since_ref -= (double)utc.getMin()*60.0;
  utc.setSec((int)sec_since_ref);
    
  return(utc);
}
