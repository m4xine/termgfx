// TODO: Compare the buffers before rendering,
//       if they're the same - dont render.
// TODO: Before rendering, get the terminal window
//       size and resize the front/back buffers.
// TODO: Add assertions and bounds checking.

#ifndef TERMGFX_TERMGFX_H
#define TERMGFX_TERMGFX_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <termios.h>
#include <sys/ioctl.h>

#define TG__MAX(x, y) ((x) > (y) ? (x) : (y))
#define TG__MIN(x, y) ((x) < (y) ? (x) : (y))

typedef struct
{
  int16_t x, y;
} tgVec2;

#define TG_VEC2(x_, y_) ((tgVec2) { .x = (x_), .y = (y_) })

typedef struct
{
  int16_t x, y, w, h;
} tgRect;

#define TG_RECT(x_, y_, w_, h_) ((tgRect) { .x = (x_), .y = (y_), .w = (w_), .h = (h_) })

uint8_t
tg__char_size(int32_t c)
{
  bool const v[] = 
    {
      c <  128,
      c >= 128   && c < 2048,
      c >= 2048  && c < 65536,
      c >= 65536 && c < 1114112,
    };

  uint8_t out = 0;
  for (uint8_t i = 0; i < 4; ++i)
    if (v[i]) out = i;

  return out + 1;
}

uint8_t 
tg__encode(int32_t c, uint8_t *out) 
{
  uint8_t len = tg__char_size(c);

  if (1 == len) out[0] = c; 
  else if (2 == len) 
  {
    out[0] = 192 + (c >> 6);
    out[1] = 128 + (c & 63);
  } 
  else if (3 == len) 
  {
    out[0] = 224 + (c >> 12);
    out[1] = 128 + ((c >> 6) & 63);
    out[2] = 128 + (c & 63);
  } 
  else if (4 == len) 
  {
    out[0] = 240 + (c >> 18);
    out[1] = 128 + ((c >> 12) & 63);
    out[2] = 128 + ((c >> 6) & 63);
    out[3] = 128 + (c & 63);
  }

  return len;
}

// Flushes stdout.
void 
tg__flush(void)
{
  fflush(stdout);
}

// Hides the cursor.
// NOTE: This does not flush stdout!
void
tg__hide_cursor(void)
{
  fputs("\e[?25l", stdout);
}

// Shows the cursor.
// NOTE: This does not flush stdout!
void
tg__show_cursor(void)
{
  fputs("\e[?25h", stdout);
}

// Sets the cursor position.
// NOTE: This does not flush stdout!
void
tg__set_cursor(tgVec2 p)
{
  fprintf(stdout, "\e[%hu;%huH", p.y + 1, p.x + 1);
}

// Terminal state.
typedef struct
{
  // Keep the initial termios state to restore the terminal.
  struct termios initial_termios;

  // Terminal window size. 
  struct winsize winsize;
} tgTerm;

// tgTerm empty constructor.
tgTerm
tg_term(void)
{
  tgTerm t;
  return *(tgTerm *)memset((void *)&t, 0, sizeof(tgTerm));
}

// Sets the terminal in raw mode for rendering.
// Returns 0 on success, <0 on error. 
int 
tg_raw_mode(tgTerm *t)
{
  if (-1 == tcgetattr(STDIN_FILENO, &t->initial_termios))
    return -1;

  struct termios termios = t->initial_termios;
  termios.c_iflag       &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  termios.c_oflag       &= ~OPOST;
  termios.c_cflag       |= CS8;
  termios.c_lflag       &= ~(ECHO | ICANON | IEXTEN | ISIG);
  termios.c_cc[VMIN]     = 0;
  termios.c_cc[VTIME]    = 1;

  if (-1 == tcsetattr(STDIN_FILENO, TCSAFLUSH, &termios))
    return -1;

  return 0;
}

// Restores the terminal to its initial state.
// Returns 0 on success, <0 on error.
int 
tg_restore(tgTerm *t)
{
  if (-1 == tcsetattr(STDIN_FILENO, TCSAFLUSH, &t->initial_termios))
    return -1;
  
  return 0;
}

// Updates `tgTerm.winsize` to the current terminal size. 
// Returns 0 on success, <0 on error.
// Note: ioctl() may not be able to retrieve the terminal 
//       size on all systems.
int
tg_get_size(tgTerm *t)
{
  if (-1 == ioctl(STDOUT_FILENO, TIOCGWINSZ, &t->winsize) 
    || 0 == t->winsize.ws_col)
    return -1;
  
  return 0;
}

// Enables alt screen.
// NOTE: This flushes stdout!
void
tg_enable_alt_screen(void)
{
  fputs("\e[?1049h", stdout);
  tg__flush();
}

// Disables alt screen.
// NOTE: This flushes stdout!
void
tg_disable_alt_screen(void)
{
  fputs("\e[?1049l", stdout);
  tg__flush();
}

// Foreground color.
typedef uint8_t tgForeground;

// Background color.
typedef uint8_t tgBackground;
enum
{
  TG_RESET = 0,
  TG_BLACK,
  TG_RED,
  TG_GREEN,
  TG_YELLOW,
  TG_BLUE,
  TG_MAGENTA,
  TG_CYAN,
  TG_WHITE,
  TG_BRIGHT_BLACK,
  TG_BRIGHT_RED,
  TG_BRIGHT_GREEN,
  TG_BRIGHT_YELLOW,
  TG_BRIGHT_BLUE,
  TG_BRIGHT_MAGENTA,
  TG_BRIGHT_CYAN,
  TG_BRIGHT_WHITE
};

// Retrieve the ANSI code for the specified foreground color.
char const *
tg_foreground_str(tgForeground fg)
{
  switch (fg)
  {
    case TG_RESET:          return "\e[0m";
    case TG_BLACK:          return "\e[30m";
    case TG_RED:            return "\e[31m";
    case TG_GREEN:          return "\e[32m";
    case TG_YELLOW:         return "\e[33m";
    case TG_BLUE:           return "\e[34m";
    case TG_MAGENTA:        return "\e[35m";
    case TG_CYAN:           return "\e[36m";
    case TG_WHITE:          return "\e[37m";
    case TG_BRIGHT_BLACK:   return "\e[30;1m";
    case TG_BRIGHT_RED:     return "\e[31;1m";
    case TG_BRIGHT_GREEN:   return "\e[32;1m";
    case TG_BRIGHT_YELLOW:  return "\e[33;1m";
    case TG_BRIGHT_BLUE:    return "\e[34;1m";
    case TG_BRIGHT_MAGENTA: return "\e[35;1m";
    case TG_BRIGHT_CYAN:    return "\e[36;1m";
    case TG_BRIGHT_WHITE:   return "\e[37;1m";
  }
}

// Retrieve the ANSI code for the specified background color.
char const *
tg_background_str(tgBackground bg)
{
  switch (bg)
  {
    case TG_RESET:          return "\e[0m";
    case TG_BLACK:          return "\e[40m";
    case TG_RED:            return "\e[41m";
    case TG_GREEN:          return "\e[42m";
    case TG_YELLOW:         return "\e[43m";
    case TG_BLUE:           return "\e[44m";
    case TG_MAGENTA:        return "\e[45m";
    case TG_CYAN:           return "\e[46m";
    case TG_WHITE:          return "\e[47m";
    case TG_BRIGHT_BLACK:   return "\e[40;1m";
    case TG_BRIGHT_RED:     return "\e[41;1m";
    case TG_BRIGHT_GREEN:   return "\e[42;1m";
    case TG_BRIGHT_YELLOW:  return "\e[43;1m";
    case TG_BRIGHT_BLUE:    return "\e[44;1m";
    case TG_BRIGHT_MAGENTA: return "\e[45;1m";
    case TG_BRIGHT_CYAN:    return "\e[46;1m";
    case TG_BRIGHT_WHITE:   return "\e[47;1m";
  }
}

// Glyph attributes.
typedef struct
{
  tgForeground fg;
  tgBackground bg;
} tgAttribs;

// `tgAttribs` constructor.
// Creates glyph attributes with the specified
// foreground and background color.
tgAttribs
tg_attribs(tgForeground fg, tgBackground bg)
{
  return (tgAttribs)
    {
      .fg = fg,
      .bg = bg
    };
}

// Compares the two provided `tgAttribs`.
bool
tg_attribs_cmp(tgAttribs const *l, tgAttribs *r)
{
  return l->fg == r->fg && l->bg == r->bg;
}

// 2D Buffer containing glyphs and glyph attribtues. 
typedef struct
{
  tgVec2     size;
  int32_t   *glyphs;  // size.x * size.y
  tgAttribs *attribs; // size.x * size.y
} tgBuffer;

// `tgBuffer` constructor.
// Creates a 2d buffer with the specified width and height.
tgBuffer
tg_buffer(tgVec2 size)
{
  return (tgBuffer)
    {
      .size = size,
      .glyphs  = (int32_t *)calloc(size.x * size.y, sizeof(int32_t)),
      .attribs = (tgAttribs *)calloc(size.x * size.y, sizeof(tgAttribs))
    };
}

// Frees and zeros a `tgBuffer`.
tgBuffer
tg_buffer_del(tgBuffer *b)
{
  free(b->glyphs);
  free(b->attribs);
  memset((void *)b, 0, sizeof(tgBuffer));
}

// Retrieves a glyph in a `tgBuffer` at the specified x
// and y coordinates.
// NOTE: This does not do out of bounds checks.
int32_t *
tg_buffer_glyph_at(tgBuffer *b, tgVec2 p)
{
  return &b->glyphs[p.x * b->size.y + p.y];
}

// Retrieves the glyph attributes in a `tgBuffer` at the 
// specified x and y coordinates.
// NOTE: This does not do out of bounds checks.
tgAttribs *
tg_buffer_attribs_at(tgBuffer *b, tgVec2 p)
{
  return &b->attribs[p.x * b->size.y + p.y];
}

// Clears the contents of a `tgBuffer`.
void
tg_buffer_clear(tgBuffer *b)
{
  memset((void *)b->glyphs, 0, sizeof(int32_t) * b->size.x * b->size.y);
  memset((void *)b->attribs, 0, sizeof(tgAttribs) * b->size.x * b->size.y);
}

// Terminal renderer state.
typedef struct
{
  tgTerm *term;
  tgBuffer front, back; 
  tgVec2 cursor; 
} tgRenderer;

void 
tg__renderer_set_cursor(tgRenderer *, tgVec2);

// `tgRenderer` constructor.
// NOTE: This flushes to stdout and resets the cursor position.
tgRenderer
tg_renderer(tgTerm *t)
{
  tg_get_size(t);

  tgRenderer r = (tgRenderer)
    {
      .term   = t,
      .front  = tg_buffer(TG_VEC2(t->winsize.ws_col, t->winsize.ws_row)),
      .back   = tg_buffer(TG_VEC2(t->winsize.ws_col, t->winsize.ws_row)),
      .cursor = TG_VEC2(0, 0)
    };
  
  tg__set_cursor(TG_VEC2(0, 0));
  tg__flush();

  return r;
}

// Destroys and zeros a `tgRenderer`.
void
tg_renderer_del(tgRenderer *r)
{
  tg_buffer_del(&r->front);
  tg_buffer_del(&r->back);
  memset((void *)r, 0, sizeof(tgRenderer));
}

// Renders the backbuffer to the terminal screen 
// and then clears it.
// NOTE: This flushes stdout!
void
tg_render(tgRenderer *r)
{
  tg__hide_cursor();
  tgVec2 old_cursor = r->cursor; 
  tgVec2 cur = TG_VEC2(0, 0);

  fprintf(
    stdout, 
    "%s%s", 
    tg_foreground_str(TG_RESET), 
    tg_background_str(TG_RESET)
  );
  tgAttribs cur_attribs = tg_attribs(TG_RESET, TG_RESET);

  for (int16_t y = 0; y < r->front.size.y; ++y)
  {
    for (int16_t x = 0; x < r->front.size.x; ++x)
    {
      tgVec2 pos = TG_VEC2(x, y);

      int32_t    back_ch    = *tg_buffer_glyph_at(&r->back, pos),
                *front_ch   =  tg_buffer_glyph_at(&r->front, pos);
      tgAttribs  back_attr  = *tg_buffer_attribs_at(&r->back, pos);
      tgAttribs *front_attr =  tg_buffer_attribs_at(&r->front, pos);

      if (!tg_attribs_cmp(front_attr, &back_attr) 
       || !tg_attribs_cmp(&back_attr, &cur_attribs))
      {
        if (cur_attribs.fg != back_attr.fg) 
          fputs(tg_foreground_str(back_attr.fg), stdout);
        if (cur_attribs.bg != back_attr.bg) 
        {
          fputs(tg_background_str(back_attr.bg), stdout);
        
          // The reset color interferes with the foreground color
          // Theres no way to reset the background color specifically
          // So we're just going to reprint the foreground color temporarily.
          if (back_attr.bg == TG_RESET && back_attr.fg != TG_RESET)
            fputs(tg_foreground_str(back_attr.fg), stdout);
        }

        *front_attr = back_attr;
        cur_attribs = back_attr;
      }

      if (*front_ch != back_ch)
      {
        if (cur.x != x) tg__set_cursor(pos);

        if (0 == back_ch) 
        {
          // We've got to clear the old character on the terminal,
          // however, if theres a current background color active
          // the space will be colored, so we have to reset the space
          // char and then restore the foreground and background color.
          if (TG_RESET != cur_attribs.bg) 
            fprintf(
              stdout, 
              "%s %s%s", 
              tg_background_str(TG_RESET), 
              tg_foreground_str(cur_attribs.fg),
              tg_background_str(cur_attribs.bg)
            );
          else fputc(' ', stdout);
        } 
        else
        {
          uint8_t ch[4];
          uint8_t ch_len = tg__encode(back_ch, ch);
          fprintf(stdout, "%.*s", ch_len, (char *)ch);
          *front_ch = back_ch;
        }

        cur.x = x + 1;
      }
    }

    cur.x = 0;
    ++cur.y;
    tg__set_cursor(cur);
  }

  tg__set_cursor(old_cursor);
  tg__show_cursor();
  tg__flush();
  tg_buffer_clear(&r->back);
}

#undef TG__MAX

#endif // TERMGFX_TERMGFX_H