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
///////////////////////////////////////////////////////////////
//
// main for Lucid
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2023
//
///////////////////////////////////////////////////////////////

#include "Lucid.hh"
#include <QApplication>
#include <toolsa/LogStream.hh>
#include <QIcon>
#include <QSocketNotifier>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <unistd.h>

// file scope

static bool init_signal_handlers();
static void reset_signal_handlers();
static void signal_trap(int signal);

// POSIX signal handlers cannot safely call Qt, log messages, or destroy C++
// objects.  Use this pipe as an async-signal-safe bridge: the handler writes
// the signal number and a QSocketNotifier handles it later on the Qt thread.

static int SignalPipe[2] = {-1, -1};
static volatile sig_atomic_t SignalWriteFd = -1;

// main

int main(int argc, char **argv)

{

  // create program object

  try {

    // The following line may fix the problem of the Boundary Editor
    // not appearing in the menu bar when Lucid is built using
    // linux, run on a Mac.                                                                                                                       
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
    
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon("://LucidCycloneIcon.icns"));
    LOG(DEBUG_VERBOSE) << "After setting Window Icon\n";
    std::unique_ptr<Lucid> prog(new Lucid(argc, argv));
    if (!prog->OK) {
      return EXIT_FAILURE;
    }

    // Arrange for SIGINT and SIGTERM to request an orderly Qt shutdown.  This
    // allows Lucid and its MDV worker threads to finish their normal cleanup
    // rather than destroying Qt objects from inside a POSIX signal handler.

    if (!init_signal_handlers()) {
      cerr << "ERROR - Lucid: cannot initialize signal handlers" << endl;
      return EXIT_FAILURE;
    }

    int terminationSignal = 0;
    QSocketNotifier signalNotifier(SignalPipe[0],
                                   QSocketNotifier::Read,
                                   &app);

    // Drain all signals already queued in the pipe, but preserve the first one
    // as the reason for termination.  Disable the notifier before leaving the
    // event loop so shutdown is initiated only once.

    auto handleSignal = [&]() {
      int receivedSignal = 0;
      while (read(SignalPipe[0],
                  &receivedSignal,
                  sizeof(receivedSignal)) == sizeof(receivedSignal)) {
        if (terminationSignal == 0) {
          terminationSignal = receivedSignal;
        }
      }

      signalNotifier.setEnabled(false);
      cerr << "Lucid: received signal " << terminationSignal << endl;
      app.quit();
    };

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QObject::connect(&signalNotifier, &QSocketNotifier::activated,
                     &app,
                     [&](QSocketDescriptor, QSocketNotifier::Type) {
                       handleSignal();
                     });
#else
    QObject::connect(&signalNotifier, &QSocketNotifier::activated,
                     &app,
                     [&](int, QSocketNotifier::Type) {
                       handleSignal();
                     });
#endif
    
    // run it
    
    int iret = prog->RunApp(app);
    
    // Clean up normally on the Qt thread. MdvReader destruction may wait for
    // an outstanding blocking request to reach its configured timeout.
    
    prog.reset();
    reset_signal_handlers();

    if (terminationSignal != 0) {
      return 128 + terminationSignal;
    }
    return iret;
    
  } catch (std::bad_alloc &a) {
    cerr << ">>>>> bad alloc: " << a.what() << endl;
    reset_signal_handlers();
    return EXIT_FAILURE;
  }

}

/*****************************************************************
 * SIGNAL_TRAP: notify the Qt event loop of a termination signal.
 *
 * write() is async-signal-safe. All logging, Qt calls and object cleanup are
 * deliberately performed later by the QSocketNotifier callback.
 */

static void signal_trap(int signal)
{
  const int savedErrno = errno;
  const int fd = SignalWriteFd;
  if (fd >= 0) {
    const int signalValue = signal;
    const ssize_t writeStatus =
      write(fd, &signalValue, sizeof(signalValue));
    (void) writeStatus;
  }
  errno = savedErrno;
}

/*****************************************************************
 * INIT_SIGNAL_HANDLERS:
 */

static bool init_signal_handlers()
{
  if (pipe(SignalPipe) != 0) {
    cerr << "ERROR - Lucid: cannot create signal pipe: "
         << strerror(errno) << endl;
    return false;
  }

  // Nonblocking I/O guarantees that signal_trap() can never wait for pipe
  // space.  Close-on-exec prevents this internal notification channel from
  // leaking into any child program Lucid might launch.

  for (int ii = 0; ii < 2; ++ii) {
    int flags = fcntl(SignalPipe[ii], F_GETFL, 0);
    if (flags < 0 ||
        fcntl(SignalPipe[ii], F_SETFL, flags | O_NONBLOCK) < 0) {
      cerr << "ERROR - Lucid: cannot make signal pipe nonblocking: "
           << strerror(errno) << endl;
      reset_signal_handlers();
      return false;
    }

    flags = fcntl(SignalPipe[ii], F_GETFD, 0);
    if (flags < 0 ||
        fcntl(SignalPipe[ii], F_SETFD, flags | FD_CLOEXEC) < 0) {
      cerr << "ERROR - Lucid: cannot set close-on-exec on signal pipe: "
           << strerror(errno) << endl;
      reset_signal_handlers();
      return false;
    }
  }

  SignalWriteFd = SignalPipe[1];

  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = signal_trap;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;

  if (sigaction(SIGINT, &action, nullptr) != 0 ||
      sigaction(SIGTERM, &action, nullptr) != 0) {
    cerr << "ERROR - Lucid: cannot install termination signal handler: "
         << strerror(errno) << endl;
    reset_signal_handlers();
    return false;
  }

#ifdef SIGIO
  // Lucid does not use asynchronous file-descriptor notification.  Preserve
  // the previous behavior of ignoring SIGIO instead of treating it as a
  // termination request.

  struct sigaction ignoreAction;
  memset(&ignoreAction, 0, sizeof(ignoreAction));
  ignoreAction.sa_handler = SIG_IGN;
  sigemptyset(&ignoreAction.sa_mask);
  ignoreAction.sa_flags = 0;
  sigaction(SIGIO, &ignoreAction, nullptr);
#endif

  return true;
}

///////////////////////////////////////////
// restore default handlers and close pipe

static void reset_signal_handlers()
{
  SignalWriteFd = -1;

  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = SIG_DFL;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  sigaction(SIGINT, &action, nullptr);
  sigaction(SIGTERM, &action, nullptr);
#ifdef SIGIO
  sigaction(SIGIO, &action, nullptr);
#endif

  for (int ii = 0; ii < 2; ++ii) {
    if (SignalPipe[ii] >= 0) {
      close(SignalPipe[ii]);
      SignalPipe[ii] = -1;
    }
  }
}
