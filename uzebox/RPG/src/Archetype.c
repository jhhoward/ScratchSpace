#include "Archetype.h"

Archetype* GetArchetype(ArchetypeId id)
{
	return &g_Archetypes[id];
}

