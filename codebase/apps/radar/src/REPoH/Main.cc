/**
 * @file Main.cc
 * @mainpage RadiaFilt
 */

#include "Repoh.hh"
#include "RepohParms.hh"
#include "Volume.hh"
#include <FiltAlgVirtVol/VirtVolParms.hh>

static void _parseCmdArgs(int argc, char **argv, VirtVolParms &vparms,
			  std::string &algParms);

//------------------------------------------------------------------
static void tidyAndExit(int i)
{
  exit(i);
}

//------------------------------------------------------------------
int main(int argc, char **argv)
{
  VirtVolParms vparms;
  string algParms;

  _parseCmdArgs(argc, argv, vparms, algParms);

  Repoh alg(algParms, tidyAndExit);
  if (!alg.ok())
  {
    exit(1);
  }
  if (!vparms.checkConsistency(*alg.getAlgorithm()))
  {
     exit(1);
  }

  RepohParms P = alg.getParms();

  Volume volume(P, vparms, argc, argv);

  time_t t;
  while (volume.trigger(t))
  {
    printf("Triggered\n");
    alg.run((VolumeData *)(&volume));
    volume.output(t);
    volume.clear();
  }
  
  printf("Done\n");
  tidyAndExit(0);
}

static void _parseCmdArgs(int argc, char **argv, VirtVolParms &vparms,
			  std::string &algParms)
{
  string app = "REPoH";
  string s1 = argv[1];
  bool printVVParms = false;
  bool printAParms = false;
  if (s1 == "-h" || s1 == "-help" || s1 == "-?") 
  {
    printf("Usage:\n");
    printf("   %s -printVirtVolParams\n", app.c_str());
    printf("   %s -printAlgorithmParams\n", app.c_str());
    printf("   %s -VirtVolParams <parmfile> -AlgorithmParams <parmfile>\n",
	   app.c_str());
    printf("   %s -help -h -?\n", app.c_str());
    exit(0);
  }
  string virtVolParmFile = "";
  algParms.clear();
  for (int i=0; i<argc; )
  {
    string s = argv[i];
    if (s == "-VirtVolParams")
    {
      if (++i >= argc)
      {
	printf("ERROR, -VirtVolParams can't be last argument\n");
	exit(1);
      }
      virtVolParmFile = argv[i];
      i++;
    }
    else if (s == "-AlgorithmParams")
    {
      if (++i >= argc)
      {
	printf("ERROR, -AlgorithmParams can't be last argument\n");
	exit(1);
      }
      algParms = argv[i];
      i++;
    }
    else if (s == "-printVirtVolParams")
    {
      printVVParms = true;
      i++;
    }
    else if (s == "-printAlgorithmParams")
    {
      printAParms = true;
      i++;
    }
    else
    {
      ++i;
    }
  }
  
  if (printVVParms)
  {
    if (virtVolParmFile.empty())
    {
      VirtVolParams P;
      P.print(stdout, PRINT_VERBOSE);
      exit(0);
    }
    else
    {
      // build the virtual volume parms
      VirtVolParams v;
  
      if (v.load(virtVolParmFile.c_str(), NULL, TRUE, FALSE))
      {
	printf("ERROR loading file %s\n", virtVolParmFile.c_str());
	exit(1);
      }
      v.print(stdout, PRINT_VERBOSE);
      exit(0);
    }
  }
  if (printAParms)
  {
    if (algParms.empty())
    {
      RepohParms rp;
      rp.print();
      exit(0);
    }
    else
    {
      RepohParms rp(algParms);
      rp.print();
      exit(0);
    }      
  }

  // if here want both virtvol and alg params
  if (algParms.empty() || virtVolParmFile.empty())
  {
    printf("ERROR need both -VirtVolParams and -AlgorithmParams arguments\n");
    exit(1);
  }

  // build the virtual volume parms
  VirtVolParams v;
  
  if (v.load(virtVolParmFile.c_str(), NULL, TRUE, FALSE))
  {
    printf("ERROR loading file %s\n", virtVolParmFile.c_str());
    exit(1);
  }
  vparms = VirtVolParms(v);
  if (!vparms.isOk())
  {
    printf("ERROR converting to VirtVolParms derived class\n");
    exit(1);
  }
}
