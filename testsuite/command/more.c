// more.c


//	File	:  /include/ansi.h
//	Creator	:  Gothic@TMI-2
//
//	The standard set of ANSI codes for mudlib use.

#ifndef ANSI_H
#define ANSI_H

#define ESC "" /* "\033" | "\e" */

#define CSI ESC + "["        /* Control Sequence Introducer */
#define SGR(x) CSI + x + "m" /* Set Graphics Rendition */

/* Foreground Colors */

#define BLK ESC + "[30m" /* Black    */
#define RED ESC + "[31m" /* Red      */
#define GRN ESC + "[32m" /* Green    */
#define YEL ESC + "[33m" /* Yellow   */
#define BLU ESC + "[34m" /* Blue     */
#define MAG ESC + "[35m" /* Magenta  */
#define CYN ESC + "[36m" /* Cyan     */
#define WHT ESC + "[37m" /* White    */

/* Hi Intensity Foreground Colors */

#define HIR ESC + "[1;31m" /* Red      */
#define HIG ESC + "[1;32m" /* Green    */
#define HIY ESC + "[1;33m" /* Yellow   */
#define HIB ESC + "[1;34m" /* Blue     */
#define HIM ESC + "[1;35m" /* Magenta  */
#define HIC ESC + "[1;36m" /* Cyan     */
#define HIW ESC + "[1;37m" /* White    */

/* Background Colors */

#define BBLK ESC + "[40m" /* Black    */
#define BRED ESC + "[41m" /* Red      */
#define BGRN ESC + "[42m" /* Green    */
#define BYEL ESC + "[43m" /* Yellow   */
#define BBLU ESC + "[44m" /* Blue     */
#define BMAG ESC + "[45m" /* Magenta  */
#define BCYN ESC + "[46m" /* Cyan     */
#define BWHT ESC + "[47m" /* White    */

/* High Intensity Background Colors */

#define HBRED ESC + "[41;1m" /* Red      */
#define HBGRN ESC + "[42;1m" /* Green    */
#define HBYEL ESC + "[43;1m" /* Yellow   */
#define HBBLU ESC + "[44;1m" /* Blue     */
#define HBMAG ESC + "[45;1m" /* Magenta  */
#define HBCYN ESC + "[46;1m" /* Cyan     */
#define HBWHT ESC + "[47;1m" /* White    */

#define NOR ESC + "[2;37;0m" /* Puts everything back to normal */

/* Additional ansi Esc codes added to ansi.h by Gothic  april 23,1993 */
/* Note, these are Esc codes for VT100 terminals, and emmulators */
/*           and they may not all work within the mud            */

#define BOLD ESC + "[1m"     /* Turn on bold mode */
#define EM ESC + "[3m"       /* Initialize italic mode */
#define U ESC + "[4m"        /* Initialize underscore mode */
#define BLINK ESC + "[5m"    /* Initialize blink mode */
#define REV ESC + "[7m"      /* Turns reverse video mode on */
#define HIREV ESC + "[1;7m"  /* Hi intensity reverse video  */
#define CLR ESC + "[2J"      /* Clear the screen  */
#define HOME ESC + "[H"      /* Send cursor to home position */
#define REF CLR + HOME       /* Clear screen and home cursor */
#define REVINDEX ESC + "M"   /* Scroll screen in opposite direction */
#define BIGTOP ESC + "#3"    /* Dbl height characters, top half */
#define BIGBOT ESC + "#4"    /* Dbl height characters, bottem half */
#define SINGW ESC + "#5"     /* Normal, single-width characters */
#define DBL ESC + "#6"       /* Creates double-width characters */
#define REST ESC + "[u"      /* Restore cursor to saved position */
#define SAVEC ESC + "[s"     /* Save cursor position */
#define FRTOP ESC + "[2;25r" /* Freeze top line */
#define FRBOT ESC + "[1;24r" /* Freeze bottom line */
#define UNFR ESC + "[r"      /* Unfreeze top and bottom lines */

#define BEEP "" /* "\07" | "\a" */

#endif

#define LINES_PER_PAGE          30

void more(string cmd, string *text, int line)
{
  int i;
  string show;

  show = ESC + "[256D" + ESC + "[K";
  show = ESC + "[1A" + ESC "[256D" + ESC + "[K";

  switch (cmd)
  {
    default:
      i = line + LINES_PER_PAGE;
      if (i >= sizeof(text)) i = sizeof(text) - 1;
      show += implode(text[line..i], "\n") + "\n";
      if (i == sizeof(text) - 1)
      {
        write(show);
        return;
      }
      line = i + 1;
      break;

    case "b":
      line -= LINES_PER_PAGE * 2;
      if (line <= 0)
      {
        line = 0;
        show += WHT "-------- 文件的顶部 --------\n" NOR;
      }

      i = line + LINES_PER_PAGE;
      if (i >= sizeof(text)) i = sizeof(text) - 1;
      show += implode(text[line..i], "\n") + "\n";
      line = i + 1;
      break;

    case "q":
      write(show);
      return;
  }
  show += sprintf(NOR WHT "== 未完继续 " HIY "%d%%" NOR
  WHT " == (ENTER 继续下一页，q 离开，b 前一页)\n" NOR,
      line * 100 / sizeof(text));
  write(show);
  input_to("more", text, line);
}

#define MAX_STRING_SIZE                 2727

void s_write(string msg)
{
  int n, nd;
  int len;

  len = strlen(msg);
  if (len > MAX_STRING_SIZE)
  {
    // the string too long ?
    n = 0;
    while (n < len)
    {
      // show section of the string
      nd = n + MAX_STRING_SIZE;
      if (nd >= len) nd = len - 1;
      write(msg[n..nd]);
      n = nd + 1;
    }
  } else
    write(msg);
}

void start_more(string msg)
{
  int len;

  if (! stringp(msg) || (len = strlen(msg)) < 1)
    // 没有内容
    return;

  write("\n");
  more("", explode(msg, "\n"), 0);
}

void more_file(string cmd, string file, int line, int total)
{
  int i;
  string show;
  string content;
  string *text;
  int goto_line;
  int page;
  int not_show;

  show = ESC + "[256D" + ESC + "[K";
  show = ESC + "[1A" + ESC "[256D" + ESC + "[K";

  page = LINES_PER_PAGE;
  goto_line = line;

  if (! cmd) cmd = "";
  if (sscanf(cmd, "%d,%d", goto_line, page) == 2)
  {
    if (page < goto_line)
    {
      i = goto_line;
      goto_line = page;
      page = i;
    }
    page = page - goto_line + 1;
  } else
  if (sscanf(cmd, "n%d", page))
    ;
  else
  if (sscanf(cmd, "%d", goto_line))
    ; else
  {
    switch (cmd)
    {
      default:
        cmd = "";
        break;

      case "b":
        goto_line = line - LINES_PER_PAGE * 2;
        if (goto_line > 1)
          break;
        // else continue to run selection "t"
      case "t":
        goto_line = 1;
        break;

      case "q":
        write(show);
        return;
    }
  }

  if (page > 301)
  {
    show += "连续显示的行数必须小于等于300。\n";
    not_show = 1;
  } else
  {
    not_show = 0;

    if (! goto_line) goto_line = 1;
    if (! page) page = 1;

    if (goto_line < 0)
    {
      // The goto line < 0, mean look bottom
      goto_line += total;
      if (goto_line  < 1)
        goto_line = 1;
    }

    if (page < 0)
    {
      // The page size < 0, mean look previous page
      goto_line += page;
      if (goto_line < 1)
      {
        page -= goto_line - 1;
        goto_line = 1;
      }
      page = -page;
    }
  }

  line = goto_line;

  if (! not_show)
  {
    if (line == 1)
      show += WHT "-------- 文件的顶部 --------\n" NOR;
    else
    if (cmd != "")
      show += sprintf(HIW "-------- 从第 %d 行开始 %d 行 --------\n" NOR,
        goto_line, page);

    content = read_file(file, line, page);
    if (! content)
    {
      i = 0;
    } else
    {
      text = explode(replace_string(content, "\n", " \n") + " ", "\n");
      if (page > sizeof(text))
        page = sizeof(text);
      for (i = 0; i < page; i++)
        text[i] = sprintf(NOR "%-8d%s", i + line, text[i]);
      content = implode(text[0..i - 1], "\n") + "\n";

      show += content;
    }
  }

  if (not_show || i > 1)
  {
    show += NOR WHT "- 未完(" HIY + total +
        NOR WHT ") - (回车继续，"
    HIY "q" NOR WHT " 离开，"
    HIY "b" NOR WHT " 前一页，"
    HIC "<num>" NOR WHT " 到第 "
    HIC "n" NOR WHT " 行，"
    HIY "n" HIC "<num>" NOR WHT
    "显示接下 " HIC "n" NOR WHT " 行)" NOR;
    s_write(show);
    input_to("more_file", file, line + page, total);
  } else
  {
    show += WHT "阅读完毕。\n" NOR;
    s_write(show);
  }
}

int file_lines(string file)
{
  return efun::file_length(file);
}

void start_more_file(string fn)
{
  if (file_size(fn) < 0)
  {
    write("没有 " + fn + " 这个文件可供阅读。\n");
    return;
  }
  write("\n");
  more_file("t", fn, 1, file_lines(fn));
}

int main(string arg)
{
  string file;
  object ob;

  if (!arg)
    return notify_fail("指令格式 : more <档名>|<物件名> \n");

  file = arg;
  if (file_size(file) < 0)
  {
    ob = present(arg, this_player());
    if (!ob)
      ob = present(arg, environment(this_player()));
    if (!ob)
      return notify_fail("没有这个档案。\n");
    file = base_name(ob) + ".c";
  }

  start_more_file(file);
  return 1;
}
