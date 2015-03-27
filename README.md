# FastLED-Serial-Music-Arduino
Uses serial data input by a Winamp plugin from music for awesome visualization effects for your LED strips. Uses FastLED.h 


I don't have the rights upload the winamp plugin, however it can be found at http://www.macetech.com/wa502_sdk.zip
You will need Visual Studio express 2008 to compile the plugin, unfortunately the com port is hardcoded in this source
I will contact the developer and see if I can post my modified version of the code that allows for comport selection

*Adjust the settings in MusicInputProgram.ino to fit your arduino/LED strip setup and compile/upload it to your arduino

*Search for the com port in the SVIS.CPP file, you should find:
  LPTSTR lpszPortName = TEXT("COM5");

*Enter the correct com port for your arduino and compile, add the SVIS.dll file to your winamp plugins directory

*Set your default recording device to "Sterio Mix"

*Open winamp and select the visualization associated with SVIS.dll, then enter CTRL+L on your keyboard, and enter linein://

*Now the winamp visualizer will send serial data for any sounds played by the computer through the speakers.

If properly configured you should see the colors and brightness of your LEDs respond to the sound output :)
