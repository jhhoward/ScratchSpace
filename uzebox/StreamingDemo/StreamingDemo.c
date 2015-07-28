#include <stdbool.h>
#include <avr/io.h>
#include <stdlib.h>
#include <math.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <uzebox.h>
#include "petitfatfs/pff.h"
#include "data/tiles.inc.h"
#include "data/overlay.inc.h"

#define SCREEN_OVERLAY_HEIGHT 2
#define SCREEN_SCROLL_HEIGHT (VRAM_TILES_V - SCREEN_OVERLAY_HEIGHT)

#define MAP_TILE_PIXEL_SIZE 16
#define MAP_TILE_PIXEL_SHIFT 4
#define VRAM_SCROLL_WRAP_X VRAM_TILES_H
#define VRAM_SCROLL_WRAP_Y SCREEN_SCROLL_HEIGHT
#define MAP_TILE_SCROLL_WRAP_X (VRAM_SCROLL_WRAP_X / 2)
#define MAP_TILE_SCROLL_WRAP_Y (VRAM_SCROLL_WRAP_Y / 2)
#define PIXEL_TO_MAP_TILE(x) ((x) >> MAP_TILE_PIXEL_SHIFT)
#define MAP_TILE_TO_PIXEL(x) ((x) << MAP_TILE_PIXEL_SHIFT)

#define MAP_HEADER_SIZE 6
#define MAP_READ_LENGTH 16
#define REMOUNT_ON_READ 0
#define NUM_READ_TRIES 1


typedef struct
{
	uint8_t version;
	uint8_t layerCount;
	int16_t width;
	int16_t height;
} Map_Header_t;

typedef struct
{
	int16_t x, y;
} Vector;

enum
{
	MapRead_Horizontal,
	MapRead_Vertical
};

Map_Header_t Map_Header;
Vector Camera_Position;
uint8_t Map_ReadBuffer[MAP_READ_LENGTH];
FATFS FileSystem;

void Map_Load()
{
	int tries = 0;
	
	Print(0, SCREEN_SCROLL_HEIGHT, PSTR("LOADING.."));

	while(tries < NUM_READ_TRIES)
	{
		tries++;
		if(pf_mount(&FileSystem) == FR_OK)
		{
			if(pf_open("map.dat") == FR_OK)
			{
				WORD bytesRead;
				
				pf_read(&Map_Header, sizeof(Map_Header), &bytesRead);
				
				Print(0, SCREEN_SCROLL_HEIGHT, PSTR("MAP LOADED!      "));
				Print(0, SCREEN_SCROLL_HEIGHT + 1, PSTR("MAP SIZE:     ,"));
				PrintInt(13, SCREEN_SCROLL_HEIGHT + 1, 128, false);
				PrintInt(19, SCREEN_SCROLL_HEIGHT + 1, 128, false);
			}
			else
			{
				Print(0, SCREEN_SCROLL_HEIGHT, PSTR("MAP LOAD FAILED"));
			}
			
			#if REMOUNT_ON_READ
			pf_mount(NULL);
			#endif
			
			return;
		}
		else
		{
			pf_mount(NULL);
			Print(0, SCREEN_SCROLL_HEIGHT, PSTR("RETRYING MOUNT   "));
		}
	}
	Print(0, SCREEN_SCROLL_HEIGHT, PSTR("DISK MOUNT FAILED"));
}

void Map_Read(uint8_t layer, int x, int y, uint8_t orientation)
{
	WORD bytesRead;
	
	#if REMOUNT_ON_READ
	pf_mount(&FileSystem);
	pf_open("map.dat");
	#endif	
	
	if(orientation == MapRead_Horizontal)
	{
		pf_lseek(sizeof(Map_Header_t) + ((2 * layer) * Map_Header.width * Map_Header.height) + (y * Map_Header.width + x));
	}
	else
	{
		pf_lseek(sizeof(Map_Header_t) + ((2 * layer + 1) * Map_Header.width * Map_Header.height) + (x * Map_Header.height + y));
	}
	
	pf_read(Map_ReadBuffer, MAP_READ_LENGTH, &bytesRead);
	
	#if REMOUNT_ON_READ
	pf_mount(NULL);
	#endif
	
	#if 0
	Map_Header.width = 128;
	Map_Header.height = 128;
	if(orientation == MapRead_Horizontal)
	{
		for(int n = 0; n < MAP_READ_LENGTH; n++)
		{
			Map_ReadBuffer[n] = (y % 3) == 0 || ((x + n) % 3) == 0 ? 1 : 0;
		}
	}
	else
	{
		for(int n = 0; n < MAP_READ_LENGTH; n++)
		{
			Map_ReadBuffer[n] = ((y + n) % 3) == 0 || ((x) % 3) == 0 ? 1 : 0;
		}
	}
	#endif
}

// Which map tile the camera is currently set to
uint16_t Map_ScrollX = 0;
uint16_t Map_ScrollY = 0;

// The VRAM scroll offset wrapping
uint8_t Map_ScrollXOffset = 0;
uint8_t Map_ScrollYOffset = 0;

void Map_UpdateAllTiles()
{
	int cameraTileX = PIXEL_TO_MAP_TILE(Camera_Position.x);
	int cameraTileY = PIXEL_TO_MAP_TILE(Camera_Position.y);
	
	Map_ScrollX = cameraTileX;
	Map_ScrollY = cameraTileY;
	
	Map_ScrollXOffset = Map_ScrollX % MAP_TILE_SCROLL_WRAP_X;
	Map_ScrollYOffset = Map_ScrollY % MAP_TILE_SCROLL_WRAP_Y;
	
	int vramY = Map_ScrollYOffset * 2;
	
	for(int row = 0; row < MAP_TILE_SCROLL_WRAP_Y; row++)
	{
		Map_Read(0, cameraTileX, cameraTileY + row, MapRead_Horizontal);
		int vramX = Map_ScrollXOffset * 2;
		
		for(int column = 0; column < MAP_TILE_SCROLL_WRAP_X; column++)
		{
			uint8_t tile = 4 * Map_ReadBuffer[column];
			
			SetTile(vramX, vramY, tile + 0);
			SetTile(vramX + 1, vramY, tile + 1);
			SetTile(vramX, vramY + 1, tile + 2);
			SetTile(vramX + 1, vramY + 1, tile + 3);
		
			vramX += 2;
			if(vramX >= VRAM_SCROLL_WRAP_X)
				vramX = 0;
		}
		
		vramY += 2;
		if(vramY >= VRAM_SCROLL_WRAP_Y)
			vramY = 0;
	}
}

void Map_Init()
{
	Camera_Position.x = 0;
	Camera_Position.y = 0;
	
	Map_Load();
	Map_UpdateAllTiles();
}

void Map_UpdateSliceVertical(int offsetX)
{
	Map_Read(0, Map_ScrollX + offsetX, Map_ScrollY, MapRead_Vertical);
	
	int vramX = (Map_ScrollXOffset + offsetX) * 2;
	int vramY = Map_ScrollYOffset * 2;
	
	while(vramX >= VRAM_SCROLL_WRAP_X)
		vramX -= VRAM_SCROLL_WRAP_X;
	
	for(int i = 0; i < MAP_TILE_SCROLL_WRAP_Y; i++)
	{
		uint8_t tile = 4 * Map_ReadBuffer[i];
		
		SetTile(vramX, vramY, tile + 0);
		SetTile(vramX + 1, vramY, tile + 1);
		SetTile(vramX, vramY + 1, tile + 2);
		SetTile(vramX + 1, vramY + 1, tile + 3);
	
		vramY += 2;
		if(vramY == VRAM_SCROLL_WRAP_Y)
			vramY = 0;
	}
}

void Map_UpdateSliceHorizontal(int offsetY)
{
	Map_Read(0, Map_ScrollX, Map_ScrollY + offsetY, MapRead_Horizontal);
	
	int vramX = Map_ScrollXOffset * 2;
	int vramY = (Map_ScrollYOffset + offsetY) * 2;
	
	while(vramY >= VRAM_SCROLL_WRAP_Y)
		vramY -= VRAM_SCROLL_WRAP_Y;
	
	for(int i = 0; i < MAP_TILE_SCROLL_WRAP_X; i++)
	{
		uint8_t tile = 4 * Map_ReadBuffer[i];
		
		SetTile(vramX, vramY, tile + 0);
		SetTile(vramX + 1, vramY, tile + 1);
		SetTile(vramX, vramY + 1, tile + 2);
		SetTile(vramX + 1, vramY + 1, tile + 3);
	
		vramX += 2;
		if(vramX == VRAM_SCROLL_WRAP_X)
			vramX = 0;
	}
}

void Map_Update()
{
	int cameraTileX = PIXEL_TO_MAP_TILE(Camera_Position.x);
	int cameraTileY = PIXEL_TO_MAP_TILE(Camera_Position.y);

	while(cameraTileX > Map_ScrollX)
	{
		Map_ScrollX ++;
		Map_ScrollXOffset++;
		if(Map_ScrollXOffset == MAP_TILE_SCROLL_WRAP_X)
			Map_ScrollXOffset = 0;
			
		Map_UpdateSliceVertical(MAP_TILE_SCROLL_WRAP_X - 1);
	}
	while(cameraTileX < Map_ScrollX)
	{
		Map_ScrollX --;
		if(Map_ScrollXOffset == 0)
			Map_ScrollXOffset = MAP_TILE_SCROLL_WRAP_X - 1;
		else
			Map_ScrollXOffset --;
			
		Map_UpdateSliceVertical(0);
	}

	while(cameraTileY > Map_ScrollY)
	{
		Map_ScrollY ++;
		Map_ScrollYOffset++;
		if(Map_ScrollYOffset == MAP_TILE_SCROLL_WRAP_Y)
			Map_ScrollYOffset = 0;
			
		Map_UpdateSliceHorizontal(MAP_TILE_SCROLL_WRAP_Y - 1);
	}
	while(cameraTileY < Map_ScrollY)
	{
		Map_ScrollY --;
		if(Map_ScrollYOffset == 0)
			Map_ScrollYOffset = MAP_TILE_SCROLL_WRAP_Y - 1;
		else
			Map_ScrollYOffset --;
			
		Map_UpdateSliceHorizontal(0);
	}
	
	Screen.scrollX = Camera_Position.x;
	Screen.scrollY = Camera_Position.y % (SCREEN_SCROLL_HEIGHT * 8);
}

int main()
{
	ClearVram();
	SetTileTable(graphicsTiles);
	Screen.overlayTileTable = overlayTiles;
	
	for(int y=0;y<32;y++){
		for(int x=0;x<32;x++){
			SetTile(x,y,0);
		}	
	}

	Screen.scrollHeight = SCREEN_SCROLL_HEIGHT;
	Screen.overlayHeight = SCREEN_OVERLAY_HEIGHT;

	Map_Init();
	int cameraSpeed = 1;
	
	while(1)
	{
		uint16_t joypad = ReadJoypad(0);
		
		if((joypad & BTN_A))
		{
			cameraSpeed = 4;
		}
		else
		{
			cameraSpeed = 1;
		}
		
		if((joypad & BTN_LEFT) && Camera_Position.x > cameraSpeed)
		{
			Camera_Position.x -= cameraSpeed;
		}
		if((joypad & BTN_RIGHT) && Camera_Position.x < (Map_Header.width - MAP_TILE_SCROLL_WRAP_X - 1) * MAP_TILE_PIXEL_SIZE - cameraSpeed)
		{
			Camera_Position.x += cameraSpeed;
		}
		if((joypad & BTN_UP) && Camera_Position.y > cameraSpeed)
		{
			Camera_Position.y -= cameraSpeed;
		}
		if((joypad & BTN_DOWN) && Camera_Position.y < (Map_Header.height - MAP_TILE_SCROLL_WRAP_Y - 1) * MAP_TILE_PIXEL_SIZE - cameraSpeed)
		{
			Camera_Position.y += cameraSpeed;
		}
		
		Map_Update();
//		Screen.scrollX = Camera_Position.x;
	//	Screen.scrollY = Camera_Position.y;

		WaitVsync(1);
	}
	
	return 0;
}
