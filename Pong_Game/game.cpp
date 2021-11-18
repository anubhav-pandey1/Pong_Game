
// <------------------------- Game Simulation ----------------------------------------->

// Game Mode
enum Gamemode {
	GM_MENU,
	GM_GAMEPLAY,
	GM_PAUSED,
	GM_QUIT,
	GM_ENDSTATE,
};

Gamemode current_gamemode = GM_MENU;

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

// Ball Collision Coefficients
float ball_max_speed_x = 135.f;
float ball_min_speed_x = 25.f;
float ball_max_speed_y = 160.f;
float front_hit_coeff_x = -1.01f;       // Velocity dir should flip when ball hits the front side
float back_hit_coeff_x = 1.1f;          // Veclocity dir should remain the same when ball hits the back side
float player_transfer_coeff_x = .5f;
float player_transfer_coeff_y = .75f;
float ball_pos_transfer_coeff_y = 1.5f;

// Common Player Data
float player_hsx = 2.5f;
float player_hsy = 12.f;
float player_px = 80.f;
#define player_fixed_ddpx 300.f
#define player_fixed_ddpy 600.f
#define player_friction_coeff 1.75f

// Gameplay Data
const int win_score = 21;                 // Default 21 pts is the win condition
float arena_coverage = .6f;               // Default 60% of each arena side allowed to move in
bool is_player1_ai = false;               // By default, player 1 is for the user
bool is_player2_ai = true;                // By default, player 2 is the AI

enum PlayerNum {
	PLAYER_NULL,
	PLAYER_ONE,
	PLAYER_TWO,
};

PlayerNum which_player_won = PLAYER_NULL; // By default, no player has won

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

// <-------------------- Menu System ----------------------------->

enum Menumode {
	MN_MAIN,
	MN_PLAY,
	MN_STATS,
	MN_QUIT,
};

Menumode current_menumode = MN_MAIN;
int hot_gameplay_button;
int hot_menu_button;
int view_stats_menu;
int hot_quit_button;
bool game_paused = false;

// --------------------- Player stats ----------------------------
#include "save_stats.cpp"

// -------------------- Menu Function ----------------------------
#include "menu.cpp"

// ------------------ (4) Helper Functions -----------------------

// ----------------- Simulate Player Helper ----------------------
internal void
simulate_player(float* player_py, float* player_dpy, float player_ddpy,
	float* player_px, float* player_dpx, float player_ddpx, float dt) {

	// Friction:-
	player_ddpy -= (*player_dpy * player_friction_coeff);
	player_ddpx -= (*player_dpx * player_friction_coeff);

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

// ----------------- Code Refactor Helpers ----------------------
internal void
reset_game() {
	player1_score = 0, player2_score = 0;
	player1_px = player_px, player2_px = -player_px;
	player1_py = 0, player2_py = 0;
	player1_dpx = 0, player1_dpy = 0;
	player2_dpx = 0, player2_dpy = 0;
	ball_py = 0.f, ball_dpy = 1.f;
	ball_px = 0.f, ball_dpx = 100.f;
	current_menumode = MN_MAIN;
	current_gamemode = GM_MENU;
}

// ---------------- Main Game Simulation ------------------------
internal void
simulate_game(Input* input, float dt) {
	// ------------------ Gameplay System ---------------------------------
	if (current_gamemode == GM_GAMEPLAY) {

		// Pause Screen
		if (pressed(BUTTON_P)) current_gamemode = GM_PAUSED;

		// Escape to Main Menu
		if (pressed(BUTTON_ESC)) current_gamemode = GM_QUIT;

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

				// aabb_vs_aabb() checks if there is a collision on any side
				if (aabb_vs_aabb(ball_px, ball_py, ball_hsx, ball_hsy, player1_px, player1_py, player_hsx, player_hsy)) {

					player1_hit_ball = true;        // Set hit ball state for player 1 to be true
					player2_hit_ball = false;       // Reset hit ball state for player 2 to allow AI movement

					if (player1_px > ball_px) {     // If the ball collides on the left (front) side of player 1
						ball_dpx *= front_hit_coeff_x;
						ball_dpx += player1_dpx * player_transfer_coeff_x;
						ball_px = player1_px - player_hsx - ball_hsx;
					}
					else {                          // If the ball collides on the right (back) side of player 1
						ball_dpx *= back_hit_coeff_x;
						ball_px = player1_px + player_hsx + ball_hsx;
					}
					ball_dpy = player1_dpy * player_transfer_coeff_y;  // Bouncing back effect
					ball_dpy += ball_pos_transfer_coeff_y * (ball_py - player1_py);
				}
				else if (aabb_vs_aabb(ball_px, ball_py, ball_hsx, ball_hsy, player2_px, player2_py, player_hsx, player_hsy)) {

					player2_hit_ball = true;        // Set hit ball state for player 2 to be true
					player1_hit_ball = false;

					if (player2_px < ball_px) {     // If the ball collides on the right (front) side of player 2
						ball_dpx *= front_hit_coeff_x;
						ball_dpx += player2_dpx * player_transfer_coeff_x;
						ball_px = player2_px + player_hsx + ball_hsx;
					}
					else {                          // If the ball collides on the left (back) side of player 2
						ball_dpx *= back_hit_coeff_x;
						ball_px = player2_px - player_hsx - ball_hsx;
					}
					ball_dpy = player2_dpy * player_transfer_coeff_y;  // Bouncing back effect
					ball_dpy += ball_pos_transfer_coeff_y * (ball_py - player2_py);
				}

				// Clamping ball's x velocity to prevent large built-up speeds
				if (ball_dpx > ball_max_speed_x) ball_dpx = ball_max_speed_x;
				else if (ball_dpx < -ball_max_speed_x) ball_dpx = -ball_max_speed_x;
				else if (ball_dpx > 0 && ball_dpx < ball_min_speed_x) ball_dpx = ball_min_speed_x;
				else if (ball_dpx < 0 && ball_dpx > -ball_min_speed_x) ball_dpx = -ball_min_speed_x;

				// Clamping ball's y velocity to prevent large built-up speeds
				if (ball_dpy > ball_max_speed_y) ball_dpy = ball_max_speed_y;
				else if (ball_dpy < -ball_max_speed_y) ball_dpy = -ball_max_speed_y;

				// Ball Collision with Arena Top and Bottom => Affects ball_dpy
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

				// Player 1 lost point, Player 2 scored point
				if (ball_px + ball_hsx > 99.f) {            // Ball collision with right side of screen instead of arena
					ball_px = 0;
					ball_py = 0;
					ball_dpx = -100.f;
					ball_dpy = currTime % 2 ? 30.f : -30.f; // Randomly decided spawn velocity direction of ball after reset
					player2_score++;
					save_data.stats[POINTS_LOST1]++;
					if (!is_player2_ai) save_data.stats[POINTS_SCORED2]++;
					// Player 1 lost, Player 2 won
					if (player2_score == win_score) {
						save_data.stats[MATCHES_LOST1]++;
						if (!is_player2_ai) save_data.stats[MATCHES_WON2]++;
						which_player_won = PLAYER_TWO;
						current_gamemode = GM_ENDSTATE;
					}
				}
				// Player 1 scored point, Player 2 lost point
				else if (ball_px + ball_hsx < -99.f) {      // Ball collision with left side of screen instead of arena
					ball_px = 0;
					ball_py = 0;
					ball_dpx = 100.f;
					ball_dpy = currTime % 2 ? -30.f : 30.f; // Randomly decided spawn velocity direction of ball after reset
					player1_score++;
					save_data.stats[POINTS_SCORED1]++;
					if (!is_player2_ai) save_data.stats[POINTS_LOST2]++;
					// Player 1 won, Player 2 lost
					if (player1_score == win_score) {
						save_data.stats[MATCHES_WON1]++;
						if (!is_player2_ai) save_data.stats[MATCHES_LOST2]++;
						which_player_won = PLAYER_ONE;
						current_gamemode = GM_ENDSTATE;
					}
				}
			}

			// ------------- (6) Player 1 Simulation ----------------------
			float player1_ddpy = 0.f, player1_ddpx = 0.f;
			if (!is_player1_ai) {
				if (is_down(BUTTON_UP)) player1_ddpy += player_fixed_ddpy;
				if (is_down(BUTTON_DOWN)) player1_ddpy -= player_fixed_ddpy;
				if (is_down(BUTTON_RIGHT)) player1_ddpx += player_fixed_ddpx;
				if (is_down(BUTTON_LEFT)) player1_ddpx -= player_fixed_ddpx;
			}
			else {
				simulate_ai(&player1_px, &player1_py, &player1_ddpx, &player1_ddpy, &player1_hit_ball);
			}

			simulate_player(&player1_py, &player1_dpy, player1_ddpy, &player1_px, &player1_dpx, player1_ddpx, dt);

			// ------------- (7) Player 2 Simulation ----------------------
			float player2_ddpy = 0.f, player2_ddpx = 0.f;
			if (!is_player2_ai) {
				if (is_down(BUTTON_W)) player2_ddpy += player_fixed_ddpy;
				if (is_down(BUTTON_S)) player2_ddpy -= player_fixed_ddpy;
				if (is_down(BUTTON_D)) player2_ddpx += player_fixed_ddpx;
				if (is_down(BUTTON_A)) player2_ddpx -= player_fixed_ddpx;
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

	else if (current_gamemode == GM_PAUSED) {
		game_paused = true;
		if pressed(BUTTON_P) {
			game_paused = false;
			current_gamemode = GM_GAMEPLAY;
		}
		draw_rect(0, 0, 30, 15, 0x006400);
		draw_text("PAUSED", -16, 2, 1, 0xffffff);
	}

	else if (current_gamemode == GM_QUIT) {
		game_paused = true;
		draw_rect(0, 0, 80, 35, 0x006400);
		draw_text("I WANT TO QUIT THE GAME", -68, 15, 1, 0xffffff);

		// Navigation between the two options
		//if (pressed(BUTTON_LEFT) || pressed(BUTTON_RIGHT)) {
		//	hot_quit_button = !hot_quit_button;
		//}
		move_hot_button(input, 2, hot_quit_button, BUTTON_LEFT, BUTTON_RIGHT);

		// Select whether to quit or not
		if (pressed(BUTTON_ENTER)) {
			game_paused = false;
			if (hot_quit_button == 0) reset_game();
			else current_gamemode = GM_GAMEPLAY;
		}

		// Yes Selected
		if (hot_quit_button == 0) {
			draw_rect(-22, -8, 20, 8, 0x000000);
			draw_text("YES", -30, -5, 1, 0xff0000);

			draw_text("NO", 20, -6, 1, 0xffffff);
		}
		// No Selected
		else {
			draw_text("YES", -30, -6, 1, 0xffffff);

			draw_rect(26, -8, 20, 8, 0x000000);
			draw_text("NO", 20, -5, 1, 0xff0000);
		}
	}

	// ------------------ Menu System -------------------------------------
	else if (current_gamemode == GM_MENU) {
		load_game();
		manage_menu(input);
	}

	// ------------------ Endgame Management ------------------------------
	else if (current_gamemode == GM_ENDSTATE) {
		save_game();
		draw_rect(0, 0, 60, 30, 0x006400);

		if (which_player_won == PLAYER_ONE) {
			if (is_player2_ai) draw_text("YOU WON", -19, 12, 1, 0xffffff);
			else draw_text("PLAYER I WON", -35, 12, 1, 0xffffff);
		}
		else if (which_player_won == PLAYER_TWO) {
			if (is_player2_ai) draw_text("YOU LOST", -21, 12, 1, 0xffffff);
			else draw_text("PLAYER II WON", -36, 12, 1, 0xffffff);
		}

		draw_rect(1, -11, 20, 8, 0x000000);
		draw_text("OK", -4, -8, 1, 0xff0000);
		if pressed(BUTTON_ENTER) {
			reset_game();
		}
	}
}
