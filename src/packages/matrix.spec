package matrix;

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
    float *lookat_rotate2(float *, float, float, float, float, float, float);
