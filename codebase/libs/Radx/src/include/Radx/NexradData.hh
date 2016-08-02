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
/*************************************************************************
 * NexradData.hh
 *
 * Based on libs/rapformats/src/include/rapformats/ridds.h
 *
 * Mike Dixon, RAL, NCAR, Boulder, CO
 * Nov 2010
 *
 *************************************************************************/

#ifndef NexradData_HH
#define NexradData_HH

#include <string>
#include <ostream>
#include <Radx/Radx.hh>
using namespace std;

///////////////////////////////////////////////////////////////////////
/// CLASS DEFINING NEXRAD LEVEL 2 ARCHIVE FORMAT

class NexradData {

public:
  
  /*
   * Type definitions
   */

  /* ---- message types ---- */

  typedef enum {
    UNKNOWN_DATA = 0,
    DIGITAL_RADAR_DATA_1 = 1,
    RDA_STATUS_DATA = 2,
    PERFORMANCE_MAINTENANCE_DATA = 3,
    CONSOLE_MESSAGE_A2G = 4,
    VOLUME_COVERAGE_PATTERN = 5,
    RDA_CONTROL_COMMANDS = 6,
    VOLUME_COVERAGE_PATTERN2 = 7,
    CLUTTER_SENSOR_ZONES = 8,
    REQUEST_FOR_DATA = 9,
    CONSOLE_MESSAGE_G2A = 10,
    LOOPBACK_TEST_RDA_RPG = 11,
    LOOPBACK_TEST_RGD_RPA = 12,
    CLUTTER_FILTER_BYPASS_MAP = 13,
    EDITED_CLUTTER_FILTER_MAP = 14,
    CLUTTER_FILTER_MAP = 15,
    RDA_ADAPTATION_DATA = 18,
    DIGITAL_RADAR_DATA_31 = 31
  } message_type_t;

  /* required for receiving data through the NSSL RPG */

  typedef enum {
    BEGIN_NEW_VOLUME = 201
  } nssl_rpg_code_t;

  /* ---- radial status ---- */

  typedef enum {
    START_OF_NEW_ELEVATION = 0,
    INTERMEDIATE_RADIAL = 1,
    END_OF_ELEVATION = 2,
    BEGINNING_OF_VOL_SCAN = 3,
    END_OF_VOL_SCAN = 4,
    END_OF_TILT_FLAG = 999,
    BAD_AZI_FLAG = 888
  } radial_status_t;

  /* ---- doppler velocity resolution ---- */

  typedef enum {
    POINT_FIVE_METERS_PER_SEC = 2,
    ONE_METER_PER_SEC = 4
  } vel_res_t;

  /* ---- volume coverage patterns ---- */

  typedef enum {
    SCAN_16_PER_5_MIN = 11,
    SCAN_11_PER_5_MIN = 21,
    SCAN_8_PER_10_MIN = 31,
    SCAN_7_PER_10_MIN = 32
  } vcp_type_t;

  // static const double HORIZ_BEAM_WIDTH;
  // static const double VERT_BEAM_WIDTH;
  // static const double FIXED_ANGLE;
  // static const double AZ_RATE;
  // static const double NOMINAL_WAVELN;

  // static const size_t NUM_DELTAS = 5;
  // static const size_t NUM_UNAMB_RNGS = 8;
  // static const size_t MAX_REC_LEN = 145920;
  // static const size_t MAX_FIELDS = 8;
  // static const int DZ_ID = 1;
  // static const int VEL_ID = 2;
  // static const int SW_ID = 4;
  // static const int DZ_ONLY = 1;
  // static const int VE_AND_SW_ONLY = 6;
  // static const int DZ_VE_SW_ALL = 7;

  static const size_t PACKET_SIZE = 2432;

  typedef struct {
    char filename[8];
  } id_rec_t;

  typedef struct {
    Radx::ui16 vol_number;
  } vol_number_t;

  typedef struct {
    char filetype[9]; /* Filetype: ARCHIVE2, ARV2xxxx */
    char vol_num[3]; /* Volume number 001, 002 etc, often blank */
    Radx::si16 julian_date; /* Modified julian date referenced from 1/1/70 */
    Radx::si16 unused1;
    Radx::si32 millisecs_past_midnight; /* Time - Millisecs of day from midnight
                                         * (GMT) when file was created */
    Radx::si32 unused2;
  } vol_title_t;

  typedef struct {
    Radx::si16 word1; /* This stuff is to be ignored. It is used for checking */
    Radx::si16 word2; /* data integrity during the transmission process */ 
    Radx::si16 word3;
    Radx::si16 word4;
    Radx::si16 word5;
    Radx::si16 word6;
  } ctm_info_t;

  /***************** Message header ***********/

  typedef struct {
    
    Radx::ui16 message_len; /* (7) Message size in halfwords measured from 
                             * this halfword to end of record */
 
    Radx::ui08 channel_id; /* (8.1) Redundant channel id */
    Radx::ui08 message_type; /* (8.2) Message type, where:
                              * 1 = digital radar data
                              * 2 = rda status data
                              * 3 = performance/maintenance data
                              * 4 = console message - rda to rpg
                              * 5 = maintenance log data
                              * 6 = rda control commands
                              * 7 = volume coverage pattern
                              * 8 = clutter sensor zones
                              * 9 = request for data
                              * 10 = console message - rpg to rda
                              * 11 = loop back test - rda to rpg
                              * 12 = loop back test - rpg to rda
                              * 13 = clutter filter bypass map - rda -> rpg
                              * 14 = edited clutter filter map rpg -> rda
                              */
 
    Radx::ui16 seq_num; /* (9) I.D. sequence = 0 to 7fff, then to 0. */
    Radx::ui16 julian_date; /* (10) Modified julian date from * 1/1/70 */ 
 
    Radx::ui32 millisecs_past_midnight; /* (11-12) Generation time of messages in 
                                         * millisecs of day past midnight (GMT). 
                                         * This time may be different than time
                                         * listed in halfwords 15-16 defined
                                         * below */
 
    Radx::ui16 num_message_segs;	/* (13) Number of message segments. */
    Radx::ui16 message_seg_num; /* (14) Message segment number */
 
  } msg_hdr_t;

  /************ message type 1 ***************/

  typedef struct {

    Radx::si32 millisecs_past_midnight; /* (15-16) Collection time for this radial in
                                         * millisecs of day past midnight (GMT). */
    Radx::si16 julian_date; /* (17) Modified julian date from 1/1/70 */
    Radx::si16 unamb_range_x10; /* (18) Unambiguous range (scaled: val/10 = KM) */
    Radx::ui16 azimuth; /* (19) Azimuth angle
                         * (coded: (val/8)*(180/4096) = DEG). An azimuth
                         * of "0 degs" points to true north while "90 degs"
                         * points east. Rotation is always counterclockwise
                         * as viewed from above the radar. */
    Radx::si16 radial_num; /* (20) Radial number within elevation scan */
    Radx::si16 radial_status; /* (21) Radial status where:
                               * 0 = start of new elevation
                               * 1 = intermediate radial
                               * 2 = end of elevation
                               * 3 = beginning of volume scan
                               * 4 = end of volume scan */
    Radx::ui16 elevation; /* (22) Elevation angle
                           * (coded: (val/8)*(180/4096) = DEG). An elevation
                           * of '0 degs' is parallel to the pedestal base
                           * while '90 degs' is perpendicular to the
                           * pedestal base. */
    Radx::si16 elev_num; /* (23) RDA elevation number within volume scan */
    Radx::si16 ref_gate1; /* (24) Range to first gate of reflectivity data
                           * METERS */
    Radx::si16 vel_gate1; /* (25) Range to first gate of Doppler data.
                           * Doppler data - velocity and spectrum width
                           * METERS */
    Radx::si16 ref_gate_width; /* (26) Reflectivity data gate size METERS */
    Radx::si16 vel_gate_width; /* (27) Doppler data gate size METERS */ 
    Radx::si16 ref_num_gates; /* (28) Number of reflectivity gates */
    Radx::si16 vel_num_gates; /* (29) Number of velocity and/or spectrum width
                               * data gates */
    Radx::si16 sector_num; /* (30) Sector number within cut */
    Radx::fl32 sys_gain_cal_const; /* (31-32) System gain calibration constant
                                    * (dB biased). */
    Radx::si16 ref_ptr; /* (33) Reflectivity data pointer (byte # from
                         * start of digital radar message header). This
                         * pointer locates the beginning of reflectivity
                         * data. */
    Radx::si16 vel_ptr; /* (34) Velocity data pointer (byte # from
                         * start of digital radar message header). This
                         * pointer locates the beginning of velocity
                         * data. */
    Radx::si16 sw_ptr; /* (35) Spectrum width pointer (byte # from
                        * start of digital radar message header). This
                        * pointer locates the beginning of spectrum width
                        * data. */
    Radx::si16 velocity_resolution; /* (36) Doppler velocity resolution
                                     * 2 = 0.5 m/s
                                     * 4 = 1.0 m/s */
    Radx::si16 vol_coverage_pattern; /* (37) Volume coverage pattern
                                      * 11 = 16 elev scans / 5 mins.
                                      * 21 = 11 elev scans / 6 mins.
                                      * 31 = 8 elev scans / 10 mins.
                                      * 32 = 7 elev scans / 10 mins. */
    Radx::si16 word_23; /* unused */
    Radx::si16 word_24; /* unused */
    Radx::si16 word_25; /* unused */
    Radx::si16 word_26; /* unused */
    Radx::si16 ref_data_playback;/* (42) Reflectivity data pointer for Archive II
                                  * playback */
    Radx::si16 vel_data_playback;/* (43) Velocity data pointer for Archive II
                                  * playback */
    Radx::si16 sw_data_playback; /* (44) Spectrum width data pointer for Archive II
                                  * playback */
    Radx::si16 nyquist_vel; /* (45) Nyquist velocity (scaled: val/100 = m/s) */
    Radx::si16 atmos_atten_factor; /* (46) Atmospheric attenuation factor.
                                    * (scaled: val/1000 = dB/KM) */
    Radx::si16 threshold_param; /* Threshold parameter for minimum difference in
                                 * echo power between two resolution volumes for
                                 * them not to be labeled range ambiguous (i.e.
                                 * overlaid). */
    Radx::si16 spot_blank_status; /* 0: spot blanking disabled
                                   * 4: enabled and none in current cut
                                   * 6: enabled and some in current cut
                                   * 7: current radial is blanked */
    Radx::si16 word_34; /* unused */
    Radx::si16 word_35; /* unused */
    Radx::si16 word_36; /* unused */
    Radx::si16 word_37; /* unused */
    Radx::si16 word_38; /* unused */
    Radx::si16 word_39; /* unused */
    Radx::si16 word_40; /* unused */
    Radx::si16 word_41; /* unused */
    Radx::si16 word_42; /* unused */
    Radx::si16 word_43; /* unused */
    Radx::si16 word_44; /* unused */
    Radx::si16 word_45; /* unused */
    Radx::si16 word_46; /* unused */
    Radx::si16 word_47; /* unused */
    Radx::si16 word_48; /* unused */
    Radx::si16 word_49; /* unused */

  } message_1_t;

  /************* Geometry header *******************/

  typedef struct {

    /* Time6 time;*/
    Radx::si32 nazimuths;
    Radx::si32 ref_ngates;
    Radx::si32 vel_ngates;
    Radx::si32 ref_resolution;
    Radx::si32 vel_resolution;
    Radx::fl32 elevation;

  } geom_hdr_t;

  /* Header of message type CLUTTER_FILTER_BYPASS_MAP and CLUTTER_FILTER_MAP */

  typedef struct {

    Radx::ui16 julian_date; /* (15) Modified julian date from * 1/1/70 */ 
    Radx::ui16 minutes_past_midnight; /* (16) Generation time of messages in 
                                       * minutes of day past midnight (GMT). */
    Radx::ui16 num_message_segs; /* (17) Number of elevation segments. */

  } clutter_hdr_t;

  /* Message type CLUTTER_FILTER_MAP after the above header
     is of following:
     For each elevation segment (num_message_segs)
     For each Azimuth segment (always 360)
     Radx::ui16 num_range_zones; (1-20)
     For each range zone ( Last range zone must have end range = 511)
     Radx::ui16 op_code; (0=Bypass Filter,
                          1=Bypass map in control,
                          2=Force Filter)
     Radx::ui16 end_range (0-511)
  */

  /* Message type CLUTTER_FILTER_BYPASS_MAP after the above header is of following:
     For each elevation segment (num_message_segs)
     Radx::ui16 segment_num; (1-5)
     For each Azimuth radial (always 360)
     Radx::ui16 bins0_15; (Range bins 0 to 15, one range bin per bit)
     Radx::ui16 bins16_31; (Range bins 16 to 31)
     ... (Each bit represents a range bin. 0=perform clutter filtering,
     1=bypass the clutter filters)
     Radx::ui16 bins496_511; (Range bins 496 to 511)
  */

  /* Message type VOLUME_COVERAGE_PATTERN and VOLUME_COVERAGE_PATTERN2 */

  typedef struct {

    Radx::ui16 message_len; /* Number of Halfwords in message */
    Radx::ui16 pattern_type; /* Constant Elevation Cut */
    Radx::ui16 pattern_number; /* VCP pattern number */
    Radx::ui16 num_elevation_cuts; /* Num elev cuts in one complete volume scan */
    Radx::ui16 clutter_map_group; /* Clutter map groups not currently implemented */
    Radx::ui08 dop_vel_resolution; /* Doppler velocity resolution 2 = .5, 4 = 1.0*/
    Radx::ui08 pulse_width; /* Pulse width values 2 = Short, 4 = Long */
    Radx::ui16 spare1;
    Radx::ui16 spare2;
    Radx::ui16 spare3;
    Radx::ui16 spare4;
    Radx::ui16 spare5;
    /* VCP header is followed by num_elevation_cuts of elevation_angle */

  } VCP_hdr_t;

  typedef struct {
  
    Radx::ui16 elevation_angle; /* The elevation angle for this cut
                                 * (coded: (val/8)*(180/4096) = DEG) */
    Radx::ui08 channel_config; /* 0 = Constant Phase, 1 = Random Phase, 2 = SZ2 Phase */
    Radx::ui08 waveform_type; /* 1 = Contiguous Surv,
                                 2 = Contiguous Doppler with ambiguity Resolution
                                 3 = Contiguous Doppler W/o ambiguity Resolution, 4 = Batch,
                                 5 = Staggered Pulse Pair */
    Radx::ui08 super_res_control; /* Bit 0 = 0.5 degree azimuth, Bit 1= 1/4km reflectivity, 
                                     Bit 2 = doppler to 300km */
    Radx::ui08 surveillance_prf_num; /* pulse repetition frequency number
                                        for surveillance cuts */
    Radx::ui16 surveillance_prf_pulse_count; /* pulse count per radial
                                                for surveillance cuts */
    Radx::si16 azimuth_rate; /* (coded: (val/8)*(22.5/2048) = DEG/s) */
    Radx::si16 reflectivity_thresh; /* SNR threshold for reflectivity
                                       (scaled: val/100 = dB) */
    Radx::si16 velocity_thresh; /* SNR threshold for velocity
                                   (scaled: val/100 = dB) */
    Radx::si16 spectrum_width_thresh; /* SNR threshold for spectrum width
                                         (scaled: val/100 = dB) */
    Radx::ui16 spare1;
    Radx::ui16 spare2;
    Radx::ui16 spare3;
    Radx::ui16 edge_angle1; /* sector 1 Azimuth clockwise edge angle (start of angle) 
                               (coded: (val/8)*(180/4096) = DEG) */
    Radx::ui16 doppler_prf_number1; /* sector 1 doppler prf number */
    Radx::ui16 doppler_prf_pulse_count1; /* sector 1 doppler pulse count per radial */
    Radx::ui16 spare4;
    Radx::ui16 edge_angle2; /* sector 2 Azimuth clockwise edge angle (start of angle)
                               (coded: (val/8)*(180/4096) = DEG) */
    Radx::ui16 doppler_prf_number2; /* sector 2 doppler prf number */
    Radx::ui16 doppler_prf_pulse_count2; /* sector 2 doppler pulse count per radial */
    Radx::ui16 spare5;
    Radx::ui16 edge_angle3; /* sector 3 Azimuth clockwise edge angle (start of angle)
                               (coded: (val/8)*(180/4096) = DEG) */
    Radx::ui16 doppler_prf_number3; /* sector 3 doppler prf number */
    Radx::ui16 doppler_prf_pulse_count3; /* sector 3 doppler pulse count per radial */
    Radx::ui16 spare6;

  } ppi_hdr_t;

  /* Message type RDA_ADAPTATION_DATA */

  typedef struct {

    char adap_file_name[12]; /* Name of Adaptation data file (0 - 11)*/
    char adap_format[4]; /* Format of Adaptation data file (12 - 15)*/
    char adap_revision[4]; /* Revision number of Adaptation data file (16 - 19)*/
    char adap_date[12]; /* Last modified date Adaptation data file (20 - 31)*/
    char adap_time[12]; /* Last modified time of Adaptation data file (32 - 43)*/
    Radx::fl32 k1; /* azimuth position gain factor (k1) (44 - 47)*/
    Radx::fl32 az_lat; /* latency of dcu azimuth measurement (48 - 51)*/
    Radx::fl32 k3; /* elevation position gain factor (k3) (52 - 55)*/
    Radx::fl32 el_lat; /* latency of dcu elevation measurement (56 - 59)*/
    Radx::fl32 parkaz; /* pedestal park position in azimuth (60 - 63)*/
    Radx::fl32 parkel; /* pedestal park position in elevation (64 - 67)*/
    Radx::fl32 a_fuel_conv[11]; /* generator fuel level height/capacity conversion
                                   (68 - 111)*/
    Radx::fl32 a_min_shelter_temp; /* minimum equipment shelter alarm temperature
                                      (112 - 115)*/
    Radx::fl32 a_max_shelter_temp; /* maximum equipment shelter alarm temperature
                                      (116 - 119)*/
    Radx::fl32 a_min_shelter_ac_temp_diff; /* minimum a/c discharge air temperature
                                              differential (120 - 123)*/
    Radx::fl32 a_max_xmtr_air_temp; /* maximum transmitter leaving air alarm
                                       temperature (124 - 127)*/
    Radx::fl32 a_max_rad_temp; /* maximum radome alarm temperature (128 - 131)*/
    Radx::fl32 a_max_rad_temp_rise; /* maximum radome minus ambient temperature
                                       difference (132 - 135)*/
    Radx::fl32 ped_28v_reg_lim; /* pedestal +28 volt power supply tolerance (136 - 139)*/
    Radx::fl32 ped_5v_reg_lim; /* pedestal +5 volt power supply tolerance (140 - 143)*/
    Radx::fl32 ped_15v_reg_lim; /* pedestal +/- 15 volt power supply tolerance (144 - 147)*/
    Radx::fl32 a_min_gen_room_temp; /* minimum generator shelter alarm temperature
                                       (148 - 151)*/
    Radx::fl32 a_max_gen_room_temp; /* maximum generator shelter alarm temperature
                                       (152 - 155)*/
    Radx::fl32 dau_5v_reg_lim; /* dau +5 volt power supply tolerance (156 - 159)*/
    Radx::fl32 dau_15v_reg_lim; /* dau +/- 15 volt power supply tolerance (160 - 163)*/
    Radx::fl32 dau_28v_reg_lim; /* dau +28 volt power (164 - 167)*/
    Radx::fl32 en_5v_reg_lim; /* encoder +5 volt power supply tolerance (168 - 171)*/
    Radx::fl32 en_5v_nom_volts; /* encoder +5 volt power supply nominal voltage (172 - 175)*/
    char rpg_co_located[4]; /* rpg co-located (176 - 179)*/
    char spec_filter_installed[4];/* transmitter spectrum filter installed (180 - 183)*/
    char tps_installed[4]; /* transition power source installed (184 - 187)*/
    char rms_installed[4]; /* faa rms installed (188 - 191)*/
    Radx::ui32 a_hvdl_tst_int; /* performance test interval (192 - 195)*/
    Radx::ui32 a_rpg_lt_int; /* rpg loop test interval (196 - 199)*/
    Radx::ui32 a_min_stab_util_pwr_time; /* required interval time for stable
                                            utility power (200 - 203)*/
    Radx::ui32 a_gen_auto_exer_interval; /* maximum generator automatic
                                            exercise interval (204 - 207)*/
    Radx::ui32 a_util_pwr_sw_req_interval; /* recommended switch to utility
                                              power time interval (208 - 211)*/
    Radx::fl32 a_low_fuel_level; /* low fuel tank warning level (212 - 215)*/
    Radx::ui32 config_chan_number; /* configuration channel number (216 - 219)*/
    Radx::ui32 a_rpg_link_type; /* rpg wideband link type (0 = direct, 1 = microwave,
                                   2 = fiber optic) (220 - 223)*/
    Radx::ui32 redundant_chan_config; /* redundant channel configuration
                                         (1 = single chan, 2 = faa, 3 = nws redundant)
                                         (224 - 227)*/
    Radx::fl32 atten_table[104]; /* test signal attenuator insertion losses
                                    (0db - 103db) (228 - 643)*/
    Radx::fl32 path_losses[69]; /* path loss (644 - 919)*/
    Radx::fl32 chan_cal_diff; /* noncontrolling channel calibration difference (920-923)*/
    Radx::fl32 path_losses_70_71; /* spare locations in the path_losses array (924 - 927)*/
    Radx::fl32 log_amp_factor_scale; /* rf detector log amplifier scale factor
                                        for converting receiver test data (928 - 931)*/
    Radx::fl32 log_amp_factor_bias; /* rf detector log amplifier bias for
                                       converting receiver test data (932 - 935)*/
    Radx::ui32 spare_1; /* n/a (936 - 939)*/
    Radx::fl32 rnscale[13]; /* receiver noise normalization (-1.0 deg to -0.5 deg)
                               (940 - 991)*/
    Radx::fl32 atmos[13]; /* two way atmospheric loss/km (-1.0 deg to -0.5 deg)
                             (992 - 1043)*/
    Radx::fl32 el_index[12]; /* bypass map generation elevation angle (1044 - 1091)*/
    Radx::ui32 tfreq_mhz; /* transmitter frequency (1092 - 1095)*/
    Radx::fl32 base_data_tcn; /* point clutter suppression threshold (tcn) (1096 - 1099)*/
    Radx::fl32 refl_data_tover; /* range unfolding overlay threshold (tover)
                                   (1100 - 1103)*/
    Radx::fl32 tar_dbz0_lp; /* target system calibration (dbz0) for long pulse
                               (1104 - 1107)*/
    Radx::ui32 spare_2; /* n/a (1108 - 1111)*/
    Radx::ui32 spare_3; /* n/a (1112 - 1115)*/
    Radx::ui32 spare_4; /* n/a (1116 - 1119)*/
    Radx::fl32 lx_lp; /* matched filter loss for long pulse (1120 - 1123)*/
    Radx::fl32 lx_sp; /* matched filter loss for short pulse (1124 - 1127)*/
    Radx::fl32 meteor_param; /* /k/xx2 hydrometeor refractivity factor (1128 - 1131)*/
    Radx::fl32 beamwidth; /* antenna beamwidth (1132 - 1135)*/
    Radx::fl32 antenna_gain; /* antenna gain including radome (1136 - 1139)*/
    Radx::ui32 spare_5; /* n/a (1140 - 1143)*/
    Radx::fl32 vel_maint_limit; /* velocity check delta maintenance limit (1144 - 1147)*/
    Radx::fl32 wth_maint_limit; /* spectrum width check delta maintenance limit
                                   (1148 - 1151)*/
    Radx::fl32 vel_degrad_limit; /* velocity check delta degrade limit (1152 - 1155)*/
    Radx::fl32 wth_degrad_limit; /* spectrum width check delta degrade limit (1156 - 1159)*/
    Radx::fl32 noisetemp_dgrad_limit; /* system noise temperature degrade limit
                                         (1160 - 1163)*/
    Radx::fl32 noisetemp_maint_limit; /* system noise temperature maintenance limit
                                         (1164 - 1167)*/
    Radx::ui32 spare_6; /* n/a (1168 - 1171)*/
    Radx::ui32 spare_7; /* n/a (1172 - 1175)*/
    Radx::fl32 kly_degrade_limit; /* klystron output target consistency degrade limit
                                     (1176 - 1179)*/
    Radx::fl32 ts_coho; /* coho power at A1J4 (1180 - 1183)*/
    Radx::fl32 ts_cw; /* cw test signal at A22J3 (1184 - 1187)*/
    Radx::fl32 ts_rf_sp; /* rf drive test signal short pulse at 3A5J4 (1188 - 1191)*/
    Radx::fl32 ts_rf_lp; /* rf drive test signal long pulse at 3A5J4 (1192 - 1195)*/
    Radx::fl32 ts_stalo; /* stalo power at A1J2 (1196 - 1199)*/
    Radx::fl32 ts_noise; /* rf noise test signal excess noise ratio at A22J4 (1200 - 1203)*/
    Radx::fl32 xmtr_peak_pwr_high_limit; /* maximum transmitter peak power alarm level
                                            (1204 - 1207)*/
    Radx::fl32 xmtr_peak_pwr_low_limit; /* minimum transmitter peak power alarm level
                                           (1208 - 1211)*/
    Radx::fl32 dbz0_delta_limit; /* limit for difference between computed and target
                                    system calibration coefficient (dbz0) (1212 - 1215)*/
    Radx::fl32 threshold1; /* bypass map generator noise threshold (1216 - 1219)*/
    Radx::fl32 threshold2; /* bypass map generator rejection ratio threshold (1220 - 1223)*/
    Radx::fl32 clut_supp_dgrad_lim; /* clutter suppression degrade limit (1224 - 1227)*/
    Radx::fl32 clut_supp_maint_lim; /* clutter suppression maintenance limit (1228 - 1231)*/
    Radx::fl32 range0_value; /* true range at start of first range bin (1232 - 1235)*/
    Radx::fl32 xmtr_pwr_mtr_scale; /* scale factor used to convert transmitter power
                                      byte data to watts (1236 - 1239)*/
    Radx::fl32 n_smooth; /* receiver noise calibration smoothing coefficient (1240 - 1243)*/
    Radx::fl32 tar_dbz0_sp; /* target system calibration (dbz0) for short pulse
                               (1244 - 1247)*/
    Radx::ui32 spare_8; /* n/a (1248 - 1251)*/
    Radx::ui32 deltaprf; /* site prf set (A=1, B=2, C=3, D=4, E=5) (1252 - 1255)*/
    Radx::ui32 spare_9; /* n/a (1256 - 1259)*/
    Radx::ui32 spare_10; /* n/a (1260 - 1263)*/
    Radx::ui32 tau_sp; /* pulse width of transmitter output in short pulse (1264 - 1267)*/
    Radx::ui32 tau_lp; /* pulse width of transmitter output in long pulse (1258 - 1271)*/
    Radx::ui32 nc_dead_value; /* number of 1/4 km bins of corrupted data at end of sweep
                                 (1272 - 1275)*/
    Radx::ui32 tau_rf_sp; /* rf drive pulse width in short pulse (1276 - 1279) */
    Radx::ui32 tau_rf_lp; /* rf drive pulse width in long pulse mode (1280 - 1283)*/
    Radx::fl32 seg1lim; /* clutter map boundary elevation between segments 1 & 2
                           (1284 - 1287)*/
    Radx::fl32 slatsec; /* site latitude - seconds (1288 - 1291)*/
    Radx::fl32 slonsec; /* site longitude - seconds (1292 - 1295)*/
    Radx::ui32 spare_11; /* spare (1296 - 1299)*/
    Radx::ui32 slatdeg; /* site latitude - degrees (1300 - 1303)*/
    Radx::ui32 slatmin; /* site latitude - minutes (1304 - 1307)*/
    Radx::ui32 slondeg; /* site longitude - degrees (1308 - 1311)*/
    Radx::ui32 slonmin; /* site longitude - minutes (1312 - 1315)*/
    char slatdir[4]; /* site latitude - direction (1316 - 1319)*/
    char slondir[4]; /* site longitude - direction (1320 - 1323)*/
    Radx::ui32 spare_12; /* n/a (1324 - 1327)*/
    Radx::ui32 vcpat11[293]; /* volume coverage pattern number 11 definition (1328 - 2499)*/
    Radx::ui32 vcpat21[293]; /* volume coverage pattern number 21 definition (2500 - 3671)*/
    Radx::ui32 vcpat31[293]; /* volume coverage pattern number 31 definition (3672 - 4843)*/
    Radx::ui32 vcpat32[293]; /* volume coverage pattern number 32 definition (4844 - 6015)*/
    Radx::ui32 vcpat300[293]; /* volume coverage pattern number 300 definition (6016 - 7187)*/
    Radx::ui32 vcpat301[293]; /* volume coverage pattern number 301 definition (7188 - 8359)*/
    Radx::fl32 az_correction_factor; /* azimuth boresight correction factor (8360 - 8363)*/
    Radx::fl32 el_correction_factor; /* elevation boresight correction factor (8364 - 8367)*/
    char site_name[4]; /* site name designation (8368 - 8371)*/
    Radx::ui32 ant_manual_setup_ielmin; /* minimum elevation angle (8372 - 8375)*/
    Radx::ui32 ant_manual_setup_ielmax; /* maximum elevation angle (8376 - 8379)*/
    Radx::ui32 ant_manual_setup_fazvelmax; /* maximum azimuth velocity (8380 - 8383)*/
    Radx::ui32 ant_manual_setup_felvelmax; /* maximum elevation velocity (8384 - 8387)*/
    Radx::ui32 ant_manual_setup_ignd_hgt; /* site ground height (above sea level)
                                             (8388 - 8391)*/
    Radx::ui32 ant_manual_setup_irad_hgt; /* site radar height (above ground) (8392 - 8395)*/
    Radx::ui32 spare_13[75]; /* n/a (8396 - 8695)*/
    Radx::ui32 RVP8NV_iwaveguide_length; /* waveguide length (8696 - 8699)*/
    Radx::ui32 spare_14[11]; /* n/a (8700 - 8743)*/
    Radx::fl32 vel_data_tover; /* velocity unfolding overlay threshold (8744 - 8747)*/
    Radx::fl32 width_data_tover; /* width unfolding overlay threshold (8748 - 8751)*/
    Radx::ui32 spare_15[3]; /* n/a (8752 - 8763)*/
    Radx::fl32 doppler_range_start; /* start range for first doppler radial (8764 - 8767)*/
    Radx::ui32 max_el_index; /* the maximum index for the el_index parameters (8768 - 8771)*/
    Radx::fl32 seg2lim; /* clutter map boundary elevation between segments 2 & 3
                           (8772 - 8775)*/
    Radx::fl32 seg3lim; /* clutter map boundary elevation between segments 3 & 4.
                           (8776 - 8779)*/
    Radx::fl32 seg4lim; /* clutter map boundary elevation between segments 4 & 5.
                           (8780 - 8783)*/
    Radx::ui32 nbr_el_segments; /* number of elevation segments in orda clutter map.
                                   (8784 - 8787)*/
    Radx::fl32 noise_long; /* receiver noise, long pulse (8788 - 8791)*/
    Radx::fl32 ant_noise_temp; /* antenna noise temperature (8792 - 8795)*/
    Radx::fl32 noise_short; /* receiver noise, short pulse (8796 - 8799)*/
    Radx::fl32 noise_tolerance; /* receiver noise tolerance (8800 - 8803)*/
    Radx::fl32 min_dyn_range; /* minimum dynamic range (8804 - 8807)*/

  } adaptation_data_t;


  /* Header of message type 31 - DIGITAL_RADAR_DATA */

  static const int MAX_DATA_BLOCKS = 9;

  typedef struct {

    char radar_icao[4]; /* (0-3) ICAO Radar Identifier */
    Radx::ui32 millisecs_past_midnight; /* (4-7) Collection time for this radial in
                                         * millisecs of day past midnight (GMT). */
    Radx::ui16 julian_date; /* (8-9) Modified julian date from 1/1/70 */
    Radx::ui16 radial_num; /* (10-11) Radial number within elevation scan 1 to 720) */
    Radx::fl32 azimuth; /* (12-15) Azimuth angle
                         * An azimuth of "0 degs" points to true north while "90 degs"
                         * points east. Rotation is always counterclockwise
                         * as viewed from above the radar. */
    Radx::ui08 compression; /* (16) Compression Indicator, 0 = uncompressed, 1 = BZIP2
                             * 2 = zlib */
    Radx::ui08 spare_1; /* (17) n/a */
    Radx::ui16 radial_length; /* (18-19) Uncompressed length of the radial in bytes
                               * including data header block */
    Radx::ui08 azimuth_spacing; /* (20) Azimuthal spacing between adjacent radials 
                                 * 1 = 0.5 deg, 2 = 1.0 deg */
    Radx::ui08 radial_status; /* (21) Radial status where:
                               * 0 = start of new elevation
                               * 1 = intermediate radial
                               * 2 = end of elevation
                               * 3 = beginning of volume scan
                               * 4 = end of volume scan */
    Radx::ui08 elev_num; /* (22) RDA elevation number within volume scan */
    Radx::ui08 sector_num; /* (23) Sector number within cut */
    Radx::fl32 elevation; /* (24-27) Elevation angle
                           * An elevation of '0 degs' is parallel to the pedestal base
                           * while '90 degs' is perpendicular to the
                           * pedestal base. */
    Radx::ui08 spot_blank; /* (28) Spot blanking status for current raidal, elevation
                            * scan and volume scan. 0 = none, 1 = radial
                            * 2 = elevation, 4 = volume */
    Radx::ui08 azimuth_index; /* (29) Azimuth indexing value (set if azimuth angle is
                               * keyed to constant angles). 0 = no indexing
                               * 1-100 means indexing angle of 0.01 to 1.00 deg */
    Radx::ui16 n_data_blocks; /* (30-31) Number of data blocks (N) (4 to 9) */
    
    /* offsets to data blocks relative to
     * start of message, in bytes.
     * Supported message types are
     * "VOL":  message_31_vol_t
     * "ELV":  message_31_elev_t
     * "RAD":  message_31_radial_t
     * "REF":  reflectivity data field
     * "VEL":  radial velocity data field
     * "SW" :  spectrum width data field
     * "ZDR":  zdr data field
     * "PHI":  phidp data field
     * "RHO":  rhohv data field
     */
    
    Radx::ui32 data_block_offset[MAX_DATA_BLOCKS];

  } message_31_hdr_t;

  /* Volume header of message type DIGITAL_RADAR_DATA_31 */

  typedef struct {

    char block_type; /* (0) "R" Indicates data constant type */
    char block_name[3]; /* (0-3) "VOL" Volume data constant block */
    Radx::ui16 block_size; /* (4-5) Size of block - 44 bytes */
    Radx::ui08 ver_major; /* (6) Version number, Major Change */
    Radx::ui08 ver_minor; /* (7) Version number, Minor Change */
    Radx::fl32 lat; /* (8-11) Latitude, degrees */
    Radx::fl32 lon; /* (12-15) Longitude, degrees */
    Radx::si16 height; /* (16-17) Site height above sea level, m */
    Radx::ui16 feedhorn_height; /* (18-19) Height of feedhor above ground, m */
    Radx::fl32 dbz0; /* (20-23) Reflectivity scaling factor without correction
                      * by the ground noise scaling factors given in the 
                      * adaptation data message */
    Radx::fl32 horiz_power; /* (24-27) Transmitter power for horizontal channel */
    Radx::fl32 vert_power; /* (28-31) Transmitter power for vertical channel */
    Radx::fl32 system_zdr; /* (32-35) Calibration of system differential reflectivity */
    Radx::fl32 system_phi; /* (36-39) Initial Differential Phase for the system */
    Radx::ui16 vol_coverage_pattern;/* (40-41) Volume coverage pattern */
    Radx::ui16 spare_1; /* (42-43) N/A */

  } message_31_vol_t;

  /* Elevation header of message type DIGITAL_RADAR_DATA_31 */
  typedef struct {

    char block_type; /* (0) "R" Indicates data constant type */
    char block_name[3]; /* (0-3) "ELV" Elevation data constant block */
    Radx::ui16 block_size; /* (4-5) Size of block - 12 bytes */
    Radx::ui16 atmos; /* (6-7) Atmospheric Attenuation Factor db/km
                       * -0.02 to -0.002 (scale factor ?) */
    Radx::fl32 dbz0; /* (8-11) Scaling constant used by the signal processor
                      * for this elevation to calculate reflectivity */
  } message_31_elev_t;


  /* Radial header of message type DIGITAL_RADAR_DATA_31 */
  typedef struct {

    char block_type; /* (0) "R" Indicates data constant type */
    char block_name[3]; /* (0-3) "RAD" Radial data constant block */
    Radx::ui16 block_size; /* (4-5) Size of block - 20 bytes */
    Radx::ui16 unamb_range_x10; /* (6-7) Unambiguous range (scaled: val/10 = KM) */
    Radx::fl32 horiz_noise; /* (8-11) Noise level horizontal channel */
    Radx::fl32 vert_noise; /* (12-15) Noise level vertical channel */
    Radx::ui16 nyquist_vel; /* (16-17) Nyquist velocity (scaled: val/100 = m/s) */
    Radx::ui16 spare_1; /* (18-19) N/A */

  } message_31_radial_t;

  /* Data Moment block of message type DIGITAL_RADAR_DATA_31 */
  /* All data moments, first through fifth, have same structure */

  typedef struct {

    char block_type; /* (0) "D" Indicates data moment type */
    char block_name[3]; /* (1-3) Name of data moment
                         * "VEL" Velocity
                         * "REF" Reflectivity
                         * "SW" Spectrum Width
                         * "RHO" Correlation Coefficient
                         * "PHI" Differential Phase
                         * "ZDR" Differential Reflectivity */
    Radx::fl32 reserved_1; /* (4-7) Reserved. Set to 0 */
    Radx::ui16 num_gates; /* (8-9) Number of data moment gates for
                           * current radial */
    Radx::si16 gate1; /* (10-11) Range to center of first range gate (m) */
    Radx::ui16 gate_width; /* (12-13) Size of data moment sample interval (m) */
    Radx::ui16 tover; /* (14-15) Threshold parameter which specifies the
                       * minimum difference in echo power between two 
                       * resolution gates for them not to labeled "overlayed" */
    Radx::si16 snr_threshold; /* (16-17) SNR threshold for valid data scale? */
    Radx::ui08 control_flags; /* (18) 0 = none, 1 = recombined azimuthal radials
                               * 2 = recombined range gates, 3 = recombined radials and range
                               * gates to legacy resolution */
    Radx::ui08 data_size; /* (19) Number of bits used for storing data moment gate data */
    Radx::fl32 scale; /* (20-23) Scale value used to convert data moments from integer
                       * to floating point data */
    Radx::fl32 offset; /* (24-27) Offset value used to convert data from integer to
                        * floating point data */
    /* Variable length moment data follows */
    /* num_gates * data_size */

  } message_31_field_t;

  ///////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////
  /// Constructor

  NexradData();

  /// Destructor

  ~NexradData();

  /// Printing
  /// Print each struct type

  static void print(const id_rec_t &val, ostream &out);
  static void print(const vol_number_t &val, ostream &out);
  static void print(const vol_title_t &val, ostream &out);
  static void print(const ctm_info_t &val, ostream &out);
  static void print(const msg_hdr_t &val, ostream &out);
  static void print(const message_1_t &val, ostream &out);
  static void print(const geom_hdr_t &val, ostream &out);
  static void print(const clutter_hdr_t &val, ostream &out);
  static void print(const VCP_hdr_t &val, ostream &out);
  static void print(const ppi_hdr_t &val, ostream &out);
  static void print(const adaptation_data_t &val, ostream &out);
  static void print(const message_31_hdr_t &val, ostream &out);
  static void print(const message_31_vol_t &val, ostream &out);
  static void print(const message_31_elev_t &val, ostream &out);
  static void print(const message_31_radial_t &val, ostream &out);
  static void print(const message_31_field_t &val, ostream &out);
  static void printTime(int julianDate, int msecsInDay, ostream &out);

  /// byte swapping
  /// Swap byte order as appropriate

  static void swap(vol_number_t &val);
  static void swap(vol_title_t &val);
  static void swap(ctm_info_t &val);
  static void swap(msg_hdr_t &val);
  static void swap(message_1_t &val);
  static void swap(geom_hdr_t &val);
  static void swap(clutter_hdr_t &val);
  static void swap(VCP_hdr_t &val);
  static void swap(ppi_hdr_t &val);
  static void swap(adaptation_data_t &val);
  static void swap(message_31_hdr_t &val);
  static void swap(message_31_vol_t &val);
  static void swap(message_31_elev_t &val);
  static void swap(message_31_radial_t &val);
  static void swap(message_31_field_t &val);

  // return string for message type
  
  static string msgType2Str(int msgType);

  // Returns true if this is a valid message type,
  // false otherwise
  
  static bool msgTypeIsValid(int msgType);

protected:
private:
  
}; /// NexradData

# endif


