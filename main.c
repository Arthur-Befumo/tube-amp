
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sndfile.h>

#define BATCH_SIZE ((sf_count_t) 10000)

typedef struct node {
	struct node *next;
	double *data;
	sf_count_t length;
} NODE;

static void
print_info(SF_INFO *info) {
	printf("Sample Rate: %d\n"
		"Channels: %d\n"
		"Frames: %ld\n", info->samplerate, info->channels, info->frames);
}

NODE *
push_node(NODE *current_node, double *ptr, sf_count_t length) {
	NODE *new_node = malloc(sizeof(NODE));
	new_node->next = current_node;
	new_node->data = ptr;
	new_node->length = length;
	return new_node;
}

NODE *
pop_node(NODE *current_node) {
	NODE * next_node = current_node->next;
	free(current_node->data);
	free(current_node);
	return next_node;
}

static void
reverse_buffer(double *ptr, sf_count_t length) {
	int a = 0;
	int b = (int) length;

	while (a < b) {
		double temp = ptr[a];
		ptr[a] = ptr[b];
		ptr[b] = temp;
		a++;
		b--;
	}
}

SNDFILE *
open_file(const char *filename, SF_INFO *info)
{
	SNDFILE *file = sf_open(filename, SFM_READ, info);
	if (file == NULL) {
		fprintf(stderr, "Failed to open file '%s'", filename);
	}
	else {
		printf("Opened file: '%s'\n", filename);
		print_info(info);
	}
	return file;
}

SNDFILE *
create_file(const char *filename, SF_INFO *info)
{
	SNDFILE *file = sf_open(filename, SFM_WRITE, info);
	return file;
}

int main(int argc, char **argv)
{	
	if (argc == 3) {
		char *filename = argv[1];
		char *newname = argv[2];
		SF_INFO *info = malloc(sizeof(struct SF_INFO));
		SNDFILE *file = open_file(filename, info);
		SNDFILE *newfile = create_file(newname, info);
		double *ptr = malloc(sizeof(double) * info->channels * BATCH_SIZE);
		sf_count_t frames_read;
		NODE *stack = malloc(sizeof(NODE));

		while((frames_read = sf_read_double(file, ptr, BATCH_SIZE)) > 0) {
			printf("%ld frames requested, %ld frames read...\n", BATCH_SIZE, frames_read);
			stack = push_node(stack, ptr, frames_read);
			ptr = malloc(sizeof(double) * info->channels * BATCH_SIZE);
		}

		

		while(stack->next != NULL) {
			reverse_buffer(stack->data, stack->length);
			sf_writef_double(newfile, stack->data, stack->length);
			stack = pop_node(stack);
		}

		printf("New file info:\n");
		print_info(info);

		free(info);
		free(ptr);
		free(stack);

		if (sf_close(file) == 0) {
			printf("File %s closed\n", filename);
		} else {
			fprintf(stderr, "Error closing file\n");
		}

		if (sf_close(newfile) == 0) {
			printf("File new_beep.wav closed\n");
		} else {
			fprintf(stderr, "Error closing file\n");
		}
	}
	return 0;
}