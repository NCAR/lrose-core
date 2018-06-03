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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* RCS info
 *   $Author: jcraig $
 *   $Locker:  $
 *   $Date: 2018/01/26 20:36:36 $
 *   $Id: Chill2netCDFSweep.hh,v 1.11 2018/01/26 20:36:36 jcraig Exp $
 *   $Revision: 1.11 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Chill2netCDFSweep: Chill2netCDFSweep program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2005
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef Chill2netCDFSweep_HH
#define Chill2netCDFSweep_HH

#include <string>
#include <sys/time.h>
#include <vector>
#include <cmath>

#include <dsdata/DsTrigger.hh>

#include "Args.hh"
#include "Params.hh"
#include "SweepFile.hh"

using namespace std;

class Chill2netCDFSweep
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  // Flag indicating whether the program status is currently okay.

  bool okay;


  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Destructor
   */

  ~Chill2netCDFSweep(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static Chill2netCDFSweep *Inst(int argc, char **argv);
  static Chill2netCDFSweep *Inst();
  

  /*********************************************************************
   * init() - Initialize the local data.
   *
   * Returns true if the initialization was successful, false otherwise.
   */

  bool init();
  

  /////////////////////
  // Running methods //
  /////////////////////

  /*********************************************************************
   * run() - run the program.
   */

  void run();
  

 private:

  ///////////////////////
  // Private constants //
  ///////////////////////

  static const int INIT_NUM_GATES;
  static const int INIT_NUM_BEAMS;
  
  static const double ANGLE_TO_DEGREE;

  static const int PARAM_AVGI_MASK;
  static const int PARAM_AVGQ_MASK;
  static const int PARAM_DBZ_MASK;
  static const int PARAM_VEL_MASK;
  static const int PARAM_NCP_MASK;
  static const int PARAM_SW_MASK;
  
  static const int CHANNEL_H_MASK;
  static const int CHANNEL_V_MASK;
  
  static const float RADAR_C;       // db
  static const float ANT_GAIN_H;    // db
  static const float ANT_GAIN_V;    // db
  static const float REC_GAIN_H;    // db
  static const float REC_GAIN_V;    // db
  static const float TX_POW_H;      // dbm
  static const float TX_POW_V;      // dbm
  

  ///////////////////
  // Private types //
  ///////////////////

  typedef struct
  {
    char id[4];
    int headlen;	    // number of words in this header, before data[]
    time_t rtime;	    // unix time word
    unsigned int xtime;	    // extended time: fraction of second * 40,000,000
    int channels_recorded;  // bit mask: 1-> H data present, 2-> V data present
    int param_recorded;	    // parameters recorded(Bit Mask) : 1->avgI,
                            //   2->avgQ, 4->dBZ, 8->vel, 16 ->NCP, 
                            //   32->Spect. Width
    int az_start, az_end;   // 65536 counts per 360 degrees
    int el_start, el_end;
    int prt;		    // microseconds
    int nyquist_vel;	    // mm/sec
    int ngates;		    // number of gates recorded
    int range_start;	    // range to first gate in mm
    int range_inc;	    // range gate spaceing in mm
    int pulses;		    // number of pulses in average
    int zhconst;
    int zvconst;            // z constants (dB) *1000
    int rsq100;             // rsq correction at gate 100 *1000
  } header_t;
  
  
  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static Chill2netCDFSweep *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Data trigger object

  DsTrigger *_dataTrigger;
  
  // Sweep information

  time_t _sweepStartTime;
  double _startElevation;
  int _sweepNumber;
  int _startRangeMm;
  int _gateSpacingMm;
  int _numGatesInSweep;
  int _samplesPerBeam;
  double _nyquistVelocity;
  int _prt;
  
  // Data arrays

  float *_azimuth;
  float *_elevation;
  double *_timeOffset;

  float *_hAvgI;
  float *_hAvgQ;
  float *_hDbz;
  float *_hVel;
  float *_hNcp;
  float *_hSw;
  float *_hNiq;
  float *_hAiq;
  float *_hDm;
  
  float *_vAvgI;
  float *_vAvgQ;
  float *_vDbz;
  float *_vVel;
  float *_vNcp;
  float *_vSw;
  float *_vNiq;
  float *_vAiq;
  float *_vDm;
  
  int _beamIndex;
  int _numGatesAlloc;
  int _numBeamsAlloc;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  Chill2netCDFSweep(int argc, char **argv);
  

  /*********************************************************************
   * _allocateDataArrays() - Allocate space for the data arrays.
   */

  void _allocateDataArrays(const int num_gates_needed,
			   const int num_beams_needed);
  

  /*********************************************************************
   * _calcAiq() - Calculate the AIQ value from the given avg I and Q values.
   */

  static inline float _calcAiq(const float avg_i, const float avg_q)
  {
    if (avg_i == 0.0 && avg_q == 0.0)
      return SweepFile::MISSING_DATA_VALUE;
    
    return 180.0 / M_PI * atan2(avg_q, avg_i);
  }

  
  /*********************************************************************
   * _calcAvgI() - Calculate the average I value from the value that was
   *               stored in the input file.
   */

  static inline float _calcAvgI(const short input_value)
  {
    return (float)input_value / 65536.0;
  }

  
  /*********************************************************************
   * _calcAvgQ() - Calculate the average Q value from the value that was
   *               stored in the input file.
   */

  static inline float _calcAvgQ(const short input_value)
  {
    return (float)input_value / 65536.0;
  }

  
  /*********************************************************************
   * _calcDbz() - Calculate the reflectivity value from the value that was
   *              stored in the input file.
   */

  static inline float _calcDbz(const short input_value)
  {
    return (float)input_value / 100.0;
  }

  
  /*********************************************************************
   * _calcDm() - Calculate the power value from the given reflectivity,
   *             Z, receiver gain and range_km values.
   */

  static inline float _calcDm(const float refl, const float z,
			      const float rec_gain, const float range_km)
  {
    return refl - z - rec_gain - (20 * log10(range_km / 100.0));
  }

  
  /*********************************************************************
   * _calcNcp() - Calculate the NCP value from the value that was
   *              stored in the input file.
   */

  static inline float _calcNcp(const short input_value)
  {
    if (input_value > 1000)
      return 1.0;
    
    return (float)input_value / 1000.0;
  }

  
  /*********************************************************************
   * _calcNiq() - Calculate the NIQ value from the given avg I and Q values.
   */

  static inline float _calcNiq(const float avg_i, const float avg_q)
  {
    if (avg_i == 0.0 && avg_q == 0.0)
      return SweepFile::MISSING_DATA_VALUE;
    
    return 10.0 * log10(sqrt((avg_i * avg_i) + (avg_q * avg_q)));
  }

  
  /*********************************************************************
   * _calcSw() - Calculate the spectral width value from the value that was
   *              stored in the input file.
   *
   * Note that this field hasn't been implemented yet on the CHILL side
   * so this code will have to be updated when the field is implemented.
   */

  static inline float _calcSw(const short input_value)
  {
    return (float)input_value;
  }

  
  /*********************************************************************
   * _calcVel() - Calculate the velocity value from the value that was
   *              stored in the input file.
   */

  static inline float _calcVel(const short input_value)
  {
    return (float)input_value / 1000.0;
  }

  
  /*********************************************************************
   * _getNumBitsSet() - Get the number of bits set in the given mask
   *
   * Returns the number of set bits.
   */

  inline int _getNumBitsSet(const int mask)
  {
    int i, bit;
    int num_bits = 0;
    
    for (i=0, bit=1; i<32; ++i, bit<<=1)
      if (mask & bit) ++num_bits;
    
    return num_bits;
  }

  
  /*********************************************************************
   * _initDataArray() - Initialize the values in the given data array to
   *                    the missing data value.
   */

  inline void _initDataArray(float *data_array)
  {
    for (int i = 0; i < _numGatesAlloc * _numBeamsAlloc; ++i)
      data_array[i] = SweepFile::MISSING_DATA_VALUE;
  }
  

  /*********************************************************************
   * _initializeDataArrays() - Initialize all of the gates in the data
   *                           arrays to missing data values.  This is
   *                           done at the beginning of processing each
   *                           file to clear out the old data.
   */

  void _initializeDataArrays();
  

  /*********************************************************************
   * _initInfoArray() - Initialize the values in the given info array to
   *                    the missing data value.
   */

  inline void _initInfoArray(float *info_array)
  {
    for (int i = 0; i < _numBeamsAlloc; ++i)
      info_array[i] = SweepFile::MISSING_DATA_VALUE;
  }
  

  inline void _initInfoArray(double *info_array)
  {
    for (int i = 0; i < _numBeamsAlloc; ++i)
      info_array[i] = SweepFile::MISSING_DATA_VALUE;
  }
  

  /*********************************************************************
   * _processBeam() - Process the next beam in the given input file.
   *
   * Returns true on success, false on failure.
   */

  bool _processBeam(FILE *input_file,
		    const string &input_file_path,
		    const bool first_beam);
  

  /*********************************************************************
   * _processFile() - Process the given input file
   */

  bool _processFile(const string &input_file_name);


  /*********************************************************************
   * _reallocDataArray() - Reallocate the space for the given data array,
   *                       setting all new gate values to missing.
   */

  inline void _reallocDataArray(float* &old_array,
				const int num_gates_needed,
				const int num_beams_needed) const
  {
    float *new_array = new float[num_gates_needed * num_beams_needed];
    
    // Process the existing beams

    for (int beam = 0; beam < _numBeamsAlloc; ++beam)
    {
      // Copy the existing data

      float *input_ptr = old_array + (beam * _numGatesAlloc);
      float *output_ptr = new_array + (beam * num_gates_needed);
      memcpy(output_ptr, input_ptr, _numGatesAlloc * sizeof(float));
      
      // Add the new gates

      for (int gate = _numGatesAlloc; gate < num_gates_needed; ++gate)
	new_array[(beam * num_gates_needed) + gate] = SweepFile::MISSING_DATA_VALUE;
      
    } /* endfor - beam */

    // Add on the new beams

    for (int beam = _numBeamsAlloc; beam < num_beams_needed; ++beam)
      for (int gate = 0; gate < num_gates_needed; ++gate)
	new_array[(beam * num_gates_needed) + gate] = SweepFile::MISSING_DATA_VALUE;
    
    // Reclaim space and replace the old pointer

    delete [] old_array;
    old_array = new_array;
  }


  /*********************************************************************
   * _reallocInfoArray() - Reallocate the space for the given info array,
   *                       setting all new beam values to missing.
   */

  inline void _reallocInfoArray(float* &old_array,
				const int num_beams_needed) const
  {
    float *new_array = new float[num_beams_needed];
    
    // Copy the existing data

    memcpy(new_array, old_array, _numBeamsAlloc * sizeof(float));
    
    // Add the info for the new beams

    for (int beam = _numBeamsAlloc; beam < num_beams_needed; ++beam)
      new_array[beam] = SweepFile::MISSING_DATA_VALUE;
    
    // Reclaim space and replace the old pointer

    delete [] old_array;
    old_array = new_array;
  }


  inline void _reallocInfoArray(double* &old_array,
				const int num_beams_needed) const
  {
    double *new_array = new double[num_beams_needed];
    
    // Copy the existing data

    memcpy(new_array, old_array, _numBeamsAlloc * sizeof(double));
    
    // Add the info for the new beams

    for (int beam = _numBeamsAlloc; beam < num_beams_needed; ++beam)
      new_array[beam] = SweepFile::MISSING_DATA_VALUE;
    
    // Reclaim space and replace the old pointer

    delete [] old_array;
    old_array = new_array;
  }


};


#endif
