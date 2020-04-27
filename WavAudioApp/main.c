#include <stdio.h>
#include <string.h>


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
	struct WAVHEADER hnew;
	char* filepath = argv[1];
	char filenewpath[FILENAME_MAX];
	char* bits = argv[2];
	int newbitrate = atoi(bits);
	if (newbitrate % 8) {
		printf("\nBits not Correct");
		return 0;
	}
	int i = 0;
	while (filepath[i])
	{
		if (filepath[i] == '_')
			filepath[i] = ' ';
		i++;
	}
	strcpy(filenewpath, filepath);
	char* off = strrchr(filenewpath, '.');
	if (off != NULL) {
		*off = 0;
	}
	strcat(filenewpath, "NEW.wav");
	FILE* fil = fopen(filepath, "rb");
	FILE* filnew = fopen(filenewpath, "wb");
	if (fil == NULL) 
	{
		printf("\nFile: %s cannot be opened", filepath);
		return 0;
	}
	fseek(fil, 0, SEEK_END);
	int filesize = ftell(fil);
	fseek(fil, 0, SEEK_SET);
	int bytesread = fread(&h, 1, sizeof(h), fil);
	if (bytesread != sizeof(h)) {
		printf("\nCould not read file header.");
		fclose(fil);
		return 0;
	}
	hnew = h;
	hnew.BlockAlign = h.NumChannels * (newbitrate / 8);
	hnew.ByteRate = hnew.BlockAlign * h.SampleRate;
	hnew.BitsPerSample = newbitrate;
	fwrite(&hnew, 1, sizeof(hnew), filnew); //check return
	int increment = h.BlockAlign;
	unsigned char* buffer = malloc(increment);
	unsigned char* outbuffer = malloc(hnew.BlockAlign);
	
	for (int i = sizeof(h); i < filesize; i += increment) 
	{
		int bytesread = fread(buffer, 1, increment, fil);
		if (bytesread != increment) 
		{
			printf("\nCould not read WAV File Sample");
			fclose(fil);
			free(buffer);
			free(outbuffer);
			return 0;
		} 
		switch (h.BitsPerSample)
		{
			case 8:
				switch (newbitrate)
				{
					case 8:
						memcpy(outbuffer, buffer, increment);
						break;
					case 16:
						{
						    unsigned char* input = buffer;
							short* output = outbuffer;
							for (int a = 0; a < h.NumChannels; a++)
							{
								output[a] = (((int)input[a]) << 8) - 0x8000;
							}
						}
						break;
					case 32:
						{
							unsigned int* output = outbuffer;
							for (int a = 0; a < h.NumChannels; a++)
							{
								output[a] = buffer[a];
								output[a] <<= 24;

							}
						}
						break;
					default:
						printf("Not Supported for Input");
						fclose(fil);
						free(buffer);
						free(outbuffer);
						return 0;
				}
				break;
			case 16:
				switch (newbitrate)
				{
					case 8:
						{
						    short* input = buffer;
						    unsigned char* output = outbuffer;
							for (int a = 0; a < h.NumChannels; a++)
							{
								output[a] = ((int)input[a] + 0x8000) >> 8;
							}
						}
						break;
					case 16: 
						memcpy(outbuffer, buffer, increment);
						break;
					case 32:
						{
							unsigned int* output = outbuffer;
							unsigned short* input = buffer;
							for (int a = 0; a < h.NumChannels; a++)
							{
								output[a] = buffer[a];
								output[a] <<= 16;
							}
						}
						break;
				default:
					printf("Not Supported for Input");
					fclose(fil);
					free(buffer);
					free(outbuffer);
					return 0;
				}
				break;
			case 32:
				switch (newbitrate)
				{
					case 8:
						{
							unsigned char* output = outbuffer;
							for (int a = 0; a < h.NumChannels; a++)
							{
								output[a] = buffer[a];
								output[a] >>= 24;
							}
						}
						break;
					case 16:
						{
							unsigned char* output = outbuffer;
							unsigned short* input = buffer;
							for (int a = 0; a < h.NumChannels; a++)
							{
								output[a] = buffer[a];
								output[a] >>= 16;
							}
						}
						break;
					case 32:
						memcpy(outbuffer, buffer, increment);
						break;
					default:
						printf("Not Supported for Input");
						fclose(fil);
						free(buffer);
						free(outbuffer);
						return 0;
				}
				break;
			default:
				printf("Not Supported for Input");
				fclose(fil);
				free(buffer);
				free(outbuffer);
				return 0;
		}
		fwrite(outbuffer, 1, hnew.BlockAlign, filnew);
	}
	free(buffer);
	free(outbuffer);
	fclose(fil);
	fclose(filnew);
	return 0;
}