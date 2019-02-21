
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sndfile.h>

#define BUFFER_LEN 1024

static void
print_info(SF_INFO *info) {
	printf("Sample Rate: %d\n"
		"Channels: %d\n", info->samplerate, info->channels);
}


SNDFILE *
read_file(const char *filename,SF_INFO *info)
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

int main(int argc, char **argv)
{	
	char *filename;
	if (argc > 1) {
		filename = argv[1];
		SF_INFO *info = malloc(sizeof(struct SF_INFO));
		read_file(filename, info);
	}
	return 0;
}