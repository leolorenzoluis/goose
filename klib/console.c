#include "klib/console.h"

#include "klib/base.h"
#include "klib/strings.h"
#include "klib/types.h"
#include "sys/io.h"

// TODO(chris): Learn more about this.
// http://wiki.osdev.org/Text_UI

Window con_windows[NUM_WINDOWS];
uint8* const screen_buffer = (uint8*) 0x000B8000;
Window* active_window = &con_windows[0];

// Private methods (not found in header).
// TODO(chris): Global state is lame. Perhaps hide this by having
// con_win_print_char take a WindowId, and return a function pointer?
Window* con_win_get_active();
void con_win_set_active(Window* win);

void con_win_print_char_active(char c);
void con_set_cursor(uint8 x, uint8 y);
void con_set_char(uint8 x, uint8 y, char c);
void con_set_color(uint8 x, uint8 y, Color foreground, Color background);
void con_win_set_focus(Window* win);
void con_win_scroll(Window* win);
void con_init_window(Window* win, const char* title,
                     uint8 offset_x, uint8 offset_y, uint8 width, uint8 height,
                     Color foreground, Color background, bool has_border);
size con_pos_to_idx(uint8 x, uint8 y);

// Returns the raw index into the screen_buffer. So screen_buffer[index] is the
// character, and screen_buffer[index + 1] is the formatting.
size con_pos_to_idx(uint8 x, uint8 y) {
  return (y * 80 + x) * 2;
}

Window* con_win_get_active() {
  return active_window;
}

void con_win_set_active(Window* win) {
  active_window = win;
}

// Prints a character to the currently active window.
void con_win_print_char_active(char c) {
  Window* win = con_win_get_active();
  // Wrap the cursor as necessary.
  if (c == '\n' || win->cursor_col >= 80) {
    win->cursor_line++;
    win->cursor_col = 0;
  }
  // Scroll as necessary.
  if (win->cursor_line >= win->height - win->offset_y) {
    win->cursor_line = win->height - win->offset_y;
    con_win_scroll(win);
  }
  // Print.
  if (c != '\n') {
    con_set_char(win->cursor_col, win->cursor_line, c);
    win->cursor_col++;
    con_set_cursor(win->cursor_col, win->cursor_line);
  }
}

// Initialize the console. Clears the screen, initializes data structures, etc.
void con_initialize() {
  // Disable the console blink. This also means we can use up to 16 colors.
  inb(0x3DA);
  outb(0x3C0, 0x30);
  inb(0x3C1);
  outb(0x3C0, 0x30);

  // Clear the string, defaulting to gray on black.
  const uint8 formatting = (BLACK << 4) | GRAY;
  for (int i = 0; i < 80 * 25; i++) {
    screen_buffer[i * 2] = '.';
    screen_buffer[i * 2 + 1] = formatting;
  }

  // Initialize the kernel windows.
  con_init_window(&con_windows[WIN_HEADER], "Goose",
                  0, 0, 80, 1, WHITE, RED, false);
  con_init_window(&con_windows[WIN_OUTPUT], "Output",
                  0, 1, 55, 24, WHITE, BLUE, true);
  con_init_window(&con_windows[WIN_DEBUG], "Debug",
		  55, 1, 25, 24, WHITE, BLACK, true);

  con_set_cursor(79, 24);
}

// Writes the text to the window. Scrolling text as necessary.
void con_writeline(WindowId win, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  con_writeline_va(win, fmt, args);
  va_end(args);
}

void con_writeline_va(WindowId window, const char* fmt, va_list args) {
  Window* win = &con_windows[window];
  con_win_set_active(win);

  base_printf_va(fmt, args, &con_win_print_char_active);

  // Handle an implicit '\n'.
  base_printf("\n", &con_win_print_char_active);
}

// Read text into the buffer, blocking until the return key is pressed.
// Output is echoed to the console.
void con_win_readline(WindowId win, char* buffer, size buffer_size) {
  // TODO(chris): Implement me. This will be hard.
  win++;
  buffer++;
  buffer_size++;
}

void con_init_window(Window* win, const char* title,
                     uint8 offset_x, uint8 offset_y, uint8 width, uint8 height,
                     Color foreground, Color background, bool has_border) {
  win->title = title;

  win->offset_x = offset_x;
  win->offset_y = offset_y;
  win->width = width;
  win->height = height;

  win->has_border = has_border;

  win->foreground = foreground;
  win->background = background;

  // Clear the window region.
  for (int y = offset_y; y < height + offset_y; y++) {
    for (int x = offset_x; x < width + offset_x; x++) {
      con_set_char(x, y, ' ');
      con_set_color(x, y, foreground, background);
    }
  }

  // Draw border. (-1 to account for width/height not being 0-indexed.)
  if (has_border) {
    for (int x = offset_x; x < width + offset_x; x++) {
      con_set_char(x, offset_y, '-');
      con_set_char(x, offset_y + height - 1, '-');
    }
    for (int y = offset_y; y < height + offset_y; y++) {
      con_set_char(offset_x, y, '|');
      con_set_char(offset_x + width - 1, y, '|');
    }
    con_set_char(offset_x, offset_y, 'X');
    con_set_char(offset_x + width - 1, offset_y, 'X');
    con_set_char(offset_x + width - 1, offset_y + height - 1, 'X');
    con_set_char(offset_x, offset_y + height - 1, 'X');

    // Draw title.
    size title_len = str_length(title);
    int8 title_start = offset_x + (width / 2) - (title_len / 2);
    con_set_char(title_start - 1, offset_y, ' ');
    for (size i = 0; i < title_len; i++) {
      con_set_char(title_start + i, offset_y, title[i]);
    }
    con_set_char(title_start + title_len, offset_y, ' ');
  }
 
  win->cursor_col = 0;
  win->cursor_line = 0;
}

void con_set_cursor(uint8 x, uint8 y) {
  uint16 pos = con_pos_to_idx(x, y);
  const uint16 kCommandPort = 0x3D4;
  const uint16 kDataPort = 0x3D5;
  const uint16 kHighByteCommand = 14;
  const uint16 kLowByteCommand = 15;
  outb(kCommandPort, kHighByteCommand);
  outb(kDataPort, ((pos >> 8) & 0x00FF));
  outb(kCommandPort, kLowByteCommand);
  outb(kDataPort,    pos & 0x00FF);
}

void con_set_char(uint8 x, uint8 y, char c) {
  uint16 index = con_pos_to_idx(x, y);
  screen_buffer[index] = c;
}

void con_set_color(uint8 x, uint8 y, Color foreground, Color background) {
  uint16 index = con_pos_to_idx(x, y);
  screen_buffer[index + 1] = ((background & 0x0F) << 4) | (foreground & 0x0F);
}

void con_win_scroll(Window* win) {
  for (int y = win->height - win->offset_y; y <= win->offset_y - 1; y--) {
    for (int x = 0; x < win->width - win->offset_x; x++) {
        uint16 from = con_pos_to_idx(x, y);
        uint16 to = con_pos_to_idx(x, y - 1);
        screen_buffer[to] = screen_buffer[from];
    }
  }
}
