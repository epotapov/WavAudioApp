#include <stdio.h>
#include <string.h>


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


int ConversionAlgo(char* buffer, char* outbuffer, int increment, FILE* fil, int newbitrate, struct WAVHEADER h) //Conversion Algorithm for swapping bit values
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
			unsigned char* input = buffer;
			int* output = (int*)outbuffer;
			for (int a = 0; a < h.NumChannels; a++)
			{
				output[a] = ((int)input[a] - 0x80) << 24;

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
			int* output = (int*)outbuffer;
			short* input = (short*)buffer;
			for (int a = 0; a < h.NumChannels; a++)
			{
				output[a] = input[a];
				output[a] <<= 16;
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
			memcpy(outbuffer, buffer, increment);
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
	int newbitrate = atoi(bits);
	if (newbitrate % 8) {
		printf("Bits not Correct");
		return 0;
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
		return 0;
	}
	fseek(fil, 0, SEEK_END);
	int filesize = ftell(fil);
	fseek(fil, 0, SEEK_SET);
	size_t bytesread = fread(&h, 1, sizeof(h), fil);
	if (bytesread != sizeof(h)) {
		printf("Could not read file header.");
		fclose(fil);
		return 0;
	}
	hnew = h;
	hnew.BlockAlign = h.NumChannels * (newbitrate / 8);
	hnew.ByteRate = hnew.BlockAlign * h.SampleRate;
	hnew.BitsPerSample = newbitrate;
	hnew.Subchunk2Size = h.Subchunk2Size / h.BlockAlign * hnew.BlockAlign;
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
			fclose(filnew);
			fclose(fil);
			remove(filenewpath);
			free(buffer);
			free(outbuffer);
			return 0;
		} 
		if (ConversionAlgo(buffer, outbuffer, increment, fil, newbitrate, h) == -1) {
			printf("Couln't convert WAV Sample");
			fclose(filnew);
			fclose(fil);
			remove(filenewpath);
			free(buffer);
			free(outbuffer);
			return 0;
		}
		fwrite(outbuffer, 1, hnew.BlockAlign, filnew);
	}
	printf("Successfully Rewrote the File");
	free(buffer);
	free(outbuffer);
	fclose(fil);
	fclose(filnew);
	return 0;
}