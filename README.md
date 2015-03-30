# FastLED-Serial-Music-Arduino


Update: You can use the included binaries for the plugin, I've edited the code to allow for it to detect the arduino unless you have multiple ports besides COM1 -- in which case I also included an INI file that you can use to override the port name:

```
overrideCom=COM5
```


Uses serial data input by a Winamp plugin from music for awesome visualization effects for your LED strips. Uses FastLED.h 



*Adjust the settings in MusicInputProgram.ino to fit your arduino/LED strip setup and compile/upload it to your arduino

*Enter the correct com port for your arduino and compile, add the SVIS.dll file to your winamp plugins directory

*Set your default recording device to "Sterio Mix"

*Open winamp and select the visualization associated with SVIS.dll, then enter CTRL+L on your keyboard, and enter 
```
linein://
```
*Now the winamp visualizer will send serial data for any sounds played by the computer through the speakers.

If properly configured you should see the colors and brightness of your LEDs respond to the sound output :)


I'd like to program in an interface to select the port in config, but it has caused me problems as C++ is not my strongest language when I don't understand the full context of the project. When I have time I will read through the winamp plugin guides and perfect this as my own version, for now, the modified version of the plugin will do
