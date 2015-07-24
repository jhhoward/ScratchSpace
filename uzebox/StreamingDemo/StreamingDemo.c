FATFS FileSystem;

#define MAP_TILE_PIXEL_SIZE 16
#define MAP_TILE_PIXEL_SHIFT 4
#define PIXEL_TO_MAP_TILE(x) ((x) >> MAP_TILE_PIXEL_SHIFT)
#define MAP_TILE_TO_PIXEL(x) ((x) << MAP_TILE_PIXEL_SHIFT)

typedef struct
{
	uint8_t version;
	uint8_t layerCount;
	uint16_t width;
	uint16_t height;
} Map_Header_t;

enum
{
	MapRead_Horizontal,
	MapRead_Vertical
};

Map_Header_t Map_Header;

void Map_Load()
{
	if(pf_mount(FileSystem) == FR_OK)
	{
		if(pf_open("map.dat") == FR_OK)
		{
			unsigned int bytesRead;
			
			pf_read(&Map_Header, sizeof(Map_Header), &bytesRead);
		}
	}
}

void Map_Read(int layer, int x, int y, int orientation, int length, uint8_t* data)
{
	unsigned int bytesRead;
	
	if(orientation == MapRead_Horizontal)
	{
		pf_lseek(sizeof(header_t) + ((2 * layer) * Map_Header.width * Map_Header.height) + (y * Map_Header.width + x));
	}
	else
	{
		pf_lseek(sizeof(header_t) + ((2 * layer + 1) * Map_Header.width * Map_Header.height) + (x * Map_Header.width + y));
	}
	
	pf_read(data, length, &bytesRead);
}

Vector Camera_Position;

// Which map tile the camera is currently set to
uint16_t Map_ScrollX = 0;
uint16_t Map_ScrollY = 0;

// The VRAM scroll offset wrapping
uint8_t Map_ScrollXOffset = 0;
uint8_t Map_ScrollYOffset = 0;

void Map_UpdateAllTiles()
{
	uint8_t buffer[16];
	int cameraTileX = PIXEL_TO_MAP_TILE(Camera_Position.x);
	int cameraTileY = PIXEL_TO_MAP_TILE(Camera_Position.y);
	
	Map_ScrollX = cameraTileX;
	Map_ScrollY = cameraTileY;
	
	Map_ScrollXOffset = Map_ScrollX % 16;
	Map_ScrollYOffset = Map_ScrollY % 16;
	
	int vramY = Map_ScreenYOffset * 2;
	
	for(int row = 0; row < 16; row++)
	{
		Map_Read(0, cameraTileX, cameraTileY, MapRead_Horizontal, 16, buffer);
		int vramX = Map_ScreenXOffset;
		
		for(int column = 0; column < 16; column++)
		{
			SetTile(vramX, vramY, buffer[column]);
			SetTile(vramX + 1, vramY, buffer[column]);
			SetTile(vramX + 1, vramY + 1, buffer[column]);
			SetTile(vramX, vramY + 1, buffer[column]);
		
			vramX += 2;
			if(vramX >= 32)
				vramX = 0;
		}
		
		vramY += 2;
		if(vramY >= 32)
			vramY = 0;
	}
}

void Map_Init()
{
	Camera_Position.x = 256;
	Camera_Position.y = 256;
	
	Map_Load();
	Map_UpdateAllTiles();
}

void Map_UpdateSliceVertical(int offsetX)
{
	uint8_t buffer[16];

	Map_Read(0, Map_ScrollX + offsetX, Map_ScrollY, MapRead_Vertical, 16, buffer);
	
	int vramX = (Map_ScrollXOffset + offsetX) * 2;
	int vramY = Map_ScrollYOffset * 2;
	
	while(vramX >= 32)
		vramX -= 32;
	
	for(int i = 0; i < 16; i++)
	{
		SetTile(vramX, vramY, buffer[i]);
		SetTile(vramX + 1, vramY, buffer[i]);
		SetTile(vramX + 1, vramY + 1, buffer[i]);
		SetTile(vramX, vramY + 1, buffer[i]);
	
		vramY += 2;
		if(vramY == 32)
			vramY = 0;
	}
}

void Map_UpdateSliceHorizontal(int offsetY)
{
	uint8_t buffer[16];

	Map_Read(0, Map_ScrollX, Map_ScrollY + offsetY, MapRead_Horizontal, 16, buffer);
	
	int vramX = Map_ScrollXOffset * 2;
	int vramY = (Map_ScrollYOffset + offsetY) * 2;
	
	while(vramY >= 32)
		vramY -= 32;
	
	for(int i = 0; i < 16; i++)
	{
		SetTile(vramX, vramY, buffer[i]);
		SetTile(vramX + 1, vramY, buffer[i]);
		SetTile(vramX + 1, vramY + 1, buffer[i]);
		SetTile(vramX, vramY + 1, buffer[i]);
	
		vramX += 2;
		if(vramX == 32)
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
		if(Map_ScrollXOffset == 16)
			Map_ScrollXOffset = 0;
			
		Map_UpdateSliceVertical(15);
	}
	while(cameraTileX < Map_ScrollX)
	{
		Map_ScrollX --;
		if(Map_ScrollXOffset == 0)
			Map_ScrollXOffset = 15;
		else
			Map_ScrollXOffset --;
			
		Map_UpdateSliceVertical(0);
	}

	while(cameraTileY > Map_ScrollY)
	{
		Map_ScrollY ++;
		Map_ScrollYOffset++;
		if(Map_ScrollYOffset == 16)
			Map_ScrollYOffset = 0;
			
		Map_UpdateSliceHorizontal(15);
	}
	while(cameraTileY < Map_ScrollY)
	{
		Map_ScrollY --;
		if(Map_ScrollYOffset == 0)
			Map_ScrollYOffset = 15;
		else
			Map_ScrollYOffset --;
			
		Map_UpdateSliceHorizontal(0);
	}
	
	Screen.scrollX = Camera_Position.x;
	Screen.scrollY = Camera_Position.y;
}

int main()
{
	Map_Init();
	int cameraSpeed = 1;
	
	while(1)
	{
		uint16_t joypad = GetJoypad(0);
		
		if((joypad & BTN_A))
		{
			cameraSpeed = 4;
		}
		else
		{
			cameraSpeed = 1;
		}
		
		if((joypad & BTN_LEFT)) && Camera_Position.x > cameraSpeed)
		{
			Camera_Position.x -= cameraSpeed;
		}
		if((joypad & BTN_RIGHT)))
		{
			Camera_Position.x += cameraSpeed;
		}
		if((joypad & BTN_UP)) && Camera_Position.y > cameraSpeed)
		{
			Camera_Position.y -= cameraSpeed;
		}
		if((joypad & BTN_DOWN)))
		{
			Camera_Position.y += cameraSpeed;
		}
		
		Map_Update();
		WaitVsync(1);
	}
	
	return 0;
}
