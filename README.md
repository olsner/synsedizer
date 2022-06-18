# The Synsedizer

Synsedizer takes as input a simple command language describing a musical
sequence and outputs 8-bit samples on stdout.

Due to limitations of the implementation, the output is intermingled with
newline characters that need to be removed before sending the data to your
sound card, e.g. by piping the output through `tr -d '\n'`.

Synsedizer may require GNU sed as it is untested with other sed versions.


## Usage


Two monophonic example inputs are provided, Twinkle Twinkle Little Star and a
short sequence of rising notes:

    cat twinkle.txt | ./synsedizer | tr -d '\n' | aplay -f U8
    cat monoscale.txt | ./synsedizer | tr -d '\n' | aplay -f U8

Using too much polyphony may make the synth slower than real time, but it's
always possible to pipe the output to a file ahead of time and play it with
`aplay` afterwards.


## Command language

Numbers in the command language are entered in a simplified Roman numeral
format, limited to the letter m, c, x and i. Digits are written in strictly
decreasing value (big endian), so 9 is written using 9 i's, not ix.

Many commands accept a count in cycles, which is related to the sample rate of
the output. To use higher sample rates, adapt the cycle counts in your sequence
accordingly and use the `-r` flag to `aplay` when playing the output.


* `s N`: Sleeps for N cycles while outputting synsedized samples before reading
  the next command from the input.

  For example, `s mmmmmmmm` with a 8kHz sample rate will sleep for 1 second.
  The sleep command determines the note and rest lengths by spacing out the
  note on and off commands.

* `[abcde] N`: "note on" command for one of the five allowed notes a through e.
  Enables output of the note and sets its wavelength to N cycles. The note will
  continue sounding until stopped with the corresponding note off command.
  Use the `s` command to control note duration.

* `[ABCDE]`: "note off" command for the corresponding note


## License

Synsedizer is available under the MIT license.
