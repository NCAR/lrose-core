

#include <stdio.h>
#include <toolsa/umisc.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/pmu.h>        
#include <toolsa/ServerSocket.hh>
#include <toolsa/Socket.hh>
   
#include <iostream>

int main(int argc, char *argv[]){

  FILE *fp = fopen("../jun09-94-2.gfa","rb");

  ServerSocket server;

  const int size=4096;
  unsigned char buffer[size];


  int status = server.openServer(4500);
  if (status != 0) {
    cerr << "Error: Could not open server. Error Num: "
	 << server.getErrNum() << ". Error String: " << endl;
    cerr << server.getErrString().c_str() << endl;
    exit(1);
  }

  fprintf(stderr,"Waiting for client....\n");
 
  //
  // Block until client calls.
  //
  Socket *socket = server.getClient();


  fprintf(stderr,"Writing....\n");

  int ir;
  do{
    ir = fread(buffer,sizeof(unsigned char),size,fp);
    fprintf(stderr,"%d read.\n",ir);
    if (socket->writeBuffer(buffer,ir)!=0){
      fprintf(stderr,"Write failed.\n");
      exit(-1);
    }
    fprintf(stderr,"%d written.\n",ir);

    sleep(1);
  }while(ir == size);




  fclose(fp);

}

