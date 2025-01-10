/*  Hello to whoever is reading this, i know it's only you dear proffessor (:3) 
    it's mot much i know , but i would realy appreciate if you keep in mind that was the coolest and hardest project 
    i ever did , it was really painful , and tiring , 
    i learned a lot and had a lot of fun ,but got a lot of sleepless nights haha ,
    and headaches from the kinds of errors i got (i got traumatized by them, LOL), but i'm sure i'll get better at this (i hope)
    anyway ,that all i got to say , i hope the effort that me and my teammates put into this project reach your expectations :)
    best regards,
    Chafii :3
*/

//there will be probably a video linked to this project because we used external libraries


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <ctype.h>
#include <time.h>


#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif


//global constants 
//(i hate myself for not recaling that earlier, it could save me a lot of time (-_-))
#define MAX_FRAMES 15
#define FRAME_PATH_SIZE 256 // allocated bytes for frame path (*-*)
#define MAX_LEVELS 3
#define MAX_WORD_LENGTH 28
#define MAX_NAME_LENGTH 16
#define MAX_TRIES 15
TTF_Font *g_default_font = NULL;
TTF_Font *g_title_font = NULL;
TTF_Font *g_menu_font = NULL;

const int WINDOW_WIDTH = 1033; 
const int WINDOW_HEIGHT = 720;


// Enums for game states and levels
enum GameState {
    MENU,
    NAME_ENTRY,
    LEVEL_SELECTION,
    WORD_ENTRY,
    PLAYING,
    WIN,
    LOSE
};

enum GameState CurrentState = MENU;

enum GameLevel {
    LEVEL_1,
    LEVEL_2,
    LEVEL_3
};

enum GameLevel CurrentLevel;

// Global variables for player names and gameplay 
//(cause why not , i'm sick of retyping everything when i change one thing)
char Player1[MAX_NAME_LENGTH] = "";
char Player2[MAX_NAME_LENGTH] = "";
char nameInput[MAX_NAME_LENGTH] = "";
char WordToGuess[MAX_WORD_LENGTH] = "";
char GuessedWord[MAX_WORD_LENGTH] = "";
char IncorrectLetters[26] = "";
int TriesRemaining = MAX_TRIES;
int CapsLockOn = 0;

SDL_Rect EXIT_BUTTON = {987.556, 18.899, 30.797, 30.469};

// 0 for Player 1, 1 for Player 2 
//(each time you think you suck , remember that you don't, you just don't know *me at 3am trying to push myself to finish this code*)
int NameEntryStage = 0; 

// Global variables for hangman frames
SDL_Texture* hangman_frames[MAX_FRAMES] = {NULL};

// Calculate centered positions
int center_x = WINDOW_WIDTH / 2;
int center_y = WINDOW_HEIGHT / 2;


// Get the absolute path to assets directory (you can ignore all the path related fct  )
char* get_asset_path(void) {
    static char path[FRAME_PATH_SIZE];
    char *base_path = SDL_GetBasePath();
    
    if (base_path) {
        snprintf(path, sizeof(path), "%sassets", base_path);
        SDL_free(base_path);
    } else {
        // Fallback to relative path if SDL_GetBasePath fails
        strncpy(path, "assets", sizeof(path) - 1);
    }
    
    return path;
}

// Function declarations
void cleanup(SDL_Window *window, SDL_Renderer *renderer);
int initialize_fonts(void);


int is_valid_character(char c, enum GameLevel level) {
    // to reject any non-letter characters (see?, i'm using my brain *wink* *wink*)
    if (!isalpha(c)) {
        return 0;  // Immediately reject numbers, symbols, spaces, etc.
    }
    
    //level-specific rules
    switch (level) {
        case LEVEL_1:
            // Level 1: must be lowercase
            return islower(c);
        case LEVEL_2:
            // Level 2: can be either case
            return isalpha(c);
        case LEVEL_3:
            // Level 3: any letter is fine (uppercase or lowercase)
            return isalpha(c);
        default:
            return 0;
    }
}

// Function to validate entire word for current level
int validate_word_for_level(const char* word, enum GameLevel level) {
    if (!word) return 0;
    
    size_t len = strlen(word);
    
    // checking length requirementsssss
    int valid_length = 0;
    switch (level) {
        case LEVEL_1:
            valid_length = (len >= 3 && len <= 4);
            break;
        case LEVEL_2:
            valid_length = (len >= 5 && len <= 7);
            break;
        case LEVEL_3:
            valid_length = (len >= 8);
            break;
        default:
            return 0;
    }
    


    if (!valid_length) return 0;
    
    // Then check each character against level rules
    for (size_t i = 0; i < len; i++) {
        if (!is_valid_character(word[i], level)) {
            return 0;
        }
    }
    
    return 1;
}

// Get the error message based on validation failure
const char* get_validation_error(const char* word, enum GameLevel level) {
    if (!word) return "Invalid word";
    
    // Check length first
    size_t len = strlen(word);
    switch (level) {
        case LEVEL_1:
            if (len < 3 || len > 4) 
                return "Word must be 3-4 letters long";
            break;
        case LEVEL_2:
            if (len < 5 || len > 7) 
                return "Word must be 5-7 letters long";
            break;
        case LEVEL_3:
            if (len < 8) 
                return "Word must be 8 or more letters long";
            break;
    }
    
    // Check for invalid characters
    for (size_t i = 0; i < len; i++) {
        if (!isalpha(word[i])) {
            return "Only letters are allowed (no numbers or symbols)";
        }
        
        if (level == LEVEL_1 && !islower(word[i])) {
            return "Level 1 requires lowercase letters only";
        }
    }
    
    return NULL;
}

// Add a function to get level requirements as string,
// it's just to give instructions to player while playing // 5am :( *i'm tired GRANDPA* ,*KEEP DIGGING* ,i don't remember the rest of the meme ('-') )
const char* get_level_requirements(enum GameLevel level) {
    switch (level) {
        case LEVEL_1:
            return "Level 1: Word must be 3-4 letters long";
        case LEVEL_2:
            return "Level 1: Word must be 5-7 letters long";
        case LEVEL_3:
            return "Level 1: Word must be 8 or more letters long";
        default:
            return "Invalid level";
    }
}

int load_hangman_frames(SDL_Renderer *renderer) {
    // Add error checking at the start
    if (!renderer) {
        printf("Invalid renderer in load_hangman_frames\n");
        return 0;
    }
    
    // Add frame loading status tracking
    int frames_loaded = 0;
    char full_path[FRAME_PATH_SIZE];
    char *asset_path = get_asset_path();
    
    // First verify assets directory exists
    SDL_RWops *check = SDL_RWFromFile(asset_path, "r");
    if (!check) {
        printf("Assets directory not found at: %s\n", asset_path);
        // Create the directory if it doesn't exist
        #ifdef _WIN32
            _mkdir(asset_path);
        #else
            mkdir(asset_path, 0777);
        #endif
    } else {
        SDL_RWclose(check);
    }
    
    // Load each frame with proper error handling
    for(int i = 0; i < MAX_FRAMES; i++) {
        snprintf(full_path, sizeof(full_path), "%s/hangman_%d.png", asset_path, i);
        
        SDL_Surface *surface = IMG_Load(full_path);
        if(!surface) {
            printf("Failed to load frame %d: %s\n", i, IMG_GetError());
            printf("Path attempted: %s\n", full_path);
            continue;
        }
        
        hangman_frames[i] = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        
        if(!hangman_frames[i]) {
            printf("Failed to create texture for frame %d: %s\n", i, SDL_GetError());
            continue;
        }
        
        frames_loaded++;
    }
    
    if(frames_loaded == 0) {
        printf("No frames were loaded! Please check your assets directory.\n");
        return 0;
    }
    
    printf("Successfully loaded %d/%d frames\n", frames_loaded, MAX_FRAMES);
    return 1;
}

//my favorite part of this project, picking a font ¨ ̄\_(ツ)_/ ̄
int initialize_fonts() {
    g_default_font = TTF_OpenFont("/home/chafii/.local/share/fonts/Google Fonts/Fredericka the Great/Fredericka_the_Great_Regular.21.ttf", 24);
    if (!g_default_font) {
        printf("Failed to load default font: %s\n", TTF_GetError());
        return 0;
    }

    g_title_font = TTF_OpenFont("/home/chafii/.local/share/fonts/Google Fonts/Fredericka the Great/Fredericka_the_Great_Regular.21.ttf", 72);
    if (!g_title_font) {
        printf("Failed to load title font: %s\n", TTF_GetError());
        TTF_CloseFont(g_default_font);
        return 0;
    }

    g_menu_font = TTF_OpenFont("/home/chafii/.local/share/fonts/Google Fonts/Fredericka the Great/Fredericka_the_Great_Regular.21.ttf", 36);
    if (!g_menu_font) {
        printf("Failed to load menu font: %s\n", TTF_GetError());
        TTF_CloseFont(g_default_font);
        TTF_CloseFont(g_title_font);
        return 0;
    }

    return 1;
}


TTF_Font* load_font(int size) {
    const char* font_path = "/home/chafii/.local/share/fonts/Google Fonts/Fredericka the Great/Fredericka_the_Great_Regular.21.ttf";
    TTF_Font* font = TTF_OpenFont(font_path, size);
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        // Fallback to system font if available
        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", size);
        if (!font) {
            printf("Failed to load fallback font: %s\n", TTF_GetError());
            return NULL;
        }
    }
    return font;
}

// Function to initialize SDL graphics
//the easiest part of this project  ~the_number_of_times_i_hade_to_rewrite_this_is_insane~

int init_graphics(SDL_Window **window, SDL_Renderer **renderer) {
    if (window == NULL || renderer == NULL) {
        printf("Window or renderer pointer is NULL\n");
        return 0;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 0;
    }

    *window = SDL_CreateWindow(
        "HANGMAN",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1033, 720,
        SDL_WINDOW_BORDERLESS
    );

    if (*window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);

    if (*renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return 0;
    }

    if (TTF_Init() < 0) {
        printf("TTF could not initialize! SDL_Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(*renderer);
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return 0;
    }

    // Initialize SDL_image
    int imgFlags = IMG_INIT_PNG;
    if(!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return 0;
    }
    
    // Load the frames
    if (!load_hangman_frames(*renderer)) {
        printf("Failed to load hangman frames\n");
        return 0;
    }

    SDL_SetRenderDrawColor(*renderer, 28, 28, 28, 255);
    SDL_RenderClear(*renderer);
    SDL_RenderPresent(*renderer);
    
    return 1;
}

// render_text function with proper dimension calculation 
// (seriously, calculating text dimensions would never came to me in the first place ,let alone with SDL, lack of sleep, and a thigh schedule)

void render_text(SDL_Renderer *renderer, const char *text, int x, int y, int w, int h, TTF_Font *font) {
    if (!renderer || !text || !font) return;
    
    SDL_Color color = {192, 192, 192, 255}; // Light gray
    
    // Get actual text dimensions
    int text_w, text_h;
    TTF_SizeText(font, text, &text_w, &text_h);
    
    // Calculate proper positioning
    SDL_Rect dest_rect;
    if (w == 0 || h == 0) {
        // Use actual text dimensions if no specific size is given
        dest_rect.w = text_w;
        dest_rect.h = text_h;
        dest_rect.x = x;
        dest_rect.y = y;
    } else {
        // Scale text to fit within given dimensions while maintaining aspect ratio
        float scale = fmin((float)w / text_w, (float)h / text_h);
        dest_rect.w = (int)(text_w * scale);
        dest_rect.h = (int)(text_h * scale);
        // Center the text in the given space
        dest_rect.x = x + (w - dest_rect.w) / 2;
        dest_rect.y = y + (h - dest_rect.h) / 2;
    }
    
    SDL_Surface *surface = TTF_RenderText_Blended(font, text, color);
    if (!surface) {
        printf("Failed to render text: %s\n", TTF_GetError());
        return;
    }
    
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    
    SDL_RenderCopy(renderer, texture, NULL, &dest_rect);
    
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}


// Function to handle menu events with proper input validation and state transitions 

void handle_menu_events(SDL_Event *event, int *running) {
    if (!event || !running) {
        printf("Invalid parameters in handle_menu_events\n");
        return;
    }

    // Define clickable button
    SDL_Rect start_button = {390, 292, 230, 70};
    SDL_Rect exit_button = {987, 19, 31, 30};

    if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        SDL_Point click_point = {event->button.x, event->button.y};
        
        // Check if the button was clicked
        if (SDL_PointInRect(&click_point, &start_button)) {
            // Reset game state before starting new game
            NameEntryStage = 0;
            memset(Player1, 0, sizeof(Player1));//a big thanks for the saver, that one tuto i watched it at 1am for this *teary eye*,thanks again
            memset(Player2, 0, sizeof(Player2));
            memset(nameInput, 0, sizeof(nameInput));
            CurrentState = NAME_ENTRY;
        }
        // Check if exit button was clicked
        else if (SDL_PointInRect(&click_point, &exit_button)) {
            *running = 0;
        }
    }
    // Handle keyboard shortcuts
    else if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_RETURN:
                CurrentState = NAME_ENTRY;
                break;
            case SDLK_ESCAPE:
                *running = 0;
                break;
        }
    }
}

void display_menu(SDL_Renderer *renderer) {

    if (!renderer) return;

    SDL_SetRenderDrawColor(renderer, 28, 28, 28, 255);
    SDL_RenderClear(renderer);

    // Use global fonts
    render_text(renderer, "HANGMAN", 350, 100, 300, 100, g_title_font);
    render_text(renderer, "Start Game", 390, 292, 230, 70, g_menu_font);
    render_text(renderer, "X", 987, 19, 31, 30, g_menu_font);

    // Draw decorative elements
    SDL_SetRenderDrawColor(renderer, 192, 192, 192, 255);
    SDL_RenderDrawLine(renderer, 390, 362, 620, 362);

    SDL_RenderPresent(renderer);
}


//Dear Diary ,it's 2am 
//That was harder than I expected  *sigh*  
//Like what do you mean by "Segmentation fault (core dumped)"  
//*screaming inside noises* 

// Function to handle name entry with proper text input
void handle_name_entry_events(SDL_Event *event) {
    if (!event) return;

    if (event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_BACKSPACE && strlen(nameInput) > 0) {
            nameInput[strlen(nameInput) - 1] = '\0';
        }
        else if (event->key.keysym.sym == SDLK_RETURN && strlen(nameInput) > 0) {
            if (NameEntryStage == 0) {
                strncpy(Player1, nameInput, MAX_NAME_LENGTH - 1);
                Player1[MAX_NAME_LENGTH - 1] = '\0';
                memset(nameInput, 0, MAX_NAME_LENGTH);
                NameEntryStage = 1;
            }
            else {
                strncpy(Player2, nameInput, MAX_NAME_LENGTH - 1);
                Player2[MAX_NAME_LENGTH - 1] = '\0';
                memset(nameInput, 0, MAX_NAME_LENGTH);
                CurrentState = LEVEL_SELECTION;
            }
        }
        else if (strlen(nameInput) < MAX_NAME_LENGTH - 1) {
            char c = event->key.keysym.sym;
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == ' ') {
                char str[2] = {c, '\0'};
                strcat(nameInput, str);
            }
        }
    }
}

// Function to display name entry screen 
void display_name_entry(SDL_Renderer *renderer, TTF_Font *font) {
    SDL_SetRenderDrawColor(renderer, 28, 28, 28, 255);
    SDL_RenderClear(renderer);

    const char *prompt = NameEntryStage == 0 ? "Enter Player 1 Name:" : "Enter Player 2 Name:";
    
    // Render prompt with automatic sizing (0 for width/height)
    render_text(renderer, prompt, 300, 200, 0, 0, font);
    
    // Create a fixed-width input box but let text maintain its natural size
    SDL_Rect inputBox = {295, 295, 410, 60};
    SDL_SetRenderDrawColor(renderer, 192, 192, 192, 255);
    SDL_RenderDrawRect(renderer, &inputBox);
    
    // Render input text with natural dimensions
    render_text(renderer, nameInput, inputBox.x + 10, inputBox.y + 10, 0, 0, font);

    SDL_RenderPresent(renderer);
}

// Function to display level selection screen with visual feedback
void display_level_selection(SDL_Renderer *renderer, TTF_Font *font) {
    if (!renderer || !font) {
        printf("Invalid parameters in display_level_selection\n");
        return;
    }

    // Clear screen with background color
    SDL_SetRenderDrawColor(renderer, 28, 28, 28, 255);
    SDL_RenderClear(renderer);

    // Load larger font for title
    TTF_Font* title_font = TTF_OpenFont("/home/chafii/.local/share/fonts/Google Fonts/Fredericka the Great/Fredericka_the_Great_Regular.21.ttf", 64);
    if (!title_font) {
        printf("Failed to load title font: %s\n", TTF_GetError());
        return;
    }

    // Display title
    render_text(renderer, "Select Difficulty", 250, 100, 500, 80, title_font);
    TTF_CloseFont(title_font);

    // Display player names
    char players_text[100];
    snprintf(players_text, sizeof(players_text), "Players: %s vs %s", Player1, Player2);
    render_text(renderer, players_text, 250, 180, 500, 40, font);

    // Define level buttons with proper spacing
    SDL_Rect level_buttons[3] = {
        {400, 250, 250, 50},  // Level 1
        {400, 350, 250, 50},  // Level 2
        {400, 450, 250, 50}   // Level 3
    };

    // Draw level buttons with descriptions
    const char* level_texts[] = {
        "Level 1 - Easy",
        "Level 2 - Medium",
        "Level 3 - Hard"
    };

    const char* level_descriptions[] = {
        "3-4 Letter Words",
        "5-7 Letter Words",
        "8+ Letter Words"
    };

    SDL_SetRenderDrawColor(renderer, 192, 192, 192, 255);
    
    for (int i = 0; i < 3; i++) {
        // Draw button background
        SDL_RenderDrawRect(renderer, &level_buttons[i]);
        
        // Draw level text
        render_text(renderer, level_texts[i], 
                   level_buttons[i].x, 
                   level_buttons[i].y, 
                   level_buttons[i].w, 
                   level_buttons[i].h, 
                   font);
        
        // Draw level description
        render_text(renderer, level_descriptions[i],
                   level_buttons[i].x + level_buttons[i].w + 20,
                   level_buttons[i].y + 10,
                   200, 30,
                   font);
    }

    // Add instruction text at bottom
    render_text(renderer, "Click a level to begin or press ESC to return", 
                250, 550, 500, 40, font);

    SDL_RenderPresent(renderer);
}



// level selection event handler 
void handle_level_selection_events(SDL_Event *event) {
    if (!event) {
        printf("Invalid event in handle_level_selection_events\n");
        return;
    }

    static SDL_Rect level_buttons[3] = {
        {400, 250, 250, 50},  // Level 1
        {400, 350, 250, 50},  // Level 2
        {400, 450, 250, 50}   // Level 3
    };

    if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        SDL_Point click_point = {event->button.x, event->button.y};
        
        for (int i = 0; i < 3; i++) {
            if (SDL_PointInRect(&click_point, &level_buttons[i])) {
                CurrentLevel = (enum GameLevel)i;
                memset(WordToGuess, 0, sizeof(WordToGuess));  // Clear any previous word
                CurrentState = WORD_ENTRY;  // Go to word entry instead of playing
                break;
            }
        }
    }
    else if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_1:
            case SDLK_KP_1:
                CurrentLevel = LEVEL_1;
                memset(WordToGuess, 0, sizeof(WordToGuess));
                CurrentState = WORD_ENTRY;
                break;
            case SDLK_2:
            case SDLK_KP_2:
                CurrentLevel = LEVEL_2;
                memset(WordToGuess, 0, sizeof(WordToGuess));
                CurrentState = WORD_ENTRY;
                break;
            case SDLK_3:
            case SDLK_KP_3:
                CurrentLevel = LEVEL_3;
                memset(WordToGuess, 0, sizeof(WordToGuess));
                CurrentState = WORD_ENTRY;
                break;
            case SDLK_ESCAPE:
                CurrentState = MENU;
                break;
        }
    }
}

// Add new function to handle word entry with level validation
void handle_word_entry_events(SDL_Event *event) {

    if (!event) return;

    if (event->type == SDL_KEYDOWN) {
        SDL_Keycode key = event->key.keysym.sym;

        // Handle Caps Lock toggle
        if (key == SDLK_CAPSLOCK) {
            CapsLockOn = !CapsLockOn; // Toggle Caps Lock state
            printf("Caps Lock %s\n", CapsLockOn ? "ON" : "OFF");
            return;
        }

        if (key == SDLK_BACKSPACE && strlen(WordToGuess) > 0) {
            WordToGuess[strlen(WordToGuess) - 1] = '\0';
        } else if (key == SDLK_RETURN && strlen(WordToGuess) > 0) {
            if (validate_word_for_level(WordToGuess, CurrentLevel)) {
                // Initialize GuessedWord with asterisks
                size_t word_length = strlen(WordToGuess);
                memset(GuessedWord, '*', word_length);
                GuessedWord[word_length] = '\0';

                TriesRemaining = MAX_TRIES;
                memset(IncorrectLetters, 0, sizeof(IncorrectLetters));
                CurrentState = PLAYING;
            }
        } else if (strlen(WordToGuess) < MAX_WORD_LENGTH - 1) {
            char input_letter = (char)key;

            // Adjust the letter case based on Caps Lock state
            if (isalpha(input_letter)) {
                if (CapsLockOn) {
                    input_letter = toupper(input_letter);
                } else {
                    input_letter = tolower(input_letter);
                }

                // Check if the character is valid for the current level
                if (is_valid_character(input_letter, CurrentLevel)) {
                    size_t len = strlen(WordToGuess);
                    WordToGuess[len] = input_letter;
                    WordToGuess[len + 1] = '\0'; // Null-terminate the string
                }
            }
        }
    }
}

// Add new function to display word entry screen with level requirements
void display_word_entry(SDL_Renderer *renderer, TTF_Font *font) {

    SDL_SetRenderDrawColor(renderer, 28, 28, 28, 255);
    SDL_RenderClear(renderer);

// Get level-specific instructions
    const char* level_instructions;
    switch (CurrentLevel) {
        case LEVEL_1:
            level_instructions = "Level 1: Use 3-4 lowercase letters only";
            break;
        case LEVEL_2:
            level_instructions = "Level 2: Use 5-7 letters (any case)";
            break;
        case LEVEL_3:
            level_instructions = "Level 3: Use 8+ letters (any case)";
            break;
        default:
            level_instructions = "Invalid level";
            break;  
    }

// Window width and height (adjust these to your window's dimensions)
    const int WINDOW_WIDTH = 1033; 
    const int WINDOW_HEIGHT = 720;

// Calculate centered positions
    int center_x = WINDOW_WIDTH / 2;
    int center_y = WINDOW_HEIGHT / 2;

// Render title and instructions
    render_text(renderer, "Player 1: Enter your word", center_x - 200, 100, 400, 50, font);
    render_text(renderer, level_instructions, center_x - 200, 150, 400, 50, font);

// Create centered input box
    SDL_Rect inputBox = {center_x - 205, center_y - 30, 410, 60};
    SDL_SetRenderDrawColor(renderer, 192, 192, 192, 255);
    SDL_RenderDrawRect(renderer, &inputBox);

// Display asterisks for the hidden word
    char hidden_word[MAX_WORD_LENGTH];
    size_t len = strlen(WordToGuess);
    memset(hidden_word, '*', len);
    hidden_word[len] = '\0';

// Render hidden word inside the input box
    render_text(renderer, hidden_word, inputBox.x + 10, inputBox.y + 10, inputBox.w - 20, inputBox.h - 20, font);

// Render additional instructions and error messages
    render_text(renderer, "Press Enter when done", center_x - 200, center_y + 80, 400, 50, font);

    const char* error = get_validation_error(WordToGuess, CurrentLevel);
    if (strlen(WordToGuess) > 0 && error) {
     SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
     render_text(renderer, error, center_x - 200, center_y + 140, 400, 50, font);
    }

    SDL_RenderPresent(renderer);
}

// Draw current frame with error handling
void draw_hangman_frame(SDL_Renderer *renderer, int mistakes) {
    if(mistakes < 0 || mistakes >= MAX_FRAMES) {
        printf("Invalid mistake count: %d\n", mistakes);
        return;
    }
    
    if(!hangman_frames[mistakes]) {
        printf("Frame %d not loaded\n", mistakes);
        return;
    }
    
    // Get texture dimensions
    int texture_w, texture_h;
    SDL_QueryTexture(hangman_frames[mistakes], NULL, NULL, &texture_w, &texture_h);
    
    // Calculate appropriate scaling while maintaining aspect ratio
    float scale = fmin(350.0f / texture_w, 500.0f / texture_h);
    int final_w = (int)(texture_w * scale);
    int final_h = (int)(texture_h * scale);
    
    // Position frame on the left side of the screen
    SDL_Rect dest = {
        50,                     // Left margin
        100,                    // Top margin
        final_w,
        final_h
    };
    
    SDL_RenderCopy(renderer, hangman_frames[mistakes], NULL, &dest);
}


void display_gameplay(SDL_Renderer *renderer) {
    if (!renderer) {
        printf("Renderer is NULL in display_gameplay\n");
        return;
    }

    // Clear screen with dark background
    SDL_SetRenderDrawColor(renderer, 28, 28, 28, 255);
    SDL_RenderClear(renderer);


    // Layout dimensions
    const int right_panel_x = 450;      // Starting x position for game info


    if (CurrentLevel > LEVEL_1) {
        char case_info[100] = "Note: Letter case matters! Match the exact capitalization";
        render_text(renderer, case_info, right_panel_x, 450, 500, 30, g_default_font);
    }    


    // Draw the hangman frame first - position it on the left side
    draw_hangman_frame(renderer, MAX_TRIES - TriesRemaining);

    // Right panel with game information
    // Display players with proper spacing
    char player_text[100];
    snprintf(player_text, sizeof(player_text), "Player 1: %s | Player 2: %s", Player1, Player2);
    render_text(renderer, player_text, right_panel_x, 50, 500, 40, g_menu_font);

    // Display level
    char level_text[20];
    snprintf(level_text, sizeof(level_text), "Level %d", CurrentLevel + 1);
    render_text(renderer, level_text, right_panel_x, 100, 200, 40, g_menu_font);

    // Display the word to guess with proper spacing
    char display_word[MAX_WORD_LENGTH * 2];
    int j = 0;
    for (size_t i = 0; i < strlen(GuessedWord); i++) {
        display_word[j++] = GuessedWord[i];
        display_word[j++] = ' ';
    }
    display_word[j] = '\0';
    
    // Center the word in the right panel
    render_text(renderer, display_word, right_panel_x, 200, 500, 50, g_title_font);

    // Display incorrect letters
    char incorrect_text[100];
    snprintf(incorrect_text, sizeof(incorrect_text), "Incorrect Letters: %s ", IncorrectLetters);
    render_text(renderer, incorrect_text, right_panel_x, 300, 500, 40, g_menu_font);

    // Display remaining tries
    char tries_text[50];
    snprintf(tries_text, sizeof(tries_text), "Tries Remaining: %d", TriesRemaining);
    render_text(renderer, tries_text, right_panel_x, 400, 300, 40, g_menu_font);

    // Add instruction text at the bottom
    render_text(renderer, "Press ESC to return to menu", right_panel_x, 500, 400, 30, g_default_font);

    SDL_RenderPresent(renderer);
}

//to handle letter player2 input 
void handle_gameplay_events(SDL_Event *event) {
        if (!event) return;

    if (event->type == SDL_KEYDOWN) {
        SDL_Keycode key = event->key.keysym.sym;

        // Handle Caps Lock toggle
        if (key == SDLK_CAPSLOCK) {
            CapsLockOn = !CapsLockOn; // Toggle Caps Lock state
            printf("Caps Lock %s\n", CapsLockOn ? "ON" : "OFF");
            return;
        }

        if (key == SDLK_ESCAPE) {
            CurrentState = MENU;
            return;
        }

        if (key >= SDLK_a && key <= SDLK_z) {
            char input_letter = (char)key;

            // Adjust letter based on Caps Lock state
            if (CapsLockOn) {
                input_letter = toupper(input_letter);
            } else {
                input_letter = tolower(input_letter);
            }

            // Handle Level 1 rules (force lowercase)
            if (CurrentLevel == LEVEL_1) {
                input_letter = tolower(input_letter);
            }

            // Process the guess
            int found = 0;
            int already_guessed = 0;

            // Check if the letter was already guessed
            for (int i = 0; i < (int)strlen(IncorrectLetters); i++) {
                if (IncorrectLetters[i] == input_letter) {
                    already_guessed = 1;
                    break;
                }
            }
            for (int i = 0; i < (int)strlen(GuessedWord); i++) {
                if (GuessedWord[i] == input_letter) {
                    already_guessed = 1;
                    break;
                }
            }

            if (!already_guessed) {
                // Check for matches in the word
                for (int i = 0; i < (int)strlen(WordToGuess); i++) {
                    if (WordToGuess[i] == input_letter) { // Case-sensitive match
                        GuessedWord[i] = WordToGuess[i];
                        found = 1;
                    }
                }

                if (!found) {
                    // Add to incorrect letters
                    size_t len = strlen(IncorrectLetters);
                    if (len < sizeof(IncorrectLetters) - 1) {
                        IncorrectLetters[len] = input_letter;
                        IncorrectLetters[len + 1] = '\0';
                        TriesRemaining--;
                    }
                }

                // Check win condition
                if (strcmp(WordToGuess, GuessedWord) == 0) {
                    CurrentState = WIN;
                } else if (TriesRemaining <= 0) {
                    CurrentState = LOSE;
                }
            }
        }
    }
}

// Function to display win/lose screen
void display_end_screen(SDL_Renderer *renderer, TTF_Font *font, const char *message) {
    SDL_SetRenderDrawColor(renderer, 28, 28, 28, 255);
    SDL_RenderClear(renderer);

    // Display main message
    render_text(renderer, message, 250, 200, 500, 100, font);
    
    // Display final word
    char word_message[100];
    sprintf(word_message, "The word was: %s", WordToGuess);
    render_text(renderer, word_message, 250, 320, 500, 50, font);
    
    // Display play again prompt
    render_text(renderer, "Press Y to play again or N to quit", 250, 400, 500, 50, font);

    SDL_RenderPresent(renderer);
}

// Function to handle end screen events
void handle_end_screen_events(SDL_Event *event, int *running) {
    if (!event || !running) return;

    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_y:
                // Reset game state
                CurrentState = MENU;
                NameEntryStage = 0;
                TriesRemaining = MAX_TRIES;
                memset(IncorrectLetters, 0, sizeof(IncorrectLetters));
                memset(GuessedWord, 0, sizeof(GuessedWord));
                memset(WordToGuess, 0, sizeof(WordToGuess));
                memset(Player1, 0, sizeof(Player1));
                memset(Player2, 0, sizeof(Player2));
                break;
            case SDLK_n:
                *running = 0;
                break;
        }
    }
}

// cleanup function 
void cleanup(SDL_Window *window, SDL_Renderer *renderer) {


    if (g_default_font) 
    TTF_CloseFont(g_default_font);
    if (g_title_font) 
    TTF_CloseFont(g_title_font);
    if (g_menu_font) 
    TTF_CloseFont(g_menu_font);

    for(int i = 0; i < MAX_FRAMES; i++) {
        if(hangman_frames[i]) {
            SDL_DestroyTexture(hangman_frames[i]);
            hangman_frames[i] = NULL;
        }
    }

    if (window) {
        SDL_DestroyWindow(window);
    }
    
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }


    TTF_Quit();
    SDL_Quit();
}


int main() {
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    
    init_graphics(&window, &renderer);
    if (!init_graphics(&window, &renderer)) {
        fprintf(stderr, "Failed to initialize graphics.\n");
        return EXIT_FAILURE;
    }

    if (!initialize_fonts()) {
        fprintf(stderr, "Failed to initialize fonts.\n");
        cleanup(window, renderer);
        return EXIT_FAILURE;
    }

    TTF_Font *default_font = load_font(24);
    if (!default_font) {
        fprintf(stderr, "Could not load font!\n");
        cleanup(window, renderer);
        return EXIT_FAILURE;
    }

    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else {
                switch (CurrentState) {
                    case MENU:
                        handle_menu_events(&event, &running);
                        break;
                    case NAME_ENTRY:
                        handle_name_entry_events(&event);
                        break;
                    case LEVEL_SELECTION:
                        handle_level_selection_events(&event);
                        break;
                    case WORD_ENTRY:
                        handle_word_entry_events(&event);
                        break;
                    case PLAYING:
                        handle_gameplay_events(&event);
                        break;
                    case WIN:
                        handle_end_screen_events(&event, &running);
                        break;
                    case LOSE:
                        handle_end_screen_events(&event, &running);
                        break;
                }
            }
        }

        // Render current state
        switch (CurrentState) {
            case MENU:
                display_menu(renderer);
                break;
            case NAME_ENTRY:
                display_name_entry(renderer, default_font);
                break;
            case LEVEL_SELECTION:
                display_level_selection(renderer, default_font);
                break;
            case WORD_ENTRY:
                display_word_entry(renderer, default_font);
                break;
            case PLAYING:
                display_gameplay(renderer);
                break;
            case WIN:
                display_end_screen(renderer, default_font, "Congratulations! You Win!");
                break;
            case LOSE:
                display_end_screen(renderer, default_font, "Game Over! You Lose!");
                break;
        }
    }

    TTF_CloseFont(default_font);
    cleanup(window, renderer);
    return 0;
}