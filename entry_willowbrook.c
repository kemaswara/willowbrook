// SPRITE STRUCTURE AND ENUM DECLARATION

// Structure representing a Sprite, containing a pointer to the image and its size
typedef struct Sprite
{
	Gfx_Image *image;
	Vector2 size;
} Sprite;

// Enum for different sprite IDs, used to reference specific sprites
typedef enum SpriteID
{
	SPRITE_nil,	   // Null sprite (default)
	SPRITE_player, // Player sprite
	SPRITE_rock,   // Rock sprite
	SPRITE_MAX	   // Maximum number of sprites (used for array sizing)
} SpriteID;

// Array to hold all sprites in the game
Sprite sprites[SPRITE_MAX];

// Function to retrieve a sprite by its ID
Sprite *get_sprite(SpriteID id)
{
	if (id >= 0 && id < SPRITE_MAX)
	{
		return &sprites[id];
	}
	// Return a default sprite if the ID is out of bounds
	return &sprites[SPRITE_nil];
}

// ENTITY STRUCTURE AND ENUM DECLARATION

// Enum representing different entity archetypes (types of entities)
typedef enum EntityArchetype
{
	arch_nil = 0,	 // Null archetype (default)
	arch_player = 1, // Player archetype
	arch_tree = 2,	 // Tree archetype
	arch_rock = 3,	 // Rock archetype
} EntityArchetype;

// Structure representing an Entity, containing its attributes and state
typedef struct Entity
{
	bool is_valid;		  // Flag to indicate if the entity is active/valid
	EntityArchetype arch; // The archetype/type of the entity
	Vector2 pos;		  // Position of the entity in the world
	bool render_sprite;	  // Flag to determine if the entity should render a sprite
	SpriteID sprite_id;	  // ID of the sprite to be rendered for this entity
} Entity;

// Maximum number of entities allowed in the world
#define MAX_ENTITY_COUNT 1024

// World structure containing all entities
typedef struct World
{
	Entity entities[MAX_ENTITY_COUNT];
} World;

// Global pointer to the world instance
World *world = 0;

// ENTITY MANAGEMENT FUNCTIONS

// Function to create a new entity
Entity *entity_create()
{
	Entity *entity_found = 0;
	for (int i = 0; i < MAX_ENTITY_COUNT; i++)
	{
		Entity *existing_entity = &world->entities[i];
		if (!existing_entity->is_valid)
		{
			entity_found = existing_entity;
			break;
		}
	}
	// Ensure an entity was found
	assert(entity_found, "No more free entities!");
	entity_found->is_valid = true;
	return entity_found;
}

// Function to destroy an entity and reset its data
void entity_destroy(Entity *entity)
{
	memset(entity, 0, sizeof(Entity)); // Clear the memory of the entity
}

// ENTITY SETUP FUNCTIONS

// Function to setup a rock entity
void setup_rock(Entity *en)
{
	en->arch = arch_rock;
	en->sprite_id = SPRITE_rock;
}

// Function to setup a player entity
void setup_player(Entity *en)
{
	en->arch = arch_player;
	en->sprite_id = SPRITE_player;
}

// UTILITY FUNCTIONS
void draw_fps_counter(float delta_t, float64 *seconds_counter, s32 *frame_count, Gfx_Font *font)
{
	// Update counters
	*seconds_counter += delta_t;
	*frame_count += 1;

	// FPS counter (logs frames per second every second)
	if (*seconds_counter > 1.0)
	{
		log("fps: %i", *frame_count);
		*seconds_counter = 0.0;
		*frame_count = 0;
	}

	// Draw FPS Counter
	draw_text(font, tprint("%04i fps", *frame_count), 12, v2((window.pixel_width / 2) - 100, (window.pixel_height / 2) - 50), v2(1, 1), COLOR_WHITE);
}

// ENTRY POINT OF THE PROGRAM

int entry(int argc, char **argv)
{
	// Window settings
	window.title = STR("Willowbrook");
	window.scaled_width = 1280; // Handle system scaling (DPI)
	window.scaled_height = 720;
	window.x = 200;
	window.y = 90;
	window.clear_color = hex_to_rgba(0x2D2D34ff);

	// Allocate memory for the world
	world = alloc(get_heap_allocator(), sizeof(World));

	// LOAD FONT
	Gfx_Font *font_mono = load_font_from_disk(STR("assets/fonts/monogram/ttf/monogram.ttf"), get_heap_allocator());
	assert(font_mono, "Failed loading monogram.ttf");
	Gfx_Font *font_base = load_font_from_disk(STR("assets/fonts/peaberry/ttf/peaberry_base.ttf"), get_heap_allocator());
	assert(font_base, "Failed loading peaberry_base.ttf");

	const u32 font_height = 48;

	// LOAD SPRITES
	sprites[SPRITE_player] = (Sprite){.image = load_image_from_disk(STR("assets/player.png"), get_heap_allocator()), .size = v2(16.0, 16.0)};
	sprites[SPRITE_rock] = (Sprite){.image = load_image_from_disk(STR("assets/rock.png"), get_heap_allocator()), .size = v2(16.0, 16.0)};

	// CREATE ENTITIES
	// Note: The order of entity creation affects rendering order (last created is rendered on top)
	for (int i = 0; i < 10; i++)
	{
		Entity *en = entity_create();
		setup_rock(en);
		en->pos = v2(get_random_float32_in_range(-10, 10), get_random_float32_in_range(-10, 10));
	}

	// Create and setup the player entity
	Entity *player_en = entity_create();
	setup_player(player_en);

	// GAME LOOP VARIABLES
	float64 seconds_counter = 0.0;
	s32 frame_count = 0;
	float64 last_time = os_get_elapsed_seconds();

	// MAIN GAME LOOP
	while (!window.should_close)
	{
		reset_temporary_storage();

		// Setup the camera projection and transformation
		draw_frame.projection = m4_make_orthographic_projection(window.width * -0.5, window.width * 0.5, window.height * -0.5, window.height * 0.5, -1, 10);
		float zoom = 1.0;
		draw_frame.camera_xform = m4_make_scale(v3(1.0 / zoom, 1.0 / zoom, 1.0));

		// Calculate delta time
		float64 now = os_get_elapsed_seconds();
		float64 delta_t = now - last_time;
		last_time = now;

		// Update OS events (input handling, etc.)
		os_update();

		// RENDERING
		for (int i = 0; i < MAX_ENTITY_COUNT; i++)
		{
			Entity *en = &world->entities[i];
			if (en->is_valid)
			{
				// Render the entity based on its archetype and sprite
				switch (en->arch)
				{
				default:
				{
					Sprite *sprite = get_sprite(en->sprite_id);
					Matrix4 xform = m4_scalar(1.0);
					xform = m4_translate(xform, v3(en->pos.x, en->pos.y, 0));
					xform = m4_translate(xform, v3(sprite->size.x * -0.5, 0.0, 0));
					draw_image_xform(sprite->image, xform, sprite->size, COLOR_WHITE);
				}
				break;
				}
			}
		}

		// FPS counter (logs frames per second every second)
		// seconds_counter += delta_t;
		// frame_count += 1;
		// if (seconds_counter > 1.0)
		// {
		// 	log("fps: %i", frame_count);
		// 	seconds_counter = 0.0;
		// 	frame_count = 0;
		// }

		// Draw FPS Counter
		draw_fps_counter(delta_t, &seconds_counter, &frame_count, font_base);

		// Exit game if ESCAPE key is pressed
		if (is_key_just_pressed(KEY_ESCAPE))
		{
			window.should_close = true;
		}

		// PLAYER MOVEMENT INPUT HANDLING
		Vector2 input_axis = v2(0, 0);
		if (is_key_down('A'))
		{
			input_axis.x -= 1.0;
		}
		if (is_key_down('D'))
		{
			input_axis.x += 1.0;
		}
		if (is_key_down('S'))
		{
			input_axis.y -= 1.0;
		}
		if (is_key_down('W'))
		{
			input_axis.y += 1.0;
		}

		// Update player position based on input and delta time
		player_en->pos = v2_add(player_en->pos, v2_mulf(input_axis, 32.0 * delta_t));

		// Update graphics
		gfx_update();
	}

	return 0;
}
