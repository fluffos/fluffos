void do_tests() {
  string tmp;

  tmp = "FluffOS is great!! :)";
  ASSERT_EQ("Rmx1ZmZPUyBpcyBncmVhdCEhIDop", base64encode(tmp));
  ASSERT_EQ(tmp, base64decode(base64encode(tmp)));

  tmp = "白日依山尽，黄河入海流。🍕📙🕡🌵🎎📙🚢😮🕡🐗🏦🕤🎎📙🕖📫，欲穷千里目，更上一层楼";
  ASSERT_EQ(
    "55m95pel5L6d5bGx5bC977yM6buE5rKz5YWl5rW35rWB44CC8J+NlfCfk5nwn5Wh8J+MtfCfjo7wn5OZ8J+aovCfmK7wn5Wh8J+Ql/Cfj6bwn5Wk8J+OjvCfk5nwn5WW8J+Tq++8jOassuept+WNg+mHjOebru+8jOabtOS4iuS4gOWxgualvA==",
    base64encode(tmp)
  );
  ASSERT_EQ(tmp, base64decode(base64encode(tmp)));
}
