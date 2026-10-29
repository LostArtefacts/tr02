#ifndef LARAFLAR_H
#define LARAFLAR_H

#define FL_HOLD_FT 1
#define FL_THROW_FT 32
#define FL_DRAW_FT 39
#define FL_IGNITE_FT 23
#define FL_2HOLD_FT 15

#define FL_HOLD_F 0
#define FL_THROW_F (FL_HOLD_F + FL_HOLD_FT)
#define FL_DRAW_F (FL_THROW_F + FL_THROW_FT)
#define FL_IGNITE_F (FL_DRAW_F + FL_DRAW_FT)
#define FL_2HOLD_F (FL_IGNITE_F + FL_IGNITE_FT)
#define FL_END_F (FL_2HOLD_F + FL_2HOLD_FT)

void set_flare_arm(int frame);
void draw_flare_meshes(void);
void undraw_flare_meshes(void);
void ready_flare(void);

#endif
