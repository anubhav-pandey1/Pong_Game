
// ---------------- Key State Management --------------------------------
struct Button_State {
	bool is_down;
	bool changed;
};

// ----------------- Keyboard Controls ----------------------------------
// Enumerate buttons to use as indices in buttons array (BUTTON UP = 0)
// BUTTON_COUNT (= 4 currently) will change if we change number of keys
enum {
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
