typedef enum EntityArchetype {
	arch_nil = 0,
	arch_player = 1,
	arch_tree = 2,
	arch_rock = 3,
} EntityArchetype;

typedef struct Entity {
	bool is_valid;
	EntityArchetype arch;
	Vector2 pos;
} Entity;
#define MAX_ENTITY_COUNT 1024

typedef struct World {
	Entity entities[MAX_ENTITY_COUNT];
} World;
World* world = 0;

Entity* entity_create() {
	Entity* entity_found = 0;
	for (int i = 0; i < MAX_ENTITY_COUNT; i++) {
		Entity* existing_entity = &world->entities[i];
		if (!existing_entity->is_valid) {
			entity_found = existing_entity;
			break;
		}
	}
	assert(entity_found, "No more free entities!");
	return entity_found;
}

void entity_destroy(Entity* entity) {
	memset(entity, 0, sizeof(Entity));
}

void setup_rock(Entity* en) {
	en->arch = arch_rock;
	//...
}

int entry(int argc, char **argv) {
	
	window.title = STR("Willowbrook");
	window.scaled_width = 1280; // We need to set the scaled size if we want to handle system scaling (DPI)
	window.scaled_height = 720; 
	window.x = 200;
	window.y = 90;
	window.clear_color = hex_to_rgba(0x2D2D34ff);

	world = alloc(get_heap_allocator(), sizeof(world));

	// Load Tileset
	Gfx_Image* player = load_image_from_disk(fixed_string("assets/player.png"), get_heap_allocator());
	assert(player, "tileset failed to load");
	Gfx_Image* rock = load_image_from_disk(fixed_string("assets/rock.png"), get_heap_allocator());
	assert(player, "tileset failed to load");

	Entity* player_en = entity_create();

	for (int i = 0; i < 10; i++) {
		Entity* en = entity_create();
		setup_rock(en);
		en->pos = v2(get_random_float32_in_range(-10.0f, 10.0f),get_random_float32_in_range(-10.0f, 10.0f));
	}

	float64 seconds_counter = 0.0;
	s32 frame_count = 0;

	float64 last_time = os_get_current_time_in_seconds();
	 

	while (!window.should_close) {
		reset_temporary_storage();

		draw_frame.projection = m4_make_orthographic_projection(window.width * -0.5, window.width * 0.5, window.height * -0.5, window.height * 0.5, -1, 10);

		float zoom = 5.3;
		draw_frame.view = m4_make_scale(v3(1.0/zoom, 1.0/zoom, 1.0));

		float64 now = os_get_current_time_in_seconds();
		float64 delta_t = now - last_time;
		last_time = now;
		
		os_update(); 



		if (is_key_just_pressed(KEY_ESCAPE)) {
			window.should_close = true;
		}

		Vector2 input_axis = v2(0, 0);
		if (is_key_down('A')) {
			input_axis.x -= 1.0;
		}
		if (is_key_down('D')) {
			input_axis.x += 1.0;
		}
		if (is_key_down('S')) {
			input_axis.y -= 1.0;
		}
		if (is_key_down('W')) {
			input_axis.y += 1.0;
		}

		player_en->pos = v2_add(player_en->pos, v2_mulf(input_axis, 32.0 * delta_t));

		{
			Vector2 size = v2(16.0, 16.0);
			Matrix4 xform = m4_scalar(1.0);
			xform = m4_translate(xform, v3(player_en->pos.x, player_en->pos.y, 0));
			draw_image_xform(player, xform, size, COLOR_RED);
			xform = m4_translate(xform, v3(size.x * -0.5, 0.0, 0));
		}

		{
			Vector2 size = v2(16.0, 16.0);
			Matrix4 xform = m4_scalar(1.0);
			xform = m4_translate(xform, v3(size.x * -0.5, 0.0, 0));
			draw_image_xform(rock, xform, size, COLOR_RED);
		}
		
		
		gfx_update();
		seconds_counter += delta_t;
		frame_count += 1;
		if (seconds_counter > 1.0) {
			log("fps: %i", frame_count);
			seconds_counter  = 0.0;
			frame_count = 0;
		}
	}

	return 0;
}