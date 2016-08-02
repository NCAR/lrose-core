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
 * handle_msg.c
 *
 * handle message from radar
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * Jan 1997
 *
 ************************************************************************/

#include "rdacs2gate.h"

void handle_msg(RDP_HDR *reply_hdr, RDP_ANY_R *msg)

{

  static int count = 0;


  if (Glob->params.debug >= DEBUG_VERBOSE) {
    fprintf(stderr, "Id, len, status: %d, %d, %d --> ", reply_hdr->Id,
	    reply_hdr->Length, reply_hdr->Status);
  }
  
  switch (reply_hdr->Id) {
    
  default:
    if (Glob->params.debug >= DEBUG_NORM) {
      fprintf(stderr, "Unrecognized message ID 0x%02X\n", reply_hdr->Id);
    }
    break;

  case ID_RDP_GetSite:
    {
      SITECFG *site = (SITECFG *) msg;
      SwapSITECFG(site);
      store_site_params(site);
      if (Glob->params.debug >= DEBUG_NORM) {
	fprintf(stderr, "Got GetSite Message\n");
	fprintf(stderr, "  Beam count: %d\n", count);
	fprintf(stderr, "  VersionNum: %d\n", site->VersionNum);
	fprintf(stderr, "  ProtocolNum: %d\n", site->ProtocolNum);
	fprintf(stderr, "  SiteName: %s\n", site->SiteName);
	fprintf(stderr, "  RadarName: %s\n", site->RadarName);
	fprintf(stderr, "  RadarType: %d\n", site->RadarType);
	fprintf(stderr, "  Capabilities: %d\n", site->Capabilities);
	fprintf(stderr, "  Latitude: %g\n", site->Latitude / 1000000.0);
	fprintf(stderr, "  Longitude: %g\n", site->Longitude / 1000000.0);
	fprintf(stderr, "  Altitude_AAT(m): %d\n", site->Altitude_AAT);
	fprintf(stderr, "  Altitude_ASL(m): %d\n", site->Altitude_ASL);
	fprintf(stderr, "  Angle360: %d\n", site->Angle360);
      } /* debug */
    }
    break;

  case ID_RDP_GetCurPgm:
    if (Glob->params.debug >= DEBUG_NORM) {
      fprintf(stderr, "Got GetCurPgm Message\n");
    }
    break;

  case ID_RDP_GetCurStep:
    if (Glob->params.debug >= DEBUG_NORM) {
      fprintf(stderr, "Got GetCurStep Message\n");
    }

    {
      RADPGM_STEP *s = (RADPGM_STEP *) (((RDP_GetCurStep_R *)msg) + 1);
      char *p;

      SwapRADPGM_STEP(s);
      handle_step(s);
      if (Glob->params.debug >= DEBUG_NORM) {
	switch (s->Mode & PMODE_MASK) {
	default: p = ""; break;
	case PMODE_POWERDOWN: p = "Powerdown"; break;
	case PMODE_STANDBY: p = "Standby"; break;
	case PMODE_LOG: p = "Log"; break;
	case PMODE_SINGLE: p = "Single"; break;
	case PMODE_DUAL: p = "Dual"; break;
	}
	fprintf(stderr, "   Radar mode = %s\n", p);
	fprintf(stderr, "     Beam count: %d\n", count);
	fprintf(stderr, "     Range = %d\n", s->Range);
	fprintf(stderr, "     Skip = %d\n", s->Skip);
	fprintf(stderr, "     Duration = %d\n", s->Duration);
	fprintf(stderr, "     Az = %g\n", (s->Az / 16384.0) * 360.0);
	fprintf(stderr, "     El = %g\n", (s->El / 16384.0) * 360.0);
	fprintf(stderr, "     EndPt = %d\n", s->EndPt);
      } /* debug */
    }
    break;

  case ID_RDP_GetMainPgm:
    if (Glob->params.debug >= DEBUG_NORM) {
      fprintf(stderr, "Got GetMainPgm Message\n");
    }
    break;

  case ID_RDP_GetRayParms:
    {
      RDP_GetRayParms_R *parms = (RDP_GetRayParms_R *)msg;
      char *pc = "?";
      ui16 prodCode;
      SwapRDP_GetRayParms_R(parms);
      store_ray_params(parms);
      prodCode = parms->ProductCode & (~PRODCODE_EngUnitsMask);
      if (Glob->params.debug >= DEBUG_NORM) {
	switch (prodCode) {
	case PRODCODE_Log: pc = "Log"; break;
	case PRODCODE_Lin: pc = "Linear"; break;
	case PRODCODE_Vel: pc = "Velocity"; break;
	case PRODCODE_Turb: pc = "Turbulence"; break;
	}
	fprintf(stderr, "Got GetRayParms Message for %s mode\n", pc);
	fprintf(stderr, "  Beam count: %d\n", count);
	fprintf(stderr, "  ProductCode: %d\n", parms->ProductCode);
	fprintf(stderr, "  NumberBins: %d\n", parms->NumberBins);
	fprintf(stderr, "  NumberLevels: %d\n", parms->NumberLevels);
	fprintf(stderr, "  SkipTime: %d\n", parms->SkipTime);
	fprintf(stderr, "  Skip: %g km\n", (parms->SkipTime * 1.5e-4));
	fprintf(stderr, "  BinTime: %d\n", parms->BinTime);
	fprintf(stderr, "  Gate spacing: %g km\n", (parms->BinTime * 1.5e-4));
	fprintf(stderr, "  ThresholdSize: %d\n", parms->ThresholdSize);
	fprintf(stderr, "  ThresholdOffset: %d\n", parms->ThresholdOffset);
      } /* debug */
    }
  break;

  case ID_RDP_GetRayData:
    {
      /* note, we ignore angle data only message for now */
      RDP_GetRayData_R *rayd = (RDP_GetRayData_R *) msg;
      char *pp = "?";
      ui16 prodCode;
      count++;
      SwapRDP_GetRayData_R(rayd);
      prodCode = rayd->ProductCode & (~PRODCODE_EngUnitsMask);
      switch (prodCode) {
      case PRODCODE_Log:
	load_beam(reply_hdr, rayd);
	pp = "Log";
	break;
      case PRODCODE_Lin:
	load_beam(reply_hdr, rayd);
	pp = "Linear";
	break;
      case PRODCODE_Vel:
	load_beam(reply_hdr, rayd);
	pp = "Velocity";
	break;
      case PRODCODE_Turb:
	load_beam(reply_hdr, rayd);
	pp = "Turbulence";
	break;
      case SPECPROD_Angle:
	pp = "Angle info";
	break;
      }
      if (Glob->params.debug >= DEBUG_VERBOSE) {
	fprintf(stderr,
		"** %s ** Beam %d, ProdCode, Compress, Az, El, Offset: "
		"%d, %d, %g, %g, %d\n", pp,
		count, prodCode,
		rayd->CompressType,
		(rayd->Azimuth / 16384.0) * 360.0,
		(rayd->Elevation / 16384.0) * 360.0,
		rayd->OffsetToData);
      } else if (Glob->params.debug >= DEBUG_NORM) {
	if ((count % 45) == 0) {
	  fprintf(stderr,
		  "** %s ** Beam %d, ProdCode, Compress, Az, El, Offset: "
		  "%d, %d, %g, %g, %d\n", pp,
		  count, prodCode,
		  rayd->CompressType,
		  (rayd->Azimuth / 16384.0) * 360.0,
		  (rayd->Elevation / 16384.0) * 360.0,
		  rayd->OffsetToData);
	}
      } /* debug */
    }
  break;

  case ID_RDP_SelectAsync:
    if (Glob->params.debug >= DEBUG_NORM) {
      fprintf(stderr, "Got SelectAsync Message\n");
    }
    break;

  case ID_RDP_GetCfg:
    if (Glob->params.debug >= DEBUG_NORM) {
      fprintf(stderr, "Got GetCfg Message\n");
    }
    break;

  case ID_RDP_CfgNotify:
    if (Glob->params.debug >= DEBUG_NORM) {
      fprintf(stderr, "Got CfgNotify Message\n");
    }
    break;

  case ID_RDP_Overflow:
    if (Glob->params.debug >= DEBUG_NORM) {
      fprintf(stderr, "Got Overflow Message\n");
    }
    break;

  case ID_RDP_GetRawData:
    if (Glob->params.debug >= DEBUG_NORM) {
      fprintf(stderr, "Got GetRawData Message\n");
    }
    break;

  case ID_RDP_ClearMask:
    if (Glob->params.debug >= DEBUG_NORM) {
      fprintf(stderr, "Got ClearMask Message\n");
    }
    break;
    
  case ID_RDP_GetStrikeData:
    if (Glob->params.debug >= DEBUG_NORM) {
      fprintf(stderr, "Got GetStrikeData Message\n");
    }
    break;

  case ID_RDP_SetOutputs:
    if (Glob->params.debug >= DEBUG_NORM) {
      fprintf(stderr, "Got SetOutputs Message\n");
    }
    break;

  case ID_RDP_GetInputs:
    if (Glob->params.debug >= DEBUG_NORM) {
      fprintf(stderr, "Got GetInputs Message\n");
    }
    break;

  } /* switch */

}


