#define _XOPEN_SOURCE 600
#include <termgfx.h>

int
main(void)
{
  tgTerm t = tg_term();
  tg_raw_mode(&t);
  tg_enable_alt_screen();
  tgRenderer render = tg_renderer(&t);

  tgBuffer buf1 = tg_buffer(TG_VEC2(20, 10)),
           buf2 = tg_buffer(TG_VEC2(10, 5));

  for (int16_t x = 0; x < 20; ++x)
    for (int16_t y = 0; y < 10; ++y)
    {
      if (x < 10 && y < 5)
      {
        *tg_buffer_glyph_at(&buf2, TG_VEC2(x, y)) = 'B';
        *tg_buffer_attribs_at(&buf2, TG_VEC2(x, y)) = tg_attribs(TG_RESET, TG_RED);
      }
      
      *tg_buffer_glyph_at(&buf1, TG_VEC2(x, y)) = 'A';
      *tg_buffer_attribs_at(&buf1, TG_VEC2(x, y)) = tg_attribs(TG_RESET, TG_BLUE);
    }

  tg_buffer_blit(
    &buf1, 
    &buf2, 
    &TG_RECT(0, 0, buf1.size.x, buf1.size.y),
    &TG_RECT(0, 0, buf2.size.x, buf2.size.y)
  );
  tg_buffer_blit(
    &render.back, 
    &buf1, 
    &TG_RECT(0, 0, render.back.size.x, render.back.size.y),
    &TG_RECT(0, 0, buf1.size.x, buf1.size.y)
  );

  tg_render(&render);
  sleep(3);
  
  tg_renderer_del(&render);
  tg_restore(&t);
  tg_disable_alt_screen();

  return 0;
}