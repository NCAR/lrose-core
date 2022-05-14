/* ----------------------------------------------------------------------------
	Sample source code for Himawari Satandard Data

	Copyright (C) 2015 MSC (Meteorological Satellite Center) of JMA

	Disclaimer:
		MSC does not guarantee regarding the correctness, accuracy, reliability,
		or any other aspect regarding use of these sample codes.

	Detail of Himawari Standard Format:
		For data structure of Himawari Standard Format, prelese refer to MSC
		Website and Himawari Standard Data User's Guide.

		MSC Website
		https://www.data.jma.go.jp/mscweb/en/index.html

		Himawari Standard Data User's Guide
		https://www.data.jma.go.jp/mscweb/en/himawari89/space_segment/hsd_sample/HS_D_users_guide_en_v13.pdf

	History
		March,  2015  First release
		June,   2015  Version 2015-06
                       Fixed bug in getData() (3-4 get count value)
                       Fixed bug in function function hisd_read_header() (hisd_read.c)
                       Fixed bug in fucntion lonlat_to_pixlin() (hisd_pixlin2lonlat.c)
                       ((8) check the reverse side of the Earth)
		July,   2020  Version 2020-07
			Fixed bug in function defNetcdf() function (main.c)
		June,	2021  Version 2021-06
			Fixed buf in function defNetcdf() function (main.c)
					      getData() function (main.c)

---------------------------------------------------------------------------- */
# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <time.h>
# include <math.h>

# include "netcdf.h"
# include "hisd.h"
# include "hisd2netcdf.h"
# include "date_utl.h"

# define  MAXFILE   10
# define  INVALID   -1

# define  WIDTH     2001        /* default pixel number */
# define  HEIGHT    2001        /* default line number */
# define  LTLON     100.0       /* default left top longitude */
# define  LTLAT     45.0        /* default left top latitude */
# define  DLON      0.02        /* default Spatial resolution (longitude) */
# define  DLAT      0.02        /* default Spatial resolution (latitude) */

typedef struct{
	char	*InFile[MAXFILE+1];
	char	*OutFile;
	char	filenum;
}argument;

typedef struct{
	float	*lon;
	float	*lat;
	float	*phys;
	double	startTime;
	double	endTime;
	char	satName[32];
}outdata;

typedef struct{
	short	width;		/* -width    pixel number */
	short	height;		/* -height   line number  */
	double	ltlon;		/* -lon      left top longitude */
	double	ltlat;		/* -lat      left top latitude  */
	double	dlon;		/* -dlon     Spatial resolution (longitude) */
	double	dlat;		/* -dlat     Spatial resolution (latitude) */
	short	band;
}parameter;

typedef struct{
	int		ncid;
	int		latVarId;
	int		lonVarId;
	int		physVarId;
	int		startTimeVarId;
	int		endTimeVarId;
}ncparam;

/* ------------------------------------------------------------------------- */
int getArg(int argc, char **argv, argument *arg,parameter *param);
int getData(argument *arg,parameter *param,outdata *data);
int makeNetCDF(char *OutFile,parameter *param,outdata *data);
int	defNetcdf(parameter *param, short ch ,ncparam  *nc, char *satName, 
	char *outFile);
int putNetcdf(ncparam *nc,parameter *param,outdata *data);

/* ---------------------------------------------------------------------------
  getArg()
 -----------------------------------------------------------------------------*/
int getArg(int argc, char **argv, argument *arg,parameter *param){
	char	*ptr;
	int		ii,nn=0;
	/* 1 init */
	arg->OutFile = NULL;
	for(ii=0;ii<MAXFILE;ii++){
		arg->InFile[ii] = NULL;
	}
	param->ltlat = LTLAT;	/* left top longitude */
	param->ltlon = LTLON;	/* left top longitude */
	param->width = WIDTH;	/* pixel number  */
	param->height= HEIGHT;	/* line number  */
	param->dlon  = DLON;	/* Spatial resolution (longitude) */
	param->dlat  = DLAT;	/* Spatial resolution (latitude) */
	/* 2 get arguments */
	for(ii=1;ii<argc-1;ii++){
		ptr=argv[ii];
		if(*ptr=='-'){
			ptr++;
			switch (*ptr){
				case 'o':   /* output file */
					arg->OutFile = argv[ii+1];
					ii++;
					break;
				case 'i':	/* input file */
					if(nn<MAXFILE){
						arg->InFile[nn] = argv[ii+1];
						nn++;
					}else{
						fprintf(stderr,"InFile : %s [%d/%d]\n",
							argv[ii+1],nn,MAXFILE);
					}
					ii++;
					break;
				case 'd':
				case 'l':
				case 'w': 
				case 'h':
					if(!strcmp(ptr,"lon")){
						param->ltlon = atof(argv[ii+1]);
					}else if(!strcmp(ptr,"lat")){
						param->ltlat = atof(argv[ii+1]);
					}else if(!strcmp(ptr,"width")){
						param->width = atoi(argv[ii+1]);
					}else if(!strcmp(ptr,"height")){
						param->height = atoi(argv[ii+1]);
					}else if(!strcmp(ptr,"dlon")){
						param->dlon = atof(argv[ii+1]);
					}else if(!strcmp(ptr,"dlat")){
						param->dlat = atof(argv[ii+1]);
					}
					ii++;
					break;
			}
		}
	}
	arg->filenum = nn;
	/* 4 check parameter */
	if(param->width < 10){param->width =10;}
	if(param->height< 10){param->height=10;}
	if(param->ltlat < -90.  || 90.< param->ltlat){ param->ltlat = LTLAT;}
	if(param->ltlon <-180.  ||180.< param->ltlon){ param->ltlon = LTLON;}
	if(param->dlat  < 0. || 10. < param->dlat ){param->dlat = DLAT;}
	if(param->dlon  < 0. || 10. < param->dlon ){param->dlon = DLON;}

	/* 4 check error */
	if(arg->InFile[0]==NULL || arg->OutFile==NULL){
		return(ERROR_ARG);
	}
	/* 4 return */
	return(NORMAL_END);
}

/* ---------------------------------------------------------------------------
  getData()
 -----------------------------------------------------------------------------*/
int getData(argument *arg,parameter *param,outdata *data){

	HisdHeader		**header;
	FILE			**fp;
	float			*Pix,*Lin;
	unsigned short	*startLine;
	unsigned short	*endLine;
	unsigned int	ii,jj,kk,ll;
	int				n;
	unsigned short	count;
	float			radiance;
	unsigned long	n_size = param->height * param->width;
	double			phys;
	float 			minLine = 99999.0;
	float			maxLine =-99999.0;

	/* 1 allocate */
	if(	NULL == ( header = (HisdHeader **)calloc(arg->filenum,sizeof(HisdHeader *))) ||
		NULL == ( fp = (FILE **)calloc(arg->filenum,sizeof(FILE *))) || 
		NULL == ( startLine = (unsigned short *)calloc(arg->filenum,sizeof(unsigned short *))) ||
		NULL == ( endLine   = (unsigned short *)calloc(arg->filenum,sizeof(unsigned short *))) ||
		NULL == ( Pix = (float *)calloc(n_size,sizeof(float *))) ||
		NULL == ( Lin = (float *)calloc(n_size,sizeof(float *)))
	){
		fprintf(stderr,"callocate error\n");
		return(ERROR_CALLOCATE);
	}
	n = -1;
	for(ii=0;ii<arg->filenum;ii++){
		/* 2-1 open file */
		if(NULL == ( fp[ii] = fopen(arg->InFile[ii],"rb"))){
			fprintf(stderr,"error : can not open [%s]\n",arg->InFile[ii]);
			continue;
		}
		/* 2-2 callocate */
		if(NULL == (header[ii] = (HisdHeader *)calloc(1,sizeof(HisdHeader)))){
			fprintf(stderr,"callocate error\n");
			return(ERROR_CALLOCATE);
		}
		/* 2-3 read hisd header */
		if(NORMAL_END != hisd_read_header(header[ii],fp[ii])){
			fprintf(stderr,"error : read header [%s]\n",arg->InFile[ii]);
			continue;
		}
		/* 2-4 starLine and endLine */
		startLine[ii] = header[ii]->seg->strLineNo;
		endLine[ii]   = startLine[ii] + header[ii]->data->nLin -1;
		/* 2-5 check header consistency */
		if(n==-1)n=ii;
		if(	header[n]->calib->bandNo       != header[ii]->calib->bandNo ||
			header[n]->calib->gain_cnt2rad != header[ii]->calib->gain_cnt2rad ||
			header[n]->proj->loff          != header[ii]->proj->loff    ||
			header[n]->proj->coff          != header[ii]->proj->coff    ){
			fprintf(stderr,"header consistency error\n");
			fprintf(stderr,"%s : %s\n",arg->InFile[n],arg->InFile[ii]);
			return(ERROR_INFO);
		}
		n=ii;
	}
	/* 2-6 check file open */
	if(n==-1){
		//
		fprintf(stderr,"error : can not open all files\n");
		return(ERROR_FILE_OPEN);
	}
	/* 2-6 satellite name & band number */
	param->band = header[n]->calib->bandNo;
	strcpy(data->satName , header[n]->basic->satName);

	/* 3 get data */
	for(jj=0;jj<param->height;jj++){
	for(ii=0;ii<param->width;ii++){
		/* 3-1 init */
		count = header[n]->calib->outCount;
		kk = jj * param->width + ii;
		/* 3-2 convert lon & lat to pix & lin */
		lonlat_to_pixlin(header[n],data->lon[ii],data->lat[jj],&Pix[kk],&Lin[kk]);
		/* 3-3 min & max line */
		if(Pix[kk] == -9999 || Lin[kk] == -9999){ /* 2021.06 added */
			data->phys[kk] = INVALID;
			continue;
		}
		if(minLine > Lin[kk]) minLine =  Lin[kk];
		if(maxLine < Lin[kk]) maxLine =  Lin[kk];
		/* 3-4 get count value */
		for(ll=0;ll<arg->filenum;ll++){
            // 2015.06.06  fixed bug
	//		if(startLine[ll] <=  Lin[kk]+0.5 && Lin[kk]+0.5 <= endLine[ll]){
			if(startLine[ll] -0.5 <=  Lin[kk] && Lin[kk] < endLine[ll] + 0.5){
				hisd_getdata_by_pixlin(header[ll],fp[ll],Pix[kk],Lin[kk],&count);
				break;
			}
		}
		/* 3-5 check count value */
		if( count == header[n]->calib->outCount || 
			count == header[n]->calib->errorCount){
			data->phys[kk] = INVALID;
		}else{
		/* 3-6 convert count value to radiance */
			radiance = (float)count * header[n]->calib->gain_cnt2rad +
						header[n]->calib->cnst_cnt2rad;
		/* 3-6 convert radiance to physical value */
			if(	(header[n]->calib->bandNo>=7 &&
				 strstr(header[n]->basic->satName,"Himawari")!=NULL ) ||
				(header[n]->calib->bandNo>=2 &&
				 strstr(header[n]->basic->satName,"MTSAT-2") !=NULL )){
				/* infrared band */
				hisd_radiance_to_tbb(header[n],radiance,&phys);
				data->phys[kk] = (float)phys ;
			}else{
				/* visible or near infrared band */
				data->phys[kk] = header[n]->calib->rad2albedo * radiance;
			}
		}
	}
	}

	/* 4 convert maxLine & minLine to scanTime */
	for(ll=0;ll<arg->filenum;ll++){
		/* 4-1 startTime */
		if(startLine[ll] <= minLine && minLine <= endLine[ll]){
			for(ii=1;ii<header[ll]->obstime->obsNum;ii++){
				if(minLine < header[ll]->obstime->lineNo[ii]){
					data->startTime = header[ll]->obstime->obsMJD[ii-1];
					break;
				}else if(minLine == header[ll]->obstime->lineNo[ii]){
					data->startTime = header[ll]->obstime->obsMJD[ii];
					break;
				}
			}
		}
		/* 4-2 endTime */
		if(startLine[ll] <= maxLine && maxLine <= endLine[ll]){
			for(ii=1;ii<header[ll]->obstime->obsNum;ii++){
				if(maxLine < header[ll]->obstime->lineNo[ii]){
					data->endTime = header[ll]->obstime->obsMJD[ii-1];
				}else if(maxLine == header[ll]->obstime->lineNo[ii]){
					data->endTime = header[ll]->obstime->obsMJD[ii];
				}
			}
		}
	}

	/* 5 check data */
	printf("Satellite Name : %s\n",data->satName);
	printf("Band Number    : %d\n",param->band);
	printf("physical value :\n      ");
	for(jj=0;jj<param->width ;jj=jj+param->width/20){
		printf("%6.1f ",data->lon[jj]);
	}
	printf("\n");
	for(ii=0;ii<param->height;ii=ii+param->height/20){
		kk = ii * param->width + jj;
		printf("%6.1f ",data->lat[ii]);
		for(jj=0;jj<param->width ;jj=jj+param->width/20){
			kk = ii * param->width + jj;
			printf("%6.2f ",data->phys[kk]);
		}
		printf("\n");
	}

	/* 6 free */
	for(ii=0;ii<arg->filenum;ii++){
		if(header[ii] != NULL){
			hisd_free(header[ii]);
		}
		if(fp[ii]     != NULL){
			 fclose(fp[ii]);
		}
	}
	free(header);
	free(fp);
	free(startLine);
	free(endLine);
	free(Pix);
	free(Lin);
	return(NORMAL_END);
}

/* ---------------------------------------------------------------------------
  defNetcdf()
 -----------------------------------------------------------------------------*/
int	defNetcdf(parameter *param, short ch ,ncparam  *nc, char *satName, 
	char *outFile){

	int		latDimId,lonDimId;
	int		physDimId[2];
	char	latName[]="latitude";
	char	lonName[]="longitude";
	char	physName[32];
	char	physUnit[16];
	char	physStdName[32];
	char	startTimeName[]="start_time";
	int		startTimeDimId;
	char	endTimeName[]="end_time";
	int		endTimeDimId;
	char	timeUnit[]="days since 1858-11-17 0:0:0";
	char	timeName[64];
	float	invalid=INVALID;
	char	title[64];
	char	cc[64];
	char	*history;
	int	strLength;
	int	iDate[7];
	char	*fileName;

	/* 1 phys name */
	if(ch <= 6){
		strcpy(physName,"albedo");
		strcpy(physUnit,"1");
		strcpy(physStdName,"reflectivity");
		sprintf(title,"%s band-%d ALBEDO",satName,ch);
	}else if(ch <=16){
			strcpy(physName,"tbb");
			strcpy(physUnit,"K");
			strcpy(physStdName,"brightness_temperature");
			sprintf(title,"%s band-%d TBB",satName,ch);
	}else{
		fprintf(stderr,"ch number error  (ch number : 1 to 16)\n");
		return(ERROR_INFO);
	}
	/* 2 dimensions */
	if( 0 != nc_def_dim(nc->ncid,latName,param->height,&latDimId) ||
		0 != nc_def_dim(nc->ncid,lonName,param->width, &lonDimId) ||
		0 != nc_def_dim(nc->ncid,startTimeName,1,&startTimeDimId) ||
		0 != nc_def_dim(nc->ncid,endTimeName,  1,&endTimeDimId) ){
		fprintf(stderr,"nc_def_dim() error\n");
		return(ERROR_INFO);
	}
	/* 3 variables */
	if( 0 != nc_def_var(nc->ncid,latName,NC_FLOAT,1,&latDimId,&nc->latVarId) ||
		0 != nc_def_var(nc->ncid,lonName,NC_FLOAT,1,&lonDimId,&nc->lonVarId)){
		fprintf(stderr,"nc_def_var() error\n");
		return(ERROR_INFO);
	}
	physDimId[0]=latDimId;
	physDimId[1]=lonDimId;
	if( 0 != nc_def_var(nc->ncid,physName,NC_FLOAT,2,physDimId,&nc->physVarId) ||
		0 != nc_def_var(nc->ncid,startTimeName,NC_DOUBLE,0,
				&startTimeDimId,&nc->startTimeVarId) ||
		0 != nc_def_var(nc->ncid,endTimeName,NC_DOUBLE,0,
				&endTimeDimId,&nc->endTimeVarId)){
		fprintf(stderr,"nc_def_var() error\n");
		return(ERROR_INFO);
	}
	/* 4 attribute */
	if( 0 != nc_put_att_text(nc->ncid,nc->latVarId,
				"units",strlen("degrees_north"),"degrees_north") ||
		0 != nc_put_att_text(nc->ncid,nc->latVarId,
				"long_name",strlen("latitude"),"latitude") ||
		0 != nc_put_att_text(nc->ncid,nc->lonVarId,
				"units",strlen("degrees_east"),"degrees_east") ||
		0 != nc_put_att_text(nc->ncid,nc->lonVarId,
				"long_name",strlen("longitude"),"longitude") ||
		0 != nc_put_att_text(nc->ncid,nc->physVarId,
				"units",strlen(physUnit), physUnit) ||
		0 != nc_put_att_text(nc->ncid,nc->physVarId,
				"long_name",strlen(physStdName),physStdName) ||
		0 != nc_put_att_float(nc->ncid,nc->physVarId,
				"_FillValue",NC_FLOAT,1,&invalid) ||
		0 != nc_put_att_text(nc->ncid,nc->startTimeVarId,
				"units",strlen(timeUnit),timeUnit) ||
		0 != nc_put_att_text(nc->ncid,nc->startTimeVarId,
				"standard_name",strlen("time"),"time") ){
		fprintf(stderr,"nc_put_att_text()\n");
		return(ERROR_INFO);
	}
	strcpy(timeName,"observation start time");
	if( 0 != nc_put_att_text(nc->ncid,nc->startTimeVarId,
				"long_name",strlen(timeName),timeName) ||
		0 != nc_put_att_text(nc->ncid,nc->endTimeVarId,
				"units",strlen(timeUnit),timeUnit) ||
		0 != nc_put_att_text(nc->ncid,nc->endTimeVarId,
				"standard_name",strlen("time"),"time") ||
		0 != nc_put_att_text(nc->ncid,nc->endTimeVarId,
				"long_name",strlen("time"),"time")){
		fprintf(stderr,"nc_put_att_text()\n");
		return(ERROR_INFO);
	}
	strcpy(timeName,"observation end time");
	if( 0 != nc_put_att_text(nc->ncid,nc->endTimeVarId,
				"long_name",strlen(timeName),timeName)){
		fprintf(stderr,"nc_put_att_text()\n");
		return(ERROR_INFO);
	}
	/* 5 global attributes */
	nc_put_att(nc->ncid,NC_GLOBAL,"title", NC_CHAR,strlen(title),title);
	nc_put_att(nc->ncid,NC_GLOBAL,"institution", NC_CHAR,strlen("MSC/JMA"),"MSC/JMA");
	sprintf(cc,"%s satellite observation",satName);
	nc_put_att(nc->ncid,NC_GLOBAL,"source",NC_CHAR,strlen(cc),cc);
	/* 6 hh:nn:ss mm/dd/yyyy: created. */
	if(NULL==(fileName=strrchr(outFile,'/'))){
		fileName=outFile;
	}else{
		fileName++;
	}
	DateGetNowInts(iDate);
	strLength = 38 + strlen(fileName) + 9 + strlen(nc_inq_libvers())+1; /* 2021.06 fixed bug */
	if( NULL == (history = calloc(strLength,sizeof(char)))){
		fprintf(stderr,"allocate error\n");
		return(ERROR_CALLOCATE);
		}
	sprintf(history,"at %02d:%02d:%02d %02d/%02d/%04d: file created. "
		"%s "
		"(netCDF %s)",
		iDate[3],iDate[4],iDate[5],iDate[1],iDate[2],iDate[0],
		fileName,
		nc_inq_libvers());
	nc_put_att(nc->ncid,NC_GLOBAL,"history",NC_CHAR,strlen(history),history);
	nc_put_att(nc->ncid,NC_GLOBAL,"Conventions",NC_CHAR,strlen("CF-1,4"),"CF-1.4");
	free(history);
	/* 7 Quit */
	if( 0 != nc_enddef(nc->ncid)){
		fprintf(stderr,"nc_enddef() error \n");
		return(ERROR_INFO);
	}
	return(NORMAL_END);
}

/* ---------------------------------------------------------------------------
  putNetCDF
 -----------------------------------------------------------------------------*/
int putNetcdf(ncparam *nc,parameter *param,outdata *data){
	size_t gridSize[2],startPos[2];

	if( 0!= nc_put_var_float(nc->ncid,nc->latVarId,data->lat)){
		fprintf(stderr,"nc_put_var_float() error (lat)\n");
		return(ERROR_WRITE);
	}
	if(0!=nc_put_var_float(nc->ncid,nc->lonVarId,data->lon)){
		fprintf(stderr,"nc_put_var_float() error (lon)\n");
		return(ERROR_WRITE);
	}
	gridSize[0]=param->height;
	gridSize[1]=param->width;
	startPos[0]=startPos[1]=0;
	if(0!=(nc_put_vara_float(nc->ncid,nc->physVarId,startPos,gridSize,data->phys))){
		fprintf(stderr,"nc_put_vara_float() error (phys) \n");
		return(ERROR_WRITE);
	}
	if(0!=nc_put_var_double(nc->ncid,nc->startTimeVarId,&data->startTime)){
		fprintf(stderr,"nc_put_var_double() error (startTime)\n");
		return(ERROR_WRITE);
	}
	if(0!=nc_put_var_double(nc->ncid,nc->endTimeVarId,&data->endTime)){
		fprintf(stderr,"nc_put_var_double() error (endTime)\n");
		return(ERROR_WRITE);
	}
	return(NORMAL_END);
}

/* ---------------------------------------------------------------------------
  makeNetCDF
 -----------------------------------------------------------------------------*/
int makeNetCDF(char *outFile,parameter *param,outdata *data){
	ncparam		nc;

	/* 1 open netcdf file */
	if(NORMAL_END != nc_create(outFile,NC_CLOBBER,&nc.ncid)){
		fprintf(stderr,"Can't open netCDF file [%s] \n",outFile);
	}
	/* 2 define netcdf file */
	defNetcdf(param, param->band, &nc, data->satName,outFile );
	/* 3 put data */
	putNetcdf(&nc, param,data);
	/* 4 close netcdf file */
	if( 0 != nc_close(nc.ncid)){
		fprintf(stderr,"Can't close netCDF file [%s] \n",outFile);
		return(ERROR_WRITE);
	}
	/* 5 end */
	return(NORMAL_END);
}
/* ---------------------------------------------------------------------------
  convert HISD file to NetCDF file
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[]){
	parameter	param;
	argument	arg;
	outdata		data;
	long		n_size;
	long		ii;

	/* -----------------------------------------------------------------------
		1 get argument
	 -----------------------------------------------------------------------*/
	if(NORMAL_END != getArg(argc,argv,&arg,&param)){
		char *cPtr=strrchr(argv[0],'/');
		if(cPtr==NULL)
			{
			cPtr=argv[0];
			}
		else
			{
			cPtr++;
			}
		fprintf(stderr,"Usage : %s [OPTION]\n"
			"  -i <InFile> [-i <InFile2> ...]\n"
			"  -o <OutFile>\n"
			"  -width  <Pixel Number>\n"
			"  -height <Line Number>\n"
			"  -lat    <Left top latitude>\n"
			"  -lon    <Left top longitude>\n"
			"  -dlat   <Spatial resolution (longitude)>\n"
			"  -dlon   <Spatial resolution (latitude)>\n",cPtr);
		return(ERROR_ARG);
	}
	/* -----------------------------------------------------------------------
		2 check parameter
	 -----------------------------------------------------------------------*/
	printf("Left top (lat,lon) : (%6.2f,%6.2f)\n",param.ltlat,param.ltlon);
	printf("width,height       : (%6d,%6d)\n",param.width,param.height);
	printf("Spatial resolution : (%6.2f,%6.2f)\n",param.dlat,param.dlon);
	n_size      = param.width * param.height;
	/* -----------------------------------------------------------------------
		3 allocate
	 -----------------------------------------------------------------------*/
	/* allocate */
	if(	NULL == (data.lat = (float *)calloc(param.height,sizeof(float *))) ||
		NULL == (data.lon = (float *)calloc(param.width, sizeof(float *))) ||
		NULL == (data.phys= (float *)calloc(n_size,      sizeof(float *))) 
	){
		fprintf(stderr,"allocate error\n");
		return(ERROR_CALLOCATE);
	}
	/* init */
	for(ii=0;ii<n_size;ii++){
		data.phys[ii] = INVALID;
	}
	/* -----------------------------------------------------------------------
		4 set longitude and latitude
	 -----------------------------------------------------------------------*/
	for(ii=0;ii<param.height;ii++){
		data.lat[ii] = param.ltlat - param.dlat * ii; 
	}
	for(ii=0;ii<param.width ;ii++){
		data.lon[ii] = param.ltlon + param.dlon * ii; 
	}
	/* -----------------------------------------------------------------------
		5 get data
	 -----------------------------------------------------------------------*/
	if(NORMAL_END != getData(&arg,&param,&data)){
		fprintf(stderr,"get data error\n");
		free(data.lon);
		free(data.lat);
		free(data.phys);
		return(ERROR_READ_DATA);
	}
	/* -----------------------------------------------------------------------
		6 make NetCDF
	 -----------------------------------------------------------------------*/
	if(NORMAL_END != makeNetCDF(arg.OutFile,&param,&data)){
		fprintf(stderr,"make netcdf error\n");
		free(data.lon);
		free(data.lat);
		free(data.phys);
		return(ERROR_WRITE);
	}	
	/* -----------------------------------------------------------------------
		6 end
	 -----------------------------------------------------------------------*/
	free(data.lon);
	free(data.lat);
	free(data.phys);
	printf("NORMAL END\n");
	return(NORMAL_END);
}

