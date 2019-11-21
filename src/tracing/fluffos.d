provider fluffos {
  probe lpc__entry(string, string, string);
  probe lpc__return(string, string, string);
  probe lpc__compile_prolog(string, string, string);
  probe lpc__compile_epilog(string, string, string);
  probe error(string);
};

