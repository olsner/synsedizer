# The Synsedizer

Synsedizer takes as input a simple command language describing a musical
sequence and outputs 8-bit samples on stdout. Due to limitations of the
implementation, the output is intermingled with newline characters that need
to be removed before sending the data to your sound card, e.g. by piping the
output through `tr -d '\n'`.


Synsedizer is untested with other sed versions and may require GNU sed.


## Usage


    cat twinkle.txt | ./mono.sed | tr -d '\n' | aplay -f U8

    cat polymusic.txt | ./poly.sed | tr -d '\n' | aplay -f U8


## Command language

Numbers in the command language are entered in a simplified Roman numeral
format, limited to the letter m, c, x and i. Digits are written in strictly
decreasing value (big endian), so 9 is written using 9 i's, not ix.

Common commands:

* `s N`: Sleeps for N cycles before reading the next command from the input.
  For example, `s mmmmmmmm` in a 8kHz output will sleep for 1 second. The sleep
  command is used to determine note and rest lengths by spacing out the note
  on and off commands.


### Command language (Monophonic)

* `g N`: "note on" command, enables output of the note and sets its wavelength
  to N cycles. If the note is already sounding it is first stopped. The note
  will continue sounding until stopped with G. Use the `s` command to control
  note duration.
* `G`: "note off" command, stops sounding the note (if any).


### Command language (Polyphonic)

* `[abcde] N`: "note on" command for one of the five allowed notes a through e.
  The wavelength argument is the same as in the monophonic synsedizer.
* `[ABCDE]`: "note off" command for the corresponding note


## License

Synsedizer is available under the MIT license.
