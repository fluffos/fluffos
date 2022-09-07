#include "base/std.h"

#include <unicode/stringpiece.h>

#include "comm.h"
#include "interactive.h"
#include "net/msp.h"
#include "net/sys_telnet.h"  // our own version of telnet header.
#include "net/telnet.h"
#include "thirdparty/libtelnet/libtelnet.h"  // for telnet_t, telnet_event_t*
#include "vm/vm.h"

// Protocol:
// https://wiki.mudlet.org/w/Manual:Supported_Protocols#MSP
// https://www.zuggsoft.com/zmud/msp.htm

void on_telnet_do_msp(interactive_t *ip) {
  if (!CONFIG_INT(__RC_ENABLE_MSP__)) return;
  ip->iflags |= USING_MSP;

  set_eval(max_eval_cost);
  safe_apply(APPLY_MSP_ENABLE, ip->ob, 0, ORIGIN_DRIVER);
}

void on_telnet_dont_msp(interactive_t *ip) {
  telnet_negotiate(ip->telnet, TELNET_WONT, TELNET_TELOPT_MSP);
  ip->iflags &= ~USING_MSP;
}

void telnet_send_msp_oob(interactive_t *ip, const char *msg, size_t len) {
  if (ip->iflags & USING_MSP) {
    telnet_subnegotiation(ip->telnet, TELNET_TELOPT_MSP, msg, len);
  }
}
