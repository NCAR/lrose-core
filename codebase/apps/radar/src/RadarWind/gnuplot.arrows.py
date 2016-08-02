#!/usr/bin/env python

import sys

# Create gnuplot "arrow" cmds to show a 2d or 3d vectorfield.


def badparms( msg):
  print '\nError: %s' % (msg,)
  print '''

Parms:

  -ndim     num dimensions, 2 or 3'

  Following parms are repeated as a group:'
    -tag      <string>  gnuplot key, or \'none\''
    -inFile   <string>  input file'
    -lt       <string>  a color spec like:'
                4'
                  0 black, forces dotted line'
                  1 red   2 green   3 dk blue   4 magenta'
                  5 cyan   6 yellow   7 black'
                  8 orange   9 gray   10 red (wraps)'
                \'rgb "blue"\''
                \'rgb "#0000FF"\''
    -lw       <float> line width'
    -scales   scalex,scaley,scalez: scale factors for arrow lengths'
                or \'none\' for omitted dim'
    -locCols  x,y,z   column indices for arrow location (origin 1)'
    -valCols  u,v,w   column indices for arrow direction (origin 1)'
    -offsets  ou,ov,ow  plot (u-ou), (v-ov), (w-ov)'
                        xlims,ylims,zlims are checked after adjusting'
                        for offsets'

  -xlims    min,max'
  -ylims    min,max'
  -zlims    min,max'
  -head     initial gnuplot cmd (may be repeated)'
  -tail     final gnuplot cmd (may be repeated)'
  -animate  xrot,zrotStart,zrotLim,zrotInc   used with gif animation'
  -outFile  output file of gnuplot cmds'

===== Example data =====

##mawk 'BEGIN {for (i=0; i<5; i++) print i, 2*i, 3*i, 4*i, 5*i, 6*i}' \
  < /dev/null > tempa

python -c 'for i in range(20): print "%7g  %7g  %7g    %7g  %7g  %7g  %7g" % (1*i, 2*i, 3*i, 1*i, 2*i, 3*i, 1*i)' > tempa

===== Example: 2 dim =====

./gnuplot.arrows.py \
  -ndim 2 \
  -tag alpha -inFile tempa \
    -lt 1 -lw 1.0 -scales 0.10,0.10 -locCols 1,2 -valCols 4,5 \
    -offsets 0,0 \
  -tag beta -inFile tempa \
    -lt 2 -lw 1.0 -scales 0.10,0.10 -locCols 1,2 -valCols 4,7 \
    -offsets 0,0 \
  -head 'set term png large size 1000,800' \
  -head "set output 'tempa.png'" \
  -head "set title 'Test alpha'" \
  -head "set bmargin 6" \
  -head "set label 1111 'Notes: someNote' at screen 0.1,0.03" \
  -head "set xlabel 'xlabel'" \
  -head "set ylabel 'ylabel'" \
  -head "set key right top width 2 height 2 box" \
  -tail 'plot NaN lt 1 title "alpha", NaN lt 2 title "beta"' \
  -xlims 0,50 \
  -ylims 0,50 \
  -outFile tempa.cmd

gnuplot tempa.cmd
display tempa.png


===== Example: 3 dim interactive =====

./gnuplot.arrows.py \
  -ndim 3 \
  -tag alpha -inFile tempa \
    -lt 1 -lw 1.0 -scales 0.1,0.1,0.1 -locCols 1,2,3 -valCols 4,5,6 \
    -offsets 0,0,0 \
  -tag beta -inFile tempa \
    -lt 2 -lw 1.0 -scales 0.1,0.1,0.1 -locCols 1,2,3 -valCols 4,5,7 \
    -offsets 0,0,0 \
  -head "set title 'Test alpha'" \
  -head "set bmargin 6" \
  -head "set label 1111 'Notes: someNote' at screen 0.1,0.03" \
  -head "set xlabel 'xlabel'" \
  -head "set ylabel 'ylabel'" \
  -head "set zlabel 'zlabel'" \
  -head "set key right top width 2 height 2 box" \
  -tail 'set view 80,30' \
  -tail 'splot [][][] NaN lt 1 title "alpha", NaN lt 2 title "beta"' \
  -tail 'pause -1 "Grab image with mouse.  Press enter to end."' \
  -xlims 0,50 \
  -ylims 0,50 \
  -zlims 0,50 \
  -outFile tempa.cmd

gnuplot tempa.cmd


===== Example: 3 dim rotating gif animation =====

./gnuplot.arrows.py \
  -ndim 3 \
  -tag alpha -inFile tempa \
    -lt 1 -lw 1.0 -scales 0.1,0.1,0.1 -locCols 1,2,3 -valCols 4,5,6 \
    -offsets 0,0,0 \
  -tag beta -inFile tempa \
    -lt 2 -lw 1.0 -scales 0.1,0.1,0.1 -locCols 1,2,3 -valCols 4,5,7 \
    -offsets 0,0,0 \
  -head 'set term gif large size 1000,800 animate delay 20 loop 0 optimize' \
  -head "set output 'tempa.gif'" \
  -head "set title 'Test alpha'" \
  -head "set bmargin 6" \
  -head "set label 1111 'Notes: someNote' at screen 0.1,0.03" \
  -head "set xlabel 'xlabel'" \
  -head "set ylabel 'ylabel'" \
  -head "set zlabel 'zlabel'" \
  -head "set key right top width 2 height 2 box" \
  -tail 'set view 80,30' \
  -tail 'splot [][][] NaN lt 1 title "alpha", NaN lt 2 title "beta"' \
  -animate 70,30,180,2 \
  -xlims 0,50 \
  -ylims 0,50 \
  -zlims 0,50 \
  -outFile tempa.cmd

gnuplot tempa.cmd
animate tempa.png

'''
  print '*** error: see above ***'
  sys.exit(1)






def main():
  ndim = None
  tags = []
  inFiles = []
  lts = []
  lws = []
  scales = []
  locCols = []
  valCols = []
  offsets = []
  heads = []
  tails = []
  xlims = []
  ylims = []
  zlims = []
  animate = None
  outFile = None

  iarg = 1
  argLen = len(sys.argv)
  while iarg < argLen:
    if iarg == argLen - 1: badparms('args must be key/value pairs')
    keystg = sys.argv[iarg]
    valstg = sys.argv[iarg+1]
    iarg += 2

    if keystg == '-ndim':
      ndim = int( valstg)
      if ndim not in [2,3]: badparms('invalid ndim')
    elif keystg == '-tag': tags.append( valstg)
    elif keystg == '-inFile': inFiles.append( valstg)
    elif keystg == '-lt': lts.append( valstg)
    elif keystg == '-lw': lws.append( valstg)
    elif keystg == '-scales':
      toks = valstg.split(',')
      if len(toks) != ndim: badparms('invalid scales: "%s"' % (valstg,))
      scvals = []
      for ii in range(len(toks)):
        if toks[ii] == 'none': scvals.append( None)
        else: scvals.append( float( toks[ii]))
      scales.append( scvals)
    elif keystg == '-locCols':
      toks = valstg.split(',')
      if len(toks) != ndim: badparms('invalid locCols: "%s"' % (valstg,))
      locCols.append( map( int, toks))
    elif keystg == '-valCols':
      toks = valstg.split(',')
      if len(toks) != ndim: badparms('invalid valCols: "%s"' % (valstg,))
      valCols.append( map( int, toks))
    elif keystg == '-offsets':
      toks = valstg.split(',')
      if len(toks) != ndim: badparms('invalid offsets: "%s"' % (valstg,))
      offsets.append( map( float, toks))
    elif keystg == '-head': heads.append( valstg)
    elif keystg == '-tail': tails.append( valstg)
    elif keystg == '-xlims':
      toks = valstg.split(',')
      if len(toks) != 2: badparms('invalid xlims: "%s"' % (valstg,))
      xlims = map( float, toks)
    elif keystg == '-ylims':
      toks = valstg.split(',')
      if len(toks) != 2: badparms('invalid ylims: "%s"' % (valstg,))
      ylims = map( float, toks)
    elif keystg == '-zlims':
      toks = valstg.split(',')
      if len(toks) != 2: badparms('invalid zlims: "%s"' % (valstg,))
      zlims = map( float, toks)
    elif keystg == '-animate': animate = valstg
    elif keystg == '-outFile': outFile = valstg
    else: badparms('unknown key: "%s"' % (keystg,))

  nspec = len(tags)

  print 'nspec: %s' % (nspec,)
  print 'ndim: %s' % (ndim,)
  print 'tags: %s' % (tags,)
  print 'inFiles: %s' % (inFiles,)
  print 'lts: %s' % (lts,)
  print 'lws: %s' % (lws,)
  print 'scales: %s' % (scales,)
  print 'locCols: %s' % (locCols,)
  print 'valCols: %s' % (valCols,)
  print 'offsets: %s' % (offsets,)
  print 'heads: %s' % (heads,)
  print 'tails: %s' % (tails,)
  print 'xlims: %s' % (xlims,)
  print 'ylims: %s' % (ylims,)
  print 'zlims: %s' % (zlims,)
  print 'animate: %s' % (animate,)
  print 'outFile: %s' % (outFile,)

  if nspec == 0: badparms('no tags specified')
  if len(inFiles) != nspec: badparms('len mismatch for inFiles')
  if len(lts) != nspec: badparms('len mismatch for lts')
  if len(lws) != nspec: badparms('len mismatch for lws')
  if len(scales) != nspec: badparms('len mismatch for scales')
  if len(locCols) != nspec: badparms('len mismatch for locCols')
  if len(valCols) != nspec: badparms('len mismatch for valCols')
  if len(offsets) != nspec: badparms('len mismatch for offsets')
  if xlims == None: badparms('parm not specified: -xlims')
  if ndim == 3 and zlims == None: badparms('parm not specified: -zlims')
  if outFile == None: badparms('parm not specified: -outFile')

  print 'outFile: "%s"' % (outFile,)

  # Change origin 1 column indices to origin 0.
  for ispec in range( nspec):
    if min(locCols[ispec]) < 1: badparms('invalid locCols; use origin 1.')
    if min(valCols[ispec]) < 1: badparms('invalid valCols; use origin 1.')
    locCols[ispec] = map( lambda x: x - 1, locCols[ispec])
    valCols[ispec] = map( lambda x: x - 1, valCols[ispec])

  fout = open( outFile, 'w')
  for head in heads:
    print >> fout, head
  print >> fout, 'set xrange [%s:%s]' % (xlims[0], xlims[1],)
  print >> fout, 'set yrange [%s:%s]' % (ylims[0], ylims[1],)
  if ndim == 3: print >> fout, 'set zrange [%s:%s]' % (zlims[0], zlims[1],)

  for ispec in range( nspec):
    fin = open( inFiles[ispec])
    iline = 0
    while True:
      line = fin.readline()
      if line == '': break
      iline += 1
      line = line.strip()
      if len(line) > 0 and not line.startswith('#'):
        toks = line.split()
        if max( max(locCols[ispec]), max(valCols[ispec])) >= len(toks):
          print 'locCols[ispec]: ', locCols[ispec]
          print 'valCols[ispec]: ', valCols[ispec]
          print 'len(toks): ', len(toks), '  toks: ', toks
          throwerr('line too short.  iline: %d  line: "%s"' % (iline, line,) )

        xa = float( toks[locCols[ispec][0]])
        ya = float( toks[locCols[ispec][1]])
        if ndim == 3: za = float( toks[locCols[ispec][2]])

        xb = xa
        if scales[ispec][0] != None:
          delta = float( toks[valCols[ispec][0]]) - offsets[ispec][0]
          xb += scales[ispec][0] * delta

        yb = ya
        if scales[ispec][1] != None:
          delta = float( toks[valCols[ispec][1]]) - offsets[ispec][1]
          yb += scales[ispec][1] * delta

        if ndim == 3:
          zb = za
          if scales[ispec][2] != None:
            delta = float( toks[valCols[ispec][2]]) - offsets[ispec][2]
            zb += scales[ispec][2] * delta

        if ndim == 2:
          if    xa >= xlims[0] and xa <= xlims[1] \
            and xb >= xlims[0] and xb <= xlims[1] \
            and ya >= ylims[0] and ya <= ylims[1] \
            and yb >= ylims[0] and yb <= ylims[1]:

            print >> fout, \
              'set arrow from %g,%g to %g,%g size screen 0.005,45,90 filled linetype %s linewidth %s' % (
                xa, ya,
                xb, yb,
                lts[ispec],
                lws[ispec],)
        elif ndim == 3:
          if    xa >= xlims[0] and xa <= xlims[1] \
            and xb >= xlims[0] and xb <= xlims[1] \
            and ya >= ylims[0] and ya <= ylims[1] \
            and yb >= ylims[0] and yb <= ylims[1] \
            and za >= zlims[0] and za <= zlims[1] \
            and zb >= zlims[0] and zb <= zlims[1]:
            print >> fout, \
              'set arrow from %g,%g,%g to %g,%g,%g size screen 0.005,45,90 filled linetype %s linewidth %s' % (
                xa, ya, za,
                xb, yb, zb,
                lts[ispec],
                lws[ispec],)
    fin.close()

  for tail in tails:
    print >> fout, tail

  if animate != None:
    try: rots = map( int, animate.split(','))
    except Exception, exc: badparms('invalid animate spec')
    for rot in range( rots[1], rots[2], rots[3]):
      print >> fout, 'set view %d,%d' % ( rots[0], rot % 360,)  # xrot, zrot
      print >> fout, 'replot'

  fout.close()



def throwerr( msg):
  print '\nError: %s' % (msg,)
  raise Exception( msg)




if __name__ == '__main__': main()

