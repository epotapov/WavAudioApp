#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//macros used to close files and free memory
#define CLOSE fclose(filnew);\
fclose(fil);\
remove(filenewpath);

#define CLOSE2 fclose(filnew);\
fclose(fil);\
remove(filenewpath);\
free(buffer);\
free(outbuffer);

#define CLOSE3 free(buffer);\
free(outbuffer);\
fclose(fil);\
fclose(filnew);


struct WAVHEADER //Abstraction of The WavHeader to Be used when writing to new Header and accessing the current file
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

struct RIFFHEADER
{
	char ChunkID[4]; 
	unsigned int ChunkSize;
	char Format[4]; //WAVE
};

struct CHUNK
{
	char ChunkID[4];
	unsigned int ChunkSize;
};

struct FMTHEADER
{
	unsigned short AudioFormat;
	unsigned short NumChannels;
	unsigned int SampleRate;
	unsigned int ByteRate;
	unsigned short BlockAlign;
	unsigned short BitsPerSample;
};

int ConversionAlgo(char* buffer, char* outbuffer, int increment, FILE* fil, int newbitrate, struct WAVHEADER h, int boolfloat, int originfloat) //Conversion Algorithm for swapping bit values
{
	switch (h.BitsPerSample) //bit of sample of original file
	{
	case 8: //case that the original file is 8 bit
		switch (newbitrate) //the bits you want to convert to
		{
		case 8: 
			memcpy(outbuffer, buffer, increment); //copies exactly 
			break;
		case 16: 
		{
			unsigned char* input = buffer;
			short* output = (short*)outbuffer;
			for (int a = 0; a < h.NumChannels; a++) //loops for each channel in the file(Stereo or mono)
			{
				output[a] = (((int)input[a]) << 8) - 0x8000;  //shifts then makes it signed. does this with hex values
			}
		}
		break;
		case 32:
		{
			if (!boolfloat) //the first case is if you want to convert to int 
			{
				unsigned char* input = buffer;
				int* output = (int*)outbuffer;
				for (int a = 0; a < h.NumChannels; a++)
				{
					output[a] = ((int)input[a] - 0x80) << 24; //makes signed then shifts

				}
			}
			else //if you want to convert to float
			{	
				unsigned char* input = buffer;
				float* output = (float*)outbuffer;
				for (int a = 0; a < h.NumChannels; a++)
				{
					output[a] = input[a];
					output[a] -= 0x80; //makes signed
					output[a] /= 0x80; //divides for new scale
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
				output[a] = ((int)input[a] + 0x8000) >> 8; //makes unsigned then shifts 
			}
		}
		break;
		case 16: 
			memcpy(outbuffer, buffer, increment); //direct copy
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
					output[a] <<= 16;  //shifts up/left 16
				}
			}
			else 
			{
				float* output = (float*)outbuffer;
				short* input = (short*)buffer;
				for (int a = 0; a < h.NumChannels; a++)
				{
					output[a] = input[a];
					output[a] /= 0x8000;  //divides for new scale
				}
			}
		}
		break;
		default:
			return -1;
		}
		break;
	case 32: //case that the original file is 32 bit
		if (!originfloat) //checks if the original was a float 
		{
			switch (newbitrate) //cases to convert to regular int
			{
			case 8:
			{
				int* input = (int*)buffer;
				unsigned char* output = outbuffer;
				for (int a = 0; a < h.NumChannels; a++)
				{
					output[a] = ((int)input[a] + 0x80000000) >> 24; //makes unsigned and shifts to the right 24 bits

				}
			}
			break;
			case 16:
			{
				short* output = (short*)outbuffer;
				int* input = (int*)buffer;
				for (int a = 0; a < h.NumChannels; a++)
				{
					input[a] >>= 16; //shifts 
					output[a] = input[a];
				}
			}
			break;
			case 32:
				if (!boolfloat)
				{
					memcpy(outbuffer, buffer, increment); //copy
				}
				else
				{
					float* output = (float*)outbuffer;
					int* input = (int*)buffer;
					for (int a = 0; a < h.NumChannels; a++)
					{
						output[a] = (float)input[a];
						output[a] /= 0x80000000; //divides for new scale
					}
				}
				break;
			default:
				return -1;
			}
			break;
		}
		else 
		{
			switch (newbitrate) //cases to convert to float
			{
			case 8:
			{ 
				float* input = (float*)buffer;
				unsigned char* output = outbuffer;
				for (int a = 0; a < h.NumChannels; a++)
				{
					input[a] *= 0x80; //reverse for regular scales
					input[a] += 0x80;
					output[a] = (unsigned char)input[a];
				}
			}
			break;
			case 16:
			{
				short* output = (short*)outbuffer;
				float* input = (float*)buffer;
				for (int a = 0; a < h.NumChannels; a++)
				{
					input[a] *= 0x8000; //reverse 
					output[a] = (short)input[a];
				}
			}
			break;
			case 32:
				if (!boolfloat)
				{
					int* output = (int*)outbuffer;
					float* input = (float*)buffer;
					for (int a = 0; a < h.NumChannels; a++)
					{
						input[a] *= 0x800000; //reverse
						output[a] = (int)input[a]; 
					}
				}
				else
				{
					memcpy(outbuffer, buffer, increment); //copy
				}
				break;
			default:
				return -1;
			}
			break;
		}
	default:
		return -1; //returns -1 if the algorithm couldn't convert 
	}
	return 0;
}

int main(int argc, char** argv)
{
	struct RIFFHEADER riffheader; //header of riff
	struct CHUNK chunk; //chunk
	struct FMTHEADER fmt; //fmt
	struct WAVHEADER hnew; //header of new file
	char* filepath = argv[1];
	char filenewpath[FILENAME_MAX];
	char* bits = argv[2];
	int newbitrate;
	int boolfloat;
	if (argc != 3) { //checks to make sure theres correct number of arguments 
		printf("You need 2 arguments for the Audio Converter to work"); //sample arguement: C:\Users\Edward_Potapov\Desktop\Audiostuff\Ring08.wav 16
		printf("\nArgument 1: File Path Argument: Sample Size");
		printf("\nSupported Sample sizes: 8, 16, 32 and fl(float)\n");
		return 0;
	}
	if (strcmp(bits, "fl") != 0) //if you want 8, 16, or 32
	{
		newbitrate = atoi(bits);
		boolfloat = 0;
		if (newbitrate % 8 || newbitrate <= 0 || newbitrate > 32 || newbitrate == 24) { //makes sure its not a wrong sample size
			printf("Bits not Correct\n");
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
	if (off != NULL) { //terminates the end of the new file for NEW.wav to be appended
		*off = 0;
	}
	strcat_s(filenewpath, FILENAME_MAX,"NEW.wav"); //appends NEW.wav
	FILE* fil = NULL;
	if (fopen_s(&fil, filepath, "rb") != 0) //opens given file
	{
		printf("Input File: %s cannot be opened\n", filepath);
		return 0;
	}
	FILE* filnew = NULL; 
	if (fopen_s(&filnew, filenewpath, "wb") != 0) //opens new file
	{
		printf("Output File: %s cannot be opened\n", filenewpath);
		fclose(fil);
		return 0;
	}
	fseek(fil, 0, SEEK_END);
	int filesize = ftell(fil);
	/*if (filesize < sizeof(h))  //makes sure its an appropiate size
	{
		printf("Not a good WAV File\n");
		CLOSE
		return 0;
	}*/
	fseek(fil, 0, SEEK_SET); //starting point for reading 
	size_t bytesread = fread(&riffheader, 1, sizeof(riffheader), fil); //reads in header
	if (bytesread != sizeof(riffheader)) {
		printf("Could not read file header.\n");
		CLOSE
		return 0;
	}
	bytesread = fread(&chunk, 1, sizeof(chunk), fil); //reads in header
	if (bytesread != sizeof(chunk)) {
		printf("Could not read file header.\n");
		CLOSE
		return 0;
	}
	fseek(fil, chunk.ChunkSize, SEEK_CUR); //starting point for reading 
	bytesread = fread(&chunk, 1, sizeof(chunk), fil); //reads in header
	if (bytesread != sizeof(chunk)) {
		printf("Could not read file header.\n");
		CLOSE
		return 0;
	}
	bytesread = fread(&fmt, 1, sizeof(fmt), fil); //reads in header
	if (bytesread != sizeof(fmt)) {
		printf("Could not read file header.\n");
		CLOSE
			return 0;
	}


	/*if (memcmp(h.ChunkID, "RIFF", 4) != 0 || memcmp(h.Format, "WAVE", 4) != 0) //checks header
	{
		printf("Invalid WAV file\n");\
		CLOSE
		return 0;
	}
	hnew = h; //changes info of the new file
	hnew.BlockAlign = h.NumChannels * (newbitrate / 8);
	hnew.ByteRate = hnew.BlockAlign * h.SampleRate;
	hnew.BitsPerSample = newbitrate;
	hnew.Subchunk2Size = h.Subchunk2Size / h.BlockAlign * hnew.BlockAlign;
	int orignFloat;
	if (h.AudioFormat == 3)  //checks if original was float
	{
		orignFloat = 1;
	}
	else
	{
		orignFloat = 0;
	}
	if (boolfloat) //checks if new one will be float
	{
		hnew.AudioFormat = 3;
	} 
	else 
	{
		hnew.AudioFormat = 1;
	}
	fwrite(&hnew, 1, sizeof(hnew), filnew); 
	int increment = h.BlockAlign; //increment is one sample with all channels 
	unsigned char* buffer = malloc(increment); //buffer for old file
	unsigned char* outbuffer = malloc(hnew.BlockAlign); //new one
	
	for (int i = sizeof(h); i < filesize; i += increment) 
	{
		size_t bytesread = fread(buffer, 1, increment, fil); //reads in data for one increment at a time
		if (bytesread != increment) 
		{
			printf("Could not read WAV File Sample\n");
			CLOSE2
			return 0;
		} 
		if (ConversionAlgo(buffer, outbuffer, increment, fil, newbitrate, h, boolfloat, orignFloat) == -1) { //runs the algorithm for each increment
			printf("Couln't convert WAV Sample\n");
			CLOSE2
			return 0;
		}
		fwrite(outbuffer, 1, hnew.BlockAlign, filnew); //writes it
	}
	printf("Successfully Rewrote the File\n");
	CLOSE3*/
	return 0; 
}