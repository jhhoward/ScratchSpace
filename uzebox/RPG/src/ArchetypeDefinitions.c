#include "Archetype.h"
#include "Entity.h"

#undef ARCHETYPE_BEGIN
#undef ARCHETYPE_END
#undef ARCHETYPE_FLAGS

#define ARCHETYPE_BEGIN(x, HandlerPrefix) \
	{	\
##HandlerPrefix##_Initialize, ##HandlerPrefix##_Update,
#define ARCHETYPE_END() \
	},
#define ARCHETYPE_FLAGS(x) \
	{ x }, \

Archetype g_Archetypes[] =
{
#include "ArchetypeDefinitions.inc.h"
};
