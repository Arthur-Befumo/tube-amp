#ifndef TUBEAMP_H_
#define TUBEAMP_H_
#include <stdbool.h>

typedef struct Batch
{
	struct Batch *next;
	int length;
	double *data;
	double bounds;
} BATCH;

typedef struct CircuitState
{
	double U;
	double W[2];
	double X[2];
	double Y;
} CIRCUITSTATE;

typedef struct TubeConfig
{
	double Mu;
	double K_p;
	double K_vb;
	double K_g;
	double Ex;
	double V_lambda;
	double R_gk;
	double K;
	double baseline;
	bool simulateGridCurrent;
} TUBECONFIG;

typedef enum TubeName
{
	_12AX7,
	CUSTOM,
} TUBENAME;

CIRCUITSTATE *
process_buffer(BATCH *batch, TUBECONFIG *config, 
		CIRCUITSTATE *state, int sample_rate, int buffer_length);

TUBECONFIG *
set_config(TUBENAME tube_name, TUBECONFIG *config, bool grid_current);

BATCH *
rescale(BATCH *batch, double factor);

#endif
