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
const float arena_half_size_x = 92.5f;
const float arena_half_size_y = 45.f;

// Ball Data
float ball_hsx = 1.f;
float ball_hsy = 1.f;
float ball_py = 0.f, ball_dpy = 0.f;
float ball_px = 0.f, ball_dpx = 20.f;

// Common Player Data
float player_hsx = 2.5f;
float player_hsy = 12.f;
float player_px = 80.f;


// ------------------ (2) Player1 data ---------------------------
float player1_py, player1_dpy;          // Speed in units per second
const float player1_px = player_px;
float player1_half_size_x = player_hsx;
float player1_half_size_y = player_hsy;
int player1_score = 0;

// ------------------ (3) Player2 data ---------------------------
float player2_py, player2_dpy;          // Speed in units per second
const float player2_px = -player_px;
float player2_half_size_x = player_hsx;
float player2_half_size_y = player_hsy;
int player2_score = 0;

// ------------------ (4) Helper Functions -----------------------

internal void
simulate_player(float* player_py, float* player_dpy, float player_ddpy, float dt) {

	// Friction:-
	player_ddpy -= *player_dpy * 1.25f;

	// Equations of motion:-
	*player_py = *player_py + (*player_dpy * dt) + (player_ddpy * dt * dt * .5f);
	*player_dpy = *player_dpy + (player_ddpy * dt);

	// Wall Collision:-
	if (*player_py + player_hsy > arena_half_size_y) {
		*player_py = arena_half_size_y - player_hsy;
		*player_dpy *= -.25f;   // Bouncing back effect
	}
	else if (*player_py - player_hsy < -arena_half_size_y) {
		*player_py = -arena_half_size_y + player_hsy;
		*player_dpy *= -.25f;   // Bouncing back effect
	}
}

internal bool
aabb_vs_aabb(float ax, float ay, float a_hsx, float a_hsy,
	float bx, float by, float b_hsx, float b_hsy) {

	return (ax + a_hsx > bx - b_hsx && // Collision in +ve X (a is ball, b is player)
		ax - a_hsx < bx + b_hsx &&     // Collision in -ve X (right side of player)
		ay + a_hsy > by - b_hsy &&     // Collision in +ve Y (top side of player)
		ay - a_hsy < by + b_hsy);      // Collision in -ve Y (bottom side of player)
}

internal void
simulate_game(Input* input, float dt) {

	clear_screen(0x006400);

	// ------------- (5) Env Simulation ---------------------------
	// Draw the central arena
	draw_rect(arena_px, arena_py, arena_half_size_x, arena_half_size_y, 0x000000);

	// Simulate the ball
	{
		// Equations of Motion:-
		ball_px += ball_dpx * dt;
		ball_py += ball_dpy * dt;

		// Ball Collision with Players :-
		if (aabb_vs_aabb(ball_px, ball_py, ball_hsx, ball_hsy, player1_px, player1_py, player_hsx, player_hsy)) {

			ball_px = player1_px - player_hsx - ball_hsy;
			ball_dpx *= -1;
			ball_dpy = player1_dpy * .75f;  // Bouncing back effect
		}
		else if (aabb_vs_aabb(ball_px, ball_py, ball_hsx, ball_hsy, player2_px, player2_py, player_hsx, player_hsy)) {

			ball_px = player2_px + player_hsx + ball_hsy;
			ball_dpx *= -1;
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
		if (ball_px + ball_hsx > arena_half_size_x) {       // Ball collision with right side of arena
			ball_px = 0;
			ball_py = 0;
			ball_dpx *= -1;
			ball_dpy = 0;
			player2_score++;
		}
		else if (ball_px + ball_hsx < -arena_half_size_x) { // Ball collision with left side of arena
			ball_px = 0;
			ball_py = 0;
			ball_dpx *= -1;
			ball_dpy = 0;
			player1_score++;
		}
	}

	// ------------- (6) Player 1 Simulation ----------------------
	float player1_ddpy = 0.f;
	if (is_down(BUTTON_UP)) player1_ddpy += 50.f;
	if (is_down(BUTTON_DOWN)) player1_ddpy -= 50.f;

	simulate_player(&player1_py, &player1_dpy, player1_ddpy, dt);

	// ------------- (7) Player 2 Simulation ----------------------
	float player2_ddpy = 0.f;
	if (is_down(BUTTON_W)) player2_ddpy += 50.f;
	if (is_down(BUTTON_S)) player2_ddpy -= 50.f;

	simulate_player(&player2_py, &player2_dpy, player2_ddpy, dt);

	// ------------- (8) Rendering --------------------------------
	draw_rect(ball_px, ball_py, ball_hsx, ball_hsy, 0xffffff);
	draw_rect(player1_px, player1_py, player1_half_size_x, player1_half_size_y, 0x8B0000);
	draw_rect(player2_px, player2_py, player2_half_size_x, player2_half_size_y, 0x8B0000);
}
