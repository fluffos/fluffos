---
layout: doc
title: cli / lpcc
---

# cli / lpcc

`lpcc` is a command line tool to dump compiled LPC code, it is useful for understanding how FluffOS compiles LPC code.

## Usage

```bash
./lpcc config_file lpc_file
```

## Example output

```bash
$ ./lpcc etc/config.test ../test.c
Processing config file: etc/config.test
maximum local variables: invalid new value, resetting to default.
living hash table size: invalid new value, resetting to default.
New Debug log location: "log/debug.log".
Execution root: ./
Initializing internal stuff ....
Event backend in use: epoll
Loading simul_efun file : /single/simul_efun
/std/base64.c line 56: Warning: Unused local variable 'p'
  }
   ^
/std/base64.c line 56: Warning: Unused local variable 'rlen'
  }
   ^
/std/number_string.c line 54: Warning: Unused local variable 'parts'
  }
   ^
/std/number_string.c line 54: Warning: Unused local variable 'part'
  }
   ^
Loading master file: /single/master
NAME: /single/master.c
INHERITS:
      name                    fio    vio
      ----------------        ---    ---
        inherit/master/valid.c      0      0
FUNCTIONS:
      name                  offset  mods   flags   fio  # locals  # args # def args
      --------------------- ------  ----  -------  ---  --------  ------ ----------
   0: get_inherit_called         0   +--  i-s----    0
   1: valid_shadow               0   +--  i-s----    1
   2: valid_author               0   +--  i-s----    2
   3: valid_override             0   +--  i-s----    3
   4: valid_seteuid              0   +--  i-s----    4
   5: valid_domain               0   +--  i-s----    5
   6: valid_socket               0   +--  i-s----    6
   7: valid_write                0   +--  i-s----    7
   8: valid_read                 0   +--  i-s----    8
   9: valid_bind                 0   +--  i-s----    9
  10: valid_hide                 0   +--  i-s----   10
  11: create                     0   +--  i-s----   11
  12: clear_last_error           0   +--  --s----              0       0          0
  13: trace_to_last_assert       1   ---  --s----              2       0          0
  14: catch_tell                 2   +--  --s----              0       1          0
  15: compile_object             3   +--  --s----              0       1          0
  16: update_file                4   ---  --s----              3       1          0
  17: log_error                  5   +--  --s----              0       2          0
  18: save_ed_setup              6   +--  --s----              1       2          0
  19: retrieve_ed_setup          7   +--  --s----              2       1          0
  20: destruct_environment_of       8   +--  --s----              0       1          0
  21: make_path_absolute         9   +--  --s----              0       1          0
  22: get_root_uid              10   +--  --s----              0       0          0
  23: get_bb_uid                11   +--  --s----              0       0          0
  24: privs_file                12   +--  --s----              0       1          0
  25: error_handler             13   ---  --s----              2       2          0
  26: get_include_path          14   +--  --s----              0       1          0
  27: valid_database            15   +--  --s----              0       3          0
  28: object_name               16   +--  --s----              0       1          0
  29: get_last_error            17   +--  --s----              0       0          0
  30: domain_file               18   +--  --s----              0       1          0
  31: creator_file              19   +--  --s----              0       1          0
  32: author_file               20   +--  --s----              0       1          0
  33: flag                      21   +--  --s----              1       1          0
  34: connect                   22   +--  --s----              2       0          0
  35: crash                     23   ---  --s----              1       3          0
  36: epilog                    24   +--  --s----              1       1          0
  37: preload                   25   +--  --s----              2       1          0
  38: #global_init#             26   ---  --s----              0       0          0

;;; inherit/master/valid.c

VARIABLES:
   0: public int inherit_called
STRINGS:
   0: inherit/master/valid.c
   1: include/globals.h
   2: include/tests.h
   3: Root
   4: query_prevent_shadow
   5: /single/simul_efun
   6: move_object
   7: /inherit/base
DISASSEMBLY:

;; Function: int get_inherit_called()
0000:  43 00 00                  global                             ; inherit_called(0)
0003:  2E                        return                             ;
; inherit/master/valid.c:10

;; Function: void create()
0004:  2F                        return_zero                        ;
; inherit/master/valid.c:10

;; Function: int valid_shadow(object)
0005:  3F 00                     local                              ; LV0
0007:  04 B9 01                  F_EFUN1                            ; EFUN: getuid(441)
000a:  0E 03                     short_string                       ; "Root"
000c:  13 04 00                  branch_ne                          ; 0004 (0011)
000f:  10                        const1                             ;
0010:  2E                        return                             ;
; inherit/master/valid.c:20
0011:  3C 00                     transfer_local                     ; LV0
0013:  02 02 04 40               F_PUSH                             ; push string 4, number 0
0017:  04 8F 00                  F_EFUN1                            ; EFUN: previous_object(143)
001a:  07 87 00 03               F_EFUNV                            ; EFUN: call_other(135)
001e:  17 03 00                  branch_when_zero                   ; 0003 (0022)
0021:  2F                        return_zero                        ;
0022:  10                        const1                             ;
0023:  2E                        return                             ;
0024:  2F                        return_zero                        ;
; inherit/master/valid.c:24

;; Function: int valid_author(string)
0025:  10                        const1                             ;
0026:  2E                        return                             ;
; inherit/master/valid.c:24

;; Function: int valid_override(string,string)
0027:  02 02 C0 05               F_PUSH                             ; push local 0, string 5
002b:  13 04 00                  branch_ne                          ; 0004 (0030)
002e:  10                        const1                             ;
002f:  2E                        return                             ;
; inherit/master/valid.c:45
0030:  3C 01                     transfer_local                     ; LV1
0032:  0E 06                     short_string                       ; "move_object"
0034:  30                        ==                                 ;
0035:  26 07 00                  &&                                 ; 0007 (003d)
0038:  02 02 C0 07               F_PUSH                             ; push local 0, string 7
003c:  31                        !=                                 ;
003d:  17 03 00                  branch_when_zero                   ; 0003 (0041)
0040:  2F                        return_zero                        ;
0041:  10                        const1                             ;
0042:  2E                        return                             ;
0043:  2F                        return_zero                        ;
; inherit/master/valid.c:48

;; Function: int valid_seteuid(object,string)
0044:  10                        const1                             ;
0045:  2E                        return                             ;
; inherit/master/valid.c:48

;; Function: int valid_domain(string)
0046:  10                        const1                             ;
0047:  2E                        return                             ;
; inherit/master/valid.c:48

;; Function: int valid_socket(object,string,mixed *)
0048:  10                        const1                             ;
0049:  2E                        return                             ;
; inherit/master/valid.c:48

;; Function: int valid_write(string,mixed,string)
004a:  44 00 00                  global_lvalue                      ; inherit_called(0)
004d:  36                        inc(x)                             ;
004e:  10                        const1                             ;
004f:  2E                        return                             ;
; inherit/master/valid.c:90

;; Function: int valid_read(string,mixed,string)
0050:  44 00 00                  global_lvalue                      ; inherit_called(0)
0053:  36                        inc(x)                             ;
0054:  10                        const1                             ;
0055:  2E                        return                             ;
; inherit/master/valid.c:99

;; Function: int valid_bind()
0056:  44 00 00                  global_lvalue                      ; inherit_called(0)
0059:  36                        inc(x)                             ;
005a:  10                        const1                             ;
005b:  2E                        return                             ;
; inherit/master/valid.c:104

;; Function: int valid_hide()
005c:  44 00 00                  global_lvalue                      ; inherit_called(0)
005f:  36                        inc(x)                             ;
0060:  10                        const1                             ;
0061:  2E                        return                             ;
; inherit/master/valid.c:110

;; Function: void #global_init#()
0062:  0F                        const0                             ;
0063:  44 00 00                  global_lvalue                      ; inherit_called(0)
0066:  61                        (void)assign                       ;
0067:  2F                        return_zero                        ;
; inherit/master/valid.c:8

;;;  *** Line Number Info ***

absolute line -> (file, line) table:
0 lines from 1 [inherit/master/valid.c]
11 lines from 2 [include/globals.h]
27 lines from 3 [include/tests.h]
25 lines from 2 [include/globals.h]
6 lines from 1 [inherit/master/valid.c]
11 lines from 2 [include/globals.h]
27 lines from 3 [include/tests.h]
25 lines from 2 [include/globals.h]
108 lines from 1 [inherit/master/valid.c]

address -> absolute line table:
0000-0004: 136
0005-0010: 146
0011-0026: 150
0027-002f: 171
0030-0049: 174
004a-004f: 216
0050-0055: 225
0056-005b: 230
005c-0061: 236
0062-0067: 134

;;; single/master.c

VARIABLES:
   0: public nosave int has_error
   1: public nosave string last_error
STRINGS:
   0: single/master.c
   1: include/globals.h
   2: include/tests.h
   3:
   4: function
   5: __assert
   6: %O
   7: test
   8: /command/tests
   9: main
  10: speed
  11: /command/speed
  12: The only supproted flag is 'test
  13: '.\n
  14: /clone/login
  15: It looks like someone is working
  16: MASTER: compile_object is called
  17: \n
  18: /test/virtual
  19: /single/void
  20: Master object shouts: Damn!\nMast
  21: /etc/preload
  22: .c
  23: Preloading :
  24: ...
  25: ??
  26: \nError
  27:  when loading
  28: (
  29: .
  30: )\n
  31: /log/compile
  32: .edrc
  33: %d
  34: The object containing you was de
  35: move
  36: query_cwd
  37: Root
  38: Backbone
  39: /single/simul_efun
  40: creator_file
  41: domain_file
  42: author_file
  43: program
  44: /single/tests/efuns/call_out.c
  45: error
  46: *Too long evaluation. Execution
  47: *Error caught\n
  48: Error: %s\nCurrent object: %O\nCur
  49: object
  50: No current object
  51: No current program
  52: file
  53: line
  54: trace
  55: Line: %O  File: %O Object: %O Pr
  56: No object
  57: No program
  58: /log/log
  59: /clone/mgip1
  60: /clone/mgip1.c
  61: /include/m_gip1
  62: /include
  63: /clone/mgip2
  64: /clone/mgip2.c
  65: /include/m_gip2
  66: /clone/mgip3
  67: /clone/mgip3.c
  68: /clone/mgip4
  69: /clone/mgip4.c
  70: :DEFAULT:
  71: MASTER valid_database called:
  72: ob:%O action:%O info:%O
  73: name
DISASSEMBLY:

;; Function: string clear_last_error()
0000:  0E 03                     short_string                       ; ""
0002:  44 02 00                  global_lvalue                      ; last_error(2)
0005:  61                        (void)assign                       ;
0006:  2F                        return_zero                        ;
; single/master.c:12

;; Function: mapping * trace_to_last_assert()
0007:  03 49 01                  F_EFUN0                            ; EFUN: dump_trace(329)
000a:  62 00                     (void)assign_local                 ; LV0
; single/master.c:17
000c:  0F                        const0                             ;
000d:  62 01                     (void)assign_local                 ; LV1
000f:  19 1A 00                  branch                             ; 001a (002a)
; single/master.c:18
0012:  02 05 40 47 04 C1 C0      F_PUSH                             ; push number 0, number 7, string 4, local 1, local 0
0019:  47                        index                              ;
001a:  47                        index                              ;
001b:  4B                        nn_range                           ;
001c:  0E 05                     short_string                       ; "__assert"
001e:  13 09 00                  branch_ne                          ; 0009 (0028)
; single/master.c:19
0021:  02 03 40 C1 C0            F_PUSH                             ; push number 0, local 1, local 0
0026:  4B                        nn_range                           ;
0027:  2E                        return                             ;
; single/master.c:20
0028:  23 01                     loop_incr                          ; LV1
; single/master.c:22
002a:  02 02 C1 C0               F_PUSH                             ; push local 1, local 0
002e:  04 91 00                  F_EFUN1                            ; EFUN: sizeof(145)
0031:  1D 20 00                  bbranch_lt                         ; 0020 (0012)
; single/master.c:18
0034:  3C 00                     transfer_local                     ; LV0
0036:  2E                        return                             ;
0037:  2F                        return_zero                        ;
; single/master.c:23

;; Function: string get_last_error()
0038:  02 02 82 03               F_PUSH                             ; push global 2, string 3
003c:  13 0D 00                  branch_ne                          ; 000d (004a)
; single/master.c:27
003f:  0E 06                     short_string                       ; "%O"
0041:  2C 0D 00 00               F_CALL_FUNCTION_BY_ADDRESS         ; trace_to_last_assert, pushed_args:0
0045:  07 0E 01 02               F_EFUNV                            ; EFUN: sprintf(270)
0049:  2E                        return                             ;
; single/master.c:28
004a:  43 02 00                  global                             ; last_error(2)
004d:  2E                        return                             ;
; single/master.c:30

;; Function: void flag(string)
004e:  3F 00                     local                              ; LV0
0050: switch
      type: 1f table: 00af-00c3 deflt: 009e
0058:  27 0C 00                  catch                              ; 000c
005b:  02 02 08 09               F_PUSH                             ; push string 8, string 9
005f:  07 87 00 02               F_EFUNV                            ; EFUN: call_other(135)
0063:  01                        pop                                ;
0064:  28                        end_catch                          ;
0065:  62 01                     (void)assign_local                 ; LV1
; single/master.c:37
0067:  3F 01                     local                              ; LV1
0069:  17 0C 00                  branch_when_zero                   ; 000c (0076)
; single/master.c:38
006c:  10                        const1                             ;
006d:  44 01 00                  global_lvalue                      ; has_error(1)
0070:  61                        (void)assign                       ;
; single/master.c:39
0071:  3F 01                     local                              ; LV1
0073:  04 C1 00                  F_EFUN1                            ; EFUN: write(193)
0076:  19 4C 00                  branch                             ; 004c (00c3)
; single/master.c:40
0079:  27 0C 00                  catch                              ; 000c
007c:  02 02 0B 09               F_PUSH                             ; push string 11, string 9
0080:  07 87 00 02               F_EFUNV                            ; EFUN: call_other(135)
0084:  01                        pop                                ;
0085:  28                        end_catch                          ;
0086:  62 01                     (void)assign_local                 ; LV1
; single/master.c:44
0088:  3F 01                     local                              ; LV1
008a:  17 0C 00                  branch_when_zero                   ; 000c (0097)
; single/master.c:45
008d:  10                        const1                             ;
008e:  44 01 00                  global_lvalue                      ; has_error(1)
0091:  61                        (void)assign                       ;
; single/master.c:46
0092:  3F 01                     local                              ; LV1
0094:  04 C1 00                  F_EFUN1                            ; EFUN: write(193)
; single/master.c:47
0097:  0F                        const0                             ;
0098:  04 48 01                  F_EFUN1                            ; EFUN: shutdown(328)
009b:  19 27 00                  branch                             ; 0027 (00c3)
; single/master.c:49
009e:  02 02 0C C0               F_PUSH                             ; push string 12, local 0
00a2:  63                        +                                  ;
00a3:  0E 0D                     short_string                       ; "'.\n"
00a5:  63                        +                                  ;
00a6:  04 C1 00                  F_EFUN1                            ; EFUN: write(193)
00a9:  19 19 00                  branch                             ; 0019 (00c3)
00ac:  19 16 00                  branch                             ; 0016 (00c3)
; single/master.c:52
      switch table (for 0050)
        "test"  0058
        "speed" 0079
; single/master.c:35
00c3:  43 01 00                  global                             ; has_error(1)
00c6:  17 07 00                  branch_when_zero                   ; 0007 (00ce)
00c9:  0C 01                     -byte                              ; -1
00cb:  04 48 01                  F_EFUN1                            ; EFUN: shutdown(328)
00ce:  2F                        return_zero                        ;
; single/master.c:55

;; Function: void catch_tell(string)
00cf:  10                        const1                             ;
00d0:  44 01 00                  global_lvalue                      ; has_error(1)
00d3:  61                        (void)assign                       ;
00d4:  2F                        return_zero                        ;
; single/master.c:60

;; Function: object connect()
00d5:  27 0B 00                  catch                              ; 000b
00d8:  0E 0E                     short_string                       ; "/clone/login"
00da:  07 8C 00 01               F_EFUNV                            ; EFUN: new(140)
00de:  62 00                     (void)assign_local                 ; LV0
00e0:  28                        end_catch                          ;
00e1:  62 01                     (void)assign_local                 ; LV1
; single/master.c:68
00e3:  3F 01                     local                              ; LV1
00e5:  17 12 00                  branch_when_zero                   ; 0012 (00f8)
; single/master.c:70
00e8:  0E 0F                     short_string                       ; "It looks like someone is work"
00ea:  04 C1 00                  F_EFUN1                            ; EFUN: write(193)
; single/master.c:71
00ed:  3F 01                     local                              ; LV1
00ef:  04 C1 00                  F_EFUN1                            ; EFUN: write(193)
; single/master.c:72
00f2:  03 89 00                  F_EFUN0                            ; EFUN: this_object(137)
00f5:  04 92 00                  F_EFUN1                            ; EFUN: destruct(146)
; single/master.c:73
00f8:  3C 00                     transfer_local                     ; LV0
00fa:  2E                        return                             ;
00fb:  2F                        return_zero                        ;
; single/master.c:75

;; Function: mixed compile_object(string)
00fc:  02 02 10 C0               F_PUSH                             ; push string 16, local 0
0100:  63                        +                                  ;
0101:  0E 11                     short_string                       ; "\n"
0103:  63                        +                                  ;
0104:  04 C1 00                  F_EFUN1                            ; EFUN: write(193)
; single/master.c:85
0107:  3C 00                     transfer_local                     ; LV0
0109:  0E 12                     short_string                       ; "/test/virtual"
010b:  13 0A 00                  branch_ne                          ; 000a (0116)
; single/master.c:86
010e:  02 02 13 41               F_PUSH                             ; push string 19, number 1
0112:  05 C6 00                  F_EFUN2                            ; EFUN: find_object(198)
0115:  2E                        return                             ;
0116:  2F                        return_zero                        ;
0117:  2F                        return_zero                        ;
; single/master.c:87

;; Function: void crash(string,object,object)
0118:  03 BE 00                  F_EFUN0                            ; EFUN: users(190)
011b:  1E 00 03                  foreach                            ; (array | string) local 3
011e:  19 09 00                  branch                             ; 0009 (0128)
; single/master.c:98
0121:  02 02 C3 14               F_PUSH                             ; push local 3, string 20
0125:  05 C2 00                  F_EFUN2                            ; EFUN: tell_object(194)
0128:  1F 08 00                  next_foreach                       ; 0008 (0121)
012b:  2F                        return_zero                        ;
; single/master.c:99

;; Function: string * update_file(string)
012c:  3C 00                     transfer_local                     ; LV0
012e:  04 E6 00                  F_EFUN1                            ; EFUN: read_file(230)
0131:  62 02                     (void)assign_local                 ; LV2
; single/master.c:124
0133:  3F 02                     local                              ; LV2
0135:  18 06 00                  branch_when_non_zero               ; 0006 (013c)
; single/master.c:125
0138:  11 00 00                  aggregate                          ; 0
013b:  2E                        return                             ;
; single/master.c:126
013c:  3C 02                     transfer_local                     ; LV2
013e:  0E 11                     short_string                       ; "\n"
0140:  05 95 00                  F_EFUN2                            ; EFUN: explode(149)
0143:  62 01                     (void)assign_local                 ; LV1
; single/master.c:128
0145:  0F                        const0                             ;
0146:  62 03                     (void)assign_local                 ; LV3
0148:  19 18 00                  branch                             ; 0018 (0161)
; single/master.c:129
014b:  02 03 40 C3 C1            F_PUSH                             ; push number 0, local 3, local 1
0150:  47                        index                              ;
0151:  47                        index                              ;
0152:  0B 23                     byte                               ; 35
0154:  13 0A 00                  branch_ne                          ; 000a (015f)
; single/master.c:130
0157:  02 02 40 C3               F_PUSH                             ; push number 0, local 3
015b:  40 01                     local_lvalue                       ; LV1
015d:  48                        index_lvalue                       ;
015e:  61                        (void)assign                       ;
; single/master.c:131
015f:  23 03                     loop_incr                          ; LV3
; single/master.c:133
0161:  02 02 C3 C1               F_PUSH                             ; push local 3, local 1
0165:  04 91 00                  F_EFUN1                            ; EFUN: sizeof(145)
0168:  1D 1E 00                  bbranch_lt                         ; 001e (014b)
; single/master.c:129
016b:  3C 01                     transfer_local                     ; LV1
016d:  2E                        return                             ;
016e:  2F                        return_zero                        ;
; single/master.c:134

;; Function: string * epilog(int)
016f:  0E 15                     short_string                       ; "/etc/preload"
0171:  2C 10 00 01               F_CALL_FUNCTION_BY_ADDRESS         ; update_file, pushed_args:1
0175:  62 01                     (void)assign_local                 ; LV1
; single/master.c:143
0177:  3C 01                     transfer_local                     ; LV1
0179:  2E                        return                             ;
; single/master.c:144

;; Function: void preload(string)
017a:  02 02 C0 16               F_PUSH                             ; push local 0, string 22
017e:  63                        +                                  ;
017f:  04 E4 00                  F_EFUN1                            ; EFUN: file_size(228)
0182:  0C 01                     -byte                              ; -1
0184:  13 03 00                  branch_ne                          ; 0003 (0188)
0187:  2F                        return_zero                        ;
; single/master.c:153
0188:  03 F2 00                  F_EFUN0                            ; EFUN: time(242)
018b:  62 01                     (void)assign_local                 ; LV1
; single/master.c:156
018d:  02 02 17 C0               F_PUSH                             ; push string 23, local 0
0191:  63                        +                                  ;
0192:  0E 18                     short_string                       ; "..."
0194:  63                        +                                  ;
0195:  04 C1 00                  F_EFUN1                            ; EFUN: write(193)
; single/master.c:157
0198:  27 0C 00                  catch                              ; 000c
019b:  02 02 C0 19               F_PUSH                             ; push local 0, string 25
019f:  07 87 00 02               F_EFUNV                            ; EFUN: call_other(135)
01a3:  01                        pop                                ;
01a4:  28                        end_catch                          ;
01a5:  62 02                     (void)assign_local                 ; LV2
; single/master.c:158
01a7:  3F 02                     local                              ; LV2
01a9:  17 16 00                  branch_when_zero                   ; 0016 (01c0)
; single/master.c:159
01ac:  02 02 1A C2               F_PUSH                             ; push string 26, local 2
01b0:  63                        +                                  ;
01b1:  0E 1B                     short_string                       ; " when loading "
01b3:  63                        +                                  ;
01b4:  3F 00                     local                              ; LV0
01b6:  63                        +                                  ;
01b7:  0E 11                     short_string                       ; "\n"
01b9:  63                        +                                  ;
01ba:  04 C1 00                  F_EFUN1                            ; EFUN: write(193)
01bd:  19 20 00                  branch                             ; 0020 (01de)
; single/master.c:160
01c0:  03 F2 00                  F_EFUN0                            ; EFUN: time(242)
01c3:  3F 01                     local                              ; LV1
01c5:  64                        subtract                           ;
01c6:  62 01                     (void)assign_local                 ; LV1
; single/master.c:162
01c8:  02 03 1C C1 7C            F_PUSH                             ; push string 28, local 1, number 60
01cd:  66                        /                                  ;
01ce:  63                        +                                  ;
01cf:  0E 1D                     short_string                       ; "."
01d1:  63                        +                                  ;
01d2:  02 02 C1 7C               F_PUSH                             ; push local 1, number 60
01d6:  67                        %                                  ;
01d7:  63                        +                                  ;
01d8:  0E 1E                     short_string                       ; ")\n"
01da:  63                        +                                  ;
01db:  04 C1 00                  F_EFUN1                            ; EFUN: write(193)
01de:  2F                        return_zero                        ;
; single/master.c:163

;; Function: void log_error(string,string)
01df:  0E 1F                     short_string                       ; "/log/compile"
01e1:  3C 01                     transfer_local                     ; LV1
01e3:  0F                        const0                             ;
01e4:  06 E1 00                  F_EFUN3                            ; EFUN: write_file(225)
01e7:  01                        pop                                ;
01e8:  2F                        return_zero                        ;
; single/master.c:172

;; Function: int save_ed_setup(object,int)
01e9:  3F 01                     local                              ; LV1
01eb:  04 CD 00                  F_EFUN1                            ; EFUN: intp(205)
01ee:  18 03 00                  branch_when_non_zero               ; 0003 (01f2)
01f1:  2F                        return_zero                        ;
; single/master.c:183
01f2:  3C 00                     transfer_local                     ; LV0
01f4:  04 B9 01                  F_EFUN1                            ; EFUN: getuid(441)
01f7:  71 15 00 01               simul_efun                         ; "user_path" args: 1
01fb:  0E 20                     short_string                       ; ".edrc"
01fd:  63                        +                                  ;
01fe:  62 02                     (void)assign_local                 ; LV2
; single/master.c:187
0200:  3F 02                     local                              ; LV2
0202:  04 EA 00                  F_EFUN1                            ; EFUN: rm(234)
0205:  01                        pop                                ;
; single/master.c:191
0206:  3C 02                     transfer_local                     ; LV2
0208:  3C 01                     transfer_local                     ; LV1
020a:  0E 03                     short_string                       ; ""
020c:  63                        +                                  ;
020d:  0F                        const0                             ;
020e:  06 E1 00                  F_EFUN3                            ; EFUN: write_file(225)
0211:  2E                        return                             ;
0212:  2F                        return_zero                        ;
; single/master.c:192

;; Function: int retrieve_ed_setup(object)
0213:  3C 00                     transfer_local                     ; LV0
0215:  04 B9 01                  F_EFUN1                            ; EFUN: getuid(441)
0218:  71 15 00 01               simul_efun                         ; "user_path" args: 1
021c:  0E 20                     short_string                       ; ".edrc"
021e:  63                        +                                  ;
021f:  62 01                     (void)assign_local                 ; LV1
; single/master.c:204
0221:  3F 01                     local                              ; LV1
0223:  04 E4 00                  F_EFUN1                            ; EFUN: file_size(228)
0226:  0F                        const0                             ;
0227:  32                        <=                                 ;
0228:  17 03 00                  branch_when_zero                   ; 0003 (022c)
022b:  2F                        return_zero                        ;
; single/master.c:208
022c:  3C 01                     transfer_local                     ; LV1
022e:  04 E6 00                  F_EFUN1                            ; EFUN: read_file(230)
0231:  0E 21                     short_string                       ; "%d"
0233:  72 01                     sscanf                             ; 1
0235:  40 02                     local_lvalue                       ; LV2
0237:  61                        (void)assign                       ;
0238:  01                        pop                                ;
; single/master.c:211
0239:  3C 02                     transfer_local                     ; LV2
023b:  2E                        return                             ;
023c:  2F                        return_zero                        ;
; single/master.c:212

;; Function: void destruct_environment_of(object)
023d:  3F 00                     local                              ; LV0
023f:  04 11 01                  F_EFUN1                            ; EFUN: interactive(273)
0242:  18 03 00                  branch_when_non_zero               ; 0003 (0246)
0245:  2F                        return_zero                        ;
; single/master.c:220
0246:  02 02 C0 22               F_PUSH                             ; push local 0, string 34
024a:  05 C2 00                  F_EFUN2                            ; EFUN: tell_object(194)
; single/master.c:223
024d:  3C 00                     transfer_local                     ; LV0
024f:  02 02 23 13               F_PUSH                             ; push string 35, string 19
0253:  07 87 00 03               F_EFUNV                            ; EFUN: call_other(135)
0257:  01                        pop                                ;
0258:  2F                        return_zero                        ;
; single/master.c:224

;; Function: string make_path_absolute(string)
0259:  0F                        const0                             ;
025a:  04 8E 00                  F_EFUN1                            ; EFUN: this_player(142)
025d:  0E 24                     short_string                       ; "query_cwd"
025f:  07 87 00 02               F_EFUNV                            ; EFUN: call_other(135)
0263:  3C 00                     transfer_local                     ; LV0
0265:  71 18 00 02               simul_efun                         ; "resolve_path" args: 2
0269:  62 00                     (void)assign_local                 ; LV0
; single/master.c:231
026b:  3C 00                     transfer_local                     ; LV0
026d:  2E                        return                             ;
; single/master.c:232

;; Function: string get_root_uid()
026e:  0E 25                     short_string                       ; "Root"
0270:  2E                        return                             ;
; single/master.c:232

;; Function: string get_bb_uid()
0271:  0E 26                     short_string                       ; "Backbone"
0273:  2E                        return                             ;
; single/master.c:232

;; Function: string creator_file(string)
0274:  02 02 27 28               F_PUSH                             ; push string 39, string 40
0278:  3C 00                     transfer_local                     ; LV0
027a:  07 87 00 03               F_EFUNV                            ; EFUN: call_other(135)
027e:  2E                        return                             ;
; single/master.c:247

;; Function: string domain_file(string)
027f:  02 02 27 29               F_PUSH                             ; push string 39, string 41
0283:  3C 00                     transfer_local                     ; LV0
0285:  07 87 00 03               F_EFUNV                            ; EFUN: call_other(135)
0289:  2E                        return                             ;
; single/master.c:252

;; Function: string author_file(string)
028a:  02 02 27 2A               F_PUSH                             ; push string 39, string 42
028e:  3C 00                     transfer_local                     ; LV0
0290:  07 87 00 03               F_EFUNV                            ; EFUN: call_other(135)
0294:  2E                        return                             ;
; single/master.c:257

;; Function: string privs_file(string)
0295:  3C 00                     transfer_local                     ; LV0
0297:  2E                        return                             ;
; single/master.c:261

;; Function: void error_handler(mapping,int)
0298:  02 02 2B C0               F_PUSH                             ; push string 43, local 0
029c:  47                        index                              ;
029d:  0E 2C                     short_string                       ; "/single/tests/efuns/call_out."
029f:  30                        ==                                 ;
02a0:  26 0A 00                  &&                                 ; 000a (02ab)
; single/master.c:269
02a3:  02 02 2D C0               F_PUSH                             ; push string 45, local 0
02a7:  47                        index                              ;
02a8:  0E 2E                     short_string                       ; "*Too long evaluation. Executi"
02aa:  30                        ==                                 ;
02ab:  17 03 00                  branch_when_zero                   ; 0003 (02af)
02ae:  2F                        return_zero                        ;
; single/master.c:270
02af:  10                        const1                             ;
02b0:  04 8E 00                  F_EFUN1                            ; EFUN: this_player(142)
02b3:  25 06 00                  ||                                 ; 0006 (02ba)
02b6:  0F                        const0                             ;
02b7:  04 8E 00                  F_EFUN1                            ; EFUN: this_player(142)
02ba:  62 02                     (void)assign_local                 ; LV2
; single/master.c:275
02bc:  3F 01                     local                              ; LV1
02be:  17 09 00                  branch_when_zero                   ; 0009 (02c8)
02c1:  0E 2F                     short_string                       ; "*Error caught\n"
02c3:  62 03                     (void)assign_local                 ; LV3
02c5:  19 06 00                  branch                             ; 0006 (02cc)
; single/master.c:277
02c8:  0E 03                     short_string                       ; ""
02ca:  62 03                     (void)assign_local                 ; LV3
; single/master.c:278
02cc:  02 03 30 2D C0            F_PUSH                             ; push string 48, string 45, local 0
; single/master.c:285
02d1:  47                        index                              ;
02d2:  02 02 31 C0               F_PUSH                             ; push string 49, local 0
02d6:  47                        index                              ;
02d7:  25 04 00                  ||                                 ; 0004 (02dc)
02da:  0E 32                     short_string                       ; "No current object"
; single/master.c:280
02dc:  02 02 2B C0               F_PUSH                             ; push string 43, local 0
02e0:  47                        index                              ;
02e1:  25 04 00                  ||                                 ; 0004 (02e6)
02e4:  0E 33                     short_string                       ; "No current program"
; single/master.c:281
02e6:  02 02 34 C0               F_PUSH                             ; push string 52, local 0
02ea:  47                        index                              ;
02eb:  02 02 35 C0               F_PUSH                             ; push string 53, local 0
02ef:  47                        index                              ;
; single/master.c:282
02f0:  0E 36                     short_string                       ; "trace"
02f2:  3C 00                     transfer_local                     ; LV0
02f4:  47                        index                              ;
; single/master.c:283
02f5:  0F                        const0                             ;
02f6:  70 05 01 24 00            (::)                               ; <functional, 1 args>: Code size: 36,
02fb:  02 03 37 35 C0            F_PUSH                             ; push string 55, string 53, local 0
; single/master.c:285
0300:  47                        index                              ;
0301:  02 02 34 C0               F_PUSH                             ; push string 52, local 0
0305:  47                        index                              ;
0306:  02 02 31 C0               F_PUSH                             ; push string 49, local 0
030a:  47                        index                              ;
030b:  25 04 00                  ||                                 ; 0004 (0310)
030e:  0E 38                     short_string                       ; "No object"
0310:  02 02 2B C0               F_PUSH                             ; push string 43, local 0
0314:  47                        index                              ;
0315:  25 04 00                  ||                                 ; 0004 (031a)
0318:  0E 39                     short_string                       ; "No program"
031a:  07 0E 01 05               F_EFUNV                            ; EFUN: sprintf(270)
031e:  2E                        return                             ;
031f:  07 2E 01 02               F_EFUNV                            ; EFUN: map(302)
0323:  0E 11                     short_string                       ; "\n"
0325:  05 97 00                  F_EFUN2                            ; EFUN: implode(151)
0328:  07 0E 01 07               F_EFUNV                            ; EFUN: sprintf(270)
; single/master.c:284
032c:  40 03                     local_lvalue                       ; LV3
032e:  60                        (void)+=                           ;
; single/master.c:279
032f:  3F 03                     local                              ; LV3
0331:  44 02 00                  global_lvalue                      ; last_error(2)
0334:  61                        (void)assign                       ;
; single/master.c:286
0335:  02 03 3A C3 40            F_PUSH                             ; push string 58, local 3, number 0
033a:  06 E1 00                  F_EFUN3                            ; EFUN: write_file(225)
033d:  01                        pop                                ;
; single/master.c:287
033e:  3C 01                     transfer_local                     ; LV1
0340:  6D                        !                                  ;
0341:  26 04 00                  &&                                 ; 0004 (0346)
0344:  3F 02                     local                              ; LV2
0346:  17 09 00                  branch_when_zero                   ; 0009 (0350)
0349:  02 02 C2 C3               F_PUSH                             ; push local 2, local 3
034d:  05 C2 00                  F_EFUN2                            ; EFUN: tell_object(194)
0350:  2F                        return_zero                        ;
; single/master.c:288

;; Function: mixed get_include_path(string)
0351:  3C 00                     transfer_local                     ; LV0
0353: switch
      type: 3f table: 0380-03d0 deflt: 0377
035b:  02 02 3D 3E               F_PUSH                             ; push string 61, string 62
035f:  11 02 00                  aggregate                          ; 2
0362:  2E                        return                             ;
; single/master.c:297
0363:  0E 41                     short_string                       ; "/include/m_gip2"
0365:  0E 3E                     short_string                       ; "/include"
0367:  11 02 00                  aggregate                          ; 2
036a:  2E                        return                             ;
; single/master.c:300
036b:  02 02 3E 3D               F_PUSH                             ; push string 62, string 61
036f:  11 02 00                  aggregate                          ; 2
0372:  2E                        return                             ;
; single/master.c:303
0373:  11 00 00                  aggregate                          ; 0
0376:  2E                        return                             ;
; single/master.c:306
0377:  0E 46                     short_string                       ; ":DEFAULT:"
0379:  11 01 00                  aggregate                          ; 1
037c:  2E                        return                             ;
037d:  19 52 00                  branch                             ; 0052 (03d0)
; single/master.c:308
      switch table (for 0353)
        "/clone/mgip1"  035b
        "/clone/mgip1.c"        035b
        "/clone/mgip2"  0363
        "/clone/mgip2.c"        0363
        "/clone/mgip3"  036b
        "/clone/mgip3.c"        036b
        "/clone/mgip4"  0373
        "/clone/mgip4.c"        0373
; single/master.c:293
03d0:  2F                        return_zero                        ;
; single/master.c:308

;; Function: int valid_database(object,string,mixed *)
03d1:  0E 47                     short_string                       ; "MASTER valid_database called:"
03d3:  0E 48                     short_string                       ; "ob:%O action:%O info:%O"
03d5:  3C 00                     transfer_local                     ; LV0
03d7:  3C 01                     transfer_local                     ; LV1
03d9:  3C 02                     transfer_local                     ; LV2
03db:  07 0E 01 04               F_EFUNV                            ; EFUN: sprintf(270)
03df:  63                        +                                  ;
03e0:  0E 11                     short_string                       ; "\n"
03e2:  63                        +                                  ;
03e3:  04 C1 00                  F_EFUN1                            ; EFUN: write(193)
03e6:  10                        const1                             ;
03e7:  2E                        return                             ;
; single/master.c:313

;; Function: string object_name(object)
03e8:  3C 00                     transfer_local                     ; LV0
03ea:  0E 49                     short_string                       ; "name"
03ec:  07 87 00 02               F_EFUNV                            ; EFUN: call_other(135)
03f0:  2E                        return                             ;
; single/master.c:320

;; Function: void #global_init#()
03f1:  2D 00 0C 00 00            call_inherited                     ;         inherit/master/valid.c::#global_init#    12
03f6:  01                        pop                                ;
; single/master.c:6
03f7:  0F                        const0                             ;
03f8:  44 01 00                  global_lvalue                      ; has_error(1)
03fb:  61                        (void)assign                       ;
; single/master.c:8
03fc:  0E 03                     short_string                       ; ""
03fe:  44 02 00                  global_lvalue                      ; last_error(2)
0401:  61                        (void)assign                       ;
0402:  2F                        return_zero                        ;
; single/master.c:9

;;;  *** Line Number Info ***

absolute line -> (file, line) table:
0 lines from 1 [single/master.c]
11 lines from 2 [include/globals.h]
27 lines from 3 [include/tests.h]
25 lines from 2 [include/globals.h]
3 lines from 1 [single/master.c]
11 lines from 2 [include/globals.h]
27 lines from 3 [include/tests.h]
25 lines from 2 [include/globals.h]
319 lines from 1 [single/master.c]

address -> absolute line table:
0000-0006: 138
0007-000b: 143
000c-0011: 144
0012-0020: 145
0021-0027: 146
0028-0029: 148
002a-0033: 144
0034-0037: 149
0038-003e: 153
003f-0049: 154
004a-004d: 156
004e-0057: 161
0058-0066: 163
0067-006b: 164
006c-0070: 165
0071-0078: 166
0079-0087: 170
0088-008c: 171
008d-0091: 172
0092-0096: 173
0097-009d: 175
009e-00c2: 178
00c3-00ce: 181
00cf-00d4: 186
00d5-00e2: 194
00e3-00e7: 196
00e8-00ec: 197
00ed-00f1: 198
00f2-00f7: 199
00f8-00fb: 201
00fc-0106: 211
0107-010d: 212
010e-0117: 213
0118-0120: 224
0121-012b: 225
012c-0132: 250
0133-0137: 251
0138-013b: 252
013c-0144: 254
0145-014a: 255
014b-0156: 256
0157-015e: 257
015f-0160: 259
0161-016a: 255
016b-016e: 260
016f-0176: 269
0177-0179: 270
017a-0187: 279
0188-018c: 282
018d-0197: 283
0198-01a6: 284
01a7-01ab: 285
01ac-01bf: 286
01c0-01c7: 288
01c8-01de: 289
01df-01e8: 298
01e9-01f1: 309
01f2-01ff: 313
0200-0205: 317
0206-0212: 318
0213-0220: 330
0221-022b: 334
022c-0238: 337
0239-023c: 338
023d-0245: 346
0246-024c: 349
024d-0258: 350
0259-026a: 357
026b-0273: 358
0274-027e: 373
027f-0289: 378
028a-0294: 383
0295-0297: 387
0298-02a2: 395
02a3-02ae: 396
02af-02bb: 401
02bc-02c7: 403
02c8-02cb: 404
02cc-02cd: 411
02ce-02db: 406
02dc-02e5: 407
02e6-02ef: 408
02f0-02f4: 409
02f5-02fc: 411
02fd-030f: 410
0310-032b: 410
032c-032e: 405
032f-0334: 412
0335-033d: 413
033e-0350: 414
0351-035a: 419
035b-0362: 423
0363-036a: 426
036b-0372: 429
0373-0376: 432
0377-03d0: 434
03d1-03e7: 439
03e8-03f0: 446
03f1-03f6: 132
03f7-03fb: 134
03fc-0402: 135
Trace duration: 16888.575000 us, dumping 71 events to trace_lpcc.json in separate thread.
[thread 140189376706112d]: Dump trace successfully to file trace_lpcc.json, cost 1 ms.
```
