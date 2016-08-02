/********************************************************
 * CLUTTER MITIGATION DECISION SYSTEM - VERSION 1
 *
 * C code as part of Functional Description
 *
 * Mike Dixon, NCAR, Boulder, CO, 80301
 *
 * 31 January 2006
 *
 *********************************************************
 *
 * NOTE: this code was derived from C++ code in Lirp2Dsr.
 *
 * It compiles, but has not actually been run and tested.
 *
 *********************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_GATES 2048

#define NGATES_KERNEL 7
#define NBEAMS_KERNEL 5

#define RATIO_LIMIT_INNER 1
#define RATIO_LIMIT_OUTER 3

float SPIN_THRESHOLD = 3.5;
float RATIO_NARROW_THRESHOLD = 5.0;
float PROB_THRESHOLD = 0.5;

float TDBZ_WEIGHT  = 1.0;
float SPIN_WEIGHT = 1.0;
float SDVE_WEIGHT = 0.5;
float RATIO_WIDE_WEIGHT = 0.5;

#define N_MAP_LOOKUP 10001

float CENSORED = -9999;

/******************************
 * Pulse implementation example
 *
 * Note that the meta-data  time, prt, el and az are not actually
 * used in this code, they are just included for context.
 */

typedef struct {
  
  /* meta data */
  
  int nGates; /* number of gates */
  double time; /* time in secs and fractions from 1 Jan 1970 */
  double prt; /* pulse repetition time (secs) */
  double el; /* elevation angle (deg) */
  double az; /* azimuth angle (deg) */
  
  /* IQ data */
  
  float iq[MAX_GATES * 2];
  
} Pulse;


/*****************************
 * Beam implementation example
 *
 * Note that the meta-data  time, prt, el and az are not actually
 * used in this code, they are just included for context.
 */

typedef struct {
  
  /* meta data */
  
  int nSamples; /* number of pulse samples in beam */
  int nGates; /* number of gates */
  double time; /* time for the center pulse of beam */
  double prt; /* pulse repetition time (secs) */
  double el; /* elevation angle for center of beam (deg) */
  double az; /* azimuth angle for center of beam(deg) */
  
  /* Array of Pulse pointers */
  
  Pulse **pulses; /* Pointers to pulses with IQ data */
  
  /* arrays for moments */
  
  float *snr; /* SNR in dB */
  float *dbz; /* reflectivity in dBZ */
  float *vel; /* velocity in m/s */
  float *width; /* spectrum width in m/s */
  
  /* arrays for intermediate fields */
  
  float *dbzDiffSq;
  float *dbzSpinFlag;
  
  /* arrays for CMD fields */
  
  float *tdbz;
  float *spin;
  float *sdve;
  float *ratioNarrow;
  float *ratioWide;
  
  /* array for clutter probability */
  
  float *clutterProb;
  
  /* array for clutter decision flag:
   *   1 means apply clutter filter
   *   0 means no not apply clutter filter
   */
  
  int *clutterFlag;
  
} Beam;

/* 
 * create a beam
 *
 * (a) set meta data
 * (b) allocate array memory
 * (c) initialize values to censored or 0
 *
 * Pulse memory is not owned by the beam. It should be
 * allocated and freed elsewhere.
 */

Beam *create_beam(int nSamples,
                  int nGates,
                  double time,
                  double prt,
                  double el,
                  double az,
                  Pulse **pulses) {

  int isample, igate;

  /* allocate space for beam */

  Beam *beam = (Beam *) malloc(sizeof(Beam));
  
  /* set meta data */
  
  beam->nSamples = nSamples;
  beam->nGates = nGates;
  beam->time = time;
  beam->prt = prt;
  beam->el = el;
  beam->az = az;

  /* allocate space for arrays */
  
  beam->pulses = (Pulse **) malloc(nSamples * sizeof(Pulse *));

  beam->snr = (float *) malloc(nGates * sizeof(float));
  beam->dbz = (float *) malloc(nGates * sizeof(float));
  beam->vel = (float *) malloc(nGates * sizeof(float));
  beam->width = (float *) malloc(nGates * sizeof(float));
  beam->dbzDiffSq = (float *) malloc(nGates * sizeof(float));
  beam->dbzSpinFlag = (float *) malloc(nGates * sizeof(float));
  beam->tdbz = (float *) malloc(nGates * sizeof(float));
  beam->spin = (float *) malloc(nGates * sizeof(float));
  beam->sdve = (float *) malloc(nGates * sizeof(float));
  beam->ratioNarrow = (float *) malloc(nGates * sizeof(float));
  beam->ratioWide = (float *) malloc(nGates * sizeof(float));
  beam->clutterProb = (float *) malloc(nGates * sizeof(float));
  beam->clutterFlag = (int *) malloc(nGates * sizeof(int));

  /* initialize arrays */

  for (isample = 0; isample < nSamples; isample++) {
    beam->pulses[isample] = pulses[isample];
  }

  for (igate = 0; igate < nGates; igate++) {
    beam->snr[igate] = CENSORED;
    beam->dbz[igate] = CENSORED;
    beam->vel[igate] = CENSORED;
    beam->width[igate] = CENSORED;
    beam->dbzDiffSq[igate] = CENSORED;
    beam->dbzSpinFlag[igate] = CENSORED;
    beam->tdbz[igate] = CENSORED;
    beam->spin[igate] = CENSORED;
    beam->sdve[igate] = CENSORED;
    beam->ratioNarrow[igate] = CENSORED;
    beam->ratioWide[igate] = CENSORED;
    beam->clutterProb[igate] = CENSORED;
    beam->clutterFlag[igate] = 0;
  }

  return beam;

}

/*
 * free memory associated with a beam
 */

void free_beam(Beam *beam) {

  free(beam->pulses);
  free(beam->snr);
  free(beam->dbz);
  free(beam->vel);
  free(beam->width);
  free(beam->dbzDiffSq);
  free(beam->dbzSpinFlag);
  free(beam->tdbz);
  free(beam->spin);
  free(beam->sdve);
  free(beam->ratioNarrow);
  free(beam->ratioWide);
  free(beam->clutterProb);
  free(beam->clutterFlag);
  free(beam);

}
  


/************************************
 * Beam Queue implementation example
 *
 * The Beam Queue can be implemented as an array of
 * pointers to Beams.
 */

Beam *beamQueue[NBEAMS_KERNEL];

/*
 * initialize the beam queue
 */

void initialize_beam_queue() {
  
  int ii;
  for (ii = 0; ii < NBEAMS_KERNEL; ii++) {
    beamQueue[ii] = (Beam *) NULL;
  }
  
}

/*
 * Add a beam to the front of the queue,
 * pop the last beam from the end of the queue.
 *
 * This assumes that that the beam has already been created.
 */

void add_beam_to_queue(Beam *beam) {
  
  int ii;
  
  /* free up last entry in queue */
  
  if (beamQueue[NBEAMS_KERNEL-1] != NULL) {
    free_beam(beamQueue[NBEAMS_KERNEL-1]);
    beamQueue[NBEAMS_KERNEL-1] = NULL; /* not strictly necessary */
  }
  
  /* move beams down queue */

  for (ii = 0; ii < NBEAMS_KERNEL-1; ii++) {
    beamQueue[ii+1] = beamQueue[ii];
  }
  
  /* add beam to front of queue */
  
  beamQueue[0] = beam;
  
}



/**************************************
 * Interest Map implementation example
 *
 * Implemented as a lookup table.
 */

/* struct for points which define interest map */

typedef struct {
  double feature;
  double interest;
} ImPoint;


/* interest map struct */

typedef struct {
  int nPoints;
  ImPoint *points;
  double minFeature;
  double maxFeature;
  double deltaFeature;
  double weight;
  float *lut;
} InterestMap;


/*
 * Initialize map by passing in the points which define the map
 * along with the weight for the map as a whole
 *
 * Returns NULL on failure.

 */

InterestMap *create_map(ImPoint *pts,
                        int npts,
                        double wt) {

  int ii, jj;
  double slope;
  InterestMap *map = NULL;

  if (npts < 2) {
    /* error - must have at least 2 points to define function */
    return NULL;
  }
  
  /* allocate map itself */

  map = malloc(sizeof(InterestMap));

  /* clear map */

  memset(map, 0, sizeof(InterestMap));

  /* set meta data */

  map->nPoints = npts;
  map->weight = wt;
  
  /* allocate space for points which define map */
  
  map->points = malloc(npts * sizeof(ImPoint));
  
  /* copy point data */
  
  memcpy(map->points, pts, npts * sizeof(ImPoint));

  /* allocate space for lookup table */
  
  map->lut = (float*) malloc(N_MAP_LOOKUP * sizeof(float));
  
  /* compute min, max and delta */

  map->minFeature = map->points[0].feature;	
  map->maxFeature = map->points[npts-1].feature;
  map->deltaFeature =
    (map->maxFeature - map->minFeature) / (N_MAP_LOOKUP - 1.0);
  
  /* compute slope for initial map segment */

  jj = 1;
  slope = ((map->points[jj].interest - map->points[jj-1].interest) /
           (map->points[jj].feature - map->points[jj-1].feature));

  /* load lookup table */
  
  for (ii = 0; ii < N_MAP_LOOKUP; ii++) {
    double interest;
    double val = map->minFeature + ii * map->deltaFeature;
    if ((val > map->points[jj].feature) && (jj < npts-1)) {
      /* move along by one segment */
      jj++;
      slope = ((map->points[jj].interest - map->points[jj-1].interest) /
               (map->points[jj].feature - map->points[jj-1].feature));
    }

    /* compute interest, add to lookup table */

    interest = map->points[jj-1].interest +
      (val - map->points[jj-1].feature) * slope;
    
    map->lut[ii] = interest;
    
  } /* ii */

  return map;
  
} /* create map */

/* free map memory */

void free_map(InterestMap *map) {
  free(map->points);
  free(map->lut);
  free(map);
}


/*
 * look up interest given the feature value
 */

double getInterest(InterestMap *map, double feature)
{
  int jj = (int) floor((feature - map->minFeature) / map->deltaFeature + 0.5);
  if (jj < 0) {
    jj= 0;
  } else if (jj > N_MAP_LOOKUP-1) {
    jj = N_MAP_LOOKUP-11;
  }
  return map->lut[jj];
}

/*
 * get weight associated with a map
 */

double getWeight(InterestMap *map)
{
  return map->weight;
}
  
/*
 * Set up interest maps for each feature field
 */

InterestMap *tdbzMap;
InterestMap *spinMap;
InterestMap *sdveMap;
InterestMap *ratioWideMap;

ImPoint tdbzPoints[2] = {{0.0, 0.0}, {45.0, 1.0}};
ImPoint spinPoints[3] = {{0.0, 0.0}, {50.0, 1.0}, {100.0, 0.0}};
ImPoint sdvePoints[2] = {{0.0, 1.0}, {0.7, 0.0}};
ImPoint ratioWidePoints[2] = {{9.0, 0.0}, {15.0, 1.0}};

void create_interest_maps() {

  tdbzMap = create_map(tdbzPoints, 2, TDBZ_WEIGHT);
  spinMap = create_map(spinPoints, 3, SPIN_WEIGHT);
  sdveMap = create_map(sdvePoints, 2, SDVE_WEIGHT);
  ratioWideMap = create_map(ratioWidePoints, 2, RATIO_WIDE_WEIGHT);

}

/******************************
 * TDBZ implementation example
 * Compute TDBZ feature field
 */

/*
 * Prepare for computing TDBZ by computing
 * the gate-to-gate squared difference in Dbz
 */

void prepare_tdbz(Beam *beam) {

  int igate;

  /*
   * compute the squared difference in dBZ from gate to gate in
   * the beam.
   */
  
  for (igate = 1; igate < beam->nGates; igate++) {

    float prevDbz = beam->dbz[igate-1];
    float dbz = beam->dbz[igate];

    if (prevDbz != CENSORED && dbz != CENSORED) {
      float dbzDiff = dbz - prevDbz;
      beam->dbzDiffSq[igate] = dbzDiff * dbzDiff;
    }

  }

  /* use gate 1 value for gate 0 */

  beam->dbzDiffSq[0] = beam->dbzDiffSq[1];

}

/*
 * Computing the TDBZ feature field
 *
 * compute the mean squared difference over the kernel,
 * setting the values for the center beam of the beam queue.
 */

void compute_tdbz() {

  Beam *centerBeam = beamQueue[NBEAMS_KERNEL/2];
  int igate;
  double sum = 0.0;
  double nn = 0.0;
  double mean = 0.0;
  
  for (igate = 0; igate < centerBeam->nGates; igate++) {
    
    int startGate, endGate;
    int jbeam, jgate;
    
    /*
     * compute the start and end index, checking to make sure we
     * do not go off either end
     */
    
    startGate = igate - NGATES_KERNEL/2;
    if (startGate < 0) {
      startGate = 0;
    }
    
    endGate = igate + NGATES_KERNEL/2;
    if (endGate > centerBeam->nGates - 1) {
      endGate = centerBeam->nGates - 1;
    }
  
    /*
     * iterate over the kernel
     * ignore censored gates
     */

    for (jbeam = 0; jbeam < NBEAMS_KERNEL; jbeam++) {
      for (jgate = startGate; jgate <= endGate; jgate++) {
        if (beamQueue[jbeam]->dbzDiffSq[jgate] != CENSORED) {
          sum += beamQueue[jbeam]->dbzDiffSq[jgate];
          nn += 1.0;
        }
      } /* jgate */
    } /* jbeam */
    
    if (nn > 0) {
      mean = sum / nn;
      centerBeam->tdbz[igate] = mean;
    }
    
  
  } /* igate */

} /* compute_tdbz() */



/*****************************
 * SPIN implementation example
 * Compute SPIN feature field
 */

/*
 * Prepare for computing spin by setting the spin flag for
 * each gate
 */

void prepare_spin(Beam *beam) {

  int igate;
  int spinSense = 0;

  /* first set spin flag for each gate */
  
  for (igate = 1; igate < beam->nGates; igate++) {
    
    float prevDbz = beam->dbz[igate-1];
    float dbz = beam->dbz[igate];
    
    if (prevDbz != CENSORED && dbz != CENSORED) {

      float dbzDiff = dbz - prevDbz;
      beam->dbzSpinFlag[igate] = 0;
    
      if (dbzDiff >= SPIN_THRESHOLD) {
        if (spinSense != 1) {
          spinSense = 1;
          beam->dbzSpinFlag[igate] = 1;
        }
      } else if (dbzDiff <= -SPIN_THRESHOLD) {
        if (spinSense != -1) {
          spinSense = -1;
          beam->dbzSpinFlag[igate] = 1;
        }
      }

    } /* if (beam->dbz[igate] != CENSORED ... */

  } /* igate */
  
  /* use gate 1 value for gate 0 */

  beam->dbzSpinFlag[0] = beam->dbzSpinFlag[1];

}


/*
 * Compute the SPIN as a percentage over the kernel
 * setting the value in the center beam
 * of the beam queue.
 */

void compute_spin() {

  Beam *centerBeam = beamQueue[NBEAMS_KERNEL/2];
  double sum = 0.0;
  double nn = 0.0;
  double mean = 0.0;
  int igate;
 
  for (igate = 0; igate < centerBeam->nGates; igate++) {
   
    int startGate, endGate;
    int jbeam, jgate;
    
    /*
     * compute the start and end index, checking to make sure we
     * do not go off either end
     */
    
    startGate = igate - NGATES_KERNEL/2;
    if (startGate < 0) {
      startGate = 0;
    }
    
    endGate = igate + NGATES_KERNEL/2;
    if (endGate > centerBeam->nGates - 1) {
      endGate = centerBeam->nGates - 1;
    }
  
    /*
     * iterate over the kernel
     * ignore censored gates
     */

    for (jbeam = 0; jbeam < NBEAMS_KERNEL; jbeam++) {
      for (jgate = startGate; jgate <= endGate; jgate++) {
        float spinFlag = beamQueue[jbeam]->dbzSpinFlag[jgate];
        if (spinFlag != CENSORED) {
          sum += spinFlag;
          nn += 1.0;
        }
      } /* jgate */
    } /* jbeam */
    
    if (nn > 0) {
      mean = sum / nn;
      centerBeam->spin[igate] = mean * 100.0;
    }
    
  } /* igate */

} /* compute_spin */



/*****************************
 * SDVE implementation example
 * Compute SDVE feature field
 */


/*
 * Compute SDVE
 * SDVE is the standard deviation of velocity over the kernel
 *
 * It is assumed that velocity has already been computed for the beam
 */

void compute_sdve() {

  Beam *centerBeam = beamQueue[NBEAMS_KERNEL/2];

  int igate;
  double sum = 0.0;
  double sumSq = 0.0;
  double nn = 0.0;
  double sdve = 0.0;
 
  for (igate = 0; igate < centerBeam->nGates; igate++) {
   
    int startGate, endGate;
    int jbeam, jgate;
    
    /*
     * compute the start and end index, checking to make sure we
     * do not go off either end
     */
    
    startGate = igate - NGATES_KERNEL/2;
    if (startGate < 0) {
      startGate = 0;
    }
    
    endGate = igate + NGATES_KERNEL/2;
    if (endGate > centerBeam->nGates - 1) {
      endGate = centerBeam->nGates - 1;
    }

    /*
     * iterate over the kernel
     * ignore censored gates
     */

    for (jbeam = 0; jbeam < NBEAMS_KERNEL; jbeam++) {
      for (jgate = startGate; jgate <= endGate; jgate++) {
        float vel = beamQueue[jbeam]->vel[jgate];
        if (vel != CENSORED) {
          sum += vel;
          sumSq += (vel * vel);
          nn += 1.0;
        }
     } /* jgate */
    } /* jbeam */
    
    if (nn >= 2) {
      double mean = sum / nn;
      double term1 = sumSq / nn;
      double term2 = mean * mean;
      if (term1 >= term2) {
        sdve = sqrt(term1 - term2);
      } else {
        sdve = 0.0;
      }
      centerBeam->sdve[igate] = sdve;
    }
    
  } /* igate */

} /* compute_sdve */
 

/*******************************
 * Ratios implemetation example
 * Compute ratioNarrow and ratioWide feature field
 */

/*
 * compute ratios for a single gate
 *
 * args in:
 *
 *   nSamples: number of samples in Beam
 *   powerSpectrum: power spectrum after applying an FFT to the IQ data.
 * 	            The first sample in the spectrum should be the DC point.
 *                  In other words,  clutter should be centered on
 *                  powerSpectrum[0].
 *
 * args out:
 *    ratioNarrow, ratioWide
 */

void compute_ratios(int nSamples,
                    const float *powerSpectrum, /* input */
                    float *ratioNarrow, /* output */
                    float *ratioWide) /* output */

{
     
  /* sum up powers in each of the 3 regions: */
  
  double sumInner = 0.0;
  double sumOuter = 0.0;
  double sumTotal = 0.0;
  int ii;
  
  for (ii = 0; ii < nSamples; ii++) {
    
    int jj = nSamples - 1 - ii;
    if (ii <= RATIO_LIMIT_INNER || jj <= RATIO_LIMIT_INNER) {
      sumInner += powerSpectrum[ii];
    }
       
    if (ii <= RATIO_LIMIT_OUTER || jj <= RATIO_LIMIT_OUTER) {
      sumOuter += powerSpectrum[ii];
    }
       
    sumTotal += powerSpectrum[ii];
    
  }
  
  *ratioNarrow = 10.0 * log10(sumInner / (sumOuter - sumInner));
  *ratioWide = 10.0 * log10(sumInner / (sumTotal - sumInner));
     
}

/********************************************
 * Clutter probability implementation example
 *
 * For the center beam in the queue:
 *    (a) compute clutter probability
 *    (b) set the clutter flag
 */

void compute_clut_prob() {

  Beam *centerBeam = beamQueue[NBEAMS_KERNEL/2];
  int igate;
  float tdbz, spin, sdve, ratioNarrow, ratioWide;
  double tdbzInterest, spinInterest, sdveInterest, ratioWideInterest;
  double tdbzWeight, spinWeight, sdveWeight, ratioWideWeight;
  double clutterProb;
  int clutterFlag;
  
  for (igate = 0; igate < centerBeam->nGates; igate++) {

    /* get the feature fields */
       
    tdbz = centerBeam->tdbz[igate];
    spin = centerBeam->spin[igate];
    sdve = centerBeam->sdve[igate];
    ratioNarrow = centerBeam->ratioNarrow[igate];
    ratioWide = centerBeam->ratioWide[igate];

    /* censor based on ratioNarrow */
    
    if (ratioNarrow < RATIO_NARROW_THRESHOLD) {
      centerBeam->clutterProb[igate] = CENSORED;
      centerBeam->clutterFlag[igate] = 0;
      continue;
    }
    
    /* convert feature fields to interest values */
    
    tdbzInterest = getInterest(tdbzMap, tdbz);
    spinInterest = getInterest(spinMap, spin);
    sdveInterest = getInterest(sdveMap, sdve);
    ratioWideInterest = getInterest(ratioWideMap, ratioWide);

    /* get weights for each field */
    
    tdbzWeight = getWeight(tdbzMap);
    spinWeight = getWeight(spinMap);
    sdveWeight = getWeight(sdveMap);
    ratioWideWeight = getWeight(ratioWideMap);

    /* compute weighted sum */

    double sum = 0.0;
    double sumWt = 0.0;
    
    sum =
      tdbzInterest * tdbzWeight +
      spinInterest * spinWeight +
      sdveInterest * sdveWeight +
      ratioWideInterest * ratioWideWeight;
    
    sumWt =
      tdbzWeight + spinWeight + sdveWeight + ratioWideWeight;
    
    clutterProb = sum / sumWt;
    
    if (clutterProb > PROB_THRESHOLD) {
      clutterFlag = 1;
    } else {
      clutterFlag = 0;
    }

    centerBeam->clutterProb[igate] = clutterProb;
    centerBeam->clutterFlag[igate] = clutterFlag;

  } /* igate */

} /* compute_clut_prob */

