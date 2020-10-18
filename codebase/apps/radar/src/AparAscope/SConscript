# -*- python -*-

tools = Split("""
qt5
qtcore
ascope
qtt_qtconfig
lrose
doxygen
boost_program_options
""")

env = Environment(tools = ['default'] + tools)
env.EnableQtModules(['QtCore'])

sources = Split("""
main.cpp
AScopeReader.cpp
""")

headers = Split("""
AScopeReader.h
""")

html = env.Apidocs(sources + headers)

tcpscope = env.Program('tcpscope', sources)

Default(tcpscope)
