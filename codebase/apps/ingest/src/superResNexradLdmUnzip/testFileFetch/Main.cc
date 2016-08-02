
#include <cstdio>

#include "fileFetch.hh"

int main(int argc, char *argv[]){

  fileFetch F("../testDriver/testDir", 900, FALSE, "BZIP", TRUE, 900);

  char filename[1024];
  date_time_t filenameTime;
  int seqnum;
  bool isEOV;

  do {
    F.getNextFile(filename, &filenameTime, &seqnum, &isEOV);
    fprintf(stderr,"Returned file %d : %s\n", seqnum, filename);
  } while(1);


  return 0;

}
