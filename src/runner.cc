/*
 * lpc.cc
 *
 *  A standalone runner for LPC programs
 *
 *  Created on: Nov 14, 2014
 *      Author: sunyc
 */

#include "base/std.h"

#include "lpc/compiler/compiler.h"

#include <iostream>
#include <fstream>

int compile(const char* name) {
  auto f = open(name, O_RDONLY);

  char name1[400];
  strcpy(name1, name);
  auto prog = compile_file(f, name1);

  return 0;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " lpc.c" << std::endl;
  }

  return compile(argv[1]);
}
