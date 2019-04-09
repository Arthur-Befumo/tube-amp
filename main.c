
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
	}
	else {
		printf("Opened file: '%s'\n", filename);
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
scale_and_write(double bound, BATCH *head, SF_INFO *info)
{
	double factor = 1 / bound;
	BATCH *ptr = rescale(head, factor);
	SNDFILE *new_file = create_file("new_file.wav", info);

	
	while((ptr = ptr->next) != NULL)
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
	if (argc !=2)
	{
		fprintf(stderr, "Incorrect usage\n");
		exit(1);
	}

	TUBECONFIG *config;
	BATCH *head;
	BATCH *ptr;
	SF_INFO *info;
	SNDFILE *file;
	CIRCUITSTATE *state = NULL;
	double bound = 0;

	info = calloc(1, sizeof(SF_INFO));
	file = open_file(argv[1], info);

	head = calloc(1,sizeof(BATCH));
	head->next = NULL;
	head->length = 0;
	ptr = head;
	config = calloc(1, sizeof(TUBECONFIG));
	config = set_config(_12AX7, config, false);

	while(true)
	{
		ptr->next = new_batch(BATCH_SIZE);
		ptr = ptr->next;
		sf_count_t frames_read = sf_read_double(file, ptr->data, BATCH_SIZE);
		state = process_buffer(ptr, config, state, (int) info->samplerate, (int) BATCH_SIZE);
		ptr->length = (int) frames_read;
		bound = fmax(ptr->bounds, bound);

		if (frames_read < BATCH_SIZE)
		{
			ptr->next = NULL;
			break;
		}
		
	}

	scale_and_write(bound, head, info);

	free(state);
	free_batches(head);
	free(config);
	sf_close(file);
	free(info);

	return 0;
}
