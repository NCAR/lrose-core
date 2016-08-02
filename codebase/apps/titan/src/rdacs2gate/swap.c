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
/***********************************************************************
 * swap.c
 *
 * Handles swapping for struct members on input
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * Feb 1995
 *
 ************************************************************************/

#include "rdacs2gate.h"
#include <dataport/swap.h>
#include <assert.h>

static InitDone = FALSE;
static NoSwap;

void SwapInit(void)
     
{
  if (Glob->params.input_big_endian) {
    if (BE_is_big_endian()) {
      NoSwap = TRUE;
    } else {
      NoSwap = FALSE;
    }
  } else {
    if (BE_is_big_endian()) {
      NoSwap = FALSE;
    } else {
      NoSwap = TRUE;
    }
  }
  
  InitDone = TRUE;
  
}

void SwapRDP_HDR(RDP_HDR *hdr)
{
  assert(InitDone);
  if (NoSwap) return;
  SWAP_array_16(&hdr->Length, 2);
  SWAP_array_16(&hdr->Status, 2);
}

void SwapRDP_Login_R(RDP_Login_R *r)
{
  assert(InitDone);
  if (NoSwap) return;
  SWAP_array_16(&r->Permissions, 2);
}

void SwapSITECFG(SITECFG *cfg)
{
  assert(InitDone);
  if (NoSwap) return;
  SWAP_array_16(&cfg->VersionNum, 2);
  SWAP_array_16(&cfg->ProtocolNum, 2);
  SWAP_array_16(&cfg->RadarType, 2);
  SWAP_array_32(&cfg->Capabilities, 4);
  SWAP_array_32((ui32 *) &cfg->Latitude, 4);
  SWAP_array_32((ui32 *) &cfg->Longitude, 4);
  SWAP_array_32((ui32 *) &cfg->Altitude_AAT, 4);
  SWAP_array_32((ui32 *) &cfg->Altitude_ASL, 4);
  SWAP_array_16(&cfg->Angle360, 2);
}

void SwapRDP_SetPgm(RDP_SetPgm *p)
{
  assert(InitDone);
  if (NoSwap) return;
  SWAP_array_16(&p->Flags, 2);
}

void SwapRDP_LoadCfg(RDP_LoadCfg *p)
{
  assert(InitDone);
  if (NoSwap) return;
  SWAP_array_16(&p->CfgNum, 2);
}

void SwapRDP_SaveCfg(RDP_SaveCfg *p)
{
  assert(InitDone);
  if (NoSwap) return;
  SWAP_array_16(&p->CfgNum, 2);
}

void SwapRDP_LoadMask(RDP_LoadMask *p)
{
  assert(InitDone);
  if (NoSwap) return;
  SWAP_array_16(&p->MaskNum, 2);
}

void SwapRDP_SaveMask(RDP_SaveMask *p)
{
  assert(InitDone);
  if (NoSwap) return;
  SWAP_array_16(&p->MaskNum, 2);
}

void SwapRDP_GetCurPgm_R(RDP_GetCurPgm_R *p)
{
  assert(InitDone);
  if (NoSwap) return;
  SWAP_array_16(&p->Flags, 2);
}

void SwapRDP_GetMainPgm_R(RDP_GetMainPgm_R *p)
{
  assert(InitDone);
  if (NoSwap) return;
  SWAP_array_16(&p->Flags, 2);
}

void SwapRDP_GetCurStep_R(RDP_GetCurStep_R *p)
{
  assert(InitDone);
  if (NoSwap) return;
  SWAP_array_16(&p->Flags, 2);
}

void SwapRDP_GetRayParms(RDP_GetRayParms *p)
{
  assert(InitDone);
  if (NoSwap) return;
  SWAP_array_16(&p->ProductCode, 2);
}

void SwapRDP_GetRayParms_R(RDP_GetRayParms_R *p)
{
  assert(InitDone);
  if (NoSwap) return;
  SWAP_array_16(&p->ProductCode, 2);
  SWAP_array_16(&p->NumberBins, 2);
  SWAP_array_16(&p->NumberLevels, 2);
  SWAP_array_32(&p->SkipTime, 4);
  SWAP_array_32(&p->BinTime, 4);
  SWAP_array_16(&p->ThresholdSize, 2);
  SWAP_array_16(&p->ThresholdOffset, 2);
}

void SwapRDP_GetRayData_R(RDP_GetRayData_R *p)
{
  assert(InitDone);
  if (NoSwap) return;
  SWAP_array_32(&p->TimeMs, 4);
  SWAP_array_16(&p->Azimuth, 2);
  SWAP_array_16(&p->Elevation, 2);
  SWAP_array_16(&p->OffsetToData, 2);
}

void SwapRDP_SelectAsync(RDP_SelectAsync *p)
{
  assert(InitDone);
  if (NoSwap) return;
  SWAP_array_32(&p->SelectBits, 4);
  SWAP_array_16(&p->RawPacing, 2);
}

void SwapRDP_Overflow_R(RDP_Overflow_R *p)
{
  assert(InitDone);
  if (NoSwap) return;
  SWAP_array_16(&p->LostCount, 2);
}

void SwapRDP_GetRawData_R(RDP_GetRawData_R *p)
{
  assert(InitDone);
  if (NoSwap) return;
  SWAP_array_16(&p->Format, 2);
}

void SwapRDP_ClearMask(RDP_ClearMask *p)
{
  assert(InitDone);
  if (NoSwap) return;
  SWAP_array_16(&p->MaskId, 2);
}

void SwapRADPGM_STEP(RADPGM_STEP *p)
{
  assert(InitDone);
  if (NoSwap) return;
  SWAP_array_16(&p->Range, 2);
  SWAP_array_16(&p->Skip, 2);
  SWAP_array_16(&p->Duration, 2);
  SWAP_array_16(&p->Az, 2);
  SWAP_array_16(&p->El, 2);
  SWAP_array_16(&p->EndPt, 2);
  SWAP_array_16(p->Res, 8);
}

