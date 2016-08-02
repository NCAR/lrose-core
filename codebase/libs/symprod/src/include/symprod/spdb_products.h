/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
#ifdef __cplusplus
 extern "C" {
#endif


/******************************************************************
 * spdb_products.h: Defines for product ids and labels used for
 *                  SPDB products.
 *
 ******************************************************************/

#ifndef spdb_products_h
#define spdb_products_h


/*********************************************************************
 * ASCII products - stored as a simple ASCII string
 */

#define SPDB_ASCII_ID               50
#define SPDB_ASCII_LABEL            "ASCII generic data"


/*********************************************************************
 * Generic point data - use methods in rapformats/GenPt.hh.
 */

#define SPDB_GENERIC_POINT_ID       60
#define SPDB_GENERIC_POINT_LABEL    "Generic point data"


/*********************************************************************
 * Products used by the Wyndemere project.
 */

#define SPDB_FLT_PATH_ID               69
#define SPDB_FLT_PATH_LABEL            "Flight Path Data"


/*********************************************************************
 * Generic polyline data - use methods in rapformats/GenPoly.hh.
 */

#define SPDB_GENERIC_POLYLINE_ID       80
#define SPDB_GENERIC_POLYLINE_LABEL    "Generic polyline data"


/*********************************************************************
 * Products used by the Ground De-icing project. (WSDDM)
 */

#define SPDB_TREC_PT_FORECAST_ID       100
#define SPDB_TREC_PT_FORECAST_LABEL    "CTREC Reflectivity Forecast Data"

#define SPDB_STATION_REPORT_ID         101
#define SPDB_STATION_REPORT_LABEL      "Mesonet Station Report Data"

#define SPDB_IRSA_FORECAST_ID          102
#define SPDB_IRSA_FORECAST_LABEL       "Snow Rate Forecast Data"

#define SPDB_STATION_REPORT_ARRAY_ID    103
#define SPDB_STATION_REPORT_ARRAY_LABEL  "Mesonet Station Report Array Data"

/*********************************************************************
 * Products used initially by the TAIWAN project.
 */

#define SPDB_RAW_METAR_ID     110
#define SPDB_RAW_METAR_LABEL  "Raw METAR Text"

/*********************************************************************
 * Products used by the NCWF project.
 */

/*
 * Lightning data is stored in the SPDB database as an array of lightning
 * strikes in the LTG_strike_t format described in <rapformats/ltg.h>.
 * Each array contains all of the strikes which occurred at the same time,
 * so there may be an array of strikes for each time in the day.  The
 * number of strikes in the array is calculated as
 *              chunk_len/sizeof(LTG_strike_t).
 * Old databases stored each strike individually, so this should be handled
 * also by any code using this data.
 *
 * Note the old SPDB_KAV_LTG defines.  These were used when all of our
 * lightning data came from Kavouras.  These should be phased out in the
 * future.
 */

#define SPDB_KAV_LTG_ID                200
#define SPDB_KAV_LTG_LABEL             "Kavouras Lightning Data"
#define SPDB_LTG_ID                    200
#define SPDB_LTG_LABEL                 "Lightning Data"

/*
 * Taiwan lightning data is stored in the SPDB database as an array
 * of lightning strikes in the TWNLTG_strike_spdb_t format described
 * in <rapformats/twnltg.h>.  Each array contains all of the strikes
 * which occurred at the same time, so there may be an array of strikes
 * for every second in the day.  The number of strikes in the array is
 * calculated as chunk_len/sizeof(TWNLTG_strike_spdb_t).  Old databases
 * stored each strike individually, so this should be handled also by
 * any code using this data.
 */

#define SPDB_TWN_LTG_ID                200
#define SPDB_TWN_LTG_LABEL             "Taiwan Lightning Data"

/*
 * SIGMET data is stored in the SPDB database in the SIGMET_spdb_t
 * structure described in <rapformats/fos.h>.  There is a separate entry
 * for each structure in the database.
 */

#define SPDB_SIGMET_ID                 201
#define SPDB_SIGMET_LABEL              "NWS SIGMET Data"



/*********************************************************************
 * Nowcaster products
 */

/*
 * Boundary data is stored in the SPDB database in the BDRY_product_t
 * structure defined in <didss/bdry.h>.  Each boundary is stored separately
 * in the database.
 */

#define SPDB_BDRY_ID                   300
#define SPDB_BDRY_LABEL                "Boundary Data"

/*
 * Sounding data is stored in the SPDB database in the 
 * SOUNDING_product_t structure defined in <rapformats/Sounding.hh>.
 * Each sounding is stored separately in the database.
 */

#define SPDB_SNDG_ID                   301
#define SPDB_SNDG_LABEL                "Sounding Data"


/*********************************************************************
 * Hydrology products
 */

/*
 * Meta data for the alert network.  This data describes the gauges in
 * the alert network.  It is updated infrequently, but is stored in an
 * SPDB database so that the information will be correct when looking
 * at archive data.
 *
 * The class for reading and writing the alert net meta data in SPDB
 * is the AlertMeta class in the hydro library (<hydro/AlertMeta.hh>).
 */

#define SPDB_ALERT_META_ID             400
#define SPDB_ALERT_META_LABEL          "Alert Net Meta Data"

/* Gauge data from the Qualimetrics stations used in the UAE.
 *
 * The class for reading and writing this data is the HydroStation class
 * in the rapformats library (<rapformats/HydroStation.hh>).
 */

#define SPDB_HYDRO_STATION_ID          500
#define SPDB_HYDRO_STATION_LABEL       "Hydrology Station Data"


/*********************************************************************
 * RAP products
 */

/*
 * Symbolic products are generally stored in SPDB databases in their
 * native formats and then transformed by the server into the SYMPROD
 * format when processing requests.  This is done since the native format
 * is generally smaller than the SYMPROD format.  The SYMPROD format is
 * described in <symprod/symprod.h>.  Each symbolic product as described
 * in this file is served out as a separate chunk.
 */

#define SPDB_SYMPROD_ID                900
#define SPDB_SYMPROD_LABEL             "Symbolic Product Data"

/*
 * Weather hazard information.  This information is controlled by
 * classes defined in the WxHazards module of the spdbFormats library.
 * This information is being used by the OCND project to define
 * weather hazards over the Pacific, but can be extended by any
 * other project that needs similar generic weather hazard information.
 */

#define SPDB_WX_HAZARDS_ID            10000
#define SPDB_WX_HAZARDS_LABEL         "Weather Hazards"

/*
 * Flight route information.  This information is controlled by
 * classes defined in the FltRoute module of the spdbFormats library.
 */

#define SPDB_FLT_ROUTE_ID             10001
#define SPDB_FLT_ROUTE_LABEL          "Flight Route"

/*
 * Aircraft position report information.  This information is controlled by
 * classes defined in the FltRoute module of the spdbFormats library.
 */

#define SPDB_POSN_RPT_ID              10002
#define SPDB_POSN_RPT_LABEL           "Aircraft Position Report"

/*
 * Aircraft tracks are stored in SPDB in the ac_posn_t structure,
 * which is defined in <rapformats/ac_posn.h>.
 */

#define SPDB_AC_POSN_ID                20001
#define SPDB_AC_POSN_LABEL             "AC_POSN"

/*
 * Aircraft tracks with data are stored in SPDB in the ac_data_t structure,
 * which is defined in <rapformats/ac_data.h>.
 */

#define SPDB_AC_DATA_ID                20002
#define SPDB_AC_DATA_LABEL             "Aircraft Position with Data"

/*
 * Aircraft tracks are stored in SPDB in the ac_posn_wmod_t structure,
 * which is defined in <rapformats/ac_posn.h>.
 */

#define SPDB_AC_POSN_WMOD_ID           20003
#define SPDB_AC_POSN_WMOD_LABEL        "AC_POSN_WMOD"

/*
 * PIREPS, which are stored in SPDB in the pirep_t structure,
 * which is defined in <rapformats/pirep.h>.
 */

#define SPDB_PIREP_ID                20010
#define SPDB_PIREP_LABEL             "PIREP"

/*
 * ACARS, which are stored in SPDB in the mdcrs_t structure,
 * which is defined in <rapformats/acars.h>.
 */

#define SPDB_ACARS_ID                20011
#define SPDB_ACARS_LABEL             "ACARS"

/*
 * TITAN storms are stored in SPDB using the tstorm_spdb structures,
 * which are defined in <titan/tstorm_spdb.h>.
 */

#define SPDB_TSTORMS_LABEL         "TITAN_STORMS"
#define SPDB_TSTORMS_ID            20020
#define SPDB_TSTORMS_PROD_TYPE     101

/**********************************************************************
 * Microburst Automatic Detection products
 */

/*
 * Shapes from the MAD family of algorithms are stored in the data base
 * in the SPDB database in the rshape_polygon_t structure defined in
 * <rsbutil/rshape.h>.  All shapes of a given type at a given time
 * are stored together in the database as one chunk.  Each such chunk
 * has 1 or more shapes with the same data_type field.  Data type values
 * are specified by the SPDB_MAD_<>_DATA_TYPE values below.
 */
#define SPDB_MAD_ID                        20100
#define SPDB_MAD_LABEL                     "MAD Family"
#define SPDB_MAD_MICROBURST_DATA_TYPE      101
#define SPDB_MAD_CONVERGENCE_DATA_TYPE     102
#define SPDB_MAD_TURBULENCE_DATA_TYPE      103

/*******************************************
 * Trec gauge data - struct trec_gauge_t
 * which is defined in <rapformats/trec_gauge.h>
 */

#define SPDB_TREC_GAUGE_LABEL         "TREC_GAUGE"
#define SPDB_TREC_GAUGE_ID            20200

/*******************************************
 * ZR params data - struct zr_params_t
 * which is defined in <rapformats/zr.h>
 */

#define SPDB_ZR_PARAMS_LABEL         "ZR_PARAMS"
#define SPDB_ZR_PARAMS_ID            20300

/*******************************************
 * ZRPF point forecast data - structs
 * zrpf_hdr_t, zr_params_t and zrpf_precip_t,
 * which are defined in:
 *    <rapformats/zr.h>
 *    <rapformats/zrpf.h>
 */

#define SPDB_ZRPF_LABEL         "ZR_POINT_FORECAST"
#define SPDB_ZRPF_ID            20400


/*******************************************
 * VerifyGridRegion data
 *   Structs VerGridRegionHdr_t and
 *           VerGridRegionData_t
 * which is defined in <rapformats/VerGridRegion.hh>
 */

#define SPDB_VERGRID_REGION_LABEL "VERGRID_REGION"
#define SPDB_VERGRID_REGION_ID     20500


#endif

#ifdef __cplusplus
}
#endif
