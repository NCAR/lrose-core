#! /bin/sh

cd $RTEST_HOME/params

if (not_running "test2gate -params test2gate.polar") then
  test2gate -params test2gate.polar &
fi

if (not_running "polar_ingest -params polar_ingest.polar") then
  kill_polar2mdv.polar
  polar_ingest -params polar_ingest.polar -summary 360 &
fi

sleep 5

if (not_running "polar2mdv -params polar2mdv.polar") then
  kill_polar_ingest.polar
  polar_ingest -params polar_ingest.polar -summary 360 &
fi

