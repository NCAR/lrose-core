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

/* The following define added by Niles, October 1999. 
It is the ceiling, in feet, to assume if the CLR message
is present. */

#define CLR_CEILING 12000.0 /* ft */
#define SKC_CEILING 12000.0 /* ft */
#define CAVOK_CEILING_FT 12000.0
#define NSC_CEILING_FT 12000.0 
#define CAVOK_VIS_KM 10.0


#include <toolsa/toolsa_macros.h>
#include <toolsa/str.h>
#include <rapformats/metar_decode.h>
#include <string.h>                                                             
#include <stdlib.h>                                                             
#include <ctype.h>                                                             
#include "metar_private.h"

/********************************************************************/
/*                                                                  */
/*  Title:         SaveTokenString                                  */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          14 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:      SaveTokenString tokenizes the input character    */
/*                 string based upon the delimeter set supplied     */
/*                 by the calling routine.  The elements tokenized  */
/*                 from the input character string are saved in an  */
/*                 array of pointers to characters.  The address of */
/*                 this array is the output from this function.     */
/*                                                                  */
/*  Input:         string - a pointer to a character string.        */
/*                                                                  */
/*                 delimiters - a pointer to a string of 1 or more  */
/*                              characters that are used for token- */
/*                              izing the input character string.   */
/*                                                                  */
/*  Output:        token  - the address of a pointer to an array of */
/*                          pointers to character strings.  The     */
/*                          array of pointers are the addresses of  */
/*                          the character strings that are token-   */
/*                          ized from the input character string.   */
/*                                                                  */
/*                                                                  */
/*  Modification History:                                           */
/*     NCAR/RAP - F. Hage  Added error checking - use calloc to     */
/*          assure tokens get null terminated, etc                  */
/*      Note: This routines allocates space - It's up to the        */
/* calling routine to free up this space. Also the token array      */
/* may or may not  have a null pointer indicating the end of        */
/* tokens. The calling routine MUST check no                        */
/* more than MAXTOKENS elements in the returned array               */
/*                                                                  */
/********************************************************************/

static char **SaveTokenString ( char *string , char *delimiters )
{

   /***************************/
   /* DECLARE LOCAL VARIABLES */
   /***************************/

   int NDEX;  /* token index */
   char *ptr; /* Temporary pointer */
   static char *token[ MAXTOKENS ], *TOKEN;


   /*********************************/
   /* BEGIN THE BODY OF THE ROUTINE */
   /*********************************/

   /* Make all token pointers NULL to start */
   memset(token,0,(MAXTOKENS * sizeof(char*)));

   /* Bail out on bad parameters */
   if(string == NULL || delimiters == NULL ) {
     return token;
   } 

   /******************************************/
   /* TOKENIZE THE INPUT CHARACTER STRING    */
   /* AND SAVE THE TOKENS TO THE token ARRAY */
   /******************************************/

   NDEX = 0;
   TOKEN = strtok( string, delimiters);

   if(TOKEN == NULL) { /* Bail out if no tokens found */
     token[NDEX] = NULL;
     return token;
   } 

   /* Allocate space for the first token */
   token[NDEX] = (char *) calloc(1,sizeof(char)*(strlen(TOKEN)+1));
   strcpy( token[ NDEX ], TOKEN );
 
 
   /* Find all tokens or give up if space is exausted */
   while ( token[NDEX] != NULL && NDEX < (MAXTOKENS -1))
   {
      NDEX++;
      /* Grab the next token pointer */
      TOKEN = strtok( NULL, delimiters );
 
      if( TOKEN != NULL )
      {
          /* Allocate space for the next token and copy it */
         token[NDEX] = (char *) calloc(1,sizeof(char)*(strlen(TOKEN)+1));
         strcpy( token[NDEX], TOKEN );
	 /* Replace equal signs with null - FH.  */
	 if((ptr = strchr(token[NDEX],'=')) != NULL ) *ptr = '\0';

      } else {
	 token[NDEX] = NULL;  /* here for clarity */
      }
   }
 
   return token;
 
}
/********************************************************************/
/*                                                                  */
/*  Title:         freeTokens                                       */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          14 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:      freeTokens frees the storage allocated for the   */
/*                 character strings stored in the token array.     */
/*                                                                  */
/*  Input:         token  - the address of a pointer to an array    */
/*                          of string tokens.                       */
/*                                                                  */
/*                                                                  */
/*  Output:        None.                                            */
/*                                                                  */
/*                                                                  */
/*  Modification History:                                           */
/*                 F. Hage - Added check so as not to exceed        */
/*               MAXTOKEN Free()'s - See above routines' comments.  */
/*                                                                  */
/********************************************************************/
 
static void freeTokens( char **token )
{
  int i;
  for (i = 0; i < MAXTOKENS; i++) {
    if (token[i] != NULL) {
      free (token[i]);
      token[i] = NULL;
    }
  } /* i */
}
/********************************************************************/
/*                                                                  */
/*  Title:         InitDcdMETAR                                     */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:  InitDcdMETAR initializes every member of the         */
/*             structure addressed by the pointer Mptr.             */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                                                                  */
/*  Input:         Mptr - ptr to a decoded_METAR structure.         */
/*                                                                  */
/*  Output:        NONE                                             */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
static void InitDcdMETAR( Decoded_METAR *Mptr )
{
 
   /***************************/
   /* DECLARE LOCAL VARIABLES */
   /***************************/
 
 
   int i,
       j;
 
 
 
   /*************************/
   /* START BODY OF ROUTINE */
   /*************************/
 
   memset(Mptr->TornadicType,'\0',15);
   memset(Mptr->TornadicLOC,'\0',10);
   memset(Mptr->TornadicDIR,'\0',4);
   Mptr->BTornadicHour = MAXINT;
   Mptr->BTornadicMinute = MAXINT;
   Mptr->ETornadicHour = MAXINT;
   Mptr->ETornadicMinute = MAXINT;
 
   memset( Mptr->autoIndicator,'\0', 5 );
 
   Mptr->RVRNO = FALSE;
   Mptr->GR = FALSE;
   Mptr->GR_Size = (float) MAXINT;
 
   Mptr->CHINO = FALSE;
   memset(Mptr->CHINO_LOC, '\0', 6);
 
   Mptr->VISNO = FALSE;
   memset(Mptr->VISNO_LOC, '\0', 6);
 
   Mptr->PNO = FALSE;
   Mptr->PWINO = FALSE;
   Mptr->FZRANO  = FALSE;
   Mptr->TSNO   = FALSE;
   Mptr->DollarSign  = FALSE;
   Mptr->hourlyPrecip = (float) MAXINT;
 
   Mptr->ObscurAloftHgt = MAXINT;
   memset(Mptr->ObscurAloft, '\0', 12);
   memset(Mptr->ObscurAloftSkyCond, '\0', 12);
 
   memset(Mptr->VrbSkyBelow, '\0', 4);
   memset(Mptr->VrbSkyAbove, '\0', 4);
   Mptr->VrbSkyLayerHgt = MAXINT;
 
   Mptr->SectorVsby = (float) MAXINT;
   memset( Mptr->SectorVsby_Dir, '\0', 3);
 
   memset(Mptr->codeName, '\0', 6);
   memset(Mptr->stnid, '\0', 5);
   Mptr->ob_hour   = MAXINT;
   Mptr->ob_minute = MAXINT;
   Mptr->ob_date   = MAXINT;
 
   memset(Mptr->synoptic_cloud_type, '\0', 6);
 
   Mptr->CloudLow    = '\0';
   Mptr->CloudMedium = '\0';
   Mptr->CloudHigh   = '\0';
 
   memset(Mptr->snow_depth_group, '\0', 6);
   Mptr->snow_depth = MAXINT;
 
   Mptr->Temp_2_tenths    = (float) MAXINT;
   Mptr->DP_Temp_2_tenths = (float) MAXINT;
 
   Mptr->OCNL_LTG = FALSE;
   Mptr->FRQ_LTG = FALSE;
   Mptr->CNS_LTG = FALSE;
   Mptr->CG_LTG = FALSE;
   Mptr->IC_LTG = FALSE;
   Mptr->CC_LTG = FALSE;
   Mptr->CA_LTG = FALSE;
   Mptr->AP_LTG = FALSE;
   Mptr->OVHD_LTG = FALSE;
   Mptr->DSNT_LTG = FALSE;
   Mptr->VcyStn_LTG = FALSE;
   Mptr->LightningVCTS = FALSE;
   Mptr->LightningTS = FALSE;
 
   memset( Mptr->LTG_DIR, '\0', 3);
 
 
   for( i = 0; i < 3; i++)
   {
      memset(Mptr->ReWx[i].Recent_weather, '\0', 5);
 
      Mptr->ReWx[i].Bhh = MAXINT;
      Mptr->ReWx[i].Bmm = MAXINT;
 
      Mptr->ReWx[i].Ehh = MAXINT;
      Mptr->ReWx[i].Emm = MAXINT;
 
   }
 
   Mptr->NIL_rpt = FALSE;
   Mptr->AUTO = FALSE;
   Mptr->COR = FALSE;
 
   Mptr->winData.windDir = MAXINT;
   Mptr->winData.windSpeed = MAXINT;
   Mptr->winData.windGust = MAXINT;
   Mptr->winData.windVRB  = FALSE;
   Mptr->winData.windEstimated  = FALSE;
   memset(Mptr->winData.windUnits, '\0', 4);
 
   Mptr->minWnDir = MAXINT;
   Mptr->maxWnDir = MAXINT;
 
   memset(Mptr->horiz_vsby, '\0', 5);
   memset(Mptr->dir_min_horiz_vsby, '\0', 3);
 
   Mptr->prevail_vsbySM = (float) MAXINT;
   Mptr->prevail_vsbyM  = (float) MAXINT;
   Mptr->prevail_vsbyKM = (float) MAXINT;
   memset(Mptr->charPrevailVsby, '\0', 12);
 
   memset(Mptr->vsby_Dir, '\0', 3);
 
   Mptr->CAVOK = FALSE;
 
   for ( i = 0; i < 12; i++ )
   {
      memset(Mptr->RRVR[ i ].runway_designator,
              '\0', 6);
 
      Mptr->RRVR[ i ].visRange = MAXINT;
 
      Mptr->RRVR[ i ].vrbl_visRange = FALSE;
      Mptr->RRVR[ i ].below_min_RVR = FALSE;
      Mptr->RRVR[ i ].above_max_RVR = FALSE;
 
 
      Mptr->RRVR[ i ].Max_visRange = MAXINT;
      Mptr->RRVR[ i ].Min_visRange = MAXINT;
   }
 
   Mptr->DVR.visRange = MAXINT;
   Mptr->DVR.vrbl_visRange = FALSE;
   Mptr->DVR.below_min_DVR = FALSE;
   Mptr->DVR.above_max_DVR = FALSE;
   Mptr->DVR.Max_visRange = MAXINT;
   Mptr->DVR.Min_visRange = MAXINT;
 
   for ( i = 0; i < MAXWXSYMBOLS; i++ )
   {
      for( j = 0; j < 8; j++ )
         Mptr->WxObstruct[i][j] = '\0';
   }
 
   /***********************/
   /* PARTIAL OBSCURATION */
   /***********************/
 
   memset( &(Mptr->PartialObscurationAmt[0][0]), '\0', 7 );
   memset( &(Mptr->PartialObscurationPhenom[0][0]), '\0',12);
 
   memset( &(Mptr->PartialObscurationAmt[1][0]), '\0', 7 );
   memset( &(Mptr->PartialObscurationPhenom[1][0]), '\0',12);
 
 
   /***************************************************/
   /* CLOUD TYPE, CLOUD LEVEL, AND SIGNIFICANT CLOUDS */
   /***************************************************/
 
 
   for ( i = 0; i < 6; i++ )
   {
      memset(Mptr->cldTypHgt[ i ].cloud_type,
              '\0', 5);
 
      memset(Mptr->cldTypHgt[ i ].cloud_hgt_char,
              '\0', 4);
 
      Mptr->cldTypHgt[ i ].cloud_hgt_meters = MAXINT;
 
      memset(Mptr->cldTypHgt[ i ].other_cld_phenom,
              '\0', 4);
   }
 
   Mptr->VertVsby = MAXINT;
   memset( Mptr->charVertVsby, '\0', 10 );
 
   Mptr->temp = MAXINT;
   Mptr->dew_pt_temp = MAXINT;
   Mptr->QFE = MAXINT;
 
   Mptr->SLPNO = FALSE;
   Mptr->SLP = (float) MAXINT;
 
   Mptr->A_altstng = FALSE;
   Mptr->inches_altstng = (double) MAXINT;
 
   Mptr->Q_altstng = FALSE;
   Mptr->hectoPasc_altstng = MAXINT;
 
   Mptr->char_prestndcy = MAXINT;
   Mptr->prestndcy = (float) MAXINT;
 
   Mptr->precip_amt = (float) MAXINT;
 
   Mptr->precip_24_amt = (float) MAXINT;
   Mptr->maxtemp       = (float) MAXINT;
   Mptr->mintemp       = (float) MAXINT;
   Mptr->max24temp     = (float) MAXINT;
   Mptr->min24temp     = (float) MAXINT;
 
   Mptr->VIRGA         = FALSE;
   memset( Mptr->VIRGA_DIR, '\0', 3 );
 
   Mptr->VOLCASH       = FALSE;
 
   Mptr->minCeiling    = MAXINT;
   Mptr->maxCeiling    = MAXINT;
 
   Mptr->CIG_2ndSite_Meters = MAXINT;
   memset(Mptr->CIG_2ndSite_LOC, '\0', 10 );
 
   Mptr->minVsby = (float) MAXINT;
   Mptr->maxVsby = (float) MAXINT;
   Mptr->VSBY_2ndSite = (float) MAXINT;
   memset(Mptr->VSBY_2ndSite_LOC,'\0',10);
 
   for( i = 0; i < 6; i++ )
      memset (&(Mptr->SfcObscuration[i][0]), '\0', 10);
 
   Mptr->Num8thsSkyObscured = MAXINT;
 
   Mptr->CIGNO = FALSE;
   Mptr->Ceiling = MAXINT;
   Mptr->Estimated_Ceiling = MAXINT;
 
   Mptr->NOSPECI = FALSE;
   Mptr->LAST    = FALSE;
 
   Mptr->SNINCR = MAXINT;
   Mptr->SNINCR_TotalDepth = MAXINT;
 
   Mptr->WaterEquivSnow = (float) MAXINT;
 
   Mptr->SunshineDur = MAXINT;
   Mptr->SunSensorOut = FALSE;
 
 
   Mptr->WshfTime_hour = MAXINT;
   Mptr->WshfTime_minute = MAXINT;
   Mptr->Wshft_FROPA     = FALSE;
   Mptr->min_vrbl_wind_dir = MAXINT;
   Mptr->max_vrbl_wind_dir = MAXINT;
 
   Mptr->PRESRR        = FALSE;
   Mptr->PRESFR        = FALSE;
 
   Mptr->TWR_VSBY = (float) MAXINT;
   Mptr->SFC_VSBY = (float) MAXINT;
 
   Mptr->PKWND_dir = MAXINT;
   Mptr->PKWND_speed = MAXINT;
   Mptr->PKWND_hour = MAXINT;
   Mptr->PKWND_minute = MAXINT;
 
   return;
 
}
/********************************************************************/
/*                                                                  */
/*  Title:         ResetMETARGroup                                  */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:  ResetMETARGroup returns a METAR_obGroup enumerated   */
/*             variable that indicates which METAR reporting group  */
/*             might next appear in the METAR report and should be  */
/*             considered for decoding.                             */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                                                                  */
/*  Input:         StartGroup - a METAR_obGroup variable that       */
/*                              indicates where or on what group    */
/*                              METAR Decoding began.               */
/*                                                                  */
/*                 SaveStartGroup - a METAR_obGroup variable that   */
/*                                  indicates the reporting group   */
/*                                  in the METAR report that was    */
/*                                  successfully decoded.           */
/*                                                                  */
/*  Output:        A METAR_obGroup variable that indicates which    */
/*                 reporting group in the METAR report should next  */
/*                 be considered for decoding                       */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
static int ResetMETARGroup( int StartGroup,
                            int SaveStartGroup )
{
 
   enum METAR_obGroup { codename, stnid, NIL1, obDateTime, NIL2,
                        AUTO, COR, windData, MinMaxWinDir, visibility,
                        RVR, presentWX, skyCond, tempGroup,
                        altimStng, NotIDed = 99};
 
   if( StartGroup == NotIDed && SaveStartGroup == NotIDed )
      return NotIDed;
   else if( StartGroup == NotIDed && SaveStartGroup != NotIDed &&
            SaveStartGroup != altimStng )
      return (++SaveStartGroup);
   else
      return (++SaveStartGroup);
 
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         CodedHgt2Meters                                  */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:  CodedHgt2Meters converts a coded cloud height into   */
/*             meters.                                              */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                                                                  */
/*  Input:         token - a pointer to a METAR report group.       */
/*                 Mptr - a pointer to a decoded_METAR structure.   */
/*                                                                  */
/*  Output:        Cloud height in meters                           */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
static int CodedHgt2Meters( char *token, Decoded_METAR *Mptr )
{
   int hgt;
   static int maxhgt = 30000;
 
 
   if( (hgt = atoi(token)) == 999 )
      return maxhgt;
   else
      return ((int)(hgt*30.48+0.5));
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isPartObscur                                     */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:  isPartObscur determines whether or not the METAR     */
/*             report element that is passed to it is or is not     */
/*             a partial obscuration indicator for an amount of     */
/*             obscuration.                                         */
/*                                                                  */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                                                                  */
/*  Input:         token - the address of a pointer to the group    */
/*                         in the METAR report that isPartObscur    */
/*                         determines is or is not a partial        */
/*                         obscuration indicator.                   */
/*                                                                  */
/*                                                                  */
/*                 Mptr - a pointer to a decoded_METAR structure.   */
/*                                                                  */
/*  Output:        TRUE, if the group is a partial obscuration      */
/*                 indicator and FALSE, if it is not.               */
/*                                                                  */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
static ubool isPartObscur( char **string, Decoded_METAR *Mptr,
                          int *NDEX )
{
   if( strcmp( *string, "FEW///" ) == 0 ||
       strcmp( *string, "SCT///" ) == 0 ||
       strcmp( *string, "BKN///" ) == 0 ||
       strcmp( *string, "FEW000" ) == 0 ||
       strcmp( *string, "SCT000" ) == 0 ||
       strcmp( *string, "BKN000" ) == 0    ) {
      strcpy( &(Mptr->PartialObscurationAmt[0][0]), *string );
      (*NDEX)++;
      string++;
 
      if( strcmp( (*string+3), "///") ) {
          if( strcmp( *string, "FEW000" ) == 0 ||
              strcmp( *string, "SCT000" ) == 0 ||
              strcmp( *string, "BKN000" ) == 0    ) {
            strcpy( &(Mptr->PartialObscurationAmt[1][0]), *string );
            (*NDEX)++;
         }
      }
      else {
         if( strcmp( *string, "FEW///" ) == 0 ||
             strcmp( *string, "SCT///" ) == 0 ||
             strcmp( *string, "BKN///" ) == 0 ) {
            strcpy( &(Mptr->PartialObscurationAmt[1][0]), *string );
            (*NDEX)++;
         }
      }
      return TRUE;
   }
   else
      return FALSE;
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isCldLayer                                       */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:      isCldLayer determines whether or not the         */
/*                 current group has a valid cloud layer            */
/*                 identifier.                                      */
/*                                                                  */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                                                                  */
/*  Input:         token - pointer to a METAR report group.         */
/*                                                                  */
/*  Output:        TRUE, if the report group is a valid cloud       */
/*                 layer indicator.                                 */
/*                                                                  */
/*                 FALSE, if the report group is not a valid cloud  */
/*                 layer indicator.                                 */
/*                                                                  */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
static ubool isCldLayer( char *token )
{
 
   if( token == NULL || strlen(token) < 6 )
      return FALSE;
   else
      return ((strncmp(token,"OVC",3) == 0 ||
               strncmp(token,"SCT",3) == 0 ||
               strncmp(token,"FEW",3) == 0 ||
               strncmp(token,"BKN",3) == 0 ||
               (isdigit(*token) &&
                strncmp(token+1,"CU",2) == 0) ||
               (isdigit(*token) &&
                strncmp(token+1,"SC",2) == 0) ) &&
               nisdigit((token+3),3)) ? TRUE:FALSE;
}
 
 
/********************************************************************/
/*                                                                  */
/*  Title:         parseCldData                                     */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:                                                       */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                                                                  */
/*  Input:         x                                                */
/*                                                                  */
/*  Output:        x                                                */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
static void parseCldData( char *token, Decoded_METAR *Mptr, int next)
{
 
 
   if(token == NULL) return;

   if( strlen(token) > 6 )
      strncpy(Mptr->cldTypHgt[next].other_cld_phenom,token+6,
              (strlen(token)-6));
 
   strncpy(Mptr->cldTypHgt[next].cloud_type,token,3);
 
   strncpy(Mptr->cldTypHgt[next].cloud_hgt_char,token+3,3);
 
   Mptr->cldTypHgt[next].cloud_hgt_meters =
                               CodedHgt2Meters( token+3, Mptr );
 
   return;
}
 
 
/********************************************************************/
/*                                                                  */
/*  Title:         isSkyCond                                        */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:                                                       */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                                                                  */
/*  Input:         x                                                */
/*                                                                  */
/*  Output:        x                                                */
/*                                                                  */
/*  Returns: 1 if *skycond is a sky condition, 0 otherwise          */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
static int isSkyCond( char **skycond, Decoded_METAR *Mptr,
                      int *NDEX )
{
 
// Code replaced by S Sullivan, 2007-06-11
//   ubool first_layer,
//        second_layer,
//        third_layer;
//   int next;
// End code replaced by S Sullivan, 2007-06-11

 
      /********************************************************/
      /* INTERROGATE skycond TO DETERMINE IF "CLR" IS PRESENT */
      /********************************************************/
 
   if( strcmp(*skycond,"CLR") == 0)
   {
      strcpy(Mptr->cldTypHgt[0].cloud_type,"CLR");

      /* The following added by Niles for our
	 Taiwan project. */

      Mptr->Estimated_Ceiling  = CLR_CEILING;
      Mptr->Ceiling  = CLR_CEILING;
      strcpy(Mptr->charPrevailVsby, *skycond);

      /* Additions by Niles end. */

/*
      memset(Mptr->cldTypHgt[0].cloud_hgt_char,'\0',1);
      memset(Mptr->cldTypHgt[0].other_cld_phenom,
              '\0', 1);
*/
      (*NDEX)++;
      return 1;
   }
 
      /********************************************************/
      /* INTERROGATE skycond TO DETERMINE IF "SKC" IS PRESENT */
      /********************************************************/
 
   else if( strcmp(*skycond,"SKC") == 0)
   {
      strcpy(Mptr->cldTypHgt[0].cloud_type,"SKC");

      Mptr->Estimated_Ceiling  = SKC_CEILING;
      Mptr->Ceiling  = SKC_CEILING;
      strcpy(Mptr->charPrevailVsby, *skycond);
/*
      memset(Mptr->cldTypHgt[0].cloud_hgt_char,'\0',1);
      memset(Mptr->cldTypHgt[0].other_cld_phenom,'\0', 1);
*/
      (*NDEX)++;
      return 1;
   }
 
      /********************************************************/
      /* INTERROGATE skycond TO DETERMINE IF "NSC" IS PRESENT */
      /********************************************************/
 
   else if( strcmp(*skycond,"NSC") == 0)
   {
      strcpy(Mptr->cldTypHgt[0].cloud_type,"NSC");

      Mptr->Estimated_Ceiling  = NSC_CEILING_FT;
      Mptr->Ceiling  = NSC_CEILING_FT;
      strcpy(Mptr->charPrevailVsby, *skycond);

      (*NDEX)++;
      return 1;
   }
 
      /****************************************/
      /* INTERROGATE skycond TO DETERMINE IF  */
      /*    VERTICAL VISIBILITY IS PRESENT    */
      /****************************************/
 
   else if( strncmp(*skycond,"VV",2) == 0
             && strlen(*skycond) == 5 &&
                  nisdigit((*skycond+2),3) )
   {
      Mptr->VertVsby = CodedHgt2Meters( (*skycond+2), Mptr);
      strcpy( Mptr->charVertVsby, *skycond);
      (*NDEX)++;
      return 1;
   }
 
      /****************************************/
      /* INTERROGATE skycond TO DETERMINE IF  */
      /*    CLOUD LAYER DATA IS PRESENT       */
      /****************************************/

   else if( strncmp(*skycond, "OVC", 3) == 0 ||
            strncmp(*skycond, "SCT", 3) == 0 ||
            strncmp(*skycond, "FEW", 3) == 0 ||
            strncmp(*skycond, "BKN", 3) == 0 ||
                  ( (isdigit(**skycond) &&
                     strncmp(*skycond+1,"CU",2) == 0) ||
                    (isdigit(**skycond) &&
                     strncmp(*skycond+1,"SC",2) == 0) ) )
   {


// New code by S Sullivan, 2007-06-11
      int ilayer = 0;
      while (ilayer < 6) {
        if (skycond[ilayer] == NULL || ! isCldLayer( skycond[ilayer]))
          break;
        parseCldData( skycond[ilayer], Mptr, ilayer);
        (*NDEX)++;
        ilayer++;
      }
      if (ilayer > 0) return 1;
      else return 0;
// End new code by S Sullivan, 2007-06-11


// Code replaced by above, by S Sullivan, 2007-06-11
//      next = 0;
// 
//      first_layer = FALSE;
//      second_layer = FALSE;
//      third_layer = FALSE;
//
//      if( *skycond && isCldLayer( *skycond ))
//      {
//         parseCldData( *skycond , Mptr, next );
//         first_layer = TRUE;
//         next++;
//      }
//
//      ++skycond;
//      if( first_layer && *skycond && isCldLayer( *skycond ) )
//      {
//         parseCldData( *skycond, Mptr, next );
//         second_layer = TRUE;
//         next++;
//      }
//      ++skycond;
//      if( first_layer && second_layer && *skycond && isCldLayer( *skycond ) )
//      {
//         parseCldData( *skycond , Mptr, next );
//         third_layer = TRUE;
//      }
// 
//      if( third_layer )
//      {
//         (*NDEX)++;
//         (*NDEX)++;
//         (*NDEX)++;
//         return 1;
//      }
//      else if( second_layer )
//      {
//         (*NDEX)++;
//         (*NDEX)++;
//         return 1;
//      }
//      else if( first_layer )
//      {
//         (*NDEX)++;
//         return 1;
//      }
//      else
//         return 0;
// End code replaced by above, by S Sullivan, 2007-06-11


   } // else if *skycond is OVC or SKT or ...


   else
      return 0;
}
/********************************************************************/
/*                                                                  */
/*  Title:         prevailVSBY                                      */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:                                                       */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                                                                  */
/*  Input:         x                                                */
/*                                                                  */
/*  Output:        x                                                */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
static float prevailVSBY( char *visibility )
{

   float Miles_vsby;
   char *Slash_ptr, *SM_KM_ptr;
   char num_str[32], denom_str[32];
   char str[32];
   int len;

   double num, denom;
   
   if( (SM_KM_ptr = strstr( visibility, "SM" )) == NULL ) {
      SM_KM_ptr = strstr(visibility, "KM");
      if (SM_KM_ptr == NULL) {
	return (0.0);
      }
   }
 
   Slash_ptr = strchr( visibility, '/' );
 
   if( Slash_ptr == NULL )
     {
      memset( str, 0, 32);
      strncpy( str, visibility, (SM_KM_ptr-visibility) );
      Miles_vsby = (float) (atoi(str));
      return Miles_vsby;
   }
   else
   {
     
      memset( num_str, 0, 32);
      memset( denom_str, 0, 32);

      len = Slash_ptr - visibility;
      if (len < 1 || len > 16) {
	return (0.0);
      }
      strncpy(num_str, visibility, len);

      len = SM_KM_ptr - Slash_ptr;
      if (len < 1 || len > 16) {
	return (0.0);
      }
      strncpy(denom_str, Slash_ptr+1, len);

      num = atoi(num_str);
      denom = atoi(denom_str);

      if (denom == 0.0) {
	return (0.0);
      } else {
	return (num / denom);
      }

   }
 
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isVisibility                                     */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:                                                       */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                                                                  */
/*  Input:         x                                                */
/*                                                                  */
/*  Output:        x                                                */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
 
static ubool isVisibility( char **visblty, Decoded_METAR *Mptr,
                          int *NDEX )
{
   char *achar, *astring, VsbyString[12];
 
   static float KM_2_miles     = 0.62137,
                METERS_2_miles = 0.00062137;
 
   /****************************************/
   /* CHECK FOR CAVOK                      */
   /****************************************/
 
   if( strcmp(*visblty,"CAVOK") == 0 ) {
     Mptr->prevail_vsbySM = KM_2_miles * CAVOK_VIS_KM;
     Mptr->prevail_vsbyM = CAVOK_VIS_KM * 1000.0;
     strcpy(Mptr->charPrevailVsby, *visblty);
     Mptr->Estimated_Ceiling  = CAVOK_CEILING_FT;
     Mptr->Ceiling  = CAVOK_CEILING_FT;
     (*NDEX)++;
     return TRUE;
   }

   /****************************************/
   /* CHECK FOR VISIBILITY MEASURED <1/4SM */
   /****************************************/
 
   if( strcmp(*visblty,"M1/4SM") == 0 ||
       strcmp(*visblty,"<1/4SM") == 0 ) {
      Mptr->prevail_vsbySM = 0.0;
      Mptr->prevail_vsbyM = 0.0;
      Mptr->prevail_vsbyKM = 0.0;
      strcpy(Mptr->charPrevailVsby, *visblty);
      (*NDEX)++;
      return TRUE;
   }
 
   /***********************************************/
   /* CHECK FOR VISIBILITY MEASURED IN KILOMETERS */
   /***********************************************/
 
   if( (achar = strstr(*visblty, "KM")) != NULL )
   {
      if( nisdigit(*visblty,(achar - *visblty)))
      {
         Mptr->prevail_vsbyKM = prevailVSBY( *visblty );
         Mptr->prevail_vsbySM = KM_2_miles * Mptr->prevail_vsbyKM;
         strcpy(Mptr->charPrevailVsby, *visblty);
         (*NDEX)++;
         return TRUE;
      }
      else
         return FALSE;
   }
 
   /***********************************/
   /* CHECK FOR VISIBILITY MEASURED   */
   /* IN A FRACTION OF A STATUTE MILE */
   /***********************************/
 
   else if( (achar = strchr( *visblty, '/' )) !=
                    NULL &&
       (astring = strstr( *visblty, "SM")) != NULL )
   {
      if( nisdigit(*visblty,(achar - *visblty))
                     &&
                nisdigit(achar+1, (astring - (achar+1))) )
      {
         Mptr->prevail_vsbySM = prevailVSBY (*visblty);
         strcpy(Mptr->charPrevailVsby, *visblty);
         (*NDEX)++;
         return TRUE;
      }
      else
         return FALSE;
   }
 
   /***********************************/
   /* CHECK FOR VISIBILITY MEASURED   */
   /*     IN WHOLE STATUTE MILES      */
   /***********************************/
 
   else if( (astring = strstr(*visblty,"SM") ) != NULL )
   {
      if( nisdigit(*visblty,(astring - *visblty)))
      {
         Mptr->prevail_vsbySM = prevailVSBY (*visblty);
         strcpy(Mptr->charPrevailVsby, *visblty);
         (*NDEX)++;
         return TRUE;
      }
      else
         return FALSE;
   }
 
   /***********************************/
   /* CHECK FOR VISIBILITY MEASURED   */
   /* IN WHOLE AND FRACTIONAL STATUTE */
   /*             MILES               */
   /***********************************/
 
   else if( nisdigit( *visblty,
               strlen(*visblty)) &&
                            strlen(*visblty) < 4 )
   {
      char save_token[128];
      if (strlen(*visblty) > 127) return FALSE;
      strcpy(save_token,*visblty);
      if( *(++visblty) == NULL)
      {
         return FALSE;
      }
 
      if( (achar = strchr( *visblty, '/' ) ) != NULL &&
          (astring = strstr( *visblty, "SM") ) != NULL  )
      {
         if( nisdigit(*visblty,
                 (achar - *visblty)) &&
             nisdigit(achar+1, (astring - (achar+1))) )
         {
            Mptr->prevail_vsbySM = prevailVSBY (*visblty);
            Mptr->prevail_vsbySM +=
                                 (float) (atoi(save_token));
 
            memset( VsbyString, ' ',12 );
            strncpy( VsbyString,save_token,strlen(save_token));
            strcpy( VsbyString+strlen(save_token)+1,*visblty);
 
            strcpy(Mptr->charPrevailVsby, VsbyString);
 
            (*NDEX)++;
            (*NDEX)++;
 
            return TRUE;
 
         }
         else {
            return FALSE;
         }
      }
      else {
         return FALSE;
      }
   }
 
   /***********************************/
   /* CHECK FOR VISIBILITY MEASURED   */
   /* IN METERS WITH OR WITHOUT DI-   */
   /*     RECTION OF OBSERVATION      */
   /***********************************/
 
   else if( nisdigit(*visblty,4) &&
                strlen(*visblty) >= 4)
   {
      if( strcmp(*visblty+4,"NE") == 0 )
      {
         memset(Mptr->vsby_Dir,'\0',3);
         strcpy(Mptr->vsby_Dir,*visblty+4);
      }
      if( strcmp(*visblty+4,"NW") == 0 )
      {
         memset(Mptr->vsby_Dir,'\0',3);
         strcpy(Mptr->vsby_Dir,*visblty+4);
      }
      if( strcmp(*visblty+4,"SE") == 0 )
      {
         memset(Mptr->vsby_Dir,'\0',3);
         strcpy(Mptr->vsby_Dir,*visblty+4);
      }
      if( strcmp(*visblty+4,"SW") == 0 )
      {
         memset(Mptr->vsby_Dir,'\0',3);
         strcpy(Mptr->vsby_Dir,*visblty+4);
      }
      if( strcmp(*visblty+4,"N") == 0 )
      {
         memset(Mptr->vsby_Dir,'\0',3);
         strcpy(Mptr->vsby_Dir,*visblty+4);
      }
      if( strcmp(*visblty+4,"S") == 0 )
      {
         memset(Mptr->vsby_Dir,'\0',3);
         strcpy(Mptr->vsby_Dir,*visblty+4);
      }
      if( strcmp(*visblty+4,"E") == 0 )
      {
         memset(Mptr->vsby_Dir,'\0',3);
         strcpy(Mptr->vsby_Dir,*visblty+4);
      }
      if( strcmp(*visblty+4,"W") == 0 )
      {
         memset(Mptr->vsby_Dir,'\0',3);
         strcpy(Mptr->vsby_Dir,*visblty+4);
      }
 
      if( (antoi(*visblty, strlen(*visblty)) >= 50) &&
          (antoi(*visblty, strlen(*visblty)) <= 800) &&
          ((antoi(*visblty, strlen(*visblty)) % 50) == 0) )
      {
         Mptr->prevail_vsbyM =
           (float) (antoi(*visblty,
                       strlen(*visblty)));
         Mptr->prevail_vsbySM = METERS_2_miles *
                                  Mptr->prevail_vsbyM;
         strcpy(Mptr->charPrevailVsby,*visblty);
         (*NDEX)++;
         return TRUE;
      }
      else if( (antoi(*visblty, strlen(*visblty)) >= 800) &&
               (antoi(*visblty, strlen(*visblty)) <= 5000) &&
               ((antoi(*visblty, strlen(*visblty)) % 100) == 0) )
      {
         Mptr->prevail_vsbyM =
            (float) (antoi(*visblty,
                      strlen(*visblty)));
         Mptr->prevail_vsbySM = METERS_2_miles *
                                  Mptr->prevail_vsbyM;
         strcpy(Mptr->charPrevailVsby,*visblty);
         (*NDEX)++;
         return TRUE;
      }
      else if( ((antoi(*visblty, strlen(*visblty)) >= 5000) &&
                (antoi(*visblty, strlen(*visblty)) <= 9999) &&
                ((antoi(*visblty, strlen(*visblty)) % 500) == 0)) ||
               (antoi(*visblty, strlen(*visblty)) == 9999) )
      {
         Mptr->prevail_vsbyM =
                (float) (antoi(*visblty,
                     strlen(*visblty)));
         Mptr->prevail_vsbySM = METERS_2_miles *
                                  Mptr->prevail_vsbyM;
         strcpy(Mptr->charPrevailVsby,*visblty);
         (*NDEX)++;
         return TRUE;
      }
      else
         return FALSE;
 
   }

   return FALSE;          /* added by Jaimi Yee */
 
}
  
 
/********************************************************************/
/*                                                                  */
/*  Title:         isMinMaxWinDir                                   */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:                                                       */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                                                                  */
/*  Input:         x                                                */
/*                                                                  */
/*  Output:        x                                                */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
static ubool isMinMaxWinDir( char *string, Decoded_METAR *Mptr,
			     int *NDEX )
{
   char buf[ 100 ];
   char *V_char;
 
 
   if( (V_char = strchr(string,'V')) == NULL )
      return FALSE;
   else
   {
      if( nisdigit(string,(V_char - string)) &&
               nisdigit(V_char+1,3) )
      {
         memset( buf, '\0', 100);
         strncpy( buf, string, V_char - string);
         Mptr->minWnDir = atoi( buf );
 
         memset( buf, '\0', 100);
         strcpy( buf, V_char+1 );
         Mptr->maxWnDir = atoi( buf );
 
         (*NDEX)++;
         return TRUE;
      }
      else
         return FALSE;
   }
}
/********************************************************************/
/*                                                                  */
/*  Title:         isRVR                                            */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:                                                       */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                                                                  */
/*  Input:         x                                                */
/*                                                                  */
/*  Output:        x                                                */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
static ubool isRVR( char *token, Decoded_METAR *Mptr, int *NDEX,
                     int ndex )
{
   char *slashPtr, *FT_ptr;
   char *vPtr;
   int length;
 
 
   if(token == NULL) return FALSE;

   if( *token != 'R' || (length = strlen(token)) < 7 ||
        (slashPtr = strchr(token,'/')) == NULL ||
        nisdigit(token+1,2) == FALSE )
      return FALSE;
 
   if( (slashPtr - (token+3)) > 0 )
      if( !nisalpha(token+3,(slashPtr - (token+3))) )
         return FALSE;
 
   if( strcmp(token+(strlen(token)-2),"FT") != 0 )
      return FALSE;
   else
      FT_ptr = token + (strlen(token)-2);
 
   if( strchr(slashPtr+1, 'P' ) != NULL )
      Mptr->RRVR[ndex].above_max_RVR = TRUE;
 
   if( strchr(slashPtr+1, 'M' ) != NULL )
      Mptr->RRVR[ndex].below_min_RVR = TRUE;
 
 
   strncpy(Mptr->RRVR[ndex].runway_designator, token+1,
           (slashPtr-(token+1)));
 
   if( (vPtr = strchr(slashPtr, 'V' )) != NULL )
   {
      Mptr->RRVR[ndex].vrbl_visRange = TRUE;
      Mptr->RRVR[ndex].Min_visRange = antoi(slashPtr+1,
                              (vPtr-(slashPtr+1)) );
      Mptr->RRVR[ndex].Max_visRange = antoi(vPtr+1,
                              (FT_ptr - (vPtr+1)) );
      (*NDEX)++;
      return TRUE;
   }
   else
   {
      if( Mptr->RRVR[ndex].below_min_RVR ||
          Mptr->RRVR[ndex].above_max_RVR    )
         Mptr->RRVR[ndex].visRange = antoi(slashPtr+2,
                           (FT_ptr - (slashPtr+2)) );
      else
         Mptr->RRVR[ndex].visRange = antoi(slashPtr+1,
                           (FT_ptr - (slashPtr+1)) );
 
      (*NDEX)++;
      return TRUE;
   }
 
}
 
 
/********************************************************************/
/*                                                                  */
/*  Title:         isAltimStng                                      */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:                                                       */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                                                                  */
/*  Input:         x                                                */
/*                                                                  */
/*  Output:        x                                                */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
static ubool isAltimStng( char *token, Decoded_METAR *Mptr, int *NDEX )
{

  int val;
  char tmpstr[8];

  // ubool foundQA = FALSE; 
  
  if(token == NULL) {
    return FALSE;
  }
  if( strlen(token) < 5 ) {
    return FALSE;
  }
  if( (*token != 'A' && *token != 'Q')) {
    return FALSE;
  }

  // special case to decode Q&A when given: Q1007(A2974)
  if (token[0] == 'Q' && token[6] == 'A')
    {
      char QAstr[strlen(token)+1];
      strcpy(QAstr,token);
      QAstr[5] = '\0';
      QAstr[11] = '\0';
      if (sscanf(QAstr+1, "%d", &val) == 1) {
	Mptr->Q_altstng = TRUE;
	Mptr->hectoPasc_altstng = val;
      } else {
	return FALSE;
      }
      if (sscanf(QAstr+7, "%d", &val) == 1) {
	Mptr->A_altstng = TRUE;
	Mptr->inches_altstng = val * 0.01;
      } else {
	return FALSE;
      }
      return TRUE;
    }

  memset(tmpstr, '\0', 8);
  STRncopy(tmpstr, token, 8);
  
  if(strlen(tmpstr) > 5) {
    tmpstr[5] = '\0';
  }
  Mptr->A_altstng = FALSE;
  Mptr->Q_altstng = FALSE;
  
  if( *tmpstr == 'A' ) {
    
    if (sscanf(tmpstr+1, "%d", &val) == 1) {
      Mptr->A_altstng = TRUE;
      Mptr->inches_altstng = val * 0.01;
    } else {
      return FALSE;
    }

   } else {
     
     if (sscanf(tmpstr+1, "%d", &val) == 1) {
       Mptr->Q_altstng = TRUE;
       Mptr->hectoPasc_altstng = val;
     } else {
       return FALSE;
     }

   }
  
  (*NDEX)++;

  if (Mptr->Q_altstng)
    Mptr->inches_altstng = Mptr->hectoPasc_altstng * 29.92 / 1013.0;
  if (Mptr->A_altstng)
    Mptr->hectoPasc_altstng = Mptr->inches_altstng * 1013.0 / 29.92;

  /*
   *
   * Niles : Commented this out on advice from Greg Thompson.
   * This is properly decoded elsewhere.
   *
  if ((Mptr->A_altstng) || (Mptr->Q_altstng))
    Mptr->SLP = Mptr->hectoPasc_altstng;
    */

  return TRUE;

}
 
 
/********************************************************************/
/*                                                                  */
/*  Title:         isTempGroup                                      */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:                                                       */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                                                                  */
/*  Input:         x                                                */
/*                                                                  */
/*  Output:        x                                                */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
static ubool isTempGroup( char *token, Decoded_METAR *Mptr, int *NDEX)
{
 
   /***************************/
   /* DECLARE LOCAL VARIABLES */
   /***************************/
 
   char *slash;
 
 
   if( (slash = strchr(token,'/')) == NULL)
      return FALSE;
   else
   {
            /******************/
            /* CASE 1 (XX/YY) */
            /******************/
      if( nisdigit(token,(slash-token)) &&
                    *(slash+1) != '\0' &&
           nisdigit(slash+1,strlen(slash+1)) )
      {
         Mptr->temp = antoi(token,(slash-token));
         Mptr->dew_pt_temp = atoi(slash+1);
         (*NDEX)++;
         return TRUE;
      }
            /********************/
            /* CASE 2 (MXX/MYY) */
            /********************/
      else if( *token == 'M' && nisdigit(token+1,(slash-(token+1)))
                && *(slash+1) != '\0' &&
            *(slash+1) == 'M' && nisdigit(slash+2,strlen(slash+2)) )
      {
         Mptr->temp = antoi(token+1,(slash-(token+1))) * -1;
         Mptr->dew_pt_temp = atoi(slash+2) * -1;
         (*NDEX)++;
         return TRUE;
      }
            /*******************/
            /* CASE 3 (MXX/YY) */
            /*******************/
      else if( *token == 'M' && nisdigit(token+1,(slash-(token+1)))
                 && *(slash+1) != '\0' &&
               nisdigit(slash+1,strlen(slash+1)) )
      {
         Mptr->temp = antoi(token+1,(slash-(token+1))) * -1;
         Mptr->dew_pt_temp = atoi(slash+1);
         (*NDEX)++;
         return TRUE;
      }
            /*******************/
            /* CASE 4 (XX/MYY) */
            /*******************/
      else if( nisdigit(token,(slash-(token)))
                 && *(slash+1) != '\0' &&
            *(slash+1) == 'M' && nisdigit(slash+2,strlen(slash+2)) )
      {
         Mptr->temp = antoi(token,(slash-(token)));
         Mptr->dew_pt_temp = atoi(slash+2) * -1;
         (*NDEX)++;
         return TRUE;
      }
            /******************/
            /* CASE 5 (MM/YY) */
            /******************/
      else if( (slash-token ) == 2 &&
                 strncmp( token, "MM", (slash-token) ) == 0 &&
                    *(slash+1) != '\0' &&
                    nisdigit(slash+1,strlen(slash+1)) )
      {
         Mptr->dew_pt_temp = atoi(slash+1);
         (*NDEX)++;
         return TRUE;
      }
            /******************/
            /* CASE 6 (XX/MM) */
            /******************/
      else if( nisdigit(token, (slash-token)) &&
                    *(slash+1) != '\0' &&
                 strcmp( slash+1,"MM") == 0 )
      {
         Mptr->temp = antoi(token,(slash-token));
         (*NDEX)++;
         return TRUE;
      }
            /*******************/
            /* CASE 7 (MM/MYY) */
            /*******************/
      else if( (slash-token ) == 2 &&
                 strncmp( token, "MM", (slash-token) ) == 0 &&
                    *(slash+1) != '\0' && *(slash+1) == 'M' &&
                    nisdigit(slash+2,strlen(slash+2)) )
      {
         Mptr->dew_pt_temp = atoi(slash+2) * -1;
         (*NDEX)++;
         return TRUE;
      }
            /********************/
            /* CASE 8 (MXX/MM ) */
            /********************/
      else if( *token == 'M' &&
               nisdigit(token+1, (slash-(token+1))) &&
                    *(slash+1) != '\0' &&
                 strcmp( slash+1,"MM") == 0 )
      {
         Mptr->temp = antoi(token+1,(slash-(token+1))) * -1;
         (*NDEX)++;
         return TRUE;
      }
            /******************/
            /* CASE 9 (MM/MM) */
            /******************/
      else if( strcmp(token,"MM/MM") == 0 )
      {
         (*NDEX)++;
         return TRUE;
      }
      else
         return FALSE;
   }
 
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isPresentWX                                      */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:                                                       */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                                                                  */
/*  Input:         x                                                */
/*                                                                  */
/*  Output:        x                                                */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
static ubool isPresentWX( char *token, Decoded_METAR *Mptr,
                        int *NDEX, int *next )
{
/*
   static char *WxSymbols[] = {"BCFG", "BLDU", "BLSA", "BLPY",
          "DRDU", "DRSA", "DRSN", "DZ", "DS", "FZFG", "FZDZ", "FZRA",
          "FG", "FC", "FU", "GS", "GR", "HZ", "IC", "MIFG",
          "PE", "PO", "RA", "SHRA", "SHSN", "SHPE", "SHGS",
          "SHGR", "SN", "SG", "SQ", "SA", "SS", "TSRA",
          "TSSN", "TSPE", "TSGS", "TSGR", "TS", "VA", "VCFG", "VCFC",
          "VCSH", "VCPO", "VCBLDU", "VCBLSA", "VCBLSN", NULL};
*/
   static char *WxSymbols[] = {"BCFG", "BLDU", "BLSA", "BLPY",
          "BLSN","BR", "FZBR", "VCBR", "+FC", "BCFG",
          "DRDU", "DRSA", "DRSN", "DZ", "DS", "FZFG", "FZDZ", "FZRA",
          "FG", "FC", "FU", "GS", "GR", "HZ", "IC", "MIFG",
          "PE", "PO", "RA", "SHRA", "SHSN", "SHPE", "SHGS",
          "SHGR", "SN", "SG", "SQ", "SA", "SS", "TSRA",
          "TSSN", "TSPE", "TSGS", "TSGR", "TS","UP",
          "VA", "VCFG", "VCFC", "VCTS", "VCSS", "VCDS", "PRFG",
	  "VCSH", "VCPO", "VCBLDU", "VCBLSA", "VCBLSN", "DU", "SA", 
          "SS", NULL};

   int i;
   char *ptr,
        *temp_token,
        *save_token;

   char token_buf[128];
 
   if (strlen(token) > 127) return FALSE;
   strcpy(token_buf, token);
   temp_token = token_buf;
   while( temp_token != NULL && (*next) < MAXWXSYMBOLS )
   {
      i = 0;
      save_token = NULL;
 
      if( *temp_token == '+' || *temp_token == '-' )
      {
         save_token = temp_token;
         temp_token++;
      }
 
      while( WxSymbols[i] != NULL )
         if( strncmp(temp_token, WxSymbols[i],
               strlen(WxSymbols[i])) != 0 )
            i++;
         else
            break;
 
      if( WxSymbols[i] == NULL ) {
         return FALSE;
      }
      else
      {
 
         if( save_token != NULL )
         {
            strncpy( Mptr->WxObstruct[*next], save_token, 1);
            strcpy( (Mptr->WxObstruct[*next])+1,
                              WxSymbols[i]);
            (*next)++;
         }
         else
         {
            strcpy( Mptr->WxObstruct[*next], WxSymbols[i]);
            (*next)++;
         }
 
 
         if( strcmp(temp_token, WxSymbols[i]) != 0)
         {
            ptr = strstr(temp_token, WxSymbols[i]);
            temp_token = ptr + strlen(WxSymbols[i]);
         }
         else
         {
            temp_token = NULL;
         }
 
      }
 
   }
 
   (*NDEX)++;
   return TRUE;
 
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isStnID                                          */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:                                                       */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                                                                  */
/*  Input:         x                                                */
/*                                                                  */
/*  Output:        x                                                */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
static ubool isStnId( char *stnID, Decoded_METAR *Mptr, int *NDEX)
{
   if( strlen(stnID) == 4 && nisalnum(stnID, 4) )
   {
      strcpy(Mptr->stnid,stnID);
      (*NDEX)++;
      return TRUE;
   }
   else
      return FALSE;
 
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isCodeName                                       */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:                                                       */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                                                                  */
/*  Input:         x                                                */
/*                                                                  */
/*  Output:        x                                                */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
static ubool isCodeName( char *codename, Decoded_METAR *Mptr, int *NDEX)
{
   if( strcmp(codename,"METAR") == 0 ||
       strcmp(codename,"SPECI") == 0   )
   {
      strcpy(Mptr->codeName, codename );
      (*NDEX)++;
      return TRUE;
   }
   else
      return FALSE;
 
}
 
 
/********************************************************************/
/*                                                                  */
/*  Title:         isNIL                                            */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:                                                       */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                                                                  */
/*  Input:         x                                                */
/*                                                                  */
/*  Output:        x                                                */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
static ubool isNIL( char *token, Decoded_METAR *Mptr, int *NDEX)
{
   if( strcmp(token, "NIL") == 0 )
   {
      Mptr->NIL_rpt = TRUE;
      (*NDEX)++;
      return TRUE;
   }
   else
      return FALSE;
 
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isAUTO                                           */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:                                                       */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                                                                  */
/*  Input:         x                                                */
/*                                                                  */
/*  Output:        x                                                */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
static ubool isAUTO( char *token, Decoded_METAR *Mptr, int *NDEX)
{
   if( strcmp(token, "AUTO") == 0 )
   {
      Mptr->AUTO = TRUE;
      (*NDEX)++;
      return TRUE;
   }
   else
      return FALSE;
 
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isCOR                                            */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          20 Nov 1995                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:                                                       */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                                                                  */
/*  Input:         x                                                */
/*                                                                  */
/*  Output:        x                                                */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
static ubool isCOR( char *token, Decoded_METAR *Mptr, int *NDEX)
{
   if( strcmp(token, "COR") == 0 )
   {
      Mptr->COR = TRUE;
      (*NDEX)++;
      return TRUE;
   }
   else
      return FALSE;
 
}
/********************************************************************/
/*                                                                  */
/*  Title:         isTimeUTC                                        */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:                                                       */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                                                                  */
/*  Input:         x                                                */
/*                                                                  */
/*  Output:        x                                                */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
static ubool isTimeUTC( char *UTC, Decoded_METAR *Mptr, int *NDEX )
{
   if( strlen( UTC ) == 5 &&
       (nisdigit(UTC,4) && (*(UTC+4) == 'Z')) )
   {
      Mptr->ob_hour = antoi(UTC,2);
      Mptr->ob_minute = antoi(UTC+2,2);
      (*NDEX)++;
      return TRUE;
   }
   else if( strlen( UTC ) == 7 &&
            (nisdigit(UTC,6) && (*(UTC+6) == 'Z')) )
   {
      Mptr->ob_date = antoi(UTC,2);
 
      Mptr->ob_hour = antoi(UTC+2, 2);
      Mptr->ob_minute = antoi(UTC+4, 2 );
      (*NDEX)++;
 
      return TRUE;
   }
   else
      return FALSE;
}
 
 
/********************************************************************/
/*                                                                  */
/*  Title:         isWindData                                       */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:                                                       */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                                                                  */
/*  Input:         x                                                */
/*                                                                  */
/*  Output:        x                                                */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
/* Original version  **********************************************

static ubool isWindData( char *wind, Decoded_METAR *Mptr, int *NDEX )
{
 
   char *GustPtr,
        *unitsPtr;
 
Modifed version P.Neilley 7/96, Handles Estimated winds  
Modified version M.Dixon 3/99, cleaned up, checks added.
****************************************************************** */

static ubool isWindData( char *windptr, Decoded_METAR *Mptr, int *NDEX )
{
 
   char *GustPtr, *wind, *unitsPtr, *endPtr;

   /*
    * basic failure
    */
   
   if (windptr == NULL) {
     return FALSE;
   }

   if (strlen(windptr) < 6) {
     return FALSE;
   }

   /*
    * check for estimated wind
    */

   wind = windptr;
   if ( strncmp(wind,"E",1) == 0 ) {
     wind++;
     Mptr->winData.windEstimated = TRUE;
   } else {
     Mptr->winData.windEstimated = FALSE;
   }

   /* 
    * set units
    */
   
   if( ( unitsPtr = strstr( wind, "KMH" ) ) != NULL ) {
     strcpy( Mptr->winData.windUnits, "KMH" );
   } else if( (unitsPtr = strstr( wind, "KT") ) != NULL ) {
     strcpy( Mptr->winData.windUnits, "KT" );
   } else if( (unitsPtr = strstr( wind, "MPS") ) != NULL ) {
     strcpy( Mptr->winData.windUnits, "MPS" );
   } else {
     return FALSE;
   }

   /*
    * is gust included?
    */
   
   if((GustPtr = strchr( wind, 'G' )) != NULL ) {
     endPtr = GustPtr;
     if (unitsPtr-(GustPtr+1) < 2) return FALSE;
     if (unitsPtr-(GustPtr+1) > 3) return FALSE;
     if (!nisdigit(GustPtr+1,(unitsPtr-(GustPtr+1)))) return FALSE;
     Mptr->winData.windGust = antoi(GustPtr+1,(unitsPtr- (GustPtr+1)));
   } else {
     Mptr->winData.windGust = 0;
     endPtr = unitsPtr;
   }

   if (endPtr-wind < 5) return FALSE;
   if (endPtr-wind > 6) return FALSE;

   if (nisdigit(wind, endPtr-wind)) {

     Mptr->winData.windSpeed = antoi(wind+3, endPtr-(wind+3));
     Mptr->winData.windDir = antoi(wind,3);

   } else if (strncmp(wind,"VRB",3) == 0 &&
	      nisdigit(wind+3, endPtr-(wind+3))) {
     
     Mptr->winData.windSpeed = antoi(wind+3, endPtr-(wind+3));
     Mptr->winData.windVRB = TRUE;
     Mptr->winData.windDir = 0;
     
   } else {
     
     return FALSE;
     
   }

   (*NDEX)++;
   return TRUE;

}

/********************************************************************/
/*                                                                  */
/*  Title:         DcdMETAR                                         */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          14 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:      DcdMETAR takes a pointer to a METAR report char- */
/*                 acter string as input, decodes the report, and   */
/*                 puts the individual decoded/parsed groups into   */
/*                 a structure that has the variable type           */
/*                 Decoded_METAR.                                   */
/*                                                                  */
/*  Input:         string - a pointer to a METAR report character   */
/*                          string.                                 */
/*                 clear_flag - Set to Non Zero to clear out        */
/*                 Decoded_METAR struct.                            */
/*                                                                  */
/*  Output:        Mptr   - a pointer to a structure that has the   */
/*                          variable type Decoded_METAR.            */
/*                                                                  */
/*  Modification History:                                           */
/*                Added clear_flag to optionally reset metar struct */
/*                F. Hage - NCAR.                                   */
/*                                                                  */
/********************************************************************/
 
 
int DcdMETAR( char *string , Decoded_METAR *Mptr,int  clear_flag)
{
 
   /***************************/
   /* DECLARE LOCAL VARIABLES */
   /***************************/
 
 
   enum METAR_obGroup { codename, stnid, NIL1, obDateTime, NIL2,
                        AUTO, COR, windData, MinMaxWinDir, visibility,
                        RVR, presentWX, PartialObscur,
                        skyCond, tempGroup,
                        altimStng, NotIDed = 99 }
     StartGroup = NotIDed,
     SaveStartGroup = NotIDed,
     MetarGroup = NotIDed;
 
 
   int    ndex,
          NDEX,
          skyCondRet;
 
 
   char   **token,
          *delimiters = {" \n\t\r"};
 
 
 
/*********************************/
/* BEGIN THE BODY OF THE ROUTINE */
/*********************************/
 
   /********************************************************/
   /* ONLY PARSE OR DECOCODE NON-NULL METAR REPORT STRINGS */
   /********************************************************/
 

   if( string == NULL )
      return 8;
 
 
   /*****************************************/
   /*   INITIALIZE STRUCTURE THAT HAS THE   */
   /*      VARIABLE TYPE Decoded_METAR      */
   /*****************************************/
 

   if(clear_flag != 0) {
       InitDcdMETAR( Mptr );
    } else {
       {               /* Only clear out the weather strings */
       int i;
       for ( i = 0; i < MAXWXSYMBOLS; i++ ) memset( Mptr->WxObstruct[i], '\0', 8);
       }
    }

 
 
   /****************************************************/
   /* TOKENIZE AND STORE THE INPUT METAR REPORT STRING */
   /****************************************************/
 
   /* Don't return without free'ing the tokens! - FH. */
   token = SaveTokenString( string, delimiters );
 
#ifdef  PRTOKEN
   printf("\n\nFROM DcdMETAR:  Print TOKEN list...\n\n");
   NDEX = 0;
   while( token[NDEX] != NULL ) {
      printf("Token[%d] = %s\n",NDEX,token[NDEX]);
      NDEX++;
   }
#endif
   /*********************************************************/
   /* DECODE THE METAR REPORT (POSITIONAL ORDER PRECEDENCE) */
   /*********************************************************/
 
   
   MetarGroup = codename;
 
   NDEX = 0;
   while(NDEX < MAXTOKENS && token[NDEX] != NULL &&
            (strcmp( token[NDEX], "RMK" )     != 0 &&
             strcmp( token[NDEX], "RMRK" )    != 0 &&
             strcmp( token[NDEX], "REMARK" )  != 0 &&
             strcmp( token[NDEX], "REMARKS" ) != 0  ) ) {

      StartGroup = NotIDed;
 
      /**********************************************/
      /* SET ID_break_CODE TO ITS DEFAULT VALUE OF  */
      /* 99, WHICH MEANS THAT NO SUCCESSFUL ATTEMPT */
      /* WAS MADE TO DECODE ANY METAR CODED GROUP   */
      /* FOR THIS PASS THROUGH THE DECODING LOOP    */
      /**********************************************/
 
      switch( MetarGroup ) {

         case( codename ):
            if( token[NDEX] && isCodeName( token[NDEX], Mptr, &NDEX ) ) {
               SaveStartGroup = StartGroup = codename;
               MetarGroup = stnid;
            }
            else
               MetarGroup = stnid;
            break;
         case( stnid ):
            if( token[NDEX] && isStnId( token[NDEX], Mptr, &NDEX ) ) {
               SaveStartGroup = StartGroup = stnid;
               MetarGroup = obDateTime;
            }
            else {
               freeTokens( token );
               return 12;
            }
            break;
         case( NIL1 ):
            if( token[NDEX] && isNIL( token[NDEX], Mptr, &NDEX ) ) {
               SaveStartGroup = StartGroup = NIL1;
               MetarGroup = obDateTime;
            }
            else
               MetarGroup = obDateTime;
            break;
         case( obDateTime ):
            if( token[NDEX] && isTimeUTC( token[NDEX], Mptr, &NDEX ) ) {
               SaveStartGroup = StartGroup = obDateTime;
               MetarGroup = NIL2;
            }
            else
               MetarGroup = NIL2;
            break;
         case( NIL2 ):
            if( token[NDEX] && isNIL( token[NDEX], Mptr, &NDEX ) ) {
               SaveStartGroup = StartGroup = NIL2;
               MetarGroup = AUTO;
            }
            else
               MetarGroup = AUTO;
            break;

         case( AUTO ):
            if( token[NDEX] && isAUTO( token[NDEX], Mptr, &NDEX ) ) {
               SaveStartGroup = StartGroup = AUTO;
               MetarGroup = COR;
            }
            else
               MetarGroup = COR;
            break;
         case( COR ):
            if( token[NDEX] && isCOR( token[NDEX], Mptr, &NDEX ) ) {
               SaveStartGroup = StartGroup = COR;
               MetarGroup = windData;
            }
            else
               MetarGroup = windData;
            break;
         case( windData ):
            if( token[NDEX] && isWindData( token[NDEX], Mptr, &NDEX ) ) {
               SaveStartGroup = StartGroup = windData;
               MetarGroup = MinMaxWinDir;
            }
            else
               MetarGroup = MinMaxWinDir;
            break;
         case( MinMaxWinDir ):
            if( token[NDEX] && isMinMaxWinDir( token[NDEX], Mptr, &NDEX ) ) {
               SaveStartGroup = StartGroup = MinMaxWinDir;
               MetarGroup = visibility;
            }
            else
               MetarGroup = visibility;
            break;
         case( visibility ):
            if( token[NDEX] && isVisibility( &(token[NDEX]), Mptr, &NDEX ) ) {
               SaveStartGroup = StartGroup = visibility;
               MetarGroup = RVR;
            }
            else
               MetarGroup = presentWX;
            break;
         case( RVR ):
            ndex = 0;
            MetarGroup = presentWX;
 
            while (token[NDEX] && isRVR( token[NDEX], Mptr, &NDEX, ndex ) &&
                               ndex < 12 ) {
               ndex++;
               SaveStartGroup = StartGroup = RVR;
               MetarGroup = presentWX;
            }
            break;

         case( presentWX ):
            ndex = 0;
            MetarGroup = skyCond;
 
            while( token[NDEX] && isPresentWX( token[NDEX], Mptr, &NDEX,
                          &ndex ) && ndex < MAXWXSYMBOLS) {
               SaveStartGroup = StartGroup = presentWX;
               MetarGroup = PartialObscur;
            }
            break;

         case( PartialObscur ):
            if( token[NDEX] && isPartObscur( &(token[NDEX]), Mptr, &NDEX ) ) {
               SaveStartGroup = StartGroup = PartialObscur;
               MetarGroup = skyCond;
            }
            else
               MetarGroup = skyCond;
            break;

         case( skyCond ):
	   if (token[NDEX] == NULL) {
	     freeTokens( token );
	     return 12;
	   }
            skyCondRet = isSkyCond( &(token[NDEX]), Mptr, &NDEX );
            if( skyCondRet == 1 ) {
               SaveStartGroup = StartGroup = skyCond;
               MetarGroup = tempGroup;
            } 
            else if( skyCondRet == 0 )
	       MetarGroup = tempGroup;
            else {
               freeTokens( token );
               return 12;
            }
            break;

         case( tempGroup ):
            if( token[NDEX] && isTempGroup( token[NDEX], Mptr, &NDEX ) ) {
               SaveStartGroup = StartGroup = tempGroup;
               MetarGroup = altimStng;
            }
            else
               MetarGroup = altimStng;
            break;
         case( altimStng ):
            if( token[NDEX] && isAltimStng( token[NDEX], Mptr, &NDEX ) ) {
               SaveStartGroup = StartGroup = altimStng;
               MetarGroup = NotIDed;
            }
            else
               MetarGroup = NotIDed;
            break;

         default:
            NDEX++;
            MetarGroup = ResetMETARGroup( StartGroup,
                                          SaveStartGroup );
            break;
      }
 
#ifdef  PRTOKEN
   printf("\n\nFROM DcdMETAR:  Print Decoded METAR...\n\n");
   prtDMETR( Mptr );
#endif
 
   }
 
                                     /******************************/
                                     /* DECODE GROUPS FOUND IN THE */
                                     /*  REMARKS SECTION OF THE    */
                                     /*       METAR REPORT         */
                                     /******************************/

#ifdef  PRTOKEN
   printf("\n\nPrior to Decision whether or "
            "not to call DcdMTRmk...\n");
   printf("\n\nFROM DcdMETAR:  Current token[%d] = %s\n\n",NDEX,
               token[NDEX]);
#endif

   if( NDEX < MAXTOKENS && ( token[NDEX] != NULL ) &&
       ( strcmp( token[NDEX], "RMK" )     == 0 ||
         strcmp( token[NDEX], "RMRK" )    == 0 ||
         strcmp( token[NDEX], "REMARK" )  == 0 ||
         strcmp( token[NDEX], "REMARKS" ) == 0 )  )
     DcdMTRmk( token, Mptr ); 

#ifdef  PRTOKEN
   printf("\n\nFollowing decision whether or "
            "not to call DcdMTRmk...\n");
   prtDMETR( Mptr );
#endif
 
                           /****************************************/
   freeTokens( token );    /* FREE THE STORAGE ALLOCATED FOR THE   */
                           /* ARRAY USED TO HOLD THE METAR REPORT  */
                           /*                GROUPS                */
                           /****************************************/
   return 0;
 
}
