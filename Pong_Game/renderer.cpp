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

internal void
draw_number(int number, float x, float y, float size, u32 color) {

	float half_size = size * .5f;
	bool drew_zero = false;      // To account for sole zero
	while (number || !drew_zero) {
		drew_zero = true;        // Set to true to prevent first place zero

		int digit = number % 10; // Obtain the curr. digit (one's, ten's etc.)
		number = number / 10;    // Reduce the no. so that one's digit can be obtained

		switch (digit) {
			case 0: {
				// Left side of zero
				draw_rect(x - size, y, half_size, 2.5f * size, color);

				// Right side of zero
				draw_rect(x + size, y, half_size, 2.5f * size, color);

				// Top side of zero
				draw_rect(x, y + size * 2.f, half_size, half_size, color);

				// Bottom side of zero
				draw_rect(x, y - size * 2.f, half_size, half_size, color);

				// Reduce x by 4 * size to allow for next ten's/hundred's digit to be printed
				x -= size * 4.f;
			} break;

			case 1: {
				draw_rect(x + size, y, half_size, 2.5f * size, color);
				x -= size * 2.f;
			} break;

			case 2: {
				// Top side of two
				draw_rect(x, y + size * 2.f, 1.5f * size, half_size, color);

				// Middle rectangle of two
				draw_rect(x, y, 1.5f * size, half_size, color);

				// Bottom rectangle of two
				draw_rect(x, y - size * 2.f, 1.5f * size, half_size, color);

				// Top right rectangle of two
				draw_rect(x + size, y + size, half_size, half_size, color);

				// Bottom left rectange of two
				draw_rect(x - size, y - size, half_size, half_size, color);

				x -= size * 4.f;
			} break;

			case 3: {
				// Top rectangle of three
				draw_rect(x - half_size, y + size * 2.f, size, half_size, color);

				// Middle rectangle of three
				draw_rect(x - half_size, y, size, half_size, color);

				// Bottom rectangle of three
				draw_rect(x - half_size, y - size * 2.f, size, half_size, color);

				// Vertical rectangle of three
				draw_rect(x + size, y, half_size, 2.5f * size, color);

				x -= size * 4.f;
			} break;

			case 4: {
				// Right Vertical rectangle of four
				draw_rect(x + size, y, half_size, 2.5f * size, color);

				// Left Vertical rectangle of four (smaller)
				draw_rect(x - size, y + size, half_size, 1.5f * size, color);

				// Horizontal rectangle of four
				draw_rect(x, y, half_size, half_size, color);

				x -= size * 4.f;
			} break;

			case 5: {
				// Five is horizontally flipped two

				// Horizontal rectangles are same as two
				draw_rect(x, y + size * 2.f, 1.5f * size, half_size, color);
				draw_rect(x, y, 1.5f * size, half_size, color);
				draw_rect(x, y - size * 2.f, 1.5f * size, half_size, color);

				// Vertical rectangles have been flipped horizontally
				draw_rect(x - size, y + size, half_size, half_size, color);
				draw_rect(x + size, y - size, half_size, half_size, color);

				x -= size * 4.f;
			} break;

			case 6: {
				// Top rectangle of six
				draw_rect(x + half_size, y + size * 2.f, size, half_size, color);

				// Middle rectangle of six
				draw_rect(x + half_size, y, size, half_size, color);

				// Bottom rectangle of six
				draw_rect(x + half_size, y - size * 2.f, size, half_size, color);

				// Left vertical rectangle of six (large)
				draw_rect(x - size, y, half_size, 2.5f * size, color);

				// Right vertical rectangle of six (small)
				draw_rect(x + size, y - size, half_size, half_size, color);

				x -= size * 4.f;
			} break;

			case 7: {
				// Vertical rectangle of seven
				draw_rect(x + size, y, half_size, 2.5f * size, color);

				// Horizontal rectangle of seven
				draw_rect(x - half_size, y + size * 2.f, size, half_size, color);

				x -= size * 4.f;
			} break;

			case 8: {
				// Left vertical rectangle of eight
				draw_rect(x - size, y, half_size, 2.5f * size, color);

				// Right vertical rectangle of eight
				draw_rect(x + size, y, half_size, 2.5f * size, color);

				// Three horizontal rectangles of eight
				draw_rect(x, y + size * 2.f, half_size, half_size, color);
				draw_rect(x, y - size * 2.f, half_size, half_size, color);
				draw_rect(x, y, half_size, half_size, color);

				x -= size * 4.f;
			} break;

			case 9: {
				// Top rectangle of nine
				draw_rect(x - half_size, y + size * 2.f, size, half_size, color);

				// Middle rectangle of nine
				draw_rect(x - half_size, y, size, half_size, color);

				// Bottom rectangle of nine 
				draw_rect(x - half_size, y - size * 2.f, size, half_size, color);

				// Large vertical rectangle of nine
				draw_rect(x + size, y, half_size, 2.5f * size, color);

				// Small vertical rectangle of nine (top left)
				draw_rect(x - size, y + size, half_size, half_size, color);

				x -= size * 4.f;
			} break;
		}

	}
}
