
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[]){

  if (argc < 2) return -1;



  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL) return -1;

  struct stat buf;
  if (stat(argv[1], &buf)) return -1;

  unsigned char b[4];


  int numCon=0;
  int lastAnum=0;
  while(1){

    int n = fread(b,1,4,fp);
    if (n != 4){
      fprintf(stderr,"First four failed, only read %d - this may be a normal exit\n", n);
      break;
    }

    if (0==strncmp((char *) b, "AR", 2)){
      fprintf(stderr,"HEADER\n");
      if (fseek(fp, 20, SEEK_CUR)){
	fprintf(stderr,"Header seek failed.\n");
	break;
      }
    } else {
      int msgSize = 256*b[0]+b[1];
      msgSize = msgSize * 2;
      int msgType = (int) b[3];


      unsigned long numRemaining = buf.st_size - ftell(fp) + 4;
      fprintf(stderr, "TYPE %d SIZE %d REMAINING %ld", 
	      msgType, msgSize, numRemaining);

      if (msgType != 31) fprintf(stderr,"\n");

      int numBytes = msgSize+16;
      if (msgType == 31)
	numBytes = msgSize+12;

      numBytes = numBytes - 4;


      if (numBytes > numRemaining){
	fprintf(stderr, "\nBottomed out with %d still in the hole\n",
		(int)(numBytes-numRemaining));
	break;
      }

      if (msgType == 31){
	//
	// Actually read this in so we can print some stuff
	//
	unsigned char *b = (unsigned char *)malloc(numBytes);
	fread(b, sizeof(unsigned char), numBytes, fp);
	int eNum = b[22+12];
	int aNum = 256*b[10+12]+b[11+12];

	if (aNum == lastAnum+1){
	  numCon++;
	} else {
	  numCon=0;
	}

	int numLost = aNum - lastAnum-1;
	lastAnum = aNum;

	float az,el;
	unsigned char *azp=(unsigned char *) &az;
	unsigned char *elp=(unsigned char *) &el;

	azp[3]=b[24];
	azp[2]=b[25];
	azp[1]=b[26];
	azp[0]=b[27];

	elp[3]=b[36];
	elp[2]=b[37];
	elp[1]=b[38];
	elp[0]=b[39];

	if (numLost > 0)
	  fprintf(stderr, " elevation %d (%g deg),  azimuth %d (%g deg) [%d consectuive az, lost %d]\n", 
		  eNum, el, aNum, az, numCon, numLost);
	else
	  fprintf(stderr, " elevation %d (%g deg),  azimuth %d (%g deg) [%d consectuive az]\n", 
		  eNum, el, aNum, az, numCon);

	free(b);
      } else {
	// Not message 31, just skip it
	if (fseek(fp, numBytes, SEEK_CUR)){
	  fprintf(stderr,"Message seek failed.\n");
	  break;
	}
      }
    }
    //    fprintf(stderr,"Offset : %d\n", ftell(fp));
  }



  fclose(fp);


  return 0;

}
