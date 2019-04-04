#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "tubeamp.h"

/* Definition of variables for circuit equations */

#define R_in 20000
#define R_p 100000
#define R_g 220000
#define R_o 22000
#define R_k 2700
#define V_bias 300
#define C_o 20
#define C_k 10000

void print_state(CIRCUITSTATE *state)
{
	printf("\
	U: %f\n\
	W: [%f, %f]\n\
	X: [%f, %f]\n\
	Y: %f\n",
	state->U,
	state->W[0],
	state->W[1],
	state->X[0],
	state->X[1],
	state->Y);
}

double
plate_current(CIRCUITSTATE *state, TUBECONFIG *config)
{
	double V_pk, V_gk, I_p, E;

	V_pk = state->W[0] - state->X[0];
	V_gk = state->W[1] - state->X[0];
	E = (V_pk / config->K_p)
		* log(1 + exp(config->K_p
		* (1 / config->Mu + V_gk / sqrt(config->K_vb + pow(V_pk, 2)))));

	I_p = 0;
	if (E > 0)
	{
		I_p = 2 * (pow(E, config->Ex) / config->K_g);
	}

	return I_p;
}
	
double
grid_current(CIRCUITSTATE *state, TUBECONFIG *config)
{
	if (!config->simulateGridCurrent) {
		return 0;
	}

	double V_gk, I_g, a, b, c;
	
	V_gk = state->W[1] - state->X[0];

	if (V_gk > config->V_lambda + config->K) 
	{
		I_g = (V_gk - config->V_lambda) / config->R_gk;
	}
	else if (V_gk > config->V_lambda - config->K)
	{
		a = 1 / (4 * config->K * config->R_gk);
		b = (config->K - config->V_lambda)
			/ (2 * config->K * config->R_gk);
		c = -a * pow((config->V_lambda - config->K), 2)
			- b * (config->V_lambda - config->K);
		I_g = a * pow(V_gk, 2) + b * V_gk + c;
	}
	else
	{
		I_g = 0;
	}

	return I_g;
}

void
dX_dt(CIRCUITSTATE *state, TUBECONFIG *config, double *buf)
{
	double I_p, I_g;

	I_p = plate_current(state, config);
	I_g = grid_current(state, config);

	buf[0] = (-state->X[0] / R_k + I_g + I_p) / C_k;
	buf[1] = -(state->X[1] + state->W[0]) / (R_o * C_o);
}

CIRCUITSTATE *
update_X(CIRCUITSTATE *state, TUBECONFIG *config, int sample_rate)
{
	double buf[2];
	dX_dt(state, config, (double *) &buf);

	double X[2] = {state->X[0], state->X[1]};

	state->X[0] = X[0] + (1 / 2) * (1 / (float) sample_rate) * buf[0];
	state->X[1] = X[1] + (1 / 2) * (1 / (float) sample_rate) * buf[1];

	dX_dt(state, config, (double *) &buf);

	state->X[0] = X[0] + (1 / (float) sample_rate) * buf[0];
	state->X[1] = X[1] + (1 / (float) sample_rate) * buf[1];

	return state;
}

CIRCUITSTATE *
update_W(CIRCUITSTATE *state, TUBECONFIG *config, int iters)
{
	double I_p, I_g, g1, g2, dg1, dg2;

	dg1 = 1 + R_p/R_o;
	dg2 = 1;

	for (int i = 0; i < iters; i++)
	{
		I_p = plate_current(state, config);
		I_g = grid_current(state, config);

		g1 = state->W[0]
			+ R_p * (I_p + (state->X[1] + state->W[0]) / R_o) - V_bias;
		g2 = state->W[1]
			- state->U + R_g * I_g;

		state->W[0] = state->W[0] - g1 / dg1;
		state->W[1] = state->W[1] - g2 / dg2;
	}

	return state;
}

CIRCUITSTATE *
update_Y(CIRCUITSTATE *state)
{
	state->Y = state->X[1] + state->W[0];
	return state;
}

CIRCUITSTATE *
update_state(CIRCUITSTATE *state, TUBECONFIG *config, int sample_rate)
{
	state = update_X(state, config, sample_rate);
	state = update_W(state, config, 4);
	state = update_Y(state);

	return state;
}

CIRCUITSTATE *
process_buffer(BATCH *batch, TUBECONFIG *config, CIRCUITSTATE *state, int sample_rate, int buffer_length)
{
	double bounds = 0;

	if (state == NULL)
	{
		state = calloc(1, sizeof(CIRCUITSTATE));
	}

	for (int i = 0; i < buffer_length; i++)
	{
		state->U = batch->data[i];
		state = update_state(state, config, sample_rate);
		batch->data[i] = state->Y - config->baseline;
		bounds = fmax(fabs(batch->data[i]), bounds);
	}

	free(state);

	return state;
}

double
get_baseline(TUBECONFIG *config)
{
	BATCH *batch;
	double baseline;

	batch = malloc(sizeof(BATCH));
	batch->data = (double *) calloc(100, sizeof(double));
	batch->next = NULL;
	process_buffer(batch, config, NULL, 44100, 100);
	
	baseline = batch->data[99];
	free(batch->data);
	free(batch);

	return baseline;
}

TUBECONFIG *
set_config(TUBENAME tube_name, TUBECONFIG *config, bool grid_current)
{

	if (tube_name == _12AX7) 
	{
		config->Mu = 100;
		config->K_p = 600;
		config->K_vb = 300;
		config->K_g = 1060;
		config->Ex = 1.4;
		config->V_lambda = 0.35;
		config->R_gk = 1300;
		config->K = 0.5;
	} 
	else if (tube_name == CUSTOM)
	{
		if (config->Mu <= 0
			|| config->K_p <= 0
			|| config->K_vb <= 0
			|| config->Ex <= 0
			|| config->R_gk <= 0
			|| config-> K < 0)
		{
			fprintf(stderr, "Error: invalid CUSTOM config.\n");
			return NULL;
		}

	}
	else
	{
		fprintf(stderr, "Error: invalid TUBENAME.\n");
		return NULL;
	}
	config->simulateGridCurrent = grid_current;
	config->baseline = get_baseline(config);
	return config;
}

BATCH *
rescale(BATCH *batch, double factor)
{
	if (batch->next != NULL)
	{
		batch->next = rescale(batch->next, factor);
	}

	for (int i = 0; i < batch->length; i++)
	{
		batch->data[i] = batch->data[i] * factor;
	}

	return batch;
}
