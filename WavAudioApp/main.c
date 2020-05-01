#include <stdio.h>
#include <string.h>

#define CLOSE fclose(filnew);\
fclose(fil);\
remove(filenewpath);

#define CLOSE2 fclose(filnew); \
fclose(fil); \
remove(filenewpath); \
free(buffer); \
free(outbuffer);

#define CLOSE3 free(buffer);\
free(outbuffer);\
fclose(fil);\
fclose(filnew);

//TODO: float to everything else and check number of command line arguments

struct WAVHEADER //Abstraction of The WavHeader to Be used when writing to new Header
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


int ConversionAlgo(char* buffer, char* outbuffer, int increment, FILE* fil, int newbitrate, struct WAVHEADER h, int boolfloat) //Conversion Algorithm for swapping bit values
{
	switch (h.BitsPerSample) //bit of sample of original file
	{
	case 8: //case that the original file is 8 bit
		switch (newbitrate)
		{
		case 8: 
			memcpy(outbuffer, buffer, increment);
			break;
		case 16: 
		{
			unsigned char* input = buffer;
			short* output = (short*)outbuffer;
			for (int a = 0; a < h.NumChannels; a++)
			{
				output[a] = (((int)input[a]) << 8) - 0x8000;
			}
		}
		break;
		case 32:
		{
			if (!boolfloat) 
			{
				unsigned char* input = buffer;
				int* output = (int*)outbuffer;
				for (int a = 0; a < h.NumChannels; a++)
				{
					output[a] = ((int)input[a] - 0x80) << 24;

				}
			}
			else 
			{	
				unsigned char* input = buffer;
				float* output = (float*)outbuffer;
				for (int a = 0; a < h.NumChannels; a++)
				{
					output[a] = input[a];
					output[a] -= 0x80;
					output[a] /= 0x80;

				}
			}
		}
		break;
		default:
			return -1;
		}
		break;
	case 16: //case that the original file is 16 bit
		switch (newbitrate)
		{
		case 8: 
		{
			short* input = (short*)buffer;
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
			if (!boolfloat) 
			{
				int* output = (int*)outbuffer;
				short* input = (short*)buffer;
				for (int a = 0; a < h.NumChannels; a++)
				{
					output[a] = input[a];
					output[a] <<= 16;
				}
			}
			else 
			{
				float* output = (float*)outbuffer;
				short* input = (short*)buffer;
				for (int a = 0; a < h.NumChannels; a++)
				{
					output[a] = input[a];
					output[a] -= 0x8000;
					output[a] /= 0x8000;
				}
			}
		}
		break;
		default:
			return -1;
		}
		break;
	case 32: //case that the original file is 32 bit
		switch (newbitrate)
		{
		case 8:
		{
			int* input = (int*)buffer;
			unsigned char* output = outbuffer;
			for (int a = 0; a < h.NumChannels; a++)
			{
				output[a] = ((int)input[a] + 0x80000000) >> 24;

			}
		}
		break;
		case 16: 
		{
			short* output = (short*)outbuffer;
			int* input = (int*)buffer;
			for (int a = 0; a < h.NumChannels; a++)
			{
				input[a] >>= 16;
				output[a] = input[a];
			}
		}
		break;
		case 32: 
			if (!boolfloat)
			{
				memcpy(outbuffer, buffer, increment);
			}
			else
			{
				float* output = (float*)outbuffer;
				int* input = (int*)buffer;
				for (int a = 0; a < h.NumChannels; a++)
				{
					output[a] = input[a];
					output[a] -= 0x80000000;
					output[a] /= 0x80000000;
				}
			}
			break;
		default:
			return -1;
		}
		break;
	default:
		return -1;
	}
	return 0;
}

int main(int argc, char** argv)
{
	struct WAVHEADER h;
	struct WAVHEADER hnew;
	char* filepath = argv[1];
	char filenewpath[FILENAME_MAX];
	char* bits = argv[2];
	int newbitrate;
	int boolfloat;
	if (strcmp(bits, "fl") != 0) 
	{
		newbitrate = atoi(bits);
		boolfloat = 0;
		if (newbitrate % 8 || newbitrate <= 0 || newbitrate > 32) {
			printf("Bits not Correct");
			return 0;
		}
	} 
	else 
	{
		newbitrate = 32;
		boolfloat = 1;
	}
	int i = 0;
	while (filepath[i]) //checks for spaces in file direction and modifies for use
	{
		if (filepath[i] == '_')
			filepath[i] = ' ';
		i++;
	}
	strcpy_s(filenewpath, FILENAME_MAX, filepath);
	char* off = strrchr(filenewpath, '.');
	if (off != NULL) {
		*off = 0;
	}
	strcat_s(filenewpath, FILENAME_MAX,"NEW.wav");
	FILE* fil = NULL;
	if (fopen_s(&fil, filepath, "rb") != 0)
	{
		printf("Input File: %s cannot be opened", filepath);
		return 0;
	}
	FILE* filnew = NULL;
	if (fopen_s(&filnew, filenewpath, "wb") != 0)
	{
		printf("Output File: %s cannot be opened", filenewpath);
		fclose(fil);
		return 0;
	}
	fseek(fil, 0, SEEK_END);
	int filesize = ftell(fil);
	if (filesize < sizeof(h)) 
	{
		printf("Not a good WAV File");
		CLOSE
		return 0;
	}
	fseek(fil, 0, SEEK_SET);
	size_t bytesread = fread(&h, 1, sizeof(h), fil);
	if (bytesread != sizeof(h)) {
		printf("Could not read file header.");
		CLOSE
		return 0;
	}
	if (memcmp(h.ChunkID, "RIFF", 4) != 0 || memcmp(h.Format, "WAVE", 4) != 0) 
	{
		printf("Invalid WAV file");\
		CLOSE
		return 0;
	}
	hnew = h;
	hnew.BlockAlign = h.NumChannels * (newbitrate / 8);
	hnew.ByteRate = hnew.BlockAlign * h.SampleRate;
	hnew.BitsPerSample = newbitrate;
	hnew.Subchunk2Size = h.Subchunk2Size / h.BlockAlign * hnew.BlockAlign;
	if (boolfloat) 
	{
		hnew.AudioFormat = 3;
	}
	fwrite(&hnew, 1, sizeof(hnew), filnew); 
	int increment = h.BlockAlign;
	unsigned char* buffer = malloc(increment);
	unsigned char* outbuffer = malloc(hnew.BlockAlign);
	
	for (int i = sizeof(h); i < filesize; i += increment) 
	{
		int bytesread = fread(buffer, 1, increment, fil); //issue in this location 
		if (bytesread != increment) 
		{
			printf("Could not read WAV File Sample");
			CLOSE2
			return 0;
		} 
		if (ConversionAlgo(buffer, outbuffer, increment, fil, newbitrate, h, boolfloat) == -1) {
			printf("Couln't convert WAV Sample");
			CLOSE2
			return 0;
		}
		fwrite(outbuffer, 1, hnew.BlockAlign, filnew);
	}
	printf("Successfully Rewrote the File");
	CLOSE3
	return 0;
}