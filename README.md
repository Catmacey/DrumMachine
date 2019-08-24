16 step sample based DrumMachine
================================

![DrumMachine](https://catmacey.files.wordpress.com/2013/08/img_36251.jpg?w=869)

[Demo of features (Youtube)](https://www.youtube.com/watch?v=TWu0VHxkMRI)

This is a 16 track Sample based Drum Machine built on a 32bit PIC (PIC32mx150f128b).

The project hardware is relatively simple in that it limits itself to using the PIC's internal Flash for sample storage and the built in OC PWM for output. 

The Drum Machine has 16 tracks (instruments) of 16 steps and can save the song to one of 16 slots on a uSD card.

Features : Updated 24 / 8 / 2019

* 16 different instruments stored in Flash (Length dependant on sample quality/size).
* 16 step patterns with 16 patterns per song.
* 16 songs loaded and saved to FAT formatted uSD card.
* 8 sample polyphony, mixed in software.
* 10bit, 16khz instruments sampled from classic drum machines. (stored as 8bit log similar to uLaw)
* Samples can be "tuned" to playback faster/slower, louder quieter and can also have the in/out points set.
* "Jam Mode" to add some variety to your tracks by fading in/out each track as it plays.
* Swing
* Basic Reverb mode.
* Analog mono output filtered from an 10bit PWM signal.
* 16 buttons for beat input plus 4 more and 4-way joystick for interface.
* 20 leds driven by a Maxim 6957 LED driver to provide feedback.
* [Nokia 1202 B/W LCD](https://github.com/Catmacey/Nokia1202LCD-breakout) mounted on a breakout board displays user interface.
* Songs are stored as plain text in JSON format to allow some human readability.


[More information is available on my blog.](https://catmacey.wordpress.com/tag/drum-machine/)


Copyright (C) 2016 Matt Casey : catmacey.com
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
	
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

