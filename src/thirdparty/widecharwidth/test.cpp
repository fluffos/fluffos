#include "wcwidth9.h"
#include "widechar_width.h"

// Return whether 'his' (wcwidth9) width is compatible with 'mine'
// (widechar_width) size.
static bool compatible(int his, int mine) {
  switch (mine) {
    case widechar_nonprint:
      return his == -1;
    case widechar_combining:
      return his == -1;
    case widechar_ambiguous:
      return his == -2;
    case widechar_private_use:
      return his == -3;
    case widechar_unassigned:
      return his == -1;
    case widechar_widened_in_9:
      return his == 1 || his == 2 || his == -2;
    default:
      return his == mine;
  }
}

// Characters that we expect to differ from wcwidth9.
static bool exceptional(int c) {
  // Characters new in Unicode 10 or 11
  if (widechar_wcwidth(c) != widechar_unassigned &&
      wcwidth9_intable(wcwidth9_not_assigned,
                       WCWIDTH9_ARRAY_SIZE(wcwidth9_not_assigned), c)) {
    return true;
  }
  switch (c) {
    case 0x20BF:  // bitcoin! new in Unicode 10 and 11

      // Format (Cf) characters which wcwidth9 reports as width 1 but should be
      // -1
    case 0x600:
    case 0x601:
    case 0x0602:
    case 0x0603:
    case 0x0604:
    case 0x0605:
    case 0x061C:
    case 0x06DD:
    case 0x08E2:

    case 0x2060:
    case 0x2061:
    case 0x2062:
    case 0x2063:
    case 0x2064:
    case 0x2066:
    case 0x2067:
    case 0x2068:
    case 0x2069:

    case 0x110BD:
    case 0x1BCA0:
    case 0x1BCA1:
    case 0x1BCA2:
    case 0x1BCA3:

    case 0x1D173:
    case 0x1D174:
    case 0x1D175:
    case 0x1D176:
    case 0x1D177:
    case 0x1D178:
    case 0x1D179:
    case 0x1D17A:

    case 0xE0001:

    case 0xE0020:
    case 0xE0021:
    case 0xE0022:
    case 0xE0023:
    case 0xE0024:
    case 0xE0025:
    case 0xE0026:
    case 0xE0027:
    case 0xE0028:
    case 0xE0029:
    case 0xE002A:
    case 0xE002B:
    case 0xE002C:
    case 0xE002D:
    case 0xE002E:
    case 0xE002F:
    case 0xE0030:
    case 0xE0031:
    case 0xE0032:
    case 0xE0033:
    case 0xE0034:
    case 0xE0035:
    case 0xE0036:
    case 0xE0037:
    case 0xE0038:
    case 0xE0039:
    case 0xE003A:
    case 0xE003B:
    case 0xE003C:
    case 0xE003D:
    case 0xE003E:
    case 0xE003F:
    case 0xE0040:
    case 0xE0041:
    case 0xE0042:
    case 0xE0043:
    case 0xE0044:
    case 0xE0045:
    case 0xE0046:
    case 0xE0047:
    case 0xE0048:
    case 0xE0049:
    case 0xE004A:
    case 0xE004B:
    case 0xE004C:
    case 0xE004D:
    case 0xE004E:
    case 0xE004F:
    case 0xE0050:
    case 0xE0051:
    case 0xE0052:
    case 0xE0053:
    case 0xE0054:
    case 0xE0055:
    case 0xE0056:
    case 0xE0057:
    case 0xE0058:
    case 0xE0059:
    case 0xE005A:
    case 0xE005B:
    case 0xE005C:
    case 0xE005D:
    case 0xE005E:
    case 0xE005F:
    case 0xE0060:
    case 0xE0061:
    case 0xE0062:
    case 0xE0063:
    case 0xE0064:
    case 0xE0065:
    case 0xE0066:
    case 0xE0067:
    case 0xE0068:
    case 0xE0069:
    case 0xE006A:
    case 0xE006B:
    case 0xE006C:
    case 0xE006D:
    case 0xE006E:
    case 0xE006F:
    case 0xE0070:
    case 0xE0071:
    case 0xE0072:
    case 0xE0073:
    case 0xE0074:
    case 0xE0075:
    case 0xE0076:
    case 0xE0077:
    case 0xE0078:
    case 0xE0079:
    case 0xE007A:
    case 0xE007B:
    case 0xE007C:
    case 0xE007D:
    case 0xE007E:
    case 0xE007F:

      // This appears to have changed category from Po to Mn
    case 0x111C9:

      // Some unassigneds.
    case 0x10FFFE:
    case 0x10FFFF:

      return true;
  }
  return false;
}

int run_tests(void) {
  int ret = 0;
  int max_cp = 0x110000;
  for (int c = 0; c <= max_cp; c++) {
    auto w1 = wcwidth9(c);
    auto w2 = widechar_wcwidth(c);
    if (compatible(w1, w2) || exceptional(c)) continue;
    printf("%04X: his %d, mine %d, system: %d\n", c, w1, w2, wcwidth(c));
    ret = EXIT_FAILURE;
  }
  return ret;
}

int main(void) {
  int ret = 0;
  ret |= run_tests();
  printf("Tests %s\n", ret == EXIT_SUCCESS ? "passed" : "failed");
  return ret;
}
