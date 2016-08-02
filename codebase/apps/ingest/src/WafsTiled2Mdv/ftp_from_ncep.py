#! /usr/bin/python
#########################################################
# script to ftp in WAFS data from NOAA site
#########################################################

import os, sys, time, calendar, math

################################
# make directory safely

def safeMkdir(d):
    """ attempt to create a directory, 
         but don't throw exception if it exists"""
    try:
        os.makedirs(d)
    except OSError, err:
        if err.errno != 17:  # 'File exists'
            raise OSError, err

################################
# write latest data file

def writeLdataFile(outDir, fileName, year, month, day, hour, min, sec, leadSecs):
    """ write latest data file"""
    command = 'LdataWriter -dir ' + outDir
    command += ' -dtype raw '
    timeStr = "%.4d%.2d%.2d%.2d%.2d%.2d" % (year, month, day, hour, min, sec)
    command += ' -ltime ' + timeStr
    leadStr = "%d" % (leadSecs)
    command += ' -lead ' + leadStr
    writer = os.path.basename(sys.argv[0])
    command += ' -writer ' + writer
    command += ' -rpath ' + fileName
    print "Command: ", command
    try:
        os.system(command)
    except OSError, err:
        print err

#################################
# ftp filea

from ftplib import FTP

def ftpGet(inPath, outPath):
    """ attempt to get file from ftp site"""
    ftpHost = 'tgftp.nws.noaa.gov'
    print 'Connecting...'
    localfile  = open(outPath, 'wb')   # where to store download
    connection = FTP(ftpHost)          # connect to ftp site
    connection.login('anonymous','dixon@ucar.edu') #log in
    print 'Downloading...'
    connection.retrbinary('RETR ' + inPath, localfile.write, 1024)
    connection.quit()
    localfile.close()

#################################
# main
    
# make the output dir

outDir = '/tmp/wafs'
safeMkdir(outDir)

# set params

modelInterval = 21600 # number of secs between model runs
modelLatency = 18000  # number of secs before model run arrives

# compute model run time

now = time.time()
searchTime = now - modelLatency
parts = math.modf(searchTime / modelInterval)
modelRunTime = parts[1] * modelInterval
modelRunGmtime = time.gmtime(modelRunTime)

runYear = modelRunGmtime[0]
runMonth = modelRunGmtime[1]
runDay = modelRunGmtime[2]
runHour = modelRunGmtime[3]

# compute input dir path

inDir = "SL.us008001/ST.opnl/MT.gfs_CY.%.2d/RD.%.4d%.2d%.2d/PT.grid_DF.gr1" % (runHour, runYear, runMonth, runDay)

# compute output subdir

runDir = "model_run_%.2d" % (runHour)
outSubDir = os.path.join(outDir, runDir)
safeMkdir(outSubDir)

print "Now         : ", time.gmtime(now)
print "Search time : ", time.gmtime(searchTime)
print "Run time    : ", modelRunGmtime
print "inDir       : ", inDir
print "runDir      : ", runDir
print "outDir      : ", outDir
print "outSubDir   : ", outSubDir

# set up list of forecast lead times and tiles

leadHours = [0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 42, 48, 54, 60, 66, 72, 78, 84, 90, 96]
tiles  = ['i', 'j', 'k', 'l', 'm', 'n', 'o', 'p']

# loop through forecast lead times and tiles

for leadHour in leadHours:
    leadSecs = leadHour * 3600
    for tile in tiles:
        fileName = "fh.%.3d_tl.press_ar.octant%s" % (leadHour, tile)
        relPath = os.path.join(runDir, fileName)
        inPath = os.path.join(inDir, fileName)
        outPath = os.path.join(outDir, relPath)

        print "==================================="
        print "fileName: ", fileName
        print "inPath: ", inPath
        print "outPath: ", outPath
        print "leadSecs: ", leadSecs
        try:
            fileStat = os.stat(inPath)
        except OSError, err:
            print "File does not exist: ", inPath
            ftpGet(inPath, outPath)
            writeLdataFile(outDir, relPath, runYear, runMonth, runDay, runHour, 0, 0, leadSecs)
        
        



