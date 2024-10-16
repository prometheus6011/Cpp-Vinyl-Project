#include <cstdlib>
#include <iostream>
#include <cstring>
#include <portaudio.h>

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 512

static void checkErr(PaError err)
{
    if (err != paNoError) {
        std::cout << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        exit(EXIT_FAILURE);
    }
}

static inline float max(float a, float b)
{
    return a > b ? a : b;
}

static inline float abs(float a)
{
    return a > 0 ? a : -a;
}

// This struct holds the input buffers from both devices
struct AudioData {
    const float* left_input;
    const float* right_input;
};

// Audio processing callback
static int paTestCallback(const void* input_buffer, void* output_buffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* time_info,
                          PaStreamCallbackFlags status_flags,
                          void* user_data)
{
    AudioData* audio_data = (AudioData*)user_data;
    const float* left_in = audio_data->left_input;
    const float* right_in = audio_data->right_input;
    float* out = (float*)output_buffer;

    float vol_left = 0;
    float vol_right = 0;

    // Process input and output data
    for (unsigned long i = 0; i < framesPerBuffer; i++) {
        vol_left = max(vol_left, abs(left_in[i]));
        vol_right = max(vol_right, abs(right_in[i]));

        // Interleaving left and right channels for stereo output
        out[2 * i] = left_in[i];  // Left channel
        out[2 * i + 1] = right_in[i];  // Right channel
    }

    // Volume bar display (console visualization)
    int dispSize = 50;
    std::cout << "\rLeft: ";
    for (int i = 0; i < dispSize; i++) {
        if (i < (vol_left * dispSize)) {
            std::cout << "|";
        } else {
            std::cout << " ";
        }
    }

    std::cout << " Right: ";
    for (int i = 0; i < dispSize; i++) {
        if (i < (vol_right * dispSize)) {
            std::cout << "|";
        } else {
            std::cout << " ";
        }
    }
    fflush(stdout);

    return 0;
}

int main()
{
    PaError err;
    err = Pa_Initialize();
    checkErr(err);

    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        std::cout << "Error getting device count." << std::endl;
        exit(EXIT_FAILURE);
    }

    int left_input_device = 1;   // Device 1 for left channel
    int right_input_device = 2;  // Device 2 for right channel
    int output_device = 0;       // Device 0 for stereo output

    PaStreamParameters leftInputParams;
    PaStreamParameters rightInputParams;
    PaStreamParameters outputParams;

    // Configure left input device
    memset(&leftInputParams, 0, sizeof(leftInputParams));
    leftInputParams.device = left_input_device;
    leftInputParams.channelCount = 1;  // Mono input
    leftInputParams.sampleFormat = paFloat32;
    leftInputParams.suggestedLatency = Pa_GetDeviceInfo(left_input_device)->defaultLowInputLatency;
    leftInputParams.hostApiSpecificStreamInfo = NULL;

    // Configure right input device
    memset(&rightInputParams, 0, sizeof(rightInputParams));
    rightInputParams.device = right_input_device;
    rightInputParams.channelCount = 1;  // Mono input
    rightInputParams.sampleFormat = paFloat32;
    rightInputParams.suggestedLatency = Pa_GetDeviceInfo(right_input_device)->defaultLowInputLatency;
    rightInputParams.hostApiSpecificStreamInfo = NULL;

    // Configure output device
    memset(&outputParams, 0, sizeof(outputParams));
    outputParams.device = output_device;
    outputParams.channelCount = 2;  // Stereo output
    outputParams.sampleFormat = paFloat32;
    outputParams.suggestedLatency = Pa_GetDeviceInfo(output_device)->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = NULL;

    // Open input streams for left and right devices
    PaStream* leftStream;
    err = Pa_OpenStream(&leftStream, &leftInputParams, NULL, SAMPLE_RATE, FRAMES_PER_BUFFER, paNoFlag, NULL, NULL);
    checkErr(err);

    PaStream* rightStream;
    err = Pa_OpenStream(&rightStream, &rightInputParams, NULL, SAMPLE_RATE, FRAMES_PER_BUFFER, paNoFlag, NULL, NULL);
    checkErr(err);

    // Open output stream for the stereo device
    PaStream* outputStream;
    AudioData audioData;
    err = Pa_OpenStream(&outputStream, NULL, &outputParams, SAMPLE_RATE, FRAMES_PER_BUFFER, paNoFlag, paTestCallback, &audioData);
    checkErr(err);

    // Start input streams
    err = Pa_StartStream(leftStream);
    checkErr(err);

    err = Pa_StartStream(rightStream);
    checkErr(err);

    // Start output stream
    err = Pa_StartStream(outputStream);
    checkErr(err);

    // Collect input from both devices and process it
    for (int i = 0; i < 1000; ++i) {
        const float* left_in;
        const float* right_in;

        // Get the input buffers
        err = Pa_ReadStream(leftStream, &left_in, FRAMES_PER_BUFFER);
        checkErr(err);
        err = Pa_ReadStream(rightStream, &right_in, FRAMES_PER_BUFFER);
        checkErr(err);

        // Pass the input buffers to the output callback
        audioData.left_input = left_in;
        audioData.right_input = right_in;

        Pa_Sleep(10);  // Wait for a short time to simulate real-time processing
    }

    // Stop and close streams
    err = Pa_StopStream(leftStream);
    checkErr(err);
    err = Pa_StopStream(rightStream);
    checkErr(err);
    err = Pa_StopStream(outputStream);
    checkErr(err);

    err = Pa_CloseStream(leftStream);
    checkErr(err);
    err = Pa_CloseStream(rightStream);
    checkErr(err);
    err = Pa_CloseStream(outputStream);
    checkErr(err);

    err = Pa_Terminate();
    checkErr(err);

    return EXIT_SUCCESS;
}
