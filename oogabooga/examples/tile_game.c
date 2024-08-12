

#define X_TILE_COUNT 128
#define Y_TILE_COUNT 128

#define TILE_WIDTH 64
#define TILE_HEIGHT 64
#define TILE_COUNT (TILE_WIDTH * TILE_HEIGHT)

#define WORLD_WIDTH  (X_TILE_COUNT * TILE_WIDTH)
#define WORLD_HEIGHT (Y_TILE_COUNT * TILE_HEIGHT)

#define Z_LAYER_TILE_GRID -1
#define Z_LAYER_TILE_LAYER_BASE 100

#define Z_LAYER_EDITOR_GUI 200

#define MAX_LAYERS 6

Gfx_Font *font; // Declare the font at a global scope

typedef struct Tile_Layer {
	Gfx_Image *tile_images[TILE_COUNT];
} Tile_Layer;
Tile_Layer tile_layers[MAX_LAYERS] = {0};

u64 get_tile_index(s32 x, s32 y) {
	return y * X_TILE_COUNT + x;
}
u64 get_tile_index_from_pos(Vector2 p) {
	s32 x = (s32)((p.x/WORLD_WIDTH)*X_TILE_COUNT);
	s32 y = (s32)((p.y/WORLD_HEIGHT)*Y_TILE_COUNT);
	return get_tile_index(x, y);
}

typedef enum {
	APP_STATE_EDITING,
	APP_STATE_PLAYING
} App_State;

float32 delta_time = 0;
App_State app_state = APP_STATE_EDITING;

s64 current_tile_layer = 0;

Matrix4 camera_view;

void update_editor();
void update_game();

int entry(int argc, char **argv) {
	
	window.title = STR("Tile game");
	window.scaled_width = 1280; // We need to set the scaled size if we want to handle system scaling (DPI)
	window.scaled_height = 720; 
	window.x = 200;
	window.y = 90;
	window.clear_color = hex_to_rgba(0x6495EDff);

	font = load_font_from_disk(STR("C:/windows/fonts/arial.ttf"), get_heap_allocator());
    assert(font, "Failed loading arial.ttf");
	
	camera_view = m4_scalar(1.0);

	float64 last_time = os_get_current_time_in_seconds();
	while (!window.should_close) {
		reset_temporary_storage();
		
		draw_frame.projection = m4_make_orthographic_projection(window.pixel_width * -0.5, window.pixel_width * 0.5, window.pixel_height * -0.5, window.pixel_height * 0.5, -1, 10);
		draw_frame.enable_z_sorting = true;
		
		float64 now = os_get_current_time_in_seconds();
		delta_time = (float32)(now - last_time);
		last_time = now;
		
		if (app_state == APP_STATE_EDITING) {
			update_editor();
		} else if (app_state == APP_STATE_PLAYING) {
			update_game();
		}
		
		os_update(); 
		gfx_update();
	}

	return 0;
}

Vector2 screen_to_world(Vector2 screen) {
	Matrix4 proj = draw_frame.projection;
	Matrix4 view = draw_frame.view;
	float window_w = window.width;
	float window_h = window.height;

	// Normalize the mouse coordinates
	float ndc_x = (screen.x / (window_w * 0.5f)) - 1.0f;
	float ndc_y = (screen.y / (window_h * 0.5f)) - 1.0f;

	// Transform to world coordinates
	Vector4 world_pos = v4(ndc_x, ndc_y, 0, 1);
	world_pos = m4_transform(m4_inverse(proj), world_pos);
	world_pos = m4_transform(view, world_pos);
	
	return world_pos.xy;
}

Vector2 get_mouse_world_pos() {
	return screen_to_world(v2(input_frame.mouse_x, input_frame.mouse_y));
}

// used to print text
void int_to_string(int value, char* buffer) {
    char* ptr = buffer;
    if (value < 0) {
        *ptr++ = '-';
        value = -value;
    }
    
    // Convert the number to string (reverse order)
    char* start = ptr;
    do {
        *ptr++ = '0' + (value % 10);
        value /= 10;
    } while (value > 0);
    
    *ptr = '\0';
    
    // Reverse the string to correct the order
    char* end = ptr - 1;
    while (start < end) {
        char temp = *start;
        *start++ = *end;
        *end-- = temp;
    }
}

void update_editor() {
    const float32 cam_move_speed = 400.0;
    Vector2 cam_move_axis = v2(0, 0);
    if (is_key_down('A')) {
        cam_move_axis.x -= 1.0;
    }
    if (is_key_down('D')) {
        cam_move_axis.x += 1.0;
    }
    if (is_key_down('S')) {
        cam_move_axis.y -= 1.0;
    }
    if (is_key_down('W')) {
        cam_move_axis.y += 1.0;
    }
    
    Vector2 cam_move = v2_mulf(cam_move_axis, delta_time * cam_move_speed);
    camera_view = m4_translate(camera_view, v3(v2_expand(cam_move), 0));
    draw_frame.view = camera_view;

    // Define the fixed range for tiles
    int start_tile = -32;
    int end_tile = 32;

    // Draw the grid with coordinates
    for (int i = start_tile; i <= end_tile; i++) {
        for (int j = start_tile; j <= end_tile; j++) {
            Vector2 pos = v2(i * TILE_WIDTH, j * TILE_HEIGHT);
            Vector4 tile_color = ((i + j) % 2 == 0) ? v4(1.0, 1.0, 1.0, 1.0) : v4(0.7, 0.7, 0.7, 1.0);
            draw_rect(pos, v2(TILE_WIDTH, TILE_HEIGHT), tile_color);
            
            // Draw the grid coordinates as text
            char text[32];
            char* text_ptr = text;
            *text_ptr++ = '(';
            int_to_string(i, text_ptr);
            while (*text_ptr) text_ptr++;
            *text_ptr++ = ',';
            *text_ptr++ = ' ';
            int_to_string(j, text_ptr);
            while (*text_ptr) text_ptr++;
            *text_ptr++ = ')';
            *text_ptr = '\0';

            Vector2 text_pos = v2_add(pos, v2(5, TILE_HEIGHT / 2 - 10)); // Adjust the position for centering
            draw_text(font, STR(text), 16, text_pos, v2(1.0, 1.0), v4(0.0, 0.0, 0.0, 1.0));
        }
    }
}

void _update_editor_checker_grid_text() {
    const float32 cam_move_speed = 400.0;
    Vector2 cam_move_axis = v2(0, 0);
    if (is_key_down('A')) {
        cam_move_axis.x -= 1.0;
    }
    if (is_key_down('D')) {
        cam_move_axis.x += 1.0;
    }
    if (is_key_down('S')) {
        cam_move_axis.y -= 1.0;
    }
    if (is_key_down('W')) {
        cam_move_axis.y += 1.0;
    }
    
    Vector2 cam_move = v2_mulf(cam_move_axis, delta_time * cam_move_speed);
    camera_view = m4_translate(camera_view, v3(v2_expand(cam_move), 0));
    draw_frame.view = camera_view;

    // Calculate the number of tiles to fit the screen
    int tile_count_x = window.width / TILE_WIDTH;
    int tile_count_y = window.height / TILE_HEIGHT;

    // Adjust the start position to (0, 0)
    Vector2 start_pos = v2(0, 0);

    // Draw the expanded grid with coordinates
    for (int i = 0; i <= tile_count_x; i++) {
        for (int j = 0; j <= tile_count_y; j++) {
            Vector2 pos = v2_add(start_pos, v2(i * TILE_WIDTH, j * TILE_HEIGHT));
            Vector4 tile_color = ((i + j) % 2 == 0) ? v4(1.0, 1.0, 1.0, 1.0) : v4(0.7, 0.7, 0.7, 1.0);
            draw_rect(pos, v2(TILE_WIDTH, TILE_HEIGHT), tile_color);
            
            // Draw the grid coordinates as text
            char text[32];
            char* text_ptr = text;
            *text_ptr++ = '(';
            int_to_string(i, text_ptr);
            while (*text_ptr) text_ptr++;
            *text_ptr++ = ',';
            *text_ptr++ = ' ';
            int_to_string(j, text_ptr);
            while (*text_ptr) text_ptr++;
            *text_ptr++ = ')';
            *text_ptr = '\0';
            
            Vector2 text_pos = v2_add(pos, v2(5, TILE_HEIGHT / 2 - 10)); // Adjust the position for centering
            draw_text(font, STR(text), 16, text_pos, v2(1.0, 1.0), v4(0.0, 0.0, 0.0, 1.0));
        }
    }
}


void _update_editor_checker_screen() {
    const float32 cam_move_speed = 400.0;
    Vector2 cam_move_axis = v2(0, 0);
    if (is_key_down('A')) {
        cam_move_axis.x -= 1.0;
    }
    if (is_key_down('D')) {
        cam_move_axis.x += 1.0;
    }
    if (is_key_down('S')) {
        cam_move_axis.y -= 1.0;
    }
    if (is_key_down('W')) {
        cam_move_axis.y += 1.0;
    }
    
    Vector2 cam_move = v2_mulf(cam_move_axis, delta_time * cam_move_speed);
    camera_view = m4_translate(camera_view, v3(v2_expand(cam_move), 0));
    draw_frame.view = camera_view;

    // Calculate the number of tiles to fit the screen
    int tile_count_x = window.width / TILE_WIDTH;
    int tile_count_y = window.height / TILE_HEIGHT;

    // Adjust the start position to center the grid
    Vector2 start_pos = v2(-tile_count_x * TILE_WIDTH / 2, -tile_count_y * TILE_HEIGHT / 2);

    // Draw the expanded grid
    for (int i = 0; i <= tile_count_x; i++) {
        for (int j = 0; j <= tile_count_y; j++) {
            Vector2 pos = v2_add(start_pos, v2(i * TILE_WIDTH, j * TILE_HEIGHT));
            Vector4 tile_color = ((i + j) % 2 == 0) ? v4(1.0, 1.0, 1.0, 1.0) : v4(0.7, 0.7, 0.7, 1.0);
            draw_rect(pos, v2(TILE_WIDTH, TILE_HEIGHT), tile_color);
        }
    }
}



// This renders a checker box
void _update_editor_checker_box() {
    const float32 cam_move_speed = 400.0;
    Vector2 cam_move_axis = v2(0, 0);
    if (is_key_down('A')) {
        cam_move_axis.x -= 1.0;
    }
    if (is_key_down('D')) {
        cam_move_axis.x += 1.0;
    }
    if (is_key_down('S')) {
        cam_move_axis.y -= 1.0;
    }
    if (is_key_down('W')) {
        cam_move_axis.y += 1.0;
    }
    
    Vector2 cam_move = v2_mulf(cam_move_axis, delta_time * cam_move_speed);
    camera_view = m4_translate(camera_view, v3(v2_expand(cam_move), 0));
    draw_frame.view = camera_view;
    
    // Draw a small grid of tiles centered on the screen
    int grid_size = 5;  // 5x5 grid
    for (int tile_x = -grid_size; tile_x <= grid_size; tile_x++) {
        for (int tile_y = -grid_size; tile_y <= grid_size; tile_y++) {
            Vector2 pos = v2(tile_x * TILE_WIDTH, tile_y * TILE_HEIGHT);
            Vector4 tile_color = ((tile_x + tile_y) % 2 == 0) ? v4(1.0, 1.0, 1.0, 1.0) : v4(0.7, 0.7, 0.7, 1.0);
            draw_rect(pos, v2(TILE_WIDTH, TILE_HEIGHT), tile_color);
        }
    }
}

// This renders a single box in the center
void _update_editor_center_box() {
    const float32 cam_move_speed = 400.0;
    Vector2 cam_move_axis = v2(0, 0);
    if (is_key_down('A')) {
        cam_move_axis.x -= 1.0;
    }
    if (is_key_down('D')) {
        cam_move_axis.x += 1.0;
    }
    if (is_key_down('S')) {
        cam_move_axis.y -= 1.0;
    }
    if (is_key_down('W')) {
        cam_move_axis.y += 1.0;
    }
    
    Vector2 cam_move = v2_mulf(cam_move_axis, delta_time * cam_move_speed);
    camera_view = m4_translate(camera_view, v3(v2_expand(cam_move), 0));
    draw_frame.view = camera_view;
    
    // Correctly calculate the center position
    Vector2 center_pos = v2(-TILE_WIDTH / 2, -TILE_HEIGHT / 2);
    Vector4 tile_color = v4(1.0, 1.0, 1.0, 1.0); // White tile

    // Draw the tile in the center
    draw_rect(center_pos, v2(TILE_WIDTH, TILE_HEIGHT), tile_color);

    // No need to pop_z_layer here since no Z-layer was pushed
}

void _update_editor() {
	
	const float32 cam_move_speed = 400.0;
	Vector2 cam_move_axis = v2(0, 0);
	if (is_key_down('A')) {
		cam_move_axis.x -= 1.0;
	}
	if (is_key_down('D')) {
		cam_move_axis.x += 1.0;
	}
	if (is_key_down('S')) {
		cam_move_axis.y -= 1.0;
	}
	if (is_key_down('W')) {
		cam_move_axis.y += 1.0;
	}
	
	Vector2 cam_move = v2_mulf(cam_move_axis, delta_time * cam_move_speed);
	camera_view = m4_translate(camera_view, v3(v2_expand(cam_move), 0));
	draw_frame.view = camera_view;
	
	Vector2 bottom_left = screen_to_world(v2(-window.width/2, -window.height/2));
	Vector2 top_right   = screen_to_world(v2( window.width/2,  window.height/2));
	
	Vector2 origin = v2(-WORLD_WIDTH/2, -WORLD_HEIGHT/2);
	int first_visible_tile_x = ((origin.x + bottom_left.x)/WORLD_WIDTH) *X_TILE_COUNT;
	int first_visible_tile_y = ((origin.y + bottom_left.y)/WORLD_HEIGHT)*Y_TILE_COUNT;
	int last_visible_tile_x  = ((origin.x + top_right.x  )/WORLD_WIDTH) *X_TILE_COUNT;
	int last_visible_tile_y  = ((origin.y + top_right.y  )/WORLD_HEIGHT)*Y_TILE_COUNT;
	
	// Visualize empty tile grid & react to mouse
	push_z_layer(Z_LAYER_TILE_GRID);
	
	for (s32 tile_x = first_visible_tile_x; tile_x <= last_visible_tile_x; tile_x += 1) {
		for (s32 tile_y = first_visible_tile_y; tile_y <=last_visible_tile_y; tile_y += 1) {
			bool variation = (tile_x%2==0 && tile_y%2==1) || (tile_x%2==1 && tile_y%2==0);
			
			Vector2 pos = v2_add(origin, v2(tile_x*TILE_WIDTH, tile_y*TILE_HEIGHT));
			draw_rect(pos, v2(TILE_WIDTH, TILE_HEIGHT), variation?v4(.27,.27,.27,1):v4(.3,.3,.3,1));
			
			push_z_layer(Z_LAYER_EDITOR_GUI);
			
			float left = pos.x;
			float bottom = pos.y;
			float right = left + TILE_WIDTH;
			float top = bottom + TILE_HEIGHT;
			Vector2 m = get_mouse_world_pos();
			bool hovered = m.x >= left && m.x < right && m.y >= bottom && m.y < top;
			
			if (hovered) {
				draw_rect(pos, v2(TILE_WIDTH, TILE_HEIGHT), v4(0, 0, 0, 0.3));
			}
			
			pop_z_layer();
		}
	}
	pop_z_layer();
	
	for (int i = 0; i < MAX_LAYERS; i++) {
		Tile_Layer *layer = &tile_layers[i];
		push_z_layer(Z_LAYER_TILE_LAYER_BASE + i);
		Vector2 origin = v2(-WORLD_WIDTH/2, -WORLD_HEIGHT/2);
		for (s32 tile_x = first_visible_tile_x; tile_x <= last_visible_tile_x; tile_x += 1) {
			for (s32 tile_y = first_visible_tile_y; tile_y <=last_visible_tile_y; tile_y += 1) {
				Gfx_Image *img = layer->tile_images[get_tile_index(tile_x, tile_y)];
				if (img) {
					Vector2 pos = v2_add(origin, v2(tile_x*TILE_WIDTH, tile_y*TILE_HEIGHT));
					draw_image(img, pos, v2(TILE_WIDTH, TILE_HEIGHT), COLOR_WHITE);
				}
			}
		}
		pop_z_layer();
	}
}
void update_game() {
	
}