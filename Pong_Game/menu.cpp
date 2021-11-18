
// Todo: Refactor into cleaner version

// Idea 1: Create a factory function to create menu screens
// Take inputs for num of options, array of options titles, array of text sizes, array of rect_sizes etc.


// Function to manipulate the current hot_menu_button using key1 and key2
internal void
move_hot_button(Input* input, int num_of_options, int& hot_menu_button, Button_Key key1, Button_Key key2) {
	if (pressed(key1)) {
		hot_menu_button = (num_of_options + hot_menu_button - 1) % num_of_options;
	}

	if (pressed(key2)) {
		hot_menu_button = (hot_menu_button + 1) % num_of_options;
	}
}

internal void
manage_menu(Input* input) {
	clear_screen(0x006400);

	// Main Menu
	if (current_menumode == MN_MAIN) {
		// Main Menu Header
		draw_rect(1, 35, 60, 15, 0x000000);
		draw_text("PING PONG", -50, 40, 2, 0xffffff);

		// Navigation between the three options
		move_hot_button(input, 3, hot_menu_button, BUTTON_UP, BUTTON_DOWN);

		// Play Game Selected
		if (hot_menu_button == 0) {
			draw_rect(-5, -2, 32, 8, 0x000000);
			draw_text("PLAY GAME", -30, 1, 1, 0xff0000);
			if (pressed(BUTTON_ENTER)) current_menumode = MN_PLAY;

			draw_text("VIEW STATS", -33, -20, 1, 0xffffff);
			draw_text("QUIT", -15, -38, 1, 0xffffff);
		}
		// View Stats Selected
		else if (hot_menu_button == 1) {
			draw_text("PLAY GAME", -30, 0, 1, 0xffffff);

			draw_rect(-5, -22, 33, 8, 0x000000);
			draw_text("VIEW STATS", -33, -19, 1, 0xff0000);
			if (pressed(BUTTON_ENTER)) current_menumode = MN_STATS;

			draw_text("QUIT", -15, -38, 1, 0xffffff);
		}
		// Quit Menu Selected
		else if (hot_menu_button == 2) {
			draw_text("PLAY GAME", -30, 0, 1, 0xffffff);
			draw_text("VIEW STATS", -33, -20, 1, 0xffffff);

			draw_rect(-5, -40, 15, 8, 0x000000);
			draw_text("QUIT", -15, -37, 1, 0xff0000);
			if (pressed(BUTTON_ENTER)) current_menumode = MN_QUIT;
		}
	}

	// Play Game Menu
	else if (current_menumode == MN_PLAY) {
		// Play Game Menu Header
		draw_rect(1, 35, 60, 15, 0x000000);
		draw_text("GAME MODE", -50, 40, 2, 0xffffff);

		// Go back to Main Menu
		if (pressed(BUTTON_ESC)) {
			current_menumode = MN_MAIN;
		}

		// Navigation between the two options
		move_hot_button(input, 2, hot_gameplay_button, BUTTON_LEFT, BUTTON_RIGHT);

		// Select gamemode to enter gameplay
		if (pressed(BUTTON_ENTER)) {
			current_gamemode = GM_GAMEPLAY;
			save_data.stats[NUM_OF_MATCHES1]++;                      // Entry point for GM_GAMEPLAY so increase stats here
			is_player2_ai = hot_gameplay_button ? 0 : 1;
			if (!is_player2_ai) save_data.stats[NUM_OF_MATCHES2]++;  // If Player 2 is not AI, increase the stats here
		}

		// Single Player Selected
		if (hot_gameplay_button == 0) {
			draw_rect(-42, -12, 42, 8, 0x000000);
			draw_text("SINGLE PLAYER", -80, -9, 1, 0xff0000);

			draw_text("MULTIPLAYER", 20, -10, 1, 0xffffff);
		}
		// Multiplayer Selected
		else {
			draw_text("SINGLE PLAYER", -80, -10, 1, 0xffffff);

			draw_rect(52, -12, 37, 8, 0x000000);
			draw_text("MULTIPLAYER", 20, -9, 1, 0xff0000);
		}
	}

	// Stats Menu
	else if (current_menumode == MN_STATS) {

		// Go back to Main Menu
		if (pressed(BUTTON_ESC)) {
			current_menumode = MN_MAIN;
		}

		// Stats Menu Header
		draw_rect(0, 37, 48, 8, 0x000000);

		// Navigate between Player 1 stats and Player 2 stats
		move_hot_button(input, 2, view_stats_menu, BUTTON_UP, BUTTON_DOWN);

		// Common Text to be rendered
		{
			draw_text("MATCHES PLAYED", -80, 20, 0.75, 0xffffff);
			draw_text("MATCHES WON", -80, 5, 0.75, 0xffffff);
			draw_text("MATCHES LOST", -80, -10, 0.75, 0xffffff);
			draw_text("POINTS SCORED", -80, -25, 0.75, 0xffffff);
			draw_text("POINTS LOST", -80, -40, 0.75, 0xffffff);
		}

		// View Stats for Player 1
		if (!view_stats_menu) {
			draw_text("PLAYER I STATS", -40, 40, 1, 0xffffff);

			// Matches played by Player 1
			draw_number(save_data.stats[NUM_OF_MATCHES1], 0, 18, 1.2, 0xff0000);

			// Matches won by Player 1
			draw_number(save_data.stats[MATCHES_WON1], 0, 3, 1.2, 0xff0000);

			// Matches lost by Player 1
			draw_number(save_data.stats[MATCHES_LOST1], 0, -12, 1.2, 0xff0000);

			// Points scored by Player 1
			draw_number(save_data.stats[POINTS_SCORED1], 0, -27, 1.2, 0xff0000);

			// Points lost by Player 1
			draw_number(save_data.stats[POINTS_LOST1], 0, -42, 1.2, 0xff0000);
		}

		// View Stats for Player 2
		else {
			draw_text("PLAYER II STATS", -43, 40, 1, 0xffffff);

			// Matches played by Player 2
			draw_number(save_data.stats[NUM_OF_MATCHES2], 0, 18, 1.2, 0xff0000);

			// Matches won by Player 2
			draw_number(save_data.stats[MATCHES_WON2], 0, 3, 1.2, 0xff0000);

			// Matches lost by Player 2
			draw_number(save_data.stats[MATCHES_LOST2], 0, -12, 1.2, 0xff0000);

			// Points scored by Player 2
			draw_number(save_data.stats[POINTS_SCORED2], 0, -27, 1.2, 0xff0000);

			// Points lost by Player 2
			draw_number(save_data.stats[POINTS_LOST2], 0, -42, 1.2, 0xff0000);
		}
	}

	// Quit Menu
	else if (current_menumode == MN_QUIT) {
		// Quit Menu Header
		draw_rect(1, 35, 60, 15, 0x000000);
		draw_text("QUIT GAME", -50, 40, 2, 0xffffff);

		draw_text("I WANT TO QUIT THE GAME", -68, 5, 1, 0xffffff);

		// Go back to Main Menu
		if (pressed(BUTTON_ESC)) {
			current_menumode = MN_MAIN;
		}

		// Navigation between the two options
		move_hot_button(input, 2, hot_quit_button, BUTTON_LEFT, BUTTON_RIGHT);

		// Select whether to quit or not
		if (pressed(BUTTON_ENTER)) {
			if (hot_quit_button == 0) running = false;
			else current_menumode = MN_MAIN;
		}

		// Yes Selected
		if (hot_quit_button == 0) {
			draw_rect(-22, -18, 20, 8, 0x000000);
			draw_text("YES", -30, -15, 1, 0xff0000);

			draw_text("NO", 20, -16, 1, 0xffffff);
		}
		// No Selected
		else {
			draw_text("YES", -30, -16, 1, 0xffffff);

			draw_rect(26, -18, 20, 8, 0x000000);
			draw_text("NO", 20, -15, 1, 0xff0000);
		}
	}
}
