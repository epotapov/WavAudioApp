#include <stdio.h>


struct WAVHEADER 
{
	char ChunkID[4]; //RIFF
	unsigned int ChunkSize;
	char Format[4]; //WAVE
	char Subchunk1ID[4];
	unsigned int Subchunk1Size;
	unsigned short AudioFormat;
	unsigned short NumChannels;
	unsigned int SampleRate;
	unsigned int ByteRate;
	unsigned short BlockAlign;
	unsigned short BitsPerSample;
	char Subchunk2ID[4];
	unsigned int Subchunk2Size;
};

int main(int argc, char** argv)
{
	struct WAVHEADER h;
	char* filepath = argv[1];

	int i = 0;
	while (filepath[i])
	{
		if (filepath[i] == '_')
			filepath[i] = ' ';
		i++;
	}
	FILE* fil = fopen(filepath, "r");
	if (fil == NULL) 
	{
		printf("File: %s cannot be opened", filepath);
		return 0;
	}
	fseek(fil, 0, SEEK_END);
	int filesize = ftell(fil);
	fseek(fil, 0, SEEK_SET);
	int bytesread = fread(&h, 1, sizeof(h), fil);
	if (bytesread != sizeof(h)) {
		printf("Could not read file header.");
		fclose(fil);
	}

	fclose(fil);
	return 0;
}