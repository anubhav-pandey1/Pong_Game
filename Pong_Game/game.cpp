// -------------------------- Key State Checker Macros ---------------------------------

// Button b is down if the Button_State in buttons array at index b has is_down == true
// Deref input to get buttons array for the Button_State value BUTTON_UP to check its is_down
// is_down() can be used to check if the key is being pressed continuously
#define is_down(b) input->buttons[b].is_down

// Button b is pressed if it is currently down and has changed from the previous frame
// pressed() can be used to check if the key was pressed in the current frame or not
#define pressed(b) (input->buttons[b].is_down && input->buttons[b].changed)

// Button b is released if it is currently up (not down) and has changed from the previous frame
// released() can be used to check if the key was released in the current frame or not
#define released(b) (!input->buttons[b].is_down && input->buttons[b].changed)
// -------------------------------------------------------------------------------------


// -------------------------- Game Simulation ------------------------------------------

// ------------------ (1) Player data ---------------------------
float player_pos_y = 0.f;
float player_pos_x = 0.f;
float player_reg_speed = 7.5f; // Speed in units per sec

internal void
simulate_game(Input* input, float dt) {

	clear_screen(0xff5500);

	// Get player speed in units using delta time (independent of frame rate)
	float player_speed = player_reg_speed * dt;

	// Increase the player speed by a multiplier if SHIFT key is held down
	if (is_down(BUTTON_SHIFT)) player_speed *= 1.75f;

	// ------------- (2) Player Simulation ----------------------
	if (is_down(BUTTON_UP)) player_pos_y += player_speed;
	if (is_down(BUTTON_DOWN)) player_pos_y -= player_speed;
	if (is_down(BUTTON_RIGHT)) player_pos_x += player_speed;
	if (is_down(BUTTON_LEFT)) player_pos_x -= player_speed;


	draw_rect(player_pos_x, player_pos_y, 2, 2, 0x00ff22);

	draw_rect_in_pixels(50, 50, 200, 500, 0x00ff22);

	// Draw the central rect if the UP KEY is down (using is_down from the input struct)
	if (!is_down(BUTTON_UP)) // Deref input to get buttons array for the Button_State value BUTTON_UP to check its is_down
		draw_rect(75, 25, 2, 2, 0x00ff22);
}
