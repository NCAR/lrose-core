#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)ntfy_test.c 20.14 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ntfy_test.c - Test suite for the notifier.
 */

#include <sys/file.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <xview_private/ntfy.h>
#include <xview_private/portable.h>

/*
 * Not yet tested: notify_set_exception_func notify_get_exception_func Set up
 * tcp connection and use SIGURG
 * 
 * notify_output notify_exception notify_wait3 Used by default prioritizer.
 * Need to do partial notify of conditions and let default do the rest.
 * 
 * Program input data: Startup parameters: Duration: Less of two limits Time:
 * -t(ime) nsecs (default - 20) Actions Processed: -a(ctions) n (default -
 * infinite) Output: (stdout) Verbose: -v(erbose) on/off (default - off)
 * Errors: (stderr) Unix event generation speed: -s(peed) fast/med/slow
 * (default med) Controls speed of generating signals and fd activity. After
 * posting an event gotten out of an action file (as test process): fast:
 * Don't delay med: Sleep 1 second slow: Sleep 5 second After posting an
 * event gotten out of an action file (as exerciser process): fast: Delay
 * 1/10th of a second med: Sleep 1 second slow: Sleep 5 second
 * Client-on-stack: -i(nput) none/read/select (default - select) none: Simply
 * call notify_start read: Use read only select: Use select and read Clients:
 * -c(lient) random/filename/fixed n (default - -c random 2 -c fixed 2)
 * (filename convention: nts.*) random: Generate condition/events randomly
 * filename: Action file to determine initial setup and condition changes and
 * event generation fixed: Built-in action file Child process exerciser: -x
 * wait3/filename (default - wait3) (filename convention: nte.*) wait3:
 * Change state repetitively filename: Get SIGNAL events out of this actions
 * file (see below) and send to parent process This process forked from
 * notifier test process and should serve as an exerciser Client action file
 * format: CONDITION cycle_count (no cycle_count means infinite) Cycle_count
 * is the number of times that will go thru a condition before becoming
 * simply NONE. action (see below) action (see below) ... CONDITION ... Each
 * condition section serves as a mini-script of condition change commands and
 * event generation commmand.  The section has 2 implicit indices that chase
 * around the section; one for the current condition and one for the current
 * event. The indices are reevaluated whenever the condition is notified.
 * 
 * Reevaluation involves incrementing the current event index until an unknown
 * event is found.  Valid event lines generate actual events during this
 * search.  There is a test to prevent the same event from being executing
 * multiple times in one sweep.
 * 
 * Next, the current event index in incremented until an known condition is
 * found.  A valid condition line sets an actual condition.  There is a test
 * to prevent unnecessarily resetting the existing condition to itself.
 * 
 * When have an exerciser script, change condition commands are not allowed.
 * 
 * When have a tester script, single events (without a condition) are not
 * allowed.
 * 
 * actions: change condition: INPUT pipe# [async] [ndelay] Default is normal
 * blocking. OUTPUT pipe# Tag EXCEPTION pipe# block/async (???) REAL seconds
 * useconds tag (Multiple reals not allowed in client. If do, first one [in
 * the action list] is the one detected as going off). VIRTUAL seconds
 * useconds tag (See REAL). ASYNC signal_name SYNC signal_name WAIT3
 * program_name args (e.g., ntfy_tchild -x wait3) (If child stopped, will do
 * next action only if event so wouldn't abandon child process). DESTROY
 * (Once death or cleanup received the client and all its conditions are
 * history because the notifier axed the client completely.  Follow by NONE.
 * However, SEND_DESTROY is still alive because notification happens
 * immediately). VETOER (See DESTROY). SAFE IMMEDIATE NONE generate event:
 * (Doesn't affect current condition). WRITE pipe# string (Companion to
 * INPUT). READ pipe# (Companion to OUTPUT). SIGNAL signal_name (If no
 * condition waiting then might blow process out of water if default action
 * is to core dump). ITIMER_REPORT real/virtual (Reports current itimer
 * value). (Shouldn't report real/virtual itimer immediately after getting
 * such a notification because the condition will be undefined). SEND_DESTROY
 * death/checking/cleanup (See DESTROY). SEND_SAFE string (If use string
 * greater than one char then can expect "NOTIFICATION UNEXPECTED: SAFE"
 * messages when stop from keyboard). SEND_IMMEDIATE string (See SEND_SAFE).
 * EXERCISER program_name args (e.g., ntfy_tchild -x nte.*) (Follow this with
 * NONE condition). NOTHING tag
 */

/*
 * Default built-in action file:
 */
char           *fixed_actions = "\
CONDITION\n\
INPUT pipe0 block\n\
\n\
CONDITION\n\
INPUT pipe2 async\n\
\n\
CONDITION\n\
OUTPUT pipe1\n\
\n\
CONDITION\n\
SYNC SIGXFSZ\n\
SEND_SAFE F\n\
\n\
CONDITION\n\
ASYNC SIGXCPU\n\
SEND_IMMEDIATE C\n\
WRITE pipe0 Hello\n\
\n\
CONDITION\n\
SAFE\n\
\n\
CONDITION\n\
IMMEDIATE\n\
\n\
CONDITION\n\
WAIT3 ntfy_tchild -x wait3\n\
NONE\n\
\n\
CONDITION\n\
VETOER\n\
\n\
CONDITION\n\
REAL 3 1 Rrrrring\n\
SEND_SAFE S\n\
REAL 0 1 poll\n\
READ pipe1\n\
ITIMER_REPORT virtual\n\
\n\
CONDITION\n\
VIRTUAL 0 100000 More_money_please\n\
SIGNAL SIGXFSZ\n\
VIRTUAL 0 1 poll1\n\
SIGNAL SIGXCPU\n\
VIRTUAL 0 10 short\n\
WRITE pipe2 Hi\n\
SIGNAL SIGIO\n\
ITIMER_REPORT real\n\
";

/*
 * Description of a single condition section in a action file.
 */
#define	MAX_CONDITIONS	20
typedef struct condition {
    int             cycle;	/* Current interaction thru actions */
    int             max_cycles;	/* How many time to go thru actions */
    char           *first_action;	/* 1st action */
    char           *current_action;	/* Current action */
    char           *current_condition;	/* Current condition registered with
					 * notifier */
    char           *current_event;	/* Current event posted to notifier */
    char           *last_char;	/* No more good data past here */
    union data {		/* Current condition related data */
	int             pipe;	/* Client pipe pair index */
	int             signal;	/* Signal waiting for notification of */
	int             pid;	/* Pid waiting for */
	int             i;	/* Generic int */
    }               data;
    int             flags;
#define	CD_NO_UNSET_COND	0x1	/* Don't unset current_condition */
    struct timeval  set_tod;	/* Remember when set itimer */
    struct timeval  set_tv;	/* Remember what set itimer */
}               CONDITION;

#define	CONDITION_NULL	((CONDITION *)0)

/*
 * Description of a client.
 */
typedef struct pipe {
    int             out;	/* Output end of pipe (read from this) */
    int             in;		/* Input end of pipe (write to this) */
    int             flags;	/* State of pipe */
#define	PIPE_ASYNC		0x1	/* async mode */
#define	PIPE_OPEN		0x2	/* open */
#define	PIPE_NDELAY		0x4	/* fndelay mode */
}               PIPE;

#define	MAX_CLIENTS	10
#define	MAX_PIPES	5
typedef struct client {
    char           *data;	/* Dynamically allocated condition string */
    struct condition conditions[MAX_CONDITIONS];
    int             cond_next;	/* Last meaningful entry of conditions */
    struct pipe     pipes[MAX_PIPES];	/* Conditions share pipes */
    int             flags;
#define	CLT_REMOVED	0x1
    Notify_func     prioritizer;/* Cached prioritizer (will call if have own) */
}               CLIENT;
CLIENT          clients[MAX_CLIENTS];
int             client_next;	/* Last meaningful entry of clients */

/*
 * Variables to control length of program execution.
 */
#define	INFINITE	100000000
struct itimerval halt_itimer = {{INFINITE, 0}, {INFINITE, 0}};

/* Program virtual time run limit before quitting */
int             action_limit = INFINITE;
/* Number of actions executed before quitting */
int             set_conditions = 1;
/* Flag to control further setting of conditions */

/*
 * Program mode flags
 */
int             verbose;	/* Print voluminous output */
int             exerciser;	/* Generating Unix activity as a separate
				 * process */
int             wait3er;	/* Acting as child process to tester */

int             test_pid;	/* Test process pid (can be multiple
				 * exerciser and/or wait3er child process) */
int             terminated;	/* SIGTERM received by wait3er or exerciser */
int             nt_errors;	/* Number of non-fatal (didn't cause an exit)
				 * errors */
int             nt_ntfy_errno_print;	/* Stored ntfy_errno_print */

/*
 * How fast to generate Unix activity that the notifier will then detect.
 */
enum generation_speed {
    FAST = 0,
    MEDIUM = 1,
    SLOW = 2,
}               generation_speed = FAST;	/* When exerciser then

						 * default is MED */

enum input_method {
    NONE = 0,
    READ = 1,
    SELECT = 2,
}               input_method = SELECT;

CONDITION      *nt_expected();

extern          errno;
char           *progname;
Notify_value    nt_halt();
Notify_value    nt_scheduler();
Notify_func     nt_cached_sched;

main(argc, argv)
    char          **argv;
{
    register int    clt_i, cond_i;
    char            c;
    int             sigurg();
    enum generation_speed gs;
    CLIENT         *cltp;
    CONDITION      *condp;

    progname = *argv;
    argc--;
    argv++;
    /* Parse cmd line and create clients */
    nt_parse(argc, argv);
    /* If exerciser then avoid normal processing */
    if (exerciser || wait3er) {
	test_pid = getppid();
	if (exerciser)
	    be_exerciser();
	else
	    be_wait3er();
	exit(0);
    }
    test_pid = getpid();
    signal(SIGURG, sigurg);	/* Used to break out of select loop */
    /* Setup my scheduler */
    nt_cached_sched = notify_set_scheduler_func(nt_scheduler);
    if (notify_get_scheduler_func() != nt_scheduler)
	fprintf(stderr, "GOT WRONG SCHEDULER\n");
    /* Setup initial conditions */
    gs = generation_speed;
    generation_speed = gs;
    for (clt_i = 0; clt_i < client_next; clt_i++)
	for (cond_i = 0; cond_i < clients[clt_i].cond_next; cond_i++)
	    nt_next_action(&(clients[clt_i]),
			   &(clients[clt_i].conditions[cond_i]));
    generation_speed = gs;
    /* Setup my prioritizer */
    if (client_next > 0) {
	Notify_value    nt_prioritizer();

	cltp = &clients[client_next - 1];
	cltp->prioritizer = notify_set_prioritizer_func(
						      cltp, nt_prioritizer);
	if (notify_get_prioritizer_func(cltp) != nt_prioritizer)
	    fprintf(stderr, "GOT WRONG PRIORITIZER\n");
    }
    /* Setup timeout */
    notify_set_itimer_func(&terminated, nt_halt, ITIMER_VIRTUAL,
			   &halt_itimer, (struct itimerval *) 0);
    /* Main processing */
    if (input_method == NONE) {
	if (notify_start() != NOTIFY_OK) {
	    notify_perror("NOTIFY_START");
	    exit(1);
	}
    } else {
	/* Wait for keyboard input */
	while (!terminated) {
	    if (input_method == SELECT) {
		int             ibits, obits, ebits, nfds;

		ibits = 1;	/* stdin */
		obits = ebits = 0;
		if ((nfds = select(32, &ibits, &obits, &ebits,
				   (struct timeval *) 0)) == -1) {
		    if (errno == EINTR)
			/* Some notifier managed sig */
			continue;
		    else {
			perror("SELECT");
			notify_perror("SELECT");
			exit(1);
		    }
		}
		if (nfds != 1) {
		    fprintf(stderr, "select nfds != 1\n");
		    notify_perror("SELECT");
		    exit(1);
		}
		if (ibits != 1) {
		    fprintf(stderr, "select ibits != 1\n");
		    notify_perror("SELECT");
		    exit(1);
		}
	    }
	    if (read(0, &c, 1) != 1) {
		fprintf(stderr, "read != 1\n");
		notify_perror("READ");
		exit(1);
	    }
	    switch (c) {
	      case '?':	/* Help */
		fprintf(stderr, "Character commands:\n\
c\tTry die with cleanup\n\
d\tTry die with process death\n\
q\tQuit\n\
s\tStop further condition setting\n\
t\tSend SIGTERM to this process\n\
v\tTry die with checking\n\
#\tRemove numbered client\n");
		break;
	      case 'c':	/* Try die with cleanup */
		fprintf(stderr, "Cleanup death\n");
		if (notify_die(DESTROY_CLEANUP) != NOTIFY_OK)
		    notify_perror("notify_die");
		else
		    terminated = 1;
		break;
	      case 'd':	/* Try die with process death */
		fprintf(stderr, "Process death\n");
		if (notify_die(DESTROY_PROCESS_DEATH) !=
		    NOTIFY_OK)
		    notify_perror("notify_die");
		else
		    terminated = 1;
		break;
	      case 'q':	/* Quit */
		terminated = 1;
		fprintf(stderr, "Tester quit from keyboard\n");
		break;
	      case 's':	/* Halt any further condition setting so will
				 * expire gracefully soon */
		set_conditions = 0;
		fprintf(stderr,
			"Tester wouldn't set anymore conditions\n");
		fprintf(stderr,
			"Type q to terminate completely\n");
		break;
	      case 't':	/* Send SIGTERM to this process */
		fprintf(stderr, "Send SIGTERM to process\n");
		if (kill(test_pid, SIGTERM) == -1)
		    perror("KILL ME SIGTERM");
		break;
	      case 'v':	/* Try die with checking */
		fprintf(stderr, "CHECKING death: ");
		switch (notify_die(DESTROY_CHECKING)) {
		  case NOTIFY_OK:
		    fprintf(stderr, "OK\n");
		    break;
		  case NOTIFY_DESTROY_VETOED:
		    fprintf(stderr, "VETOED\n");
		    break;
		  default:
		    notify_perror("notify_die");
		}
		break;
	      case '\n':	/* Ignore newlines returns */
		break;
	      default:{
		    char            str[2];
		    int             clt_num;

		    /* Remove/restore client specified by number */
		    str[0] = c;
		    str[1] = '\n';
		    clt_num = atoi(str);
		    if (clt_num >= 0 && clt_num < MAX_CLIENTS) {
			cltp = &clients[clt_num];
			if (cltp->flags & CLT_REMOVED) {
			    fprintf(stderr,
				    "Restore client [%ld]\n", clt_num);
			    for (cond_i = 0;
				 cond_i < cltp->cond_next;
				 cond_i++) {
				condp =
				    &(cltp->conditions[cond_i]);
				/* Force virgin restart */
				condp->current_condition = NULL;
				condp->current_event = NULL;
				nt_next_action(cltp, condp);
			    }
			    cltp->flags &= ~CLT_REMOVED;
			} else {
			    fprintf(stderr,
				    "Remove client [%ld]\n", clt_num);
			    if (notify_remove(cltp) != NOTIFY_OK)
				notify_perror("notify_remove");
			    else
				cltp->flags |= CLT_REMOVED;
			}
		    } else
			fprintf(stderr,
				"Try q(uit) or s(top)\n");
		}
	    }
	}
    }
    /* Print client condition report before exit */
    for (clt_i = 0; clt_i < client_next; clt_i++) {
	if (clients[clt_i].flags & CLT_REMOVED)
	    continue;
	for (cond_i = 0; cond_i < clients[clt_i].cond_next; cond_i++)
	    nt_cleanup_condition(&(clients[clt_i]),
				 &(clients[clt_i].conditions[cond_i]));
    }
    /* Undo timeout */
    if (notify_set_itimer_func(&terminated, NOTIFY_FUNC_NULL,
	  ITIMER_VIRTUAL, (struct itimerval *) 0, (struct itimerval *) 0) !=
	nt_halt) {
	fprintf(stderr, "Unsetting itimer yeilds unexpected result\n");
	exit(1);
    }
    /* See if notifier cleanup out of clients */
    fprintf(stderr, "Making sure notifier cleaned out...\n");
    nt_surpress_ntfy_msg();
    if (notify_start() != NOTIFY_NO_CONDITION) {
	notify_perror("Notifier not cleaned out");
	exit(1);
    }
    nt_reset_ntfy_msg();
    /* Parting message */
    if (nt_errors) {
	fprintf(stderr, "...Done (%ld errors)\n", nt_errors);
	exit(0);
    } else {
	fprintf(stderr, "...Done!\n");
	exit(0);
    }

    exit(0);
}

Notify_value
nt_input_func(client, fd)
    CLIENT         *client;
    int             fd;
{
    CONDITION      *cond;
    char            buf[100];
    int             count;
    int             pipe_id;
    struct pipe    *p;

    for (pipe_id = 0; pipe_id < MAX_PIPES; pipe_id++) {
	p = &(client->pipes[pipe_id]);
	if (p->flags & PIPE_OPEN && p->out == fd)
	    goto Found;
    }
    fprintf(stderr, "Unexpected notification for INPUT\n");
    return (NOTIFY_DONE);
Found:
    /* Check to see if client expected */
    if ((cond = nt_expected(client, "INPUT", pipe_id, 1)) == (CONDITION *) 0)
	return (NOTIFY_UNEXPECTED);
    /* Do the read */
    if ((count = read(fd, buf, 100)) == -1)
	perror(cond->current_condition);
    if (count == 0)
	fprintf(stderr, "Zero count on read\n");
    else if (verbose) {
	write(1, buf, count);
	fprintf(stdout, "\n");
    }
    /* Do next action (may or may not unset current condition) */
    nt_next_action(client, cond);
    return (NOTIFY_DONE);
}

Notify_value
nt_itimer_func(client, which)
    CLIENT         *client;
    int             which;
{
    CONDITION      *cond;
    struct timezone zone;
    extern struct timeval ndet_tv_subt();	/* Borrowing from ntfy impl */
    struct timeval  tod, elasped_tv, error_tv;

    /* Check to see if client expected */
    if (which == ITIMER_REAL) {
	if ((cond = nt_expected(client, "REAL", 0, 0)) ==
	    (CONDITION *) 0)
	    return (NOTIFY_UNEXPECTED);
    }
    if (which == ITIMER_VIRTUAL) {
	if ((cond = nt_expected(client, "VIRTUAL", 0, 0)) ==
	    (CONDITION *) 0)
	    return (NOTIFY_UNEXPECTED);
    }
    /* Measure error of itimer (only good for real itimer) */
    if (gettimeofday(&tod, &zone))
	perror("gettimeofday");
    elasped_tv = ndet_tv_subt(tod, cond->set_tod);
    if ((elasped_tv.tv_sec <= cond->set_tv.tv_sec) &&
	(elasped_tv.tv_usec < cond->set_tv.tv_usec))
	/* elasped is lesser */
	error_tv = ndet_tv_subt(cond->set_tv, elasped_tv);
    else
	/* elasped is greater */
	error_tv = ndet_tv_subt(elasped_tv, cond->set_tv);
    if (verbose && which == ITIMER_REAL)
	fprintf(stdout, "	itimer error sec=%ld, usec=%ld\n",
		error_tv.tv_sec, error_tv.tv_usec);
    /* See if notifier cleared the condition */
    nt_surpress_ntfy_msg();
    if (notify_get_itimer_func(client, which) != NOTIFY_FUNC_NULL)
	fprintf(stderr, "ITIMER not automatically cleared\n");
    else
	/*
	 * Tell nt_unset_condition to clear current_condition without
	 * unsetting it.
	 */
	cond->flags |= CD_NO_UNSET_COND;
    nt_reset_ntfy_msg();
    /* Do next action (may or may not unset current condition) */
    nt_next_action(client, cond);
    return (NOTIFY_DONE);
}

Notify_value
nt_signal_func(client, signal, mode)
    CLIENT         *client;
    int             signal;
    Notify_signal_mode mode;
{
    CONDITION      *cond;

    /* Check to see if client expected */
    if (mode == NOTIFY_ASYNC) {
	if ((cond = nt_expected(client, "ASYNC", signal, 1)) ==
	    (CONDITION *) 0)
	    return (NOTIFY_UNEXPECTED);
    } else if (mode == NOTIFY_SYNC) {
	if ((cond = nt_expected(client, "SYNC", signal, 1)) ==
	    (CONDITION *) 0)
	    return (NOTIFY_UNEXPECTED);
    } else {
	fprintf(stderr, "Unknown signal mode\n");
    }
    /*
     * NOTE: DANGER, doing things when mode == async can screw up ntfy_test
     * internal data structures.
     */
    /* Do next action (may or may not unset current condition) */
    nt_next_action(client, cond);
    return (NOTIFY_DONE);
}

Notify_value
nt_wait3_func(client, pid, status, rusage)
    CLIENT         *client;
    int             pid;
#ifndef SVR4
    union wait     *status;
#else SVR4
    int     *status;
#endif SVR4
    struct rusage  *rusage;
{
    CONDITION      *cond;

    /* Check to see if client expected */
    if ((cond = nt_expected(client, "WAIT3", pid, 1)) == (CONDITION *) 0)
	return (NOTIFY_UNEXPECTED);
    if (WIFEXITED(*status)) {
	/* See if notifier cleared the condition */
	nt_surpress_ntfy_msg();
	if (notify_get_wait3_func(client, pid) != NOTIFY_FUNC_NULL)
	    fprintf(stderr, "Wait not automatically cleared\n");
	else
	    /*
	     * Tell nt_unset_condition to clear current_condition without
	     * unsetting it.
	     */
	    cond->flags |= CD_NO_UNSET_COND;
	nt_reset_ntfy_msg();
	if (verbose)
	    fprintf(stdout, "CHILD EXITED code = %ld\n",
		    WEXITSTATUS(*status));
    } if (WIFSIGNALED(*status)) {
	/* See if notifier cleared the condition */
	nt_surpress_ntfy_msg();
	if (notify_get_wait3_func(client, pid) != NOTIFY_FUNC_NULL)
	    fprintf(stderr, "Wait not automatically cleared\n");
	else
	    /*
	     * Tell nt_unset_condition to clear current_condition without
	     * unsetting it.
	     */
	    cond->flags |= CD_NO_UNSET_COND;
	nt_reset_ntfy_msg();
	if (verbose)
	    fprintf(stdout, "CHILD SIGNALED sig = %ld\n",
		    WTERMSIG(*status));
    } else if (WIFSTOPPED(*status)) {
	char           *cur_cond;

	if (verbose)
	    fprintf(stdout, "CHILD STOPPED sig = %ld\n",
		    WSTOPSIG(*status));
	/* See if done setting conditions */
	if (!set_conditions) {
	    /* Kill the process */
	    if (kill(pid, SIGTERM) == -1)
		perror("KILL TERM");
	} else {
	    /* Continue the process */
	    if (kill(pid, SIGCONT) == -1)
		perror("KILL SIGCONT");
	    /*
	     * Do next action only if it is an event, because can't abandon
	     * child process
	     */
	    cur_cond = cond->current_condition;
	    nt_advance_cur_action(cond);
	    /* Try to send as event */
	    if (nt_try_send_event(client, cond) != 0)
		/* Reset current_condition on failure */
		cond->current_condition = cur_cond;
	    return (NOTIFY_DONE);
	}
    }
    /* Do next action (may or may not unset current condition) */
    nt_next_action(client, cond);
    return (NOTIFY_DONE);
}

Notify_value
nt_output_func(client, fd)
    CLIENT         *client;
    int             fd;
{
    CONDITION      *cond;
    int             pipe_id;
    struct pipe    *p;

    for (pipe_id = 0; pipe_id < MAX_PIPES; pipe_id++) {
	p = &(client->pipes[pipe_id]);
	if (p->flags & PIPE_OPEN && p->in == fd)
	    goto Found;
    }
    fprintf(stderr, "Unexpected notification for OUTPUT\n");
    return (NOTIFY_DONE);
Found:
    /* Check to see if client expected */
    if ((cond = nt_expected(client, "OUTPUT", pipe_id, 1)) ==
	(CONDITION *) 0)
	return (NOTIFY_UNEXPECTED);
    /* Unset condition so that don't keep getting notifications */
    (void) nt_unset_condition(client, cond);
    /* Do next action */
    nt_next_action(client, cond);
    return (NOTIFY_DONE);
}

Notify_value
nt_destroy_func(client, status)
    CLIENT         *client;
    Destroy_status  status;
{
    CONDITION      *cond;

    /* Check to see if client expected */
    if ((cond = nt_expected(client, "DESTROY", 0, 0)) == (CONDITION *) 0)
	return (NOTIFY_UNEXPECTED);
    switch (status) {
      case DESTROY_CHECKING:
	if (verbose)
	    fprintf(stdout, "NOT VETOED\n");
	break;
      case DESTROY_CLEANUP:
      case DESTROY_PROCESS_DEATH:
	/*
	 * Tell nt_unset_condition to clear current_condition without
	 * unsetting it.  Notifier will unset it.
	 */
	cond->flags |= CD_NO_UNSET_COND;
	break;
      default:
	fprintf(stderr, "Unknown status\n");
    }
    /* Do next action (may or may not unset current condition) */
    nt_next_action(client, cond);
    return (NOTIFY_DONE);
}

Notify_value
nt_veto_func(client, status)
    CLIENT         *client;
    Destroy_status  status;
{
    CONDITION      *cond;

    /* Check to see if client expected */
    if ((cond = nt_expected(client, "VETOER", 0, 0)) == (CONDITION *) 0)
	return (NOTIFY_UNEXPECTED);
    switch (status) {
      case DESTROY_CHECKING:
	if (notify_veto_destroy(client) != NOTIFY_OK)
	    notify_perror("VETO");
	else {
	    if (verbose)
		fprintf(stdout, "VETO\n");
	}
	break;
      case DESTROY_CLEANUP:
      case DESTROY_PROCESS_DEATH:
	/*
	 * Tell nt_unset_condition to clear current_condition without
	 * unsetting it.  Notifier will unset it.
	 */
	cond->flags |= CD_NO_UNSET_COND;
	break;
      default:
	fprintf(stderr, "Unknown status\n");
    }
    /* Do next action (may or may not unset current condition) */
    nt_next_action(client, cond);
    return (NOTIFY_DONE);
}

Notify_value
nt_event_func(client, event, type)
    CLIENT         *client;
    Notify_event    event;
    Notify_event_type type;
{
    CONDITION      *cond;

    /* Check to see if client expected */
    if (type == NOTIFY_IMMEDIATE) {
	if ((cond = nt_expected(client, "IMMEDIATE", 0, 0)) ==
	    (CONDITION *) 0)
	    return (NOTIFY_UNEXPECTED);
    }
    if (type == NOTIFY_SAFE) {
	if ((cond = nt_expected(client, "SAFE", 0, 0)) ==
	    (CONDITION *) 0)
	    return (NOTIFY_UNEXPECTED);
    }
    /*
     * NOTE: DANGER, doing things when type == immediate when sent from
     * interrupt level can screw up ntfy_test internal data structures.
     */
    /* Do next action (may or may not unset current condition) */
    nt_next_action(client, cond);
    return (NOTIFY_DONE);
}

nt_cleanup_condition(client, cond)
    CLIENT         *client;
    CONDITION      *cond;
{
    if (cond->current_condition != NULL) {
	/* Print what waiting for */
	fprintf(stderr, "[%ld] registered condition: %s\n",
		(client - &clients[0]), cond->current_condition);
	/* Unset the condition */
	(void) nt_unset_condition(client, cond);
    }
    return;
}

nt_next_action(client, cond)
    CLIENT         *client;
    CONDITION      *cond;
{
    char           *cur_event;
    char           *original_event = NULL;

    if (cond->cycle >= cond->max_cycles)
	return;
    /* Wait for a while */
    switch (generation_speed) {
      case SLOW:
	sleep(5);
	break;
      case MEDIUM:
	sleep(1);
	break;
    }
    /* Continue getting events until run into condition */
    for (; 1;) {
	/* Get original event if not set yet */
	if (original_event == NULL)
	    original_event = cond->current_event;
	/* Get next action */
	nt_advance_cur_action(cond);
	/* Try to send as event */
	switch (nt_try_send_event(client, cond)) {
	  case 0:		/* Valid event */
	    /* Don't repeat original event */
	    if (cond->current_event == original_event)
		goto NextCond;
	    /* Try next one */
	    break;
	  case -1:		/* Unknown token/Unsuccessful parse */
	    goto Cond;
	    break;
	  case -2:		/* Error with the event */
	    /* Don't repeat original event */
	    if (cond->current_event == original_event)
		goto NextCond;
	    /* Try next one */
	    break;
	  default:
	    fprintf(stderr,
		    "Unexpected return value from nt_try_send_event\n");
	}
    }
NextCond:
    nt_advance_cur_action(cond);
Cond:
    /* If not an event, try to send as condition, unset old condition 1st */
    (void) nt_try_set_condition(client, cond);
    return;
}

nt_advance_cur_action(cond)
    CONDITION      *cond;
{
    /* Get next action */
    cond->current_action = XV_INDEX(cond->current_action, NULL);
    /* See if exceded last char or are uninitialized */
    if (cond->current_action >= cond->last_char ||
	(cond->current_condition == NULL && cond->current_event == NULL)) {
	if (cond->current_action >= cond->last_char)
	    cond->cycle++;
	cond->current_action = cond->first_action;
    } else
	/* Move to beginning of line */
	cond->current_action++;
    if (--action_limit < 1)
	(void) nt_halt();
}

int
nt_try_send_event(client, cond)
    CLIENT         *client;
    register CONDITION *cond;
{
    int             n, i, surpress_newline = 0;
    register char   token[100], arg1[200], arg2[200];
    int             cn = (client - &clients[0]);

    n = sscanf(cond->current_action, "%s%s%s", token, arg1, arg2);
    if (n < 2)
	return (-1);
    if (strcmp(token, "WRITE") == 0) {
	int             pipe_id;
	struct pipe    *p;

	/* See what pipe pair using */
	if (sscanf(arg1, "pipe%ld", &pipe_id) != 1 ||
	    pipe_id >= MAX_PIPES) {
	    fprintf(stderr, "Invalid pipe id %s\n", arg1);
	    goto Error;
	}
	p = &(client->pipes[pipe_id]);
	if ((~p->flags) & PIPE_OPEN) {
	    fprintf(stderr, "Tried to write to unopened %s\n",
		    arg1);
	    goto Error;
	}
	if (verbose)
	    fprintf(stdout, "[%ld] %s: ", cn, cond->current_action);
	if (n < 3) {
	    arg2[0] = 'a';
	    arg2[1] = NULL;
	}
	if (write(p->in, arg2, strlen(arg2)) == -1) {
	    perror(cond->current_action);
	    goto Error;
	}
    } else if (strcmp(token, "READ") == 0) {
	int             pipe_id;
	struct pipe    *p;
	char            c;

	/* See what pipe pair using */
	if (sscanf(arg1, "pipe%ld", &pipe_id) != 1 ||
	    pipe_id >= MAX_PIPES) {
	    fprintf(stderr, "Invalid pipe id %s\n", arg1);
	    goto Error;
	}
	p = &(client->pipes[pipe_id]);
	if ((~p->flags) & PIPE_OPEN) {
	    fprintf(stderr, "Tried to read from unopened %s\n",
		    arg1);
	    goto Error;
	}
	if (verbose)
	    fprintf(stdout, "[%ld] %s: ", cn, cond->current_action);
	/* Read from pipe to provoke output condition */
	if (read(p->out, &c, 1) == -1) {
	    perror(cond->current_action);
	    goto Error;
	}
    } else if (strcmp(token, "SIGNAL") == 0) {
	int             sig;

	if (verbose)
	    fprintf(stdout, "[%ld] %s:\n", cn, cond->current_action);
	surpress_newline = 1;
	sig = nt_sig(arg1);
	if (kill(test_pid, sig) == -1) {
	    perror(cond->current_action);
	    goto Error;
	}
    } else if (strcmp(token, "ITIMER_REPORT") == 0) {
	struct itimerval it;
	int             which = (strcmp(arg1, "real")) ? ITIMER_VIRTUAL : ITIMER_REAL;
	Notify_error    rc;

	if (verbose)
	    fprintf(stdout, "[%ld] %s: ", cn, cond->current_action);
	/* Don't report itimer condition missing */
	nt_surpress_ntfy_msg();
	if ((rc = notify_itimer_value(client, which, &it)) != NOTIFY_OK
	    && rc != NOTIFY_NO_CONDITION && rc != NOTIFY_UNKNOWN_CLIENT) {
	    nt_reset_ntfy_msg();
	    notify_perror("notify_itimer_value");
	    goto Error;
	}
	nt_reset_ntfy_msg();
	if (verbose)
	    fprintf(stdout, "sec=%ld usec=%ld",
		    it.it_value.tv_sec, it.it_value.tv_usec);
    } else if (strcmp(token, "SEND_DESTROY") == 0) {
	Destroy_status  status;
	Notify_error    ne;

	if (verbose)
	    fprintf(stdout, "[%ld] %s:\n", cn, cond->current_action);
	surpress_newline = 1;
	if (strcmp(arg1, "cleanup") == 0)
	    status = DESTROY_CLEANUP;
	else if (strcmp(arg1, "death") == 0)
	    status = DESTROY_PROCESS_DEATH;
	else if (strcmp(arg1, "checking") == 0)
	    status = DESTROY_CHECKING;
	else {
	    fprintf(stderr, "%s in unknown destroy status\n", arg1);
	    goto Error;
	}
	if ((ne = notify_post_destroy(client, status)) != NOTIFY_OK) {
	    if (!(status == DESTROY_CHECKING &&
		  ne == NOTIFY_DESTROY_VETOED)) {
		notify_perror("notify_destroy");
		goto Error;
	    }
	}
	if (verbose && ne == NOTIFY_DESTROY_VETOED)
	    fprintf(stdout, "VETOED\n");
    } else if (strcmp(token, "SEND_SAFE") == 0) {
	if (verbose)
	    fprintf(stdout, "[%ld] %s: ", cn, cond->current_action);
	for (i = 0; i < strlen(arg1); i++) {
	    if (notify_post_event(client, arg1[i], NOTIFY_SAFE) !=
		NOTIFY_OK) {
		notify_perror("notify_post_event");
		goto Error;
	    }
	}
    } else if (strcmp(token, "SEND_IMMEDIATE") == 0) {
	if (verbose)
	    fprintf(stdout, "[%ld] %s:\n", cn, cond->current_action);
	surpress_newline = 1;
	for (i = 0; i < strlen(arg1); i++) {
	    if (notify_post_event(client, arg1[i],
				  NOTIFY_IMMEDIATE) != NOTIFY_OK) {
		notify_perror("notify_post_event");
		goto Error;
	    }
	}
    } else if (strcmp(token, "EXERCISER") == 0) {
	if (verbose)
	    fprintf(stdout, "[%ld] %s: ", cn, cond->current_action);
	if (nt_forker(cond) == -1)
	    goto Error;
    } else if (strcmp(token, "NOTHING") == 0) {
	if (verbose)
	    fprintf(stdout, "[%ld] %s: ", cn, cond->current_action);
    } else {
	/* Might be a condition */
	return (-1);
    }
    if (verbose && !surpress_newline)
	fprintf(stdout, "\n");
    cond->current_event = cond->current_action;
    return (0);
Error:
    if (verbose && !surpress_newline)
	fprintf(stdout, "\n");
    return (-2);
}

int
nt_try_set_condition(client, cond)
    CLIENT         *client;
    CONDITION      *cond;
{
    int             n;
    char            token[100], arg1[200], arg2[200], arg3[200], arg4[200];

    /* Don't do any thing if condition match */
    if (set_conditions && cond->current_condition &&
	!(cond->flags & CD_NO_UNSET_COND) &&
	strcmp(cond->current_condition, cond->current_action) == 0) {
	cond->current_condition = cond->current_action;
	return (0);
    }
    /* Unset current condition and see if stopping */
    if (nt_unset_condition(client, cond))
	return (0);
    /* Decompose action */
    n = sscanf(cond->current_action, "%s%s%s%s%s", token, arg1, arg2, arg3,
	       arg4);
    /* Skip blank actions */
    if (n < 1)
	return (0);
    if (strcmp(token, "INPUT") == 0 && n > 1) {
	struct pipe    *p;
	int             fdflags = 0;

	/* See what pipe pair using */
	if (sscanf(arg1, "pipe%ld", &cond->data.pipe) != 1 ||
	    cond->data.pipe >= MAX_PIPES) {
	    fprintf(stderr, "Invalid pipe id %s\n", arg1);
	    return (-1);
	}
	p = &(client->pipes[cond->data.pipe]);
	/* Open pipe to read from */
	if (pipe(&(p->out)) == -1) {
	    fprintf(stderr, "Pipe failed for %s\n", arg1);
	    return (-1);
	}
	p->flags |= PIPE_OPEN;
	if (notify_set_input_func(client, nt_input_func, p->out) ==
	    NOTIFY_FUNC_NULL)
	    goto BadSet;
	if ((n >= 3 && strcmp(arg2, "async") == 0) ||
	    (n >= 4 && strcmp(arg3, "async") == 0)) {
	    fdflags |= FASYNC;
	    p->flags |= PIPE_ASYNC;
	}
	if ((n >= 3 && strcmp(arg2, "ndelay") == 0) ||
	    (n >= 4 && strcmp(arg3, "ndelay") == 0)) {
	    fdflags |= FNDELAY;
	    p->flags |= PIPE_NDELAY;
	}
	if (fdflags) {
	    /* Set non-blocking mode */
	    if (fcntl(p->out, F_SETFL, fdflags) == -1) {
		perror("fcntl");
		return (-1);
	    }
	}
	if (notify_get_input_func(client, p->out) != nt_input_func)
	    goto BadGet;
    } else if (strcmp(token, "OUTPUT") == 0 && n > 1) {
	struct pipe    *p;

	/* See what pipe pair using */
	if (sscanf(arg1, "pipe%ld", &cond->data.pipe) != 1 ||
	    cond->data.pipe >= MAX_PIPES) {
	    fprintf(stderr, "Invalid pipe id %s\n", arg1);
	    return (-1);
	}
	p = &(client->pipes[cond->data.pipe]);
	/* Open pipe to write to */
	if (pipe(&(p->out)) == -1) {
	    fprintf(stderr, "Pipe failed for %s\n", arg1);
	    return (-1);
	}
	p->flags |= PIPE_OPEN;
	/* Set async mode of end that write into to be non-blocking */
	if (fcntl(p->in, F_SETFL, FNDELAY) == -1) {
	    perror("fcntl");
	    return (-1);
	}
	/* Fill pipe with first char of tag (or default) until block */
	if (n < 3)
	    strcpy(&arg2[0], "Buffer fill material...");
	/* Fill with big chunks first */
	if (verbose)
	    fprintf(stdout, "Filling output buffer...\n");
	for (; 1;) {
	    if (write(p->in, arg2, strlen(arg2)) == -1) {
		/* Fill remainder with little chunks */
		if (errno == EMSGSIZE) {
		    for (; 1;)
			if (write(p->in, arg2, 1) ==
			    -1 && errno == EMSGSIZE)
			    goto Filled;
		}
		perror("write");
		return (-1);
	    }
	}
Filled:
	if (verbose)
	    fprintf(stdout, "...done filling\n");
	/* Setup output condition to detect when can output some more */
	if (notify_set_output_func(client, nt_output_func, p->in) ==
	    NOTIFY_FUNC_NULL)
	    goto BadSet;
	if (notify_get_output_func(client, p->in) != nt_output_func)
	    goto BadGet;
    } else if ((strcmp(token, "REAL") == 0 || strcmp(token, "VIRTUAL") == 0)
	       && n > 2) {
	struct itimerval it, it_old;
	struct timezone zone;
	int             which = (strcmp(token, "REAL")) ? ITIMER_VIRTUAL : ITIMER_REAL;

	/* Get secs and usecs */
	cond->set_tv.tv_sec = it.it_value.tv_sec = atoi(arg1);
	cond->set_tv.tv_usec = it.it_value.tv_usec = atoi(arg2);
	timerclear(&it.it_interval);
	/* Set condition */
	if (notify_set_itimer_func(client, nt_itimer_func, which,
				   &it, &it_old) == NOTIFY_FUNC_NULL)
	    goto BadSet;
	/* Check old itimer */
	if (timerisset(&it_old.it_interval) ||
	    timerisset(&it_old.it_value)) {
	    fprintf(stderr, "old interval timer not empty\n");
	    nt_errors++;
	}
	/* Remember when set so can see how close we came */
	if (gettimeofday(&cond->set_tod, &zone))
	    perror("gettimeofday");
	if (notify_get_itimer_func(client, which) != nt_itimer_func)
	    goto BadGet;
    } else if ((strcmp(token, "ASYNC") == 0 || strcmp(token, "SYNC") == 0)
	       && n > 1) {
	Notify_signal_mode mode = (strcmp(token, "ASYNC")) ? NOTIFY_SYNC :
	NOTIFY_ASYNC;

	/* Get sig */
	cond->data.signal = nt_sig(arg1);
	if (notify_set_signal_func(client, nt_signal_func,
			       cond->data.signal, mode) == NOTIFY_FUNC_NULL)
	    goto BadSet;
	if (notify_get_signal_func(client, cond->data.signal, mode) !=
	    nt_signal_func)
	    goto BadGet;
    } else if (strcmp(token, "WAIT3") == 0) {
	if ((cond->data.pid = nt_forker(cond)) == -1)
	    return (-1);
	if (notify_set_wait3_func(client, nt_wait3_func,
				  cond->data.pid) == NOTIFY_FUNC_NULL)
	    goto BadSet;
	if (notify_get_wait3_func(client, cond->data.pid) !=
	    nt_wait3_func)
	    goto BadGet;
    } else if (strcmp(token, "DESTROY") == 0) {
	if (notify_set_destroy_func(client, nt_destroy_func) ==
	    NOTIFY_FUNC_NULL)
	    goto BadSet;
	if (notify_get_destroy_func(client) != nt_destroy_func)
	    goto BadGet;
    } else if (strcmp(token, "VETOER") == 0) {
	if (notify_set_destroy_func(client, nt_veto_func) ==
	    NOTIFY_FUNC_NULL)
	    goto BadSet;
	if (notify_get_destroy_func(client) != nt_veto_func)
	    goto BadGet;
    } else if (strcmp(token, "SAFE") == 0) {
	if (notify_set_event_func(client, nt_event_func, NOTIFY_SAFE) ==
	    NOTIFY_FUNC_NULL)
	    goto BadSet;
	if (notify_get_event_func(client, NOTIFY_SAFE) !=
	    nt_event_func)
	    goto BadGet;
    } else if (strcmp(token, "IMMEDIATE") == 0) {
	if (notify_set_event_func(client, nt_event_func,
				  NOTIFY_IMMEDIATE) == NOTIFY_FUNC_NULL)
	    goto BadSet;
	if (notify_get_event_func(client, NOTIFY_IMMEDIATE) !=
	    nt_event_func)
	    goto BadGet;
    } else if (strcmp(token, "NONE") == 0) {
    } else {
	fprintf(stderr, "Unknown action: %s\n", cond->current_action);
	return (-1);
    }
    cond->current_condition = cond->current_action;
    return (0);
BadGet:
    nt_errors++;
    fprintf(stderr, "Bad get_func for %s\n", cond->current_action);
    notify_perror("notify_error");
    return (-1);
BadSet:
    nt_errors++;
    fprintf(stderr, "Bad set_func for %s\n", cond->current_action);
    notify_perror("notify_error");
    cond->current_condition = NULL;
    return (-1);
}

nt_forker(cond)
    CONDITION      *cond;
{
    char            token[100], arg1[200], arg2[200];
#define	ARGS_MAX        100
    char           *args[ARGS_MAX];
    int             pidchild, n;

    /* Reparse line to get all extra args into arg2 */
    n = sscanf(cond->current_action, "%s%s%[^\n]", token, arg1, arg2);
    (void) nt_constructargs(args, arg1, arg2, ARGS_MAX);
    /* Fork progam */
    pidchild = fork();
    if (pidchild < 0) {
	perror("forking");
	return (-1);
    }
    if (pidchild == 0) {
	/* Exec program */
	execvp(arg1, args, 0);
	perror(arg1);
	exit(1);
	/* NOTREACHED */
    }
    /*
     * Give child process chance to get thru execvp so as to avoid (reduce
     * the likelyhood of) a race with the parent process while debugging the
     * parent.  Supposedly, sleep resets the SIGALRM handler so we would mess
     * up the notifier.
     */
    sleep(2);
    return (pidchild);
}

int
nt_unset_condition(client, cond)
    CLIENT         *client;
    CONDITION      *cond;
{
    int             n;
    char            token[100];
    Notify_func     old_func;

    /* Tell see if should null current_condition without unsetting it. */
    if (cond->flags & CD_NO_UNSET_COND) {
	cond->flags &= ~CD_NO_UNSET_COND;
	cond->current_condition = NULL;
    }
    if (cond->current_condition == NULL)
	goto Done;
    n = sscanf(cond->current_condition, "%s", token);
    if (n < 1)
	goto BadCond;
    if (strcmp(token, "INPUT") == 0) {
	struct pipe    *p = &(client->pipes[cond->data.pipe]);

	old_func = notify_set_input_func(client, NOTIFY_FUNC_NULL,
					 p->out);
	close(p->in);
	close(p->out);
	p->flags = 0;
    } else if (strcmp(token, "OUTPUT") == 0) {
	struct pipe    *p = &(client->pipes[cond->data.pipe]);

	old_func = notify_set_output_func(client, NOTIFY_FUNC_NULL,
					  p->in);
	close(p->in);
	close(p->out);
	p->flags = 0;
    } else if (strcmp(token, "REAL") == 0) {
	old_func = notify_set_itimer_func(client, NOTIFY_FUNC_NULL,
		    ITIMER_REAL, &NOTIFY_NO_ITIMER, (struct itimerval *) 0);
    } else if (strcmp(token, "VIRTUAL") == 0) {
	old_func = notify_set_itimer_func(client, NOTIFY_FUNC_NULL,
		 ITIMER_VIRTUAL, &NOTIFY_NO_ITIMER, (struct itimerval *) 0);
    } else if (strcmp(token, "ASYNC") == 0) {
	old_func = notify_set_signal_func(client, NOTIFY_FUNC_NULL,
					  cond->data.signal, NOTIFY_ASYNC);
    } else if (strcmp(token, "SYNC") == 0) {
	old_func = notify_set_signal_func(client, NOTIFY_FUNC_NULL,
					  cond->data.signal, NOTIFY_SYNC);
    } else if (strcmp(token, "DESTROY") == 0) {
	old_func = notify_set_destroy_func(client, NOTIFY_FUNC_NULL);
    } else if (strcmp(token, "VETOER") == 0) {
	old_func = notify_set_destroy_func(client, NOTIFY_FUNC_NULL);
    } else if (strcmp(token, "SAFE") == 0) {
	old_func = notify_set_event_func(client, NOTIFY_FUNC_NULL,
					 NOTIFY_SAFE);
    } else if (strcmp(token, "IMMEDIATE") == 0) {
	old_func = notify_set_event_func(client, NOTIFY_FUNC_NULL,
					 NOTIFY_IMMEDIATE);
    } else if (strcmp(token, "WAIT3") == 0) {
	old_func = notify_set_wait3_func(client, NOTIFY_FUNC_NULL,
					 cond->data.pid);
	/* Kill child process, OK if process not there */
	if (kill(cond->data.pid, SIGTERM) == -1 && errno != ESRCH)
	    perror("kill");
    } else if (strcmp(token, "NONE") == 0) {
    } else
	goto BadCond;
    if (old_func == NOTIFY_FUNC_NULL) {
	fprintf(stderr, "Unset_func error %ld on %s\n", notify_errno,
		cond->current_condition);
	return (!set_conditions);
    }
    cond->current_condition = NULL;
Done:
    return (!set_conditions);
BadCond:
    fprintf(stderr, "Unknown action in unset for %s\n",
	    cond->current_condition);
    cond->current_condition = NULL;
    return (!set_conditions);
}

CONDITION      *
nt_expected(client, token, data, use)
    CLIENT         *client;
    char           *token;
    int             data, use;
{
    char            name[100];
    int             i, n;
    CONDITION      *cond;

    /* Search client for condition of token that has data */
    for (i = 0; i < client->cond_next; i++) {
	cond = &(client->conditions[i]);
	/* Current condition may be null */
	if (cond->current_condition == NULL)
	    continue;
	n = sscanf(cond->current_condition, "%s", name);
	if (n == 1 && strcmp(token, name) == 0) {
	    if (use) {
		if (cond->data.i != data) {
		    continue;
		}
	    }
	} else
	    continue;
	/* Print normal occurence in history log if verbose */
	if (verbose)
	    fprintf(stdout, "[%ld] Notification Received: %s\n",
		    (client - &clients[0]), cond->current_condition);
	return (cond);
    }
    /* Print unexpected notification in error log */
    fprintf(stderr, "NOTIFICATION UNEXPECTED: %s\n", token);
    nt_errors++;
    return ((CONDITION *) 0);
}

nt_parse(argc, argv)
    char          **argv;
{
    int             bump;
    char            tmp[100];

    while (argc > 0 && argv[0][0] == '-' && argv[0][1]) {
	bump = 0;
	switch (argv[0][1]) {
	  case 't':		/* Set time limit */
	    if (argc < 2)
		goto ArgNum;
	    if (sscanf(argv[1], "%hd", &halt_itimer.it_value.tv_sec)
		!= 1)
		goto ArgFormat;
	    /*
	     * Set interval so that when unset itimer condition it will still
	     * be there, even though it expired once.
	     */
	    halt_itimer.it_interval.tv_sec =
		halt_itimer.it_value.tv_sec;
	    bump = 2;
	    break;
	  case 'a':		/* Set action limit */
	    if (argc < 2)
		goto ArgNum;
	    if (sscanf(argv[1], "%hd", &action_limit) != 1)
		goto ArgFormat;
	    bump = 2;
	    break;
	  case 'v':		/* Set verbose mode */
	    verbose = 1;
	    bump = 1;
	    break;
	  case 's':		/* Set Unix event generation speed */
	    if (argc < 2)
		goto ArgNum;
	    if (sscanf(argv[1], "%hs", tmp) != 1)
		goto ArgFormat;
	    if (strcmp(tmp, "fast") == 0)
		generation_speed = FAST;
	    else if (strcmp(tmp, "slow") == 0)
		generation_speed = SLOW;
	    else if (strcmp(tmp, "medium") == 0)
		generation_speed = MEDIUM;
	    else
		goto ArgFormat;
	    bump = 2;
	    break;
	  case 'i':		/* Client on stack method */
	    if (argc < 2)
		goto ArgNum;
	    if (sscanf(argv[1], "%hs", tmp) != 1)
		goto ArgFormat;
	    if (strcmp(tmp, "none") == 0)
		input_method = NONE;
	    else if (strcmp(tmp, "read") == 0)
		input_method = READ;
	    else if (strcmp(tmp, "select") == 0)
		input_method = SELECT;
	    else
		goto ArgFormat;
	    bump = 2;
	    break;
	  case 'c':		/* Client specification */
	    if (argc < 2)
		goto ArgNum;
	    if (sscanf(argv[1], "%hs", tmp) != 1)
		goto ArgFormat;
	    if (nt_get_client(tmp) == -1)
		goto Help;
	    bump = 2;
	    break;
	  case 'x':		/* Acting as exerciser process */
	    if (argc < 2)
		goto ArgNum;
	    if (sscanf(argv[1], "%hs", tmp) != 1)
		goto ArgFormat;
	    if (strcmp(tmp, "wait3") == 0)
		wait3er = 1;
	    else if (nt_get_client(tmp) == -1)
		goto Help;
	    else
		exerciser = 1;
	    bump = 2;
	    break;
	  default:
	    fprintf(stderr, "Unknown flag %s\n", argv[0]);
	    goto Help;
	}
	argc -= bump;
	argv += bump;
    }
    return;
ArgNum:
    fprintf(stderr, "Wrong number of args at %s\n", argv[0]);
    goto Help;
ArgFormat:
    fprintf(stderr, "Wrong type of arg at %s\n", argv[0]);
Help:
    fprintf(stderr, "Args:\n\
-t\tnsecs\t\t\tTime limit\n\
-n\tcount\t\t\tNotification limit\n\
-v\t-\t\t\tVerbose\n\
-s\tfast/medium/slow\tUnix event generation speed\n\
-i\tnone/read/select\tClient-on-stack input method\n\
-c\trandom/filename/fixed\tClient specification\n\
-x\twait3/filename\t\tExerciser cmd line\n");
    exit(1);
}

int
nt_get_client(type_str)
    char           *type_str;
{
    CLIENT         *clt;
    CONDITION      *cond = CONDITION_NULL;
    int             c, i, size, n;
    char            token[200], args[200], *s, *s_last;
    FILE           *file;

    if (client_next >= MAX_CLIENTS) {
	fprintf(stderr, "Max number of clients exceded\n");
	return (-1);
    }
    clt = &clients[client_next];
    if (strcmp(type_str, "random") == 0) {
	fprintf(stderr, "Random client not supported\n");
	return (-1);
    } else if (strcmp(type_str, "fixed") == 0) {
	/* Copy fixed_actions to client */
	size = strlen(fixed_actions);
	clt->data = xv_calloc(1, size + 1 /* For NULL */ );
	strcpy(clt->data, fixed_actions);
    } else {
	/* Open action file */
	if ((file = fopen(type_str, "r+")) == (FILE *) 0) {
	    perror("fopen");
	    perror(type_str);
	    return (-1);
	}
	/* Determine its length */
	if (ioctl(fileno(file), FIONREAD, &size)) {
	    perror("FIONREAD");
	    perror(type_str);
	    return (-1);
	}
	if (size < 1) {
	    fprintf(stderr, "Empty action file: %s\n", type_str);
	    return (-1);
	}
	/* Copy file characters to client */
	clt->data = xv_calloc(1, size + 1 /* For NULL */ );
	for (i = 0; i < size; ++i)
	    clt->data[i] = getc(file);
	clt->data[size] = NULL;
    }
    /* Turn raw data into conditions */
    s_last = s = clt->data;
    s_last += size;
    while (s < s_last) {
	/* Ignore leading newlines */
	if (*s == '\n') {
	    s++;
	    continue;
	}
	/* Don't overflow buffers */
	if ((XV_INDEX(s, '\n') - s) >= sizeof(token)) {
	    fprintf(stderr, "Cond line too long: %s\n", s);
	    return (-1);
	}
	/* Pick off next line */
	switch (n = sscanf(s, "%s%[^\n]\n", token, args)) {
	  case EOF:		/* End of input string */
	    goto Done;
	  case 0:		/* Blank line */
	    s++;
	    break;
	  default:		/* Something on line */
	    if (strcmp(token, "CONDITION") == 0) {
		/* Start new condition */
		if (clt->cond_next >= MAX_CONDITIONS) {
		    fprintf(stderr,
			    "Too many conditions in %s\n", type_str);
		}
		cond = &clt->conditions[clt->cond_next++];
		XV_BZERO(cond, sizeof(cond));
		/* See if specified number of cycles */
		if (n == 1)
		    cond->max_cycles = INFINITE;
		else
		    cond->max_cycles = atoi(args);
		/* Skip over CONDITION line to first action */
		s = XV_INDEX(s, '\n');
		if (s == NULL)
		    goto Done;
		s++;
		cond->first_action = cond->current_action = s;
		cond->last_char = s;
	    } else {
		/* Ignore junk at beginning of file */
		if (cond == CONDITION_NULL)
		    continue;
		/* Error check token??? */
		/* Set end of action line to NULL */
		s = XV_INDEX(s, '\n');
		if (s == NULL)
		    goto Done;
		*s = NULL;
		cond->last_char = s;
		/* Point to beginning of next line */
		s++;
	    }
	}
    }
Done:
    client_next++;
    return (0);
}

Notify_value
nt_halt()
{
    kill(0, SIGURG);
    return (NOTIFY_DONE);
}

nt_reset_ntfy_msg()
{
    extern          ntfy_errno_print;

    ntfy_errno_print = nt_ntfy_errno_print;
}

nt_surpress_ntfy_msg()
{
    extern          ntfy_errno_print;

    nt_ntfy_errno_print = ntfy_errno_print;
    ntfy_errno_print = 0;
}

sigurg()
{
    terminated = 1;
}

sigterm()
{
    terminated = 1;
}

be_exerciser()
{
    CLIENT         *client = &clients[0];
    CONDITION      *cond;
    int             cond_i = 0;
    int             err;

    /* Check assumptions */
    if (client_next != 1 || client->cond_next == 0) {
	fprintf(stderr, "EXERCISER no client or condition %ld %ld\n",
		client_next, client->cond_next);
	exit(1);
    }
    signal(SIGTERM, sigterm);
    set_conditions = 0;
    do {
	/* Wait for a while */
	switch (generation_speed) {
	  case SLOW:
	    sleep(5);
	    break;
	  case MEDIUM:
	    sleep(1);
	    break;
	  default:{
		int             bits = 0;
		struct timeval  tv;

		tv.tv_sec = 0;
		tv.tv_usec = 100000;
		/* Wait 10th of a second */
		if (notify_select(0, &bits, &bits, &bits, &tv)) {
		    perror("EXERCISER select error");
		    exit(1);
		}
	    }
	}
	/* Look at next condition */
	if (cond_i >= client->cond_next)
	    cond_i = 0;
	cond = &(client->conditions[cond_i]);
	/* Get next action */
	nt_advance_cur_action(cond);
	/* Try to send as event */
	if (err = nt_try_send_event(client, cond)) {
	    if (err == -2)
		fprintf(stderr, "EXERCISER nonevent action\n");
	    fprintf(stderr, "EXERCISER terminating\n");
	    exit(1);
	}
	cond_i++;
    } while (!terminated);
    exit(0);
}

be_wait3er()
{
    signal(SIGTERM, sigterm);
    set_conditions = 0;
    do {
	/* Wait for a while */
	switch (generation_speed) {
	  case SLOW:
	    sleep(15);
	    break;
	  case MEDIUM:
	    sleep(10);
	    break;
	  default:
	    sleep(5);
	}
	if (terminated)
	    break;
	/* Send SIGSTOP to self */
	if (verbose)
	    fprintf(stdout, "CHILD PROCESS stop self\n");
	if (kill(getpid(), SIGSTOP) == -1) {
	    perror("CHILD PROCESS kill");
	    exit(1);
	}
    } while (!terminated);
    if (verbose)
	fprintf(stdout, "CHILD PROCESS terminated\n");
    exit(0);
}

/* Just schedule the last client and let the default scheduler do the rest */
Notify_value
nt_scheduler(n, nclients)
    int             n;
    register Notify_client *nclients;
{
    register Notify_client nclient;
    register int    i = n - 1;

    if (n == 0)
	return (NOTIFY_DONE);
    nclient = *(nclients + i);
    /* Notify client if haven't been done yet */
    if (nclient != NOTIFY_CLIENT_NULL) {
	/* notify_client detects errors from nclients */
	if (notify_client(nclient) != NOTIFY_OK)
	    return (NOTIFY_UNEXPECTED);
	/*
	 * Null out client entry prevents it from being notified again.
	 */
	*(nclients + i) = NOTIFY_CLIENT_NULL;
    }
    return (nt_cached_sched(n, nclients));
}

Notify_value
nt_prioritizer(nclient, nfd, ibits_ptr, obits_ptr, ebits_ptr,
	       nsig, sigbits_ptr, auto_sigbits_ptr, event_count_ptr, events)
    register Notify_client nclient;
    int            *ibits_ptr, *obits_ptr, *ebits_ptr, nsig, *sigbits_ptr, *event_count_ptr;
    register int   *auto_sigbits_ptr, nfd;
    Notify_event   *events;
{
    /* Send real itimers */
    if (*auto_sigbits_ptr & SIG_BIT(SIGALRM)) {
	(void) notify_itimer(nclient, ITIMER_REAL);
	*auto_sigbits_ptr &= ~SIG_BIT(SIGALRM);
    }
    /* Send SIGWINCH */
    if (*sigbits_ptr & SIG_BIT(SIGWINCH)) {
	(void) notify_signal(nclient, SIGWINCH);
	*sigbits_ptr &= ~SIG_BIT(SIGWINCH);
    }
    /* Send only last client event */
    if (*event_count_ptr > 0) {
	(void) notify_event(nclient, *(events + (*event_count_ptr) - 1));
	*event_count_ptr--;
    }
    /* Send fd 1 input */
    if (*ibits_ptr & 1) {
	(void) notify_input(nclient, 1);
	*ibits_ptr &= ~(1);
    }
    return (((CLIENT *) nclient)->prioritizer(nclient, nfd, ibits_ptr,
		  obits_ptr, ebits_ptr, nsig, sigbits_ptr, auto_sigbits_ptr,
					      event_count_ptr, events));
}
