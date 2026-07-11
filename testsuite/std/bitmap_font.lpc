/*****************************************************************************
Copyright: 2020, Mud.Ren
File name: bitmap_font.c
Author: xuefeng@mud.ren
Version: v1.1
Date: 2020-02-20
Description:
    display ASCII and chinese with bitmap font
*****************************************************************************/
// 字体文件(请根据需要修改路径)
#define HZK "/std/fonts/HZK"
#define ASC "/std/fonts/ASC"
// 默认前景字符
#define DEFAULT_FILL "8"
// 默认背景字符
#define DEFAULT_BG "-"
// 默认前景颜色
#define DEFAULT_FCOLOR ""
// 默认背景颜色
#define DEFAULT_BGCOLOR ""
// 默认点阵大小
#define AUTO_SIZE 12
/**
 * 字符、字号、内容填充、背景填充、前景色、背景色
 */
varargs string bitmap_font(string str, int size, string fill, string bg, string fcolor, string bgcolor)
{
    int offset, fontsize, scale;
    int *mask = ({0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1});
    buffer char, bstr = string_encode(str, "GBK");
    string file, *out;
    // 当前可用字库16x12、16x14、16x16
    if (member_array(size, ({12, 14, 16})) < 0)
        size = AUTO_SIZE;

    // 中文字体占用的字节数(支持32x32等字库)
    if (size < 16)
        fontsize = size * 2;
    else
        fontsize = size * size / 8;

    out = allocate(size, "");

    if (!sizeof(fill)) fill = DEFAULT_FILL;
    if (!sizeof(bg)) bg = DEFAULT_BG;
    if (strwidth(fill) != strwidth(bg))
    {
        fill = DEFAULT_FILL;
        bg = DEFAULT_BG;
    }

    if (!fcolor) fcolor = DEFAULT_FCOLOR;
    if (!bgcolor) bgcolor = DEFAULT_BGCOLOR;
    // 16x16中文字库随机字体
    if (size == 16)
        file = HZK + size + element_of(({"C", "F", "H", "K", "L", "S", "V", "X", "Y"}));
    else
        file = HZK + size;

    for (int k = 0; k < sizeof(bstr); k++)
    {
        if (mask[0] & bstr[k])
        {
            // 区码：汉字的第一个字节-0xA0
            // 位码：汉字的第二个字节-0xA0
            // offset = (94 * (区码 - 1) + (位码 - 1)) * fontsize;
            offset = fontsize * ((bstr[k] - 0xA1) * 94 + bstr[k+1] - 0xA1);
            char = read_buffer(file, offset, fontsize);
            scale = fontsize / size;
            k++;
        }
        else
        {
            // 英文每个字符占1字节
            offset = bstr[k] * size;
            char = read_buffer(ASC + size, offset, size);
            scale = 1;
        }

        if (!sizeof(char)) return "Can't read bytes from character lib.\n";

        //填充字符
        for (int i = 0; i < sizeof(char); i++)
        {
            for (int j = 0; j < 8; j++)
            {
                if (mask[j] & char[i])
                    out[i / scale] += fcolor + fill;
                else
                    out[i / scale] += bgcolor + bg;
            }
        }
        // 清除多余的颜色代码
        for (int i = 0; i < sizeof(out); i++)
        {
            out[i] = replace_string(out[i], fill + fcolor + fill, fill + fill);
            out[i] = replace_string(out[i], bg + bgcolor + bg, bg + bg);
        }
    }

    return implode(out, "\n");
}
