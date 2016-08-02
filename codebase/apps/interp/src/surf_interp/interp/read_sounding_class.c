
#include <stdio.h>
#include <string.h>

/* Re-coded from Fortran, Niles Oien Jan 1999.

There is definitely room for improvement in the way this is done - all
that has been perpetrated here is a translation from Fortran. Many of the
file handling routines could be done by C routines rather
than calling 'system'. I am not sure that this code will be retained,
however, which is why I am not upgrading it now.

      subroutine read_sounding_class(sounding_dir,pres_li,tli,
     1                               tli_default,badsf,debug)

      dimension press(1000),temp(1000)
      character fname1*64,fname2*128,dummyc*72
      character*128 comm
      character*64 sounding_dir
      logical debug


c.. first find latest sounding
*/

void read_sounding_class(char *sounding_dir,
			 float pres_li, float *tli,
			 float tli_default, float badsf,
			 int debug)
{
	char comm[1024], line[1024];
	FILE *ifp,*ifp2;
	char *last5;
	char fname1[1024], fname2[1024];
	int ilevel,ilevel2, nlevel;
	float press[1000],temp[1000];
        float td,hum,mixr,u,v,dd,ff;

      sprintf(comm,"/bin/rm dirsound.lis");
      system(comm); /* Could be replaced with a remove. */
 
  /* This should be done by reading the directory with diropen. */
      sprintf(comm,"ls -C1 -t %s >dirsound.lis",sounding_dir);
      system(comm);

      ifp=fopen("dirsound.lis","ra"); /* Was unit 2 */
	if (ifp==NULL){
		fprintf(stderr,"Listing of sounding directory failed.\n");
		exit(-1);
	}


	while(NULL!=fgets(fname1,1023,ifp)){
	 
        if (strlen(fname1) > 5) 
	  last5 = fname1 + strlen(fname1)-5;
	else
	  last5=NULL;
	
        if ((strlen(fname1) < 6) ||
	   (strcmp(last5,"class"))){

          fprintf(stderr,"%s is not a class file.\n",fname1);

          continue;

        }

	sprintf(fname2,"%s/%s",sounding_dir,fname1); /* should use PATH_DELIM */
  
	ifp2=fopen(fname2,"ra"); /* Was unit 20. */

	if (ifp2==NULL){
	  fprintf(stderr,"Could not open %s\n",fname2);
	  continue;
	}


	ilevel=-1;
      while((NULL!=fgets(line,1023,ifp2)) && (ilevel < 1000)){

	if (ilevel > -1){ /* Skip the first line. */
           sscanf(line,"%d %f %f %f %f %f %f %f %f %f",
	     &nlevel,&press[ilevel],&temp[ilevel],
           &td,&hum,&mixr,&u,&v,&dd,&ff);
	}

      ilevel++;

      } /* End of while. */

      fclose(ifp2);

      for (ilevel2 = 0; ilevel2 < ilevel-2; ilevel2++){

      if ((press[ilevel2] >= pres_li) && 
	   (press[ilevel2+1]<= pres_li)){

        *tli=temp[ilevel2]+(temp[ilevel2+1]-temp[ilevel2])*
     (press[ilevel2]-pres_li)/(press[ilevel2]-press[ilevel2+1]);

        if (debug)
	  fprintf(stdout,"Sounding %s reaches pressure %f mb\n",
	          fname1,pres_li);

        if (debug)
	  fprintf(stdout,"Temperature at %f mb is %f\n",
	          pres_li,*tli);                                                                                     
        fclose(ifp);
 
        return;

         }

	} /* End of ilevel2 loop */
/*
c.. if current sounding doesn't reach to pres_li, go to next latest sounding
*/
	} /* End of ilast loop. */

      fprintf(stdout,"No class soundings that pres_li in directory %s\n",
	sounding_dir);

      fprintf(stdout,"tli set to missing\n");
      *tli = badsf;
      if (ifp != NULL) fclose(ifp);
      return;



}


