#ifndef ENTITY_H_
#define ENTITY_H_

#include "Defines.h"
#include "Vector.h"
#include "Archetype.h"

#define MAX_ENTITIES 10

typedef struct Entity_s Entity;
typedef int8_t EntityId;

struct Entity_s
{
	ArchetypeId type;
	Vector position;
};

extern Entity g_Entities[];

extern Entity* GetEntity(EntityId id);

extern void Entity_Initialize(Entity* entity);
extern void Entity_Update(Entity* entity);

extern void Entity_UpdateAll();
extern Entity* Entity_Spawn(ArchetypeId type, Vector position);
extern void Entity_SystemInit();

#endif // ENTITY_H_