
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sndfile.h>
#include <math.h>
#include "tubeamp.h"

#define BATCH_SIZE ((sf_count_t) 10000)

SNDFILE *
open_file(const char *filename, SF_INFO *info)
{
	SNDFILE *file = sf_open(filename, SFM_READ, info);
	if (file == NULL) {
		fprintf(stderr, "Failed to open file '%s'\n", filename);
		exit(1);
	}
	return file;
}

SNDFILE *
create_file(const char *filename, SF_INFO *info)
{
	SNDFILE *file = sf_open(filename, SFM_WRITE, info);
	return file;
}

void
print_config(TUBECONFIG * config)
{
	printf("\
	CONFIG:\
	Mu: %f\n\
	K_p: %f\n\
	K_vb: %f\n\
	K_g: %f\n\
	E_x: %f\n\
	V_lambda: %f\n\
	R_gk: %f\n\
	K: %f\n\
	Baseline: %f\n",
		config->Mu,
		config->K_p,
		config->K_vb,
		config->K_g,
		config->Ex,
		config->V_lambda,
		config->R_gk,
		config->K,
		config->baseline);
}

void
free_batches(BATCH *ptr)
{
	do
	{
		BATCH *temp = ptr;
		ptr = ptr->next;
		free(temp->data);
		free(temp);
	} while (ptr != NULL);
}

void
scale_and_write(double factor, BATCH *head, SF_INFO *info, char *filename)
{
	BATCH *ptr = rescale(head, factor);
	SNDFILE *new_file = create_file(filename, info);

	
	while ((ptr = ptr->next) != NULL)
	{
		sf_write_double(new_file, ptr->data, (sf_count_t) ptr->length);
	}

	sf_close(new_file);
}

BATCH *
new_batch(sf_count_t size)
{
	BATCH *ptr = (BATCH *) malloc(sizeof(BATCH));
	ptr->data = calloc(size, sizeof(double));
	ptr->next = NULL;
	ptr->bounds = 0;
	return ptr;
}

int main(int argc, char **argv)
{	
	double input_gain = 0;
	double output_scale = 0;
	char * input_file = NULL;
	char * output_file = NULL;
	bool debug = false;

	if (argc > 1)
	{
		input_file = argv[1];
	}
	if (argc > 2)
	{
		output_file = argv[2];
	}

	for (int arg = 3; arg < argc; arg++)
	{
		if (!strcmp(argv[arg], "-g"))
		{
			if (arg == argc - 1)
			{
				fprintf(stderr, "Error: Missing argument: -g [input gain]\n");
				exit(0);
			}
			else if (!(input_gain = (double) atof(argv[++arg])))
			{
				fprintf(stderr, "Error: Invalid argument for -g [input gain]: %s\n", argv[arg]);
				exit(0);
			}
		}
		else if (!strcmp(argv[arg], "-s"))
		{
			if (arg == argc - 1)
			{
				fprintf(stderr, "Error: Missing argument: -g [input gain]\n");
				exit(0);
			}
			else if (!(output_scale = (double) atof(argv[++arg])) || output_scale > 1 || output_scale < -1)
			{
				fprintf(stderr, "Error: Invalid argument for -s [output scale]: %s\n", argv[arg]);
				exit(0);
			}
		}
		else if (!strcmp(argv[arg], "-debug"))
		{
			debug = true;
		}
		else
		{
			fprintf(stderr, "Error: Invalid flag: %s\n", argv[arg]);
			exit(0);
		}
	}

	bool is_error = false;

	if (input_file == NULL)
	{
		fprintf(stderr, "Error: Input file not set\n");
		is_error = true;
	}
	if (output_file == NULL)
	{
		fprintf(stderr, "Error: Output file is not set\n");
		is_error = true;
	}
	if (is_error)
	{ 
		fprintf(stderr, "Usage: %s input_file output_file [-g input_gain] [-s output_scale] [-debug]\n", argv[0]);
		exit(1);
	}
	
	if (!input_gain)
	{
		input_gain = 1;
		if (debug)
		{
			printf("debug: input_gain not specified, defaulting to 1\n");
		}
	}

	if (!output_scale)
	{
		output_scale = 1;
		if (debug)
		{
			printf("debug: output_scale not specified, defaulting to 1\n");
		}
	}


	TUBECONFIG *config;
	BATCH *head;
	BATCH *ptr;
	SF_INFO *info;
	SNDFILE *file;
	CIRCUITSTATE *state = NULL;
	double bound = 0;

	info = calloc(1, sizeof(SF_INFO));
	file = open_file(input_file, info);

	if (debug)
	{
		printf("debug: file %s opened, samplerate: %d\n", input_file, (int) info->samplerate);
	}

	head = calloc(1,sizeof(BATCH));
	head->next = NULL;
	head->length = 0;
	ptr = head;
	config = calloc(1, sizeof(TUBECONFIG));
	config = set_config(_12AX7, config, false);

	while (true)
	{
		ptr->next = new_batch(BATCH_SIZE);
		ptr = ptr->next;
		sf_count_t frames_read = sf_read_double(file, ptr->data, BATCH_SIZE);
		ptr->length = (int) frames_read;
		state = process_buffer(ptr, config, state, (int) info->samplerate, ptr->length, input_gain);
		
		bound = fmax(ptr->bounds, bound);

		if (debug)
		{
			printf("debug: local batch processed:\nsamples read: %d\nmax (absolute) output: %f\n", (int) frames_read, bound);
		}

		if (frames_read < BATCH_SIZE)
		{
			ptr->next = NULL;
			break;
		}
		
	}

	scale_and_write((output_scale / bound), head, info, output_file);

	if (debug)
	{
		printf("debug: write successful\n");
	}

	free(state);
	free_batches(head);
	free(config);
	sf_close(file);
	free(info);

	return 0;
}
