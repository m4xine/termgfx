#include <termgfx.h>

int 
main(void)
{
  tgTerm t = tg_term();
  tg_raw_mode(&t);
  tg_enable_alt_screen();
  tgRenderer render = tg_renderer(&t);

  for (uint16_t x = 0; x < 5; ++x)
    for (uint16_t y = 0; y < 5; ++y)
    {
      if ((x * y) % 2 == 0)
        *tg_buffer_attribs_at(&render.back, TG_VEC2(x, y)) = tg_attribs(TG_RESET, TG_BLUE);

      *tg_buffer_glyph_at(&render.back, TG_VEC2(x, y)) = 53187; // 쿃
    }

  for (uint16_t x = 2; x < 20; ++x)
  {
    *tg_buffer_attribs_at(&render.back, TG_VEC2(x, 2)) = tg_attribs(TG_RED, TG_RESET);
    *tg_buffer_glyph_at(&render.back, TG_VEC2(x, 2)) = 'B';
  }

  tg_render(&render);

  tg__set_cursor(TG_VEC2(0, render.front.size.y - 1));
  printf("%hu %hu", render.front.size.x, render.front.size.y);
  tg__flush();

  sleep(5);
  tg_renderer_del(&render);
  tg_restore(&t);
  tg_disable_alt_screen();

  return 0;
}