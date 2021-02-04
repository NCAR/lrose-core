#!/usr/bin/env python2
import sys,os
import getopt
import pdb
#import profile as cProfile
import sre
import random
import traceback

import MySQLdb

def usage():
    # usage help and exit
    print '''Usage: %s [options]
Options:

      -h, --help                 This usage
      -q, --query=QUERY          SQL query - see below for restrictions
      -H, --host=HOST            name of host
      -u, --user=USER            username for connection
      -p, --passwd=PASSWD        password <better idea to use config file for this>
      -d, --db=DB                name of database to PUT data into
      -P, --port=PORT            port number
      -s, --socket=FILENAME      filename of socket - typically only needed if host is localhost
      -c, --config=FILENAME      filename of config file
      -t, --table=TABLE          name of table to put data into
      -D, --drop                 add DROP TABLE statement to drop the table.  Otherwise it
                                 will error if the table already exists.
      --leave-proc               create the procedure, call it, but do not drop
      --no-call                  create the procedure but do not call or drop
      --proc-name=NAME           name of the procedure.  A random one is normally generated
                                 this is really only useful for debugging, or if you want to
                                 keep the procedure - which is not normally useful.
      --drop-proc                drop any existing procedure *before* defining a new one
                                 this is useful if debugging with --no-call/--leave-proc
      --debug                    just print out the SQL and exit

This program is very specific to the MySQL insitu turbulence database.  The point of it is to
create a table with records corresponding to (a subset of) the insitu_orig table with fields
specifying time before and after a particular event.  This table can then be joined to insitu_orig
using insitu_orig_id so that one can pull out time segments around an event.   It is somewhat
flexible in that the query has some freedom.

It creates a table in the DB database in which time before and after a specified event
is calculated.

For example: --query="SELECT insitu_orig_id, measurement_time, tail_num, peak_edr_qc_flag=103 
   from insitu.insitu_orig where measurement_time BETWEEN "2007-05-01" and "2007-06-01"
   order by tail_num, measurement_time"  --db=scratch -t foo103

This will create a table foo103 that has insitu_orig_id, seconds_before, and seconds_after fields.
Every record in the query will have a corresponding record in the new table.  The seconds_before field
will contain the number of seconds *before* the next time that peak_edr_qc_flag=103.  Likewise for
*after*.  NULL for those fields implies that that tail number does not occur.

NOTE that the new table will be created in DB so you must have premissions in that database to
create, alter, insert, and update tables in that database.

QUERY:  There are pretty harsh restrictions on the query.

* Names of tables that are not in the DB database must have the database name prefixed. (in the
  example, note that the from insitu.inistu_orig
* Structure MUST be:  SELECT insitu_orig_id, measurement_time, tail_num, <BOOLEAN_EXPR> from
                      <TABLE_EXPR> [<WHERE_COND>] order by tail_num, measurement_time"

  where anything in the <> is something for you to add in, and [] are optional.  The TABLE_EXPR
  can be complex in that there can be joins.  

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
    def __init__(self,onechar_opts= "hq:t:H:u:p:d:P:s:c:D", word_opts=\
                 ["help","query=","table=","host=","user=","passwd=","db=","port=","socket=","drop",
                  "leave-proc","no-call","proc-name=","debug","drop-proc"]):
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
            print argv
            usage()
            print traceback.print_exc()
            #print sys.exc_info()
            sys.exit(2)


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

            elif o in ("-t", "--table"):
                self['table'] = a.strip()

            elif o in ("-D", "--drop"):
                self['drop'] = True

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

            elif o in ("--leave-proc",):
                self['leave_proc'] = True

            elif o in ("--drop-proc",):
                self['drop_proc'] = True

            elif o in ("--no-call",):
                self['no_call'] = True

            elif o in ("--proc-name",):
                self['procname'] = a.strip()

            elif o in ("--debug",):
                self['debug'] = True

    def define_defaults(self):
        self['query'] = ""
        self['output'] = ""
        self['drop'] = False
        self['con_opts'] = {}
        self['leave_proc'] = False
        self['drop_proc'] = False
        self['no_call'] = False
        self['procname'] = ""
        self['debug'] = False
        

    def sanitize(self):
        # sanatize the query to guard against non selects
        if not sre.search('^\s*select[^;]*$',self['query'],sre.I):
            usage()
            print "ERROR: Queries must be select statements without ';'s"
            sys.exit(3)


        if not sre.search('^[a-zA-Z][\w_]*$', self['table'],sre.I):
            usage()
            print "ERROR: TABLE must be 1 word consisting of alphanumeric + '_'"
            sys.exit(3)

        if not sre.search('^([a-zA-Z][\w_]*|)$', self['procname'],sre.I):
            usage()
            print "ERROR: proc-name must be 1 word consisting of alphanumeric + '_'"
            sys.exit(3)

        if not sre.search('order\s+by\s+([a-zA-Z][\w_]*[.]|)(?P<tn>tail_num)\s*,\s*([a-zA-Z][\w_]*[.]|)(?P<mt>measurement_time)',self['query'],sre.I):
            usage()
            print "ERROR: Queries must have an order by tail_num, measurement_time"
            sys.exit(3)
            
procedure_string = """CREATE PROCEDURE `<<PROCNAME>>`()
BEGIN

DECLARE done BOOL DEFAULT FALSE;
DECLARE id BIGINT unsigned;
DECLARE mt datetime;
DECLARE tn int unsigned;
DECLARE flag BOOL;
DECLARE last_time datetime;
DECLARE last_tail int;
DECLARE sec int;
DECLARE cur1 CURSOR FOR <<SELECTASC>>;
DECLARE cur2 CURSOR FOR <<SELECTDESC>>;
DECLARE
        CONTINUE HANDLER FOR
        SQLSTATE '02000'
        SET done = TRUE;

<<DROP>>
create table <<TABLE>> (insitu_orig_id BIGINT UNSIGNED ,  
  seconds_before int, seconds_after int);

OPEN cur1;

myLoop: LOOP
        FETCH cur1 INTO id, mt, tn, flag;
        IF done THEN
            CLOSE cur1;
            LEAVE myLoop;
        END IF;

        IF flag THEN
            set last_time = mt;
        ELSEIF last_tail <> tn THEN
            set last_time = NULL;
            set last_tail = tn;
       END IF;

        IF  last_time is NULL THEN
           set sec = NULL;
        ELSE
           set sec = unix_timestamp(mt) - unix_timestamp(last_time);
        END IF;

        INSERT <<TABLE>> (insitu_orig_id, seconds_after) VALUES (id,sec);
 
END LOOP;

ALTER TABLE <<TABLE>> add PRIMARY KEY (insitu_orig_id);

OPEN cur2;

set done = FALSE;
set last_time = NULL;
set last_tail = NULL;
 
myLoop2: LOOP
        FETCH cur2 INTO id, mt, tn, flag;
        IF done THEN
            CLOSE cur2;
            LEAVE myLoop2;
        END IF;

        IF flag THEN
            set last_time = mt;
        ELSEIF last_tail <> tn THEN
            set last_time = NULL;
            set last_tail = tn;
       END IF;

        IF  last_time is NULL THEN
           set sec = NULL;
        ELSE
           set sec = unix_timestamp(last_time) - unix_timestamp(mt);
        END IF;

        UPDATE <<TABLE>> set seconds_before = sec where insitu_orig_id = id;
 
END LOOP;

ALTER TABLE <<TABLE>> add index  (seconds_before,seconds_after);

END;
"""

class BadFilename(Exception):
    "Bad filename exception"

def main(argv):
    # Main program.  Takes string containing arguments a la unix command line
    p = Params()
    p.define_defaults()
    p.read_params(argv)
    p.read_config()
    p.parse_param_opts()

    p.sanitize()

    # connect
    my = MySQLdb.connect(**p['con_opts'])

    # create cursor to contain data about tail_nums to parse
    mycur = my.cursor()

    # create procedure name or get it from user
    if p['procname']=="":
        rn = random.randint(0,1000000)
        procname = 'P%s' % rn
    else:
        procname = p['procname']

    # string to store all cmds issued to DB
    all_cmds = ""

    # if dropping procedure
    if p['drop_proc']:
        cmd = 'DROP PROCEDURE IF EXISTS %s;\n' % procname
        all_cmds +=  cmd
        if not p['debug']:
            mycur.execute(cmd)

    # start working on procedure string
    ps = procedure_string

    # if add drop table
    if p['drop']:
        ps = ps.replace('<<DROP>>','DROP TABLE IF EXISTS <<TABLE>>;')
    else:
        ps = ps.replace('<<DROP>>','')        

    # sub table and first select
    ps = ps.replace('<<TABLE>>',p['table'])
    ps = ps.replace('<<SELECTASC>>',p['query'])

    # turn ASC to DESC
    mtch = sre.search('order\s+by\s+([a-zA-Z][\w_]*[.]|)(?P<tn>tail_num)\s*,\s*([a-zA-Z][\w_]*[.]|)(?P<mt>measurement_time)',p['query'],sre.I)
    tn_pos = mtch.span('tn')
    mt_pos = mtch.span('mt')
    qry = p['query'][:tn_pos[0]] + 'tail_num DESC' +  p['query'][tn_pos[1]:mt_pos[0]] + 'measurement_time DESC'

    # sub second select
    ps = ps.replace('<<SELECTDESC>>',qry)

    # finally substitute procname
    ps = ps.replace('<<PROCNAME>>',procname)

    # if not debug execute, otherwise just print
    if not p['debug']:
        mycur.execute(ps)

    # add to all cmds string
    all_cmds += ps

    # if do no all call
    if not p['no_call']:
        cmd = 'call %s;\n' % procname
        all_cmds += cmd
        if not p['debug']:
            mycur.execute(cmd)
        
    # if leave_proc
    if not (p['leave_proc'] or p['no_call']):
        cmd = 'DROP PROCEDURE %s;\n' % procname
        all_cmds += cmd
        if not p['debug']:
            mycur.execute(cmd)

    if p['debug']:
        print all_cmds
        

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
