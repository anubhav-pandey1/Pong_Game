// <------------------------- Key State Checker Macros -------------------------------->

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
// <----------------------------------------------------------------------------------->


// <------------------------- Game Simulation ----------------------------------------->

// Game Mode
enum Gamemode {
	GM_MENU,
	GM_GAMEPLAY,
};

Gamemode current_gamemode = GM_MENU;

enum Menumode {
	MN_MAIN,
	MN_PLAY,
};

Menumode current_menumode = MN_MAIN;
int hot_gameplay_button;
int hot_menu_button;

// Game stats
bool game_paused = false;

// ------------------ (1) Environment data -----------------------
// Arena Data
const float arena_px = 0.f;
const float arena_py = 0.f;
const float arena_half_size_x = 90.f;
const float arena_half_size_y = 45.f;

// Ball Data
float ball_hsx = 1.f;
float ball_hsy = 1.f;
float ball_py = 0.f, ball_dpy = 1.f;
float ball_px = 0.f, ball_dpx = 100.f;
float ball_max_speed_x = 175.f;
float ball_min_speed_x = 25.f;

// Common Player Data
float player_hsx = 2.5f;
float player_hsy = 12.f;
float player_px = 80.f;
float arena_coverage = .6f;       // Default 60% of each arena side allowed to move in
bool is_player1_ai = false;       // By default, player 1 is for the user
bool is_player2_ai = true;        // By default, player 2 is the AI

// ------------------ (2) Player1 data ---------------------------
float player1_py, player1_dpy;          // Speed in units per second
float player1_px = player_px, player1_dpx;
float player1_half_size_x = player_hsx;
float player1_half_size_y = player_hsy;
bool player1_hit_ball = false;
int player1_score = 0;

// ------------------ (3) Player2 data ---------------------------
float player2_py, player2_dpy;          // Speed in units per second
float player2_px = -player_px, player2_dpx;
float player2_half_size_x = player_hsx;
float player2_half_size_y = player_hsy;
bool player2_hit_ball = false;
int player2_score = 0;

// ------------------ (4) Helper Functions -----------------------

// ----------------- Simulate Player Helper ----------------------
internal void
simulate_player(float* player_py, float* player_dpy, float player_ddpy,
	float* player_px, float* player_dpx, float player_ddpx, float dt) {

	// Friction:-
	player_ddpy -= *player_dpy * 1.5f; // Friction Coeff is 1.5
	player_ddpx -= *player_dpx * 1.5f; // Friction Coeff is 1.5

	// Equations of motion:-
	*player_py = *player_py + (*player_dpy * dt) + (player_ddpy * dt * dt * .5f);
	*player_dpy = *player_dpy + (player_ddpy * dt);
	*player_px = *player_px + (*player_dpx * dt) + (player_ddpx * dt * dt * .5f);
	*player_dpx = *player_dpx + (player_ddpx * dt);

	// Wall Collisions:-
	if (*player_py + player_hsy > arena_half_size_y) {
		*player_py = arena_half_size_y - player_hsy;
		*player_dpy *= -.05f;   // Bouncing back effect
	}
	else if (*player_py - player_hsy < -arena_half_size_y) {
		*player_py = -arena_half_size_y + player_hsy;
		*player_dpy *= -.05f;   // Bouncing back effect
	}

	if (*player_px + player_hsx > arena_half_size_x) {
		*player_px = arena_half_size_x - player_hsx;
		*player_dpx *= -.05f;   // Bouncing back effect
	}
	else if (*player_px - player_hsx < -arena_half_size_x) {
		*player_px = -arena_half_size_x + player_hsx;
		*player_dpx *= -.05f;   // Bouncing back effect
	}

	// Border Collisions:-
	if (*player_px > 0 && *player_px + player_hsx < (1.f - arena_coverage) * arena_half_size_x) {
		*player_px = (1.f - arena_coverage) * arena_half_size_x - player_hsx;
		*player_dpx *= -0.f;   // No bouncing back effect
	}
	else if (*player_px < 0 && *player_px - player_hsx > -(1.f - arena_coverage) * arena_half_size_x) {
		*player_px = -(1.f - arena_coverage) * arena_half_size_x + player_hsx;
		*player_dpx *= -0.f;   // No bouncing back effect
	}
}

// ----------------- AABB vs AABB Collision ----------------------
internal bool
aabb_vs_aabb(float ax, float ay, float a_hsx, float a_hsy,
	float bx, float by, float b_hsx, float b_hsy) {

	// a is ball, b are the players

	float right_ball = ax + a_hsx;
	float left_player = bx - b_hsx;
	float left_ball = ax - a_hsx;
	float right_player = bx + b_hsx;

	float top_ball = ay + a_hsy;
	float bottom_player = by - b_hsy;
	float bottom_ball = ay - a_hsy;
	float top_player = by + b_hsy;

	return (right_ball > left_player && // Collision in +ve X (left of player)
		left_ball < right_player &&     // Collision in -ve X (right of player)
		top_ball > bottom_player &&     // Collision in +ve Y (top side of player)
		bottom_ball < top_player);      // Collision in -ve Y (bottom side of player)
}

// ----------------- AI Simulation Helper -----------------------
internal void
simulate_ai(const float* player_px, const float* player_py, float* player_ddpx, float* player_ddpy, const bool* player_hit_ball) {
	float epsilon_y = 10.f;
	if (ball_py - *player_py > epsilon_y)
		*player_ddpy += 750.f * ((ball_py - *player_py) / arena_half_size_y);        // Acc. based on dist from ball
	else if (ball_py - *player_py < -epsilon_y)
		*player_ddpy -= 750.f * ((*player_py - ball_py) / arena_half_size_y);

	// TODO: Clean up AI code and create perfect mirrored AI for both sides
	if (player_px < 0 && (ball_px > (1.f - arena_coverage) * arena_half_size_x) ||  // If ball is in the other player's court
		// (ball_py - *player_py > epsilon_y) ||    // ie. Ball Y is greater than player_y: Out of vertical range
		// (ball_py - *player_py < -epsilon_y) ||   // // ie. Ball Y is lower than player_y: Out of vertical range
		*player_hit_ball)
		*player_ddpx += 350.f;
	else if	(player_px > 0 && (ball_px < -(1.f - arena_coverage) * arena_half_size_x) || // If ball is in the other player's court
		*player_hit_ball)
		*player_ddpx += 350.f;
	// If the ball is in the player's vertical range
	else if (player_px < 0 &&
		(ball_py > 0 && ball_py - *player_py < epsilon_y) ||
		(ball_py <= 0 && ball_py - *player_py > -epsilon_y))
		*player_ddpx -= 350.f;
	else if (player_px > 0 &&
		(ball_py > 0 && ball_py - *player_py < epsilon_y) ||
		(ball_py <= 0 && ball_py - *player_py > -epsilon_y))
		*player_ddpx -= 350.f;
}

internal void
simulate_game(Input* input, float dt) {
	// ------------------ Gameplay System ---------------------------------
	if (current_gamemode == GM_GAMEPLAY) {

		// Pause Screen
		if (pressed(BUTTON_P)) game_paused = !game_paused;
		draw_rect(0, 0, 30, 15, 0x006400);
		draw_text("PAUSED", -16, 2, 1, 0xffffff);

		// Escape to Main Menu
		if (pressed(BUTTON_ESC)) {
			current_menumode = MN_MAIN;
			current_gamemode = GM_MENU;
		}

		// Gameplay if game is not paused
		if (!game_paused) {

			// ------------- (5) Env Simulation ---------------------------

			// Draw the central arena
			draw_arena(arena_half_size_x, arena_half_size_y, 0x000000, 0x006400);
			draw_bounds(arena_half_size_x, arena_half_size_y, 1, 3, player_hsx, arena_coverage, 0xc0c0c0);

			// Simulate the ball
			{
				// Equations of Motion:-
				ball_px += ball_dpx * dt;
				ball_py += ball_dpy * dt;

				// Ball Collision with Players :-
				float back_hit_coeff_x = -1.1f;
				float front_hit_coeff_x = 1.1f;
				float player_transfer_coeff_x = .5f;
				float player_transfer_coeff_y = .75f;
				float ball_pos_transfer_coeff_y = 1.5f;
				// aabb_vs_aabb() checks if there is a collision on any side
				if (aabb_vs_aabb(ball_px, ball_py, ball_hsx, ball_hsy, player1_px, player1_py, player_hsx, player_hsy)) {

					player1_hit_ball = true;        // Set hit ball state for player 1 to be true
					player2_hit_ball = false;       // Reset hit ball state for player 2 to allow AI movement

					if (player1_px > ball_px) {     // If the ball collides on the left side of player 1
						ball_dpx *= back_hit_coeff_x;
						ball_px = player1_px - player_hsx - ball_hsy;
					}
					else {                          // If the ball collides on the right side of player 1
						ball_dpx *= front_hit_coeff_x;
						ball_dpx += player1_dpx * player_transfer_coeff_x;
						ball_px = player1_px + player_hsx + ball_hsy;
					}
					ball_dpy = player1_dpy * player_transfer_coeff_y;  // Bouncing back effect
					ball_dpy += ball_pos_transfer_coeff_y * (ball_py - player1_py);
				}
				else if (aabb_vs_aabb(ball_px, ball_py, ball_hsx, ball_hsy, player2_px, player2_py, player_hsx, player_hsy)) {

					player2_hit_ball = true;        // Set hit ball state for player 2 to be true
					player1_hit_ball = false;

					if (player2_px < ball_px) {     // If the ball collides on the right side of player 2
						ball_dpx *= back_hit_coeff_x;

						ball_px = player2_px + player_hsx + ball_hsy;
					}
					else {                          // If the ball collides on the left side of player 2
						ball_dpx *= front_hit_coeff_x;
						ball_dpx += player2_dpx * player_transfer_coeff_x;
						ball_px = player2_px - player_hsx - ball_hsy;
					}
					ball_dpy = player2_dpy * player_transfer_coeff_y;  // Bouncing back effect
					ball_dpy += ball_pos_transfer_coeff_y * (ball_py - player2_py);
				}

				// Clamping ball's x velocity to prevent large built-up speeds
				if (ball_dpx > ball_max_speed_x) ball_dpx = ball_max_speed_x;
				else if (ball_dpx < -ball_max_speed_x) ball_dpx = -ball_max_speed_x;
				else if (ball_dpx > 0 && ball_dpx < ball_min_speed_x) ball_dpx = ball_min_speed_x;
				else if (ball_dpx < 0 && ball_dpx > -ball_min_speed_x) ball_dpx = -ball_min_speed_x;

				// Ball Collision with Arena Top and Bottom
				if (ball_py + ball_hsy > arena_half_size_y) {
					ball_py = arena_half_size_y - ball_hsy;
					ball_dpy *= -1;                 // Bouncing back effect
				}
				else if (ball_py - ball_hsy < -arena_half_size_y) {
					ball_py = -arena_half_size_y + ball_hsy;
					ball_dpy *= -1;                 // Bouncing back effect
				}

				// Random Integer Helpers
				LARGE_INTEGER curr_time;              // ISO time stored using large_integer
				QueryPerformanceCounter(&curr_time);  // Store high resolution (<1us) time stamp in curr_time
				int currTime = curr_time.QuadPart;    // Convert time stamp to integer

				// Reset: Ball Collision with Arena Left and Right
				if (ball_px + ball_hsx > 99.f) {            // Ball collision with right side of screen instead of arena
					ball_px = 0;
					ball_py = 0;
					ball_dpx = -100.f;
					ball_dpy = currTime % 2 ? 30.f : -30.f; // Randomly decided spawn velocity direction of ball after reset
					player2_score++;
				}
				else if (ball_px + ball_hsx < -99.f) {      // Ball collision with left side of screen instead of arena
					ball_px = 0;
					ball_py = 0;
					ball_dpx = 100.f;
					ball_dpy = currTime % 2 ? -30.f : 30.f; // Randomly decided spawn velocity direction of ball after reset
					player1_score++;
				}
			}

			// ------------- (6) Player 1 Simulation ----------------------
			float player1_ddpy = 0.f, player1_ddpx = 0.f;
			if (!is_player1_ai) {
				if (is_down(BUTTON_UP)) player1_ddpy += 650.f;
				if (is_down(BUTTON_DOWN)) player1_ddpy -= 650.f;
				if (is_down(BUTTON_RIGHT)) player1_ddpx += 350.f;
				if (is_down(BUTTON_LEFT)) player1_ddpx -= 350.f;
			}
			else {
				simulate_ai(&player1_px, &player1_py, &player1_ddpx, &player1_ddpy, &player1_hit_ball);
			}

			simulate_player(&player1_py, &player1_dpy, player1_ddpy, &player1_px, &player1_dpx, player1_ddpx, dt);

			// ------------- (7) Player 2 Simulation ----------------------
			float player2_ddpy = 0.f, player2_ddpx = 0.f;
			if (!is_player2_ai) {
				if (is_down(BUTTON_W)) player2_ddpy += 650.f;
				if (is_down(BUTTON_S)) player2_ddpy -= 650.f;
				if (is_down(BUTTON_D)) player2_ddpx += 350.f;
				if (is_down(BUTTON_A)) player2_ddpx -= 350.f;
			}
			else {
				simulate_ai(&player2_px, &player2_py, &player2_ddpx, &player2_ddpy, &player2_hit_ball);
			}

			simulate_player(&player2_py, &player2_dpy, player2_ddpy, &player2_px, &player2_dpx, player2_ddpx, dt);

			// ------------- (8) Rendering --------------------------------
			draw_rect(ball_px, ball_py, ball_hsx, ball_hsy, 0xffff66);
			draw_rect(player1_px, player1_py, player1_half_size_x, player1_half_size_y, 0x8B0000);
			draw_rect(player2_px, player2_py, player2_half_size_x, player2_half_size_y, 0x8B0000);
			// Display Scores
			draw_number(player1_score, 10, 40, 1.f, 0xbbffbb);
			draw_number(player2_score, -10, 40, 1.f, 0xbbffbb);
		}
	}
	// ------------------ Menu System -------------------------------------
	// Todo: Refactor into cleaner version
	else if (current_gamemode == GM_MENU) {
		clear_screen(0x006400);

		// Main Menu
		if (current_menumode == MN_MAIN) {
			// Main Menu Header
			draw_rect(1, 35, 60, 15, 0x000000);
			draw_text("PING PONG", -50, 40, 2, 0xffffff);

			// Navigation between the two options
			if (pressed(BUTTON_UP) || pressed(BUTTON_DOWN)) {
				hot_menu_button = !hot_menu_button;
			}

			// Play Game Selected
			if (hot_menu_button == 0) {
				draw_rect(-5, -2, 32, 8, 0x000000);
				draw_text("PLAY GAME", -30, 1, 1, 0xff0000);
				if (pressed(BUTTON_ENTER)) current_menumode = MN_PLAY;

				draw_text("VIEW STATS", -33, -20, 1, 0xffffff);
			}
			// View Stats Selected
			else {
				draw_text("PLAY GAME", -30, 0, 1, 0xffffff);

				draw_rect(-5, -22, 33, 8, 0x000000);
				draw_text("VIEW STATS", -33, -19, 1, 0xff0000);
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
			if (pressed(BUTTON_LEFT) || pressed(BUTTON_RIGHT)) {
				hot_gameplay_button = !hot_gameplay_button;
			}

			// Select gamemode to enter gameplay
			if (pressed(BUTTON_ENTER)) {
				current_gamemode = GM_GAMEPLAY;
				is_player2_ai = hot_gameplay_button ? 0 : 1;
			}

			// Single Player Selected
			if (hot_gameplay_button == 0) {
				draw_rect(-42, -12, 42, 8, 0x000000);
				draw_text("SINGLE PLAYER", -80, -10, 1, 0xff0000);

				draw_text("MULTIPLAYER", 20, -10, 1, 0xffffff);
			}
			// Multiplayer Selected
			else {
				draw_text("SINGLE PLAYER", -80, -10, 1, 0xffffff);

				draw_rect(52, -12, 37, 8, 0x000000);
				draw_text("MULTIPLAYER", 20, -10, 1, 0xff0000);
			}
		}
	}
}
