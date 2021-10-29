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

// ------------------ (1) Environment data -----------------------
// Arena Data
const float arena_px = 0.f;
const float arena_py = 0.f;
const float arena_half_size_x = 90.f;
const float arena_half_size_y = 45.f;

// Ball Data
float ball_hsx = 1.f;
float ball_hsy = 1.f;
float ball_py = 0.f, ball_dpy = 0.f;
float ball_px = 0.f, ball_dpx = 75.f;

// Common Player Data
float player_hsx = 2.5f;
float player_hsy = 12.f;
float player_px = 80.f;
float arena_coverage = .6f;             // 60% of each arena side allowed to move in

// ------------------ (2) Player1 data ---------------------------
float player1_py, player1_dpy;          // Speed in units per second
float player1_px = player_px, player1_dpx;
float player1_half_size_x = player_hsx;
float player1_half_size_y = player_hsy;
int player1_score = 0;

// ------------------ (3) Player2 data ---------------------------
float player2_py, player2_dpy;          // Speed in units per second
float player2_px = -player_px, player2_dpx;
float player2_half_size_x = player_hsx;
float player2_half_size_y = player_hsy;
int player2_score = 0;

// ------------------ (4) Helper Functions -----------------------

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

internal void
simulate_game(Input* input, float dt) {

	clear_screen(0x006400);

	// ------------- (5) Env Simulation ---------------------------
	// Draw the central arena
	draw_rect(arena_px, arena_py, arena_half_size_x, arena_half_size_y, 0x000000);
	draw_bounds(arena_half_size_x, arena_half_size_y, 1, 3, player_hsx, arena_coverage, 0xc0c0c0);

	// Simulate the ball
	{
		// Equations of Motion:-
		ball_px += ball_dpx * dt;
		ball_py += ball_dpy * dt;

		// Ball Collision with Players :-
		// aabb_vs_aabb() checks if there is a collision on any side
		if (aabb_vs_aabb(ball_px, ball_py, ball_hsx, ball_hsy, player1_px, player1_py, player_hsx, player_hsy)) {

			if (player1_px > ball_px) {     // If the ball collides on the left side of player 1
				ball_dpx *= -1.f;
				ball_px = player1_px - player_hsx - ball_hsy;
			}
			else {                          // If the ball collides on the right side of player 1
				ball_dpx *= 1.1f;
				ball_px = player1_px + player_hsx + ball_hsy;
			}
			ball_dpy = player1_dpy * .75f;  // Bouncing back effect
		}
		else if (aabb_vs_aabb(ball_px, ball_py, ball_hsx, ball_hsy, player2_px, player2_py, player_hsx, player_hsy)) {

			if (player2_px < ball_px) {     // If the ball collides on the right side of player 2
				ball_dpx *= -1.f;
				ball_px = player2_px + player_hsx + ball_hsy;
			}
			else {                          // If the ball collides on the left side of player 2
				ball_dpx *= 1.1f;
				ball_px = player2_px - player_hsx - ball_hsy;
			}
			ball_dpy = player2_dpy * .75f;  // Bouncing back effect
		}

		// Ball Collision with Arena Top and Bottom
		if (ball_py + ball_hsy > arena_half_size_y) {
			ball_py = arena_half_size_y - ball_hsy;
			ball_dpy *= -1;                 // Bouncing back effect
		}
		else if (ball_py - ball_hsy < -arena_half_size_y) {
			ball_py = -arena_half_size_y + ball_hsy;
			ball_dpy *= -1;                 // Bouncing back effect
		}

		// Reset: Ball Collision with Arena Left and Right
		if (ball_px + ball_hsx > 99.f) {       // Ball collision with right side of screen instead of arena
			ball_px = 0;
			ball_py = 0;
			ball_dpx *= -1;
			ball_dpy = 0;
			player2_score++;
		}
		else if (ball_px + ball_hsx < -99.f) { // Ball collision with left side of screen instead of arena
			ball_px = 0;
			ball_py = 0;
			ball_dpx *= -1;
			ball_dpy = 0;
			player1_score++;
		}
	}

	// ------------- (6) Player 1 Simulation ----------------------
	float player1_ddpy = 0.f, player1_ddpx = 0.f;
	if (is_down(BUTTON_UP)) player1_ddpy += 650.f;
	if (is_down(BUTTON_DOWN)) player1_ddpy -= 650.f;
	if (is_down(BUTTON_RIGHT)) player1_ddpx += 350.f;
	if (is_down(BUTTON_LEFT)) player1_ddpx -= 350.f;

	simulate_player(&player1_py, &player1_dpy, player1_ddpy, &player1_px, &player1_dpx, player1_ddpx, dt);

	// ------------- (7) Player 2 Simulation ----------------------
	float player2_ddpy = 0.f, player2_ddpx = 0.f;
	if (is_down(BUTTON_W)) player2_ddpy += 650.f;
	if (is_down(BUTTON_S)) player2_ddpy -= 650.f;
	if (is_down(BUTTON_D)) player2_ddpx += 350.f;
	if (is_down(BUTTON_A)) player2_ddpx -= 350.f;

	simulate_player(&player2_py, &player2_dpy, player2_ddpy, &player2_px, &player2_dpx, player2_ddpx, dt);

	// ------------- (8) Rendering --------------------------------
	draw_rect(ball_px, ball_py, ball_hsx, ball_hsy, 0xffff66);
	draw_rect(player1_px, player1_py, player1_half_size_x, player1_half_size_y, 0x8B0000);
	draw_rect(player2_px, player2_py, player2_half_size_x, player2_half_size_y, 0x8B0000);
}
