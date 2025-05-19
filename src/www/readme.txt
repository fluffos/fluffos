更新xterm.js为5.3.0版

增加自定义MXP插件xterm-addon-mxp-links.js支持MXP标签

lpc发送mxp标签到客户端增加 对话框DAIL 图片IMG 确认框YRN三种扩展
MXPMENU发送菜单增加对话框支持扩展，如果命令以'#DAIL '开头则先选后弹出对话框


可将以下代码加入到simul_efun或者定义为宏用于快速生成标签

#define MXPO "\x1B[1z"

string  MXPSEND(string a,string b){
  return sprintf(MXPO"<send href='%s' SEND='LEFT'>%s</send>",a,b);
}

string  MXPSENDH(string a,string b,string c) {
  return sprintf(MXPO"<send href='%s' hint='%s'>%s</send>",a,c,b);
}

string  MXPMENU(string a,string b,string c)  {
  return sprintf(MXPO"<send href='%s' SEND='LEFT' hint='%s'>%s</send>",a,b,c);
}

string  MXPDIAL(string a,string b,string c)  {
  return sprintf(MXPO"<send href='%s' SEND='DIAL' hint='%s'>%s</send>",a,b,c);
}

string  MXPIMG(string a,string b,string c)  {
  return sprintf(MXPO"<send href='%s' SEND='IMG' hint='%s'>%s</send>",a,b,c);
}

string  MXPYRN(string a)  {
  return sprintf(MXPO"<send href='YORN' SEND='YRN' hint='%s'>%s</send>",a,a);
}