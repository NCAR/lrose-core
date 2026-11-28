
//
// Routine to read WindTracer lidar *.prd files. You pass in the filename
// and the array size. You get back data.
//
// Niles Oien September 2004.
//
#include <cstdio>
#include <cstring>
#include <cmath>

extern "C" {

//
// File scope.
//
int readShort(FILE **fp);
char *idToStr(int ID);
void processSpecial(FILE *fp, float *az, float *el);
void processText(FILE *fp, int len,
		 int *retYear, int *retMonth, int *retDay,
		 int *retNumGates, float *retFirstGate, 
		 float *retGateSpace,
		 int *debug);
void readKeyword(FILE *fp, char *key, double *val, int tryRead);
void byteSwap4(void *p);
void processHeader(FILE *fp, int *Hour, int *Min, int *Sec, int *Msec);
int readLong(FILE **fp);
void processExistingText(int *retYear, int *retMonth, int *retDay,
			 int *retNumGates, float *retFirstGate, 
			 float *retGateSpace,
			 int *debug);
//
// Main routine.
//
void readprd_(char *filename,
	     int *ntimes,
	     int *ngates,
	     float *firstRange,
	     float *deltaRange,
	     int *year,
	     int *month,
	     int *day,
	     int *hour,
	     int *min,
	     int *sec,
	     int *msec,
	     float *azimuth,
	     float *elevation,
	     float *vel,
	     float *snr,
	     float *cfar,
	     float *sw,
	     float *backscatter,
	     float *mcfar,
	     float *quality,
	     float *fvel,
	     float *fsnr,
	     int *maxNumGates,
	     int *maxNumTimes,
	     int *debug,
	     int *problem,
	     int lenStr ){

  // filename     - filename (char)                                           [INPUT]
  // ntimes       - number of data times (records) (int)                      [OUTPUT]
  // ngates       - number of range-gates (int)                               [OUTPUT]
  // firstRange   - distance to center of first range-gate (m) (float)        [OUTPUT]
  // deltaRange   - distance between range-gates (m) (float)                  [OUTPUT]
  // year         - Single value for year at file start time (int)            [OUTPUT]
  // month        - Single value for month at file start time (int)           [OUTPUT]
  // day          - Single value for day at file start time (int)             [OUTPUT]
  // hour         - array for hour (int)                                      [OUTPUT]
  // min          - array for min (int)                                       [OUTPUT]
  // sec          - array for sec (int)                                       [OUTPUT]
  // msec         - array for msec (int)                                      [OUTPUT]
  // azimuth      - array for azimuth angle (deg) (float)                     [OUTPUT]
  // elevation    - array for elevation angle (deg) (float)                   [OUTPUT]
  // vel          - array for radial velocity (ngates*ntimes) (float)         [OUTPUT]
  // snr          - array for snr (ngates*ntimes) (float)                     [OUTPUT]
  // cfar         - array for false alarm flag (ngates*ntimes) (float)        [OUTPUT]
  // sw           - array for spectral width (ngates*ntimes) (float)          [OUTPUT]
  // backscatter  - array for backscatter (ngates*ntimes) (float)             [OUTPUT]
  // mcfar        - array for modified false alarm (ngates*ntimes) (float)    [OUTPUT]
  // quality      - array for quality flag (ngates*ntimes) (float)            [OUTPUT]
  // fvel         - array for filtered velocity (ngates*ntimes) (float)       [OUTPUT]
  // fsnr         - array for filtered snr (ngates*ntimes) (float)            [OUTPUT]
  // maxNumGates  - array size (int)                                          [INPUT]
  // maxNumTimes  - array size (int)                                          [INPUT]
  // debug        - zero => quiet 1 => print messages (int)                   [INPUT]
  // problem      - zero => no problem                                        [OUTPUT]
  // lenStr - length of the string. There for fortran calls. From C, use 0.   [INPUT]

  // Start off pessimistic.

  *problem = 1; 

  if (*debug){
    fprintf(stderr, "In readPrd()\n");
  }

  // We may have been called from Fortran, in which case the
  // filename may have trailing blanks - strip them off.

  char Cfilename[256];
  int k=0;
  int go=1;
  do {

    if (filename[k] == ' '){
      Cfilename[k] = char(0);
    } else {
      Cfilename[k] = filename[k];
      Cfilename[k+1] = char(0);
    }
    //
    if (Cfilename[k] == char(0)){
      go=0;
    }
    k++;
  } while (( go ) && ( k < lenStr));


  if (*debug){
    fprintf(stderr, "Filename is %s\n", Cfilename);
  }
 
  FILE *fp = fopen(Cfilename, "r");
  if (fp == NULL){
    if (*debug){
      fprintf(stderr, "File %s not found.", Cfilename);
    }
    return;
  }
 
  int Iyear, Imonth, Iday, InumGates;
  float firstGate, deltaGate;

  int irec = 0;
  int len = 0;

  int numFields = 0;
  int numBeams = 0;
  int id, ver;
  int maxLen = 0;
  int iAlElIndex = 0;
  int iTimeIndex = 0;

  do {

    id = readShort( &fp );
    ver = readShort( &fp );
    len = readLong( &fp );

    irec++;
    
    //
    // Pick the number of fields and the max len off the
    // first set
    //  
    if (numBeams == 1){
      if (
	  (id > 0x1d) &&
	  (strcmp("UNKNOWN", idToStr( id )))
	  ){
	numFields++;
	if (*debug){
	  fprintf(stderr, "Field %s found.\n", idToStr( id ));
	}
	//
	if (len > maxLen){
	  maxLen = len;
	}
      }
    }
    
    switch (id) {
      
    case 0x10 :
      numBeams++;
      if (numBeams == *maxNumTimes) break;
      processSpecial(fp, &azimuth[iAlElIndex], &elevation[iAlElIndex]);
      iAlElIndex++;
      break;

    case 0x6 :
      processText(fp, len-2, &Iyear, &Imonth, &Iday,
		  &InumGates, &firstGate, &deltaGate, debug);
      break;

    case 0xf :
      processHeader(fp, &hour[iTimeIndex], &min[iTimeIndex], &sec[iTimeIndex], &msec[iTimeIndex]);
      iTimeIndex++;
      break;

    default :
      int byteLen = 4*len-8;
      fseek(fp, byteLen, SEEK_CUR);
      break;

    } 

  } while ((numBeams < *maxNumTimes) && (id != 0) && (len > 0) && (!(feof(fp))));


  if (*debug){
    fprintf(stderr,"%d fields, %d beams.\n", numFields, numBeams);
  }

  *ntimes = numBeams;
  *ngates = InumGates;
  *year = Iyear;
  *month = Imonth;
  *day = Iday;
  *firstRange = firstGate;
  *deltaRange = deltaGate;
  //
  // Check that the output arrays have enough space.
  //
  if (InumGates > *maxNumGates){
    if (*debug){
      fprintf(stderr,"%d gates, only room for %d\n", InumGates, *maxNumGates);
      fprintf(stderr,"Recompile and re-run.\n");
    }
    fclose(fp);
    return;
  }


  if (numBeams > *maxNumTimes){
    if (*debug){
      fprintf(stderr,"%d beams, only room for %d\n", numBeams, *maxNumTimes);
      fprintf(stderr,"Recompile and re-run.\n");
    }
    fclose(fp);
    return;
  }

  ////////////////////////////////////////////////////////////
  //
  // Rewind to start of file and commence reading the 2D variables.
  //
  rewind(fp);

  iTimeIndex = 0;
  iAlElIndex = 0;

  int velIndex=0, snrIndex=0, cfarIndex=0, swIndex=0;
  int fvelIndex=0, fsnrIndex=0, mcfarIndex=0, qualityIndex=0, backscatterIndex=0;

  do {

    id = readShort( &fp );
    ver = readShort( &fp );
    len = readLong( &fp );

    
    switch (id) {

 

    case 0x1e : // VEL
      //
      fread(vel + velIndex, sizeof(float), InumGates, fp);
      for (int j=0; j < InumGates; j++){
	byteSwap4(vel + velIndex + j);
      }
      velIndex = velIndex + InumGates;
      break;

    case 0x1f : // SNR
      //
      fread(snr + snrIndex, sizeof(float), InumGates, fp);
      for (int j=0; j < InumGates; j++){
	byteSwap4(snr + snrIndex + j);
      }
      snrIndex = snrIndex + InumGates;
      break;
      
    case 0x20 : // CFAR
      //
      fread(cfar + cfarIndex, sizeof(float), InumGates, fp);
      for (int j=0; j < InumGates; j++){
	byteSwap4(cfar + cfarIndex + j);
      }
      cfarIndex = cfarIndex + InumGates;
      break;

      
    case 0x21 : // SW
      //
      fread(sw + swIndex, sizeof(float), InumGates, fp);
      for (int j=0; j < InumGates; j++){
	byteSwap4(sw + swIndex + j);
      }
      swIndex = swIndex + InumGates;
      break;
   
    case 0x55 : // FVEL
      //
      fread(fvel + fvelIndex, sizeof(float), InumGates, fp);
      for (int j=0; j < InumGates; j++){
	byteSwap4(fvel + fvelIndex + j);
      }
      fvelIndex = fvelIndex + InumGates;
      break;


    case 0x56 : // FSNR
      //
      fread(fsnr + fsnrIndex, sizeof(float), InumGates, fp);
      for (int j=0; j < InumGates; j++){
	byteSwap4(fsnr + fsnrIndex + j);
      }
      fsnrIndex = fsnrIndex + InumGates;
      break;

    case 0x58 : // MCFAR
      //
      fread(mcfar + mcfarIndex, sizeof(float), InumGates, fp);
      for (int j=0; j < InumGates; j++){
	byteSwap4(mcfar + mcfarIndex + j);
      }
      mcfarIndex = mcfarIndex + InumGates;
      break;

    case 0x59 : // QUALITY
      //
      fread(quality + qualityIndex, sizeof(float), InumGates, fp);
      for (int j=0; j < InumGates; j++){
	byteSwap4(quality + qualityIndex + j);
      }
      qualityIndex = qualityIndex + InumGates;
      break;

    case 0x5a : // BACKSCATTER
      //
      fread(backscatter + backscatterIndex, sizeof(float), InumGates, fp);
      for (int j=0; j < InumGates; j++){
	byteSwap4(backscatter + backscatterIndex + j);
      }
      backscatterIndex = backscatterIndex + InumGates;
      break;

      //
      // Default : skip it, we don't need it.
      //
    default :
      int byteLen = 4*len-8;
      fseek(fp, byteLen, SEEK_CUR);
      break;

    }
    

  } while ((numBeams < *maxNumTimes) && (id != 0) && (len > 0) && (!(feof(fp))));







  fclose(fp);

  *problem = 0;
  return;

}

//
// Read a short int, byte swapped.
//
int readShort(FILE **fp){

  unsigned char a,b;
  fread(&a, sizeof(unsigned char), 1, *fp);
  fread(&b, sizeof(unsigned char), 1, *fp);
  
  return a*256+b;

}

//
// Return a string for a field ID.
//
char *idToStr(int ID){

  switch (ID) {
 
  case 0x0 :
    return "EOF";
    break;

  case 0xf :
    return "CREC HDR FOR PRODUCT RECORD";
    break;

  case 0x10 :
    return "CREC SPECIAL FOR PRODUCT RECORD";
    break;

  case 0x11 :
    return "CREC RESULTS FOR PRODUCT RECORD";
    break;

  case 0x6 :
    return "TEXT";
    break;

  case 0x15 :
    return "TEXT";
    break;

  case 0x1e :
    return "VEL";
    break;

  case 0x1f :
    return "SNR";
    break;

  case 0x20 :
    return "CFAR";
    break;

  case 0x21 :
    return "SW";
    break;
   
  case 0x55 :
    return "FVEL";
    break;

  case 0x56 :
    return "FSNR";
    break;

  case 0x58 :
    return "MCFAR";
    break;

  case 0x59 :
    return "QUALITY";
    break;

  case 0x5a :
    return "BACKSCAT";
    break;

  default :
    return "UNKNOWN";
    break;

  }
}

//
// Process the special header - for us, this holds the azimuth
// and the elevation.
//
void processSpecial(FILE *fp, float *az, float *el){


  for (int i=0; i < 18; i++){
    float h;
    fread(&h, sizeof(float), 1, fp); 
    byteSwap4(&h);
    if (i == 11) *az = h;
    if (i == 12) *el = h;
  }

  // Make sure the angles are still on the desired range.

  do {
    if (*az > 360.0) *az = *az - 360.0;
    if (*az < 0.0) *az = *az + 360.0;
    if (*el > 90.0){
      if (*el > 180.0){
	fprintf(stderr,"Silly elevation of %g, I cannot cope.\n", *el);
	exit(-1);
      }
      *el = 180.0 - *el;
      *az = *az + 180.0;
    }
  } while ((*el > 90.0) || (*az > 360.0) || (*az < 0.0));

  //
  return;

}

//
// Process the header - for us, holds the hour, minute and second.
//
void processHeader(FILE *fp, int *Hour, int *Min, int *Sec, int *Msec){

  int hour=0, min=0, sec=0, msec=0;

  for (int i=0; i < 6; i++){
    int k;
    fread(&k, sizeof(int), 1, fp); 
    byteSwap4(&k);
   
    if (i == 1) hour = k;
    if (i == 2) min = k;
    if (i == 3) sec = k;
    if (i == 4) msec = k;

  }

  //
  // Return these.
  //
  *Hour = hour; *Min = min; 
  *Sec = sec;  *Msec = msec;

  return;

}

//
// Routine to process the text part of the file. The text is pulled
// out and written to a temporary file, and then that temporary file is processed.
//
void processText(FILE *fp, int len,
		 int *retYear, int *retMonth, int *retDay,
		 int *retNumGates, float *retFirstGate, 
		 float *retGateSpace,
		 int *debug){

  //
  // Pull the text section of the input file out and
  // write it to a file.
  //
  FILE *ofp = fopen("/tmp/lidarASCII.dat","w");
  if (ofp == NULL){
    fprintf(stderr,"Could not create /tmp/lidarASCII.dat\n");
    exit(-1);
  }
  
  int byteLen = len * 4;
  for(int i=0; i < byteLen; i++){
    char c;
    fread(&c, sizeof(c), 1, fp);
    //
    // Leave out control code 13 - windows file.
    //
    if ((int)c != 13)    fprintf(ofp,"%c", c);
  }
  
  fclose(ofp);
  //
  //  Process the file we have just written.
  //
  processExistingText(retYear, retMonth, retDay,
		      retNumGates, retFirstGate, 
		      retGateSpace,
		      debug);

}

void processExistingText(int *retYear, int *retMonth, int *retDay,
			 int *retNumGates, float *retFirstGate, 
			 float *retGateSpace,
			 int *debug){
  //
  // Process the file to read the year, month, date and the
  // parameters we will need to calulate range gate spacing.
  //
  FILE *ifp = fopen("/tmp/lidarASCII.dat", "r");
  if (ifp == NULL){
    fprintf(stderr,"Could not read file /tmp/lidarASCII.dat\n");
    exit(-1);
  }

  double year, month, day;
  double sample_frequency;
  double raw_data_offset_meters;
  double raw_data_first_sample;
  double raw_data_sample_count;
  double range_gates;
  double samples_per_gate;
  double gates_to_merge;

  //
  // Read the year, month, day and sample frequency.
  // Must be done in this order (the order in which they appear
  // in the file).

  readKeyword(ifp, "YEAR", &year, 1);
  readKeyword(ifp, "MONTH", &month, 1);
  readKeyword(ifp, "DAY", &day, 1);
  
  *retYear = (int)rint(year);
  *retMonth = (int)rint(month);
  *retDay = (int)rint(day);
  
  readKeyword(ifp, "SAMPLE_FREQUENCY", &sample_frequency, 1);

  //
  // Seek ahead to the tag line prior to reading the other parameters.
  //
  // 
  readKeyword(ifp, 
	      "# VELOCITY, ASCOPE, VAD, UVW, BACK PROP RAW DATA PARAMETERS SECTION",
	      NULL, 0);

  //
  // From this section, read the rest of the parameters we need and close the file.
  //
  readKeyword(ifp, "RAW_DATA_OFFSET_METERS", &raw_data_offset_meters, 1);

  readKeyword(ifp, "RAW_DATA_FIRST_SAMPLE", &raw_data_first_sample, 1);
  readKeyword(ifp, "RAW_DATA_SAMPLE_COUNT", &raw_data_sample_count, 1);
  readKeyword(ifp, "RANGE_GATES", &range_gates, 1);
  readKeyword(ifp, "SAMPLES_PER_GATE", &samples_per_gate, 1);
  readKeyword(ifp, "GATES_TO_MERGE", &gates_to_merge, 1);

  fclose(ifp);

  //
  // Print these out.
  //

  if ( *debug ){
    fprintf(stderr,"\nTEXT DATA FOR %d/%02d/%02d :\n", (int)year, (int)month, (int)day);

    fprintf(stderr,"Sample frequency : %g\n", sample_frequency);
    fprintf(stderr,"Raw data offset (meters) : %g\n", raw_data_offset_meters);
    fprintf(stderr,"Raw data first sample : %g\n", raw_data_first_sample);
    fprintf(stderr,"Raw data sample count : %g\n", raw_data_sample_count);
    fprintf(stderr,"Range gates : %g\n", range_gates);
    fprintf(stderr,"Samples per gate : %g\n", samples_per_gate);
    fprintf(stderr,"Gates to merge : %g\n", gates_to_merge);
  }
  //
  // Now, do the calculation of range gate spacing. This is
  // right out of the text I have.
  //
  double RangePerSample = 1.5e+08/sample_frequency;

  int SamplesBetweenRangeGateCenters = (int)rint((raw_data_sample_count-samples_per_gate)/(range_gates-1.0));

  double RangeBetweenGateCenters = SamplesBetweenRangeGateCenters * RangePerSample;

  double RangePerGate = samples_per_gate * RangePerSample;

  double FirstRange = raw_data_offset_meters + (raw_data_first_sample + samples_per_gate/2.0)*RangePerSample;

  double CorrectedFirstRange = FirstRange + ((gates_to_merge-1.0)/2.0) * RangePerGate;
  //
  // Print out the gate spacing parameters.
  //
  if ( *debug ){
    fprintf(stderr,"First range : %gm\n", CorrectedFirstRange);
    fprintf(stderr,"Range step : %gm\n", RangeBetweenGateCenters);
    fprintf(stderr,"Last range : %gm\n\n", 
	    CorrectedFirstRange+range_gates*RangeBetweenGateCenters);
  }
  //
  // Store them in some class parameters.
  //
  *retFirstGate = CorrectedFirstRange;
  *retGateSpace = RangeBetweenGateCenters;
  *retNumGates = (int)rint(range_gates) - (int)rint(gates_to_merge) + 1;

  return;

}

//
// Small routine to read values from the text file. If tryRead is
// set, then a value is read into the double pointed at by val - otherwise the
// stream is positioned just after the keyword.
//
void readKeyword(FILE *fp, char *key, double *val, int tryRead){

  const int lineLen = 1024;
  char Line[lineLen];

  do {
    if (NULL == fgets(Line, lineLen, fp)){
      //
      // Must have hit the end of the file - should not have happened.
      //
      fprintf(stderr,"Could not locate keyword %s\n", key);
      exit(-1);
    }
    if (!(strncmp(Line, key, strlen(key)))){
      //
      // Found the key word.
      // If we don't have to read a value, return.
      //
      if (!(tryRead)) return;
      //
      // Otherwise, read the value.
      //
      char *p = Line + strlen(key);
      if (1 != sscanf(p, "%lf", val)){
	fprintf(stderr,"Could not decode keyword %s\n", key);
	exit(-1);
      }
      return;
    }
  } while(1);


}

//
// Byte swap routine.
//
void byteSwap4(void *p){

  unsigned char *b = (unsigned char *)p;

  unsigned char b1 = *b;
  unsigned char b2 = *(b+1);
  unsigned char b3 = *(b+2);
  unsigned char b4 = *(b+3);


  *(b+3) = b1;
  *(b+2) = b2;
  *(b+1) = b3;
  *b = b4;

  return;

}
//

//
// Read a long int, byte swapped.
//
int readLong(FILE **fp){

  unsigned char a,b,c,d;

  fread(&a, sizeof(unsigned char), 1, *fp);
  fread(&b, sizeof(unsigned char), 1, *fp);
  fread(&c, sizeof(unsigned char), 1, *fp);
  fread(&d, sizeof(unsigned char), 1, *fp);

  return a*256*256*256 + b*256*256 + c*256 + d;

}

} // End of extern C
