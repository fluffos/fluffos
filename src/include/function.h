/* codes returned by the functionp() efun */

#define FP_LOCAL              2
#define FP_EFUN               3
#define FP_SIMUL              4
#define FP_FUNCTIONAL         5

/* internal use */
#define FP_G_VAR              6
#define FP_L_VAR              7
#define FP_ANONYMOUS          8

/* additional flags */
#define FP_MASK             0x0f
#define FP_HAS_ARGUMENTS    0x10
#define FP_OWNER_DESTED     0x20
#define FP_NOT_BINDABLE     0x40
