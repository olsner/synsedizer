#!/bin/sed -runf
# Monophonic synsedizer, prototyping :)

# First line is always a command, and we're never sleeping.
1 bhandle_command

Q
:read_command
z
n
:handle_command
#i DEBUG: read_command
/^s (.*)$/ {
#i DEBUG: sleep
    # Add sleep. We know there is no sleep yet, as we only read commands when
    # not sleeping.
    s//S\1/g
    H
    bmain
}
/^g (.*)$/ {
#i DEBUG: note on
    s//W,1,0,\1,\1/g
    H
    bread_command
}
/^G$/ {
#i DEBUG: note off
    g
    s/^W,.*$//mg
    h
    bread_command
}
i\
DEBUG: unrecognized command:
p
bread_command

:main
#i DEBUG: main
g

# If sleep state is not present, read one line of input and process.
/^S/M! {
    bread_command
}

# Append initial accumulator value (empty string) at end, then shift through
# state list from the start until we reach the accumulator again.
s/\n?$/\nA/

:state_loop
#i\
#DEBUG: state loop: state {
#p
#i\
#} state

# Strip leading newlines
s/^\n+//g

# Sleep state (finished):
/^S\n/ {
    s/^S\n//g
}
# Sleep state:
/^S/ {
    s/^(Sm*)m$/\1cccccccccc/m
    s/^(S[mc]*)c$/\1xxxxxxxxxx/m
    s/^(S[mcx]*)x$/\1iiiiiiiiii/m
    s/^(S[mcxi]*)i$/\1/m
    # Still sleeping to do, shuffle and continue
    s/^(S[mcxi]+)\n(.*)$/\2\n\1/
}
#* Noise state:
#    read line from /dev/urandom, see if the line length is odd or even
#    add 1 to output accumulator if odd
#* Wave state: W<id>,<curval>,<nextval>,initWL,counterWL
/^W/ {
    # if value=1: add value into output
    /W([^,]*),1,/ s/^A/Ai/mg
    # Subtract WL counter
    s/\`(W.*,m*)m$/\1cccccccccc/m
    s/\`(W.*,[mc]*)c$/\1xxxxxxxxxx/m
    s/\`(W.*,[mcx]*)x$/\1iiiiiiiiii/m
    s/\`(W.*,[mcxi]*)i$/\1/m
    # if WL counter 0: reset WL counter to initial WL, flip value
    /\`W.*,$/M {
        s/^(W[^,]*,[01],[01])(,[mcxi]+),$/\1\2\2/m
        s/^(W[^,]*,)([01],)([01],)/\1\3\2/m
    }

    s/^([^\n]+)\n(.*)$/\2\n\1/g
}
# Accumulator (finished processing)
/^A/ {
    # 6 values: 00, 2a, 55, 80, aa, d4, ff
    h
    # Strip away the non-accumulator part
    s/\n.*$//
    # ASCII Debug output:
#    s/^/Output: /p
    # Binary 8-bit unsigned output
    s/^Aiiiiii+$/\xff/
    s/^Aiiiii$/\xd4/
    s/^Aiiii$/\xaa/
    s/^Aiii$/\x80/
    s/^Aii$/\x55/
    s/^Ai$/\x2a/
    s/^A$/\x00/
    P

    # TODO the start of the main-loop can remove the previous accumulator and
    # add new one at the end...
    g
    s/^Ai*\n//
    h

    bmain
}

bstate_loop

bmain
