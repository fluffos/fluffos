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
        case widechar_non_character:
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
        wcwidth9_intable(wcwidth9_not_assigned, WCWIDTH9_ARRAY_SIZE(wcwidth9_not_assigned), c)) {
        return true;
    }
    switch (c) {
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

        // wcwidth thinks these two are combining, but they are not.
        case 0x1CF2:
        case 0x1CF3:

        case 0x2060:
        case 0x2061:
        case 0x2062:
        case 0x2063:
        case 0x2064:
        case 0x2066:
        case 0x2067:
        case 0x2068:
        case 0x2069:

        // bitcoin! new in Unicode 10 and 11
        case 0x20BF:

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

        // regional indicator symbols.
        // EastAsianWidth says these are narrow even though they appear in
        // emoji-data.txt. It's a mess.
        case 0x1F1E6:
        case 0x1F1E7:
        case 0x1F1E8:
        case 0x1F1E9:
        case 0x1F1EA:
        case 0x1F1EB:
        case 0x1F1EC:
        case 0x1F1ED:
        case 0x1F1EE:
        case 0x1F1EF:
        case 0x1F1F0:
        case 0x1F1F1:
        case 0x1F1F2:
        case 0x1F1F3:
        case 0x1F1F4:
        case 0x1F1F5:
        case 0x1F1F6:
        case 0x1F1F7:
        case 0x1F1F8:
        case 0x1F1F9:
        case 0x1F1FA:
        case 0x1F1FB:
        case 0x1F1FC:
        case 0x1F1FD:
        case 0x1F1FE:
        case 0x1F1FF:

        // Some more emoji explicitly marked as narrow in EAW.
        case 0x1F321:
        case 0x1F324:
        case 0x1F325:
        case 0x1F326:
        case 0x1F327:
        case 0x1F328:
        case 0x1F329:
        case 0x1F32A:
        case 0x1F32B:
        case 0x1F32C:
        case 0x1F336:
        case 0x1F37D:
        case 0x1F396:
        case 0x1F397:
        case 0x1F399:
        case 0x1F39A:
        case 0x1F39B:
        case 0x1F39E:
        case 0x1F39F:
        case 0x1F3CB:
        case 0x1F3CC:
        case 0x1F3CD:
        case 0x1F3CE:
        case 0x1F3D4:
        case 0x1F3D5:
        case 0x1F3D6:
        case 0x1F3D7:
        case 0x1F3D8:
        case 0x1F3D9:
        case 0x1F3DA:
        case 0x1F3DB:
        case 0x1F3DC:
        case 0x1F3DD:
        case 0x1F3DE:
        case 0x1F3DF:
        case 0x1F3F3:
        case 0x1F3F5:
        case 0x1F3F7:
        case 0x1F43F:
        case 0x1F441:
        case 0x1F4FD:
        case 0x1F549:
        case 0x1F54A:
        case 0x1F56F:
        case 0x1F570:
        case 0x1F573:
        case 0x1F574:
        case 0x1F575:
        case 0x1F576:
        case 0x1F577:
        case 0x1F578:
        case 0x1F579:
        case 0x1F587:
        case 0x1F58A:
        case 0x1F58B:
        case 0x1F58C:
        case 0x1F58D:
        case 0x1F590:
        case 0x1F5A5:
        case 0x1F5A8:
        case 0x1F5B1:
        case 0x1F5B2:
        case 0x1F5BC:
        case 0x1F5C2:
        case 0x1F5C3:
        case 0x1F5C4:
        case 0x1F5D1:
        case 0x1F5D2:
        case 0x1F5D3:
        case 0x1F5DC:
        case 0x1F5DD:
        case 0x1F5DE:
        case 0x1F5E1:
        case 0x1F5E3:
        case 0x1F5E8:
        case 0x1F5EF:
        case 0x1F5F3:
        case 0x1F5FA:
        case 0x1F6CB:
        case 0x1F6CD:
        case 0x1F6CE:
        case 0x1F6CF:
        case 0x1F6E0:
        case 0x1F6E1:
        case 0x1F6E2:
        case 0x1F6E3:
        case 0x1F6E4:
        case 0x1F6E5:
        case 0x1F6E9:
        case 0x1F6F0:
        case 0x1F6F3:
        case 0x1F93B:
        case 0x1F946:

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
