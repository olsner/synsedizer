# The Synsedizer

Synsedizer takes as input a simple command language describing a musical
sequence and outputs a synsedized `.au` stream on stdout.

Synsedizer probably requires GNU sed as it uses some GNU extensions.


## Usage


Two monophonic example inputs are provided, Twinkle Twinkle Little Star and a
short sequence of rising notes:

    ./synsedizer samples/twinkle.txt | aplay
    ./synsedizer samples/scale.txt | aplay


To demonstrate polyphony, there's also a version of Bad Apple:

    ./synsedizer samples/bad-apple.txt | aplay


Check the `samples/` directory for additional sample inputs.

Using too much polyphony or a too high sample rate may make synsedizer slower
than real time, but you can also pipe the output to a file ahead of time and
play it with `aplay` (or your favorite media player) afterwards.

For example, on a less reasonable platform where you need to launch sed
explicitly because the hashbang line doesn't work, you might do something like:

    sed -rnf ./synsedizer samples/bad-apple.txt > bad-apple.au
    start bad-apple.au


## Command language

Most commands take a cycle count in number of samples, corresponding to some
frequency (or time delay) depending on the sample rate of the output as
specified by the `r` command at the start of the file.


* `r R`: emit a `.au` header for a sample rate of R (decimal, Hz).

  Recognized sample rates are 8000, 16000 and 44100Hz.

  It's strongly recommended that this be the first non-comment/blank line of
  the file, although synsedizer can also be used to output raw samples without
  a header. In this case, you may need to manually specify the sample format as
  signed 16-bit big-endian in your player.

* `s N`: Sleeps for N cycles while outputting synsedized samples before reading
  the next command from the input.

  For example, `s 8000` with a 8kHz sample rate will sleep for 1 second.
  The sleep command determines the note and rest lengths by spacing out the
  note on and off commands.

* `[abcde] N`: "note on" command for one of the five allowed notes a through e.
  Enables output of the note and sets its wavelength to N cycles. The note will
  continue sounding until stopped with the corresponding note off command.
  Use the `s` command to control note duration.

* `[ABCDE]`: "note off" command for the corresponding note

* `P[tag]`: print the internal synsedizer state on stderr for debugging along
  with the original command (including the optional tag).


## License

Synsedizer is available under the MIT license.
