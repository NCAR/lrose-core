// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 

//
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <vector>
#include <signal.h>
#include <pthread.h>

#include <toolsa/pmu.h>
#include <didss/DsInputPath.hh>
//
//
#include "Params.hh"
#include "hiResNidsStormPrecip2Mdv.hh"
//
//
using namespace std;

// Mutex and integer we want to lock access to
// so we can count exited threads
pthread_mutex_t m;
int numThreadsExited;

// Similar, used to count number of threads actually working.
//
pthread_mutex_t ntMutex;
int nt;




//
// Struct to pass args into the method we use for threads.
//
typedef struct {
  int radarIndex;
  Params *paramPointer;
} args_t;

//
// Space for an array of these (memory will be allocated later).
//
args_t *argArray;



//
// The method we use for threads.
//
void *threadMethod(void *a){

  args_t *A;
  A = (args_t *) a;

  //
  // Make local copies of the args and instantiate object from those.
  //
  int radarIndex = A->radarIndex;
  Params *p = A->paramPointer;
  
  hiResNidsStormPrecip2Mdv H(p, radarIndex);

  //
  // Increment the number of exited threads. Needs mutex lock.
  //
  pthread_mutex_lock(&m);
  numThreadsExited++;
  pthread_mutex_unlock(&m);

  pthread_exit(NULL);

}



//
// The 'tidy_and_exit' routine is called by the program if the
// system gets an interrupt signal.
//
static void tidy_and_exit (int sig);
//
// Main program starts.
// 
int main(int argc, char *argv[])
{

  for (int i=1; i < argc; i++){
    if (0==strcmp(argv[i], "-h")){
      cerr << "Use " << argv[0] << " -print_params for help" << endl;
      exit(0);
    }
  }
  //
  // Trap signals with reference to the 'tidy_and_exit' routine.
  //
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  

  //
  // Get the Table Driven Runtime Parameters loaded
  // into the 'tdrp' structure.
  //
  Params *tdrp = new  Params(); 

  if (tdrp->loadFromArgs(argc, argv, NULL, NULL)){
    fprintf(stderr,"Specify params file with -params option.\n");
    exit(-1);
  } 

  PMU_auto_init("hiResNidsStormPrecip2Mdv", tdrp->instance, PROCMAP_REGISTER_INTERVAL);

  if (tdrp->forkIt){
    //
    // Fork processes to deal with all radars.
    //
    vector <pid_t> pids;
  
    int numSkipped = 0;
    for (int iradar=0; iradar < tdrp->radars_n; iradar++){
      //
      // In some settings where this app is to be used, the radar name
      // will be an env var and if TDRP does not expand it then it is
      // not valid. If that's the case it starts with "$(". Skip these
      // radar names.
      //
      char *q = tdrp->_radars[iradar];

      if ((strlen(q) < 1) || ((q[0] == '$') && (q[1] == '('))){
	fprintf(stderr,"Radar name %s looks like an undefined env var, skipping...\n",q);
	numSkipped++;
	continue;
      }
    
      PMU_auto_register("Forking processes");
      
      pid_t p = fork();
      if (p == -1){ // Fork fail - highly unlikely.
	fprintf(stderr,"Failed on fork for radar %s, field %s\n", 
		tdrp->_radars[iradar], tdrp->field.inFieldName);
	return -1;
      }
      
      if (p != 0){ // Parent. Just remember the forked pid.
	pids.push_back( p );
      } else { // Child. Instantiate an object to watch the directory.
	hiResNidsStormPrecip2Mdv H(tdrp, iradar);
	exit(0);
      }
      
    }

    if (tdrp->radars_n == numSkipped) return -1; // Skipped all radars?

    int numExited = 0;
 
    pid_t changedPid;
    while (1) {
      PMU_auto_register("Waiting for forked processes.");
      sleep (1);
      int status = -1;
      changedPid = waitpid(-1, &status, WNOHANG );
      if (changedPid == 0) continue; // No child left behind
      if (changedPid == -1) break; // waitpid() fail
      //
      // If we got here a child has been left behind.
      //
      if (tdrp->mode == Params::REALTIME) break; // In realtime no child should exit, we need to restart
      //
      // If we got here we are in archive mode, waiting for all child processes to exit
      //
      numExited++;
      if (numExited == (tdrp->radars_n - numSkipped)) break;
    }

    if ((changedPid == -1) || (tdrp->mode == Params::REALTIME)){
      //
      // Either the call to waitpid() failed, or a child exited in realtime.
      // In either case, terminate the children and exit.
      //
      for (unsigned i=0; i < pids.size(); i++){
	if (pids[i] != changedPid) kill(pids[i], SIGKILL);
      }
    }

    tidy_and_exit( 0 );

  } else {
    //
    // Use threads rather than forks.
    //
    numThreadsExited = 0;
    pthread_mutex_init( &m, NULL);

    nt=0;
    pthread_mutex_init( &ntMutex, NULL);

    vector <pthread_t> threads;
    unsigned argIndex = 0;

    argArray = (args_t *)malloc(tdrp->radars_n*sizeof(args_t));

    if (argArray == NULL){
      fprintf(stderr,"Malloc failed\n");
      exit(-1);
    }

    for (int iradar=0; iradar < tdrp->radars_n; iradar++){
      //
      // In some settings where this app is to be used, the radar name
      // will be an env var and if TDRP does not expand it then it is
      // not valid. If that's the case it starts with "$(". Skip these
      // radar names.
      //
      char *q = tdrp->_radars[iradar];
      
      if ((strlen(q) < 1) || ((q[0] == '$') && (q[1] == '('))){
	fprintf(stderr,"Radar name %s looks like an undefined env var, skipping thread...\n",q);
	argIndex++;
	continue;
      }
      
      PMU_auto_register("Threading processes");
      
      argArray[argIndex].radarIndex = iradar;
      argArray[argIndex].paramPointer = tdrp;
      
      pthread_t thread;
      
      int rc = pthread_create(&thread, NULL, threadMethod, (void *) &argArray[argIndex] );
      if (rc){
	fprintf(stderr, "ERROR : radar %d return code from pthread_create() is %d\n",
		iradar, rc);
	return -1;
      }
      argIndex++;

      //
      // Detatch the threads. This means we can't pthread_join() on them
      // later - but pthread_join() blocks anyway. Instead, we'll use the
      // mutex to count threads as they exit and then exit
      // the main thread when all the sub threads(?) have exited.
      //
      if (pthread_detach(thread)){
	fprintf(stderr, "Failed to detatch thread\n");
	return -1;
      }

      threads.push_back(thread);

    }


    //
    // In realtime, exit if one thread exits.
    // In archive mode, exit after all threads exit.
    //
    while(1){
      PMU_auto_register("Running");
      sleep(1);
      if (tdrp->mode == Params::REALTIME){
	if (numThreadsExited != 0) break; // Should not happen
      } else {
	if (numThreadsExited == (int)threads.size()) break; // Hopefully will happen.
      }
    }


    if (tdrp->mode == Params::REALTIME){
      // Need to kill all remaining threads.
      for (unsigned it=0; it < threads.size(); it++){
	pthread_kill(threads[it], SIGKILL);
      }
    }

    free(argArray);
    threads.clear();

    pthread_mutex_destroy( &m );
    pthread_mutex_destroy( &ntMutex );

    pthread_exit(NULL);
    
  }



}

/////////////////////////////////////////////
//
// Small routine called in the event of an interrupt signal.
//
static void tidy_and_exit (int sig){
  PMU_auto_unregister();
  exit(sig);
}








