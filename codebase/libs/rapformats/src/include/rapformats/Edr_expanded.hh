// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1992 - 2010 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2010/10/7 23:12:49 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/*
 *   Module: Edr_expanded.hh
 *
 *   Author: Gary Blackburn
 *
 *   Description: Defines EDR structure, static constants and 
 *   methods to manage EDR class.  From an historical perspective
 *   this version handles 4 letter ICAO identifiers and adds space
 *   for future expansions.
 */

/**
 * @file Edr_espanded.hh
 * @brief Header file for defining Edr_t structure, static constants and methods to 
 * manage EDR class
 * @class Edr 
 * @brief container for EDR information, defines the Edr_t stucture, static 
 * constands and various methods for byte swapping, getting, setting and 
 * printing EDR information
 */


#ifndef EDR_VER2_HH
#define EDR_VER2_HH

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <toolsa/MemBuf.hh>
#include <dataport/port_types.h>
using namespace std;


class EDR {

public:

  static const si32 SRC_NAME_LEN = 80;
  static const si32 SRC_FMT_LEN = 40;
  static const si32 AIRPORT_NAME_LEN = 5;  
  static const si32 TAILNUM_NAME_LEN = 16; 
  static const si32 FLIGHTNUM_NAME_LEN = 16;
  static const si32 AIRLINE_NAME_LEN = 8; 

  static const si32 NUM_SPARE_INTS = 9;
  static const si32 NUM_SPARE_FLOATS = 8;
  static const si32 NUM_QC_INDS = 4;
  static const si32 NUM_QC_FLAGS = 4;

  static const si32 NBYTES_32 = 156;
  
  static const fl32 VALUE_UNKNOWN; // = -9999.0;

  //                EDR QC FLAGS
  static const si32 QC_NO_VALUE = 99990;
  static const si32 QC_BAD_CHR = 99991;
  static const si32 QC_BAD_CGA = 99992;
  static const si32 QC_BAD_ALT = 99993;
  static const si32 QC_BAD_MACH = 99994;
  static const si32 QC_BAD_OTHER = 99995;
  static const si32 QC_BAD_BASIS = 99996;
  static const si32 QC_BAD_VALUE = 99999;
  static const si32 QC_GOOD_VALUE = 1;

  //                QC ALGORITHM BIT FLAGS
  static const si32 BELOW_MIN_ALT = 1;
  static const si32 FAILED_ONBOARD_QC = 2;
  static const si32 FAILED_BOUNDS_CK = 4;
  static const si32 BAD_TAIL = 8;
  static const si32 LOW_ONBOARD_QC = 16;
  static const si32 UNKNOWN_TAIL_STATUS = 32;

  /**
   * @brief Holds all information contained in an EDR message
   */
  typedef struct
  {
    si32 time;                        /*!< Data time */
    si32 fileTime;                    /*!< File time */
    fl32 lat;                         /*!< Report latitude */
    fl32 lon;                         /*!< Report longitude */
    fl32 alt;                         /*!< Altitude (feet) */
    si32 interpLoc;                   /*!< Interpolated location vs actual */
    fl32 edrPeak;                     /*!< EDR peak */
    fl32 edrAve;                      /*!< EDR median */
    fl32 wspd;                        /*!< Wind speed */
    fl32 wdir;                        /*!< Wind direction */
    fl32 sat;                         /*!< Static air temperature */
    fl32 qcConf;                      /*!< QC confidence */
    fl32 qcThresh;                    /*!< QC threshold */
    fl32 qcVersion;                   /*!< QC version that processed the EDR message */
    fl32 edrAlgVersion;               /*!< Algorithm version running on aircraft */

    fl32 PeakConf;                    /*!< Peak confidence (onboard QC) */
    fl32 MeanConf;                    /*!< Mean confidence (onboard QC) */
    fl32 PeakLocation;                /*!< Peak location (onboard QC) */
    fl32 NumGood;                     /*!< Number of good values (onboard QC) */

    fl32 edrAveQcInds[NUM_QC_INDS];   /*!< EDR average QC indices (onboard QC) */ 
    si32 edrPeakQcFlags[NUM_QC_FLAGS];/*!< EDR peak QC flags (onboard QC)  (EDR QC FLAGS)*/
    si32 edrAveQcFlags[NUM_QC_FLAGS]; /*!< EDR average QC flags (onboard QC) (EDR QC FLAGS) */

    fl32 mach;                        /*!< Mach speed */
    fl32 rms;                         /*!< Max rms during previous 15 minutes */
    fl32 runningMinConf;              /*!< Running minimum confidence (onboard QC) */        
    fl32 spareFloats[NUM_SPARE_FLOATS];/*!< Allows for future additions */

    si32 maxNumBad;                   /*!< Maximum number of bad (onboard QC) */   
    si32 QcDescriptionBitFlags;       /*!< Bit masked flags (QC ALGORITHM BIT FLAGS) */
    si32 computedAirspeed;            /*!< Airspeed */
    si32 spareInts[NUM_SPARE_INTS];   /*!< Allows for future additions */
    si32 isIcing;                     /*!< Boeing 777 also includes an icing indicater */

    char aircraftRegNum[TAILNUM_NAME_LEN];  /*!< Aircraft registry number */
    char encodedAircraftRegNum[TAILNUM_NAME_LEN];  /*!< Optional Encoded Aircraft registry number */
    char flightNum[FLIGHTNUM_NAME_LEN]; /*!< Flight number */
    char origAirport[AIRPORT_NAME_LEN]; /*!< Origination airport */
    char destAirport[AIRPORT_NAME_LEN]; /*!< Destination airport */
    char airlineId[AIRLINE_NAME_LEN];   /*!< Airline ID */
    char sourceName[SRC_NAME_LEN];      /*!< Aircraft type */
    char sourceFmt[SRC_FMT_LEN];        /*!< Downlinked message format */


  } Edr_t;

  // constructor

  EDR();

  // destructor

  ~EDR();
  
  /**
   * assemble()
   * Loads the buffer from the object.
   * Handles byte swapping if required - Software stores in little endian;
   * System determines machine endianness
   */
  void assemble();

  /**
   * Returns the pointer to the beginning of the EDR report stored in memory
   */
  
  void *getBufPtr() const { return _memBuf.getPtr(); };

  /**
   * Returns the size of the buffer containing the EDR report
   */
  int getBufLen() const { return _memBuf.getLen(); };

  /**
   * Disassembles a buffer, sets the values in the object.
   * Handles byte swapping.
   * Returns 0 on success, -1 on failure
  */
  int disassemble(const void *buf, int len);

  /**
   * set private Edr_t structure
   */
  void setEdr(const Edr_t &rep);
 
  /**
   * return report time from private Edr_t structure
   */

  si32 getTime() { return _edr.time; }  

  /**
   * return complete private edr report
   */

  const Edr_t &getRep() const { return _edr; } 
 
  /** 
   * Print EDR (Edr_t) field values 
   * @param[in] out usually set to cerr 
   * @param[in] spacer initialized as null, defines indention for printed output 
   */
 
  void print(ostream &out, string spacer = "") const;

protected:

private:
  
  //
  // data members 
  //
  MemBuf _memBuf; /*!< Automatically resizeable memory buffer for holding EDR data */
  
  Edr_t _edr; /*!< EDR structure */
 
  
  /**
   * Byte swapping to big endian
   */
  static void _edr_to_BE(Edr_t& rep);

  /**
   * Byte swapping from big endian
   */
  static void _edr_from_BE(Edr_t& rep);

};

#endif /* EDR_HH */
