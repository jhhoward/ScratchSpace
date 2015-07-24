#include <stdio.h>

enum eResult
{
	Failure,
	Success
};

class LUTGen
{
public:
	LUTGen(int numColours)
	{
		this->numColours = numColours;
		numEntries = 0;
		entries = new int[numColours * numColours + 1];
		pairMap = new int[numColours * numColours];
		
		for(int n = 0; n < numColours * numColours; n++)
		{
			pairMap[n] = -1;
		}
	}
	
	int GetPairIndex(int a, int b)
	{
		if(a >= numColours || b >= numColours)
			return 0;
			
		return pairMap[b * numColours + a];
	}
	
	void SetPairIndex(int a, int b, int index)
	{
		pairMap[b * numColours + a] = index;
	}
	
	void Push(int val)
	{
		entries[numEntries] = val;
		numEntries++;
	}
	
	void Pop()
	{
		numEntries --;
	}
	
	eResult Generate()
	{
		if(numEntries == numColours * numColours + 1)
		{
			return Success;
		}
		
		for(int n = 0; n < numColours; n++)
		{
			if(numEntries > 0)
			{
				int prev = entries[numEntries - 1];
				
				if(GetPairIndex(prev, n) == -1)
				{
					SetPairIndex(prev, n, numEntries - 1);
					Push(n);
					
					if(Generate() == Success)
					{
						return Success;
					}
					
					SetPairIndex(prev, n, -1);
					
					Pop();
				}
			}
			else
			{
				Push(n);
				
				if(Generate() == Success)
				{
					return Success;
				}
				
				Pop();
			}
		}
		
		return Failure;
	}
	
	void Print(FILE* fs)
	{
		int convertTable[256];
	
		for(int i = 0; i < 256; i++)
		{
			convertTable[i] = 0;
		}
		
		fprintf(fs, "const u8 PaletteEncodingTable[] PROGMEM =\n");
		fprintf(fs, "{\n\t");
		for(int n = 0; n < numEntries; n++)
		{
			fprintf(fs, "0x%x", entries[n]);
			
			if(n != numEntries -1)
			{
				fprintf(fs, ", ");
				if(((n + 1) % numColours) == 0)
				{
					fprintf(fs, "\n\t");
				}
			}
		}
		fprintf(fs, "\n};\n\n");
		
		fprintf(fs, "const u8 PaletteStandardToExtendedTable[] PROGMEM =\n");
		fprintf(fs, "{\n\t");
		for(int i = 0; i < 256; i++)
		{
			int b = i >> 4;
			int a = i & 0xF;
			int index = GetPairIndex(a, b);
			
			convertTable[index] = i;

			fprintf(fs, "0x%02x", index);
			
			if(i != 255)
			{
				fprintf(fs, ",");

				if(((i + 1) % 16) == 0)
				{
					fprintf(fs, "\n\t");
				}
			}
		}
		fprintf(fs, "\n};\n\n");

		fprintf(fs, "const u8 PaletteExtendedToStandardTable[] PROGMEM =\n");
		fprintf(fs, "{\n\t");
		for(int i = 0; i < 256; i++)
		{
			fprintf(fs, "0x%02x", convertTable[i]);
			
			if(i != 255)
			{
				fprintf(fs, ",");

				if(((i + 1) % 16) == 0)
				{
					fprintf(fs, "\n\t");
				}
			}
		}
		fprintf(fs, "\n};\n\n");
	}
		
	int numColours;
	int* entries;
	int numEntries;
	int* pairMap;
};

int main()
{
	LUTGen gen(16);
	
	if(gen.Generate() == Success)
	{
		FILE* fs = fopen("PaletteTables.h", "w");
		gen.Print(fs);
		fclose(fs);
	}
	
	return 0;
}
