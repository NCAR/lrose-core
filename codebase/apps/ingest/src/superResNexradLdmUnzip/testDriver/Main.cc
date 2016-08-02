
#include <cstdio>

#include <toolsa/umisc.h>
#include <toolsa/file_io.h>

int main(int argc, char *argv[]){

  date_time_t D;

  D.unix_time = time (NULL);
  D.unix_time = ((D.unix_time / 86400) + 2) * 86400;

  D.unix_time -= 500;

  int icount = 0;

  do {

    uconvert_from_utime( &D );

    char dirName[1024];
    sprintf(dirName,"./testDir/%d%02d%02d", D.year, D.month, D.day);

    ta_makedir_recurse(dirName);

    for (int seqnum = 1; seqnum < 11; seqnum++){
      char fileName[1024];
      char seqChar = 'I';
      if (seqnum == 1) seqChar = 'S';
      if (seqnum == 10) seqChar = 'E';
      sprintf(fileName, "%s/%02d%02d%02d_%02d_%c_V04.BZIP2",
	      dirName, D.hour, D.min, D.sec, seqnum, seqChar);

      FILE *fp = fopen(fileName, "w");
      fprintf(fp, "Test data\n");
      fclose(fp);

      fprintf(stderr,"Just created %s\n", fileName);

      sleep(10);

    }

    D.unix_time += 900; icount++;

  } while (icount < 2);


  return 0;

}
