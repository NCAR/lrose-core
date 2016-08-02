
#define RGates  460
#define VGates  920
#define WGates  920

#define RGatesMem  920
#define VGatesMem  920
#define WGatesMem  920

#define MaxCuts		20
#define MaxRads		360

#define CODE_INVALID 0
#define CODE_RANFOLD 1

#define VALUE_INVALID -999.
#define VALUE_RANFOLD  999.

#define RES_POINT_FIVE 2
#define RES_ONE_POINT  4

#define VOL_BEG 3
#define VOL_END 4
#define ELV_BEG 0
#define ELV_END 2

#define HIST_SIZE 100

#define RADIAN  3.14159/180.
//////////////////////////////////////////////////////////////////////
//
typedef struct tagBaseData
{
	unsigned short		temp1[7];
	unsigned short		RadarStatus;
	unsigned short		temp2[6];
	unsigned int		mSeconds;
	unsigned short		JulianDate;
	unsigned short		URange;
	unsigned short		Az;
	unsigned short		RadialNumber;
	unsigned short		RadialStatus;
	unsigned short		El;
	unsigned short		ElNumber;
        short				RangeToFirstGateOfRef;
  //can be negative
        short				RangeToFirstGateOfDop;	
	unsigned short		GateSizeOfReflectivity;
	unsigned short		GateSizeOfDoppler;
	unsigned short		GatesNumberOfReflectivity;
	unsigned short		GatesNumberOfDoppler;
	unsigned short		CutSectorNumber; 
	unsigned int		CalibrationConst; 
	unsigned short		PtrOfReflectivity;
	unsigned short		PtrOfVelocity;	
	unsigned short		PtrOfSpectrumWidth;
	unsigned short		ResolutionOfVelocity;
	unsigned short		VcpNumber;
	unsigned short		temp4[4];
	unsigned short		PtrOfArcReflectivity;
	unsigned short		PtrOfArcVelocity;
	unsigned short		PtrOfArcWidth;	
	unsigned short		Nyquist;
	unsigned short      temp46; 
	unsigned short      temp47; 
	unsigned short      temp48; 
	unsigned short      CircleTotal;
	unsigned char		temp5[30];
	unsigned char		Echodata[RGates+VGates+WGates];
	unsigned char		temp[4];
						
}RADIALDATA;


float RData[MaxCuts][MaxRads][RGatesMem];
float VData[MaxCuts][MaxRads][VGatesMem];
float WData[MaxCuts][MaxRads][WGatesMem];
float Elevation[MaxCuts];

bool RRadialFlag[MaxCuts][MaxRads];
bool VRadialFlag[MaxCuts][MaxRads];
bool WRadialFlag[MaxCuts][MaxRads];

int NumValidCuts;

//bool SavedataIntoFiles();
bool ReadBaseData(char filename[1000],char radtype[10],int station);
unsigned short ftohs(int rtype, short in);
unsigned long ftohl(int rtype, long in);
void addElevHist(float elev, int elevHist[], float elevValues[], int *elevCount);
float getBestElev(int elevHist[], float elevValues[], int elevCount);
void fillMissingRadials(bool *flag, float *data, int elev_count, int numGates);




RADIALDATA* pOneRadial;

char nex_header[24];


float DecodeSpw(unsigned char code);
float DecodeVel(unsigned char code, short ResType);
float DecodeRef(unsigned char code);

void createScanProduct(int rfd, float *data, e_data_type type, int numGates, int elevation,int gateSize,
		       int firstGate,float nyquist,int scan_count,int total_scans,float ElevAngle,time_t scanTime,int station);
void dumpRadialHeader(int rtype,	RADIALDATA* pOneRadial);


unsigned char *src_array, *dest_array;

    int		src_xdim, src_ydim, dest_xdim, dest_ydim;
    
    //bool	valid;
    int		scan_vol_no;	    //scan no. in this volume
    int		minNeighbors;	    // set min neighbors for filter
    int		newRange;
    int		s1fd; //output file handle
    int		scancount;
    int		dummy;
time_t startTime;
time_t lastTime;

#define NEXRAD 0
#define CINRAD 1
