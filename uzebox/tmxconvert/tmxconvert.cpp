#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <list>
#include <string>
#include "tmxparser.h"

#define VERSION_NUMBER 1
#define DEFAULT_OUTPUT_FILENAME "out.dat"

typedef struct
{
	uint8_t version;
	uint8_t layerCount;
	uint16_t width;
	uint16_t height;
} header_t;

const char* programName = NULL;

void printUsage();
void writeFile(const char* filename);

void printUsage()
{
	printf("Usage: %s [options] [input.tmx]\n"
			"\nOptions:\n"
			"\t-o\t\t\t set output file name\n"
			"\t-layers=layer1,layer2\t specify output layers\n\n", programName);
	exit(1);
}

uint8_t getTile(tmxparser::TmxMap& map, int layer, int x, int y)
{
	tmxparser::TmxLayerTile& tile = map.layerCollection[layer].tiles[y * map.width + x];
	tmxparser::TmxTileset& tileset = map.tilesetCollection[tile.tilesetIndex];

	uint32_t index = tile.gid - tileset.firstgid;
	return (uint8_t) index;
}

void writeFile(tmxparser::TmxMap& map, const char* filename)
{
	if(map.width > 65535 || map.height > 65535)
	{
		fprintf(stderr, "Input map dimensions too large!\n");
		exit(2);
	}

	// Check layers are all the same dimension
	for(int layer = 0; layer < map.layerCollection.size(); layer++)
	{
		if(map.width != map.layerCollection[layer].width
		|| map.height != map.layerCollection[layer].height)
		{
			fprintf(stderr, "Layer %s dimensions (%d, %d) do not match map (%d, %d)\n",
					map.layerCollection[layer].name.c_str(),
					map.layerCollection[layer].width, map.layerCollection[layer].height,
					map.width, map.height);
			exit(2);
		}
	}

	header_t fileHeader;
	fileHeader.version = VERSION_NUMBER;
	fileHeader.width = (uint16_t)(map.width);
	fileHeader.height = (uint16_t)(map.height);
	fileHeader.layerCount = map.layerCollection.size();
	
	FILE* fs = fopen(filename, "wb");
	
	if(fs == NULL)
	{
		fprintf(stderr, "Error opening %s for write\n", filename);
		exit(2);
	}
	
	fwrite(&fileHeader, sizeof(fileHeader), 1, fs);
	
	for(int layer = 0; layer < map.layerCollection.size(); layer++)
	{
		for(int y = 0; y < map.height; y++)
		{
			for(int x = 0; x < map.width; x++)
			{
				uint8_t tile = getTile(map, layer, x, y);
				fwrite(&tile, 1, 1, fs);
			}
		}
		for(int x = 0; x < map.width; x++)
		{
			for(int y = 0; y < map.height; y++)
			{
				uint8_t tile = getTile(map, layer, x, y);
				fwrite(&tile, 1, 1, fs);
			}
		}
	}
	
	fclose(fs);
}

int main(int argc, char* argv[])
{
	const char* inputFile = NULL;
	const char* outputFile = NULL;
	
	programName = argv[0];
	
	for(int n = 1; n < argc; n++)
	{
		if(!strcmp(argv[n], "-o"))
		{
			if(n + 1 < argc)
			{
				outputFile = argv[n + 1];
				n++;
			}
			else printUsage();
		}
		else if(!strcmp(argv[n], "-layers"))
		{
			if(n + 1 < argc)
			{
				n++;
			}
			else printUsage();
		}
		else if(argv[n][0] == '-')
		{
			printUsage();
		}
		else
		{
			if(inputFile == NULL)
			{
				inputFile = argv[n];
			}
			else printUsage();
		}
	}
	
	if(inputFile == NULL)
	{
		printUsage();
	}
	if(outputFile == NULL)
	{
		outputFile = DEFAULT_OUTPUT_FILENAME;
	}

	tmxparser::TmxMap map;
	tmxparser::TmxReturn error = tmxparser::parseFromFile(inputFile, &map, "");
	
	if(error != tmxparser::kSuccess)
	{
		fprintf(stderr, "Error reading file %s\n", inputFile);
		exit(2);
	}	
			
	writeFile(map, outputFile);

	printf("Input: %s\n"
			"Dimensions: %d x %d\n"
			"Output: %s\n", inputFile, map.width, map.height, outputFile);
	
	return 0;
}
