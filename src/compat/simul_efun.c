/*
 * Author: Matt Messier (Marius)
 * Description: Implementations of efuns that used to be a part of the driver
 *              but have been removed in favour of implementing them in LPC.
 */

// origin.h can be found in the driver include directory
#include <origin.h>

object simul(void)
{
    return this_object();
}

#include "simuls/apply.c"
//#include "simuls/break_string.c"
#include "simuls/call_out.c"
#include "simuls/dump_socket_status.c"
#include "simuls/ed.c"
#include "simuls/heart_beat.c"
#include "simuls/message.c"
//#include "simuls/parse_command.c"
#include "simuls/printf.c"
//#include "simuls/process_string.c"
#include "simuls/say.c"
#include "simuls/shout.c"
//#include "simuls/tail.c"
#include "simuls/tell_object.c"
#include "simuls/tell_room.c"
#include "simuls/wizard.c"
#include "simuls/write.c"
