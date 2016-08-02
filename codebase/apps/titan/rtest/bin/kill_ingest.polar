#! /bin/csh -f

snuff "test2gate -params test2gate.polar"
snuff "polar_ingest -params polar_ingest.polar"
snuff "polar2mdv -params polar2mdv.polar"







