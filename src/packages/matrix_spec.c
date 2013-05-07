#include "spec.h"
/*
 * Matrix efuns for Jacques' 3d mud etc.
 */
    float *id_matrix();
    float *translate(float *, float, float, float);
    float *scale(float *, float, float, float);
    float *rotate_x(float *, float);
    float *rotate_y(float *, float);
    float *rotate_z(float *, float);
    float *lookat_rotate(float *, float, float, float);
/* for this efun to work again, the compiler needs support for min_arg > 4. */
/* The problem is that type checking was changed to be done for all
   arguments, and a limit of 4 args was imposed.  This can probably
   be fixed simply by forcing such efuns to check the additional ones,
   however this is the only efun affected so the fix is low priority. */
#if 0
    float *lookat_rotate2(float *, float, float, float, float, float, float);
#endif
