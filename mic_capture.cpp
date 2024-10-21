#include <alsa/asoundlib.h>
#include <libusb-1.0/libusb.h>
#include <iostream>

int main()
{
	
	snd_pcm_t *handle ;
	int err ;

	if ((err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_CAPTURE, 0)) < 0) {
		std::cerr << "Error opening PCM device: " << snd_strerror(err) << std::endl ;
		return EXIT_FAILURE ;
	}

	int sample_rate = 44100 ;

	snd_pcm_hw_params_t *params ;
	snd_pcm_hw_params_alloca(&params) ;

	snd_pcm_hw_params_any(handle, params) ;
	snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED) ;
	snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE) ;
	snd_pcm_hw_params_set_rate_near(handle, params, reinterpret_cast<unsigned int*>(&sample_rate), 0) ;
	snd_pcm_hw_params_set_channels(handle, params, 1) ;
	
	snd_pcm_hw_params(handle, params) ;

	short buffer[1024] ;
	int frames ;
	while (1) {
		frames = snd_pcm_readi(handle, buffer, 1024) ;
		if (frames < 0) {
			std::cerr << "Error reading PCM device: " << snd_strerror(frames) << std::endl ;
			break ;
		}
	}

	snd_pcm_close(handle) ;

	return EXIT_SUCCESS ;
}
