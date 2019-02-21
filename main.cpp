
#include <stdio.h>
#include <string.h>
#include <sndfile.h>

#define BUFFER_LEN 1024

static void
read_file (const char *filename) 
{
	static short buffer[BUFFER_LEN];
	SndfileHandle file = SndfileHandle(filename);

	printf("Opened file '%s'\n\
		Sample rate: %d\n\
		Channels: %d\n", filename, file.samplerate(), file.channels());

	file.read(buffer, BUFFER_LEN);

	puts("");
}

int main(int argc, char **argv)
{	
	char *filename;
	if (argc > 1) {
		filename = argv[1];
		read_file(filename);
	}
	return 0;
}