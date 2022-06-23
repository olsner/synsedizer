#!/usr/bin/env python3
# Script to convert the original roman numerals format to the current decimal
# number format for synsedizer. Ought to have been written in sed of course :)

import sys

def unroman(s):
    res = 0
    for dig,val in [('m', 1000),('c', 100), ('x',10), ('i',1)]:
        res += val * s.count(dig)
    return res


for line in sys.stdin:
    oline = line
    if "#" in line:
        line = line.split("#", 1)[0]
    line = line.strip()
    if not line:
        print(oline, end="")
        continue

    # Line with command...
    words = line.split()
    if len(words) < 2 or words[0] == "r":
        print(oline, end="")
        continue

    # Line with command with args, convert argument
    print(words[0], unroman(words[1]))
