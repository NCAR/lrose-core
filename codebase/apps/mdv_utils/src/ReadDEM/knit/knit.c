
#include <stdlib.h>
#include <stdio.h>

/* 

Small program to knit together two bits of the DEM
elevation model. User must edit header file manually, if desired.

Niles Oien 21 Feb 1999

*/


main(int argc, char *argv[])
{


  FILE *ifp1, *ifp2, *ofp;
  int LineSize1, LineSize2;
  int NumLines = 0,i,j,k,l;
  unsigned char *b1, *b2;

  /* Get CLI arguments. */

  if (argc < 6){

    fprintf(stderr,"USAGE : knit file1 linesize1 file2 linesize2 outfile\n");
    fprintf(stderr,"\tNOTE : line sizes are in BYTES not pixels.\n");
    exit(-1);
  }

  ifp1=fopen(argv[1],"rb");
  if (ifp1 == NULL){
    fprintf(stderr,"Failed to open %s\n",argv[1]);
    exit(-1);
  }

  ifp2=fopen(argv[3],"rb");
  if (ifp2 == NULL){
    fprintf(stderr,"Failed to open %s\n",argv[3]);
    fclose(ifp1);
    exit(-1);
  }

  LineSize1 = atoi(argv[2]); LineSize2=atoi(argv[4]);

  b1= (unsigned char *) malloc (LineSize1);
  b2= (unsigned char *) malloc (LineSize2);

  if ((b1 == NULL) || (b2 == NULL)){
    fprintf(stderr,"Malloc failed.\n");
    exit(-1);
  }

  ofp=fopen(argv[5],"wb");
  if (ofp == NULL){
    fprintf(stderr,"Failed to create %s\n",argv[5]);
    fclose(ifp1);
    exit(-1);
  }


  while ( (!(feof(ifp1))) && (!(feof(ifp2))) ){

    i=fread(b1,sizeof(char),LineSize1,ifp1);
    j=fread(b2,sizeof(char),LineSize2,ifp2);

    if ( ((feof(ifp1))) && ((feof(ifp2))) ) continue;
    NumLines++;

    if ( (i!=LineSize1) || (j!=LineSize2)){
     fprintf(stderr,
	      "Read failed on line %d - are the line sizes correct?\n",
	      NumLines);
      exit(-1);
    }

    k=fwrite(b1,sizeof(char),LineSize1,ofp);
    l=fwrite(b2,sizeof(char),LineSize2,ofp);
 
    if ( (k!=LineSize1) && (l!=LineSize2)){
      fprintf(stderr, "Write failed on line %d\n",NumLines);
      exit(-1);
    }

  }



  if ( (!(feof(ifp1))) || (!(feof(ifp2))) ){

    fprintf(stderr,"ERROR : both files did not end at the same time.\n");

  } else {

    fprintf(stdout,"%d lines written to %s\n",NumLines, argv[5]);

  }


  fclose(ifp1); fclose(ifp2); fclose(ofp);
  free(b1); free(b2);


}
