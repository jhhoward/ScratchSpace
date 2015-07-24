#include "Entity.h"
#include "Archetype.h"

#include <stdio.h>

Entity g_Entities[MAX_ENTITIES];

Entity* GetEntity(EntityId id)
{
	return &g_Entities[id];
}

void Entity_Initialize(Entity* entity)
{
	printf("Init %x\n", (uintptr_t*)entity);
}

void Entity_Update(Entity* entity)
{
	printf("Update %x\n", (uintptr_t*)entity);
	
	printf("Flags: %d\n", GetArchetype(entity->type)->allFlags);
}

void Entity_UpdateAll()
{
	int i;
	
	for(i = 0; i < MAX_ENTITIES; i++)
	{
		if(g_Entities[i].type != INVALID_ID)
		{
			EntityMethod updateFunction = GetArchetype(g_Entities[i].type)->Update;
			if(updateFunction)
			{
				updateFunction(&g_Entities[i]);
			}
		}
	}
}

Entity* Entity_Spawn(ArchetypeId type, Vector position)
{
	EntityId freeId = 0;
	Entity* newEntity = NULL;
	Archetype* archetype = GetArchetype(type);
	
	for(freeId = 0; freeId < MAX_ENTITIES; freeId++)
	{
		if(g_Entities[freeId].type == INVALID_ID)
		{
			newEntity = &g_Entities[freeId];
			break;
		}
	}
	
	if(freeId == MAX_ENTITIES)
	{
		return NULL;
	}
	
	newEntity->type = type;
	newEntity->position = position;
	archetype->Initialize(newEntity);
	
	return newEntity;
}

void Entity_SystemInit()
{
	EntityId i;
	for(i = 0; i < MAX_ENTITIES; i++)
	{
		g_Entities[i].type = INVALID_ID;
	}
}

