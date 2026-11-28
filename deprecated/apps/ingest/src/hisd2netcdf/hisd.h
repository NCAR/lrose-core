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
		http://www.jma-net.go.jp/msc/en/

		Himawari Standard Data User's Guide
		http://www.data.jma.go.jp/mscweb/en/himawari89/space_segment/hsd_sample/HS_D_users_guide_en.pdf

	History
		April,   2014  First release
		January, 2015  Change for version 1.1
		May,     2015  Change for version 1.2

 ----------------------------------------------------------------------------*/

#define INVALID_VALUE -10000000000.0

/* #1 */
typedef struct{
	unsigned char	HeaderNum;			/* 1 */
	unsigned short	BlockLen;			/* 2 */
	unsigned short	headerNum;			/* 3 */
	unsigned char	byteOrder;			/* 4 */
	char			satName[17];		/* 5 */
	char			proName[17];		/* 6 */
	char			ObsType1[5];		/* 7 */
	char			ObsType2[3];		/* 8 */
	unsigned short	TimeLine;			/* 9 */
	double			ObsStartTime;		/*10 */
	double			ObsEndTime;			/*11 */
	double			fileCreationMjd;	/*12 */
	unsigned int	totalHeaderLen;		/*13 */
	unsigned int	dataLen;			/*14 */
	unsigned char	qflag1;				/*15 */
	unsigned char	qflag2;				/*16 */
	unsigned char	qflag3;				/*17 */
	unsigned char	qflag4;				/*18 */
	char			verName[33];		/*19 */
	char			fileName[129];		/*20 */
	char			spare[41];			/*21 */
}Basic_Info;

/* #2 */
typedef struct{
	unsigned char	HeaderNum;			/* 1 */
	unsigned short	BlockLen;			/* 2 */
	unsigned short	bitPix;				/* 3 */
	unsigned short	nPix;				/* 4 */
	unsigned short	nLin;				/* 5 */
	unsigned char	comp;				/* 6 */
	char			spare[41];			/* 7 */
}Data_Info;

/* #3 */
typedef struct{
	unsigned char	HeaderNum;			/* 1 */
	unsigned short	BlockLen;			/* 2 */
	double			subLon;				/* 3 */
	unsigned int	cfac;				/* 4 */
	unsigned int	lfac;				/* 5 */
	float			coff;				/* 6 */
	float			loff;				/* 7 */
	double			satDis;				/* 8 */
	double			eqtrRadius;			/* 9 */
	double			polrRadius;			/*10 */
	double			projParam1;			/*11 */
	double			projParam2;			/*12 */
	double			projParam3;			/*13 */
	double			projParamSd;		/*14 */
	unsigned short  resampleKind;       /*15 */
	unsigned short	resampleSize;		/*16 */
	char			spare[41];			/*17 */
}Proj_Info;

/* #4 */
typedef struct{
	unsigned char	HeaderNum;			/* 1 */
	unsigned short	BlockLen;			/* 2 */
	double			navMjd;				/* 3 */
	double			sspLon;				/* 4 */
	double			sspLat;				/* 5 */
	double			satDis;				/* 6 */
	double			nadirLon;			/* 7 */
	double			nadirLat;			/* 8 */
	double			sunPos_x;			/* 9 */
	double			sunPos_y;			/*10 */
	double			sunPos_z;			/*11 */
	double			moonPos_x;			/*12 */
	double			moonPos_y;			/*13 */
	double			moonPos_z;			/*14 */
	char			spare[41];			/*15 */
}Navi_Info;

/* #5 */
typedef struct{
	unsigned char	HeaderNum;			/* 1 */
	unsigned short	BlockLen;			/* 2 */
	unsigned short	bandNo;				/* 3 */
	double			waveLen;			/* 4 */
	unsigned short	bitPix;				/* 5 */
	unsigned short	errorCount;			/* 6 */
	unsigned short	outCount;			/* 7 */
	/* count-radiance conversion equation */
	double			gain_cnt2rad;		/* 8 */
	double			cnst_cnt2rad;		/* 9 */
	/* correction coefficient of sensor Planck function */
	/* for converting radiance to brightness temperature */
	double			rad2btp_c0;			/*10 */
	double			rad2btp_c1;			/*11 */
	double			rad2btp_c2;			/*12 */
	/* for converting brightness temperature to radiance */
	double			btp2rad_c0;			/*13 */
	double			btp2rad_c1;			/*14 */
	double			btp2rad_c2;			/*15 */
	double			lightSpeed;			/*16 */
	double			planckConst;		/*17 */
	double			bolzConst;			/*18 */
	char			spare[41];			/*19 */
	/* transformation coefficient from radiance to albedo */
	double			rad2albedo;			/*10 */
	char			spareV[105];		/*11 */
}Calib_Info;

/* #6 */
	/*  --------------------------------------
	version 1.0
		11 Ancillary text
		12 Spare
    version 1.1
		11 Radiance valid range of GSICS Calibration Coefficients
			(upper limit)
		12 Radiance valid range of GSICS Calibration Coefficients
			(lower limit)
		13 File name of GSICS Correction
		14 Spare
	version 1.2
		3 GSICS calibration coefficient (Intercept)
		4 GSICS calibration coefficient (Slope)
		5 GSICS calibration coefficient (Quadratic term)
		6 Radiance bias for standard scene
		7 Uncertainty of radiance bias for standard scene
		8 Radiance for standard scene
	-------------------------------------- */
typedef struct{	                        /* 1.0,1.1,1.2*/
	unsigned char	HeaderNum;			/* 1 , 1 , 1  */
	unsigned short	BlockLen;			/* 2 , 2 , 2  */
	double			gsicsCorr_C;		/* 3 , 3 , 3  */
	double			gsicsCorr_C_er;		/* 4 , 4 , -  */
	double			gsicsCorr_1;		/* 5 , 5 , 4  */
	double			gsicsCorr_1_er;		/* 6 , 6 , -  */
	double			gsicsCorr_2;		/* 7 , 7 , 5  */
	double			gsicsCorr_2_er;		/* 8 , 8 , -  */
	double			gsicsBias;			/* - , - , 6  */
	double			gsicsUncert;		/* - , - , 7  */
	double			gsicsStscene;		/* - , - , 8  */
	double			gsicsCorr_StrMJD;	/* 9 , 9 , 9  */
	double			gsicsCorr_EndMJD;	/*10 ,10 , 10 */
	char			gsicsCorrInfo[65];	/*11 , - , -  */
	float			gsicsUpperLimit;	/* - ,11 , 11 */
	float			gsicsLowerLimit;	/* - ,12 , 12 */
	char			gsicsFilename[129];	/* - ,13 , 13 */
	char			spare[129];			/*12 ,14 , 14 */
}InterCalib_Info;

/* #7 */
typedef struct{
	unsigned char	HeaderNum;			/* 1 */
	unsigned short	BlockLen;			/* 2 */
	char			totalSegNum;		/* 3 */
	char			segSeqNo;			/* 4 */
	unsigned short	strLineNo;			/* 5 */
	char			spare[41];			/* 6 */
}Segm_Info;

/* #8 */
typedef struct{
	unsigned char	HeaderNum;			/* 1 */
	unsigned short	BlockLen;			/* 2 */
	float			RoCenterColumn;		/* 3 */
	float			RoCenterLine;		/* 4 */
	double			RoCorrection;		/* 5 */
	unsigned short	correctNum;			/* 6 */
	unsigned short	*lineNo;			/* 7 */
	float			*columnShift;		/* 8 */
	float			*lineShift;			/* 9 */
	char			spare[41];			/*10 */
}NaviCorr_Info;

/* #9 */
typedef struct{
	unsigned char	HeaderNum;			/* 1 */
	unsigned short	BlockLen;			/* 2 */
	unsigned short	obsNum;				/* 3 */
	unsigned short	*lineNo;			/* 4 */
	double			*obsMJD;			/* 5 */
	char			spare[41];			/* 6 */
}ObsTime_Info;

/* #10 */
typedef struct{
	unsigned char	HeaderNum;			/* 1 */
	unsigned int	BlockLen;			/* 2 */
	unsigned short	errorNum;			/* 3 */
	unsigned short	*lineNo;			/* 4 */
	unsigned short  *errPixNum;			/* 5 */
	char			spare[41];			/* 6 */
}Error_Info;

/* #11 */
typedef struct{
	unsigned char	HeaderNum;			/* 1 */
	unsigned short	BlockLen;			/* 2 */
	char			spare[257];			/* 3 */
}Spare;

/* navigation correction information table */
typedef struct{
	char	flag;
	int		startLineNo;
	int		lineNum;
	float	*cmpCoff;
	float	*cmpLoff;
}Correct_Table;

typedef struct{
	Basic_Info		*basic;			/* 1 */
	Data_Info		*data;			/* 2 */
	Proj_Info		*proj;			/* 3 */
	Navi_Info		*nav;			/* 4 */
	Calib_Info		*calib;			/* 5 */
	InterCalib_Info	*interCalib;	/* 6 */
	Segm_Info		*seg;			/* 7 */
	NaviCorr_Info	*navcorr;		/* 8 */
	ObsTime_Info	*obstime;		/* 9 */
	Error_Info		*error;			/*10 */
	Spare			*spare;			/*11 */
	Correct_Table	*correct_table;
}HisdHeader;

/* hisd_read.c */
int hisd_read_header(HisdHeader *header ,FILE *fp);
int hisd_free(HisdHeader *header);
int hisd_getdata_by_pixlin(HisdHeader *header,FILE *fp,float pix,float lin,
    unsigned short *sout);
void hisd_radiance_to_tbb(HisdHeader *header,double radiance,
	double *tbb);
int swapBytes(void *buf,int size,int nn );
int byteOrder(void);

/* hisd_info.c */
int hisd_info(HisdHeader *header);

/* hisd_pixlin2lonlat.c */
int pixlin_to_lonlat(HisdHeader *head,float pix,float lin,
	double *lon,double *lat);
int lonlat_to_pixlin(HisdHeader *head,double lon,double lat,
	float *pix,float *lin);


