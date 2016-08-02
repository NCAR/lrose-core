/*
	signals.c

	Signal Handler for 3D-Rapic

*/

#define _BSD_SIGNALS
#include <signal.h>
#include <stdio.h>
#include "bool.h"

bool signals_handled = false;


#ifdef USE_SIGACTION
extern void HandleSignal(int signo, siginfo_t *siginfo, void *ptr);
void init_signals() {
  struct sigaction sa;
  sa.sa_sigaction = HandleSignal;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_SIGINFO;
  sigaction(SIGHUP, &sa, 0);
  sigaction(SIGUSR1, &sa, 0);
  sigaction(SIGUSR2, &sa, 0);
  sigaction(SIGCHLD, &sa, 0);
  sigaction(SIGTERM, &sa, 0);
  sigaction(SIGABRT, &sa, 0);
  sigaction(SIGQUIT, &sa, 0);
  sigaction(SIGINT, &sa, 0);
  sigaction(SIGPIPE, &sa, 0);
  sigaction(SIGALRM, &sa, 0);
  sigaction(SIGCONT, &sa, 0);
  sigaction(SIGSTOP, &sa, 0);
  sigaction(SIGTTIN, &sa, 0);
  sigaction(SIGTTOU, &sa, 0);
  sigaction(SIGURG, &sa, 0);
  sigaction(SIGXCPU, &sa, 0);
  sigaction(SIGXFSZ, &sa, 0);
  sigaction(SIGVTALRM, &sa, 0);
//   sigaction(SIGPROF, &sa, 0);
  sigaction(SIGWINCH, &sa, 0);
  sigaction(SIGPOLL, &sa, 0);
  sigaction(SIGSYS, &sa, 0);
  sigaction(SIGIO, &sa, 0);
#ifdef AIX
  sigaction(SIGMSG, &sa, 0);
  sigaction(SIGDANGER, &sa, 0);
#endif // AIX
  // Ignore the SIGPIPE signal, which can occur if a socket closes.
  sa.sa_handler = SIG_IGN;
}
#else  // USE_SIGACTION
#ifdef sun
void HandleSignal(int signo);
void sighandler(int signo) {
  HandleSignal(signo);
//  return(0);
  }
#else  // sun
extern void HandleSignal(int signo, int code, sigcontext *sc );
void sighandler(int signo, int code, sigcontext *sc ) {
  HandleSignal(signo, code, sc);
//  return(0);
  }
#endif // sun

void init_signals() {
//  IRIX5.3 uses this form, comment out void (*)(int) form
/*
#ifdef sgi
    signal(SIGHUP, (void (*)(... )) sighandler);
    signal(SIGINT, (void (*)(... )) sighandler);
    signal(SIGALRM, (void (*)(... )) sighandler);
    signal(SIGTERM, (void (*)(... )) sighandler);
    signal(SIGPIPE, (void (*)(... )) sighandler);
    signal(SIGABRT, (void (*)(... )) sighandler);
*/
    signal(SIGHUP, (void (*)(int)) sighandler);
    signal(SIGINT, (void (*)(int)) sighandler);
    signal(SIGQUIT, (void (*)(int)) sighandler);
    //  signal(SIGILL, (void (*)(int)) sighandler);
    signal(SIGTRAP, (void (*)(int)) sighandler);
    signal(SIGABRT, (void (*)(int)) sighandler);
    //  signal(SIGBUS, (void (*)(int)) sighandler);
    //  signal(SIGFPE, (void (*)(int)) sighandler);
    signal(SIGKILL, (void (*)(int)) sighandler);
    signal(SIGUSR1, (void (*)(int)) sighandler);
    //  signal(SIGSEGV, (void (*)(int)) sighandler);
    signal(SIGUSR2, (void (*)(int)) sighandler);
    signal(SIGPIPE, (void (*)(int)) sighandler);
    signal(SIGALRM, (void (*)(int)) sighandler);
    signal(SIGTERM, (void (*)(int)) sighandler);
    //  signal(SIGSTKFLT, (void (*)(int)) sighandler);
    signal(SIGCHLD, (void (*)(int)) sighandler);
    signal(SIGCONT, (void (*)(int)) sighandler);
    signal(SIGSTOP, (void (*)(int)) sighandler);
    //    signal(SIGTSTP, (void (*)(int)) sighandler);
    signal(SIGTTIN, (void (*)(int)) sighandler);
    signal(SIGTTOU, (void (*)(int)) sighandler);
    signal(SIGURG, (void (*)(int)) sighandler);
    signal(SIGXCPU, (void (*)(int)) sighandler);
    signal(SIGXFSZ, (void (*)(int)) sighandler);
    signal(SIGVTALRM, (void (*)(int)) sighandler);
//     signal(SIGPROF, (void (*)(int)) sighandler);
    signal(SIGWINCH, (void (*)(int)) sighandler);
    signal(SIGPOLL, (void (*)(int)) sighandler);
    signal(SIGSYS, (void (*)(int)) sighandler);
    signal(SIGIO, (void (*)(int)) sighandler);
#ifdef AIX
    signal(SIGMSG, (void (*)(int)) sighandler);
    signal(SIGDANGER, (void (*)(int)) sighandler);
#endif // AIX
    signals_handled = true;
}
#endif  // USE_SIGACTION

