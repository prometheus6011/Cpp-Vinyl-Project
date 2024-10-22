#include <alsa/asoundlib.h>
#include <SDL2/SDL.h>
#include <libusb-1.0/libusb.h>
#include <iostream>
#include <vector>

const int WINDOW_WIDTH = 800 ;
const int WINDOW_HEIGHT = 400 ;

void draw_waveform(SDL_Renderer* renderer, std::vector<short> & buffer)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255) ;
	SDL_RenderClear(renderer) ;

	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255) ;
	int middle = WINDOW_HEIGHT / 2 ;
	for (size_t i = 0; i < buffer.size() - 1; ++i) {
		int x1 = (i * WINDOW_WIDTH) / buffer.size() ;
		int y1 = middle - (buffer[i] * middle) / 32768 ;
		int x2 = ((i + 1) * WINDOW_WIDTH) / buffer.size() ;
		int y2 = middle - (buffer[i + 1] * middle) / 32768 ;

		SDL_RenderDrawLine(renderer, x1, y1, x2, y2) ;
	}

	SDL_RenderPresent(renderer) ;
}

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

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl ;
		return EXIT_FAILURE ;
	}

	SDL_Window* window = SDL_CreateWindow("Mic Input Visualization", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN) ;

	if (window == nullptr) {
		std::cerr << "Window could not be opened! SDL Error: " << SDL_GetError() << std::endl ;
		return EXIT_FAILURE ;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED) ;
	if (renderer == nullptr) {
		std::cerr << "Renderer could not be created. SDL Error: " << SDL_GetError() << std::endl ;
		return EXIT_FAILURE ;
	}

	std::vector<short> waveform_buffer(1024) ;
	bool quit = false ;
	SDL_Event e ;

	while (!quit) {

		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = true ;
			} else if (e.type == SDL_KEYDOWN) {
				if (e.key.keysym.sym = SDLK_ESCAPE) {
					quit = true ;
				}
			}

		}

		frames = snd_pcm_readi(handle, buffer, 1024) ;
		if (frames < 0) {
			std::cerr << "Error reading PCM device: " << snd_strerror(frames) << std::endl ;
			break ;
		}

		waveform_buffer.assign(buffer, buffer + frames) ;

		draw_waveform(renderer, waveform_buffer) ;

		SDL_Delay(10) ;
	}

	snd_pcm_close(handle) ;
	SDL_DestroyRenderer(renderer) ;
	SDL_DestroyWindow(window) ;
	SDL_Quit() ;

	return EXIT_SUCCESS ;
}
