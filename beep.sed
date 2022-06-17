#!/bin/sed -runf

# input: xxxxxxxxxxxxxxxxxx<NL>
# -> repeat 18 x and 18 NLs indefinitely

h
:rep
s/./\x01/g
P
g
s/./\xff/g
P
brep
