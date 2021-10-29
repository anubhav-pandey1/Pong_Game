internal void
render_background() {
	u32* pixel = (u32*)render_state.memory;
	for (int y = 0; y < render_state.height; y++) {
		for (int x = 0; x < render_state.width; x++) {
			*pixel++ = (y * y) / 4 + (x * x) / 3 - (x * y);
		}
	}
}

internal void
clear_screen(u32 color) {
	u32* pixel = (u32*)render_state.memory;
	for (int y = 0; y < render_state.height; y++) {
		for (int x = 0; x < render_state.width; x++) {
			*pixel++ = color;
		}
	}
}

internal void
draw_rect_in_pixels(int x0, int y0, int x1, int y1, u32 color) {
	// Clamp the rectangle coordinates to avoid memory access errors
	x0 = clamp(0, x0, render_state.width);
	x1 = clamp(0, x1, render_state.width);
	y0 = clamp(0, y0, render_state.height);
	y1 = clamp(0, y1, render_state.height);

	for (int y = y0; y < y1; y++) {
		u32* pixel = (u32*)render_state.memory + x0 + y*render_state.width;
		for (int x = x0; x < x1; x++) {
			*pixel++ = color;
		}
	}
}

global_variable float render_scale = 0.01f;

internal void
draw_rect(float x, float y, float half_size_x, float half_size_y, u32 color) {
	// Choose size scaler - width or height
	// Heigth allows for wider screens to view more of the buffer frame
	// Width allows for longer screens to view more of the buffer frame
	int size_scaler = render_state.height*render_scale;

	// Scale percentages to full screen size
	x *= size_scaler;
	y *= size_scaler;
	half_size_x *= size_scaler;
	half_size_y *= size_scaler;

	// Center the x and y coordinates
	x += render_state.width / 2.f;  // Shift x = 0 to center of screen horizontally
	y += render_state.height / 2.f; // Shift y = 0 to center of screen vertically

	// Get coordinates from floating points
	int x0 = x - half_size_x;
	int x1 = x + half_size_x;
	int y0 = y - half_size_y;
	int y1 = y + half_size_y;

	// Change to pixels and draw it
	draw_rect_in_pixels(x0, y0, x1, y1, color);
}

internal void
draw_bounds(float arena_hsx, float arena_hsy, float line_hsx, float line_hsy, float player_hsx, float arena_cvg, u32 color) {
	int gap = 2;

	// Central Line
	for (int y = -(int)arena_hsy + (int)line_hsy; y < (int)arena_hsy - (int)line_hsy; y += 2*line_hsy + gap) {
		draw_rect(0, y + gap, line_hsx, line_hsy, color);
	}

	// Right-arena player boundary
	for (int y = -(int)arena_hsy + (int)line_hsy; y < (int)arena_hsy - (int)line_hsy; y += line_hsy + .5f * gap) {
		draw_rect((1.f - arena_cvg)*arena_hsx - 2*player_hsx, y + gap, .5f*line_hsx, .5f*line_hsy, color);
	}

	// Left-arena player boundary
	for (int y = -(int)arena_hsy + (int)line_hsy; y < (int)arena_hsy - (int)line_hsy; y += line_hsy + .5f * gap) {
		draw_rect(-(1.f - arena_cvg) * arena_hsx + 2*player_hsx, y + gap, .5f * line_hsx, .5f * line_hsy, color);
	}
}
