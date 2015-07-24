#ifndef ARCHETYPE_H_
#define ARCHETYPE_H_

#include "Defines.h"
#include "Vector.h"

// Typedefs
typedef struct Archetype_s Archetype;
typedef int8_t ArchetypeId;
typedef void (*EntityMethod)(struct Entity_s*);

// Flags
enum ArchetypeFlags
{
	AF_Solid	= BIT(0),
};

// Structures
struct Archetype_s
{
	EntityMethod Initialize;
	EntityMethod Update;

	union
	{
		uint8_t allFlags;
		struct flags
		{
			bool solid : 1;
		};
	};
};

extern Archetype g_Archetypes[];

// Functions
extern Archetype* GetArchetype(ArchetypeId id);

#define ARCHETYPE_BEGIN(x, HandlerPrefix) Archetype_##x##,
#define ARCHETYPE_END()
#define ARCHETYPE_FLAGS(x)

enum Archetypes
{
#include "ArchetypeDefinitions.inc.h"
	NumArchetypes
};

#endif // ARCHETYPE_H_