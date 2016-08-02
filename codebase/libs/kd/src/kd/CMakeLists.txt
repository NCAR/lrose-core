cmake_minimum_required(VERSION 2.8)

project(kd)

ADD_LIBRARY(kd
    fileoper.cc
    kd.cc
    metric.cc
    naive.cc
    pqueue.cc
    kd_interp.cc
    kd_query.cc
    tokenize.cc
    )

include_directories ($ENV{GTG_INC_DIR} $ENV{LOCAL_INC_DIR})

INSTALL(FILES libkd.a
        DESTINATION $ENV{GTG_LIB_DIR}
	)

