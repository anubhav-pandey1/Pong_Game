
// ---------------- Key State Management --------------------------------
struct Button_State {
	bool is_down;
	bool changed;
};

// ----------------- Keyboard Controls ----------------------------------
// Enumerate buttons to use as indices in buttons array (BUTTON UP = 0)
// BUTTON_COUNT (= 4 currently) will change if we change number of keys
enum Button_Key {
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_LEFT,
	BUTTON_RIGHT,
	BUTTON_W,
	BUTTON_S,
	BUTTON_A,
	BUTTON_D,
	BUTTON_P,
	BUTTON_ENTER,
	BUTTON_ESC,

	// To keep track of count of keys here
	BUTTON_COUNT,
};

// ---------------- Key State Storage -----------------------------------
// Struct to keep track of the inputs using an array of type Button_State
struct Input {
	Button_State buttons[BUTTON_COUNT];
};

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
