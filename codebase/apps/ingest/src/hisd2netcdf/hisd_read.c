/* ----------------------------------------------------------------------------
	Sample source code for Himawari Standard Data

	Copyright (C) 2014 MSC (Meteorological Satellite Center) of JMA

	Disclaimer:
		MSC does not guarantee regarding the correctness, accuracy, reliability,
		or any other aspect regarding use of these sample codes.

	Detail of Himawari Standard Format: 
		For data structure of Himawari Standard Format, prelese refer to MSC
		Website and Himawari Standard Data User's Guide.

		MSC Website
		http://mscweb.kishou.go.jp/index.htm

		Himawari Standard Data User's Guide
		http://mscweb.kishou.go.jp/himawari89/space_segment/hsd_sample/HS_D_users_guide_en.pdf

	History
		April,  2014 First release
		January,2015 Change for version 1.1
		Febrary,2015 Bug fixed of hisd_getdata_by_pixlin()
		May,    2015 Change for version 1.2
        June,   2015 Fixed bug in function hisd_read_header() (Line 337)

---------------------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "hisd.h"

#define    NORMAL_END  0
#define    ERROR_ALLOCATE  72
#define    ERROR_READ  79
#define    ERROR_DATA 80

static int hisd_comp_table(HisdHeader *header);

/* ----------------------------------------------------------------------------
	hisd_read_header()
 ----------------------------------------------------------------------------*/
int hisd_read_header(HisdHeader *header ,FILE *fp){
	float version;
	char byteFlag;
	int ii;

	if(fp == NULL)return(ERROR_READ);

	/* allocate--------------------------------------------------------------*/
	header->basic		= (Basic_Info      *)calloc(1,sizeof(Basic_Info));
	header->data		= (Data_Info       *)calloc(1,sizeof(Data_Info));
	header->proj		= (Proj_Info       *)calloc(1,sizeof(Proj_Info));
	header->nav			= (Navi_Info       *)calloc(1,sizeof(Navi_Info));
	header->calib		= (Calib_Info      *)calloc(1,sizeof(Calib_Info));
	header->interCalib	= (InterCalib_Info *)calloc(1,sizeof(InterCalib_Info));
	header->seg			= (Segm_Info       *)calloc(1,sizeof(Segm_Info));
	header->navcorr		= (NaviCorr_Info   *)calloc(1,sizeof(NaviCorr_Info));
	header->obstime		= (ObsTime_Info    *)calloc(1,sizeof(ObsTime_Info));
	header->error		= (Error_Info      *)calloc(1,sizeof(Error_Info));
	header->spare		= (Spare           *)calloc(1,sizeof(Spare));
	header->correct_table=(Correct_Table   *)calloc(1,sizeof(Correct_Table));
	if(	header->basic	== NULL || header->data       == NULL ||
		header->proj	== NULL || header->nav        == NULL ||
		header->calib   == NULL || header->interCalib == NULL ||
		header->seg     == NULL || header->navcorr    == NULL ||
		header->obstime == NULL || header->error      == NULL || 
		header->spare   == NULL || 
		header->correct_table == NULL  ){
		fprintf(stderr,"calloc error\n");
		return(ERROR_ALLOCATE);
	}
	/* #1--------------------------------------------------------------------*/
	if( (1>fread(&header->basic->HeaderNum,1,1,fp)) ||
		(1>fread(&header->basic->BlockLen,2,1,fp)) || 
		(1>fread(&header->basic->headerNum,2,1,fp)) ||
		(1>fread(&header->basic->byteOrder,1,1,fp)) ||
		(1>fread(&header->basic->satName,16,1,fp)) ||
		(1>fread(&header->basic->proName,16,1,fp)) ||
		(1>fread(&header->basic->ObsType1,4,1,fp)) ||
		(1>fread(&header->basic->ObsType2,2,1,fp)) ||
		(1>fread(&header->basic->TimeLine,2,1,fp)) ||
		(1>fread(&header->basic->ObsStartTime,8,1,fp)) ||
		(1>fread(&header->basic->ObsEndTime,8,1,fp)) ||
		(1>fread(&header->basic->fileCreationMjd,8,1,fp)) ||
		(1>fread(&header->basic->totalHeaderLen,4,1,fp)) ||
		(1>fread(&header->basic->dataLen,4,1,fp)) ||
		(1>fread(&header->basic->qflag1,1,1,fp)) ||
		(1>fread(&header->basic->qflag2,1,1,fp)) ||
		(1>fread(&header->basic->qflag3,1,1,fp)) ||
		(1>fread(&header->basic->qflag4,1,1,fp)) ||
		(1>fread(&header->basic->verName,32,1,fp)) ||
		(1>fread(&header->basic->fileName,128,1,fp)) ||
		(1>fread(&header->basic->spare,40,1,fp)) ){
		fprintf(stderr,"header #1 read error\n");
		return(ERROR_READ);
	}
	byteFlag=0;
	if(header->basic->BlockLen != 282){ /* byte swap */
		byteFlag=1;
		swapBytes(&header->basic->BlockLen,2,1);
		swapBytes(&header->basic->headerNum,2,1);
		swapBytes(&header->basic->ObsStartTime,8,1);
		swapBytes(&header->basic->ObsEndTime,8,1);
		swapBytes(&header->basic->TimeLine,2,1);
		swapBytes(&header->basic->fileCreationMjd,8,1);
		swapBytes(&header->basic->totalHeaderLen,4,1);
		swapBytes(&header->basic->dataLen,4,1);
	}
	if(	header->basic->HeaderNum   != 1	|| header->basic->BlockLen    !=282	){
		fprintf(stderr,"header #1 read error\n");
		fprintf(stderr,"HeaderNum=%d\n",header->basic->HeaderNum);
		fprintf(stderr,"BlockLen=%d\n" ,header->basic->BlockLen);
		return(ERROR_READ);
	}
	/* #2--------------------------------------------------------------------*/
	if( (1>fread(&header->data->HeaderNum,1,1,fp)) ||
		(1>fread(&header->data->BlockLen,2,1,fp)) ||
		(1>fread(&header->data->bitPix,2,1,fp)) ||
		(1>fread(&header->data->nPix,2,1,fp)) ||
		(1>fread(&header->data->nLin,2,1,fp)) ||
		(1>fread(&header->data->comp,1,1,fp)) ||
		(1>fread(&header->data->spare,40,1,fp)) ){
		fprintf(stderr,"header #2 read error\n");
		return(ERROR_READ);
	} 
	if(byteFlag==1){
		swapBytes(&header->data->BlockLen,2,1);
		swapBytes(&header->data->bitPix,2,1);
		swapBytes(&header->data->nPix,2,1);
		swapBytes(&header->data->nLin,2,1);
	}
	if( header->data->HeaderNum    != 2	|| header->data->BlockLen     != 50	){
		fprintf(stderr,"header #2 read error\n");
		fprintf(stderr,"HeaderNum=%d\n",header->data->HeaderNum);
		fprintf(stderr,"BlockLen=%d\n" ,header->data->BlockLen);
		return(ERROR_READ);
	}
	/* #3--------------------------------------------------------------------*/
	if( (1>fread(&header->proj->HeaderNum,1,1,fp)) || 
		(1>fread(&header->proj->BlockLen,2,1,fp)) ||
		(1>fread(&header->proj->subLon,8,1,fp)) ||
		(1>fread(&header->proj->cfac,4,1,fp)) ||
		(1>fread(&header->proj->lfac,4,1,fp)) ||
		(1>fread(&header->proj->coff,4,1,fp)) ||
		(1>fread(&header->proj->loff,4,1,fp)) ||
		(1>fread(&header->proj->satDis,8,1,fp)) ||
		(1>fread(&header->proj->eqtrRadius,8,1,fp)) ||
		(1>fread(&header->proj->polrRadius,8,1,fp)) ||
		(1>fread(&header->proj->projParam1,8,1,fp)) ||
		(1>fread(&header->proj->projParam2,8,1,fp)) ||
		(1>fread(&header->proj->projParam3,8,1,fp)) ||
		(1>fread(&header->proj->projParamSd,8,1,fp)) ||
		(1>fread(&header->proj->resampleKind,2,1,fp)) ||
		(1>fread(&header->proj->resampleSize,2,1,fp)) ||
		(1>fread(&header->proj->spare,40,1,fp)) ){
		fprintf(stderr,"header #3 read error\n");
		return(ERROR_READ);
	}
	if(byteFlag==1){
		swapBytes(&header->proj->BlockLen,2,1);
		swapBytes(&header->proj->subLon,8,1);
		swapBytes(&header->proj->cfac,4,1);
		swapBytes(&header->proj->lfac,4,1);
		swapBytes(&header->proj->coff,4,1);
		swapBytes(&header->proj->loff,4,1);
		swapBytes(&header->proj->satDis,8,1);
		swapBytes(&header->proj->eqtrRadius,8,1);
		swapBytes(&header->proj->polrRadius,8,1);
		swapBytes(&header->proj->projParam1,8,1);
		swapBytes(&header->proj->projParam2,8,1);
		swapBytes(&header->proj->projParam3,8,1);
		swapBytes(&header->proj->projParamSd,8,1);
		swapBytes(&header->proj->resampleKind,2,1);
		swapBytes(&header->proj->resampleSize,2,1);
	}
	if( header->proj->HeaderNum    != 3	|| header->proj->BlockLen     !=127	){
		fprintf(stderr,"header #3 read error\n");
		fprintf(stderr,"HeaderNum=%d\n",header->proj->HeaderNum);
		fprintf(stderr,"BlockLen=%d\n" ,header->proj->BlockLen);
		return(ERROR_READ);
	}
	/* #4--------------------------------------------------------------------*/
	if( (1>fread(&header->nav->HeaderNum,1,1,fp)) ||
		(1>fread(&header->nav->BlockLen,2,1,fp)) ||
		(1>fread(&header->nav->navMjd,8,1,fp)) ||
		(1>fread(&header->nav->sspLon,8,1,fp)) ||
		(1>fread(&header->nav->sspLat,8,1,fp)) ||
		(1>fread(&header->nav->satDis,8,1,fp)) ||
		(1>fread(&header->nav->nadirLon,8,1,fp)) ||
		(1>fread(&header->nav->nadirLat,8,1,fp)) ||
		(1>fread(&header->nav->sunPos_x,8,1,fp)) ||
		(1>fread(&header->nav->sunPos_y,8,1,fp)) ||
		(1>fread(&header->nav->sunPos_z,8,1,fp)) ||
		(1>fread(&header->nav->moonPos_x,8,1,fp)) ||
		(1>fread(&header->nav->moonPos_y,8,1,fp)) ||
		(1>fread(&header->nav->moonPos_z,8,1,fp)) ||
		(1>fread(&header->nav->spare,40,1,fp)) ){
		fprintf(stderr,"header #4 read error\n");
		return(1);
		return(ERROR_READ);
	}
	if(byteFlag==1){
		swapBytes(&header->nav->BlockLen,2,1);
		swapBytes(&header->nav->navMjd,8,1);
		swapBytes(&header->nav->sspLon,8,1);
		swapBytes(&header->nav->sspLat,8,1);
		swapBytes(&header->nav->satDis,8,1);
		swapBytes(&header->nav->nadirLon,8,1);
		swapBytes(&header->nav->nadirLat,8,1);
		swapBytes(&header->nav->sunPos_x,8,1);
		swapBytes(&header->nav->sunPos_y,8,1);
		swapBytes(&header->nav->sunPos_z,8,1);
		swapBytes(&header->nav->moonPos_x,8,1);
		swapBytes(&header->nav->moonPos_y,8,1);
		swapBytes(&header->nav->moonPos_z,8,1);
	}
	if( header->nav->HeaderNum     != 4	|| header->nav->BlockLen      !=139	){
		fprintf(stderr,"header #4 read error\n");
		fprintf(stderr,"HeaderNum=%d\n",header->nav->HeaderNum);
		fprintf(stderr,"BlockLen=%d\n" ,header->nav->BlockLen);
		return(ERROR_READ);
	}
	/* #5--------------------------------------------------------------------*/
	if( (1>fread(&header->calib->HeaderNum,1,1,fp)) ||
		(1>fread(&header->calib->BlockLen,2,1,fp)) ||
		(1>fread(&header->calib->bandNo,2,1,fp)) ||
		(1>fread(&header->calib->waveLen,8,1,fp)) ||
		(1>fread(&header->calib->bitPix,2,1,fp)) ||
		(1>fread(&header->calib->errorCount,2,1,fp)) ||
		(1>fread(&header->calib->outCount,2,1,fp)) ||
		(1>fread(&header->calib->gain_cnt2rad,8,1,fp)) ||
		(1>fread(&header->calib->cnst_cnt2rad,8,1,fp)) ){
		fprintf(stderr,"header #5 read error\n");
		return(ERROR_READ);
	}
	if(byteFlag==1){
		swapBytes(&header->calib->BlockLen,2,1);
		swapBytes(&header->calib->bandNo,2,1);
		swapBytes(&header->calib->waveLen,8,1);
		swapBytes(&header->calib->bitPix,2,1);
		swapBytes(&header->calib->errorCount,2,1);
		swapBytes(&header->calib->outCount,2,1);
		swapBytes(&header->calib->gain_cnt2rad,8,1);
		swapBytes(&header->calib->cnst_cnt2rad,8,1);
	}
	if(	(header->calib->bandNo>=7 &&
		 strstr(header->basic->satName,"Himawari")!=NULL ) ||
		(header->calib->bandNo>=2 &&
		 strstr(header->basic->satName,"MTSAT-2") !=NULL )	){
		if( (1>fread(&header->calib->rad2btp_c0,8,1,fp)) || 
			(1>fread(&header->calib->rad2btp_c1,8,1,fp)) ||
			(1>fread(&header->calib->rad2btp_c2,8,1,fp)) ||
			(1>fread(&header->calib->btp2rad_c0,8,1,fp)) ||
			(1>fread(&header->calib->btp2rad_c1,8,1,fp)) ||
			(1>fread(&header->calib->btp2rad_c2,8,1,fp)) ||
			(1>fread(&header->calib->lightSpeed,8,1,fp)) ||
			(1>fread(&header->calib->planckConst,8,1,fp)) ||
			(1>fread(&header->calib->bolzConst,8,1,fp)) ||
			(1>fread(&header->calib->spare,40,1,fp)) ){
			fprintf(stderr,"header #5 read error\n");
			return(ERROR_READ);
		}
		if(byteFlag==1){
			swapBytes(&header->calib->rad2btp_c0,8,1);
			swapBytes(&header->calib->rad2btp_c1,8,1);
			swapBytes(&header->calib->rad2btp_c2,8,1);
			swapBytes(&header->calib->btp2rad_c0,8,1);
			swapBytes(&header->calib->btp2rad_c1,8,1);
			swapBytes(&header->calib->btp2rad_c2,8,1);
			swapBytes(&header->calib->lightSpeed,8,1);
			swapBytes(&header->calib->planckConst,8,1);
			swapBytes(&header->calib->bolzConst,8,1);
		}
	}else{
		if( (1>fread(&header->calib->rad2albedo,8,1,fp)) ||
			(1>fread(&header->calib->spareV,104,1,fp))){
			fprintf(stderr,"header #5 read error\n");
			return(ERROR_READ);
		}
		if(byteFlag==1){
			swapBytes(&header->calib->rad2albedo,8,1);
		}
	}
	if( header->calib->HeaderNum   != 5	|| header->calib->BlockLen    !=147	){
			fprintf(stderr,"header #5 read error\n");
			fprintf(stderr,"HeaderNum=%d\n",header->calib->HeaderNum);
			fprintf(stderr,"BlockLen=%d\n" ,header->calib->BlockLen);
			return(ERROR_READ);
		}
	/* #6--------------------------------------------------------------------*/
	version = atof(header->basic->verName);
	header->interCalib->gsicsCorr_C = INVALID_VALUE;
	header->interCalib->gsicsCorr_1 = INVALID_VALUE;
	header->interCalib->gsicsCorr_2 = INVALID_VALUE;
	header->interCalib->gsicsBias   = INVALID_VALUE;
	header->interCalib->gsicsUncert = INVALID_VALUE;
	header->interCalib->gsicsStscene     = INVALID_VALUE;
	header->interCalib->gsicsCorr_StrMJD = INVALID_VALUE;
	header->interCalib->gsicsCorr_EndMJD = INVALID_VALUE;
	header->interCalib->gsicsUpperLimit  = INVALID_VALUE;
	header->interCalib->gsicsLowerLimit  = INVALID_VALUE;
	header->interCalib->gsicsCorr_C_er   = INVALID_VALUE;
	header->interCalib->gsicsCorr_1_er   = INVALID_VALUE;
	header->interCalib->gsicsCorr_2_er   = INVALID_VALUE;
	if(version >  1.1){
		/* for version 1.2 */
		if( (1>fread(&header->interCalib->HeaderNum,1,1,fp)) ||
			(1>fread(&header->interCalib->BlockLen,2,1,fp)) ||
			(1>fread(&header->interCalib->gsicsCorr_C,8,1,fp)) ||
			(1>fread(&header->interCalib->gsicsCorr_1,8,1,fp)) ||
			(1>fread(&header->interCalib->gsicsCorr_2,8,1,fp)) ||
			(1>fread(&header->interCalib->gsicsBias,8,1,fp)) ||
			(1>fread(&header->interCalib->gsicsUncert,8,1,fp)) ||
			(1>fread(&header->interCalib->gsicsStscene,8,1,fp)) ||
			(1>fread(&header->interCalib->gsicsCorr_StrMJD,8,1,fp)) ||
			(1>fread(&header->interCalib->gsicsCorr_EndMJD,8,1,fp)) ||
			(1>fread(&header->interCalib->gsicsUpperLimit,4,1,fp)) ||
			(1>fread(&header->interCalib->gsicsLowerLimit,4,1,fp)) ||
			(1>fread(&header->interCalib->gsicsFilename,128,1,fp)) ||
			(1>fread(&header->interCalib->spare,56,1,fp)) ){
			fprintf(stderr,"header #6 read error\n");
			return(ERROR_READ);
		}
		if(byteFlag==1){
			swapBytes(&header->interCalib->BlockLen,2,1);
			swapBytes(&header->interCalib->gsicsCorr_2,8,1);
			swapBytes(&header->interCalib->gsicsCorr_1,8,1);
			swapBytes(&header->interCalib->gsicsCorr_C,8,1);
			swapBytes(&header->interCalib->gsicsBias,8,1);
			swapBytes(&header->interCalib->gsicsUncert,8,1);
			swapBytes(&header->interCalib->gsicsStscene,8,1);
			swapBytes(&header->interCalib->gsicsCorr_StrMJD,8,1);
			swapBytes(&header->interCalib->gsicsCorr_EndMJD,8,1);
			swapBytes(&header->interCalib->gsicsUpperLimit,4,1);
			swapBytes(&header->interCalib->gsicsLowerLimit,4,1);	/* 2015.06.18 add */	
		}
	}else if(version >  1.0){
		/* for version 1.1 */
		if( (1>fread(&header->interCalib->HeaderNum,1,1,fp)) ||
			(1>fread(&header->interCalib->BlockLen,2,1,fp)) ||
			(1>fread(&header->interCalib->gsicsCorr_C,8,1,fp)) ||
			(1>fread(&header->interCalib->gsicsCorr_C_er,8,1,fp)) ||
			(1>fread(&header->interCalib->gsicsCorr_1,8,1,fp)) ||
			(1>fread(&header->interCalib->gsicsCorr_1_er,8,1,fp)) ||
			(1>fread(&header->interCalib->gsicsCorr_2,8,1,fp)) ||
			(1>fread(&header->interCalib->gsicsCorr_2_er,8,1,fp)) ||
			(1>fread(&header->interCalib->gsicsCorr_StrMJD,8,1,fp)) ||
			(1>fread(&header->interCalib->gsicsCorr_EndMJD,8,1,fp)) ||
			(1>fread(&header->interCalib->gsicsUpperLimit,4,1,fp)) ||
			(1>fread(&header->interCalib->gsicsLowerLimit,4,1,fp)) ||
			(1>fread(&header->interCalib->gsicsFilename,128,1,fp)) ||
			(1>fread(&header->interCalib->spare,56,1,fp)) ){
			fprintf(stderr,"header #6 read error\n");
			return(ERROR_READ);
		}
		if(byteFlag==1){
			swapBytes(&header->interCalib->BlockLen,2,1);
			swapBytes(&header->interCalib->gsicsCorr_2,8,1);
			swapBytes(&header->interCalib->gsicsCorr_2_er,8,1);
			swapBytes(&header->interCalib->gsicsCorr_1,8,1);
			swapBytes(&header->interCalib->gsicsCorr_1_er,8,1);
			swapBytes(&header->interCalib->gsicsCorr_C,8,1);
			swapBytes(&header->interCalib->gsicsCorr_C_er,8,1);
			swapBytes(&header->interCalib->gsicsCorr_StrMJD,8,1);
			swapBytes(&header->interCalib->gsicsCorr_EndMJD,8,1);
			swapBytes(&header->interCalib->gsicsUpperLimit,4,1);
			swapBytes(&header->interCalib->gsicsLowerLimit,4,1);	
		}
	}else{
		/* for version 1.0 and 0.0 */
		if( (1>fread(&header->interCalib->HeaderNum,1,1,fp)) ||
			(1>fread(&header->interCalib->BlockLen,2,1,fp)) ||
			(1>fread(&header->interCalib->gsicsCorr_C,8,1,fp)) ||
			(1>fread(&header->interCalib->gsicsCorr_C_er,8,1,fp)) ||
			(1>fread(&header->interCalib->gsicsCorr_1,8,1,fp)) ||
			(1>fread(&header->interCalib->gsicsCorr_1_er,8,1,fp)) ||
			(1>fread(&header->interCalib->gsicsCorr_2,8,1,fp)) ||
			(1>fread(&header->interCalib->gsicsCorr_2_er,8,1,fp)) ||
			(1>fread(&header->interCalib->gsicsCorr_StrMJD,8,1,fp)) ||
			(1>fread(&header->interCalib->gsicsCorr_EndMJD,8,1,fp)) ||
			(1>fread(&header->interCalib->gsicsCorrInfo,1,64,fp)) ||
			(1>fread(&header->interCalib->spare,128,1,fp)) ){
			fprintf(stderr,"header #6 read error\n");
			return(ERROR_READ);
		}
		if(byteFlag==1){
			swapBytes(&header->interCalib->BlockLen,2,1);
			swapBytes(&header->interCalib->gsicsCorr_2,8,1);
			swapBytes(&header->interCalib->gsicsCorr_2_er,8,1);
			swapBytes(&header->interCalib->gsicsCorr_1,8,1);
			swapBytes(&header->interCalib->gsicsCorr_1_er,8,1);
			swapBytes(&header->interCalib->gsicsCorr_C,8,1);
			swapBytes(&header->interCalib->gsicsCorr_C_er,8,1);
			swapBytes(&header->interCalib->gsicsCorr_StrMJD,8,1);
			swapBytes(&header->interCalib->gsicsCorr_EndMJD,8,1);
		}
	}
	if( header->interCalib->HeaderNum != 6 || 
		header->interCalib->BlockLen !=259 ){
		fprintf(stderr,"header #6 read error\n");
		fprintf(stderr,"HeaderNum=%d\n",header->interCalib->HeaderNum);
		fprintf(stderr,"BlockLen=%d\n" ,header->interCalib->BlockLen);
		return(ERROR_READ);
	}
	/* #7--------------------------------------------------------------------*/
	if( (1>fread(&header->seg->HeaderNum,1,1,fp)) ||
		(1>fread(&header->seg->BlockLen,2,1,fp)) ||
		(1>fread(&header->seg->totalSegNum,1,1,fp)) ||
		(1>fread(&header->seg->segSeqNo,1,1,fp)) ||
		(1>fread(&header->seg->strLineNo,2,1,fp)) ||
		(1>fread(&header->seg->spare,40,1,fp)) ){
		fprintf(stderr,"header #7 read error\n");
		return(ERROR_READ);
	}
	if(byteFlag==1){
		swapBytes(&header->seg->BlockLen,2,1);
		swapBytes(&header->seg->strLineNo,2,1);
	}
	if( header->seg->HeaderNum     != 7	|| header->seg->BlockLen      !=47 ){
		fprintf(stderr,"header #7 read error\n");
		fprintf(stderr,"HeaderNum=%d\n",header->seg->HeaderNum);
		fprintf(stderr,"BlockLen=%d\n" ,header->seg->BlockLen);
		return(ERROR_READ);
	}
	/* #8--------------------------------------------------------------------*/
	if( (1>fread(&header->navcorr->HeaderNum,1,1,fp)) ||
		(1>fread(&header->navcorr->BlockLen,2,1,fp)) ||
		(1>fread(&header->navcorr->RoCenterColumn,4,1,fp)) ||
		(1>fread(&header->navcorr->RoCenterLine,4,1,fp)) ||
		(1>fread(&header->navcorr->RoCorrection,8,1,fp)) ||
		(1>fread(&header->navcorr->correctNum,2,1,fp)) ){
		fprintf(stderr,"header #8 read error\n");
		return(ERROR_READ);
	}
	if(byteFlag==1){
		swapBytes(&header->navcorr->BlockLen,2,1);
		swapBytes(&header->navcorr->RoCenterColumn,4,1);
		swapBytes(&header->navcorr->RoCenterLine,4,1);
		swapBytes(&header->navcorr->RoCorrection,8,1);
		swapBytes(&header->navcorr->correctNum,2,1);
	}
	header->navcorr->lineNo=(unsigned short *)
		calloc(header->navcorr->correctNum,sizeof(unsigned short));
	header->navcorr->columnShift=(float *)
		calloc(header->navcorr->correctNum,sizeof(float));
	header->navcorr->lineShift=(float *)
		calloc(header->navcorr->correctNum,sizeof(float));
	if(header->navcorr->lineNo      == NULL ||
	   header->navcorr->columnShift == NULL ||
	   header->navcorr->lineShift   == NULL ){
		fprintf(stderr,"calloc error\n");
		return(ERROR_READ);
	}
	for(ii=0;ii<header->navcorr->correctNum;ii++){
		if( (1>fread(&header->navcorr->lineNo[ii],2,1,fp)) ||
			(1>fread(&header->navcorr->columnShift[ii],4,1,fp)) ||
			(1>fread(&header->navcorr->lineShift[ii],4,1,fp)) ){
			fprintf(stderr,"header #8 read error\n");
			return(ERROR_READ);
		}
	}
	if( (1>fread(&header->navcorr->spare,40,1,fp))){
		fprintf(stderr,"header #8 read error\n");
		return(ERROR_READ);
	}
	if(byteFlag==1){
		swapBytes(&header->navcorr->lineNo[0],2,header->navcorr->correctNum);
		swapBytes(&header->navcorr->columnShift[0],4,
			header->navcorr->correctNum);
		swapBytes(&header->navcorr->lineShift[0],4,header->navcorr->correctNum);
	}
	if( header->navcorr->HeaderNum != 8	){
		fprintf(stderr,"header #8 read error\n");
		fprintf(stderr,"HeaderNum=%d\n",header->navcorr->HeaderNum);
		fprintf(stderr,"BlockLen=%d\n" ,header->navcorr->BlockLen);
		return(ERROR_READ);
	}
	/* #9--------------------------------------------------------------------*/
	if( (1>fread(&header->obstime->HeaderNum,1,1,fp)) ||
		(1>fread(&header->obstime->BlockLen,2,1,fp)) ||
		(1>fread(&header->obstime->obsNum,2,1,fp)) ){
		fprintf(stderr,"header #9 read error\n");
		return(ERROR_READ);
	}
	if(byteFlag==1){
		swapBytes(&header->obstime->BlockLen,2,1);
		swapBytes(&header->obstime->obsNum,2,1);
	}
	header->obstime->lineNo=(unsigned short *)
		calloc(header->obstime->obsNum,sizeof(unsigned short));
	header->obstime->obsMJD=(double *)
		calloc(header->obstime->obsNum,sizeof(double));
	if(header->obstime->lineNo == NULL ||
	   header->obstime->obsMJD == NULL ){
		fprintf(stderr,"calloc error\n");
		return(ERROR_READ);
	}
	for(ii=0;ii<header->obstime->obsNum;ii++){
		if( (1>fread(&header->obstime->lineNo[ii],2,1,fp)) ||
			(1>fread(&header->obstime->obsMJD[ii],8,1,fp)) ){
			fprintf(stderr,"header #9 read error\n");
			return(ERROR_READ);
		}
	}
	if( (1>fread(&header->obstime->spare,40,1,fp)) ){
		fprintf(stderr,"header #9 read error\n");
		return(ERROR_READ);
	}
	if(byteFlag==1){
		swapBytes(&header->obstime->lineNo[0],2,header->obstime->obsNum);
		swapBytes(&header->obstime->obsMJD[0],8,header->obstime->obsNum);
	}
	if( header->obstime->HeaderNum != 9	){
		fprintf(stderr,"header #9 read error\n");
		fprintf(stderr,"HeaderNum=%d\n",header->obstime->HeaderNum);
		fprintf(stderr,"BlockLen=%d\n" ,header->obstime->BlockLen);
		return(ERROR_READ);
	}
	/* #10-------------------------------------------------------------------*/
	if( (1>fread(&header->error->HeaderNum,1,1,fp)) ||
		(1>fread(&header->error->BlockLen,4,1,fp)) ||
		(1>fread(&header->error->errorNum,2,1,fp)) ){
		fprintf(stderr,"header #10 read error\n");
		return(ERROR_READ);
	}
	if(byteFlag==1){
		swapBytes(&header->error->BlockLen,4,1);
		swapBytes(&header->error->errorNum,2,1);
	}
	if(header->error->errorNum>=1){
		header->error->lineNo=(unsigned short *)
			calloc(header->error->errorNum,sizeof(unsigned short));
		header->error->errPixNum=(unsigned short *)
			calloc(header->error->errorNum,sizeof(unsigned short));
		if(header->error->lineNo == NULL ||
		   header->error->errPixNum == NULL ){
			fprintf(stderr,"calloc error\n");
			return(ERROR_READ);
		}
		for(ii=0;ii<header->error->errorNum;ii++){
			if( (1>fread(&header->error->lineNo[ii],2,1,fp)) ||
				(1>fread(&header->error->errPixNum[ii],2,1,fp)) ){
				fprintf(stderr,"header #10 read error\n");
				return(ERROR_READ);
			}
		}
	}
	if( (1>fread(&header->error->spare,40,1,fp)) ){
		fprintf(stderr,"header #10 read error\n");
		return(ERROR_READ);
	}
	if(byteFlag==1){
		swapBytes(&header->error->lineNo[0],2,header->error->errorNum);
		swapBytes(&header->error->errPixNum[0],2,header->error->errorNum);
	}
	if( header->error->HeaderNum   !=10	){
		fprintf(stderr,"header #10 read error\n");
		fprintf(stderr,"HeaderNum=%d\n",header->error->HeaderNum);
		fprintf(stderr,"BlockLen=%d\n" ,header->error->BlockLen);
		return(ERROR_READ);
	}
	/* #11-------------------------------------------------------------------*/
	if( (1>fread(&header->spare->HeaderNum,1,1,fp)) ||
		(1>fread(&header->spare->BlockLen,2,1,fp)) ||
		(1>fread(&header->spare->spare,256,1,fp)) ){
		fprintf(stderr,"header #11 read error\n");
		return(ERROR_READ);
	}
	if(byteFlag==1){
		swapBytes(&header->spare->BlockLen,2,1);
	}
	if( header->spare->HeaderNum   !=11	|| header->spare->BlockLen    !=259 ){
		fprintf(stderr,"header #11 read error\n");
		fprintf(stderr,"HeaderNum=%d\n",header->spare->HeaderNum);
		fprintf(stderr,"BlockLen=%d\n" ,header->spare->BlockLen);
		return(ERROR_READ);
	}
	/* ----------------------------------------------------------------------*/
	return(NORMAL_END);
}

/* ----------------------------------------------------------------------------
	hisd_free()
 ----------------------------------------------------------------------------*/
int hisd_free(HisdHeader *header){

	if(header->navcorr->lineNo      != NULL){ free(header->navcorr->lineNo);     }
	if(header->navcorr->columnShift != NULL){ free(header->navcorr->columnShift);}
	if(header->navcorr->lineShift   != NULL){ free(header->navcorr->lineShift);  }
	if(header->obstime->lineNo      != NULL){ free(header->obstime->lineNo);     }
	if(header->obstime->obsMJD      != NULL){ free(header->obstime->obsMJD);     }
	if(header->error->lineNo        != NULL){ free(header->error->lineNo);       }
	if(header->error->errPixNum     != NULL){ free(header->error->errPixNum);    }

	if(header->correct_table->cmpCoff != NULL){ free(header->correct_table->cmpCoff);}
	if(header->correct_table->cmpLoff != NULL){ free(header->correct_table->cmpLoff);}
	if(header->correct_table          != NULL){ free(header->correct_table);}

    if(header->basic      != NULL)	{ free(header->basic); 		}
    if(header->data       != NULL)	{ free(header->data);		}
    if(header->proj       != NULL)	{ free(header->proj);		}
    if(header->nav        != NULL)	{ free(header->nav);		}
    if(header->calib      != NULL)	{ free(header->calib);		}
    if(header->interCalib != NULL)	{ free(header->interCalib);	}
    if(header->seg        != NULL)	{ free(header->seg);		}
    if(header->navcorr    != NULL)	{ free(header->navcorr);	}
    if(header->obstime    != NULL)	{ free(header->obstime);	}
    if(header->error      != NULL)	{ free(header->error);		}
    if(header->spare      != NULL)	{ free(header->spare);		}

    return(NORMAL_END);
}

/* ----------------------------------------------------------------------------
	hisd_getdata_by_pixlin()
 ----------------------------------------------------------------------------*/
int hisd_getdata_by_pixlin(HisdHeader *header,FILE *fp, float hPix,float hLin, 
	unsigned short *sout){

	float  pShift,lShift;
	int    ii;

	float  R_C = header->navcorr->RoCenterColumn;
	float  R_L = header->navcorr->RoCenterLine;
	double R_A = header->navcorr->RoCorrection / 1000. / 1000.;

	// init
	*sout=header->calib->outCount;	// count value of out of scan area

	// shift correction table
	if(header->correct_table->flag == 0){
		hisd_comp_table(header);
	}

	if(header->correct_table->flag == 1){
		// shift amount for column and line direction
		ii = (short)(hLin+0.5) - header->correct_table->startLineNo;
		if(ii < 0 ){
			ii = 0;
		}else if(header->correct_table->lineNum -1 < ii){
			ii = header->correct_table->lineNum -1;
		}
		pShift = header->correct_table->cmpCoff[ii];
		lShift = header->correct_table->cmpLoff[ii];

		// shift correction
		hPix = hPix - pShift;
		hLin = hLin - lShift;
	}

	// rotational correction
	float hLin2=    (hLin - R_L) * cos(R_A) - (hPix - R_C) * sin(R_A) + R_L;
	float hPix2=    (hPix - R_C) * cos(R_A) + (hLin - R_L) * sin(R_A) + R_C;
	hLin = hLin2;
	hPix = hPix2;

	// Nearest Neighbor
	int ll = (int)(hLin + 0.5) - header->seg->strLineNo ;
//	int pp = (int)(hPix + 0.5);
	int pp = (int)(hPix + 0.5) -1;  // Feb.2015 bug fixed

	// check ll and pp
	if(ll < 0 || header->data->nLin < ll){
		return(ERROR_DATA);
	}
	if(pp < 0 || header->data->nPix < pp){
		return(ERROR_DATA);
	}

	// read data
	unsigned long seek_pt;
	seek_pt = header->basic->totalHeaderLen 
					+ (ll * header->data->nPix + pp ) * sizeof(unsigned short);
	if(seek_pt > header->basic->totalHeaderLen + header->basic->dataLen){
		return(ERROR_DATA);
	}
	fseek(fp,seek_pt,SEEK_SET);
	fread(sout,sizeof(unsigned short), 1,fp);
	
	/* byte swap */
	if(byteOrder() != header->basic->byteOrder ){
		swapBytes(sout,sizeof(unsigned short),1);
	}
	return(NORMAL_END);
}

/* ----------------------------------------------------------------------------
	hisd_radiance_to_tbb()
 ----------------------------------------------------------------------------*/
void hisd_radiance_to_tbb(HisdHeader *header,double radiance,double *tbb){
	
	double effective_temperature;

	/* central wave length */
	double lambda = header->calib->waveLen / 1000000.0; // [micro m] => [m]
	/* radiance */
	radiance = radiance * 1000000.0; // [ W/(m^2 sr micro m)] => [ W/(m^2 sr m)]
	/* planck_c1 = (2 * h * c^2 / lambda^5) */
	double planck_c1= 2.0 * header->calib->planckConst * 
				pow(header->calib->lightSpeed,2.0) /
				pow(lambda,5.0) ;
	/* planck_c2 = (h * c / k / lambda ) */
	double planck_c2= header->calib->planckConst * header->calib->lightSpeed /
				header->calib->bolzConst / lambda ;

	if(radiance > 0 ){
		effective_temperature =     planck_c2 /
								log( (planck_c1 / radiance ) + 1.0 );
		*tbb = header->calib->rad2btp_c0 +
			   header->calib->rad2btp_c1 * effective_temperature +
			   header->calib->rad2btp_c2 * pow(effective_temperature,2.0);
	}else{
		*tbb = INVALID_VALUE;
	}
}
/* ----------------------------------------------------------------------------
	hisd_comp_table()
 ----------------------------------------------------------------------------*/
static int hisd_comp_table(HisdHeader *header){
	int ii,jj;
	int lineNo,lineNo1,lineNo2;
	float coff1,coff2,loff1,loff2;

	// allocate
	if(header->correct_table==NULL){
		header->correct_table = (Correct_Table *)calloc(1,sizeof(Correct_Table));
		if(header->correct_table == NULL){
			fprintf(stderr,"calloc error\n");
			return(ERROR_ALLOCATE);
		}
	}

	// make table
	if(header->navcorr->correctNum <2){
		return(ERROR_DATA); 	// check correctNum
	}else{
		header->correct_table->startLineNo = header->navcorr->lineNo[0];
		header->correct_table->lineNum = 
			header->navcorr->lineNo[header->navcorr->correctNum-1] -
			header->navcorr->lineNo[0] +1;

		if(header->correct_table->lineNum < 2){
			return(ERROR_DATA);	// check lineNum
		}

		// allocate
		header->correct_table->cmpCoff = 
			(float *)calloc(header->correct_table->lineNum,sizeof(float));
		header->correct_table->cmpLoff = 
			(float *)calloc(header->correct_table->lineNum,sizeof(float));
		if(header->correct_table->cmpCoff == NULL ||
		   header->correct_table->cmpLoff == NULL	){
			fprintf(stderr,"calloc error\n");
			return(ERROR_ALLOCATE);
		}

		// init
		for(ii=0;ii<header->correct_table->lineNum;ii++){
			header->correct_table->cmpCoff[ii] = 0.0;
			header->correct_table->cmpLoff[ii] = 0.0;
		}
		// navigation correction table
		for(ii=0;ii<header->navcorr->correctNum-1;ii++){
			lineNo1 = (int)header->navcorr->lineNo[ii];
			lineNo2 = (int)header->navcorr->lineNo[ii+1];
			coff1   = header->navcorr->columnShift[ii];
			coff2   = header->navcorr->columnShift[ii+1];
			loff1   = header->navcorr->lineShift[ii];
			loff2   = header->navcorr->lineShift[ii+1];
			
			for(lineNo=lineNo1;lineNo<=lineNo2;lineNo++){
				jj = lineNo - header->correct_table->startLineNo;
				if( 0<= jj && jj <= header->correct_table->lineNum){
					header->correct_table->cmpCoff[jj] = coff1 +
						(coff2   - coff1   ) /
						(float)(lineNo2 - lineNo1 ) * (float)(lineNo - lineNo1);
					header->correct_table->cmpLoff[jj] = loff1 +
						(loff2   - loff1   ) /
						(float)(lineNo2 - lineNo1 ) * (float)(lineNo - lineNo1);
				}
			}
		}
	}
	header->correct_table->flag = 1;

	return(NORMAL_END);
}

int swapBytes(
	void *buf,  /**< inout  */
	int size,   /**< in  */
	int nn )    /**< in  */
{
	char *ba, *bb, *buf2 = buf;
	while( nn-- ) {
		bb = ( ba = buf2 ) + size -1;
		do {
			char a;
			a   = *ba;
			*ba = *bb;
			*bb =  a;
		} while( ++ba < --bb );
		buf2 += size;
	}
	return 0;
}

int byteOrder(void){
	int i=1;
	if( *((char *)&i) ) return 0;	/*  Little */
	else if( *( (char *)&i + (sizeof(int)-1) ) ) return 1; /* Big */
	else return -1;
}
