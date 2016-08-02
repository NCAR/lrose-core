import os
import re
import SCons
from SCons.Builder import Builder
from SCons.Action import Action
from SCons.Node import FS
from SCons.Node.Python import Value

def svninfo_emitter(target, source, env):
    """Given an argument for svn info in the first source, replace that
    source with a Value() node with the svn info contents."""
    
    svndict = { "Revision":None, "Last Changed Date":None, "URL":None }
    workdir = source[0].get_abspath()
    if FS.default_fs.isfile(workdir):
        workdir = source[0].dir.get_abspath()
    svncmd = "svn info %s" % (workdir)
    svndict.update ( {"Working Directory":"Working Directory: %s" % workdir} )
    # print svncmd
    svninfo = os.popen(svncmd).read()
    svnversion = os.popen("svnversion -n %s" % (workdir)).read()
    for k in svndict.keys():
        match = re.search(r"^%s: .*$" % (k), svninfo, re.M)
        if (match):
            svndict[k] = match.group()
    svndict.update ( {'workdir':workdir} )
    svndict['Revision'] = svnversion
    svnheader = \
"""
#ifndef SVNINFOINC
#define SVNINFOINC
#define SVNREVISION \"%(Revision)s\"
#define SVNLASTCHANGEDDATE \"%(Last Changed Date)s\"
#define SVNURL \"%(URL)s\"
#define SVNWORKDIRSPEC \"%(Working Directory)s\"
#define SVNWORKDIR \"%(workdir)s\"
#endif
""" % svndict
    # print svnheader
    env['SVNREVISION'] = svndict['Revision']
    env['SVNLASTCHANGEDDATE'] = svndict['Last Changed Date']
    env['SVNURL'] = svndict['URL']
    env['SVNWORKDIRSPEC'] = svndict['Working Directory']
    env['SVNWORKDIR'] = svndict['workdir']
    return target, [Value(svnheader)]

def svninfo_build(env, target, source):
    out = open(target[0].path, "w")
    out.write(source[0].get_contents())
    out.close()

action = Action(svninfo_build, lambda t,s,e: "Generating %s"%t[0])

svninfobuilder = Builder(action = action,
                         source_factory = FS.default_fs.Entry,
                         emitter = svninfo_emitter)

class SvnInfoWarning(SCons.Warnings.Warning):
    pass

def generate(env):
    env['BUILDERS']['SvnInfo'] = svninfobuilder

def exists(env):
    svn = env.WhereIs('svn')
    if not svn:
        SCons.Warnings.warn(
            SvnInfoWarning,
            "Could not find svn program.  svninfo tool not available.")
        return false
    return true
