#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define MAP_SIZE 6
#define MAX_TILE_VALUE (16 * 4)

#define EAST_MASK 1
#define SOUTH_MASK 2
#define WEST_MASK 4
#define NORTH_MASK 8

#define DEADEND_FREQUENCY 70
#define FORKING_FREQUENCY 40
#define CROSSROAD_FREQUENCY 20
#define REQUIRED_FILL 1 / 2
#define MAX_DUPLICATION 20

typedef int bool;
#define true 1
#define false 0

int map[MAP_SIZE * MAP_SIZE];
int startX, startY;
int endX, endY;

bool generateTile(int x, int y);

void printMap()
{
	int x, y;
	
	for(y = 0; y < MAP_SIZE; y++)
	{
		for(x = 0; x < MAP_SIZE; x++)
		{
			int nibble1 = (map[y * MAP_SIZE + x] & 0xF0) >> 4;
			int nibble2 = (map[y * MAP_SIZE + x] & 0x0F);
			
			if(map[y * MAP_SIZE + x] & NORTH_MASK)
			{
				printf(" %x|%x ", nibble1, nibble2);
			}
			else
			{
				printf(" %x %x ", nibble1, nibble2);
			}
		}
		printf("\n");
		for(x = 0; x < MAP_SIZE; x++)
		{
			if(map[y * MAP_SIZE + x] & WEST_MASK)
			{
				printf("--");
			}
			else
			{
				printf("  ");
			}
			if(map[y * MAP_SIZE + x] != 0)
			{
				if(x == startX && y == startY)
				{
					printf("S");
				}
				else if(x == endX && y == endY)
				{
					printf("E");
				}
				else
				{
					printf("o");
				}
			}
			else
			{
				printf(" ");
			}
			if(map[y * MAP_SIZE + x] & EAST_MASK)
			{
				printf("--");
			}
			else
			{
				printf("  ");
			}
		}
		printf("\n");
		for(x = 0; x < MAP_SIZE; x++)
		{
			if(map[y * MAP_SIZE + x] & SOUTH_MASK)
			{
				printf("  |  ");
			}
			else
			{
				printf("     ");
			}
		}
		printf("\n");
	}
}

int numExits(int tile)
{
	int count = 0;
	tile = tile & 0xF;
	
	while(tile)
	{
		if((tile & 0x1) != 0)
		{
			count ++;
		}
		tile >>= 1;
	}
	
	return count;
}

int requiredMask(int x, int y)
{
	int mask = 0;
	
	if(y > 0 && (map[(y - 1) * MAP_SIZE + x] & SOUTH_MASK))
	{
		mask |= NORTH_MASK;
	}
	if(x > 0 && (map[y * MAP_SIZE + x - 1] & EAST_MASK))
	{
		mask |= WEST_MASK;
	}
	if(y < MAP_SIZE - 1 && (map[(y + 1) * MAP_SIZE + x] & NORTH_MASK))
	{
		mask |= SOUTH_MASK;
	}
	if(x < MAP_SIZE - 1 && (map[y * MAP_SIZE + x + 1] & WEST_MASK))
	{
		mask |= EAST_MASK;
	}
	
	return mask;
}

int allowedMask(int x, int y)
{
	int mask = requiredMask(x, y);
	
	if(y > 0 && (map[(y - 1) * MAP_SIZE + x] == 0))
	{
		mask |= NORTH_MASK;
	}
	if(x > 0 && (map[y * MAP_SIZE + x - 1] == 0))
	{
		mask |= WEST_MASK;
	}
	if(y < MAP_SIZE - 1 && (map[(y + 1) * MAP_SIZE + x] == 0))
	{
		mask |= SOUTH_MASK;
	}
	if(x < MAP_SIZE - 1 && (map[y * MAP_SIZE + x + 1] == 0))
	{
		mask |= EAST_MASK;
	}
	
	return mask;
}

int countTileUsed(int tile)
{
	int n;
	int count = 0;
	
	for(n = 0; n < MAP_SIZE * MAP_SIZE; n++)
	{
		if(tile == map[n])
			count++;
	}
	
	return count;
}

int countOpenTiles()
{
	int x, y;
	int count = 0;

	for(y = 0; y < MAP_SIZE; y++)
	{
		for(x = 0; x < MAP_SIZE; x++)
		{
			if(map[y * MAP_SIZE + x])
			{
				if(y > 0 && (map[y * MAP_SIZE + x] & NORTH_MASK) != 0 && map[(y - 1) * MAP_SIZE + x] == 0)
				{
					count++;
				}
				if(x > 0 && (map[y * MAP_SIZE + x] & WEST_MASK) != 0 && map[y * MAP_SIZE + x - 1] == 0)
				{
					count++;
				}
				if(y < MAP_SIZE - 1 && (map[y * MAP_SIZE + x] & SOUTH_MASK) != 0 && map[(y + 1) * MAP_SIZE + x] == 0)
				{
					count++;
				}
				if(x < MAP_SIZE - 1 && (map[y * MAP_SIZE + x] & EAST_MASK) != 0 && map[y * MAP_SIZE + x + 1] == 0)
				{
					count++;
				}
			}
		}
	}
	
	return count;
}

int maxAvailableExits(int x, int y)
{
	int exits = 0;
	
	if(y != 0
	&& (map[(y - 1) * MAP_SIZE + x] == 0
	|| (map[(y - 1) * MAP_SIZE + x] & SOUTH_MASK) != 0))
	{
		exits++;
	}	

	if(y != MAP_SIZE - 1
	&& (map[(y + 1) * MAP_SIZE + x] == 0
	|| (map[(y + 1) * MAP_SIZE + x] & NORTH_MASK) != 0))
	{
		exits++;
	}	

	if(x != 0
	&& (map[y * MAP_SIZE + x - 1] == 0
	|| (map[y * MAP_SIZE + x - 1] & EAST_MASK) != 0))
	{
		exits++;
	}	

	if(x != MAP_SIZE - 1
	&& (map[y * MAP_SIZE + x + 1] == 0
	|| (map[y * MAP_SIZE + x + 1] & WEST_MASK) != 0))
	{
		exits++;
	}	
	
	return exits;
}

bool checkNeighbours(int x, int y, int tile)
{
	if(tile & NORTH_MASK)
	{
		if(y == 0)
			return false;
		if(map[(y - 1) * MAP_SIZE + x] != 0
		&& (map[(y - 1) * MAP_SIZE + x] & SOUTH_MASK) == 0)
			return false;
	}
	else
	{
		if(y != 0
		&& map[(y - 1) * MAP_SIZE + x] != 0
		&& (map[(y - 1) * MAP_SIZE + x] & SOUTH_MASK) != 0)
			return false;
	}
	if(tile & SOUTH_MASK)
	{
		if(y == MAP_SIZE - 1)
			return false;
		if(map[(y + 1) * MAP_SIZE + x] != 0
		&& (map[(y + 1) * MAP_SIZE + x] & NORTH_MASK) == 0)
			return false;
	}
	else
	{
		if(y != MAP_SIZE - 1
		&& map[(y + 1) * MAP_SIZE + x] != 0
		&& (map[(y + 1) * MAP_SIZE + x] & NORTH_MASK) != 0)
			return false;
	}
	if(tile & WEST_MASK)
	{
		if(x == 0)
			return false;
		if(map[y * MAP_SIZE + x - 1] != 0
		&& (map[y * MAP_SIZE + x - 1] & EAST_MASK) == 0)
			return false;
	}
	else
	{
		if(x != 0
		&& map[y * MAP_SIZE + x - 1] != 0
		&& (map[y * MAP_SIZE + x - 1] & EAST_MASK) != 0)
			return false;
	}
	if(tile & EAST_MASK)
	{
		if(x == MAP_SIZE - 1)
			return false;
		if(map[y * MAP_SIZE + x + 1] != 0
		&& (map[y * MAP_SIZE + x + 1] & WEST_MASK) == 0)
			return false;
	}
	else
	{
		if(x != MAP_SIZE - 1
		&& map[y * MAP_SIZE + x + 1] != 0
		&& (map[y * MAP_SIZE + x + 1] & WEST_MASK) != 0)
			return false;
	}
	
	return true;
}

bool generateNeighbours(int x, int y)
{
	int tile = map[y * MAP_SIZE + x];
	
	if((tile & NORTH_MASK) && map[(y - 1) * MAP_SIZE + x] == 0)
	{
		if(!generateTile(x, y - 1))
			return false;
	}
	if((tile & SOUTH_MASK) && map[(y + 1) * MAP_SIZE + x] == 0)
	{
		if(!generateTile(x, y + 1))
			return false;
	}
	if((tile & EAST_MASK) && map[y * MAP_SIZE + x + 1] == 0)
	{
		if(!generateTile(x + 1, y))
			return false;
	}
	if((tile & WEST_MASK) && map[y * MAP_SIZE + x - 1] == 0)
	{
		if(!generateTile(x - 1, y))
			return false;
	}
	
	return true;
}

bool generateTile(int x, int y)
{
	bool found = false;
	int tile;
	int attempts = 0;
	bool allowDeadEnd = false;
	bool filledMap = false;
	bool pickingAtRandom = true;
	int required, allowed;
	
	if(map[y * MAP_SIZE + x] != 0)
		return false;
		
	required = requiredMask(x, y);
	allowed = allowedMask(x, y);
		
	filledMap = numRooms() >= (MAP_SIZE * MAP_SIZE) * REQUIRED_FILL;
	//printf("Generate(%d, %d)\n", x, y);
	allowDeadEnd = countOpenTiles() > 1;
	
	while(!found)
	{
		if(pickingAtRandom)
		{
			tile = rand() % MAX_TILE_VALUE;
			
			attempts++;
			if(attempts == 255)
			{
				pickingAtRandom = false;
				tile = 1;
			}
		}
		else
		{
			tile++;
			if(tile == MAX_TILE_VALUE)
			{
				/*printf("Failing: no solutions for tile placement\n");
				printf("Open tiles: %d\n", countOpenTiles());
				printf("Required: %d Allowed: %d\n", required, allowed);
				printf("x: %d y: %d\n", x, y);
				printMap();
*/
				return false;
			}
		}
		
		if((tile & required) == required
		&& (tile & allowed) == (tile & 0xF)
		&& countTileUsed(tile) < MAX_DUPLICATION) // && checkNeighbours(x, y, tile))
		{
			switch(numExits(tile))
			{
				case 1:
//					found = maxAvailableExits(x, y) == 1 || (allowDeadEnd && (rand() % 100) < DEADEND_FREQUENCY);
					found = !pickingAtRandom || (allowDeadEnd && (rand() % 100) < DEADEND_FREQUENCY);
					break;
				case 2:
					found = true;
					break;
				case 3:
					found = !pickingAtRandom || (rand() % 100) < FORKING_FREQUENCY;
					break;
				case 4:
					found = !pickingAtRandom || (rand() % 100) < CROSSROAD_FREQUENCY;
					break;				
			}
		}
		
		if(found)
		{
			map[y * MAP_SIZE + x] = tile;
			
			if(!filledMap && countOpenTiles() == 0)
			{
				//printf("Prevented closing of map: %d\n", tile & 0xF);
				map[y * MAP_SIZE + x] = 0;
				found = false;
			}
		}
	}
	
	//printf("start---\n");
	//printMap();
	//printf("ends---\n");

	return generateNeighbours(x, y);
}

bool generateMap()
{
	int seedTile;
	bool found = false;

	startX = rand() % MAP_SIZE;
	startY = rand() % MAP_SIZE;
	
	while(!found)
	{
		seedTile = rand() % MAX_TILE_VALUE;
		
		if(numExits(seedTile) == 1 && checkNeighbours(startX, startY, seedTile))
		{
			found = true;
		}
	}
	
	map[startY * MAP_SIZE + startX] = seedTile;
	
	return generateNeighbours(startX, startY);
}

int numRooms()
{
	int n;
	int numRooms = 0;
	for(n = 0; n < MAP_SIZE * MAP_SIZE; n++)
	{
		if(map[n] != 0)
		{
			numRooms++;
		}
	}
	return numRooms;
}

void clearMap()
{
	int n;
	for(n = 0; n < MAP_SIZE * MAP_SIZE; n++)
	{
		map[n] = 0;
	}
}

void findBestEndPoint(int* distanceFromStart, int x, int y, int distance)
{
	if(map[y * MAP_SIZE + x] == 0)
		return;
		
	if(distanceFromStart[y * MAP_SIZE + x] != 0
	&& distanceFromStart[y * MAP_SIZE + x] <= distance)
		return;
	
	distanceFromStart[y * MAP_SIZE + x] = distance;
	
	if(y > 0 && map[y * MAP_SIZE + x] & NORTH_MASK)
	{
		findBestEndPoint(distanceFromStart, x, y - 1, distance + 1);
	}
	if(x > 0 && map[y * MAP_SIZE + x] & WEST_MASK)
	{
		findBestEndPoint(distanceFromStart, x - 1, y, distance + 1);
	}
	if(y < MAP_SIZE - 1 && map[y * MAP_SIZE + x] & SOUTH_MASK)
	{
		findBestEndPoint(distanceFromStart, x, y + 1, distance + 1);
	}
	if(x < MAP_SIZE - 1 && map[y * MAP_SIZE + x] & EAST_MASK)
	{
		findBestEndPoint(distanceFromStart, x + 1, y, distance + 1);
	}
}

void generateStartEnd()
{
	int distanceFromStart[MAP_SIZE * MAP_SIZE];
	int n;
	int bestDistance = 0;
	int x, y;
	bool deadEndOnly = true;
	
	for(n = 0; n < MAP_SIZE * MAP_SIZE; n++)
	{
		distanceFromStart[n] = 0;
	}
	
	findBestEndPoint(distanceFromStart, startX, startY, 1);

	while(bestDistance == 0)
	{
		for(y = 0; y < MAP_SIZE; y++)
		{
			for(x = 0; x < MAP_SIZE; x++)
			{
				if(x != startX || y != startY)
				{
					if((!deadEndOnly || numExits(map[y * MAP_SIZE + x]) == 1) && distanceFromStart[y * MAP_SIZE + x] > bestDistance )
					{
						endX = x;
						endY = y;
						bestDistance = distanceFromStart[y * MAP_SIZE + x];
					}
				}
			}
		}
		
		deadEndOnly = false;
	}
}

int main()
{
	int attempts = 0;
	bool success = false;
	clock_t start = clock();
	
	srand(time(0));

	do
	{
		clearMap();
		success = generateMap();
		
		if(success && numRooms() < (MAP_SIZE * MAP_SIZE) * REQUIRED_FILL)
		{
			success = false;
			//printf("Failed as map was too small!\n");
			//printMap();
		}
		attempts++;
	} while(!success);

	generateStartEnd();
	
	printf("Took %f seconds\n", (float)(clock() - start) / (float)CLOCKS_PER_SEC);
	printf("After %d attempts:\n", attempts);
	printMap();

	return 0;
}
