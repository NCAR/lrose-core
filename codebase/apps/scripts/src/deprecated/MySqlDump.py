#!/usr/bin/env python2
import sys,os
import getopt
#import pdb
#import profile as cProfile
import re
import datetime
import math

import _mysql

import MySQLdb,  MySQLdb.converters
import MySQLdb.constants.FIELD_TYPE as myfld

def usage():
    # usage help and exit
    print '''Usage: %s [options]
Options:

      -h, --help                 This usage
      -q, --query=QUERY          string containing query
      -o, --output=FILE          output file name - defaults to stdout
      -H, --host=HOST            name of host
      -u, --user=USER            username for connection
      -p, --passwd=PASSWD        password <better idea to use config file for this>
      -d, --db=DB                name of database
      -P, --port=PORT            port number
      -s, --socket=FILENAME       filename of socket - typically only needed if host is localhost
      -c, --config=FILENAME      filename of config file

      --rows-per-file=NUM        determines the number of rows to be saved into
                                 a single file.  default is 0
                                 implying all data goes into 1 file.  If = n>0, then files will
                                 have length n.

This program reads query from mysql and outputs in CSV.
If queries are large, use the --rows-per-file option to create multiple
files with the query results.

Config files:

A valid config file is structured just as the standard argument list at the
command line is structured, with the exceptions that
  1) linefeeds seperate commands
  2) do NOT put spaces between the optionname, = and option value
  3) do NOT use quotes unless you want the string to contain the quotes
For example:
-qselect * from foo
--user=me
--passwd=flubber

is the same as the following given at the command line:
-q\'select * from foo\' --user=\'me\' --passwd=\'flubber\'

    '''%sys.argv[0]

class Params(dict):
    def __init__(self,onechar_opts= "hq:o:H:u:p:d:P:s:c:", word_opts=\
                 ["help","query=","output=","host=",
                 "user=","passwd=","db=","port=","socket=","config=",
                 "rows-per-file="]):
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
            raise


    def read_config(self,onechar_opt='-c',word_opt='--config'):
        config = [a for o,a in self._param_opts if o in (onechar_opt,word_opt)]
        # read the config file
        if len(config)>=1:
            if len(config)>1:
                sys.stderr.write('more than one config found in arguments. Only using last one.\n')
                
            conf = open(config[-1])
            lines = conf.readlines()
            conf.close()
            lines = [line[:-1].strip() for line in lines]
            self._file_opts, self._file_args = getopt.gnu_getopt(lines,self._onechar_opts, \
                                                                   self._word_opts)

    def parse_param_opts(self):
        # Now really parse the options
        for o, a in (self._file_opts + self._param_opts):
            if o in ("-h", "--help"):
                usage()
                sys.exit()
            elif o in ("-q", "--query"):
                self['query'] = a

            elif o in ("--rows-per-file",):
                self['rows_per_file'] = int(a)

            elif o in ("-o", "--output"):
                self['output'] = a

            elif o in ("-H", "--host"):
                self['con_opts']['host'] = a

            elif o in ("-u", "--user"):
                self['con_opts']['user'] = a

            elif o in ("-p", "--passwd"):
                self['con_opts']['passwd'] = a

            elif o in ("-d", "--db"):
                self['con_opts']['db'] = a

            elif o in ("-P", "--port"):
                self['con_opts']['port'] = int(a)

            elif o in ("-s", "--socket"):
                self['con_opts']['unix_socket'] = a

    def define_defaults(self):
        self['query'] = ""
        self['output'] = ""
        self['rows_per_file'] = 0
        self['con_opts'] = {}

    def sanitize(self):
        # sanatize the query to guard against non selects
        if not re.search('^\s*select[^;]*$',self['query'],re.I):
            usage()
            print "ERROR: Queries must be select statements without ';'s"
            sys.exit(3)

        if self['rows_per_file'] and self['output']=='':
            usage()
            print "ERROR: Writing to stdout and breaking into blocks is not allowed"
            sys.exit(3)

def None2Fill(x,y):
    if x==None:
        return y
    return x    

class BadFilename(Exception):
    "Bad filename exception"

class OutputBase(object):
    """Base class for output type objects - for example netcdf.
    This class is not to be used as a standalone but it does provide some
    infrastructure support like set up filenames, increments file number index,
    sanitize the filename, etc."""
    
    def __init__(self,filename, multiple=False):
        self._base_filename = filename
        self._filename = None
        self._file = None
        self._index = 0
        self._multiple = multiple

        if self._multiple:
            mtch = re.findall(r'%[^%]', self._base_filename)
            if len(mtch) != 1:
                raise BadFilename, "There should be exactly 1 % type tag in the filename. e.g." +\
                      ' %03.0f.  Allowed are any of the numeric tags: diuoxXeEfFgG'
            mtch = re.findall(r'%\d*[.]*\d*[diuoxXeEfFgG]', self._base_filename)
            if len(mtch) != 1:
                raise BadFilename, "There should be exactly 1 % type tag in the filename. e.g." +\
                      " 'foo_%03u.nc'.  Allowed are any of the numeric tags: diuoxXeEfFgG"

    def create_file(self,num=None):
        # if open, close the file, if multiple, create the new name # if file exists nuke it
        if self._file:
            self.close_file()

        if self._multiple:
            self._filename = self._base_filename % self._index 
        else:
            self._filename = self._base_filename

        if os.path.exists(self._filename):
            os.remove(self._filename)


    def close_file(self):
        if self._multiple:
            self._index += 1
        self._file = None

    def write_data(self,rows,offset=None):
        pass

    def description2dict(self,description):
        out = []
        found_bad_name = False
        #for fld in dir(myfld):
        #    if type(getattr(myfld,fld))==int:
        #        print '%s: %i' % (fld,getattr(myfld,fld))
            
        for fld in description:
            tmp = {}
            tmp['name'], tmp['type'], tmp['display_size'], tmp['internal_size'], tmp['precision'], \
                         tmp['scale'], tmp['null_ok'] = fld
        #    print '%s: %i %i %i %i' % (tmp['name'],tmp['type'],tmp['display_size'], tmp['internal_size'], tmp['precision'])
            if not re.search('^[\w\-\_]+$',tmp['name']):
                print "ERROR: you have a field named '%s'. " % tmp['name'],
                print "names must be alphanumeric, '-', '_'"
                print "Solved by 'select expr as new_name ...'"
                found_bad_name = True
            out.append(tmp)

        if found_bad_name:
            sys.exit(5)

        return out


class CSVOutput(OutputBase):
    def __init__(self,filename, multiple, description):
        OutputBase.__init__(self,filename, multiple)
        self._description = description #self.description2dict(description)

    def create_file(self,num):
        OutputBase.create_file(self)
      
        if os.path.exists(self._filename):
            os.remove(self._filename)

        if self._filename!='':
            self._file = open(self._filename,'wb')
        else:
            self._file = sys.stdout
            
        #self._csv.writerow([fld['name'] for fld in self._description])

    def write_data(self,rows,offset=None):
            self._file.write('\t'.join([None2Fill(x,r'\N') for x in rows]) +'\n')

    def close_file(self):
        if self._filename!='':
            self._file.close()
        OutputBase.close_file(self)
        
def main(argv):
    # Main program.  Takes string containing arguments a la unix command line
    p = Params()
    p.define_defaults()
    p.read_params(argv)
    p.read_config()
    p.parse_param_opts()

    p.sanitize()

    # connect
    my = _mysql.connect(**p['con_opts'])

    # grab the query
    # create temporary table to put the query into on server side
    my.query(p['query'])
    
    r=my.use_result()

    if p['rows_per_file'] == 0:
        multiple = False
    else:
        multiple = True
    
            
    # loop grabbing the blocks of data 1 at a time
    fini = False
    loop_num = 0
    while not fini:
        data = r.fetch_row()
        if loop_num == 0:
            # first round through create the output object
            outfile = CSVOutput(p['output'],multiple, None)

        if len(data)>0:
            # if data available, go ahead and write it
            
            # figure out what loop number relative to the file we are working on
            if p['rows_per_file']:
                loop_num_file = loop_num % p['rows_per_file']
            else:
                loop_num_file = loop_num
            
            # first see if we have to create a new file
            if loop_num_file == 0:
                # create a file that can handle the smaller of file_length and number_left
                outfile.create_file(0)

            # now write it
            outfile.write_data(data[0])

            # see if we should break out of the loop
        else:
            fini = True

        # increment counter
        loop_num += 1


    # we are done so close the file
    outfile.close_file()
        


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
