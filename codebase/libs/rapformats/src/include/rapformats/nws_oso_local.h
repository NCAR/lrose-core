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
/*********************************************************************/         
/*                                                                   */         
/*  Title: local h                                                   */         
/*  Updated: 18 September 1995                                       */         
/*  Organization: W/OSO242 - Graphics and Display Section            */         
/*  Language: C/370                                                  */         
/*                                                                   */         
/*  Abstract:                                                        */         
/*  This header file provides all function definitions necessary for */         
/*  the OSO242 C function library.                                   */         
/*                                                                   */         
/*********************************************************************/         
                                                                                
#ifndef locallib_defined                                                        
#define locallib_defined                                                        
                                                                                
#ifdef __cplusplus
 extern "C" {
#endif

                                                                                
/*****************/                                                             
/* Include Files */                                                             
/*****************/                                                             

#include <math.h>                                                               
#include <time.h>                                                               
#include <stdio.h>                                                              
#include <string.h>                                                              
                                                                                
#define MAXINT 2147483647  /* maximum integer */

/*********************************************************************/         
/*                                                                   */         
/* General typedefs                                                  */         
/*                                                                   */         
/*********************************************************************/         
                                                                                
/* typedef unsigned char byte; */                                                    
                                                                                
typedef unsigned char ubool;                                                
                                                                                
typedef unsigned short int Devaddr;                                             
                                                                                
typedef struct diskaddr {                                                       
   int cylinder;                                                                
   int track;                                                                   
   int record;                                                                  
} Diskaddr;                                                                     
                                                                                
                                                                                
typedef struct record_id {                                                      
                                                                                
   char id[8];                                                                  
   time_t write_timestamp;                                                      
                                                                                
} Record_ID;                                                                    
                                                                                
                                                                                
typedef struct location {                                                       
                                                                                
   union {                                                                      
      unsigned bsn;                                                             
      char cs[9];                                                               
      unsigned short msn;                                                       
   } loc;                                                                       
                                                                                
   unsigned location_is_bsn:1,                                                  
            location_is_cs:1,                                                   
            location_is_msn:1;                                                  
                                                                                
} Location;                                                                     
                                                                                
                                                                                
                                                                                
/*********************************************************************/         
/*********************************************************************/         
/*                                                                   */         
/*                                                                   */         
/* Functions specific defines, typedefs, and structures              */         
/*                                                                   */         
/*                                                                   */         
/*********************************************************************/         
/*********************************************************************/         
                                                                                
                                                                                
                                                                                
/*********************************************************************/         
/*                                                                   */         
/* Function prototype and structure(s) used in -                     */         
/*                                                                   */         
/* bldstree - Build station information tree                         */         
/* delstree - Delete station information tree                        */         
/* getstinf - Get station information from tree                      */         
/*                                                                   */         
/*********************************************************************/         
                                                                                
typedef struct stn_info_node {                                                  
     int key;                                                                   
     int block;                                                                 
     int station;                                                               
     int latitude;                                                              
     int longitude;                                                             
     int elev;                                                                  
     struct stn_info_node * right;                                              
     struct stn_info_node * left;                                               
} Stninfo;                                                                      
                                                                                
struct stn_info_node *bldstree(void);                                           
void delstree(struct stn_info_node *);                                          
struct stn_info_node *getstinf(struct stn_info_node *,                          
                               int,                                             
                               int);                                            
                                                                                
                                                                                
                                                                                
/*********************************************************************/         
/*                                                                   */         
/* Function prototype and structure(s) used in -                     */         
/*                                                                   */         
/* capqread - Read bulletins from CAPQ chain                         */         
/*                                                                   */         
/*********************************************************************/         
                                                                                
typedef struct CAPQ_data {                                                      
   char * bulletin;                                                             
   int bulletin_length;                                                         
   char * WMO_heading;                                                          
   char * AFOS_pil;                                                             
   char * current_CAPQ_end_address;                                             
   int start_offset;                                                            
   int record_count;                                                            
   int end_offset;                                                              
   char * bulletin_address;                                                     
   int input_line;                                                              
   int receive_line;                                                            
   int receive_hour;                                                            
   int receive_minute;                                                          
   int CAPQ_day;                                                                
   int CAPQ_hour;                                                               
   int CAPQ_minute;                                                             
   int rc;                                                                      
   char flag1;                                                                  
   char flag2;                                                                  
} CAPQdata;                                                                     
                                                                                
struct CAPQ_data * capqread (char *, ...);                                      
                                                                                
                                                                                
                                                                                
/*********************************************************************/         
/*                                                                   */         
/* Function prototype and structure(s) used in -                     */         
/*                                                                   */         
/* mdadread - Read bulletins from MDAD chain                         */         
/*                                                                   */         
/*********************************************************************/         
                                                                                
typedef struct MDAD_data {                                                      
   char * bulletin;                                                             
   int bulletin_length;                                                         
   char * WMO_heading;                                                          
   char * AFOS_pil;                                                             
   char * current_MDAD_end_address;                                             
   int start_offset;                                                            
   int record_count;                                                            
   int end_offset;                                                              
   char * bulletin_address;                                                     
   int input_line;                                                              
   int receive_line;                                                            
   int receive_hour;                                                            
   int receive_minute;                                                          
   int MDAD_year;                                                               
   int MDAD_month;                                                              
   int MDAD_day;                                                                
   int MDAD_hour;                                                               
   int MDAD_minute;                                                             
   int rc;                                                                      
   int part_number;                                                             
   int number_of_parts;                                                         
   char MDAD_flag;                                                              
   char flag1;                                                                  
   char flag2;                                                                  
   char flag3;                                                                  
   char MDAD_flag2;                                                             
} MDADdata;                                                                     
                                                                                
MDADdata * mdadread (char *, ...);                                              
                                                                                
MDADdata * mdadscan (char *, ...);                                              
                                                                                
void mdadinpt ( MDADdata *, char, ubool );                                       
                                                                                
                                                                                
#define MDAD_HISTORY_LIMIT 39                                                   
                                                                                
typedef                                                                         
struct MDAD_history_entry                                                       
{                                                                               
   unsigned short hour;                                                         
   unsigned short minute;                                                       
   Diskaddr MDAD_chain_addr;                                                    
}                                                                               
MDAD_History_Entry;                                                             
                                                                                
typedef                                                                         
struct MDAD_history                                                             
{                                                                               
   MDAD_History_Entry history_array[MDAD_HISTORY_LIMIT];                        
}                                                                               
MDAD_History;                                                                   
                                                                                
MDAD_History *mdadhist ( void );                                                
                                                                                
                                                                                
/*********************************************************************/         
/*                                                                   */         
/* Function prototype and structure(s) used in -                     */         
/*                                                                   */         
/* gethdgi - Get bulletin heading information                        */         
/*                                                                   */         
/*********************************************************************/         
                                                                                
typedef struct bltn_heading_info {                                              
    int bltn_day;                                                               
    int bltn_hour;                                                              
    int bltn_min;                                                               
    int rtd_present;                                                            
    int cor_present;                                                            
    int amd_present;                                                            
    char * first_report;                                                        
    char TTAAii[7];                                                         
    char CCCC[5];                                                           
    char amd_seq;                                                               
    char cor_seq;                                                               
    char rtd_seq;                                                               
} Abbrevhdg;                                                                    
                                                                                
Abbrevhdg *gethdgi(char * );                                                    
                                                                                
                                                                                
                                                                                
/*********************************************************************/         
/*                                                                   */         
/* Function prototype and structure(s) used in -                     */         
/*                                                                   */         
/* getime  - Get current system time                                 */         
/* suspend - Delay execution until specified minute boundary         */         
/*                                                                   */         
/*********************************************************************/         
                                                                                
                                                                                
typedef struct tm_struct{                                                       
   int hour;                                                                    
   int min;                                                                     
} Stime;                                                                        
                                                                                
Stime *gettime(void);                                                           
int suspend(Stime *, int);                                                      
int timediff(Stime *, Stime *);                                                 
#define timecmp timediff                                                        
                                                                                
                                                                                
                                                                                
/*********************************************************************/         
/*                                                                   */         
/* Function prototype and structure(s) used in -                     */         
/*                                                                   */         
/* rdtaend - Specify rdtaread Ending Address                         */         
/* rdtaread - Read From RGTR Data Tank                               */         
/* rdtastrt - Specify rdtaread Starting Address                      */         
/* rdtatend - Specify rdtaread End Time                              */         
/* rdtatnke - Specify rdtaread Ending Address                        */         
/* rdtarstr - Specify rdtaread Start Time                            */         
/*                                                                   */         
/*********************************************************************/         
                                                                                
typedef struct rgtrdata {                                                       
   Diskaddr forward_chain;                                                      
   Diskaddr bulletin_addr;                                                      
   int receive_line;                                                            
   int receive_day;                                                             
   Stime receive_time;                                                          
   Stime RGTR_time;                                                             
   int length;                                                                  
   char *bulletin;                                                              
   char datatype;                                                               
} RGTRdata;                                                                     
                                                                                
int rdtaend(char, Diskaddr *);                                                  
int rdtaread(RGTRdata *);                                                       
int rdtastrt(char, Diskaddr *);                                                 
int rdtatend (char, Stime *);                                                   
int rdtatnke(char);                                                             
int rdtatstr(char, Stime *);                                                    
void rdtainit(void);                                                            
                                                                                
                                                                                
                                                                                
/*********************************************************************/         
/*                                                                   */         
/*  Typedefs and function prototypes for bulletin and report parsing */         
/*  functions.                                                       */         
/*                                                                   */         
/*********************************************************************/         
                                                                                
                                                                                
                                                                                
typedef struct rptNode {                                                        
   char *rptPtr;                                                                
   int rptLength;                                                               
   struct rptNode* next;                                                        
} RptNode;                                                                      
                                                                                
                                                                                
typedef struct synpBltn {                                                       
   Abbrevhdg heading;                                                           
   short int day;                                                               
   short int hour;                                                              
   int reportCount;                                                             
   RptNode *rptList;                                                            
   ubool valid;                                                                  
} SynpBltn;                                                                     
                                                                                
                                                                                
typedef struct shipBltn {                                                       
   Abbrevhdg heading;                                                           
   int reportCount;                                                             
   RptNode *rptList;                                                            
   ubool valid;                                                                  
} ShipBltn;                                                                     
                                                                                
                                                                                
typedef struct tepiBltn {                                                       
   Abbrevhdg heading;                                                           
   int reportCount;                                                             
   RptNode *rptList;                                                            
   ubool valid;                                                                  
} TePiBltn;                                                                     
                                                                                
                                                                                
typedef struct drftBltn {                                                       
   Abbrevhdg heading;                                                           
   int reportCount;                                                             
   RptNode *rptList;                                                            
   ubool valid;                                                                  
} DrftBltn;                                                                     
                                                                                
                                                                                
typedef struct airpBltn {                                                       
   Abbrevhdg heading;                                                           
   int reportCount;                                                             
   RptNode *rptList;                                                            
   ubool valid;                                                                  
} AirpBltn;                                                                     
                                                                                
                                                                                
typedef struct amdrBltn {                                                       
   Abbrevhdg heading;                                                           
   short int day;                                                               
   short int hour;                                                              
   int reportCount;                                                             
   RptNode *rptList;                                                            
   ubool valid;                                                                  
} AmdrBltn;                                                                     
                                                                                
                                                                                
typedef struct bthyBltn {                                                       
   Abbrevhdg heading;                                                           
   int reportCount;                                                             
   RptNode *rptList;                                                            
   ubool valid;                                                                  
} BthyBltn;                                                                     
                                                                                
                                                                                
typedef struct tescBltn {                                                       
   Abbrevhdg heading;                                                           
   int reportCount;                                                             
   RptNode *rptList;                                                            
   ubool valid;                                                                  
} TescBltn;                                                                     
                                                                                
                                                                                
typedef struct tracBltn {                                                       
   Abbrevhdg heading;                                                           
   int reportCount;                                                             
   RptNode *rptList;                                                            
   ubool valid;                                                                  
} TracBltn;                                                                     
                                                                                
                                                                                
typedef struct climBltn {                                                       
   Abbrevhdg heading;                                                           
   int reportCount;                                                             
   int month;                                                                   
   int year;                                                                    
   RptNode *rptList;                                                            
   ubool valid;                                                                  
} ClimBltn;                                                                     
                                                                                
                                                                                
typedef struct clmtBltn {                                                       
   Abbrevhdg heading;                                                           
   int reportCount;                                                             
   int month;                                                                   
   int year;                                                                    
   RptNode *rptList;                                                            
   ubool valid;                                                                  
} ClmtBltn;                                                                     
                                                                                
                                                                                
typedef struct metBltn {                                                        
   Abbrevhdg heading;                                                           
   int reportCount;                                                             
   RptNode *rptList;                                                            
   ubool valid;                                                                  
   short int day;              /* -1 indicates missing/invalid */               
   short int hour;             /* -1 indicates missing/invalid */               
   short int min;              /* -1 indicates missing/invalid */               
} MetBltn;                                                                      
                                                                                
                                                                                
typedef struct saoBltn {                                                        
   Abbrevhdg heading;                                                           
   int reportCount;                                                             
   RptNode *rptList;                                                            
   ubool valid;                                                                  
} SAOBltn;                                                                      
                                                                                
                                                                                
typedef struct prBltn {                                                         
   Abbrevhdg heading;                                                           
   int reportCount;                                                             
   RptNode *rptList;                                                            
   ubool valid;                                                                  
} PRBltn;                                                                       
                                                                                
                                                                                
typedef struct tafBltn {                                                        
   Abbrevhdg heading;                                                           
   int reportCount;                                                             
   RptNode *rptList;                                                            
   ubool valid;                                                                  
} TafBltn;                                                                      
                                                                                
                                                                                
typedef struct metrRptP {                                                       
   char locind[4];                                                              
   int groupCount;                                                              
   short int hour;            /* -1 indicates missing or invalid */             
   short int min;             /* -1 indicates missing or invalid */             
   ubool valid;                                                                  
} MetrRptP;                                                                     
                                                                                
                                                                                
typedef struct saoRptP {                                                        
   char locind[4];                                                              
   int groupCount;                                                              
   short int hour;            /* -1 indicates missing or invalid */             
   short int min;             /* -1 indicates missing or invalid */             
   ubool valid;                                                                  
} SAORptP;                                                                      
                                                                                
                                                                                
typedef struct prRptP {                                                         
   char locind[4];                                                              
   int groupCount;                                                              
   short int hour;            /* -1 indicates missing or invalid */             
   short int min;             /* -1 indicates missing or invalid */             
   ubool valid;                                                                  
} PRRptP;                                                                       
                                                                                
                                                                                
typedef struct tafRptP {                                                        
   char locind[4];                                                              
   int groupCount;                                                              
   short int YY;                                                                
   short int GG;                                                                
   short int validPeriod;                                                       
   ubool ammendment;                                                             
   ubool valid;                                                                  
} TafRptP;                                                                      
                                                                                
                                                                                
typedef struct synpRptP {                                                       
   short int II;                                                                
   short int iii;                                                               
   int groupCount;                                                              
   ubool valid;                                                                  
} SynpRptP;                                                                     
                                                                                
                                                                                
typedef struct climRptP {                                                       
   short int II;                                                                
   short int iii;                                                               
   int groupCount;                                                              
   ubool valid;                                                                  
} ClimRptP;                                                                     
                                                                                
                                                                                
typedef struct clmtRptP {                                                       
   short int II;                                                                
   short int iii;                                                               
   int groupCount;                                                              
   ubool valid;                                                                  
} ClmtRptP;                                                                     
                                                                                
                                                                                
typedef struct tepiRptP {                                                       
   short int II;                                                                
   short int iii;                                                               
   short int YY;                                                                
   short int GG;                                                                
   short int quad;                                                              
   short int ulatitude;                                                         
   short int ulongitude;                                                        
   int msquare;                                                                 
   int latitude;                                                                
   int longitude;                                                               
   int groupCount;                                                              
   char callsign[15];                                                           
   char type;                                                                   
   char part;                                                                   
   ubool valid;                                                                  
} TePiRptP;                                                                     
                                                                                
                                                                                
SynpBltn *pbsynp(char *);                                                       
ShipBltn *pbship(char *);                                                       
TePiBltn *pbtepi(char *);                                                       
DrftBltn *pbdrft(char *);                                                       
AirpBltn *pbairp(char *);                                                       
AmdrBltn *pbamdr(char *);                                                       
BthyBltn *pbbthy(char *);                                                       
TescBltn *pbtesc(char *);                                                       
TracBltn *pbtrac(char *);                                                       
ClimBltn *pbclim(char *);                                                       
ClmtBltn *pbclmt(char *);                                                       
MetBltn  *pbmetr(char *);                                                       
MetBltn  *pbspec(char *);                                                       
TafBltn  *pbtaf(char *);                                                        
SAOBltn  *pbsao(char *);                                                        
PRBltn   *pbpirep(char *);                                                      
                                                                                
SynpRptP *prpsynp(char *, int);                                                 
TePiRptP *prptepi(char *, int);                                                 
ClimRptP *prpclim(char *, int);                                                 
ClmtRptP *prpclmt(char *, int);                                                 
MetrRptP *prpmetr(char *, int);                                                 
TafRptP  *prptaf(char *, int);                                                  
SAORptP  *prpsao(char *, int);                                                  
PRRptP   *prppirep(char *, int);                                                
                                                                                
                                                                                
                                                                                
                                                                                
/*********************************************************************/         
/*                                                                   */         
/*  Structures and Function Prototypes for RRN physical I/O          */         
/*                                                                   */         
/*********************************************************************/         
                                                                                
                                                                                
typedef struct RRN_device {                                                     
                                                                                
   char name[44],                                                               
        ownerid[8];                                                             
                                                                                
   unsigned short dev_addr,                                                     
                  base_cylinder,                                                
                  base_track,                                                   
                  base_record,                                                  
                  max_cylinder,                                                 
                  max_track,                                                    
                  max_record,                                                   
                  records_per_track,                                            
                  tracks_per_cylinder,                                          
                  record_length;                                                
                                                                                
} RRN_Device;                                                                   
                                                                                
                                                                                
ubool readrrn(char *device_name,                                                 
             unsigned int rrn,                                                  
             void *input_buffer,                                                
             unsigned int read_count);                                          
                                                                                
ubool writerrn(char *device_name,                                                
              unsigned int rrn,                                                 
              void *output_buffer,                                              
              unsigned int write_count);                                        
                                                                                
RRN_Device *devinfo(char *device_name);                                         
                                                                                
ubool valid_dn(char *device_name);                                               
                                                                                
                                                                                
                                                                                
/*********************************************************************/         
/*                                                                   */         
/*  Function prototype for string value test functions.              */         
/*                                                                   */         
/*********************************************************************/         
                                                                               #include <string.h> 
                                                                                
int sisalnum(char *);                                                           
int sisalpha(char *);                                                           
int siscntrl(char *);                                                           
int sisdigit(char *);                                                           
int sisgraph(char *);                                                           
int sislower(char *);                                                           
int sisprint(char *);                                                           
int sispunct(char *);                                                           
int sisspace(char *);                                                           
int sisupper(char *);                                                           
int sisxdigi(char *);                                                           
                                                                                
int nisalnum(char *, int);                                                      
int nisalpha(char *, int);                                                      
int niscntrl(char *, int);                                                      
int nisdigit(char *, int);                                                      
int nisgraph(char *, int);                                                      
int nislower(char *, int);                                                      
int nisprint(char *, int);                                                      
int nispunct(char *, int);                                                      
int nisspace(char *, int);                                                      
int nisupper(char *, int);                                                      
int nisxdigi(char *, int);                                                      
                                                                                
char *nxtalnum(char *);                                                         
char *nxtalpha(char *);                                                         
char *nxtcntrl(char *);                                                         
char *nxtdigit(char *);                                                         
char *nxtgraph(char *);                                                         
char *nxtlower(char *);                                                         
char *nxtprint(char *);                                                         
char *nxtpunct(char *);                                                         
char *nxtspace(char *);                                                         
char *nxtupper(char *);                                                         
char *nxtxdigi(char *);                                                         
                                                                                
char *lstalnum(char *, int);                                                    
char *lstalpha(char *, int);                                                    
char *lstcntrl(char *, int);                                                    
char *lstdigit(char *, int);                                                    
char *lstgraph(char *, int);                                                    
char *lstlower(char *, int);                                                    
char *lstprint(char *, int);                                                    
char *lstpunct(char *, int);                                                    
char *lstspace(char *, int);                                                    
char *lstupper(char *, int);                                                    
char *lstxdigi(char *, int);                                                    
                                                                                
                                                                                
/*********************************************************************/         
/*                                                                   */         
/*  Enumeration type and declaration for code form identification    */         
/*  function                                                         */         
/*                                                                   */         
/*********************************************************************/         
                                                                                
                                                                                
typedef                                                                         
enum codeform {AIREP, AMDAR, ARFOR, ARMET, BATHY, CLIMAT, CLIMAT_SHIP,          
               CLIMAT_TEMP, CLIMAT_TEMP_SHIP, CODAR, DRIFTER, FC,               
               HYFOR, IAC, IAC_FLEET, ICEAN, METAR, PILOT, PILOT_MOBILE,        
               PILOT_SHIP, RECCO, ROCOB, ROCOB_SHIP, ROFOR, SAO, PIREP,         
               SATEM, SATOB, SHIP, SPECI, SYNOP, TAF, AIR_TEMP, TEMP_DROP,          
               TEMP_MOBILE, TEMP_SHIP, TESAC, TRACKOB, WAVEOB,                  
               UNKNOWN_FORM, TEMP_A, TEMP_B, TEMP_C, TEMP_D,                    
               TEMP_DROP_A, TEMP_DROP_B, TEMP_DROP_C, TEMP_DROP_D,              
               TEMP_MOBILE_A, TEMP_MOBILE_B, TEMP_MOBILE_C,                     
               TEMP_MOBILE_D, TEMP_SHIP_A, TEMP_SHIP_B, TEMP_SHIP_C,            
               TEMP_SHIP_D, PILOT_A, PILOT_B, PILOT_C, PILOT_D,                 
               PILOT_MOBILE_A, PILOT_MOBILE_B, PILOT_MOBILE_C,                  
               PILOT_MOBILE_D, PILOT_SHIP_A, PILOT_SHIP_B,                      
               PILOT_SHIP_C, PILOT_SHIP_D }                                     
CodeForm;                                                                       
                                                                                
CodeForm idcode(char *);                                                        
                                                                                
char *codename(CodeForm);                                                       
CodeForm name2cf ( char * );                                                    

/*********************************************************************/         
/*                                                                   */         
/*  Bulletin Generator declarations                                  */         
/*                                                                   */         
/*********************************************************************/         
                                                                                
typedef ubool (*ParseBltnFnPtr) ( char *bltn,                                    
                                 char **rptPtr,                                 
                                 char *bbbTypePtr,                              
                                 char **prefixPtr,                              
                                 short *YYPtr,                                  
                                 short *GGPtr,                                  
                                 char *bltnTypePtr,                             
                                 char **headingPtr );                           
                                                                                
void bltngen ( ParseBltnFnPtr fnPtr,                                            
               char *filename,                                                  
               Devaddr *historyDevice,                                          
               Diskaddr *historyAddr,                                           
               unsigned * bltnInCountPtr,                                       
               unsigned * bltnOutCountPtr,                                      
               unsigned * rptOutCountPtr );                                     
                                                                                
typedef ubool (*ParseBltnFnPtrX) ( MDADdata *mdata,                              
                                  char **rptPtr,                                
                                  char *bbbTypePtr,                             
                                  char **prefixPtr,                             
                                  short *YYPtr,                                 
                                  short *GGPtr,                                 
                                  char *bltnTypePtr,                            
                                  char **headingPtr );                          
                                                                                
void tbltngnx ( ParseBltnFnPtrX fnPtr,                                          
                char *filename,                                                 
                Devaddr *historyDevice,                                         
                Diskaddr *historyAddr,                                          
                unsigned * bltnInCountPtr,                                      
                unsigned * bltnOutCountPtr,                                     
                unsigned * rptOutCountPtr );                                    
                                                                                
                                                                                
                                                                                
/*********************************************************************/         
/*                                                                   */         
/*  Typedefs and function prototypes for retrieving information from */         
/*  switching directory.                                             */         
/*                                                                   */         
/*********************************************************************/         
                                                                                
typedef struct bltn_history{                                                    
  Diskaddr bltn_disk_addr;                                                      
  unsigned short bltn_start_offset;                                             
  unsigned short bltn_record_count;                                             
  unsigned short bltn_end_offset;                                               
  time_t bltn_time_recvd;                                                       
} BltnHistory;                                                                  
                                                                                
typedef struct sw_dir_info_rec {                                                
  char wmo_header[11];                                                          
  char AFOS_pil[10];                                                            
  char multiple_line;                                                           
  short int line_num;                                                           
  short int recvd_line;                                                         
  char flag1;                                                                   
  char flag2;                                                                   
  char flag3;                                                                   
  char Class;                                                                   
  short int domestic_cat_num;                                                   
  char afos_tmp;                                                                
  char ccb[2];                                                                  
  char region_addr;                                                             
  short int output_line_count;                                                  
  unsigned short trans_line[128];                                               
  time_t change_date;                                                           
  char dir_flags;                                                               
  Diskaddr history_file_addr;                                                   
  BltnHistory bltn_history[4];                                                  
} SwDirInfo;                                                                    
                                                                                
SwDirInfo *rtswdir(char *, int);                                                
SwDirInfo *rtpswdir(void);                                                      
SwDirInfo *rtnswdir(void);                                                      
                                                                                
                                                                                
                                                                                
                                                                                
                                                                                
/*********************************************************************/         
/*                                                                   */         
/*  General local functions                                          */         
/*                                                                   */         
/*********************************************************************/         
                                                                                
                                                                                
int itoc(int, char *, int);                                                     
                                                                                
int antoi(char *, int);                                                         
                                                                                
float antof(char *, int);                                                       
                                                                                
void errmsg(char *, ...);                                                       
                                                                                
void logmsg(char *, ...);                                                       
                                                                                
void opermsg(char *, ...);                                                      
                                                                                
int lmsg(const char *, const char *, ...);                                      
int emsg(const char *, const char *, ...);                                      
int omsg(const char *, const char *, ...);                                      
                                                                                
void ASCTOEB(char *, int);                                                      
                                                                                
void EAXLATE(char *, int);                                                      
                                                                                
void PASCTOEB(char *, int);                                                     
                                                                                
char **bldhdarr(char *);                                                        
                                                                                
void dalchdar(char **);                                                         
                                                                                
void *CCAPREAD(char *, int);                                                    
                                                                                
void CCAPWRIT(char *, char *, int);                                             
                                                                                
int PPTOI(char);                                                                
                                                                                
char itopp(int);                                                                
                                                                                
int diffmin(int, int, int, int, int, int);                                      
                                                                                
char incrseq(char);                                                             
                                                                                
void nextdate(int *, int *, int *);                                             
                                                                                
void prevdate(int *, int *, int *);                                             
                                                                                
void rdstaddr(char *, char *);                                                  
                                                                                
int wrenaddr(char *, char *);                                                   
                                                                                
int vfydigit (char *, int);                                                     
                                                                                
int readline(char * , int);                                                     
                                                                                
int prevjul(int, int);                                                          
                                                                                
int nextjul(int, int);                                                          
                                                                                
/*  int fcomppos(fpos_t *, fpos_t *);  */                                              
                                                                                
void lfprint(char *);                                                           
                                                                                
void flfprint(FILE *, char *);                                                  
                                                                                
void slfprint(char *, int, char *);                                             
                                                                                
void flfnprnt(FILE *, char *, int);                                             
                                                                                
void slfnprnt(char *, int, char *, int);                                        
                                                                                
int strhash(char *);                                                            
                                                                                
void reverse(char *);                                                           
                                                                                
ubool itoa(int, char *, int);                                                    
                                                                                
int getsnn(char * , int);                                                       
                                                                                
int fgetsnn(char *, int, FILE *);                                               
                                                                                
int getreply(char *, char *, int);                                              
                                                                                
ubool strfit(char *, char *, size_t);                                            
                                                                                
ubool addrfrm3(char *, Diskaddr *);                                              
                                                                                
ubool addrfrm5(char *, Diskaddr *);                                              
                                                                                
ubool addrto3(Diskaddr *, char *);                                               
                                                                                
ubool addrto5(Diskaddr *, char *);                                               
                                                                                
int addrcmp(Diskaddr *, Diskaddr *);                                            
                                                                                
void incraddr(Diskaddr *, Diskaddr *, Diskaddr *);                              
void decraddr(Diskaddr *, Diskaddr *, Diskaddr *);                              
                                                                                
char *readrec(Diskaddr *, Devaddr *, int, void *);                              
                                                                                
int writerec(Diskaddr*, Devaddr *, int, void *);                                
                                                                                
char prhold(char *, ...);                                                       
                                                                                
void dump(char *, int);                                                         
                                                                                
void fdump(FILE *, char *, int);                                                
                                                                                
void fwdump(FILE *, char *, int);

/*char toascii(char);*/

char *strtoas(char *);                                                          
                                                                                
char *strntoas(char *, int);                                                    
                                                                                
char toebcdic(char);                                                            
                                                                                
char *strtoeb(char *);                                                          
                                                                                
char *strntoeb(char *, int);                                                    
                                                                                
char *lfind(char *, char *, int, int, int(*)(char *, char *));                  
                                                                                
char *lsearch(char *, char *, int *, int, int(*)(char *, char *));              
                                                                                
ubool strcmpw(char *, char *);                                                   
                                                                                
int strccnt(char *, int);                                                       
                                                                                
int strnccnt(char *, int, size_t);                                              
                                                                                
int pprt(FILE *, char *, char *, char *, char *, ...);                          
                                                                                
ubool pprtbrk(FILE *, char *, char *, char *);                                   
                                                                                
ubool pprtend(FILE *, char *);                                                   
                                                                                
ubool pprtinit(int, char, char *, char *, char *);                               
                                                                                
char *monthnam(int, char);                                                      
                                                                                
char *getrec(FILE *, int, char *);                                              
                                                                                
ubool jtog(int, int, int *, int *, int *);                                       
                                                                                
ubool gtoj(int, int, int, int *, int *);                                         
                                                                                
ubool ccap2std(char *, Devaddr *, Diskaddr *);                                   
                                                                                
ubool std2ccap(Devaddr *, Diskaddr *, char *);                                   
                                                                                
char *strupr(char *);                                                           
char *strlwr(char *);                                                           
#if !defined(CYGWIN)
int strcmpi(char *, char *);                                                    
#endif                                                                                
char *rptstrip(char *);                                                         
char *rptfmt(char *);                                                           
                                                                                
int stregion(int);                                                              
int ccregion(char *);                                                           
char *rgnname(int);                                                             
                                                                                
/* void *memrchr(const void *, int, size_t); */   
                                                                                
ubool sysmonms(char *, char *, ...);                                             
ubool sysmoncl(char *);                                                          
                                                                                
short prevndx ( short max, short min, short current );                          
short nextndx ( short max, short min, short current );                          
                                                                                
time_t extrym ( unsigned day, unsigned hour, unsigned minute );                 
time_t extrymd ( unsigned hour, unsigned minute );                              
                                                                                
int cmptimet ( time_t t1, time_t t2 );                                          
                                                                                
int tfprintf ( FILE *, const char *, ... );                                     
                                                                                
ubool purgelog ( char *filename, unsigned short delete_age );                    
                                                                                
time_t odbtime ( void );                                                        
                                                                                
int bltnpcnt ( char *, int );                                                   
void bltnpage ( char *, int, int );                                             
                                                                                
void rot( char *, unsigned int );                                               
void unrot( char *, unsigned int );                                             
                                                                                
int HEXTOI( char *, int );                                                      
                                                                                
char **hdgxref( char * );                                                       
                                                                                
struct tm *zonetime( unsigned short, unsigned short, char );                    
                                                                                
int wordcnt( char * );                                                          
int wordcntn( char *, unsigned int );                                           
                                                                                
char *word( char *, unsigned int );                                             
char *wordn( char *, unsigned int, unsigned int );                              
                                                                                
char *crlfstrp( char * );                                                       
                                                                                
                                                                                
#ifdef __cplusplus
}
#endif

#endif                                                                          
