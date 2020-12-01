# -*- python -*-

tools = Split("""
qt5
qtcore
qtgui
qtt_scopeplot
qtt_knob
doxygen
fftw
""")

env = Environment(tools = ['default'] + tools)

# This will create ui_AScope.h
env.Uic(['AScope.ui',])

sources = Split("""
AScope.cpp
PlotInfo.cpp
""") 

headers = Split("""
AScope.h
PlotInfo.h
""")

env['DOXYFILE_DICT'].update({'PROJECT_NAME':'Ascope'})
html = env.Apidocs(sources + headers)

ascope = env.Library('ascope', sources)

Default(ascope)

tooldir = env.Dir('.').srcnode().abspath    # this directory

def ascope(env):
    env.AppendUnique(CPPPATH = [tooldir])
    env.AppendLibrary('ascope')
    env.AppendDoxref('ascope')
    env.Require(tools)

Export('ascope')



