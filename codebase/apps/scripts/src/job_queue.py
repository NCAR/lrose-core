#!/usr/bin/env python2
import sys,os
import getopt
import time
#import pdb
#import profile as cProfile
import re
import datetime
import math
import shlex
import subprocess
import fcntl

def usage():
    # usage help and exit
    print '''Usage: %s [options]
Options:

      -h, --help                 This usage
      -j, --jobs=NUM             The number of processed to run simultaneously
                                 default is 4
      -f, --file=FILE            File that contains a list of commands to execute
                                 If not given, this takes from STDIN
      -F, --forever              Keep running until killed.  This is only used when
                                 running on files.  This will make a server that sits
                                 there waiting for commands to execute.  BECAREFUL OF
                                 SECURITY if using on a FILE.

This function allows easy multiprocessing of a series of commands either from a
file or from STDIN.  Output is reported using STDOUT.  The order at which jobs
are completed are not necessarily the same as the order that the commands are
given to the program.



    '''%sys.argv[0]

class Params(dict):
    def __init__(self,onechar_opts= "hj:f:F", word_opts=\
                 ["help","jobs=","--file=","forever"]):
        self._onechar_opts = onechar_opts
        self._word_opts = word_opts
        self._param_opts = []
        self._param_args = []
        self._file_opts = []
        self._file_args = []

    def read_params(self,argv):
        # Parse options
        try:
            self._param_opts, self._param_args = getopt.gnu_getopt(argv, \
                                                                     self._onechar_opts, \
                                                                     self._word_opts)
        except getopt.GetoptError:
            # print help information and exit:
            usage()
            sys.exit(2)


    def parse_param_opts(self):
        # Now really parse the options
        for o, a in (self._file_opts + self._param_opts):
            if o in ("-h", "--help"):
                usage()
                sys.exit()
            if o in ("-j", "--jobs"):
                self['jobs'] = int(a)
            if o in ("-f", "--file"):
                self['file'] = a.strip()
            if o in ("-F", "--forever"):
                self['forever'] = True
                
    def define_defaults(self):
        self['jobs'] = 4
        self['file'] = None
        self['forever'] = False

    def sanitize(self):
        if self['jobs']<1 or self['jobs']>30:
            raise Exception("The number of jobs must be between 1 and 30")
        if self['file'] is not None and (not os.path.lexists(self['file']) or os.path.isdir(self['file'])):
            raise Exception("The file '%s' does not exist" % self['file'])
        if self['file'] is None and self['forever']:
            self['forever'] = False
        

    @staticmethod
    def full_process(argv):
        p = Params()
        p.define_defaults()
        p.read_params(argv)
        p.parse_param_opts()
        p.sanitize()
        return p


class Job(object):
    def __init__(self,args=None,extras=None):
        self._max_time=float('inf')
        self._args = args
        self._job = None
        self._run = False
        self._suppress = False
        self.extras = None
        self.output = ''


    def set_stuff(self,args=None,extras=None):
        if args is not None:
            self._args = args
        if extras is not None:
            self.extras = extras

    #@property
    #def not_ran(self):
    #    return not self._run and self.args is not None
        
    @property
    def is_open(self):
        return self._args is None

    @property
    def running(self):
        if self._run and self._job.poll() is None:
            if not self._suppress and len(self.output)<10000000:
                try:
                    self.output += self._job.stdout.read()
                except:
                    pass
            else:
                if not self._suppress:
                    self.output += "\nThe rest of the output is suppressed"
                    self._suppress = True
                try:
                    self._job.stdout.read()
                except:
                    pass
            return True
        else:
            return False

    @property
    def done(self):
        return self._run and not self.running

    def run(self):
        try:
            self._job = subprocess.Popen(self._args,stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            fd = self._job.stdout.fileno()
            fl = fcntl.fcntl(fd, fcntl.F_GETFL)
            fcntl.fcntl(fd, fcntl.F_SETFL, fl | os.O_NONBLOCK)
        except OSError:
            self.__init__()
            raise

        self._run = True
        
    @property
    def pid(self):
        if self.running:
            return self._job.pid
        return None

    @property
    def returncode(self):
        if self.running or self.done:
            # little sneaky.  The above checks runs poll which updates the returncode
            return self._job.returncode
        return None

    def kill(self,signal=9):
        if self.running:
            os.kill(self.pid,signal)

    def get_output(self):
        self.output += self._job.stdout.read()
        return self.output

class Processes(object):
    def __init__(self,num_jobs):
        self._num_jobs = num_jobs
        self._jobs = [Job() for ind in range(num_jobs)]
        self._sleep_time = .05

    def __len__(self):
        return len(self._jobs)

    def job_done(self,ind):
        return self._jobs[ind].done

    def wait_for_available_job(self):
        while not self.any_available_jobs:
            time.sleep(self._sleep_time)

    @property
    def any_available_jobs(self):
        for job in self._jobs:
            if job.is_open or job.done:
                return True
        return False

    @property
    def any_open_jobs(self):
        for job in self._jobs:
            if job.is_open:
                return True
        return False

    @property
    def open_jobs(self):
        jobs = []
        for ind,job in enumerate(self._jobs):
            if job.is_open:
                jobs.append(ind)
        return jobs

    @property
    def any_done_jobs(self):
        for job in self._jobs:
            if job.done:
                return True
        return False

    @property
    def all_open_jobs(self):
        for job in self._jobs:
            if not job.is_open:
                return False
        return True

    @property
    def done_jobs(self):
        jobs = []
        for ind,job in enumerate(self._jobs):
            if job.done:
                jobs.append(ind)
        return jobs

    def get_job(self,ind):
        return self._jobs[ind]

    def add_job(self,*kargs,**kwargs):
        if not self.any_open_jobs:
            raise Exception("No open jobs.  Cannot Add.")
        ind = self.open_jobs[0]
        self._jobs[ind].set_stuff(*kargs,**kwargs)
        self._jobs[ind].run()

    def reset_job(self,ind):
        if self._jobs[ind].is_open:
            return 
        if not self._jobs[ind].done:
            raise Exception("Will not reset a job that was not done.")
        self._jobs[ind] = Job()
        
    
def main(argv):
    # Main program.  Takes string containing arguments a la unix command line
    p = Params.full_process(argv)
    if p['file'] is None:
        instream = sys.stdin
    else:
        instream = open(p['file'],'r',1)
    outstream = sys.stdout

    #sys.stdin = os.fdopen(sys.stdin.fileno(),'r',0)
    jobs = Processes(p['jobs'])

    exit_at_end = False

    while not exit_at_end:
        line = instream.readline()
        # if not EOF process
        if line:
            line = line.strip()
            if not(len(line)==0 or line[0]=='#'):
                arg = shlex.split(line)
                try:
                    jobs.add_job(arg,{'command':line})
                except OSError,e:
                    outstream.write('--Could not start job: %s\n' % line)
                    outstream.write('--Error: %s\n' % e)
                    outstream.write('\n--End of job: %s\n' % line)
                    outstream.flush()                
                
        else:
            if p['forever']:
                time.sleep(.05)
            elif jobs.all_open_jobs:
                exit_at_end = True

        jobs.wait_for_available_job()
        for ind in jobs.done_jobs:
            job = jobs.get_job(ind)
            outstream.write('--Executing Job: %s\n' % job.extras['command'])
            outstream.write('--Exit code: %i\n' % job.returncode)
            #data = (stuff for stuff in iter(lambda :job.get_output_pipe().read(1024),""))
            #data = iter(lambda :job.get_output_pipe().read(1024),"")
            #for d in data:
            #    outstream.write(d)
            outstream.write(job.get_output())
            outstream.write('\n--End of job: %s\n' % job.extras['command'])
            outstream.flush()
            jobs.reset_job(ind)


if __name__ == "__main__":
    # if running from UNIX execute main

    #To profile, uncomment the following

    #cProfile.run('main(sys.argv[1:])','prof.dmp')

    #or
    
    #import lsprofcalltree
    #p = cProfile.Profile()
    #p.run('main(sys.argv[1:])')
    #k = lsprofcalltree.KCacheGrind(p)
    #data = open('prof%05.0f.kgrind' % os.getpid(),'wt')
    #k.output(data)
    #data.close()
    
    main(sys.argv[1:])
