// Things to do

//DONE Sort out LED flashing when playing so that LCD doesn't flash (Or at least that it flashes on beat 1)

//DONE Add option to save/load to choose the file number (1 of 16). Indicate choice using step LEDs. Double press for confirmation.

//TODO Rewrite pattern grid display code to scale vertically using gxf library. Also modularise it to allow redraw of individual tracks or maybe columns...

//TODO Add "dirty rectangles" to gfx lib to allow LCD to redraw only what is needed? Maybe not worth it.

//DONE Create a better display for menu mode.

//DONE Display value of pattern length when changing it in menu mode.

//DONE Add brightness controls to menu mode.

//TODO Add contrast controls to menu mode.

//TODO Store the device config some where?  Maybe as a JSON file in the SDCard. Could save default song,

//DONE Add song wide track volume control in menu mode.

//TODO Add a per-pattern volume control to each track to allow more variety to songs.

//TODO Add free-play mode where you can play each sample using the step buttons.  Maybe allow song to be running in the background?

//TODO Add pattern select play mode where step buttons select patterns rather than program the song. This would allow you to set up a bunch of nice patterns then whilst playing the song you could jump to a different pattern.

//TODO Add random pattern mode?  Maybe use the repeat button, each press switches mode. Normal, Repeat, Random

//TODO Add mechanism to copy/move patterns within the song.

//TODO Add mechanism to allow copy/move single track to entire song or selected patterns. This would allow you to setup your baseline then copy it to the entire song without overwriting other tracks.

//TODO Add option in menu to clear a track, pattern or entire song

//TODO Always save/Load entire song. Allow song length to set the loop point. Then in freeplay mode the patterns after the repeat can be used as fills, they play then return to the loop

//DONE Implement pitch shifting using simple linear interpolation (See. http://yehar.com/blog/wp-content/uploads/2009/08/deip.pdf)

//TODO Have a look at allowing samples to be played backwards by switching in/out points



//DONE Implement some sort of "Swing" function. Delays every second beat. See http://www.attackmagazine.com/technique/passing-notes/daw-drum-machine-swing/2/

//DONE Save swing value into song json

//TODO Display swing a little better, either as percentage (based at 50%) or maybe witha graphic.

//TODO Add more buttons using spare channels on row 4

//TODO add controls over reverb. Maybe allow include/exclude channels from reverb

//TODO Add a bitcrush effect? Allow each channel to be added/removed

//TODO Add a simple auto control over volume per track to allow some variation in sounds. Could be emphasis, random...?

//TODO Add a pattern edit mode to display all patterns and allow copy/move/clear


// Notes

// Add mode to set envelope on a sample.  Not sure if this would really bring anything?

// Add potentiometer to spare GPIO (Ex LCD D/C) (call is POTIN) could use this for making UI easier to use. Is more natural to set things like volume and speed with a pot rather than a joystick.

// Might be able to use the row ouputs in combination with POTIN to allow for 3 different pots... Do I need three? What would I do with them?  I'd need to diode mix them into the single input... I think that'd work...