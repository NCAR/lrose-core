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
#include <toolsa/toolsa_macros.h>
#include <rapformats/metar_decode.h>
#include <string.h>                                                             
#include <stdlib.h>                                                             
#include <ctype.h>                                                             
#include "metar_private.h"

/********************************************************************/
/*                                                                  */
/*  Title:         isDVR                                            */
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
 
static ubool isDVR( char *token, Decoded_METAR *Mptr, int *NDEX )
{
   char *slashPtr, *FT_ptr;
   char *vPtr;
   int length;
 
 
   if(token == NULL) return FALSE;

   if( (length = strlen( token )) < 4 )
      return FALSE;
 
   if( strncmp( token, "DVR", 3 ) != 0 )
      return FALSE;
 
   if( *(slashPtr = token+3) != '/' ) {
      (*NDEX)++;
      return FALSE;
   }
 
   if( strcmp(token+(strlen(token)-2),"FT") != 0 )
      return FALSE;
   else
      FT_ptr = token + (strlen(token)-2);
 
   if( strchr(slashPtr+1, 'P' ) != NULL )
      Mptr->DVR.above_max_DVR = TRUE;
 
   if( strchr(slashPtr+1, 'M' ) != NULL )
      Mptr->DVR.below_min_DVR = TRUE;
 
 
   if( (vPtr = strchr(slashPtr, 'V' )) != NULL )
   {
      Mptr->DVR.vrbl_visRange = TRUE;
      Mptr->DVR.Min_visRange = antoi(slashPtr+1,
                              (vPtr-(slashPtr+1)) );
      Mptr->DVR.Max_visRange = antoi(vPtr+1,
                              (FT_ptr - (vPtr+1)) );
      (*NDEX)++;
      return TRUE;
   }
   else
   {
      if( Mptr->DVR.below_min_DVR ||
          Mptr->DVR.above_max_DVR    )
         Mptr->DVR.visRange = antoi(slashPtr+2,
                           (FT_ptr - (slashPtr+2)) );
      else
         Mptr->DVR.visRange = antoi(slashPtr+1,
                           (FT_ptr - (slashPtr+1)) );
 
      (*NDEX)++;
      return TRUE;
   }
 
}
/********************************************************************/
/*                                                                  */
/*  Title:         isTornadicActiv                                  */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:   Determines whether or not the input character       */
/*              string is signals the beginning of TORNADIC         */
/*              ACTIVITY data.  If it is, then interrogate subse-   */
/*              quent report groups for time, location, and movement*/
/*              of tornado.  Return TRUE, if TORNADIC ACTIVITY is   */
/*              found.  Otherwise, return FALSE.                    */
/*                                                                  */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                                                                  */
/*  Input:         string - the address of a pointer to a charac-   */
/*                          ter string that may or may not signal   */
/*                          TORNADIC ACTIVITY.                      */
/*                                                                  */
/*                 Mptr - a pointer to a structure that has the     */
/*                        data type Decoded_METAR.                  */
/*                                                                  */
/*                 NDEX - a pointer to an integer that is the index */
/*                        into an array that contains the indi-     */
/*                        vidual groups of the METAR report being   */
/*                        decoded.  Upon entry, NDEX is the index   */
/*                        of the current group of the METAR report  */
/*                        that is to be indentified.                */
/*                                                                  */
/*  Output:        TRUE - if TORNADIC ACTIVITY is found.            */
/*                 FALSE - if no TORNADIC ACTIVITY is found.        */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
static ubool isTornadicActiv( char **string, Decoded_METAR *Mptr,
                             int *NDEX )
{
   int saveNdex,
       TornadicTime;
   ubool Completion_flag;
 
 
   if (*string == NULL) return FALSE;

   saveNdex = *NDEX;
 
   if( !( strcmp(*string, "TORNADO")         == 0 ||
          strcmp(*string, "TORNADOS")        == 0 ||
          strcmp(*string, "TORNADOES")       == 0 ||
          strcmp(*string, "WATERSPOUT")      == 0 ||
          strcmp(*string, "WATERSPOUTS")     == 0 ||
          strcmp(*string, "FUNNEL")     == 0  ) )
         return FALSE;
   else {
      if( strcmp(*string, "FUNNEL") == 0 ) {
         (++string);
	 if (*string == NULL) return FALSE;
         if( !(strcmp(*string,"CLOUD") == 0 ||
               strcmp(*string,"CLOUDS") == 0 ) ) {
            (*NDEX)++;
            return FALSE;
         }
         else
               strcpy(Mptr->TornadicType,"FUNNEL CLOUD");
      }
      else {
         strcpy(Mptr->TornadicType, *string);
         (*NDEX)++;
         (++string);
      }
 
      Completion_flag = FALSE;
 
      while( !Completion_flag ) {
	 if (*string == NULL) return FALSE;
         if( (*(*string) =='B' || *(*string) == 'E') &&
                  nisdigit( (*string)+1,strlen((*string)+1)) ) {
            TornadicTime = antoi( (*string)+1,strlen((*string)+1));
            if(*(*string) == 'B' ) {
               if( TornadicTime > 99 ) {
                  Mptr->BTornadicHour = TornadicTime/100;
                  Mptr->BTornadicMinute = TornadicTime%100;
                  (*NDEX)++;
                  (++string);
               }
               else {
                  Mptr->BTornadicMinute = TornadicTime;
                  (*NDEX)++;
                  (++string);
               }
            }
            else {
               if( TornadicTime > 99 ) {
                  Mptr->ETornadicHour = TornadicTime/100;
                  Mptr->ETornadicMinute = TornadicTime%100;
                  (*NDEX)++;
                  (++string);
               }
               else {
                  Mptr->ETornadicMinute = TornadicTime;
                  (*NDEX)++;
                  (++string);
               }
            }
         }
         else if(strcmp(*string,"DSNT")  == 0 ||
                 strcmp(*string,"VC")    == 0 ||
                 strcmp(*string,"VCY")   == 0 ) {
            if( strcmp(*string,"VCY") == 0 ||
                  strcmp(*string,"VC") == 0  ) {
               (++string);
	       if (*string == NULL) return FALSE;
               if( strcmp(*string,"STN") == 0 ){
                  strcpy(Mptr->TornadicLOC,"VC STN");
                  (*NDEX)++;
                  (*NDEX)++;
                  (++string);
               }
               else {
                  strcpy(Mptr->TornadicLOC,"VC");
                  (*NDEX)++;
               }
            }
            else {
               strcpy(Mptr->TornadicLOC,"DSNT");
               (*NDEX)++;
               (++string);
            }
         }
         else if(strcmp(*string,"N")  == 0  ||
                 strcmp(*string,"NE") == 0  ||
                 strcmp(*string,"NW") == 0  ||
                 strcmp(*string,"S")  == 0  ||
                 strcmp(*string,"SE") == 0  ||
                 strcmp(*string,"SW") == 0  ||
                 strcmp(*string,"E")  == 0  ||
                 strcmp(*string,"W")  == 0   ) {
            strcpy(Mptr->TornadicDIR, *string);
            (*NDEX)++;
            (++string);
         }
         else
            Completion_flag = TRUE;
      }
 
      if( saveNdex == *NDEX )
         return FALSE;
      else
         return TRUE;
 
   }
 
}
/********************************************************************/
/*                                                                  */
/*  Title:         isPartObscur                                     */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:   Determine whether or not the input character string */
/*              is a partial obscuration phenomenon.  If a partial  */
/*              obscuration is found, then take the preceding group */
/*              as the obscuring phenomenon.  If a partial obscura- */
/*              tion is found, then return TRUE.  Otherwise, return */
/*              false.                                              */
/*                                                                  */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                                                                  */
/*  Input:         string - the address of a pointer to a group     */
/*                          in a METAR report that may or may not   */
/*                          be a partial obscuration indicator.     */
/*                                                                  */
/*                 Mptr - a pointer to a structure that has the     */
/*                        data type Decoded_METAR.                  */
/*                                                                  */
/*                 NDEX - a pointer to an integer that is the index */
/*                        into an array that contains the indi-     */
/*                        vidual groups of the METAR report being   */
/*                        decoded.  Upon entry, NDEX is the index   */
/*                        of the current group of the METAR report  */
/*                        that is to be indentified.                */
/*                                                                  */
/*  Output:        TRUE - if the input string is a partial obscura- */
/*                        tion.                                     */
/*                 FALSE - if the input string is not a partial ob- */
/*                         scuration.                               */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
static ubool isPartObscur( char **string, Decoded_METAR *Mptr,
                          int ndex, int *NDEX )
{
 
   /***************************/
   /* DECLARE LOCAL VARIABLES */
   /***************************/
 
   int i;
 
   static char *phenom[] = {"-DZ", "DZ", "+DZ",
   "FZDZ", "-RA", "RA", "+RA",
   "SHRA", "TSRA", "FZRA", "-SN", "SN", "+SN", "DRSN", "BLSN",
   "SHSN", "TSSN", "-SG", "SG", "+SG", "IC", "-PE", "PE", "+PE",
   "SHPE", "TSPE", "GR", "SHGR", "TSGR", "GS", "SHGS", "TSGS", "-GS",
   "+GS", "TS", "VCTS", "-TSRA", "TSRA", "+TSRA", "-TSSN", "TSSN",
   "+TSSN", "-TSPE", "TSPE", "+TSPE", "-TSGS", "TSGS", "+TSGS",
   "VCSH", "-SHRA", "+SHRA", "-SHSN", "+SHSN", "-SHPE", "+SHPE",
   "-SHGS", "+SHGS", "-FZDZ", "+FZDZ", "-FZRA", "+FZRA", "FZFG",
   "+FZFG", "BR", "FG", "VCFG", "MIFG", "PRFG", "BCFG", "FU",
   "VA", "DU", "DRDU", "BLDU", "SA", "DRSA", "BLSA", "HZ",
   "BLPY", "BLSN", "+BLSN", "VCBLSN", "BLSA", "+BLSA",
   "VCBLSA", "+BLDU", "VCBLDU", "PO", "VCPO", "SQ", "FC", "+FC",
   "VCFC", "SS", "+SS", "VCSS", "DS", "+DS", "VCDS", NULL};
 
   if (*string == NULL) return FALSE;

#ifdef DEBUGXX
   printf("isPartObscur:  Routine Entered...\n");
   printf("isPartObscur:  *string = %s\n",*string);
   if( Mptr->PartialObscurationAmt[ndex][0] != '\0' ) {
      printf("PartialObscurationAmt = %s\n",
                &(Mptr->PartialObscurationAmt[ndex][0]));
      if( strcmp( *string, "FEW///" ) == 0 ||
          strcmp( *string, "SCT///" ) == 0 ||
          strcmp( *string, "BKN///" ) == 0 ||
          strcmp( *string, "FEW000" ) == 0 ||
          strcmp( *string, "SCT000" ) == 0 ||
          strcmp( *string, "BKN000" ) == 0   ) {
 
          --string;
         printf("isPartObscur:  Preceding group = %s\n",
                  *string);
         ++string;
      }
   }
#endif
 
   if (*string == NULL) return FALSE;
   if( strcmp( *string, "FEW///" ) == 0 ||
       strcmp( *string, "SCT///" ) == 0 ||
       strcmp( *string, "BKN///" ) == 0 ||
       strcmp( *string, "FEW000" ) == 0 ||
       strcmp( *string, "SCT000" ) == 0 ||
       strcmp( *string, "BKN000" ) == 0   ) {
      if( Mptr->PartialObscurationAmt[ndex][0] == '\0' )
      {
         (*NDEX)++;
         return FALSE;
      }
      else {
         if( strcmp( *string,
                     &(Mptr->PartialObscurationAmt[ndex][0]) ) == 0 )
         {
            --string;
            i = 0;
            while( phenom[i] != NULL ) {
               if( strcmp( *string, phenom[i] ) == 0 ) {
                  strcpy(&(Mptr->PartialObscurationPhenom[ndex][0]),
                         *string);
 
                  (*NDEX)++;
                  return TRUE;
               }
               else
                  i++;
            }
 
            (*NDEX)++;
            return FALSE;
 
         }
         else {
            (*NDEX)++;
            return FALSE;
         }
 
      }
 
   }
   else
      return FALSE;
 
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isA0indicator                                    */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:   Identify the input character string as an automated */
/*              station code type.  If the input character string   */
/*              is an automated station code type, then return      */
/*              TRUE.  Otherwise, return FALSE.                     */
/*                                                                  */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                                                                  */
/*  Input:         indicator - a pointer to a character string      */
/*                             that may or may not be an ASOS       */
/*                             automated station code type.         */
/*                                                                  */
/*                 Mptr - a pointer to a structure that has the     */
/*                        data type Decoded_METAR.                  */
/*                                                                  */
/*                 NDEX - a pointer to an integer that is the index */
/*                        into an array that contains the indi-     */
/*                        vidual groups of the METAR report being   */
/*                        decoded.  Upon entry, NDEX is the index   */
/*                        of the current group of the METAR report  */
/*                        that is to be indentified.                */
/*                                                                  */
/*  Output:        TRUE - if the input string matches one of the    */
/*                        valid ASOS automated station indicators.  */
/*                 FALSE - the input string did not match one of the*/
/*                        valid ASOS automated station indicators.  */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
static ubool isA0indicator( char *indicator, Decoded_METAR *Mptr,
                           int *NDEX )
{
 
   /***************************/
   /* DECLARE LOCAL VARIABLES */
   /***************************/
 
   char *autoFlag[] = {"A01", "A01A", "A02", "A02A", "AOA",
                       "A0A", "AO1", "AO1A", "AO2", "AO2A", NULL};
   int i;
 
   /*************************/
   /* START BODY OF ROUTINE */
   /*************************/
   /*******************************************/
   /* COMPARE THE INPUT CHARACTER STRING WITH */
   /* VALID AUTOMATED STATION CODE TYPE.  IF  */
   /* A MATCH IS FOUND, RETURN TRUE.  OTHER-  */
   /*           WISE, RETURN FALSE            */
   /*******************************************/
 
   if (indicator == NULL) return FALSE;
 
   i = 0;
 
   while( autoFlag[ i ] != NULL )
   {
      if( strcmp( indicator, autoFlag[ i ]) == 0 )
      {
         strcpy(Mptr->autoIndicator, indicator);
         (*NDEX)++;
         return TRUE;
      }
      i++;
   }
 
   return FALSE;
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isPeakWind                                       */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:  Determine whether or not the current and subsequent  */
/*             groups from the METAR report make up a valid report  */
/*             of peak wind.                                        */
/*                                                                  */
/*                                                                  */
/*  Input:         string - the addr of a ptr to a character string */
/*                             that may or may not be the indicator */
/*                             for a peak wind data group.          */
/*                                                                  */
/*                 Mptr - a pointer to a structure that has the     */
/*                        data type Decoded_METAR.                  */
/*                                                                  */
/*                 NDEX - a pointer to an integer that is the index */
/*                        into an array that contains the indi-     */
/*                        vidual groups of the METAR report being   */
/*                        decoded.  Upon entry, NDEX is the index   */
/*                        of the current group of the METAR report  */
/*                        that is to be indentified.                */
/*                                                                  */
/*  Output:        TRUE - if the input string (and subsequent grps) */
/*                        are decoded as peak wind.                 */
/*                 FALSE - if the input string (and subsequent grps)*/
/*                         are not decoded as peak wind.            */
/*  External Functions Called:                                      */
/*                 nisdigit                                         */
/*                                                                  */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
static ubool isPeakWind( char **string, Decoded_METAR *Mptr,
                        int *NDEX )
{
 
   /***************************/
   /* DECLARE LOCAL VARIABLES */
   /***************************/
 
   char buf[ 6 ];
   char *slash;
   int temp;
 
   /*************************/
   /* START BODY OF ROUTINE */
   /*************************/
 
   if (*string == NULL) return FALSE;
 
   /******************************************************/
   /* IF THE CURRENT AND NEXT GROUPS ARE "PK WND", THEN  */
   /* DETERMINE WHETHER OR NOT THE GROUP THAT FOLLOWS IS */
   /* A VALID PK WND GROUP.  IF IT IS, THEN DECODE THE   */
   /* GROUP AND RETURN TRUE.  OTHERWISE, RETURN FALSE.   */
   /******************************************************/
 
 
   if( strcmp(*string,"PK") != 0 ) {
      return FALSE;
   }

   string++;
   if (*string == NULL) return FALSE;
   if( strcmp(*string,"WND") != 0 ) {
     (*NDEX)++;
     return FALSE;
   }

   string++;
   if (*string == NULL) {
     (*NDEX)++;
     (*NDEX)++;
     return FALSE;
   }

   if( *string != NULL &&
       (slash = strchr(*string,'/')) == NULL ) {
     /********************************/
     /* INVALID PEAK WIND. BUMP PAST */
     /* PK AND WND GROUP AND RETURN  */
     /*             FALSE.           */
     /********************************/
     (*NDEX)++;
     (*NDEX)++;
     return FALSE;
   }

   if( strlen(*string) >= 8 && strlen(*string) <= 11 &&
       nisdigit(slash+1,strlen(slash+1)) &&
       nisdigit(*string, (slash - *string)) &&
       (slash - *string) <= 6 )
   {
      memset( buf, '\0', 4);
      strncpy( buf, *string, 3 );
      Mptr->PKWND_dir = atoi( buf );
 
      memset( buf, '\0', 4);
      strncpy( buf, *string+3, slash-(*string+3) );
      Mptr->PKWND_speed =  atoi( buf );
 
      memset( buf, '\0', 5);
      strcpy( buf, slash+1 );
      temp             =  atoi( buf );
 
      if( temp > 99 )
      {
         Mptr->PKWND_hour = atoi(buf)/100;
         Mptr->PKWND_minute = (atoi(buf)) % 100;
      }
      else
         Mptr->PKWND_minute =  atoi( buf );
                              /********************************/
                              /* VALID PEAK WIND FOUND.  BUMP */
                              /* PAST PK, WND, AND PEAK WIND  */
                              /* GROUPS AND RETURN TRUE.      */
                              /********************************/
      (*NDEX)++;
      (*NDEX)++;
      (*NDEX)++;
      return TRUE;
   }
   else
      return FALSE;
}
/********************************************************************/
/*                                                                  */
/*  Title:         isWindShift                                      */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:  Determine whether or not the current and subsequent  */
/*             groups from the METAR report make up a valid report  */
/*             of wind shift and frontal passage, if included.      */
/*                                                                  */
/*                                                                  */
/*  Input:         string - the addr of a ptr to a character string */
/*                           that may or may not be the indicator   */
/*                           for a wind shift data group.           */
/*                                                                  */
/*                 Mptr - a pointer to a structure that has the     */
/*                        data type Decoded_METAR.                  */
/*                                                                  */
/*                 NDEX - a pointer to an integer that is the index */
/*                        into an array that contains the indi-     */
/*                        vidual groups of the METAR report being   */
/*                        decoded.  Upon entry, NDEX is the index   */
/*                        of the current group of the METAR report  */
/*                        that is to be indentified.                */
/*                                                                  */
/*  Output:        TRUE - if the input string (and subsequent grps) */
/*                        are decoded as wind shift.                */
/*                 FALSE - if the input string (and subsequent grps)*/
/*                         are not decoded as wind shift.           */
/*  External Functions Called:                                      */
/*                 nisdigit                                         */
/*                                                                  */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
static ubool isWindShift( char **string, Decoded_METAR *Mptr,
                        int *NDEX)
{
 
   /***************************/
   /* DECLARE LOCAL VARIABLES */
   /***************************/
 
   int temp;
 
   /*************************/
   /* START BODY OF ROUTINE */
   /*************************/
 
   if (*string == NULL) return FALSE;
 
   /****************************************************/
   /* IF THE CURRENT GROUP IS "WSHFT", THEN DETERMINE  */
   /* WHETHER OR NOT THE GROUP THAT FOLLOWS IS A VALID */
   /* WSHFT GROUP.  IF IT IS, THEN DECODE THE GROUP    */
   /* GROUP AND RETURN TRUE.  OTHERWISE, RETURN FALSE. */
   /****************************************************/
 
   if( strcmp( *string, "WSHFT" ) != 0 )
      return FALSE;
   else
      (++string);
   if (*string == NULL) return FALSE;

   if( nisdigit(*string,strlen(*string)) && strlen(*string) <= 4)
   {
      temp = atoi( *string );
 
      if( temp > 100 )
      {
         Mptr->WshfTime_hour = (atoi(*string))/100;
         Mptr->WshfTime_minute = (atoi(*string)) % 100;
      }
      else
         Mptr->WshfTime_minute = (atoi(*string)) % 100;
 
      (++string);
      if (*string == NULL) return FALSE;
 
      if( **string == '\0') {
         (*NDEX)++;
         (*NDEX)++;
         return TRUE;
      }
      else if( strcmp( *string, "FROPA") == 0 )
      {
         Mptr->Wshft_FROPA = TRUE;
                              /********************************/
                              /* VALID WIND SHIFT FOUND. BUMP */
                              /* PAST WSHFT, WSHFT GROUP, AND */
                              /* FROPA GROUPS AND RETURN TRUE.*/
                              /********************************/
         (*NDEX)++;
         (*NDEX)++;
         (*NDEX)++;
         return TRUE;
      }
      else {
                              /********************************/
                              /* VALID WIND SHIFT FOUND. BUMP */
                              /* PAST WSHFT AND WSHFT GROUP   */
                              /*       AND RETURN TRUE.       */
                              /********************************/
         (*NDEX)++;
         (*NDEX)++;
         return TRUE;
      }
   }
   else {
                              /**********************************/
                              /* INVALID WIND SHIFT FOUND. BUMP */
                              /* PAST WSHFT AND RETURN FALSE.   */
                              /********************************/
      (*NDEX)++;
      return FALSE;
   }
}
/********************************************************************/
/*                                                                  */
/*  Title:         isTowerVsby                                      */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:  Determine whether or not the current and subsequent  */
/*             groups from the METAR report make up a valid report  */
/*             of tower visibility.                                 */
/*                                                                  */
/*                                                                  */
/*  Input:         string - the addr of a ptr to a character string */
/*                          that may or may not be the indicator    */
/*                          for tower visibility.                   */
/*                                                                  */
/*                 Mptr - a pointer to a structure that has the     */
/*                        data type Decoded_METAR.                  */
/*                                                                  */
/*                 NDEX - a pointer to an integer that is the index */
/*                        into an array that contains the indi-     */
/*                        vidual groups of the METAR report being   */
/*                        decoded.  Upon entry, NDEX is the index   */
/*                        of the current group of the METAR report  */
/*                        that is to be indentified.                */
/*                                                                  */
/*  Output:        TRUE - if the input string (and subsequent grps) */
/*                        are decoded as tower visibility.          */
/*                 FALSE - if the input string (and subsequent grps)*/
/*                         are not decoded as tower visibility      */
/*  External Functions Called:                                      */
/*                 nisdigit                                         */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
static ubool isTowerVsby( char **token, Decoded_METAR *Mptr, int *NDEX)
{
 
   /***************************/
   /* DECLARE LOCAL VARIABLES */
   /***************************/
 
   char *slash;
   float T_vsby;
 
   /*************************/
   /* START BODY OF ROUTINE */
   /*************************/
 
   if (*token == NULL) return FALSE;
 
   /****************************************************************/
   /* IF THE CURRENT AND NEXT GROUPS ARE "TWR VIS", THEN DETERMINE */
   /* WHETHER OR NOT THE GROUP(S) THAT FOLLOWS IS(ARE) A VALID     */
   /* TOWER VISIBILITY  GROUP.  IF IT IS, THEN DECODE THE GROUP    */
   /* GROUP AND RETURN TRUE.  OTHERWISE, RETURN FALSE.             */
   /****************************************************************/
 
 
   if(strcmp(*token,"TWR") != 0)
      return FALSE;
   else if( *(++token) != NULL && strcmp(*token,"VIS") != 0) {
      (*NDEX)++;
      return FALSE;
   }
    
   (++token);
   if (*token == NULL) return FALSE;
 
   if( nisdigit(*token,
              strlen(*token)))
   {
      Mptr->TWR_VSBY = (float) atoi(*token);
      (++token);
      if( *token != NULL )
      {
         if( (slash = strchr(*token, '/'))
                             != NULL )
         {
            if( nisdigit(slash+1,strlen(slash+1)) &&
                         nisdigit(*token,
                             (slash-*token)))
            {
               T_vsby = fracPart(*token);
               Mptr->TWR_VSBY += T_vsby;
               (*NDEX)++;
               (*NDEX)++;
               (*NDEX)++;
               (*NDEX)++;
               return TRUE;
            }
            else {
               (*NDEX)++;
               (*NDEX)++;
               (*NDEX)++;
               return TRUE;
            }
 
         }
         else {
            (*NDEX)++;
            (*NDEX)++;
            (*NDEX)++;
            return TRUE;
         }
      }
      else {
         (*NDEX)++;
         (*NDEX)++;
         (*NDEX)++;
         return TRUE;
      }
 
   }
   else if( (slash = strchr(*token, '/'))
                             != NULL )
   {
      if( nisdigit(slash+1,strlen(slash+1)) &&
                         nisdigit(*token,
                             (slash-*token)))
      {
         Mptr->TWR_VSBY = fracPart(*token);
         (*NDEX)++;
         (*NDEX)++;
         (*NDEX)++;
         return TRUE;
      }
      else {
         (*NDEX)++;
         (*NDEX)++;
         return FALSE;
      }
   }
   else {
      (*NDEX)++;
      (*NDEX)++;
      return FALSE;
   }

   return FALSE;
 
}

/********************************************************************/
/*                                                                  */
/*  Title:         isSurfaceVsby                                    */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:  Determine whether or not the current and subsequent  */
/*             groups from the METAR report make up a valid report  */
/*             of surface visibility.                               */
/*                                                                  */
/*                                                                  */
/*  Input:         string - the addr of a ptr to a character string */
/*                          that may or may not be the indicator    */
/*                          for surface visibility.                 */
/*                                                                  */
/*                 Mptr - a pointer to a structure that has the     */
/*                        data type Decoded_METAR.                  */
/*                                                                  */
/*                 NDEX - a pointer to an integer that is the index */
/*                        into an array that contains the indi-     */
/*                        vidual groups of the METAR report being   */
/*                        decoded.  Upon entry, NDEX is the index   */
/*                        of the current group of the METAR report  */
/*                        that is to be indentified.                */
/*                                                                  */
/*  Output:        TRUE - if the input string (and subsequent grps) */
/*                        are decoded as surface visibility.        */
/*                 FALSE - if the input string (and subsequent grps)*/
/*                         are not decoded as surface visibility.   */
/*  External Functions Called:                                      */
/*                 nisdigit                                         */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
static ubool isSurfaceVsby( char **token, Decoded_METAR *Mptr,
                           int *NDEX)
{
 
   /***************************/
   /* DECLARE LOCAL VARIABLES */
   /***************************/
 
   char *slash;
   float S_vsby;
 
 
   /*************************/
   /* START BODY OF ROUTINE */
   /*************************/
 
   if (*token == NULL) return FALSE;
 
   /****************************************************************/
   /* IF THE CURRENT AND NEXT GROUPS ARE "SFC VIS", THEN DETERMINE */
   /* WHETHER OR NOT THE GROUP(S) THAT FOLLOWS IS(ARE) A VALID     */
   /* SURFACE VISIBILITY  GROUP.  IF IT IS, THEN DECODE THE GROUP  */
   /* GROUP AND RETURN TRUE.  OTHERWISE, RETURN FALSE.             */
   /****************************************************************/
 
 
   if(strcmp(*token,"SFC") != 0)
      return FALSE;
   else if( *(++token) != NULL && strcmp(*token,"VIS") != 0) {
      (*NDEX)++;
      return FALSE;
   }
   (++token);
   if (*token == NULL) return FALSE;
 
   if( nisdigit(*token,
              strlen(*token)))
   {
      Mptr->SFC_VSBY = (float) atoi(*token);
      (++token);
      if( *token != NULL )
      {
         if( (slash = strchr(*token, '/'))
                             != NULL )
         {
            if( nisdigit(slash+1,strlen(slash+1)) &&
                         nisdigit(*token,
                             (slash-*token)))
            {
               S_vsby = fracPart(*token);
               Mptr->SFC_VSBY += S_vsby;
               (*NDEX)++;
               (*NDEX)++;
               (*NDEX)++;
               (*NDEX)++;
               return TRUE;
            }
            else {
               (*NDEX)++;
               (*NDEX)++;
               (*NDEX)++;
               return TRUE;
            }
 
         }
         else {
            (*NDEX)++;
            (*NDEX)++;
            (*NDEX)++;
            return TRUE;
         }
      }
      else {
         (*NDEX)++;
         (*NDEX)++;
         (*NDEX)++;
         return TRUE;
      }
 
   }
   else if( (slash = strchr(*token, '/'))
                             != NULL )
   {
      if( nisdigit(slash+1,strlen(slash+1)) &&
                         nisdigit(*token,
                             (slash-*token)))
      {
         Mptr->SFC_VSBY = fracPart(*token);
         (*NDEX)++;
         (*NDEX)++;
         (*NDEX)++;
         return TRUE;
      }
      else {
         (*NDEX)++;
         (*NDEX)++;
         return FALSE;
      }
   }
   else {
      (*NDEX)++;
      (*NDEX)++;
      return FALSE;
   }

   return FALSE;
 
}
/********************************************************************/
/*                                                                  */
/*  Title:         isVariableVsby                                   */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          21 Nov 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:  Determine whether or not the current and subsequent  */
/*             groups from the METAR report make up a valid report  */
/*             of variable prevailing visibility.                   */
/*                                                                  */
/*                                                                  */
/*  Input:         string - the addr of a ptr to a character string */
/*                          that may or may not be the indicator    */
/*                          for variable prevailing visibility.     */
/*                                                                  */
/*                 Mptr - a pointer to a structure that has the     */
/*                        data type Decoded_METAR.                  */
/*                                                                  */
/*                 NDEX - a pointer to an integer that is the index */
/*                        into an array that contains the indi-     */
/*                        vidual groups of the METAR report being   */
/*                        decoded.  Upon entry, NDEX is the index   */
/*                        of the current group of the METAR report  */
/*                        that is to be indentified.                */
/*                                                                  */
/*  Output:        TRUE - if the input string (and subsequent grps) */
/*                        are decoded as variable prevailing vsby.  */
/*                 FALSE - if the input string (and subsequent grps)*/
/*                         are not decoded as variable prevailing   */
/*                         vsby.                                    */
/*  External Functions Called:                                      */
/*                 nisdigit                                         */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
static ubool isVariableVsby( char **string, Decoded_METAR *Mptr,
                              int *NDEX )
{
 
   /***************************/
   /* DECLARE LOCAL VARIABLES */
   /***************************/
 
   char *slash = NULL,
        *slash1 = NULL,
        *slash2 = NULL,
        buf[ 5 ],
        *V_char;
   float minimumVsby,
         maximumVsby;
 
 
 
   /*************************/
   /* START BODY OF ROUTINE */
   /*************************/
 
   if (*string == NULL) return FALSE;
 
   /***************************************************/
   /* IF THE CURRENT GROUP IS  "VIS", THEN DETERMINE  */
   /* WHETHER OR NOT THE GROUPS THAT FOLLOW ARE VALID */
   /* VARIABLE PREVAILING VSBY.  IF THEY ARE, THEN    */
   /* DECODE THE GROUPS AND RETURN TRUE.  OTHERWISE,  */
   /* RETURN FALSE.                                   */
   /***************************************************/
 
 
 
   if( strcmp(*string, "VIS") != 0 ) {
      return FALSE;
   } else {
     (++string);
     if (*string == NULL) return FALSE;
   }

   if( !((V_char = strchr(*string, 'V')) != NULL ||
         nisdigit(*string,strlen(*string))) )
      return FALSE;
   else if( nisdigit(*string,strlen(*string)) ) {
      minimumVsby = (float) atoi(*string);
      (++string);
      if (*string == NULL) return FALSE;
      if( (V_char = strchr(*string,'V')) == NULL )
         return FALSE;
      else {
         if( (slash = strchr(*string,'/')) == NULL )
            return FALSE;
         else {
            if( nisdigit(*string,(slash - *string)) &&
                  nisdigit(slash+1,(V_char-(slash+1))) &&
                  nisdigit(V_char+1,strlen(V_char+1)) ) {
               if( (V_char - *string) > 4 )
                  return FALSE;
               else {
                  memset(buf,'\0',5);
                  strncpy(buf,*string,(V_char - *string));
                  Mptr->minVsby = minimumVsby + fracPart(buf);
                  maximumVsby = (float) atoi(V_char+1);
               }
 
               (++string);
	       if (*string == NULL) return FALSE;
 
               if( (slash = strchr(*string,'/')) == NULL ) {
                  Mptr->maxVsby = maximumVsby;
                  (*NDEX)++;
                  (*NDEX)++;
                  (*NDEX)++;
                  return TRUE;
               }
               else if( nisdigit(*string,(slash - *string)) &&
                           nisdigit(slash+1, strlen(slash+1)) ) {
                  Mptr->maxVsby = maximumVsby + fracPart(*string);
                  (*NDEX)++;
                  (*NDEX)++;
                  (*NDEX)++;
                  (*NDEX)++;
                  return TRUE;
               }
               else {
                  Mptr->maxVsby = maximumVsby;
                  (*NDEX)++;
                  (*NDEX)++;
                  (*NDEX)++;
                  return TRUE;
               }
            }
            else
               return FALSE;
         }
      }
   }
   else {
      if( (V_char = strchr(*string,'V')) == NULL )
         return FALSE;
      if(nisdigit(*string,(V_char - *string)) &&
            nisdigit(V_char+1,strlen(V_char+1)) ) {
         Mptr->minVsby = (float) antoi(*string,(V_char - *string));
         maximumVsby = (float) atoi(V_char+1);
 
         (++string);
	 if (*string == NULL) return FALSE;
 
         if( (slash = strchr(*string,'/')) == NULL ) {
            Mptr->maxVsby = maximumVsby;
            (*NDEX)++;
            (*NDEX)++;
            return TRUE;
         }
         else if( nisdigit(*string, (slash - *string)) &&
                     nisdigit(slash+1,strlen(slash+1)) ) {
            Mptr->maxVsby = maximumVsby + fracPart( *string );
            (*NDEX)++;
            (*NDEX)++;
            (*NDEX)++;
            return TRUE;
         }
         else {
            Mptr->maxVsby = maximumVsby;
            (*NDEX)++;
            (*NDEX)++;
            return TRUE;
         }
      }
      else {
	 slash1 = strchr(*string,'/');
	 slash2 = strchr(V_char+1,'/');

         if( slash2 == NULL && slash1 == NULL )
            return FALSE;
         else if( slash1 == NULL )
            return FALSE;
         else if( slash == slash2 )
            return FALSE;
         else if( nisdigit(*string,(slash1 - *string)) &&
                     nisdigit((slash1+1),(V_char-(slash1+1))) ) {
            if( (V_char - *string) > 4 )
               return FALSE;
            else {
               memset(buf,'\0',5);
               strncpy(buf,*string,(V_char - *string));
               minimumVsby = fracPart(buf);
            }
            if( slash2 == NULL) {
               if( nisdigit(V_char+1, strlen(V_char+1)) ) {
                  maximumVsby = (float) atoi(V_char+1);
 
                  (++string);
		  if (*string == NULL) return FALSE;
 
                  if( (slash = strchr(*string,'/')) == NULL ) {
                     Mptr->minVsby = minimumVsby;
                     Mptr->maxVsby = maximumVsby;
                     (*NDEX)++;
                     (*NDEX)++;
                     return TRUE;
                  }
                  else if( nisdigit(*string,(slash-*string)) &&
                         nisdigit((slash+1),strlen(slash+1)) ) {
                     Mptr->minVsby = minimumVsby;
                     Mptr->maxVsby = maximumVsby +
                                        fracPart(*string);
                     (*NDEX)++;
                     (*NDEX)++;
                     (*NDEX)++;
                     return TRUE;
                  }
                  else{
                     Mptr->minVsby = minimumVsby;
                     Mptr->maxVsby = maximumVsby;
                     (*NDEX)++;
                     (*NDEX)++;
                     return TRUE;
                  }
               }
               else
                  return FALSE;
            }
            else {
               if( nisdigit(V_char+1,(slash2-V_char+1)) &&
                     nisdigit((slash2+1),strlen(slash2+1)) ) {
                  Mptr->minVsby = minimumVsby;
                  Mptr->maxVsby = fracPart(V_char+1);
                  (*NDEX)++;
                  (*NDEX)++;
                  return TRUE;
               }
               else
                  return FALSE;
            }
         }

	 else            /* P.Neilley 7/96, Avoids infinite loop */
	   return FALSE;
      }
   }
}
/********************************************************************/
/*                                                                  */
/*  Title:         isVsby2ndSite                                    */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:  Determine whether or not the current and subsequent  */
/*             groups from the METAR report make up a valid report  */
/*             of visibility at a secondary site.                   */
/*                                                                  */
/*                                                                  */
/*  Input:         token  - the addr of a ptr to a character string */
/*                          that may or may not be the indicator    */
/*                          for visibility at a secondary site.     */
/*                                                                  */
/*                 Mptr - a pointer to a structure that has the     */
/*                        data type Decoded_METAR.                  */
/*                                                                  */
/*                 NDEX - a pointer to an integer that is the index */
/*                        into an array that contains the indi-     */
/*                        vidual groups of the METAR report being   */
/*                        decoded.  Upon entry, NDEX is the index   */
/*                        of the current group of the METAR report  */
/*                        that is to be indentified.                */
/*                                                                  */
/*  Output:        TRUE - if the input string (and subsequent grps) */
/*                        are decoded as visibility at a 2ndry site.*/
/*                 FALSE - if the input string (and subsequent grps)*/
/*                         are not decoded as visibility at a 2ndry */
/*                         site.                                    */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 nisalnum                                         */
/*                 fracPart                                         */
/*                 nisdigit                                         */
/*                                                                  */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
static ubool isVsby2ndSite( char **token, Decoded_METAR *Mptr,
                           int *NDEX)
{
 
   /***************************/
   /* DECLARE LOCAL VARIABLES */
   /***************************/
 
   char *slash;
   float S_vsby,
         VSBY_2ndSite;
 
 
   /*************************/
   /* START BODY OF ROUTINE */
   /*************************/
 
   if (*token == NULL) return FALSE;
 
   /***************************************************/
   /* IF THE CURRENT GROUP IS  "VIS", THEN DETERMINE  */
   /* WHETHER OR NOT THE GROUPS THAT FOLLOW ARE VALID */
   /* VSBILITY AT A 2NDRY SITE.  IF THEY ARE, THEN    */
   /* DECODE THE GROUPS AND RETURN TRUE.  OTHERWISE,  */
   /* RETURN FALSE.                                   */
   /***************************************************/
 
 
 
   if(strcmp(*token,"VIS") != 0) {
     return FALSE;
   } else {
     (++token);
     if (*token == NULL) return FALSE;
   }
 
 
   if( nisdigit(*token, strlen(*token))) {

     VSBY_2ndSite = (float) atoi(*token);
     (++token);
     if (*token == NULL) return FALSE;

     if( (slash = strchr(*token, '/')) != NULL ) {

       if( nisdigit(slash+1,strlen(slash+1)) &&
	   nisdigit(*token, (slash-*token))) {

	 S_vsby = fracPart(*token);
	 (++token);
	 if (*token == NULL) return FALSE;

	 if( strncmp( *token, "RWY", 3 ) == 0) {
	   if( nisalnum( *token, strlen(*token) ) ) {
	     strcpy(Mptr->VSBY_2ndSite_LOC, *token);
	     Mptr->VSBY_2ndSite = VSBY_2ndSite + S_vsby;
	     (*NDEX)++;
	     (*NDEX)++;
	     (*NDEX)++;
	     (*NDEX)++;
	     return TRUE;
	   } else {
	     return FALSE;
	   }

	 } else {
	   return FALSE;
	 }

       } else {

	 if( strncmp( *token, "RWY", 3 ) == 0) {

	   if( nisalnum( *token, strlen(*token) ) ) {
	     strcpy(Mptr->VSBY_2ndSite_LOC, *token);
	     Mptr->VSBY_2ndSite = VSBY_2ndSite;
	     (*NDEX)++;
	     (*NDEX)++;
	     (*NDEX)++;
	     return TRUE;
	   } else {
	     return FALSE;
	   }

	 } else {
	   return FALSE;
	 }

       } /* if( nisdigit(slash+1,strlen(slash+1)) ... */
 
     } else { /* if( (slash = strchr(*token, '/')) != NULL ) */

       if( strncmp( *token, "RWY", 3 ) == 0) {

	 if( nisalnum( *token, strlen(*token) ) ) {
	   strcpy(Mptr->VSBY_2ndSite_LOC, *token);
	   Mptr->VSBY_2ndSite = VSBY_2ndSite;
	   (*NDEX)++;
	   (*NDEX)++;
	   (*NDEX)++;
	 } else {
	   (*NDEX)++;
	 }
	 return TRUE;
       } else {
	 return FALSE;
       }

     } /* if( (slash = strchr(*token, '/')) != NULL ) */
     
   } else if( (slash = strchr(*token, '/')) != NULL ) {

     if( nisdigit(slash+1,strlen(slash+1)) &&
	 nisdigit(*token, (slash-*token))) {
       
       VSBY_2ndSite = fracPart(*token);
       if (*(++token) == NULL) return FALSE;
       if( strncmp( *token, "RWY", 3 ) == 0) {
	 if( nisalnum( *token, strlen(*token) ) ) {
	   Mptr->VSBY_2ndSite = VSBY_2ndSite;
	   strcpy(Mptr->VSBY_2ndSite_LOC, *token);
	   (*NDEX)++;
	   (*NDEX)++;
	   (*NDEX)++;
	   return TRUE;
	 } else {
	   return FALSE;
	 }
       } else {
	 return FALSE;
       }
     } else {
       return FALSE;
     }
   } else {
     return FALSE;
   }

   return FALSE;

}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isLTGfreq                                        */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:  Determine whether or not the current and subsequent  */
/*             groups from the METAR report make up a valid report  */
/*             of lightning.                                        */
/*                                                                  */
/*                                                                  */
/*  Input:        string  - the addr of a ptr to a character string */
/*                          that may or may not be the indicator    */
/*                          for lightning.                          */
/*                                                                  */
/*                 Mptr - a pointer to a structure that has the     */
/*                        data type Decoded_METAR.                  */
/*                                                                  */
/*                 NDEX - a pointer to an integer that is the index */
/*                        into an array that contains the indi-     */
/*                        vidual groups of the METAR report being   */
/*                        decoded.  Upon entry, NDEX is the index   */
/*                        of the current group of the METAR report  */
/*                        that is to be indentified.                */
/*                                                                  */
/*  Output:        TRUE - if the input string (and subsequent grps) */
/*                        are decoded as lightning.                 */
/*                 FALSE - if the input string (and subsequent grps)*/
/*                         are not decoded as lightning.            */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 NONE.                                            */
/*                                                                  */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
ubool static isLTGfreq( char **string, Decoded_METAR *Mptr, int *NDEX )
{
 
   /***************************/
   /* DECLARE LOCAL VARIABLES */
   /***************************/
 
   ubool LTG_FREQ_FLAG,
        LTG_TYPE_FLAG,
        LTG_LOC_FLAG,
        LTG_DIR_FLAG,
        SkipFlag;
 
   /*************************/
   /* START BODY OF ROUTINE */
   /*************************/
 
   if (*string == NULL) return FALSE;
 
   /***************************************************/
   /* IF THE CURRENT GROUP IS  "LTG", THEN DETERMINE  */
   /* WHETHER OR NOT THE PREVIOUS GROUP AS WELL AS    */
   /* GROUPS THAT FOLLOW ARE VALID LIGHTNING REPORT   */
   /* PARAMETERS.  IF THEY ARE, THEN DECODE THE       */
   /* GROUPS AND RETURN TRUE.  OTHERWISE, RETURN      */
   /*                   FALSE.                        */
   /***************************************************/
   
   if( strcmp( *string, "LTG" ) != 0 &&
                 strstr(*string,"LTG") == NULL) {
      return FALSE;
   }
   else {
      (--string);
 
 
      LTG_FREQ_FLAG = FALSE;
                        /*-- CHECK FOR LIGHTNING FREQUENCY -----------*/
      if( strcmp( *string, "OCNL" ) == 0 ) {
         Mptr->OCNL_LTG = TRUE;
         LTG_FREQ_FLAG = TRUE;
      }
      else if( strcmp( *string, "FRQ" ) == 0 ) {
         Mptr->FRQ_LTG = TRUE;
         LTG_FREQ_FLAG = TRUE;
      }
      else if( strcmp( *string, "CNS" ) == 0 ) {
         Mptr->CNS_LTG = TRUE;
         LTG_FREQ_FLAG = TRUE;
      }
 
 
      (++string);
      if (*string == NULL) return FALSE;
 
      SkipFlag = FALSE;
 
      if( strcmp(*string,"LTG") == 0 ) {
         SkipFlag = TRUE;
         (++string);
	 if (*string == NULL) return FALSE;
      }

      LTG_TYPE_FLAG = FALSE;
                        /*-- CHECK FOR LIGHTNING TYPE ----------------*/
      if( strstr( *string, "CG" ) != NULL ) {
         Mptr->CG_LTG = TRUE;
         LTG_TYPE_FLAG = TRUE;
      }
      if( strstr( *string, "IC" ) != NULL ) {
         Mptr->IC_LTG = TRUE;
         LTG_TYPE_FLAG = TRUE;
      }
      if( strstr( *string, "CC" ) != NULL ) {
         Mptr->CC_LTG = TRUE;
         LTG_TYPE_FLAG = TRUE;
      }
      if( strstr( *string, "CA" ) != NULL ) {
         Mptr->CA_LTG = TRUE;
         LTG_TYPE_FLAG = TRUE;
      }
 
      if( LTG_TYPE_FLAG ) {
	(++string);
	if (*string == NULL) return FALSE;
      }
 

      LTG_LOC_FLAG = FALSE;
      if ( *string != NULL ) {
                        /*-- CHECK FOR LIGHTNING LOCATION ------------*/
      if( strcmp( *string, "DSNT" ) == 0 ) {
         Mptr->DSNT_LTG = TRUE;
         LTG_LOC_FLAG = TRUE;
      }
      else if( strcmp( *string, "AP" ) == 0 ) {
         Mptr->AP_LTG = TRUE;
         LTG_LOC_FLAG = TRUE;
      }
      else if( strcmp( *string, "OVHD" ) == 0 ) {
         Mptr->OVHD_LTG = TRUE;
         LTG_LOC_FLAG = TRUE;
      }
      else if( strcmp( *string, "VCY" ) == 0 ) {
         (++string);
	 if (*string == NULL) return FALSE;
	 if( strcmp( *string, "STN" ) == 0 ) {
            Mptr->VcyStn_LTG = TRUE;
            LTG_LOC_FLAG = TRUE;
         }
      }
    }

      if( LTG_LOC_FLAG ) {
	(++string);
	if (*string == NULL) return FALSE;
      }
 
      LTG_DIR_FLAG = FALSE;
                        /*-- CHECK FOR LIGHTNING DIRECTION -----------*/

      if ( *string != NULL ) {

      if( strcmp( *string, "N" ) == 0 ||
             strcmp( *string, "NE" ) == 0 ||
             strcmp( *string, "NW" ) == 0 ||
             strcmp( *string, "S" ) == 0 ||
             strcmp( *string, "SE" ) == 0 ||
             strcmp( *string, "SW" ) == 0 ||
             strcmp( *string, "E" ) == 0 ||
             strcmp( *string, "W" ) == 0    ) {
         strcpy( Mptr->LTG_DIR, *string);
         LTG_DIR_FLAG = TRUE;
      }

    }
 
      if( SkipFlag )
         (*NDEX)++;
 
      if( LTG_TYPE_FLAG )
         (*NDEX)++;
      if( LTG_LOC_FLAG ) {
         (*NDEX)++;
         if( Mptr->VcyStn_LTG )
            (*NDEX)++;
      }
      if( LTG_DIR_FLAG )
         (*NDEX)++;

      if ( !SkipFlag && !LTG_TYPE_FLAG && !LTG_LOC_FLAG && !LTG_DIR_FLAG ) 
         (*NDEX)++;
 
      return TRUE;
   }
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isRecentWx                                       */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:  Determine whether or not the current and subsequent  */
/*             groups from the METAR report make up a valid report  */
/*             recent weather.                                      */
/*                                                                  */
/*  Input:         token  - the addr of a ptr to a character token */
/*                          that may or may not be a recent weather */
/*                          group.                                  */
/*                                                                  */
/*                 Mptr - a pointer to a structure that has the     */
/*                        data type Decoded_METAR.                  */
/*                                                                  */
/*                 NDEX - a pointer to an integer that is the i*NDEX */
/*                        into an array that contains the indi-     */
/*                        vidual groups of the METAR report being   */
/*                        decoded.  Upon entry, NDEX is the i*NDEX   */
/*                        of the current group of the METAR report  */
/*                        that is to be indentified.                */
/*                                                                  */
/*  Output:        TRUE - if the input token (and possibly subse-  */
/*                        quent groups) are decoded as recent wx.   */
/*                 FALSE - if the input token (and possibly subse- */
/*                         quent groups) are not decoded as recent  */
/*                         wx.                                      */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 nisdigit                                         */
/*                                                                  */
/*  Input:         x                                                */
/*                                                                  */
/*  Output:        x                                                */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
static ubool isRecentWX( char **token, Decoded_METAR *Mptr,
                        int *NDEX )
{
   static char *phenom[] = {"-DZB", "DZB", "+DZB",
   "FZDZB", "-RAB", "RAB", "+RAB",
   "SHRAB", "TSRAB", "FZRAB", "-SNB",
   "SNB", "+SNB", "DRSNB", "BLSNB",
   "SHSNB", "TSSNB", "-SGB", "SGB",
   "+SGB", "ICB", "-PEB", "PEB", "+PEB",
   "SHPEB", "TSPEB", "GRB", "SHGRB",
   "TSGRB", "GSB", "SHGSB", "TSGSB", "-GSB", "UPB", "-UPB", "+UPB",
   "+GSB", "TSB", "VCTSB", "-TSRAB",
   "TSRAB", "+TSRAB", "-TSSNB", "TSSNB",
   "+TSSNB", "-TSPEB", "TSPEB", "+TSPEB",
   "-TSGSB", "TSGSB", "+TSGSB",
   "VCSHB", "-SHRAB", "+SHRAB", "-SHSNB",
   "+SHSNB", "-SHPEB", "+SHPEB",
   "-SHGSB", "+SHGSB", "-FZDZB", "+FZDZB",
   "-FZRAB", "+FZRAB", "FZFGB",
   "+FZFGB", "BRB", "FGB", "VCFGB", "MIFGB",
   "PRFGB", "BCFGB", "FUB",
   "VAB", "DUB", "DRDUB", "BLDUB", "SAB",
   "DRSAB", "BLSAB", "HZB",
   "BLPYB", "BLSNB", "+BLSNB", "VCBLSNB",
   "BLSAB", "+BLSAB",
   "VCBLSAB", "+BLDUB", "VCBLDUB", "POB",
   "VCPOB", "SQB", "FCB", "+FCB",
   "VCFCB", "SSB", "+SSB", "VCSSB", "DSB",
   "+DSB", "VCDSB",
 
 
   "-DZE", "DZE", "+DZE",
   "FZDZE", "-RAE", "RAE", "+RAE",
   "SHRAE", "TSRAE", "FZRAE", "-SNE",
   "SNE", "+SNE", "DRSNE", "BLSNE",
   "SHSNE", "TSSNE", "-SGE", "SGE",
   "+SGE", "ICE", "-PEE", "PEE", "+PEE",
   "SHPEE", "TSPEE", "GRE", "SHGRE", "UPE", "-UPE", "+UPE",
   "TSGRE", "GSE", "SHGSE", "TSGSE", "-GSE",
   "+GSE", "TSE", "VCTSE", "-TSRAE",
   "TSRAE", "+TSRAE", "-TSSNE", "TSSNE",
   "+TSSNE", "-TSPEE", "TSPEE", "+TSPEE",
   "-TSGSE", "TSGSE", "+TSGSE",
   "VCSHE", "-SHRAE", "+SHRAE", "-SHSNE",
   "+SHSNE", "-SHPEE", "+SHPEE",
   "-SHGSE", "+SHGSE", "-FZDZE", "+FZDZE",
   "-FZRAE", "+FZRAE", "FZFGE",
   "+FZFGE", "BRE", "FGE", "VCFGE", "MIFGE",
   "PRFGE", "BCFGE", "FUE",
   "VAE", "DUE", "DRDUE", "BLDUE", "SAE",
   "DRSAE", "BLSAE", "HZE",
   "BLPYE", "BLSNE", "+BLSNE", "VCBLSNE",
   "BLSAE", "+BLSAE",
   "VCBLSAE", "+BLDUE", "VCBLDUE", "POE",
   "VCPOE", "SQE", "FCE", "+FCE",
   "VCFCE", "SSE", "+SSE", "VCSSE", "DSE",
   "+DSE", "VCDSE", NULL};
 
   int i,
       beg_hour,
       beg_min,
       end_hour,
       end_min;
 
   char *temp,
        *savetemp,
        *numb_char,
        *C_char;
 
   if (*token == NULL) return FALSE;

#ifdef INSPECT
   printf("isRecentWx routine entered...\n");
   __ctest(NULL);
#endif
 
/* printf("\n\n>>>  isRecentWx Entered w/token value...  <<<\n\n");*/
/* printf("TOKEN = %s\n",*token);  */
   if( (savetemp = temp = (char *) calloc(1,sizeof(char) *
             (strlen(*token) + 1))) == NULL ) {
      printf("isRecentWx:  storage allocation failed\n");
      return FALSE;
   }
/* printf("isRecentWx:  storage allocation successful\n"); */
   strcpy(temp,*token);
/* printf("isRecentWx:  strcpy complete...\n");  */
/* printf("\n temp = %s\n\n", temp );            */
 
   while ( *temp != '\0'  && *NDEX < 3 ) {
/*    printf("\n\n IN isRecentWx\n\n"); */
/*    printf("\n temp = %s\n\n", temp ); */
      i = 0;
 
      beg_hour = beg_min = end_hour = end_min = MAXINT;
 
      while( phenom[i] != NULL )
         if( strncmp(temp, phenom[i],strlen(phenom[i])) != 0 )
            i++;
         else
            break;
 
      if( phenom[i] != NULL ) {
 
/*       printf("PHENOM = %s\n",phenom[i]);  */
 
         C_char = (strlen(phenom[i]) - 1) + temp;
         numb_char = C_char + 1;
         if( nisdigit(numb_char,4) && strlen(numb_char) >= 4) {
            if( *C_char == 'B' ) {
               beg_hour = antoi( numb_char, 2 );
               beg_min = antoi( numb_char+2,2 );
/*             printf("BEG_HOUR1 = %d\n",beg_hour);
               printf("BEG_MIN1  = %d\n",beg_min );  */
               temp = numb_char+4;
 
               if( *NDEX < 3 ) {
                  Mptr->ReWx[*NDEX].Bmm = beg_min;
                  Mptr->ReWx[*NDEX].Bhh = beg_hour;
               }
 
               if( *(numb_char+4) == 'E' ) {
                  numb_char += 5;
                  if( nisdigit(numb_char,4) &&
                              strlen(numb_char) >= 4 ) {
                     end_hour = antoi( numb_char, 2 );
                     end_min = antoi( numb_char+2,2 );
/*                   printf("END_HOUR2 = %d\n",end_hour);
                     printf("END_MIN2  = %d\n",end_min );  */
                     temp = numb_char+4;
 
                     if( *NDEX < 3 ) {
                        Mptr->ReWx[*NDEX].Emm = end_min;
                        Mptr->ReWx[*NDEX].Ehh = end_hour;
                     }
                  }
                  else if( nisdigit(numb_char,2) &&
                            strlen(numb_char) >= 2 ) {
                     end_min = antoi( numb_char,2 );
 
                     if( *NDEX < 3 ) {
                        Mptr->ReWx[*NDEX].Emm = end_min;
/*                      printf("END_MIN3  = %d\n",end_min );*/
                     }
                     temp = numb_char+2;
                  }
 
               }
 
               if( *NDEX < 3 )
                  strncpy(Mptr->ReWx[*NDEX].Recent_weather,
                             phenom[i], (strlen(phenom[i])-1) );
               (*NDEX)++;
/*             free( temp );   */
/*             return TRUE;    */
 
            }
            else {
               end_hour = antoi( numb_char, 2 );
               end_min = antoi( numb_char+2,2 );
 
               if( *NDEX < 3 ) {
                  Mptr->ReWx[*NDEX].Emm = end_min;
                  Mptr->ReWx[*NDEX].Ehh = end_hour;
 
/*                printf("END_HOUR4 = %d\n",end_hour);
                  printf("END_MIN4  = %d\n",end_min );  */
               }
 
               temp = numb_char+4;
 
               if( *(numb_char+4) == 'B' ) {
                  numb_char += 5;
                  if( nisdigit(numb_char,4) &&
                             strlen(numb_char) >= 4 ) {
/*                   beg_hour = antoi( numb_char, 2 );
                     beg_min = antoi( numb_char+2,2 );  */
 
                     if( *NDEX < 3 ) {
                        Mptr->ReWx[*NDEX].Bmm = beg_min;
                        Mptr->ReWx[*NDEX].Bhh = beg_hour;
 
/*                      printf("BEG_HOUR5 = %d\n",beg_hour);
                        printf("BEG_MIN5  = %d\n",beg_min );  */
                     }
 
                     temp = numb_char+4;
                  }
                  else if( nisdigit(numb_char,2) &&
                           strlen(numb_char) >= 2 ) {
                     beg_min = antoi( numb_char,2 );
 
                     if( *NDEX < 3 ) {
                        Mptr->ReWx[*NDEX].Bmm = beg_min;
/*                      printf("BEG_MIN6  = %d\n",beg_min );  */
                     }
 
                     temp = numb_char+2;
                  }
 
               }
 
                  if( *NDEX < 3 )
                     strncpy(Mptr->ReWx[*NDEX].Recent_weather,
                             phenom[i], (strlen(phenom[i])-1) );
                  (*NDEX)++;
/*                free( temp );   */
/*                return TRUE;    */
 
            }
 
         }
         else if(nisdigit(numb_char,2) && strlen(numb_char) >= 2 ) {
            if( *C_char == 'B' ) {
               beg_min = antoi( numb_char,2 );
 
               if( *NDEX < 3 ) {
                  Mptr->ReWx[*NDEX].Bmm = beg_min;
/*                printf("BEG_MIN7  = %d\n",beg_min );  */
               }
 
               temp = numb_char+2;
 
               if( *(numb_char+2) == 'E' ) {
                  numb_char += 3;
                  if( nisdigit(numb_char,4) &&
                           strlen(numb_char) >= 4 ) {
                     end_hour = antoi( numb_char,2 );
                     end_min = antoi( numb_char+2,2 );
 
                     if( *NDEX < 3 ) {
                        Mptr->ReWx[*NDEX].Emm = end_min;
                        Mptr->ReWx[*NDEX].Ehh = end_hour;
 
/*                      printf("END_HOUR8 = %d\n",end_hour);
                        printf("END_MIN8  = %d\n",end_min );  */
                     }
 
                     temp = numb_char+4;
                  }
                  else if( nisdigit(numb_char,2) &&
                             strlen(numb_char) >= 2 ) {
                     end_min = antoi( numb_char,2 );
 
                     if( *NDEX <= 3 ) {
                        Mptr->ReWx[*NDEX].Emm = end_min;
/*                      printf("END_MIN9  = %d\n",end_min );  */
                     }
 
                     temp = numb_char+2;
                  }
               }
            }
            else {
               end_min = antoi( numb_char, 2 );
 
               if( *NDEX < 3 ) {
                  Mptr->ReWx[*NDEX].Emm = end_min;
/*                printf("END_MIN10  = %d\n",end_min );  */
               }
 
               temp = numb_char+2;
 
               if( *(numb_char+2) == 'B' ) {
                  numb_char += 3;
                  if( nisdigit(numb_char,4) &&
                               strlen(numb_char) >= 4 ) {
                     beg_hour = antoi( numb_char,2 );
                     beg_min = antoi( numb_char+2,2 );
 
                     if( *NDEX < 3 ) {
                        Mptr->ReWx[*NDEX].Bmm = beg_min;
                        Mptr->ReWx[*NDEX].Bhh = beg_hour;
 
/*                      printf("BEG_HOUR11 = %d\n",beg_hour);
                        printf("BEG_MIN11  = %d\n",beg_min ); */
                     }
 
                     temp = numb_char+4;
                  }
                  else if( nisdigit(numb_char,2) &&
                             strlen(numb_char) >= 2 ) {
                     beg_min = antoi( numb_char,2 );
 
                     if( *NDEX < 3 ) {
                        Mptr->ReWx[*NDEX].Bmm = beg_min;
/*                      printf("BEG_MIN12  = %d\n",beg_min );  */
                     }
 
                     temp = numb_char+2;
                  }
 
               }
 
            }
 
            if( *NDEX <= 3 )
               strncpy(Mptr->ReWx[*NDEX].Recent_weather,
                       phenom[i], (strlen(phenom[i])-1) );
 
            (*NDEX)++;
/*          free( temp );  */
/*          return TRUE;   */
 
         }

         else
	    (*NDEX)++;  /* PNeilley 7/8/96, eliminated infinite loop */ 

      }
      else {
         free( savetemp );
         return FALSE;
      }
 
   }
   free ( savetemp );
 
   if( *NDEX >= 3 )
      return FALSE;
   else
      return TRUE;
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isVariableCIG                                    */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          21 Nov 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:      isVariableCIG determines whether or not the      */
/*                 current group in combination with the next       */
/*                 one or more groups is a report of variable       */
/*                 ceiling.                                         */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 nisdigit                                         */
/*                                                                  */
/*  Input:         token - a pointer to an array of METAR report    */
/*                           groups.                                */
/*                 Mptr - a pointer to a decoded_METAR structure    */
/*                 NDEX - the index value of the current METAR      */
/*                        report group array element.               */
/*                                                                  */
/*  Output:        TRUE, if the token is currently pointing to      */
/*                 METAR report group(s) that a report of vari-     */
/*                 ble ceiling.                                     */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
static ubool isVariableCIG( char **token, Decoded_METAR *Mptr,
                           int *NDEX )
{
   char *V_char;
 
   if (*token == NULL) return FALSE;

   if( strcmp(*token, "CIG") != 0 ) {
      return FALSE;
   } else {
     (++token);
     if (*token == NULL) return FALSE;
   }
 
   if( (V_char = strchr(*token,'V')) != NULL ) {
      if( nisdigit(*token, (V_char - *token)) &&
            nisdigit( V_char+1, strlen(V_char+1)) ) {
         Mptr->minCeiling = antoi(*token, (V_char - *token));
         Mptr->maxCeiling = atoi(V_char+1);
 
         (*NDEX)++;
         (*NDEX)++;
         return TRUE;
      }
      else
         return FALSE;
   }
   else
      return FALSE;
}
/********************************************************************/
/*                                                                  */
/*  Title:         isCeil2ndSite                                    */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:      isCeil2ndSite determines whether or not the      */
/*                 current group in combination with the next       */
/*                 one or more groups is a report of a ceiling      */
/*                 at a secondary site.                             */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 nisdigit                                         */
/*                                                                  */
/*  Input:         token - a pointer to an array of METAR report    */
/*                           groups.                                */
/*                 Mptr - a pointer to a decoded_METAR structure    */
/*                 NDEX - the index value of the current METAR      */
/*                        report group array element.               */
/*                                                                  */
/*  Output:        TRUE, if the token is currently pointing to      */
/*                 METAR report group(s) that are reporting         */
/*                 ceiling at a secondary site.                     */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 nisdigit                                         */
/*                                                                  */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
static ubool isCIG2ndSite( char **token, Decoded_METAR *Mptr,
                           int *NDEX)
{
   int CIG2ndSite;
 
   if (*token == NULL) return FALSE;

   if(strcmp(*token,"CIG") != 0) {
     return FALSE;
   } else {
     (++token);
     if (*token == NULL) return FALSE;
   }
 
 
   if( strlen(*token) != 3 )
      return FALSE;
 
   if( nisdigit(*token,3) )
   {
      CIG2ndSite = atoi(*token ) * 10;
 
      if (*(++token) == NULL) return FALSE;
      if( strncmp(*token,"RWY",3) != 0)
         return FALSE;
      else {
         strcpy(Mptr->CIG_2ndSite_LOC, *token );
         Mptr->CIG_2ndSite_Meters = CIG2ndSite;
         (*NDEX)++;
         (*NDEX)++;
         (*NDEX)++;
         return TRUE;
      }
   }
   else
     return FALSE;
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isPRESFR                                         */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          20 Nov 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:      x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Input:         x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Output:        x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
static ubool isPRESFR( char *string, Decoded_METAR *Mptr, int *NDEX)
{
 
   if (string == NULL) return FALSE;
   if( strcmp(string, "PRESFR") != 0 )
      return FALSE;
   else {
      Mptr->PRESFR = TRUE;
      (*NDEX)++;
      return TRUE;
   }
 
}
/********************************************************************/
/*                                                                  */
/*  Title:         isPRESRR                                         */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          20 Nov 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:      x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Input:         x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Output:        x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
static ubool isPRESRR( char *string, Decoded_METAR *Mptr, int *NDEX)
{
 
   if (string == NULL) return FALSE;
   if( strcmp(string, "PRESRR") != 0 )
      return FALSE;
   else {
      Mptr->PRESRR = TRUE;
      (*NDEX)++;
      return TRUE;
   }
 
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isSLP                                            */
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
 
static ubool isSLP( char **token, Decoded_METAR *Mptr, int *NDEX )
{
 
   /***************************/
   /* DECLARE LOCAL VARIABLES */
   /***************************/
 
   int pressure;
 
   /*************************/
   /* BEGIN BODY OF ROUTINE */
   /*************************/
 
   if (*token == NULL) return FALSE;

   if( strcmp(*token, "SLPNO") == 0 ) {
      Mptr->SLPNO = TRUE;
      (*NDEX)++;
      return TRUE;
   }
 
 
   if( strncmp(*token, "SLP", 3) != 0 ) 
      return FALSE;

   else
   {
      if( strncmp(*token, "SLP", 3) == 0 &&
                  strcmp(*token,"SLP") != 0 )
      {
         if( nisdigit( *token+3, 3) )
         {
            pressure = atoi(*token+3);
 
            if(pressure >= 550 )
               Mptr->SLP = ((float) pressure)/10. + 900.;
            else
               Mptr->SLP = ((float) pressure)/10. + 1000.;
            (*NDEX)++;
            return TRUE;
         }
         else
            return FALSE;
      }
      else
      {
         ++token;
	 if (*token == NULL) return FALSE;
         if( nisdigit( *token, 3) )
         {
            pressure = atoi(*token);
 
            if(pressure >= 550 )
               Mptr->SLP = ((float) pressure)/10. + 900.;
            else
               Mptr->SLP = ((float) pressure)/10. + 1000.;
 
            (*NDEX)++;
            (*NDEX)++;
            return TRUE;
         }
         else
            return FALSE;
      }
 
   }
 
}
static ubool isSectorVsby( char **string, Decoded_METAR *Mptr,
                          int  *NDEX )
{
   /***************************/
   /* DECLARE LOCAL VARIABLES */
   /***************************/
 
   float vsby = 0;
   char  dd[3],
         *slash;
 
   /*************************/
   /* START BODY OF ROUTINE */
   /*************************/
 
   memset( dd, '\0', 3 );
 
   if (*string == NULL) return FALSE;

   if( strcmp(*string, "VIS") != 0 ) {
      return FALSE;
   }

   ++string;
   if (*string == NULL) return FALSE;
 
   if( strncmp(*string,"NE", 2) == 0 ) {
     strncpy(dd,*string,2);
     ++string;
   }
   else if( strncmp(*string,"SE",2) == 0 ) {
     strncpy(dd,*string,2);
     ++string;
   }
   else if( strncmp(*string,"NW",2) == 0 ) {
     strncpy(dd,*string,2);
     ++string;
   }
   else if( strncmp(*string,"SW",2) == 0 ) {
     strncpy(dd,*string,2);
     ++string;
   }
   else if( strncmp(*string,"N",1) == 0 ) {
     strncpy(dd,*string,1);
     ++string;
   }
   else if( strncmp(*string,"E",1) == 0 ) {
     strncpy(dd,*string,1);
     ++string;
   }
   else if( strncmp(*string,"S",1) == 0 ) {
     strncpy(dd,*string,1);
     ++string;
   }
   else if( strncmp(*string,"W",1) == 0 ) {
     strncpy(dd,*string,1);
     ++string;
   } else {
     return FALSE;
   }

/* PNeilley 11/01/96.  Prevents failure if dist not in same token as dir */
   
   if ( *string == NULL ) return FALSE; 

/* End of mods */

   if(nisdigit(*string,strlen(*string))) {
     vsby = atoi(*string);
     (++string);
     if ( *string == NULL ) return FALSE; 
   }

   if( (slash = strchr(*string,'/')) == NULL ) {
     strcpy(Mptr->SectorVsby_Dir,dd);
     Mptr->SectorVsby = vsby;
     (*NDEX)++;
     (*NDEX)++;
     return TRUE;
   } else if( nisdigit(*string,(slash-*string)) &&
	      nisdigit(slash+1,strlen(slash+1)) ) {
     vsby += fracPart(*string);
     strcpy( Mptr->SectorVsby_Dir, dd );
     Mptr->SectorVsby = vsby;
     (*NDEX)++;
     (*NDEX)++;
     (*NDEX)++;
     return TRUE;
   } else {
     return FALSE;
   }

   return FALSE;
 
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isGR                                             */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          20 Nov 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:      x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Input:         x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Output:        x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
static ubool isGR( char **string, Decoded_METAR *Mptr, int *NDEX)
{
   char *slash;
 
   if (*string == NULL) return FALSE;

   if( strcmp(*string, "GS") == 0 ) {
      Mptr->GR = TRUE;
      (*NDEX)++;
      return TRUE;
   }
 
 
   if( strcmp(*string, "GR") != 0 ) {
     return FALSE;
   }

   (++string);
   if (*string == NULL) return FALSE;
 
   if( (slash = strchr( *string, '/' )) != NULL ) {
     if( strcmp( *string, "M1/4" ) == 0 ) {
       Mptr->GR_Size = 1./8.;
       Mptr->GR = TRUE;
       (*NDEX)++;
       (*NDEX)++;
       return TRUE;
     } else if( nisdigit( *string, (slash - *string) ) &&
		nisdigit( slash+1, strlen(slash+1)) ) {
       Mptr->GR_Size = fracPart( *string );
       Mptr->GR = TRUE;
       (*NDEX)++;
       (*NDEX)++;
       return TRUE;
     } else {
       Mptr->GR = TRUE;
       (*NDEX)++;
       return TRUE;
     }

   } else if( nisdigit( *string, strlen(*string) ) ) {

     Mptr->GR_Size = antoi( *string, strlen(*string) );
     Mptr->GR = TRUE;
 
     (++string);
     if (*string == NULL) return FALSE;

     if( (slash = strchr( *string, '/' )) != NULL ) {

       if( nisdigit( *string, (slash - *string) ) &&
	   nisdigit( slash+1, strlen(slash+1)) ) {

	 Mptr->GR_Size += fracPart( *string );
	 (*NDEX)++;
	 (*NDEX)++;
	 (*NDEX)++;
	 return TRUE;
       } else {
	 (*NDEX)++;
	 (*NDEX)++;
	 return TRUE;
       }
     } else {
       (*NDEX)++;
       (*NDEX)++;
       return TRUE;
     }

   } else {
     Mptr->GR = TRUE;
     (*NDEX)++;
     return TRUE;
   }

}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isVIRGA                                          */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          20 Nov 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:      x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Input:         x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Output:        x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
static ubool isVIRGA( char **string, Decoded_METAR *Mptr, int *NDEX)
{
 
   if (*string == NULL) return FALSE;

   if( strcmp(*string, "VIRGA") != 0 )
      return FALSE;
   else {
      Mptr->VIRGA = TRUE;
      (*NDEX)++;
 
      (++string);
      if (*string == NULL) return FALSE;
 
      if( strcmp( *string, "N" ) == 0 ||
          strcmp( *string, "S" ) == 0 ||
          strcmp( *string, "E" ) == 0 ||
          strcmp( *string, "W" ) == 0 ||
          strcmp( *string, "NE" ) == 0 ||
          strcmp( *string, "NW" ) == 0 ||
          strcmp( *string, "SE" ) == 0 ||
          strcmp( *string, "SW" ) == 0    ) {
         strcpy(Mptr->VIRGA_DIR, *string);
         (*NDEX)++;
      }
      return TRUE;
   }
 
}
 
static ubool isSfcObscuration( char *string, Decoded_METAR *Mptr,
                              int *NDEX )
{
   /***************************/
   /* DECLARE LOCAL VARIABLES */
   /***************************/
 
   static char *WxSymbols[] = {"BCFG", "BLDU", "BLSA", "BLPY",
          "DRDU", "DRSA", "DRSN", "DZ", "DS", "FZFG", "FZDZ", "FZRA",
          "FG", "FC", "FU", "GS", "GR", "HZ", "IC", "MIFG",
          "PE", "PO", "RA", "SHRA", "SHSN", "SHPE", "SHGS",
          "SHGR", "SN", "SG", "SQ", "SA", "SS", "TSRA",
          "TSSN", "TSPE", "TSGS", "TSGR", "TS",
          "VCSH", "VCPO", "VCBLDU", "VCBLSA", "VCBLSN",
          "VCFG", "VCFC","VA", NULL};
   int i,
       ndex;
   char *numLoc,
        ww[12],
        *temp;
 
   /*************************/
   /* START BODY OF ROUTINE */
   /*************************/
 
   if (string == NULL) return FALSE;

   memset( ww, '\0', sizeof(ww) );
 
   if( strlen(string) < 4 )
      return FALSE;
 
   if( strncmp(string, "-X",2 ) != 0 )
      return FALSE;
 
   if( !(nisdigit(string+(strlen(string)-1), 1)) )
      return FALSE;
   else {
      temp = string + 2;
      strncpy( ww, temp, (strlen(string)-2) );
 
      ndex = 0;
      temp = ww;
      numLoc = temp + (strlen(temp) - 1 );
 
      while( temp < numLoc && ndex < 6 ) {
         i = 0;
 
         while( WxSymbols[i] != NULL )
            if( strncmp( WxSymbols[i], temp, strlen(WxSymbols[i]))
                 != 0 )
               i++;
            else
               break;
 
         if( WxSymbols[i] == NULL ) {
            (*NDEX)++;
            return FALSE;
         }
         else {
            strcpy(&(Mptr->SfcObscuration[ndex][0]),WxSymbols[i]);
            temp += strlen(WxSymbols[i]);
            ndex++;
         }
 
      }
 
      if( ndex > 0 ) {
         Mptr->Num8thsSkyObscured = antoi( numLoc,1 );
         (*NDEX)++;
         return TRUE;
      }
      else {
         (*NDEX)++;
         return FALSE;
      }
 
   }
 
}
 
static ubool isCeiling( char *string, Decoded_METAR *Mptr, int *NDEX )
{
 
 
   if (string == NULL) return FALSE;

   if( !(strncmp(string,"CIG",3) == 0 && strlen(string) >= 5) )
      return FALSE;
   else {
      if( strcmp(string, "CIGNO") == 0 ) {
         Mptr->CIGNO = TRUE;
         (*NDEX)++;
         return TRUE;
      }
      else if( strlen( string+3 ) == 3 ) {
         if( nisdigit(string+3, strlen(string+3)) &&
                    strlen(string+3) == 3 ) {
            Mptr->Ceiling = atoi(string+3) * 100;
            (*NDEX)++;
            return TRUE;
         }
         else
            return FALSE;
      }
      else if( strlen(string+3) == 4 ) {
         if( *(string+3) == 'E' && nisdigit(string+4,3) ) {
            Mptr->Estimated_Ceiling = antoi(string+4,3) * 100;
            (*NDEX)++;
            return TRUE;
         }
         else
            return FALSE;
      }
      else
         return FALSE;
 
   }
 
}
static ubool isVrbSky( char **string, Decoded_METAR *Mptr, int *NDEX )
{
   static char *cldPtr[] = {"FEW", "SCT", "BKN", "OVC", NULL };
 
   int i;
   char SKY1[ 100 ];
 
   if (*string == NULL) return FALSE;

   memset( SKY1, '\0', 100 );
   i = 0;
 
   while( cldPtr[i] != NULL )
      if( strncmp(*string, cldPtr[i], strlen(cldPtr[i])) != 0 )
         i++;
      else
         break;
 
   if( cldPtr[i] == NULL )
      return FALSE;
   else {

      strcpy( SKY1, *string );
 
      (++string);
      if (*string == NULL) return FALSE;
 
      if(strcmp(*string, "V") != 0)
         return FALSE;
      else {
         (++string);
	 if (*string == NULL) return FALSE;
         i = 0;
         while( cldPtr[i] != NULL )
            if(strncmp(*string, cldPtr[i], strlen(cldPtr[i])) != 0 )
               i++;
            else
               break;

         if( cldPtr[i] == NULL ) {
            (*NDEX)++;
            (*NDEX)++;
            return FALSE;
         }
         else {
            if(strlen(SKY1) == 6 ) {
               if( nisdigit(SKY1+3,3)) {
                  strncpy(Mptr->VrbSkyBelow,SKY1,3);
                  strcpy(Mptr->VrbSkyAbove,cldPtr[i]);
                  Mptr->VrbSkyLayerHgt = antoi(SKY1+3,3)*100;
                  (*NDEX)++;
                  (*NDEX)++;
                  (*NDEX)++;
                  return TRUE;
               }
               else {
                  (*NDEX)++;
                  (*NDEX)++;
                  (*NDEX)++;
                  return TRUE;
               }
            }
            else {
               strcpy(Mptr->VrbSkyBelow,SKY1);
               strcpy(Mptr->VrbSkyAbove,cldPtr[i]);
               (*NDEX)++;
               (*NDEX)++;
               (*NDEX)++;
               return TRUE;
            }
 
         }
 
      }

   }
 
}
 
static ubool isObscurAloft( char **string, Decoded_METAR *Mptr,
                           int *NDEX )
{
 
   /***************************/
   /* DECLARE LOCAL VARIABLES */
   /***************************/
 
   static char *WxSymbols[] = {"BCFG", "BLDU", "BLSA", "BLPY",
          "DRDU", "DRSA", "DRSN", "DZ", "DS", "FZFG", "FZDZ", "FZRA",
          "FG", "FC", "FU", "GS", "GR", "HZ", "IC", "MIFG",
          "PE", "PO", "RA", "SHRA", "SHSN", "SHPE", "SHGS",
          "SHGR", "SN", "SG", "SQ", "SA", "SS", "TSRA",
          "TSSN", "TSPE", "TSGS", "TSGR", "TS",
          "VCSH", "VCPO", "VCBLDU", "VCBLSA", "VCBLSN",
          "VCFG", "VCFC","VA", NULL};
   int i;
   char *saveTemp,
        *temp;
 
   /*************************/
   /* START BODY OF ROUTINE */
   /*************************/
 
   if (*string == NULL) return FALSE;

   saveTemp = temp = *string;
 
   if(temp == NULL ||  *temp == '\0' )
      return FALSE;
 
   while( temp != NULL &&  *temp != '\0' ) {
      i = 0;
 
      while( WxSymbols[i] != NULL )
         if( strncmp(temp,WxSymbols[i],strlen(WxSymbols[i])) != 0 )
            i++;
         else
            break;
 
      if( WxSymbols[i] == NULL ) {
         return FALSE;
      }
      else
         temp += strlen(WxSymbols[i]);
   }
 
   (++string);
   if (*string == NULL) return FALSE;
   if( strlen(*string) != 6 )
      return FALSE;
   else {
      if((strncmp(*string,"FEW",3) == 0 ||
          strncmp(*string,"SCT",3) == 0 ||
          strncmp(*string,"BKN",3) == 0 ||
          strncmp(*string,"OVC",3) == 0  ) &&
                 (nisdigit(*string+3,3) &&
                  strcmp(*string+3,"000") != 0  )) {
         strcpy(Mptr->ObscurAloft,saveTemp);
         strncpy(Mptr->ObscurAloftSkyCond, *string,3);
         Mptr->ObscurAloftHgt = atoi(*string+3)*100;
         (*NDEX)++;
         (*NDEX)++;
         (*NDEX)++;
         return TRUE;
      }
      else {
         (*NDEX)++;
         return TRUE;
      }
 
   }
 
}
static ubool isNOSPECI( char *string, Decoded_METAR *Mptr, int *NDEX )
{
   if (string == NULL) return FALSE;

   if( strcmp(string,"NOSPECI") != 0 )
      return FALSE;
   else {
      Mptr->NOSPECI = TRUE;
      (*NDEX)++;
      return TRUE;
   }
}
static ubool isLAST( char *string, Decoded_METAR *Mptr, int *NDEX )
{
   if (string == NULL) return FALSE;

   if( strcmp(string,"LAST") != 0 )
      return FALSE;
   else {
      Mptr->LAST = TRUE;
      (*NDEX)++;
      return TRUE;
   }
}
/********************************************************************/
/*                                                                  */
/*  Title:         isSynopClouds                                    */
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
static ubool isSynopClouds( char *token, Decoded_METAR *Mptr,
                           int *NDEX )
{
 
   if (token == NULL) return FALSE;

   if(strlen(token) != 5)
      return FALSE;
 
   if( *token == '8' &&
       *(token+1) == '/'  &&
       ((*(token+2) <= '9' && *(token+2) >= '0') || *(token+2) == '/')
                          &&
       ((*(token+3) <= '9' && *(token+3) >= '0') || *(token+3) == '/')
                          &&
       ((*(token+4) <= '9' && *(token+4) >= '0') || *(token+4) == '/'))
   {
      strcpy(Mptr->synoptic_cloud_type,token);
 
      Mptr->CloudLow    = *(token+2);
      Mptr->CloudMedium = *(token+3);
      Mptr->CloudHigh   = *(token+4);
 
      (*NDEX)++;
      return TRUE;
   }
   else
      return FALSE;
}
 
static ubool isSNINCR( char **string, Decoded_METAR *Mptr, int *NDEX )
{
 
   char *slash;
 
   if (*string == NULL) return FALSE;
 
   if( strcmp( *string, "SNINCR") != 0 )
      return FALSE;
   else {
      (++string);
      if (*string == NULL) return FALSE;
 
      if( (slash = strchr(*string,'/')) == NULL ) {
         (*NDEX)++;
         return FALSE;
      }
      else if( nisdigit (*string,(slash-*string)) &&
                 nisdigit(slash+1,strlen(slash+1)) ) {
         Mptr->SNINCR = antoi(*string,(slash-*string));
         Mptr->SNINCR_TotalDepth = antoi(slash+1,strlen(slash+1));
         (*NDEX)++;
         (*NDEX)++;
         return TRUE;
      }
      else {
         (*NDEX)++;
         return FALSE;
      }
 
   }
 
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isSnowDepth                                      */
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
static ubool isSnowDepth( char *token, Decoded_METAR *Mptr,
                         int *NDEX )
{
 
   if (token == NULL) return FALSE;

   if(strlen(token) != 5)
      return FALSE;
 
   if( *token == '4' &&
       *(token+1) == '/'  &&
       nisdigit( (token+2),3) )
   {
      strcpy(Mptr->snow_depth_group,token);
      Mptr->snow_depth = antoi(token+2,3);
      (*NDEX)++;
      return TRUE;
   }
   else
      return FALSE;
}
 
static ubool isWaterEquivSnow( char *string,
                               Decoded_METAR *Mptr,
                               int *NDEX )
{
   if( strlen(string) != 6 )
      return FALSE;
   else if( !(nisdigit(string,6)) )
      return FALSE;
   else if( strncmp(string, "933", 3) != 0 )
      return FALSE;
   else {
      Mptr->WaterEquivSnow = ((float) atoi(string+3))/10.;
      (*NDEX)++;
      return TRUE;
   }
 
}
static ubool isSunshineDur( char *string, Decoded_METAR *Mptr,
                           int *NDEX )
{
   if( strlen(string) != 5 )
      return FALSE;
   else if( strncmp(string, "98", 2) != 0 )
      return FALSE;
   else if(nisdigit(string+2,3)) {
      Mptr->SunshineDur = atoi(string+2);
      (*NDEX)++;
      return TRUE;
   }
   else if( strncmp(string+2, "///", 3) == 0 ) {
      Mptr->SunSensorOut = TRUE;
      (*NDEX)++;
      return TRUE;
   }
   else
      return FALSE;
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isHourlyPrecip                                   */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          20 Nov 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:      x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Input:         x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Output:        x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
static ubool isHourlyPrecip( char *string, Decoded_METAR *Mptr,
                        int *NDEX )
{
 
   if (string == NULL) return FALSE;

   if( *string == 'P' && (nisdigit(string+1,4) ||
                          strcmp(string+1,"////") == 0) ) {
      if( strcmp(string+1, "////") == 0 ) {
         Mptr->precip_amt = (float) MAXINT;
         (*NDEX)++;
         return TRUE;
      }
      else {
         Mptr->precip_amt = ((float) atoi(string+1)) / 100;
         (*NDEX)++;
         return TRUE;
      }
   }
   else
      return FALSE;
 
}


/* ORIGINAL VERSION WITH BUGS.  REPLACED WITH VERSION ABOVE 
   P.Neilley, NCAR/RAP 7/3/96

static ubool isHourlyPrecip( char **string, Decoded_METAR *Mptr,
                            int *NDEX)
{
 

   if (*string == NULL) return FALSE;

  if( strcmp(*string, "P") == 0 ) {
      (++string);
      if (*string == NULL) return FALSE;
      if( strlen(*string) != 4 )
         return FALSE;
      else {
         if( nisdigit(*string,strlen(*string)) ) {
            Mptr->hourlyPrecip =  ((float)
                         atoi(*string)) * 0.01;
            (*NDEX)++;
            (*NDEX)++;
            return TRUE;
         }
         else
            return FALSE;
      }
   }
 
  return FALSE;

   if( **string != 'P' )
      return FALSE;
   else
      (*string)++;
 
   if (*string == NULL) return FALSE;

   if( **string == '\0' ) {
      (*string)--;
      return FALSE;
   }

   if( nisdigit(*string,strlen(*string)) ) {
      Mptr->hourlyPrecip =  ((float)
                     atoi(*string)) * 0.01;
      (*NDEX)++;
      return TRUE;
   }
   else {
      (*string)--;
      return FALSE;
   }
 
}

END OF REPLACED VERSION */
 
/********************************************************************/
/*                                                                  */
/*  Title:         isP6Precip                                       */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          20 Nov 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:      x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Input:         x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Output:        x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
static ubool isP6Precip( char *string, Decoded_METAR *Mptr,
                        int *NDEX )
{
 
   if (string == NULL) return FALSE;

   if( *string == '6' && (nisdigit(string+1,4) ||
                          strcmp(string+1,"////") == 0) ) {
      if( strcmp(string+1, "////") == 0 ) {
         Mptr->precip_amt = (float) MAXINT;
         (*NDEX)++;
         return TRUE;
      }
      else {
         Mptr->precip_amt = ((float) atoi(string+1)) / 100;
         (*NDEX)++;
         return TRUE;
      }
   }
   else
      return FALSE;
 
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isP24Precip                                      */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          20 Nov 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:      x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Input:         x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Output:        x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
static ubool isP24Precip( char *string, Decoded_METAR *Mptr,
                        int *NDEX )
{
 
   if (string == NULL) return FALSE;

   if( *string == '7' && (nisdigit(string+1,4) ||
                          strcmp(string+1,"////") == 0) ) {
      if( strcmp(string+1, "////") == 0 ) {
         Mptr->precip_24_amt = (float) MAXINT;
         (*NDEX)++;
         return TRUE;
      }
      else {
         Mptr->precip_24_amt = ((float) atoi(string+1)) / 100.;
         (*NDEX)++;
         return TRUE;
      }
   }
   else
      return FALSE;
 
}
/********************************************************************/
/*                                                                  */
/*  Title:         isTTdTenths                                      */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          16 Nov 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:      x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Input:         x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Output:        x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
static ubool isTTdTenths( char *token, Decoded_METAR *Mptr, int *NDEX)
{
 
   /***************************/
   /* DECLARE LOCAL VARIABLES */
   /***************************/
 
   ubool returnFlag = FALSE;
   float sign;
 
   if (token == NULL) return FALSE;

   if( *token != 'T' )
      return FALSE;
   else if( !(strlen(token) == 5 || strlen(token) == 9) )
      return FALSE;
   else
   {
      if( (*(token+1) == '0' || *(token+1) == '1') &&
                 nisdigit(token+2,3) )
      {
         if( *(token+1) == '0' )
            sign = 0.1;
         else
            sign = -0.1;
 
         Mptr->Temp_2_tenths = sign * ((float) antoi(token+2,3));
         returnFlag = TRUE;
      }
      else
        return FALSE;
 
      if( (*(token+5) == '0' || *(token+5) == '1') &&
                 nisdigit(token+6,3) )
      {
         if( *(token+5) == '0' )
            sign = 0.1;
         else
            sign = -0.1;
 
         Mptr->DP_Temp_2_tenths = sign * ((float) atoi(token+6));
         (*NDEX)++;
         return TRUE;
 
      }
      else
      {
         if( returnFlag )
         {
            (*NDEX)++;
            return TRUE;
         }
         else
            return FALSE;
      }
   }
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isMaxTemp                                        */
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
static ubool isMaxTemp(char *string, Decoded_METAR *Mptr, int *NDEX)
{
   char buf[ 6 ];
 
   if (string == NULL) return FALSE;

   if(strlen(string) != 5 )
      return FALSE;
   else if(*string == '1' && (*(string+1) == '0' ||
                              *(string+1) == '1' ||
                              *(string+1) == '/'   ) &&
          (nisdigit((string+2),3) ||
            strncmp(string+2,"///",3) == 0) )
   {
      if(nisdigit(string+2,3))
      {
         memset(buf,'\0',6);
         strncpy(buf,string+2,3);
         Mptr->maxtemp = ( (float) atoi(buf))/10.;
 
         if( *(string+1) == '1' )
            Mptr->maxtemp *= (-1.0);
 
         (*NDEX)++;
         return TRUE;
      }
      else
         Mptr->maxtemp = (float) MAXINT;
         (*NDEX)++;

   }
   else
      return FALSE;

   return FALSE;        /* added by Jaimi Yee - for compilation purposes */
 
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isMinTemp                                        */
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
static ubool isMinTemp(char *string, Decoded_METAR *Mptr, int *NDEX)
{
   char buf[ 6 ];
 
   if (string == NULL) return FALSE;

   if(strlen(string) != 5 )
      return FALSE;
   else if(*string == '2' && (*(string+1) == '0' ||
                              *(string+1) == '1' ||
                              *(string+1) == '/'   ) &&
          (nisdigit((string+2),3) ||
              strncmp(string+2,"///",3) == 0) )
   {
      if(nisdigit(string+2,3))
      {
         memset(buf,'\0',6);
         strncpy(buf,string+2,3);
         Mptr->mintemp = ( (float) atoi(buf) )/10.;
 
         if( *(string+1) == '1' )
            Mptr->mintemp *= (-1.0);
         (*NDEX)++;
         return TRUE;
      }
      else
         (*NDEX)++;
         return TRUE;
   }
   else
      return FALSE;
 
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isT24MaxMinTemp                                  */
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
static ubool isT24MaxMinTemp( char *string, Decoded_METAR *Mptr,
                             int *NDEX )
{
   char buf[ 6 ];
 
   if (string == NULL) return FALSE;

 
   if( strlen(string) != 9 )
      return FALSE;
   else if( (*string == '4' && (*(string+1) == '0' ||
                                *(string+1) == '1' ||
                                *(string+1) == '/')     &&
             (nisdigit((string+2),3) || strncmp(string+2,"///",3)))
                              &&
             ((*(string+5) == '0' || *(string+5) == '1' ||
              *(string+5) == '/') &&
              (nisdigit((string+6),3) ||
               strncmp(string+6,"///",3) == 0 )) )
   {
      if(nisdigit(string+1,4) && (*(string+1) == '0' ||
                                  *(string+1) == '1')   )
      {
         memset(buf, '\0', 6);
         strncpy(buf, string+2, 3);
         Mptr->max24temp = ( (float) atoi( buf ) )/10.;
 
         if( *(string+1) == '1' )
            Mptr->max24temp *= -1.;
      }
      else
         Mptr->max24temp = (float) MAXINT;
 
 
      if(nisdigit(string+5,4) && (*(string+5) == '0' ||
                                  *(string+5) == '1' )  )
      {
         memset(buf, '\0', 6);
         strncpy(buf, string+6, 3);
         Mptr->min24temp = ( (float) atoi(buf) )/10.;
 
         if( *(string+5) == '1' )
            Mptr->min24temp *= -1.;
      }
      else
         Mptr->min24temp = (float) MAXINT;
 
      (*NDEX)++;
      return TRUE;
 
   }
   else
      return FALSE;
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isPtendency                                      */
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
 
static ubool isPtendency(char *string, Decoded_METAR *Mptr, int *NDEX)
{
   char buf[ 6 ];
 
   if (string == NULL) return FALSE;

   if(strlen(string) != 5)
      return FALSE;
   else if(*string == '5' && ('0' <= *(string+1) <= '8') &&
             (nisdigit(string+2,3) || strncmp(string+2,"///",3)
                                             == 0) )
   {
      if( !(nisdigit(string+2,3)) )
      {
         memset(buf,'\0',6);
         strncpy(buf,(string+1),1);
         Mptr->char_prestndcy = atoi(buf);
         (*NDEX)++;
         return TRUE;
      }
      else
      {
         memset(buf,'\0',6);
         strncpy(buf,(string+1),1);
         Mptr->char_prestndcy = atoi(buf);
 
         Mptr->prestndcy = ((float) atoi(string+2)) * 0.1;
 
         (*NDEX)++;
         return TRUE;
      }
 
   }
   else
      return FALSE;
 
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isPWINO                                          */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          20 Nov 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:      x                                                */
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
 
static ubool isPWINO( char *string, Decoded_METAR *Mptr, int *NDEX)
{
 
   if (string == NULL) return FALSE;

#ifdef PRTOKENX
   printf("isPWINO:  Entered...\n");
   printf("isPWINO:  string = %s\n",string);
#endif
 
   if( strcmp(string, "PWINO") != 0 )
#ifdef PRTOKENX
   {
   printf("isPWINO:  PWINO NOT SET\n");
#endif
      return FALSE;
#ifdef PRTOKENX
   }
#endif
   else {
      Mptr->PWINO = TRUE;
#ifdef PRTOKENX
   printf("isPWINO:  PWINO SET TO TRUE\n");
#endif
      (*NDEX)++;
      return TRUE;
   }
 
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isPNO                                            */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          20 Nov 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:      x                                                */
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
 
static ubool isPNO( char *string, Decoded_METAR *Mptr, int *NDEX)
{
 
   if (string == NULL) return FALSE;

#ifdef PRTOKENX
   printf("isPNO:  Entered...\n");
   printf("isPNO:  string = %s\n",string);
#endif
 
   if( strcmp(string, "PNO") != 0 )
#ifdef PRTOKENX
   {
   printf("isPNO:  PNO NOT SET \n");
#endif
      return FALSE;
#ifdef PRTOKENX
   }
#endif
   else {
      Mptr->PNO = TRUE;
#ifdef PRTOKENX
   printf("isPNO:  PNO SET TO TRUE\n");
#endif
      (*NDEX)++;
      return TRUE;
   }
 
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isRVRNO                                          */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          20 Nov 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:      x                                                */
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
 
static ubool isRVRNO( char *string, Decoded_METAR *Mptr, int *NDEX)
{
 
   if (string == NULL) return FALSE;

   if( strcmp(string, "RVRNO") != 0 )
      return FALSE;
   else {
      Mptr->RVRNO = TRUE;
      (*NDEX)++;
      return TRUE;
   }
 
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isCHINO                                          */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          20 Nov 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:      x                                                */
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
 
static ubool isCHINO( char **string, Decoded_METAR *Mptr, int *NDEX)
{
 
   if (*string == NULL) return FALSE;

   if( strcmp(*string, "CHINO") != 0 )
      return FALSE;
   else
      string++;
 
   if (*string == NULL) return FALSE;

   if( strlen(*string) <= 3 ) {
      (*NDEX)++;
      /* FALSE; */        /* statement with no effect */
      return FALSE;       /* added by Jaimi Yee */ 
   }
   else {
      if( strncmp( *string, "RWY", 3 ) == 0 &&
            nisdigit(*string+3,strlen(*string+3)) ) {
         Mptr->CHINO = TRUE;
         strcpy(Mptr->CHINO_LOC, *string);
         (*NDEX)++;
         (*NDEX)++;
         return TRUE;
      }
      else {
         (*NDEX)++;
         /* FALSE; */    /* statement with no effect */
         return FALSE;   /* added by Jaimi Yee */
      }
   }

   return FALSE;       /* added by Jaimi Yee - for compilation purposes */
 
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isVISNO                                          */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          20 Nov 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:      x                                                */
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
 
static ubool isVISNO( char **string, Decoded_METAR *Mptr, int *NDEX)
{
 
   if (*string == NULL) return FALSE;

   if( strcmp(*string, "VISNO") != 0 )
      return FALSE;
   else
      string++;
 
   if (*string == NULL) return FALSE;

   if( strlen(*string) <= 3 ) {
      (*NDEX)++;
      /* FALSE */     /* statement with no effect */
      return FALSE;   /* added by Jaimi Yee */
   }
   else {
      if( strncmp( *string, "RWY", 3 ) == 0 &&
            nisdigit(*string+3,strlen(*string+3))) {
         Mptr->VISNO = TRUE;
         strcpy(Mptr->VISNO_LOC, *string);
         (*NDEX)++;
         (*NDEX)++;
         return TRUE;
      }
      else {
         (*NDEX)++;
         /* FALSE; */    /* statement with no effect */
         return FALSE;   /* added by Jaimi Yee */
      }
   }

   return FALSE;   /* added by Jaimi Yee - for compilation purposes */
 
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isFZRANO                                         */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          20 Nov 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:      x                                                */
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
 
static ubool isFZRANO( char *string, Decoded_METAR *Mptr, int *NDEX)
{
 
   if (string == NULL) return FALSE;
   if( strcmp(string, "FZRANO") != 0 )
      return FALSE;
   else {
      Mptr->FZRANO = TRUE;
      (*NDEX)++;
      return TRUE;
   }
 
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isTSNO                                            */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          20 Nov 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:      x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Input:         x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Output:        x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                 x                                                */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
static ubool isTSNO( char *string, Decoded_METAR *Mptr, int *NDEX)
{
 
   if (string == NULL) return FALSE;
   if( strcmp(string, "TSNO") != 0 )
      return FALSE;
   else {
      Mptr->TSNO = TRUE;
      (*NDEX)++;
      return TRUE;
   }
 
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         isDollarSign                                 */
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
 
static ubool isDollarSign( char *indicator, Decoded_METAR *Mptr,
                              int *NDEX )
{
   if( strcmp(indicator,"$") != 0 )
      return FALSE;
   else
   {
      (*NDEX)++;
      Mptr->DollarSign = TRUE;
      return TRUE;
   }
}
 
/********************************************************************/
/*                                                                  */
/*  Title:         DcdMTRmk                                         */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          15 Sep 1994                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:      DcdMTRmk takes a pointer to a METAR              */
/*                 report and parses/decodes data elements from     */
/*                 the remarks section of the report.               */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 None.                                            */
/*                                                                  */
/*  Input:         token - the address of a pointer to a METAR      */
/*                         report character string.                 */
/*                 Mptr  - a pointer to a structure of the vari-    */
/*                         able type Decoded_METAR.                 */
/*                                                                  */
/*                                                                  */
/*  Output:        x                                                */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
void DcdMTRmk( char **token, Decoded_METAR *Mptr )
{
 
   /***************************/
   /* DECLARE LOCAL VARIABLES */
   /***************************/
 
   int TornadicActvty = 0, A0indicator = 0,
       peakwind = 0, windshift = 0, towerVsby = 0, surfaceVsby = 0,
       variableVsby = 0, LTGfreq = 0,
       recentWX = 0, variableCIG = 0, PRESFR = 0,
       Vsby2ndSite = 0, CIG2ndSite = 0,
       PRESRR = 0, SLP = 0, PartObscur = 0,
       SectorVsby = 0, GR = 0, Virga = 0,
       SfcObscur = 0, Ceiling = 0, VrbSkyCond = 0, ObscurAloft = 0,
       NoSPECI = 0, Last = 0, SynopClouds = 0, Snincr = 0,
       SnowDepth = 0, WaterEquivSnow = 0, SunshineDur = 0,
       hourlyPrecip = 0, P6Precip = 0, P24Precip = 0,
       TTdTenths = 0, MaxTemp = 0, MinTemp = 0, T24MaxMinTemp = 0,
       Ptendency = 0, PWINO = 0,
       FZRANO = 0, TSNO = 0, maintIndicator = 0, CHINO = 0, RVRNO = 0,
       VISNO = 0, PNO = 0, DVR = 0;
 
   int  NDEX,
        i;
 
   ubool ContnuFlag;

 
   /*************************/
   /* START BODY OF ROUTINE */
   /*************************/

   if (*token == NULL) return;

#ifdef  PRTOKEN 
   int KEEP_NDEX;
   printf("\n\nDcdMTRMK ENTERED...\n\n");
   printf("\n\nFROM DcdMTRMK:  Print TOKEN list...\n\n");
   NDEX = 0;
   KEEP_NDEX = NDEX;
   while( token[NDEX] != NULL ) {
      printf("Token[%d] = %s\n",NDEX,token[NDEX]);
      NDEX++;
   }
   NDEX = KEEP_NDEX;
#endif
 
   NDEX = 0;
 
   /*************************************************/
   /* LOCATE THE START OF THE METAR REMARKS SECTION */
   /*************************************************/
   ContnuFlag = TRUE;
   while( token[ NDEX ] != NULL && ContnuFlag ) {
      if( strcmp(token[ NDEX ], "RMK")     != 0 &&
          strcmp(token[ NDEX ], "RMRK")    != 0 &&
          strcmp(token[ NDEX ], "REMARK")  != 0 &&
          strcmp(token[ NDEX ], "REMARKS") != 0   )
         NDEX++;
      else
         ContnuFlag = FALSE;
   }
 
#ifdef  PRTOKEN
   printf("\n\nFROM DcdMTRMK:  Print TOKEN list...\n\n");
   KEEP_NDEX = NDEX;
   while( token[NDEX] != NULL ) {
      printf("Token[%d] = %s\n",NDEX,token[NDEX]);
      NDEX++;
   }
   NDEX = KEEP_NDEX;
#endif
 
   /***********************************************/
   /* IF THE METAR REPORT CONTAINS NO REMARKS     */
   /* SECTION, THEN RETURN TO THE CALLING ROUTINE */
   /***********************************************/
 
   if( token[ NDEX ] == NULL )
      return;
   else
      NDEX++;
 
#ifdef  PRTOKEN
   printf("\n\nFROM DcdMTRMK:  Print TOKEN list...\n\n");
   KEEP_NDEX = NDEX;
   while( token[NDEX] != NULL ) {
      printf("Token[%d] = %s\n",NDEX,token[NDEX]);
      NDEX++;
   }
   NDEX = KEEP_NDEX;
#endif
   /*****************************************/
   /* IDENTIFY AND VALIDATE REMARKS SECTION */
   /*   DATA GROUPS FOR PARSING/DECODING    */
   /*****************************************/
 
   while(token[NDEX] != NULL) {

#ifdef  PRTOKENX
   printf("\n\nFROM DcdMTRMK:  Within WHILE-LOOP..\n\n");
   printf("Token[%d] = %s\n",NDEX,token[NDEX]);
      if( strcmp( token[NDEX], "PWINO" ) == 0 ||
       strcmp( token[NDEX], "PNO" ) == 0 )
      __ctest(NULL);
#endif
 
      if( token[NDEX] != NULL &&
	  isTornadicActiv( &(token[NDEX]), Mptr, &NDEX ) ) {
         TornadicActvty++;
         if( TornadicActvty > 1 ) {
            memset(Mptr->TornadicType,'\0',15);
            memset(Mptr->TornadicLOC,'\0',10);
            memset(Mptr->TornadicDIR,'\0',4);
            Mptr->BTornadicHour = MAXINT;
            Mptr->BTornadicMinute = MAXINT;
            Mptr->ETornadicHour = MAXINT;
            Mptr->ETornadicMinute = MAXINT;
         }
      }

      else if( token[NDEX] != NULL &&
	       isA0indicator( token[NDEX], Mptr, &NDEX ) ) {
         A0indicator++;
         if( A0indicator > 1 )
            memset(Mptr->autoIndicator,'\0',5);
      }
      else if( token[NDEX] != NULL &&
	       isPeakWind( &(token[NDEX]), Mptr, &NDEX ) ) {
         peakwind++;
         if( peakwind > 1 ) {
            Mptr->PKWND_dir = MAXINT;
            Mptr->PKWND_speed = MAXINT;
            Mptr->PKWND_hour = MAXINT;
            Mptr->PKWND_minute = MAXINT;
         }
      }
      else if( token[NDEX] != NULL &&
	       isWindShift( &(token[NDEX]), Mptr, &NDEX ) ) {
         windshift++;
         if( windshift > 1 ) {
            Mptr->WshfTime_hour = MAXINT;
            Mptr->WshfTime_minute = MAXINT;
         }
      }
      else if( token[NDEX] != NULL &&
	       isTowerVsby( &(token[NDEX]), Mptr, &NDEX ) ) {
         towerVsby++;
         if( towerVsby > 1 )
            Mptr->TWR_VSBY = (float) MAXINT;
      }

      else if( token[NDEX] != NULL &&
	       isSurfaceVsby( &(token[NDEX]), Mptr, &NDEX ) ) {
         surfaceVsby++;
         if( surfaceVsby > 1 )
            Mptr->TWR_VSBY = (float) MAXINT;
      }
      else if( token[NDEX] != NULL &&
	       isVariableVsby( &(token[NDEX]), Mptr, &NDEX ) ) {
         variableVsby++;
         if( variableVsby > 1 ) {
            Mptr->minVsby = (float) MAXINT;
            Mptr->maxVsby = (float) MAXINT;
         }
      }
      else if( token[NDEX] != NULL &&
	       isVsby2ndSite( &(token[NDEX]), Mptr, &NDEX ) ) {
         Vsby2ndSite++;
         if( Vsby2ndSite > 1 ) {
            Mptr->VSBY_2ndSite = (float) MAXINT;
            memset(Mptr->VSBY_2ndSite_LOC,'\0',10);
         }
      }
      else if( token[NDEX] != NULL &&
	       isLTGfreq( &(token[NDEX]), Mptr, &NDEX ) ) {
         LTGfreq++;
         if( LTGfreq > 1 ) {
            Mptr->OCNL_LTG = FALSE;
            Mptr->FRQ_LTG = FALSE;
            Mptr->CNS_LTG = FALSE;
            Mptr->CG_LTG = FALSE;
            Mptr->IC_LTG = FALSE;
            Mptr->CC_LTG = FALSE;
            Mptr->CA_LTG = FALSE;
            Mptr->DSNT_LTG = FALSE;
            Mptr->VcyStn_LTG = FALSE;
            memset(Mptr->LTG_DIR,'\0',3 );
         }
      }

      else if( token[NDEX] != NULL &&
	       isRecentWX( &(token[NDEX]), Mptr, &recentWX ) ) {
         recentWX++;
         if( recentWX > 3 ) {
            for( i = 0; i < 3; i++ ) {
               memset(Mptr->ReWx[i].Recent_weather, '\0', 5);
 
               Mptr->ReWx[i].Bhh = MAXINT;
               Mptr->ReWx[i].Bmm = MAXINT;
 
               Mptr->ReWx[i].Ehh = MAXINT;
               Mptr->ReWx[i].Emm = MAXINT;
            }
 
            NDEX++;
         }
         else {
            NDEX++;
	 }
      }
      else if( token[NDEX] != NULL &&
	       isVariableCIG( &(token[NDEX]), Mptr, &NDEX ) ) {
         variableCIG++;
         if( variableCIG > 1) {
            Mptr->minCeiling = MAXINT;
            Mptr->maxCeiling = MAXINT;
         }
      }

      else if( token[NDEX] != NULL &&
	       isCIG2ndSite( &(token[NDEX]), Mptr, &NDEX ) ) {
         CIG2ndSite++;
         if( CIG2ndSite > 1) {
            Mptr->CIG_2ndSite_Meters = MAXINT;
            memset( Mptr->CIG_2ndSite_LOC, '\0', 10);
         }
      }
      else if( token[NDEX] != NULL &&
	       isPRESFR( token[NDEX], Mptr, &NDEX ) ) {
         PRESFR++;
         if( PRESFR > 1 )
            Mptr->PRESFR = FALSE;
      }
      else if( token[NDEX] != NULL &&
	       isPRESRR( token[NDEX], Mptr, &NDEX ) ) {
         PRESRR++;
         if( PRESRR > 1 )
            Mptr->PRESRR = FALSE;
      }
      else if( token[NDEX] != NULL &&
	       isSLP( &(token[NDEX]), Mptr, &NDEX ) ) {
         SLP++;
         if( SLP > 1 )
            Mptr->SLP = (float) MAXINT;
      }

      else if( token[NDEX] != NULL &&
	       isPartObscur( &(token[NDEX]), Mptr, PartObscur,
               &NDEX ) ) {
         PartObscur++;
         if( PartObscur > 2 ) {
            memset(&(Mptr->PartialObscurationAmt[0][0]), '\0', 7 );
            memset(&(Mptr->PartialObscurationPhenom[0][0]),'\0',12 );
            memset(&(Mptr->PartialObscurationAmt[1][0]), '\0', 7 );
            memset(&(Mptr->PartialObscurationPhenom[1][0]),'\0',12 );
         }
      }

      else if( token[NDEX] != NULL &&
	       isSectorVsby( &(token[NDEX]), Mptr, &NDEX ) ) {
         SectorVsby++;
         if( SectorVsby > 1 ) {
            Mptr->SectorVsby = (float) MAXINT;
            memset(Mptr->SectorVsby_Dir, '\0', 3);
         }
      }
      else if( token[NDEX] != NULL &&
	       isGR( &(token[NDEX]), Mptr, &NDEX ) ) {
         GR++;
         if( GR > 1 ) {
            Mptr->GR_Size = (float) MAXINT;
            Mptr->GR = FALSE;
         }
      }
      else if( token[NDEX] != NULL &&
	       isVIRGA( &(token[NDEX]), Mptr, &NDEX ) ) {
         Virga++;
         if( Virga > 1 ) {
            Mptr->VIRGA = FALSE;
            memset(Mptr->VIRGA_DIR, '\0', 3);
         }
      }

      else if( token[NDEX] != NULL &&
	       isSfcObscuration( token[NDEX], Mptr, &NDEX ) ) {
         SfcObscur++;
         if( SfcObscur > 1 ) {
            for( i = 0; i < 6; i++ ) {
               memset(&(Mptr->SfcObscuration[i][0]), '\0', 10);
               Mptr->Num8thsSkyObscured = MAXINT;
            }
         }
      }


      else if( token[NDEX] != NULL &&
	       isCeiling( token[NDEX], Mptr, &NDEX ) ) {
         Ceiling++;
         if( Ceiling > 1 ) {
            Mptr->CIGNO = FALSE;
            Mptr->Ceiling = MAXINT;
          /* Mptr->Estimated_Ceiling; */  /* statement with no effect */
         }
      }

      else if( token[NDEX] != NULL &&
	       isVrbSky( &(token[NDEX]), Mptr, &NDEX ) ) {
         VrbSkyCond++;
         if( VrbSkyCond > 1 ) {
            memset(Mptr->VrbSkyBelow, '\0', 4);
            memset(Mptr->VrbSkyAbove, '\0', 4);
            Mptr->VrbSkyLayerHgt = MAXINT;
         }
      }

      else if( token[NDEX] != NULL &&
	       isObscurAloft( &(token[NDEX]), Mptr, &NDEX ) ) {
         ObscurAloft++;
         if( ObscurAloft > 1 ) {
            Mptr->ObscurAloftHgt = MAXINT;
            memset( Mptr->ObscurAloft, '\0', 12 );
            memset( Mptr->ObscurAloftSkyCond, '\0', 12 );
         }
      }
      else if( token[NDEX] != NULL &&
	       isNOSPECI( token[NDEX], Mptr, &NDEX ) ) {
         NoSPECI++;
         if( NoSPECI > 1 )
            Mptr->NOSPECI = FALSE;
      }
      else if( token[NDEX] != NULL &&
	       isLAST( token[NDEX], Mptr, &NDEX ) ) {
         Last++;
         if( Last > 1 )
            Mptr->LAST = FALSE;
      }
      else if( token[NDEX] != NULL &&
	       isSynopClouds( token[NDEX], Mptr, &NDEX ) ) {
         SynopClouds++;
         if( SynopClouds > 1 ) {
            memset( Mptr->synoptic_cloud_type, '\0', 6 );
            Mptr->CloudLow    = '\0';
            Mptr->CloudMedium = '\0';
            Mptr->CloudHigh   = '\0';
         }
      }

      else if( token[NDEX] != NULL &&
	       isSNINCR( &(token[NDEX]), Mptr, &NDEX ) ) {
         Snincr++;
         if( Snincr > 1 ) {
            Mptr->SNINCR = MAXINT;
            Mptr->SNINCR_TotalDepth = MAXINT;
         }
      }
      else if( token[NDEX] != NULL &&
	       isSnowDepth( token[NDEX], Mptr, &NDEX ) ) {
         SnowDepth++;
         if( SnowDepth > 1 ) {
            memset( Mptr->snow_depth_group, '\0', 6 );
            Mptr->snow_depth = MAXINT;
         }
      }
      else if( token[NDEX] != NULL &&
	       isWaterEquivSnow( token[NDEX], Mptr, &NDEX ) ) {
         WaterEquivSnow++;
         if( WaterEquivSnow > 1 )
            Mptr->WaterEquivSnow = (float) MAXINT;
      }
      else if( token[NDEX] != NULL &&
	       isSunshineDur( token[NDEX], Mptr, &NDEX ) ) {
         SunshineDur++;
         if( SunshineDur > 1 ) {
            Mptr->SunshineDur = MAXINT;
            Mptr->SunSensorOut = FALSE;
         }
      }

      else if( token[NDEX] != NULL &&
	       isHourlyPrecip( token[NDEX], Mptr, &NDEX ) ) { 
         hourlyPrecip++;
         if( hourlyPrecip > 1 )
            Mptr->hourlyPrecip = (float) MAXINT;
      }

      else if( token[NDEX] != NULL &&
	       isP6Precip( token[NDEX], Mptr, &NDEX ) ) {
         P6Precip++;
         if( P6Precip > 1 )
            Mptr->precip_amt = (float) MAXINT;
      }
      else if( token[NDEX] != NULL &&
	       isP24Precip( token[NDEX], Mptr, &NDEX ) ) {
         P24Precip++;
         if( P24Precip > 1 )
            Mptr->precip_24_amt = (float) MAXINT;
      }

      else  if( token[NDEX] != NULL &&
		isTTdTenths( token[NDEX], Mptr, &NDEX ) ) {
         TTdTenths++;
         if( TTdTenths > 1 ) {
            Mptr->Temp_2_tenths = (float) MAXINT;
            Mptr->DP_Temp_2_tenths = (float) MAXINT;
         }
      }
      else if( token[NDEX] != NULL &&
	       isMaxTemp( token[NDEX], Mptr, &NDEX ) ) {
         MaxTemp++;
         if( MaxTemp > 1 )
            Mptr->maxtemp = (float) MAXINT;
      }
      else if( token[NDEX] != NULL &&
	       isMinTemp( token[NDEX], Mptr, &NDEX ) ) {
         MinTemp++;
         if( MinTemp > 1 )
            Mptr->mintemp = (float) MAXINT;
      }
      else if( token[NDEX] != NULL &&
	       isT24MaxMinTemp( token[NDEX],
                                          Mptr, &NDEX ) ) {
         T24MaxMinTemp++;
         if( T24MaxMinTemp > 1 ) {
            Mptr->max24temp = (float) MAXINT;
            Mptr->min24temp = (float) MAXINT;
         }
      }
      else if( token[NDEX] != NULL &&
	       isPtendency( token[NDEX], Mptr, &NDEX ) ) {
         Ptendency++;
         if( Ptendency > 1 ) {
            Mptr->char_prestndcy = MAXINT;
            Mptr->prestndcy = (float) MAXINT;
         }
      }
      else if( token[NDEX] != NULL &&
	       isPWINO( token[NDEX], Mptr, &NDEX ) ) {
         PWINO++;
         if( PWINO > 1 )
            Mptr->PWINO = FALSE;
      }
      else if( token[NDEX] != NULL &&
	       isFZRANO( token[NDEX], Mptr, &NDEX ) ) {
         FZRANO++;
         if( FZRANO > 1 )
            Mptr->FZRANO = FALSE;
      }
      else if( token[NDEX] != NULL &&
	       isTSNO( token[NDEX], Mptr, &NDEX ) ) {
         TSNO++;
         /* if( TSNO > 1 )      
            Mptr->TSNO; */  /* statment with no effect */
      }


      else if( token[NDEX] != NULL &&
	       isDollarSign( token[NDEX], Mptr, &NDEX ) ) {
         maintIndicator++;
         if( maintIndicator > 1 )
            Mptr->DollarSign = FALSE;
      }
      else if( token[NDEX] != NULL &&
	       isRVRNO( token[NDEX], Mptr, &NDEX ) ) {
         RVRNO++;
         if( RVRNO > 1 )
            Mptr->RVRNO = FALSE;
      }
      else if( token[NDEX] != NULL &&
	       isPNO( token[NDEX], Mptr, &NDEX ) ) {
         PNO++;
         if( PNO > 1 )
            Mptr->PNO = FALSE;
      }
      else if( token[NDEX] != NULL &&
	       isVISNO( &(token[NDEX]), Mptr, &NDEX ) ) {
         VISNO++;
         if( VISNO > 1 ) {
            Mptr->VISNO = FALSE;
            memset(Mptr->VISNO_LOC, '\0', 6);
         }
      }
      else if( token[NDEX] != NULL &&
	       isCHINO( &(token[NDEX]), Mptr, &NDEX ) ) {
         CHINO++;
         if( CHINO > 1 ) {
            Mptr->CHINO = FALSE;
            memset(Mptr->CHINO_LOC, '\0', 6);
         }
      }
      else if( token[NDEX] != NULL &&
	       isDVR( token[NDEX], Mptr, &NDEX ) ) {
         DVR++;
         if( DVR > 1 ) {
            Mptr->DVR.Min_visRange = MAXINT;
            Mptr->DVR.Max_visRange = MAXINT;
            Mptr->DVR.visRange = MAXINT;
            Mptr->DVR.vrbl_visRange = FALSE;
            Mptr->DVR.below_min_DVR = FALSE;
            Mptr->DVR.above_max_DVR = FALSE;
         }
      } else NDEX++;
 
#ifdef  PRTOKEN
   printf("\n\nFROM DcdMTRMK:  Print Decoded METAR...\n\n");
   prtDMETR( Mptr );
#endif

   } /* while (token[NDEX} != NULL) */
 
   return;
}
