#include <cstdlib>
#include <iostream>

#include <portaudio.h>

static void checkErr(PaError err)
{
	if (err != paNoError) {
		std::cout << "PortAudio error: " << Pa_GetErrorText(err) << std::endl ;
		exit(EXIT_FAILURE) ;
	}
}

int main()
{
	PaError err ;
	err = Pa_Initialize() ;
	checkErr(err) ;

	int numDevices = Pa_GetDeviceCount() ;
	std::cout << "Number of Devices: " << numDevices << std::endl ;

	err = Pa_Terminate() ;
	checkErr(err) ;	

	return EXIT_SUCCESS ;
}
