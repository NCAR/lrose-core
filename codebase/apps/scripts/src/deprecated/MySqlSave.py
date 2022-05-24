#!/usr/bin/env python2
import sys,os
import getopt
#import pdb
#import profile as cProfile
import re
import datetime
import math

import MySQLdb,  MySQLdb.converters
import MySQLdb.constants.FIELD_TYPE as myfld

try:
    import numpy as N
except:
    print '''
The problem below may be that numpy requires the libatalas3gf-base (in Debian)
library.  Please make sure that package is installed.
(dpkg-query -l "libatlas3*" should return that package with "ii" in the first column
If not, contact your system administrator.)
'''
    raise

def usage():
    # usage help and exit
    print '''Usage: %s [options]
Options:

      -h, --help                 This usage
      -q, --query=QUERY          string containing query
      -o, --output=FILE          output file name - defaults to stdout
      -N, --netcdf               output to NetCDF - This is the default.  You must use -o option
      -C, --CSV                  output to csv file
      -H, --host=HOST            name of host
      -u, --user=USER            username for connection
      -p, --passwd=PASSWD        password <better idea to use config file for this>
      -d, --db=DB                name of database
      -P, --port=PORT            port number
      -s, --socket=FILENAME       filename of socket - typically only needed if host is localhost
      -c, --config=FILENAME      filename of config file

      -b, --block                Process the query in blocks.  Default is false.
                                 Requires create_tmp_table permission in MySQL
      -B, --block-size=NUM       if -b given, determines the size of the blocks.
                                 default 10000
      --blocks-per-file=NUM      if -b given, determines the number of blocks to be saved into
                                 a single file.  This allows the user to not only break the query
                                 into pieces, but to also break the files in pieces.  default is 0
                                 implying all data goes into 1 file.  If = n>0, then files will
                                 have length n*block_size.
      -S, --sort-fields="fld1[,...]"
                                 if -b is turned on, specify the sort fields.
                                 The valid field names are those from inside the
                                 temporary table!  So use those \'as\'s in the
                                 select statement.

This program reads query from mysql and outputs in NetCDF or CSV.  Eventually more formats
are expected to be added.  If queries are large, use the -b option to create multiple
files with the query results.

NetCDF notes:

This script attempts to be convert a MySQL type to an appropriate netcdf type.  Currently only
netcdf3 is supported since that is all that the Nio library in PyNGL currently supports.  Thus,
unfortunately, there are some conversion issues.  In some cases, the netcdf files will be larger
than ought to be necessary, and in other cases, it is possible that information could be truncated.

MySQL                NetCDF              comment
tiny (int8)          16 bit integers
shorts (int16)       32 bit integers     because MySQL can be unsigned and NetCDF3 are signed
mediumint (int24)    32 bit integers
int (int32)          32 bit integers     if MySQL is unsigned, numbers larger >= 2^31 will
                                         be encoded as negatives
bigint (int64)       32 bit integers     numbers >= 2^31 or < 2^31 will be wrapped around
                                         i.e. only lowest precision 32 bits are stored
Time                 32 bit float        Unixtime conversion (the date is assumed to be 1970-01-01)
decimal              32 bit float
float                32 bit float        nothing lost if MySQL field is unsigned
date                 64 bit float        Unixtime conversion
datetime             64 bit float        Unixtime conversion
timestamp            64 bit float        Unixtime conversion
year                 64 bit float        Unixtime conversion
double               64 bit float        nothing lost if MySQL field is unsigned
string               char                padded with ASCII NULL characters

CSV notes:
STDOUT is allowed.

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
    def __init__(self,onechar_opts= "hq:o:NCH:u:p:d:P:s:c:bB:S:", word_opts=\
                 ["help","query=","output=","netcdf","CSV","host=",
                 "user=","passwd=","db=","port=","socket=","config=",
                 "block","block-size","blocks-per-file=","sort-fields="]):
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

            elif o in ('-b', "--break"):
                self['break_into_blocks'] = True

            elif o in ('-B', "--block-size"):
                self['block_size'] = int(a)

            elif o in ("--blocks-per-file",):
                self['blocks_per_file'] = int(a)

            elif o in ("-o", "--output"):
                self['output'] = a

            elif o in ("-N", "--netcdf"):
                self['out_type'] = "netcdf"

            elif o in ("-C", "--CSV"):
                self['out_type'] = "CSV"

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
        self['out_type'] = "netcdf"
        self['break_into_blocks'] = False
        self['block_size'] = 10000
        self['blocks_per_file'] = 0
        self['sort_fields'] = ""
        self['con_opts'] = {}

    def sanitize(self):
        if self['out_type'] not in ('netcdf','CSV'):
            print "Sorry, only netcdf and CSV are supported right now."
            sys.exit(3)
        
        if self['out_type']=="netcdf" and self['output']=="":
            usage()
            print "ERROR: You must give an output file for netcdf's"
            sys.exit(3)

        # sanatize the query to guard against non selects
        if not re.search('^\s*select[^;]*$',self['query'],re.I):
            usage()
            print "ERROR: Queries must be select statements without ';'s"
            sys.exit(3)

        if self['break_into_blocks'] and self['output']=='':
            usage()
            print "ERROR: Writing to stdout and breaking into blocks is not allowed"
            sys.exit(3)

def None2Fill(x,y):
    if x==None:
        return y
    return x    

def datetime2unixtime(dt):
    if dt.__class__ == datetime.datetime:
        td = dt - datetime.datetime(1970,1,1)
        return td.days*24*60**2 + td.seconds + td.microseconds*1e-3
    else:
        return dt

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


class NetcdfOutput(OutputBase):
    def __init__(self,filename, multiple, block, description):
        OutputBase.__init__(self,filename, multiple)
        self._ncvars = []
        self._converters = []
        self._description = self.description2dict(description)
        global Nio
        try:
            from PyNGL import Nio
        except:
            from PyNIO import Nio


    def create_file(self,num):
        OutputBase.create_file(self)
        
        possible_flds = [x for x in dir(MySQLdb.FIELD_TYPE) if re.search('^[^_]',x)]

        if os.path.exists(self._filename):
            os.remove(self._filename)
        self._file = Nio.open_file(self._filename,'rw')
        self._file.create_dimension('DIMREC',int(num))

        self._ncvars = range(len(self._description))
        self._converters = range(len(self._description))
        tmpdim = {}
        for ind, fld in enumerate(self._description):
            if fld['type'] in [myfld.SHORT,myfld.INT24, myfld.LONG, myfld.LONGLONG]:
                self._ncvars[ind] = self._file.create_variable(fld['name'],'l',('DIMREC',))
                self._ncvars[ind]._FillValue = N.int32(-9999)
                self._converters[ind] = N.int32
            elif fld['type'] in [myfld.TINY]:
                self._ncvars[ind] = self._file.create_variable(fld['name'],'h',('DIMREC',))
                self._ncvars[ind]._FillValue = N.int16(-9999)
                self._converters[ind] = N.int16
            elif fld['type'] in [myfld.TIME, myfld.DECIMAL, myfld.FLOAT, \
                                 myfld.NEWDECIMAL ]:
                self._ncvars[ind] = self._file.create_variable(fld['name'],'f',('DIMREC',))
                self._ncvars[ind]._FillValue = N.float32(-9999.999)
                self._converters[ind] = N.float32
            elif fld['type'] in [myfld.DATE, myfld.DATETIME, myfld.NEWDATE, myfld.TIMESTAMP, \
                                 myfld.YEAR, myfld.DOUBLE]:
                self._ncvars[ind] = self._file.create_variable(fld['name'],'d',('DIMREC',))
                self._ncvars[ind]._FillValue = N.float64(-9999.999)
                self._converters[ind] = N.float64
            elif fld['type'] in [myfld.STRING,myfld.VAR_STRING,myfld.VARCHAR,myfld.CHAR]:
                # strings require another dimension
                # so we dynamically create dims as needed and store into tmpdim
                sz = fld['internal_size']
                # if no tmp dimension of size sz was created, then create it
                if not tmpdim.has_key(sz):
                    self._file.create_dimension('DIM%0.0f' % sz, sz)
                    tmpdim[sz] = 'DIM%0.0f' % sz

                self._ncvars[ind] = self._file.create_variable(fld['name'],'c',('DIMREC',tmpdim[sz]))
                self._ncvars[ind]._FillValue = '\0'
                self._converters[ind] = None
                    
            else:
                fld_types = ' '.join([x for x in possible_flds if eval('myfld.%s' % x)==fld['type']])
                print "Warning: skipping variable %s because its type '%s' is unhandled\n" % \
                      (fld['name'],fld_types)


    def write_data(self,rows,offset=0):
        for fld_ind, var in enumerate(self._ncvars):

            if var.typecode() == 'S1':
                # hack to deal with strings
                tmp = [x[fld_ind] for x in rows]
                fv = var._FillValue

                # create a matrix of fill values [[fv,fv,fv,...],...]
                # the number of columsn should be var.shape()[1]
                # and the number of rows given by len(rows)
                chararray = N.zeros((len(rows),var.shape[1]),'c')
                chararray[:] = fv
                for ind, val in enumerate(tmp):
                    if val!= None:
                        chararray[ind,:len(val)] = N.array(val,'c')

                var[offset:(offset+len(rows)),:] = chararray 
                
            else:
                # generate array (could be any type depending on contents)
                # Note (mysql) Null -> (python) None -> (numpy) None -> Fillvalue
                tmp = [None2Fill(x[fld_ind],var._FillValue) for x in rows]
                # srub out datetimes to unix time
                tmp = [datetime2unixtime(x) for x in tmp]
                # enforce conversion to type
                if self._converters is not None:
                    for ind,x in enumerate(tmp):
                        if x is not None:
                            tmp[ind] = N.cast[self._converters[fld_ind]](x)
                var[offset:offset+len(rows)] = tmp

    def close_file(self):
        self._file.close()
        OutputBase.close_file(self)
        
class CSVOutput(OutputBase):
    def __init__(self,filename, multiple, description):
        OutputBase.__init__(self,filename, multiple)
        self._csv = None;
        self._description = self.description2dict(description)
        global csv
        import csv

    def create_file(self,num):
        OutputBase.create_file(self)
      
        if os.path.exists(self._filename):
            os.remove(self._filename)

        if self._filename!='':
            self._file = open(self._filename,'wb')
        else:
            self._file = sys.stdout
            
        self._csv = csv.writer(self._file,quoting=csv.QUOTE_NONNUMERIC)
        self._csv.writerow([fld['name'] for fld in self._description])

    def write_data(self,rows,offset=None):
            self._csv.writerows(rows)

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

    # change the converters so that the decimal type in Mysql is returned as
    # float rather than decimal.decimal (Very slow!)
    conversions = MySQLdb.converters.conversions.copy() 
    conversions[MySQLdb.constants.FIELD_TYPE.NEWDECIMAL] = float
    p['con_opts']['conv'] = conversions

    # connect
    my = MySQLdb.connect(**p['con_opts'])

    # create cursor to contain data about tail_nums to parse
    mycur = my.cursor()

    # grab the query
    if p['break_into_blocks']:
        # create temporary table to put the query into on server side
        mycur.execute('drop temporary table if exists TMLDMPA')
        if len(p['sort_fields'])>0:
            mycur.execute('create temporary table TMLDMPA (index PRIM (%s)) %s' % \
                          (p['sort_fields'],p['query'] ))
        else:
            mycur.execute('create temporary table TMLDMPA %s' % p['query'] )
            
        # figure out how many records for netcdf file(s)
        mycur.execute('select count(*) from TMLDMPA')
        NUM = mycur.fetchall()[0][0]
        if NUM==0:
            print "ERROR: No records returned"
            sys.exit(4)
        
        if len(p['sort_fields'])>0:
            tmp_query = 'select * from TMLDMPA order by ' + p['sort_fields'] + \
                        ' limit %0.0f, %0.0f'
        else:
            tmp_query = 'select * from TMLDMPA limit %0.0f, %0.0f'

        if p['blocks_per_file'] == 0:
            # just need this big enough so that we don't create new files
            p['blocks_per_file'] = int(math.ceil(float(NUM) / p['block_size']))
            multiple = False
        else:
            multiple = True
    
            
        file_length = p['block_size']*p['blocks_per_file']
        # loop grabbing the blocks of data 1 at a time
        fini = False
        loop_num = 0
        while not fini:
            # get a block of data
            mycur.execute(tmp_query % (loop_num*p['block_size'],p['block_size']))
            if loop_num == 0:
                # first round through create the output object
                if p['out_type'] == 'netcdf':
                    outfile = NetcdfOutput(p['output'],multiple, file_length, mycur.description)
                elif p['out_type'] == 'CSV':
                    outfile = CSVOutput(p['output'],multiple, mycur.description)
                # ...and create the file

            if mycur.rowcount>0:
                # if data available, go ahead and write it

                # figure out what loop number relative to the file we are working on
                loop_num_file = loop_num % p['blocks_per_file'] 
                
                # first see if we have to create a new file
                if loop_num_file == 0:
                    # figure out how many we have left to write out
                    number_left = NUM-(loop_num)*p['block_size']
                    # create a file that can handle the smaller of file_length and number_left
                    outfile.create_file(min(file_length,number_left))

                # now write it
                outfile.write_data(mycur.fetchall(),loop_num_file *p['block_size'])

            # see if we should break out of the loop
            if mycur.rowcount < p['block_size']:
                fini = True

            # increment counter
            loop_num += 1


        # we are done so close the file
        outfile.close_file()
        
    else:
        # execute query
        mycur.execute(p['query'])
        if mycur.rowcount==0:
            print "ERROR: No records returned"
            sys.exit(4)

        # create the output object
        if p['out_type'] == 'netcdf':
            outfile = NetcdfOutput(p['output'], False, mycur.rowcount, mycur.description)
        elif p['out_type'] == 'CSV':
            outfile = CSVOutput(p['output'],False,mycur.description)

        # create the file
        outfile.create_file(mycur.rowcount)
        # write the data
        outfile.write_data(mycur.fetchall())
        # close the file
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
