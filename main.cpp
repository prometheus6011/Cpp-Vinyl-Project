#include <cstdlib>
#include <iostream>
#include <cstring>
#include <portaudio.h>

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 512

static void checkErr(PaError err)
{
	if (err != paNoError) {
		std::cout << "PortAudio error: " << Pa_GetErrorText(err) << std::endl ;
		exit(EXIT_FAILURE) ;
	}
}

static inline float max(float a, float b)
{
	return a > b ? a : b ;
}

static inline float abs(float a)
{
	return a > 0 ? a : -a ;
}

static int paTestCallback(const void* input_buffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags status_flags, void* user_date)
{
	float* in = (float*)input_buffer ;
	(void)outputBuffer ;

	int dispSize = 100 ;
	std::cout << "\r" ;

	float vol_1 = 0 ;

	for (unsigned long i = 0; i < framesPerBuffer * 2; i += 2) {
		vol_1 = max(vol_1, std::abs(in[i])) ;
	}

	for (int i = 0; i < dispSize; i++) {
		float barProportion = i / (float)dispSize ;
		if (barProportion <= vol_1) {
			printf("%s", 0xE2) ;
		} else {
			printf(" ") ;
		}
	}

	fflush(stdout) ;
	
	return 0 ;
}

int main()
{
	PaError err ;
	err = Pa_Initialize() ;
	checkErr(err) ;

	int numDevices = Pa_GetDeviceCount() ;
	std::cout << "Number of Devices: " << numDevices << std::endl ;
	if (numDevices < 0) {
		std::cout << "Error getting device count." << std::endl ;
		exit(EXIT_FAILURE) ;
	} else if (numDevices == 0) {
		std::cout << "There are no available audio devices on this machine." << std::endl ;
		exit(EXIT_SUCCESS) ;
	}

	const PaDeviceInfo* deviceInfo ;
	for (int i = 0; i < numDevices; i++) {
		deviceInfo = Pa_GetDeviceInfo(i) ;
		std::cout << "Device: " << i << std::endl ;
		std::cout << "\tname: " << deviceInfo->name << std::endl ;
		std::cout << "\tmaxInputChannel: " << deviceInfo->maxInputChannels << std::endl ;
		std::cout << "\tmaxOutputChannels: " << deviceInfo->maxOutputChannels << std::endl ;
		std::cout << "\tdefaultSampleRate: " << deviceInfo->defaultSampleRate << std::endl ;
	}
	
	int output_device = 0 ;
	int input_device_1 = 1 ;
	int input_device_2 = 2 ;

	PaStreamParameters inputParameters ;
	PaStreamParameters outputParameters ;

	memset(&inputParameters, 0, sizeof(inputParameters)) ;
	inputParameters.channelCount = 1 ;
	inputParameters.device = input_device_1 ;
	inputParameters.hostApiSpecificStreamInfo = NULL ;
	inputParameters.sampleFormat = paFloat32 ;
	inputParameters.suggestedLatency = Pa_GetDeviceInfo(input_device_1)->defaultLowInputLatency ;

	memset(&outputParameters, 0, sizeof(outputParameters)) ;
	outputParameters.channelCount = 1 ;
	outputParameters.device = output_device ;
	outputParameters.hostApiSpecificStreamInfo = NULL ;
	outputParameters.sampleFormat = paFloat32 ;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo(output_device)->defaultLowInputLatency ;

	PaStream* stream ;
	err = Pa_OpenStream(
			&stream,
			&inputParameters,
			&outputParameters,
			SAMPLE_RATE,
			FRAMES_PER_BUFFER,
			paNoFlag,
			paTestCallback,
			NULL
		) ;
	checkErr(err) ;


	err = Pa_StartStream(stream) ;
	checkErr(err) ;


	Pa_Sleep(10 * 1000) ;

	err = Pa_StopStream(stream) ;
	checkErr(err) ;

	err = Pa_CloseStream(stream) ;
	checkErr(err) ;



	err = Pa_Terminate() ;
	checkErr(err) ;	

	return EXIT_SUCCESS ;
}
