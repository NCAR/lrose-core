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
/////////////////////////////////////////////////////////////
// DoradeData.hh
//
// Data structs for Dorade in Radx
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2010
//
///////////////////////////////////////////////////////////////

#ifndef DoradeData_HH
#define DoradeData_HH

#include <string>
#include <ostream>
#include <Radx/Radx.hh>
using namespace std;

///////////////////////////////////////////////////////////////////////
/// CLASS DEFINING DORADE DATA FORMAT
///
/// Defines the header structures used in DORADE format radar
/// files.
///
/// Dorade data files are made up of a sequence of data structures,
/// each of which has at the start a 4-character ID, followed by a
/// length in bytes.
///
/// For 2 of the structure types, id RDAT and QDAT, the structure is
/// followed by field data. The length in bytes is therefore longer
/// than the structure itself, since it includes the bytes in the
/// following data fields.
///
/// - SSWB, struct super_SWIB_t,
///   super sweep indenitifcation block
///
/// - VOLD, struct volume_t,
///   volume description block
///
/// - RADD, struct radar_t,
///   radar description block
///
/// - CFAC, struct correction_t,
///   correction factors block
///
/// - PARM, struct parameter_t,
///   parameter (data field) description block
///
/// - CELV, struct cell_t,
///   cell (gate) spacing array block
///
/// - CSFD, struct cell_spacing_fp_t,
///   cell spacing table block
///
/// - SWIB, struct sweepinfo_t,
///   sweep information description block
///
/// - ASIB, struct platform_t,
///   platform description block
///
/// - RYIB, struct ray_t,
///   ray information block
///
/// - RDAT, struct paramdata_t,
///   field parameter data block
///
/// - QDAT, struct qparamdata_t,
///   field parameter data block, extended version
///
/// - XSTF, struct extra_stuff_t,
///   miscellaneous block
///
/// - NULL, struct null_block_t,
///   null block, end of main data
///
/// - RKTB, struct rot_ang_table_t,
///   rotation angle table, after null block
///
/// - SEDS, ASCII
///   Edit history block
///
/// - FRAD, struct radar_test_status_t,
///   RADAR test pulse and status block
///
/// - FRIB, struct field_radar_t,
///   field RADAR information block
///
/// - LIDR, struct lidar_t,
///   LIDAR description
///
/// - FLIB, struct field_lidar_t,
///   field LIDAR information block
///
/// - SITU, struct insitu_descript_t,
///   in-situ description block
///
/// - ISIT, struct insitu_data_t,
///   in-situ parameters block
///
/// - INDF, struct indep_freq_t,
///   independent frequency block
///
/// - MINI, struct minirims_data_t,
///   minirims data block
///
/// - NDDS, struct nav_descript_t,
///   navigation block
///
/// - TIME, struct time_series_t,
///   time series block
///
/// - WAVE, struct waveform_t,
///   Waveform descriptor block
///
/// The most tricky part of a DORADE file is the rotation angle table
/// (RKTB). This table is stored in a block after the NULL block, and
/// is pointed to by the key_table in the SWIB block. The rotation
/// angle table comprises 3 sections:
///
/// -# The rot_angle_table_t structure at the start of the block.
/// -# An array of integers: Radx::si32[ndx_que_size], which is a
///    lookup table for locating the ray for a given angle.
/// -# An array of table entries: rot_table_entry_t[num_rays], which
///    stores the rotation_angle for each ray, as well as the offset
///    and length of the ray data in the file.

class DoradeData {

public:

  /// binary format

  typedef enum {
    BINARY_FORMAT_INT8 = 1, /**< signed 8-bit integer */
    BINARY_FORMAT_INT16 = 2, /**< signed 16-bit integer */
    BINARY_FORMAT_INT32 = 3, /**< signed32-bit integer */
    BINARY_FORMAT_FLOAT32 = 4 /**< IEEE 32-bit float */
  } binary_format_t;

  /// code word bits

  typedef enum {
    REFLECTIVITY_BIT  = 0x8000, /**< bit 15 */
    VELOCITY_BIT  = 0x4000, /**< bit 14 */
    WIDTH_BIT  = 0x2000, /**< bit 13 */
    TA_DATA_BIT  = 0x1000, /**< bit 12 */
    LF_DATA_BIT  = 0x0800, /**< bit 11 */
    TIME_SERIES_BIT  = 0x0400 /**< bit 10 */
  } code_word_bits_t;

  /// radar types

  typedef enum {
    RADAR_GROUND = 0, /**< ground-based radar */
    RADAR_AIR_FORE = 1, /**< aircraft forward-looking radar (Eldora) */
    RADAR_AIR_AFT = 2, /**< aircraft aft-looking radar (Eldora) */
    RADAR_AIR_TAIL = 3, /**< aircraft tail radar */
    RADAR_AIR_LF = 4, /**< aircraft lower fuselage radar */
    RADAR_SHIP = 5, /**< ship-borne radar */
    RADAR_AIR_NOSE = 6, /**< aircraft nose radar */
    RADAR_SATELLITE = 7, /**< satellite-based radar */
    LIDAR_MOVING = 8, /**< mobile lidar - deprecated */
    LIDAR_FIXED = 9 /**< fixed lidar - deprecated */
  } radar_type_t;

  /// lidar types

  typedef enum {
    LIDAR_GROUND = 0, /**< ground-based lidar */
    LIDAR_AIR_FORE = 1, /**< aircraft forward-looking lidar */
    LIDAR_AIR_AFT = 2, /**< aircraft aft-looking lidar */
    LIDAR_AIR_TAIL = 3, /**< aircraft tail lidar */
    LIDAR_AIR_LF = 4, /**< aircraft lower fuselage lidar */
    LIDAR_SHIP = 5, /**< ship-borne lidar */
    LIDAR_AIR_FIXED = 6, /**< fixed lidar */
    LIDAR_SATELLITE = 7 /**< satellite-based lidar */
  } lidar_type_t;

  /// field IDs

  typedef enum {
    SW_ID_NUM = 1, /**< ID for spectrum width */
    VR_ID_NUM = 2, /**< ID for radial velocity */
    NCP_ID_NUM = 3, /**< ID for normalized coherent power */
    DBZ_ID_NUM = 4, /**< ID for dbz */
    DZ_ID_NUM = 5, /**< ID for ZDR */
    VE_ID_NUM = 6, /**< ID for radial velocity */
    VG_ID_NUM = 7, /**< ID for combined radial velocity */
    VU_ID_NUM = 8, /**< ID for radial velocity */
    VD_ID_NUM = 9, /**< ID for radial velocity */
    DBM_ID_NUM = 10 /**< ID for dbm - power */
  } field_id_t;

  /// polarization types

  typedef enum {
    POLARIZATION_HORIZONTAL = 0, /**< horizontal polarization */
    POLARIZATION_VERTICAL = 1, /**< vertical polarization */
    POLARIZATION_CIRCULAR_RIGHT = 2, /**< right circular polarization */
    POLARIZATION_ELLIPTICAL = 3, /**< elliptical polarization */
    POLARIZATION_HV_ALT = 4, /**< dual-pol alternating polarization */
    POLARIZATION_HV_SIM = 5 /**< dual-pol simultaneous polarization */
  } polarization_t;

  /// scan modes

  typedef enum {
    SCAN_MODE_CAL = 0, /**< calibration scan */ 
    SCAN_MODE_PPI = 1, /**< PPI sector scan */
    SCAN_MODE_COP = 2, /**< coplane scan */
    SCAN_MODE_RHI = 3, /**< RHI scan */
    SCAN_MODE_VER = 4, /**< vertically pointing scan */
    SCAN_MODE_TAR = 5, /**< follow target scan */
    SCAN_MODE_MAN = 6, /**< manual scan */
    SCAN_MODE_IDL = 7, /**< idle scan */
    SCAN_MODE_SUR = 8, /**< 360-deg surveillance scan */
    SCAN_MODE_AIR = 9, /**< airborne scan (e.g. eldora) */
    SCAN_MODE_HOR = 10 /**< horizontal scan */
  } scan_mode_t;

  /// compression types

  typedef enum {
    COMPRESSION_NONE = 0, /**< no compression */
    COMPRESSION_HRD = 1, /**< run-length encoding compression */
    COMPRESSION_RDP_8_BIT = 8 /**< deprecated */
  } compression_t;

  /// primary axis of rotation types
  
  typedef enum {
    Z = 1, /**< z */
    Y = 2, /**< y */
    X = 3, /**< x */
    Z_PRIME = 4, /**< z-prime */
    Y_PRIME = 5, /**< y-prime */
    X_PRIME = 6 /**< x-prime */
  } primary_axis_t;


  ///////////////////////////////////////////////////
  // structure definitions

  ////////////////////////////////////////////////////
  /// Comment block - COMM

  typedef struct comment {

    char id[4];  /**< Comment descriptor identifier: ASCII
                  * characters "COMM" stand for Comment
                  * Descriptor. */
    Radx::si32 nbytes;    /**< Comment descriptor length in bytes. */
    char  comment[500];	  /**< comment*/
    
  } comment_t;

  ////////////////////////////////////
  // key tables - in super sweep block
  //
  // These point to special blocks at the end of the file
  
  const static int MAX_KEYS = 8; /**< dimension for key table in SSIB */

  /// type of entry in key table in SWIB

  typedef enum {
    KEYED_BY_TIME = 1, /**< time series block */
    KEYED_BY_ROT_ANG = 2, /**< rotation angle table */
    SOLO_EDIT_SUMMARY = 3 /**< block containing ASCII editing details */
  } key_table_type_t;

  /// indexes for key table in SWIB

  typedef enum {
    NDX_ROT_ANG = 0, /**< rotation angle table */
    NDX_SEDS  = 1 /**< editing block */
  } key_table_index_t;

  /// key table entries in SWIB

  typedef struct key_table_info {
    Radx::si32 offset; /**< offset from start of file, in bytes */
    Radx::si32 size; /**< size of block, in bytes */
    Radx::si32 type; /**< see key_table_index_t */
  } key_table_info_t;
  
  /////////////////////////////////////
  /// super sweep ident block - SSWB
  
  typedef struct super_SWIB {

    char id[4];	/**< "SSWB" */
    Radx::si32 nbytes; /**< number of bytes in this struct block */

    /**< parameters from the first version */

    Radx::si32 last_used; /**< Last time used - Unix time */
    Radx::si32 start_time; /**< start time of volume - Unix time */
    Radx::si32 stop_time; /**< end time of volume - Unix time */
    Radx::si32 sizeof_file; /**< Length of file in bytes */
    Radx::si32 compression_flag; /**< See compression_t */
    Radx::si32 volume_time_stamp; /**< to reference current volume */
    Radx::si32 num_params; /**< number of parameters (fields) */
    
    /**< end of first version parameters */

    char radar_name[8]; /**< radar name */

    Radx::si32 pad; /**< Padding fl64s onto 8-byte boundaries */
    Radx::fl64 d_start_time; /**< Volume start time, high precision */
    Radx::fl64 d_stop_time; /**< Volume end time, high precision */

    /**<
     * "last_used" is an age off indicator where > 0 implies Unix time
     * of the last access and
     * 0 implies this sweep should not be aged off
     */

    Radx::si32 version_num; /**< version number of this format */
    Radx::si32 num_key_tables; /**< number of key tables in this file */
    Radx::si32 status; /**< status */
    Radx::si32 place_holder[7]; /**< unused */
    key_table_info_t key_table[MAX_KEYS]; /**< key table */

    /**<
     * offset and key info to a table containing key value such as
     * the rot. angle and the offset to the corresponding ray
     * in the disk file
     */

  } super_SWIB_t;

  // SWIB struct for 32-bit machines, does not force the start and
  // end times onto 8-byte boundaries

  typedef struct super_SWIB_32bit {

    char id[4];	/**< "SSWB" */
    Radx::si32 nbytes; /**< number of bytes in this struct block */

    /**< parameters from the first version */

    Radx::si32 last_used; /**< Last time used - Unix time */
    Radx::si32 start_time; /**< start time of volume - Unix time */
    Radx::si32 stop_time; /**< end time of volume - Unix time */
    Radx::si32 sizeof_file; /**< Length of file in bytes */
    Radx::si32 compression_flag; /**< See compression_t */
    Radx::si32 volume_time_stamp; /**< to reference current volume */
    Radx::si32 num_params; /**< number of parameters (fields) */
    
    /**< end of first version parameters */

    char radar_name[8]; /**< radar name */

    Radx::ui08 d_start_time[8]; /**< Volume start time, actually a double */
    Radx::ui08 d_stop_time[8]; /**< Volume end time, actually a double */

    /**<
     * "last_used" is an age off indicator where > 0 implies Unix time
     * of the last access and
     * 0 implies this sweep should not be aged off
     */

    Radx::si32 version_num; /**< version number of this format */
    Radx::si32 num_key_tables; /**< number of key tables in this file */
    Radx::si32 status; /**< status */
    Radx::si32 place_holder[7]; /**< unused */
    key_table_info_t key_table[MAX_KEYS]; /**< key table */

    /**<
     * offset and key info to a table containing key value such as
     * the rot. angle and the offset to the corresponding ray
     * in the disk file
     */

  } super_SWIB_32bit_t;

  ////////////////////////////////
  /// volume description - VOLD

  typedef struct volume {

    char id[4];	/**< Volume descriptor identifier: ASCII
                 * characters "VOLD" stand for Volume Descriptor. */
    Radx::si32 nbytes;	/**< Volume descriptor length in bytes. */
    Radx::si16 format_version;	/**< ELDORA/ASTRAEA field tape format
                                 * revision number. */
    Radx::si16 volume_num;	/**< Volume Number into current tape. */
    Radx::si32 maximum_bytes;	/**< Maximum number of bytes in any.
                                 * physical record in this volume. */
    char proj_name[20];	        /**< Project number or name. */
    Radx::si16 year;		/**< Year data taken in years. */
    Radx::si16 month;		/**< Month data taken in months. */
    Radx::si16 day;		/**< Day data taken in days. */
    Radx::si16 data_set_hour;	/**< hour data taken in hours. */
    Radx::si16 data_set_minute;	/**< minute data taken in minutes. */
    Radx::si16 data_set_second;	/**< second data taken in seconds. */
    char flight_num[8];	        /**< Flight number. */
    char gen_facility[8];	/**< identifier of facility that
                                 * generated this recording. */
    Radx::si16 gen_year;	/**< year this recording was generated
                                 * in years. */
    Radx::si16 gen_month;	/**< month this recording was generated
                                 * in months. */
    Radx::si16 gen_day;		/**< day this recording was generated in days. */
    Radx::si16 number_sensor_des; /**< Total Number of sensor descriptors
                                 * that follow. */
  } volume_t;

  ////////////////////////////
  /// radar description - RADD

  typedef struct radar {
    
    char id[4];		/**< Identifier for a radar descriptor
                         * block (ascii characters "RADD"). */
    Radx::si32 nbytes;	/**< Length of a radar descriptor block in bytes. */
    char radar_name[8];	/**< Eight character radar name. */
    Radx::fl32 radar_const;	/**< Radar/lidar constant in ?? */
    Radx::fl32 peak_power;	/**< Typical peak power of the sensor in kw.
                                 * Pulse energy is really the
				 * peak_power * pulse_width */
    Radx::fl32 noise_power;	/**< Typical noise power of the sensor in dBm. */
    Radx::fl32 receiver_gain;	/**< Gain of the receiver in db. */
    Radx::fl32 antenna_gain;	/**< Gain of the antenna in db. */
    Radx::fl32 system_gain;	/**< System Gain in db.
				 * (Ant G - WG loss) */
    Radx::fl32 horz_beam_width;	/**< Horizontal beam width in degrees.
                                 * beam divergence in milliradians
				 * is equivalent to beamwidth */
    Radx::fl32 vert_beam_width;	/**< Vertical beam width in degrees. */
    Radx::si16 radar_type;	/**< Radar Type (0)Ground, 1)Airborne
                                 * Fore, 2)Airborne Aft, 3)airborne
                                 * Tail, 4)Airborne Lower Fuselage,
                                 * 5)Shipborne. */
    Radx::si16 scan_mode;	/**< Scan Mode (0)Calibration, 1)PPI
                                 * (constant Elevation) 2)Coplane,
                                 * 3)RHI (Constant Azimuth), 4)
                                 * Vertical Pointing, 5)Target
                                 * (Stationary), 6)Manual, 7)Idle (Out
                                 * of Control). */
    Radx::fl32 req_rotat_vel;	/**< Requested rotational velocity of
                                 * the antenna in degrees/sec. */
    Radx::fl32 scan_mode_pram0;	/**< Scan mode specific parameter #0
                                 * (Has different meaning for
                                 * different scan modes). */
    Radx::fl32 scan_mode_pram1;	/**< Scan mode specific parameter #1. */
    Radx::si16 num_parameter_des;/**< Total number of parameter
                                  * descriptor blocks for this radar. */
    Radx::si16 total_num_des;	/**< Total number of additional
                                 * descriptor block for this radar. */
    Radx::si16 data_compress;	/**< Data compression. 0 = none, 1 = HRD
                                 * scheme. */
    Radx::si16 data_reduction;	/**< Data Reduction algorithm: 1 = none,
                                 * 2 = between 2 angles, 3 = Between
                                 * concentric circles, 4 = Above/below
                                 * certain altitudes.*/
    Radx::fl32 data_red_parm0;	/**< 1 = smallest positive angle in
                                 * degrees, 2 = inner circle diameter,
                                 * km, 4 = minimum altitude, km. */
    Radx::fl32 data_red_parm1;	/**< 1 = largest positve angle, degress,
                                 * 2 = outer cicle diameter, km, 4 =
                                 * maximum altitude, km. */
    Radx::fl32 radar_longitude;	/**< longitude of radar in degrees. */
    Radx::fl32 radar_latitude;	/**< Latitude of radar in degrees. */
    Radx::fl32 radar_altitude;	/**<  Altitude of radar above msl in km. */
    Radx::fl32 eff_unamb_vel;	/**< Effective unambiguous velocity, m/s. */
    Radx::fl32 eff_unamb_range;	/**< Effective unambiguous range, km. */
    Radx::si16 num_freq_trans;	/**< Number of frequencies transmitted. */
    Radx::si16 num_ipps_trans;	/**< Number of different inter-pulse
                                 * periods transmitted. */
    Radx::fl32 freq1;		/**< Frequency 1. */
    Radx::fl32 freq2;		/**< Frequency 2. */
    Radx::fl32 freq3;		/**< Frequency 3. */
    Radx::fl32 freq4;		/**< Frequency 4. */
    Radx::fl32 freq5;		/**< Frequency 5. */
    Radx::fl32 prt1;      	/**< Interpulse period 1. */
    Radx::fl32 prt2;            /**< Interpulse period 2. */
    Radx::fl32 prt3;            /**< Interpulse period 3. */
    Radx::fl32 prt4;            /**< Interpulse period 4. */
    Radx::fl32 prt5;            /**< Interpulse period 5. */

				/**< 1995 extension #1 */
    Radx::si32  extension_num;  /**< not sure */
                                /**< harnessing this unused field for primary axis of rotation */
    char config_name[8];	/**< used to identify this set of
				 * unique radar characteristics */
    Radx::si32  config_num;	/**< facilitates a quick lookup of radar
				 * characteristics for each ray */
    /**
     * extend the radar descriptor to include unique lidar parameters
     */
    Radx::fl32 aperture_size;  /**< Diameter of the lidar aperature in cm. */
    Radx::fl32 field_of_view;  /**< Field of view of the receiver. mra; */
    Radx::fl32 aperture_eff;   /**< Aperature efficiency in %. */
    Radx::fl32 aux_freq[11];   /**< make space for a total of 16 freqs */
    Radx::fl32 aux_prt[11];    /**< and ipps */

    /**
     * other extensions to the radar descriptor
     */
    Radx::fl32 pulse_width;  /**< Typical pulse width in microseconds.
                              * pulse width is inverse of the
                              * band width */
    Radx::fl32 primary_cop_baseln; /**< coplane baselines */
    Radx::fl32 secondary_cop_baseln; /**< not sure */
    Radx::fl32 pc_xmtr_bandwidth;  /**< pulse compression
                                    * transmitter bandwidth */
    Radx::si32  pc_waveform_type;  /**< pulse compression waveform type */
    char site_name[20]; /**< instrument site name */
    
  } radar_t;

  /// some files have an older structure, with only the
  /// first part filled in

  static const int radar_alt_len = 144;

  /////////////////////////////
  /// correction factors - CFAC

  typedef struct correction {

    char id[4];	/**< Correction descriptor identifier: ASCII
                 * characters "CFAC" stand for Volume
                                 * Descriptor. */
    Radx::si32 nbytes; /**<Correction  descriptor length in bytes. */
    Radx::fl32 azimuth_corr;     /**< Correction added to azimuth[deg] */
    Radx::fl32 elevation_corr;   /**< Correction added to elevation[deg] */
    Radx::fl32 range_delay_corr; /**< Correction used for range delay[m] */
    Radx::fl32 longitude_corr;   /**< Correction added to radar longitude */
    Radx::fl32 latitude_corr;    /**< Correction added to radar latitude */
    Radx::fl32 pressure_alt_corr;/**< Correction added to pressure altitude
                                  * (msl)[km] */
    Radx::fl32 radar_alt_corr;   /**< Correction added to radar altitude above
                                  * ground level(agl) [km] */
    Radx::fl32 ew_gndspd_corr;   /**< Correction added to radar platform
                                  * ground speed(E-W)[m/s] */
    Radx::fl32 ns_gndspd_corr;   /**< Correction added to radar platform
                                  * ground speed(N-S)[m/s] */
    Radx::fl32 vert_vel_corr;    /**< Correction added to radar platform
                                  * vertical velocity[m/s] */
    Radx::fl32 heading_corr;     /**< Correction added to radar platform
                                  * heading [deg]) */
    Radx::fl32 roll_corr;        /**< Correction added to radar platform
                                  * roll[deg] */
    Radx::fl32 pitch_corr;       /**< Correction added to radar platform
                                  * pitch[deg] */
    Radx::fl32 drift_corr;       /**< Correction added to radar platform
                                  * drift[deg] */
    Radx::fl32 rot_angle_corr;   /**< Corrrection add to radar rotation angle
                                  *[deg] */
    Radx::fl32 tilt_corr;        /**< Correction added to radar tilt angle */
    
  } correction_t;

  /////////////////////////////////////////////
  /// parameter (data field) description - PARM

  typedef struct parameter {

    char id[4];	/**< Parameter Descriptor identifier
                 * (ascii characters "PARM"). */
    Radx::si32 nbytes;	/**< Parameter Descriptor length in bytes.*/
    char parameter_name[8]; /**< Name of parameter being described. */
    char param_description[40]; /**< Detailed description of this parameter. */
    char param_units[8];	/**< Units parameter is written in. */
    Radx::si16 interpulse_time;	/**< Inter-pulse periods used. bits 0-1
                                 * = frequencies 1-2. */
    Radx::si16 xmitted_freq;	/**< Frequencies used for this 
                                 * parameter. */
    Radx::fl32 recvr_bandwidth;	/**< Effective receiver bandwidth for
                                 * this parameter in MHz.*/
    Radx::si16 pulse_width;	/**< Effective pulse width of parameter
                                 * in m. */
    Radx::si16 polarization;	/**< Polarization of the radar beam for
                                 * this parameter (0 Horizontal,1
                                 * Vertical,2 Circular,3 Elliptical) in na. */
    Radx::si16 num_samples;	/**< Number of samples used in estimate
                                 * for this parameter. */
    Radx::si16 binary_format;	/**< Binary format of radar data. */
    char  threshold_field[8];	/**< Name of parameter upon which this
                                 * parameter is thresholded (ascii
                                 * characters NONE if not
                                 * thresholded). */
    Radx::fl32 threshold_value;	/**< Value of threshold in ? */
    Radx::fl32 parameter_scale;	/**< Scale factor for parameter. */
    Radx::fl32 parameter_bias;	/**< Bias factor for parameter. */
    Radx::si32  bad_data;	/**< Bad data flag. */
    
				/**< 1995 extension #1 */

    Radx::si32 extension_num;   /**< not sure */
    char config_name[8];	/**< used to identify this set of
				 * unique radar characteristics */
    Radx::si32 config_num;      /**< not sure */
    Radx::si32 offset_to_data;	/**< bytes added to the data struct pointer
				 * to point to the first datum whether it's
				 * an RDAT or a QDAT
				 */
    Radx::fl32 mks_conversion;  /**< not sure */
    Radx::si32 num_qnames;	/**< not sure */	
    char qdata_names[32];	/**< each of 4 names occupies 8 characters
				 * of this space
				 * and is blank filled. Each name identifies
				 * some interesting segment of data in a
				 * particular ray for this parameter.
				 */
    Radx::si32 num_criteria;    /**< not sure */
    char criteria_names[32];	/**< each of 4 names occupies 8 characters
				 * and is blank filled. These names identify
				 * a single interesting Radx::fl32ing point value
				 * that is associated with a particular ray
				 * for a this parameter. Examples might
				 * be a brightness temperature or
				 * the percentage of cells above or
				 * below a certain value */
    Radx::si32 number_cells;    /**< number of gates */
    Radx::fl32 meters_to_first_cell; /**< distance to center - meters */
    Radx::fl32 meters_between_cells; /**< gate spacing - meters */
    Radx::fl32 eff_unamb_vel;	/**< Effective unambiguous velocity, m/s. */

  } parameter_t;

  /// some files have an older structure, with only the
  /// first part filled in

  static const int parameter_alt_len = 104;

  /// dimension of dist_cells in cell_vector_t

  static const int MAXCVGATES = 1500;
  
  //////////////////////////////
  /// cell (gate) spacing - CELV
  
  typedef struct cell {

    char id[4]; /**< Cell descriptor identifier: ASCII
                 * characters "CELV" stand for cell vector. */
    Radx::si32 nbytes;	     /**< Comment descriptor length in bytes */
    Radx::si32 number_cells; /**< Number of sample volumes */
    Radx::fl32 dist_cells[MAXCVGATES]; /**< Distance from the radar to cell
                                        * n in meters */
    
  } cell_vector_t;
  
  //////////////////////////////
  /// cell spacing table - CSFD

  typedef struct cell_spacing_fp {

    char id[4];   /**< Identifier for a cell spacing descriptor
                   * (ascii characters CSFD). */
    Radx::si32 nbytes;  /**< Cell Spacing descriptor length in bytes. */
    Radx::si32 num_segments;  /**< Number of segments that contain cells of */
    Radx::fl32 dist_to_first; /**< Distance to first gate in meters. */
    Radx::fl32 spacing[8];    /**< Width of cells in each segment in m. */
    Radx::si16 num_cells[8] ; /**< Number of cells in each segment.
                               * equal widths. */

  } cell_spacing_fp_t;

  ////////////////////////////////////////
  /// sweep information description - SWIB

  typedef struct sweepinfo {
    char id[4];            /**< Comment descriptor identifier: ASCII
                            * characters "SWIB" stand for sweep info
                            * block Descriptor. */
    Radx::si32 nbytes;     /**< Sweep  descriptor length in bytes. */
    char radar_name[8];    /**< comment*/
    Radx::si32 sweep_num;  /**< Sweep number from the beginning of the volume*/
    Radx::si32 num_rays;   /**<number of rays recorded in this sweep*/
    Radx::fl32 start_angle;/**<true start angle [deg]*/
    Radx::fl32 stop_angle; /**<true stop angle  [deg]*/
    Radx::fl32 fixed_angle; /**< not sure */
    Radx::si32 filter_flag; /**< not sure */

  } sweepinfo_t;

  ///////////////////////////////
  /// platform description - ASIB

  typedef struct platform {

    char id[4];	/**< Identifier for the aircraft/ship
                 * parameters block (ascii characters ASIB) */
    Radx::si32 nbytes;/**< Length in Bytes of the
                       * aircraft/ship arameters block */
    Radx::fl32 longitude;	/**< Antenna longitude (Eastern
                                 * Hemisphere is positive, West
                                 * negative) in degrees */
    Radx::fl32 latitude;	/**< Antenna Latitude (Northern
                                 * Hemisphere is positive, South
                                 * Negative) in degrees */
    Radx::fl32 altitude_msl;	/**< Antenna Altitude above mean sea
                                 * level (MSL) in km */
    Radx::fl32 altitude_agl;	/**< Antenna Altitude above ground level
                                 * (AGL) in km */
    Radx::fl32 ew_velocity;	/**< Antenna east-west ground speed
                                 * (towards East is positive) in m/sec */
    Radx::fl32 ns_velocity;	/**< Antenna north-south ground speed
                                 * (towards North is positive) in m/sec */
    Radx::fl32 vert_velocity;	/**< Antenna vertical velocity (Up is
                                 * positive) in m/sec */
    Radx::fl32 heading;		/**< Antenna heading (angle between
                                 * rotodome rotational axis and true
                                 * North, clockwise (looking down)
                                 * positive) in degrees */
    Radx::fl32 roll;		/**< Roll angle of aircraft tail section
                                 * (Horizontal zero, Positive left wing up)
                                 * in degrees */
    Radx::fl32 pitch;		/**< Pitch angle of rotodome (Horizontal
                                 * is zero positive front up) in degrees*/
    Radx::fl32 drift_angle;	/**< Antenna drift Angle. (angle between
                                 * platform true velocity and heading,
                                 * positive is drift more clockwise
                                 * looking down) in degrees */
    Radx::fl32 rotation_angle;	/**< Angle of the radar beam with
                                 * respect to the airframe (zero is
                                 * along vertical stabilizer, positive
                                 * is clockwise) in deg */
    Radx::fl32 tilt;		/**< Angle of radar beam and line normal
                                 * to longitudinal axis of aircraft,
                                 * positive is towards nose of
                                 * aircraft) in degrees */
    Radx::fl32 ew_horiz_wind;	/**< east - west wind velocity at the
                                 * platform (towards East is positive)
                                 * in m/sec */
    Radx::fl32 ns_horiz_wind;	/**< North - South wind velocity at the
                                 * platform (towards North is 
                                 * positive) in m/sec */
    Radx::fl32 vert_wind;	/**< Vertical wind velocity at the
                                 * platform (up is positive) in m/sec */
    Radx::fl32 heading_change;	/**< Heading change rate in degrees/second. */
    Radx::fl32 pitch_change;	/**< Pitch change rate in degrees/second. */

  } platform_t;

  /////////////////////////////////////////////////////////////////////
  /// ray information - RYIB

  typedef struct ray {

    char id[4];		/**< Identifier for a data ray info.
                         * block (ascii characters "RYIB"). */
    Radx::si32 nbytes;	/**< length of a data ray info block in bytes. */
    Radx::si32 sweep_num;	/**< sweep number for this radar. */
    Radx::si32 julian_day;      /**< Guess. */
    Radx::si16 hour;		/**< Hour in hours. */
    Radx::si16 minute;		/**< Minute in minutes. */
    Radx::si16 second;		/**< Second in seconds. */
    Radx::si16 millisecond;	/**< Millisecond in milliseconds. */
    Radx::fl32 azimuth;		/**< Azimuth in degrees. */
    Radx::fl32 elevation;	/**< Elevation in degrees. */
    Radx::fl32 peak_power;	/**< Last measured peak transmitted
                                 * power in kw. */
    Radx::fl32 true_scan_rate;	/**< Actual scan rate in degrees/second. */
    Radx::si32 ray_status;	/**< 0 = normal, 1 = transition, 2 = bad. */

  } ray_t;

  ////////////////////////////////////////
  /// field parameter data - RDAT

  typedef struct paramdata {

    char id[4];	         /**< parameter data descriptor identifier: ASCII
                          * characters "RDAT" stand for parameter data
                          * block Descriptor. */
    Radx::si32 nbytes;   /**< parameter data descriptor length in bytes. */
    char pdata_name[8];  /**< name of parameter */

  } paramdata_t;
  
  ///////////////////////////////////////////
  /// field parameter data - extended - QDAT

  typedef struct qparamdata {
    
    char id[4]; /**< parameter data descriptor identifier: ASCII
                 * characters "QDAT" for a block that contains
                 * the data plus some supplemental and
                 * identifying information */
    
    Radx::si32 nbytes; /**< parameter data descriptor length in bytes.
                        * this represents the size of this header
                        * information plus the data
                        *
                        * for this data block the start of the data
                        * is determined by using "offset_to_data"
                        * in the corresponding parameter descriptor
                        * "struct parameter_d"
                        * the offset is from the beginning of
                        * this descriptor/block
                        */
    
    char pdata_name[8];	/**< name of parameter */

    Radx::si32 extension_num; /**< not sure */
    Radx::si32 config_num; /**< facilitates indexing into an array
                            * of radar descriptors where the radar
                            * characteristics of each ray and each
                            * parameter might be unique such as phased
                            * array antennas */

    Radx::si16 first_cell[4]; /**< see num_cells */
    Radx::si16 num_cells[4]; /**< first cell and num cells demark
                              * some feature in the data and it's
                              * relation to the cell vector
                              * first_cell[n] = 0 implies the first datum
                              * present corresponds to "dist_cells[0]
                              * in "struct cell_d"
                              * for TRMM data this would be the
                              * nominal sample where the cell vector is
                              * at 125 meter resolution instead of 250 meters
                              * and identified segments might be the
                              * rain echo oversample "RAIN_ECH" and the
                              * surface oversample "SURFACE" */

    Radx::fl32 criteria_value[4]; /**< criteria value associated
                                   * with a criteria name
                                   * in "struct parameter_d" */
  } qparamdata_t;


  //////////////////////////
  /// extra stuff - XSTF

  typedef struct extra_stuff {

    char id[4];	/**< "XSTF" */
    Radx::si32 nbytes; /**< number of bytesin this struct */
    
    Radx::si32 one;		/**< always set to one (endian flag) */
    Radx::si32 source_format;	/**< as per ../include/dd_defines.h */
    
    Radx::si32 offset_to_first_item; /**< bytes from start of struct */
    Radx::si32 transition_flag; /**< beam in transition? */

  } extra_stuff_t;

  ////////////////////////////
  /// null block - NULL

  typedef struct null_block {

    char id[4];	/**< "NULL" */
    Radx::si32 nbytes; /**< number of bytesin this struct */
    
  } null_block_t;

  /// entry for rotation angle table

  typedef struct rot_table_entry {
    Radx::fl32 rotation_angle; /**< azimuth or elevation angle, depending
                                * on scan mode */
    Radx::si32 offset; /**< offset of ray from start of file, in bytes */
    Radx::si32 size; /**< ray data length, in bytes */
  } rot_table_entry_t;
  
  /// rotation angle table - RKTB block

  typedef struct rot_ang_table {
    char id[4];	/**< "RKTB" */
    Radx::si32 nbytes; /**< number of bytesin this struct */
    Radx::fl32 angle2ndx; /**< ratio 360.0 / ndx_que_size */
    Radx::si32 ndx_que_size; /**< lookup table size */
    Radx::si32 first_key_offset; /**< offset of start of lookup table,
                                  * from start of file, in bytes */
    Radx::si32 angle_table_offset; /**< offset of start of angle table,
                                    * from start of file, in bytes */
    Radx::si32 num_rays; /**< number of rays in file */
  } rot_angle_table_t;

  /// radar angles for
  /// on-the-fly utility structure, not for file storage

  typedef struct radar_angles {
    double azimuth; /**< azimuth in degrees */
    double elevation; /**< azimuth in degrees */
    double x; /**< Cartesian X */
    double y; /**< Cartesian Y */
    double z; /**< Cartesian Z */
    double psi; /**< not sure */
    double rotation_angle; /**< rotation in degrees */
    double tilt; /**< tilt in degrees */
  } radar_angles_t;

  ////////////////////////////////////////////
  /// Radar test pulse and status block - FRAD

  typedef struct radar_test_status {

    char id[4]; /**< Field parameter data identifier
                 * (ascii characters FRAD) */
    Radx::si32 nbytes; /**< Length of the field parameter
                        * data block in bytes */
    Radx::si32 data_sys_status;	/**< Status word, bits will be assigned
                                 *  particular status when needed */
    char radar_name[8];	/**< Name of radar from which this data ray 
                         * came from */
    Radx::fl32 test_pulse_level; /**< Test pulse power level as measured by the
                                  *  power meter in dbm */
    Radx::fl32 test_pulse_dist; /**< Distance from antenna to middle of
                                 * test pulse in km */
    Radx::fl32 test_pulse_width; /**< Test pulse width in m  */
    Radx::fl32 test_pulse_freq; /**< Test pulse frequency in Ghz */
    Radx::si16 test_pulse_atten; /**< Test pulse attenuation in db */
    Radx::si16 test_pulse_fnum; /**< Frequency number being calibrated
                                 * with the test pulse (what mux on 
                                 * timing module is set to) */
    Radx::fl32 noise_power; /**< Total estimated noise power in dbm */
    Radx::si32 ray_count; /**< Data Ray counter For this
                           * particular type of data ray */
    Radx::si16 first_rec_gate; /**< First recorded gate number (N) */
    Radx::si16 last_rec_gate; /**< Last recorded gate number (M) */

  } radar_test_status_t;

  ////////////////////////////////////////
  /// Field radar information block - FRIB

  typedef struct field_radar {

    char id[4];	/**< Identifier for a field written
                 * radar information block
                 * (ascii characters FRIB). */
    Radx::si32 nbytes;	/**< Length of this field written radar
                         * information block in bytes. */
    Radx::si32 data_sys_id; /**< Data system identification. */
    Radx::fl32 loss_out; /**< Waveguide Losses between Transmitter and
                          * antenna in db. */
    Radx::fl32 loss_in;	/**< Waveguide Losses between antenna and Low
                         * noise amplifier in db. */
    Radx::fl32 loss_rjoint; /**< Losses in the rotary joint in db. */
    Radx::fl32 ant_v_dim; /**< Antenna Vertical Dimension in m. */
    Radx::fl32 ant_h_dim; /**< Antenna Horizontal Dimension in m. */
    Radx::fl32 ant_noise_temp; /**< Antenna Noise Temperature in degrees K. */
    Radx::fl32 r_noise_figure; /**< Receiver noise figure in dB*/
    Radx::fl32 xmit_power[5]; /**< Nominal Peak transmitted power in dBm
                               * by channel */
    Radx::fl32 x_band_gain; /**< X band gain in dB */
    Radx::fl32 receiver_gain[5]; /**< Measured receiver gain in dB (by channel) */
    Radx::fl32 if_gain[5]; /**< Measured IF gain in dB (by channel) */
    Radx::fl32 conversion_gain; /**< A to D conversion gain in dB */
    Radx::fl32 scale_factor[5]; /**< Scale factor to account for differences in
                                 * the individual channels, and the inherent
                                 * gain due to summing over the dwell time */
    Radx::fl32 processor_const; /**< Constant used to scale dBz to
                                 * units the display processors understand */
    Radx::si32 dly_tube_antenna; /**< Time delay from RF being applied to
                                  * tube and energy leaving antenna in ns. */
    Radx::si32 dly_rndtrip_chip_atod; /**< Time delay from a chip generated in
                                       * the yiming module and the RF pulse
                                       * entering the A to D converters.
                                       * Need to take the RF input to the HPA 
                                       * and inject it into the waveguide back
                                       * at the LNA to make this measurement
                                       * in ns */
    Radx::si32 dly_timmod_testpulse; /**< Time delay from timing Module test
                                      * pulse edge and test pulse arriving at
                                      * the A/D converter in ns. */
    Radx::si32 dly_modulator_on; /**< Modulator rise time (Time between
                                  * video on into HPA and modulator full up in
                                  * the high power amplifier) in ns. */
    Radx::si32 dly_modulator_off; /**< Modulator fall time (Time between
                                   * video off into the HPA
				   * and modulator full off) in ns. */
    Radx::fl32 peak_power_offset; /**< Added to the power meter reading of the
                                   * peak output power this yields actual
				   * peak output power (in dB) */ 
    Radx::fl32 test_pulse_offset; /**< Added to the power meter reading of the
                                   * test pulse this yields actual injected
                                   * test pulse power (dB) */
    Radx::fl32 E_plane_angle; /**< E-plane angle (tilt) this is the angle in
                               * the horizontal plane (when antennas are
                               * vertical) between a line normal to the
                               * aircraft's longitudinal axis and the radar
                               * beam in degrees.  Positive is in direction
                               * of motion (fore) */
    Radx::fl32 H_plane_angle; /**< H plane angle in degrees - this follows
                               * the sign convention described in the
                               * DORADE documentation for ROLL angle */
    Radx::fl32 encoder_antenna_up; /**< Encoder reading minus IRU roll angle
                                    * when antenna is up and horizontal */
    Radx::fl32 pitch_antenna_up; /**< Antenna pitch angle (measured with
                                  * transit) minus IRU pitch angle when
                                  * antenna is pointing up */
    Radx::si16 indepf_times_flg; /**< 0 = neither recorded, 1 = independent
                                  * frequency data only, 3 = independent 
                                  * frequency and time series data recorded */
    Radx::si16 indep_freq_gate; /**< gate number where the independent frequency
                                 * data comes from */
    Radx::si16 time_series_gate; /**< gate number where the time series data come
                                  * from */
    Radx::si16 num_base_params; /**< Number of base parameters. */
    char  file_name[80]; /**< Name of this header file. */

  } field_radar_t;

  ////////////////////////////////////////
  /// Lidar description - LIDR

  typedef struct lidar {    

    char id[4]; /**< Identifier  a lidar descriptor
                 * block (four ASCII characters
                 * "LIDR"). */
    Radx::si32 nbytes; /**< Length of a lidar descriptor block. */
    char lidar_name[8]; /**< Eight character lidar
                         * name. (Characters SABL) */
    Radx::fl32 lidar_const; /**< Lidar constant. */
    Radx::fl32 pulse_energy; /**< Typical pulse energy of the lidar. */
    Radx::fl32 peak_power; /**< Typical peak power of the lidar. */
    Radx::fl32 pulse_width; /**< Typical pulse width. */
    Radx::fl32 aperture_size; /**< Diameter of the lidar aperture. */
    Radx::fl32 field_of_view; /**< Field of view of the receiver. mra; */
    Radx::fl32 aperture_eff; /**< Aperture efficiency. */
    Radx::fl32 beam_divergence; /**< Beam divergence. */
    Radx::si16 lidar_type; /**< Lidar type: 0) Ground,  1) Airborne
                            * fore,  2) Airborne aft,  3)
                            * Airborne tail,  4) Airborne lower
                            * fuselage,  5) Shipborne. 6)
                            * Airborne Fixed */
    Radx::si16 scan_mode; /**< Scan mode:  0) Calibration,  1) PPI
                           * (constant elevation),  2) Co-plane,
                           * 3) RHI (Constant azimuth),  4)
                           * Vertical pointing up,  5) Target
                           * (stationary),  6) Manual,  7) Idle
                           * (out of control), 8) Surveillance,
                           * 9) Vertical sweep, 10) Vertical
                           * scan. 11) Vertical pointing down,
                           * 12 Horizontal pointing right, 13)
                           * Horizontal pointing left */
    Radx::fl32 req_rotat_vel; /**< Requested rotational velocity of
                               * the scan mirror. */
    Radx::fl32 scan_mode_pram0; /**< Scan mode specific parameter #0
                                 * (Has different meanings for
                                 * different scan modes) (Start angle
                                 * for vertical scanning). */
    Radx::fl32 scan_mode_pram1; /**< Scan mode specific parameter #1
                                 * (Has different meaning for
                                 * different scan modes) (Stop angle
                                 * for vertical scanning). */
    Radx::si16 num_parameter_des; /**< Total number of parameter
                                   * descriptor blocks for this lidar. */
    Radx::si16 total_num_des; /**< Total number of all descriptor
                               * blocks for this lidar. */
    Radx::si16 data_compress; /**< Data compression scheme in use:  0)
                               * no data compression, 1) using HRD
                               * compression scheme. */
    Radx::si16 data_reduction; /**< Data reduction algorithm in use:
                                * 0) None, 1) Between two angles, 2)
                                * Between concentric circles. 3)
                                * Above and below certain altitudes. */
    Radx::fl32 data_red_parm0; /**< Data reduction algorithm specific/
                                * parameter  #0:  0) Unused, 1)
                                * Smallest positive angle in degrees,
                                * 2) Inner circle diameter in km,  3)
                                * Minimum altitude in km. */
    Radx::fl32 data_red_parm1; /**< Data reduction algorithm specific
                                * parameter  #1 0) unused, 1) Largest
                                * positive angle in degrees, 2) Outer
                                * circle diameter in km,  3) Maximum
                                * altitude in km. */
    Radx::fl32 lidar_longitude; /**< Longitude of airport from which
                                 * aircraft took off  northern
                                 * hemisphere is positive, southern
                                 * negative. */
    Radx::fl32 lidar_latitude; /**< Latitude of airport from which
                                * aircraft took off eastern
                                * hemisphere is positive, western
                                * negative. */
    Radx::fl32 lidar_altitude; /**< Altitude of airport from which
                                * aircraft took off up is positive,
                                * above mean sea level. */
    Radx::fl32 eff_unamb_vel; /**< Effective unambiguous velocity. */
    Radx::fl32 eff_unamb_range; /**< Effective unambiguous range. */
    Radx::si32 num_wvlen_trans; /**< Number of different wave lengths
                                 * transmitted. */
    Radx::fl32 prf; /**< Pulse repetition frequency. */
    Radx::fl32 wavelength[10]; /**< Wavelengths of all the different
                                * transmitted light. */
  } lidar_t;

  ////////////////////////////////////////
  /// Field lidar information block - FLIB

  typedef struct field_lidar {

    char id[4];	/**< Identifier for a field written
                 * lidar information block
                 * (ascii characters FLIB). */
    Radx::si32 nbytes;	/**< Length of this field written lidar
                         * information block in bytes. */
    Radx::si32 data_sys_id; /**< Data system identification number. */
    Radx::fl32 transmit_beam_div[10]; /**< Transmitter beam divergence. Entry
                                       * [0] is for wavelength #1 etc. */
    Radx::fl32 xmit_power[10]; /**< Nominal peak transmitted power (by
                                * channel). Entry [0] is for
                                * wavelength #1 etc. */
    Radx::fl32 receiver_fov[10]; /**< Receiver field of view. */
    Radx::si32 receiver_type[10]; /**< 0=direct detection,no
                                   * polarization,1=direct detection
                                   * polarized parallel to transmitted 
                                   * beam,2 = direct detection, 
                                   * polarized perpendicular to 
                                   * transmitted beam,3= photon counting 
                                   * no polarization, 4= photon counting 
                                   * polarized parallel to transmitted 
                                   * beam,5 = photon counting, polarized 
                                   * perpendicular to transmitted beam. */
    Radx::fl32 r_noise_floor[10];    /**< Receiver noise floor. */
    Radx::fl32 receiver_spec_bw[10]; /**< Receiver spectral bandwidth */
    Radx::fl32 receiver_elec_bw[10]; /**< Receiver electronic bandwidth */
    Radx::fl32 calibration[10];      /**< 0 = linear receiver,  non zero log
                                      * reciever */
    Radx::si32 range_delay; /**< Delay between indication of
                             * transmitted pulse in the data 
                             * system and the pulse actually 
                             * leaving the telescope (can be 
                             * negative). */
    Radx::fl32 peak_power_multi[10]; /**< When the measured peak transmit
                                      * power is multiplied by this number 
                                      * it yields the actual peak transmit 
                                      * power. */
    Radx::fl32 encoder_mirror_up; /**< Encoder reading minus IRU roll 
                                   * angle when scan mirror is pointing 
                                   * directly vertically up in the roll 
                                   * axes. */
    Radx::fl32 pitch_mirror_up; /**< Scan mirror pointing angle in pitch 
                                 * axes, minus IRU pitch angle, when 
                                 * mirror is pointing directly 
                                 * vertically up in the roll axes. */
    Radx::si32 max_digitizer_count; /**< Maximum value (count) out of the 
                                     * digitizer  */
    Radx::fl32 max_digitizer_volt; /**< Voltage that causes the maximum 
                                    * count out of the digitizer. */
    Radx::fl32 digitizer_rate; /**< Sample rate of the digitizer. */
    Radx::si32 total_num_samples; /**< Total number of A/D samples to
                                   * take. */
    Radx::si32 samples_per_cell; /**< Number of samples average in range
                                    per data cell. */
    Radx::si32 cells_per_ray; /**< Number of data cells averaged
                                 per data ray. */
    Radx::fl32 pmt_temp; /**< PMT temperature */
    Radx::fl32 pmt_gain; /**< D/A setting for PMT power supply */
    Radx::fl32 apd_temp; /**< APD temperature */
    Radx::fl32 apd_gain; /**< D/A setting for APD power supply */
    Radx::si32 transect; /**< transect number */
    char derived_names[10][12]; /**< Derived parameter names */
    char derived_units[10][8]; /**< Derived parameter units */
    char temp_names[10][12]; /**< Names of the logged temperatures */

  } field_lidar_t;

  ///////////////////////////////////////////////
  /// entry for params list in insitu_descript_t

  typedef struct insitu_parameter {
    char name[8]; /**< parameter name */
    char units[8]; /**< parameter units */
  } insitu_parameter_t;
  
  //////////////////////////////
  /// in-situ parameters - SITU

  typedef struct insitu_descript {
    char id[4]; /**< Identifier = SITU. */
    Radx::si32 nbytes; /**< Block size in bytes. */
    Radx::si32 number_params;	/**< Number of paramters. */
    insitu_parameter_t params[256]; /**< Is this enough? */
  } insitu_descript_t;
  
  //////////////////////////////
  /// in-situ parameters - ISIT

  typedef struct insitu_data {
    char id[4];	/**< Identifier = ISIT. */
    Radx::si32 nbytes;	/**< Block size in bytes. */
    Radx::si16 julian_day; /**< day in year */
    Radx::si16 hours; /**< time - hours */
    Radx::si16 minutes; /**< time - minutes */
    Radx::si16 seconds; /**< time - seconds */
  } insitu_data_t;

  /////////////////////////////////
  /// independent frequency - INDF

  typedef struct indep_freq {
    char id[4];	/**< Identifier = INDF. */
    Radx::si32 nbytes;	/**< Block size in bytes. */
  } indep_freq_t;

  /////////////////////////////////
  /// MiniRims data - MINI

  typedef struct minirims_data {
    char id[4]; /**< Identifier = MINI. */
    Radx::si32 nbytes; /**< Block size in bytes. */
    Radx::si16 command; /**< Current command latch setting. */
    Radx::si16 status; /**< Current status. */
    Radx::fl32 temperature; /**< Degrees C. */
    Radx::fl32 x_axis_gyro[128]; /**< Roll axis gyro position. */
    Radx::fl32 y_axis_gyro[128]; /**< Pitch axis gyro position. */
    Radx::fl32 z_axis_gyro[128]; /**< Yaw axis gyro position. */
    Radx::fl32 xr_axis_gyro[128]; /**< Roll axis redundate gyro position. */
    Radx::fl32 x_axis_vel[128]; /**< Longitudinal axis velocity. */
    Radx::fl32 y_axis_vel[128]; /**< Lateral axis velocity. */
    Radx::fl32 z_axis_vel[128]; /**< Vertical axis velocity. */
    Radx::fl32 x_axis_pos[128]; /**< Roll axis gimbal. */
  } minirims_data_t;

  /////////////////////////////////
  /// Nav description - NDDS

  typedef struct nav_descript {
    char id[4]; /**< Identifier = NDDS. */
    Radx::si32 nbytes; /**< Block size in bytes. */
    Radx::si16 ins_flag; /**< 0 = no INS data, 1 = data recorded. */
    Radx::si16 gps_flag; /**< 0 = no GPS data, 1 = data recorded. */
    Radx::si16 minirims_flag; /**< 0 = no MiniRIMS data, 1 = data recorded. */
    Radx::si16 kalman_flag; /**< 0 = no kalman data, 1 = data recorded. */
  } nav_descript_t;

  /////////////////////////////////
  /// Time series data header - TIME

  typedef struct time_series {
    char id[4];	/**< Identifier = TIME. */
    Radx::si32 nbytes; /**< Block size in bytes. */
  } time_series_t;

  /////////////////////////////////
  /// Waveform descriptor - WAVE

  typedef struct waveform {

    char id[4];	/**< Identifier for the waveform
                 * descriptor (ascii characters "WAVE"). */
    Radx::si32 nbytes; /**< Length of the waveform descriptor
                        * in bytes. */
    char ps_file_name[16]; /**< Pulsing scheme file name.*/
    Radx::si16 num_chips[6]; /**< Number of chips in a repeat.
                              * sequence for each frequency. */
    char blank_chip[256]; /**< Blanking RAM sequence. */
    Radx::fl32 repeat_seq; /**< Number of milliseconds in a repeat
                            * sequence in ms. */
    Radx::si16 repeat_seq_dwel;	/**< Number of repeat sequences in a
                                 * dwell time. */
    Radx::si16 total_pcp; /**< Total Number of PCP in a repeat sequence. */
    Radx::si16 chip_offset[6]; /**< Number of 60 Mhz clock cycles to
				* wait before starting a particular
				* chip in 60 MHz counts. */
    Radx::si16 chip_width[6]; /**< Number of 60 Mhz clock cycles in
                               * each chip in 60 MHz counts. */
    Radx::fl32 ur_pcp; /**< Number of PCP that set the
                        * unambiguous range, after real time
                        * unfolding. */
    Radx::fl32 uv_pcp; /**< Number of PCP that set the
                        * unambiguous velocity, after real
                        * time unfolding. */
    Radx::si16 num_gates[6]; /**< Total number of gates sampled. */
    Radx::si16 gate_dist1[2]; /**< Distance from radar to data cell #1 
                               * in 60 MHz counts in 0, subsequent
                               * spacing in 1 for freq 1. */
    Radx::si16 gate_dist2[2]; /**< Ditto for freq 2. */
    Radx::si16 gate_dist3[2]; /**< Ditto for freq 3. */
    Radx::si16 gate_dist4[2]; /**< Ditto for freq 4. */
    Radx::si16 gate_dist5[2]; /**< Ditto for freq 5. */

  } waveform_t;

  ///////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////
  /// Constructor

  DoradeData();

  /// Destructor

  ~DoradeData();

  //////////////////////////////////////////////////////////////
  /// \name Structure initialization:
  //@{

  /// Initialize comment struct

  static void init(comment_t &val);

  /// Initialize SWIB struct

  static void init(super_SWIB_t &val);
  static void init(super_SWIB_32bit_t &val);

  /// copy between versions of Super_SWIB

  static void copy(const DoradeData::super_SWIB_t &val,
                   DoradeData::super_SWIB_32bit_t &val32);

  static void copy(const DoradeData::super_SWIB_32bit_t &val32,
                   DoradeData::super_SWIB_t &val);

  /// Initialize volume description struct

  static void init(volume_t &val);

  /// Initialize radar description struct

  static void init(radar_t &val);

  /// Initialize scalar corrections struct

  static void init(correction_t &val);

  /// Initialize field parameter struct

  static void init(parameter_t &val);

  /// Initialize cell vector struct

  static void init(cell_vector_t &val);

  /// Initialize cell spacing struct

  static void init(cell_spacing_fp_t &val);

  /// Initialize sweep information struct

  static void init(sweepinfo_t &val);

  /// Initialize platform georeference struct

  static void init(platform_t &val);

  /// Initialize ray header struct

  static void init(ray_t &val);

  /// Initialize field parameter struct

  static void init(paramdata_t &val);

  /// Initialize extended field parameter struct

  static void init(qparamdata_t &val);

  /// Initialize miscellaneous info struct

  static void init(extra_stuff_t &val);

  /// Initialize null struct

  static void init(null_block_t &val);

  /// Initialize rotation table entry struct
  
  static void init(rot_table_entry_t &val);
  
  /// Initialize rotation angle table struct

  static void init(rot_angle_table_t &val);

  /// Initialize radar test struct

  static void init(radar_test_status_t &val);

  /// Initialize field radar struct

  static void init(field_radar_t &val);

  /// Initialize lidar description struct

  static void init(lidar_t &val);

  /// Initialize field lidar struct

  static void init(field_lidar_t &val);

  /// Initialize in-situ description struct

  static void init(insitu_descript_t &val);

  /// Initialize in-situ data struct

  static void init(insitu_data_t &val);

  /// Initialize independent frequency struct

  static void init(indep_freq_t &val);

  /// Initialize minirims struct

  static void init(minirims_data_t &val);

  /// Initialize navigation description struct

  static void init(nav_descript_t &val);

  /// Initialize time series header struct

  static void init(time_series_t &val);

  /// Initialize waveform struct

  static void init(waveform_t &val);

  //@}

  //////////////////////////////////////////////////////////////
  /// \name Check whether a struct has been initialized or read:
  //@{
  
  /// Check if generic struct has has been initialized or read
  
  static bool isValid(const void *ddStruct);

  /// Check if comment struct has been initialized or read

  static bool isValid(const comment_t &val);
  
  /// Check if SWIB struct has been initialized or read.

  static bool isValid(const super_SWIB_t &val);
  static bool isValid(const super_SWIB_32bit_t &val);

  /// Check if volume struct has been initialized or read.

  static bool isValid(const volume_t &val);

  /// Check if radar description struct has been initialized or read.

  static bool isValid(const radar_t &val);

  /// Check if correction struct has been initialized or read.

  static bool isValid(const correction_t &val);

  /// Check if field param struct has been initialized or read.

  static bool isValid(const parameter_t &val);

  /// Check if cell vector struct has been initialized or read.

  static bool isValid(const cell_vector_t &val);

  /// Check if cell spacing struct has been initialized or read.

  static bool isValid(const cell_spacing_fp_t &val);

  /// Check if sweep information struct has been initialized or read.

  static bool isValid(const sweepinfo_t &val);

  /// Check if platform georeference struct has been initialized or read.

  static bool isValid(const platform_t &val);

  /// Check if ray struct has been initialized or read.

  static bool isValid(const ray_t &val);

  /// Check if parameter data struct has been initialized or read.

  static bool isValid(const paramdata_t &val);

  /// Check if extended parameter struct has been initialized or read.

  static bool isValid(const qparamdata_t &val);
  
  /// Check if miscellaneous struct has been initialized or read.

  static bool isValid(const extra_stuff_t &val);

  /// Check if null struct has been initialized or read.

  static bool isValid(const null_block_t &val);

  /// Check if rotation angle table struct has been initialized or read.

  static bool isValid(const rot_angle_table_t &val);

  /// Check if radar test struct has been initialized or read.

  static bool isValid(const radar_test_status_t &val);

  /// Check if field radar struct has been initialized or read.

  static bool isValid(const field_radar_t &val);

  /// Check if lidar description struct has been initialized or read.

  static bool isValid(const lidar_t &val);

  /// Check if field lidar struct has been initialized or read.

  static bool isValid(const field_lidar_t &val);

  /// Check if struct has been initialized or read.

  static bool isValid(const insitu_descript_t &val);

  /// Check if insitu data struct has been initialized or read.

  static bool isValid(const insitu_data_t &val);

  /// Check if independent frequency struct has been initialized or read.

  static bool isValid(const indep_freq_t &val);

  /// Check if minirims struct has been initialized or read.

  static bool isValid(const minirims_data_t &val);

  /// Check if nav description struct has been initialized or read.

  static bool isValid(const nav_descript_t &val);

  /// Check if time series header struct has been initialized or read.

  static bool isValid(const time_series_t &val);

  /// Check if waveform struct has been initialized or read.

  static bool isValid(const waveform_t &val);

  //@}

  //////////////////////////////////////////////////////////////
  /// \name Printing:
  //@{
  
  /// Print comment struct

  static void print(const comment_t &val, ostream &out);

  /// Print SWIB struct

  static void print(const super_SWIB_t &val, ostream &out);
  static void print(const super_SWIB_32bit_t &val, ostream &out);

  /// Print volume struct

  static void print(const volume_t &val, ostream &out);

  /// Print radar description struct

  static void print(const radar_t &val, ostream &out);

  /// Print scalar correction struct

  static void print(const correction_t &val, ostream &out);

  /// Print field parameter struct

  static void print(const parameter_t &val, ostream &out);

  /// Print cell location vector struct

  static void print(const cell_vector_t &val, ostream &out);

  /// Print cell spacing struct

  static void print(const cell_spacing_fp_t &val, ostream &out);

  /// Print sweep information struct

  static void print(const sweepinfo_t &val, ostream &out);

  /// Print platform georeference struct

  static void print(const platform_t &val, ostream &out);

  /// Print ray header struct

  static void print(const ray_t &val, ostream &out);

  /// Print field parameter struct

  static void print(const paramdata_t &val, ostream &out);

  /// Print extended field parameter struct

  static void print(const qparamdata_t &val, ostream &out);

  /// Print miscellaneous information struct

  static void print(const extra_stuff_t &val, ostream &out);

  /// Print null struct

  static void print(const null_block_t &val, ostream &out);

  /// Print rotation table entry struct

  static void print(const rot_table_entry_t &val, ostream &out);

  /// Print rotation angle table struct

  static void print(const rot_angle_table_t &val, ostream &out);

  /// Print radar test struct

  static void print(const radar_test_status_t &val, ostream &out);

  /// Print field radar struct

  static void print(const field_radar_t &val, ostream &out);

  /// Print lidar description struct

  static void print(const lidar_t &val, ostream &out);

  /// Print field lidar struct

  static void print(const field_lidar_t &val, ostream &out);

  /// Print insitu description struct

  static void print(const insitu_descript_t &val, ostream &out);

  /// Print insitu data struct

  static void print(const insitu_data_t &val, ostream &out);

  /// Print independent frequency struct

  static void print(const indep_freq_t &val, ostream &out);

  /// Print minirims struct

  static void print(const minirims_data_t &val, ostream &out);

  /// Print nav description struct

  static void print(const nav_descript_t &val, ostream &out);

  /// Print time series header struct

  static void print(const time_series_t &val, ostream &out);

  /// Print waveform struct

  static void print(const waveform_t &val, ostream &out);

  //@}

  //////////////////////////////////////////////////////////////
  /// \name Printing struct formats:
  //@{

  static const char *_hform;
  static const char *_dform;
  static const int _formDividerLen = 50;
  static void _printFormatDivider(char val, FILE *out);
  static void _printFormatHeader(FILE *out);

  /// Print format of all DORADE structs

  static void printAllFormats(FILE *out);

  /// Print format of comment struct

  static void printFormat(const comment_t &val, FILE *out);

  /// Print format of SWIB struct

  static void printFormat(const super_SWIB_t &val, FILE *out);

  /// Print format of volume struct

  static void printFormat(const volume_t &val, FILE *out);

  /// Print format of radar description struct

  static void printFormat(const radar_t &val, FILE *out);

  /// Print format of scalar correction struct

  static void printFormat(const correction_t &val, FILE *out);

  /// Print format of field parameter struct

  static void printFormat(const parameter_t &val, FILE *out);

  /// Print format of cell location vector struct

  static void printFormat(const cell_vector_t &val, FILE *out);

  /// Print format of cell spacing struct

  static void printFormat(const cell_spacing_fp_t &val, FILE *out);

  /// Print format of sweep information struct

  static void printFormat(const sweepinfo_t &val, FILE *out);

  /// Print format of platform georeference struct

  static void printFormat(const platform_t &val, FILE *out);

  /// Print format of ray header struct

  static void printFormat(const ray_t &val, FILE *out);

  /// Print format of field parameter struct

  static void printFormat(const paramdata_t &val, FILE *out);

  /// Print format of extended field parameter struct

  static void printFormat(const qparamdata_t &val, FILE *out);

  /// Print format of miscellaneous information struct

  static void printFormat(const extra_stuff_t &val, FILE *out);

  /// Print format of null struct

  static void printFormat(const null_block_t &val, FILE *out);

  /// Print format of rotation table entry struct

  static void printFormat(const rot_table_entry_t &val, FILE *out);

  /// Print format of rotation angle table struct

  static void printFormat(const rot_angle_table_t &val, FILE *out);

  /// Print format of radar test struct

  static void printFormat(const radar_test_status_t &val, FILE *out);

  /// Print format of field radar struct

  static void printFormat(const field_radar_t &val, FILE *out);

  /// Print format of lidar description struct

  static void printFormat(const lidar_t &val, FILE *out);

  /// Print format of field lidar struct

  static void printFormat(const field_lidar_t &val, FILE *out);

  /// Print format of insitu description struct

  static void printFormat(const insitu_descript_t &val, FILE *out);

  /// Print format of insitu data struct

  static void printFormat(const insitu_data_t &val, FILE *out);

  /// Print format of independent frequency struct

  static void printFormat(const indep_freq_t &val, FILE *out);

  /// Print format of minirims struct

  static void printFormat(const minirims_data_t &val, FILE *out);

  /// Print format of nav description struct

  static void printFormat(const nav_descript_t &val, FILE *out);

  /// Print format of time series header struct

  static void printFormat(const time_series_t &val, FILE *out);

  /// Print format of waveform struct

  static void printFormat(const waveform_t &val, FILE *out);

  //@}

  //////////////////////////////////////////////////////////////
  /// \name Byte swapping:
  //@{
  
  /// swap comment struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(comment_t &val, bool force = false);

  /// swap SWIB struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(super_SWIB_t &val, bool force = false);
  static void swap(super_SWIB_32bit_t &val, bool force = false);

  /// swap volume struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(volume_t &val, bool force = false);

  /// swap time fields in volume struct if on little-endian host.
  /// These may need swapping even if the volume as a whole does not.
  /// Swapping is done based on obtaining reasonable values
  /// for the time fields.

  static void swapTimeToReasonable(volume_t &val);


  /// swap radar description struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(radar_t &val, bool force = false);

  /// swap scalar correction struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(correction_t &val, bool force = false);

  /// swap struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(parameter_t &val, bool force = false);

  /// swap cell range vector struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(cell_vector_t &val, bool force = false);

  /// swap cell spacing struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(cell_spacing_fp_t &val, bool force = false);

  /// swap sweep information struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(sweepinfo_t &val, bool force = false);

  /// swap platform georeference struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(platform_t &val, bool force = false);

  /// swap ray header struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(ray_t &val, bool force = false);

  /// swap field parameter struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(paramdata_t &val, bool force = false);

  /// swap extended field parameter struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(qparamdata_t &val, bool force = false);

  /// swap miscellaneous information struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(extra_stuff_t &val, bool force = false);

  /// swap null struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(null_block_t &val, bool force = false);

  /// swap rotation table entry struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(rot_table_entry_t &val, bool force = false);

  /// swap rotation angle table struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(rot_angle_table_t &val, bool force = false);

  /// swap radar test struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(radar_test_status_t &val, bool force = false);

  /// swap field radar struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(field_radar_t &val, bool force = false);

  /// swap lidar description struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(lidar_t &val, bool force = false);

  /// swap field lidar struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(field_lidar_t &val, bool force = false);

  /// swap insitu description struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(insitu_descript_t &val, bool force = false);

  /// swap insitu data struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(insitu_data_t &val, bool force = false);

  /// swap independent frequency struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(indep_freq_t &val, bool force = false);

  /// swap minirims struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(minirims_data_t &val, bool force = false);

  /// swap nav description struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(nav_descript_t &val, bool force = false);

  /// swap time series header struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(time_series_t &val, bool force = false);

  /// swap waveform struct if on little-endian host.
  ///
  /// If force is true, swapping is forced on all hosts.

  static void swap(waveform_t &val, bool force = false);

  //@}

  //////////////////////////////////////////////////////////////
  /// \name Converting enums to strings:
  //@{
  
  /// convert binary to string

  static string binaryFormatToStr(DoradeData::binary_format_t format);

  /// convert radar type to string

  static string radarTypeToStr(DoradeData::radar_type_t rtype);

  /// convert lidar type to string

  static string lidarTypeToStr(DoradeData::lidar_type_t ltype);

  /// convert field ID to string

  static string fieldIdToStr(DoradeData::field_id_t id);

  /// convert polarization type to string

  static string polarizationToStr(DoradeData::polarization_t ptype);

  /// convert scan mode to string

  static string scanModeToStr(DoradeData::scan_mode_t mode);

  /// convert scan mode to abbreviated string string

  static string scanModeToShortStr(DoradeData::scan_mode_t mode);

  //@}

  ////////////////////////////////////////////////////////////// 
  /// \name Get int for enums and vice versa.
  //@{                                                                                                                                      

  static primary_axis_t primaryAxisFromInt(Radx::ui32 value);

  static Radx::ui32 primaryAxisToInt(DoradeData::primary_axis_t ptype);

  static Radx::PrimaryAxis_t convertToRadxType(DoradeData::primary_axis_t doradeAxis);

  static DoradeData::primary_axis_t convertToDoradeType(Radx::PrimaryAxis_t radxAxis);

  //@}                                              

  ////////////////////////////////////////////////////////////////////
  /// Decompress HRD 16-bit data.
  ///
  /// Routine to unpack data assuming MIT/HRD compression.
  ///
  /// The compression scheme simply encodes the bad values as a run,
  /// storing the run length. Good data is stored unchanged.
  ///
  /// Assumes data is in native byte order - i.e. it has already been
  /// swapped as appropriate.
  ///
  /// Inputs:
  ///   -comp: compressed data
  ///   -n_comp: number of compressed words
  ///   -bad_val: value to be used for bad data value
  ///
  /// Outputs:
  ///   -uncomp: uncompressed data
  ///            uncomp must be size at least max_uncomp
  ///   -n_bads: number of bad values
  ///
  /// Returns:
  ///   -number of 16-bit words in uncompressed data
  
  static int decompressHrd16(const unsigned short *comp, int n_comp,
                             unsigned short *uncomp, int max_uncomp,
                             int bad_val, int *n_bads);

  ///////////////////////////////////////////////////////////////////////
  /// Implement hrd compression of 16-bit values.
  ///
  /// The compression scheme simply encodes the bad values as a run,
  /// storing the run length. Good data is stored unchanged.
  //
  /// Assumes data is in native host byte order.
  //
  /// Inputs:
  ///   -uncomp: uncompressed data
  ///   -n_uncomp: number of uncompressed words
  ///   -bad_val: value to be used for bad data value
  ///   -max_comp: max number of compressed words,
  ///              should be at least n_uncomp + 16
  /// Output:
  ///   -comp: compressed data
  ///
  /// Returns:
  ///   -number of 16-bit words in compressed data

  static int compressHrd16(const unsigned short *uncomp, int n_uncomp,
                           unsigned short *comp, int max_comp,
                           unsigned short bad_val);
protected:
private:

  static const double missingDouble;
  static const float missingFloat;
  static const int missingInt;
  
}; /// DoradeData

# endif
