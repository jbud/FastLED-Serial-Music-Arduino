# FastLED-Serial-Music-Arduino
Uses serial data input by a Winamp plugin from music for awesome visualization effects for your LED strips. Uses FastLED.h 


I don't have the rights upload the winamp plugin, however it can be found at http://www.macetech.com/wa502_sdk.zip
You will need Visual Studio express 2008 to compile the plugin, unfortunately the com port is hardcoded in this source
I will contact the developer and see if I can post my modified version of the code that allows for comport selection

*Adjust the settings in MusicInputProgram.ino to fit your arduino/LED strip setup and compile/upload it to your arduino

*Search for the com port in the SVIS.CPP file, you should find:
```cpp
  LPTSTR lpszPortName = TEXT("COM5");
```
*Enter the correct com port for your arduino and compile, add the SVIS.dll file to your winamp plugins directory

*Set your default recording device to "Sterio Mix"

*Open winamp and select the visualization associated with SVIS.dll, then enter CTRL+L on your keyboard, and enter 
```
linein://
```
*Now the winamp visualizer will send serial data for any sounds played by the computer through the speakers.

If properly configured you should see the colors and brightness of your LEDs respond to the sound output :)


So I've decided to post the code modifications I've made to the winamp plugin:
```cpp
#include <string>
using namespace std;
using namespace System;
using namespace System::IO::Ports;
using namespace System::ComponentModel;

std::string makeStd(String ^in);
std::string getComPort();

// Required function converts String^ to std::string
std::string makeStd(String ^in){ 
	array<Byte> ^chars = System::Text::Encoding::ASCII->GetBytes(in);
	pin_ptr<Byte> charsPointer = &(chars[0]);
	char *nativeCharsPointer = reinterpret_cast<char *>(static_cast<unsigned char *>(charsPointer));
	std::string native(nativeCharsPointer, chars->Length);
	return nativeCharsPointer;
}
std::string getComPort(){
		array<String^>^ serialPorts = nullptr;
		try {
			serialPorts = SerialPort::GetPortNames();
		}
		catch (Win32Exception^ ex)
		{
			//Do nothing...
		}
		std::string port = makeStd(serialPorts[1]); // Selects the second indexed port automatically (In my case COM3 or COM5, my arduino doesn't make up it's mind, but it is the only serial device except for the motherboard header at COM1 and GetPortNames() will order the indexes alphabetically.)
		return port;
}
```



After inserting this code replace this:
```cpp
LPTSTR lpszPortName = TEXT("COM5");
```
with:
```cpp
std::string port = getComPort();
LPTSTR lpszPortName =  const_cast<char *>(port.c_str());
```



I also replaced config() to show a simple messagebox with the port indexed at 1:
```cpp
void config(struct winampVisModule *this_mod)
{
	std::string port = getComPort();
	lpszPortName =  const_cast<char *>(port.c_str());
	MessageBox(this_mod->hwndParent,port.c_str(),"Port",MB_OK);
}
```


I'd like to program in an interface to select the port in config, but it has caused me problems as C++ is not my strongest language when I don't understand the full context of the project. When I have time I will read through the winamp plugin guides and perfect this as my own version, for now, the modified version of the plugin will do
