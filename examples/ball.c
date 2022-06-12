#define _XOPEN_SOURCE 600
#include <termgfx.h>

int 
main(void)
{
  tgTerm t = tg_term();
  tg_raw_mode(&t);
  tg_enable_alt_screen();
  tgRenderer render = tg_renderer(&t);

  tgVec2 ball_pos  = TG_VEC2(0, 0), 
         direction = TG_VEC2(1, 1);

  for (size_t i = 0; i < 200; ++i)
  {
    if(ball_pos.x + direction.x < 0) direction.x = -direction.x;
    if(ball_pos.y + direction.y < 0) direction.y = -direction.y;
    if(ball_pos.x + direction.x >= render.back.size.x) direction.x = -direction.x;
    if(ball_pos.y + direction.y >= render.back.size.y) direction.y = -direction.y;

    ball_pos = tg_vec2_add(ball_pos, direction);
    *tg_buffer_glyph_at(&render.back, ball_pos) = 'O';

    tg_render(&render);
    usleep(50 * 1000);
  }

  tg_renderer_del(&render);
  tg_restore(&t);
  tg_disable_alt_screen();

  return 0;
}