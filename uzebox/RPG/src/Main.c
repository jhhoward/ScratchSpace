#include "Defines.h"
#include "Entity.h"
#include "Archetype.h"
#include "Vector.h"

int main()
{
	Vector position = { 0, 0 };
	Entity* entity;
	
	Entity_SystemInit();
	
	entity = Entity_Spawn(Archetype_Test, position);
	
	Entity_UpdateAll();

	return 0;
}
