#include "headers/Microphone.hpp"
#include <atomic>
#include <math.h>
#include <portaudio.h>
#include <raylib.h>
#include <vector>

std::atomic<int> threshold_exceeded{0};
std::atomic<double> shared_rms{0.0};
std::atomic<double> rms_threshold{500.0};

std::vector<std::string> GetMicrophoneList() {
  int numDevices = Pa_GetDeviceCount();
  std::vector<std::string> deviceNames;
  for (int i = 0; i < numDevices; i++) {
    const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(i);

    if (deviceInfo->maxInputChannels > 0) {
      deviceNames.push_back(deviceInfo->name);
    }
  }
  return deviceNames;
}

static int paCallback(const void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo *timeInfo,
                      PaStreamCallbackFlags statusFlags, void *userData) {
  const short *buffer = (const short *)inputBuffer;
  if (buffer == NULL)
    return paContinue;

  double sum = 0.0;
  for (unsigned long i = 0; i < framesPerBuffer; i++) {
    sum += buffer[i] * buffer[i];
  }
  double rms = sqrt(sum / framesPerBuffer);
  std::atomic_store(&shared_rms, rms);
  double threshold = std::atomic_load(&rms_threshold);
  std::atomic_store(&threshold_exceeded, rms > threshold ? 1 : 0);
  return paContinue;
}

int GetDeviceIdByName(const std::string &targetMicName) {
  int numDevices = Pa_GetDeviceCount();
  if (numDevices < 0) {
    TraceLog(LOG_ERROR, "Error getting audio devices: %s\n",
             Pa_GetErrorText(numDevices));
    return paNoDevice;
  }

  for (int i = 0; i < numDevices; i++) {
    const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(i);
    if (deviceInfo->maxInputChannels > 0 && deviceInfo->name != nullptr) {
      std::string currentDeviceName(deviceInfo->name);

      if (currentDeviceName == targetMicName) {
        return i;
      }
    }
  }
  return paNoDevice;
}

PaError StartAudioCapture(PaStream **stream, const std::string &micname) {
  PaError initErr = Pa_Initialize();
  if (initErr != paNoError) {
    TraceLog(LOG_ERROR, "PortAudio init failed: %s\n",
             Pa_GetErrorText(initErr));
    return initErr;
  }

  PaStreamParameters inputParameters;
  int foundId = GetDeviceIdByName(micname);

  if (foundId != paNoDevice) {
    inputParameters.device = foundId;
  } else {
    inputParameters.device = Pa_GetDefaultInputDevice();
  }

  if (inputParameters.device == paNoDevice) {
    TraceLog(LOG_ERROR, "No audio input device found.\n");
    return paDeviceUnavailable;
  }

  const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(inputParameters.device);
  if (deviceInfo == nullptr) {
    TraceLog(LOG_ERROR, "Failed to get device info for input device.\n");
    return paInvalidDevice;
  }

  inputParameters.channelCount = 1;
  inputParameters.sampleFormat = paInt16;
  inputParameters.suggestedLatency =
      deviceInfo->defaultLowInputLatency;
  inputParameters.hostApiSpecificStreamInfo = NULL;

  double sampleRate = deviceInfo->defaultSampleRate;
  PaError formatErr =
      Pa_IsFormatSupported(&inputParameters, nullptr, sampleRate);
  if (formatErr != paNoError) {
    TraceLog(LOG_WARNING,
             "Default sample rate %.0f unsupported, trying %d: %s\n",
             sampleRate, SAMPLE_RATE, Pa_GetErrorText(formatErr));
    sampleRate = SAMPLE_RATE;
    formatErr = Pa_IsFormatSupported(&inputParameters, nullptr, sampleRate);
    if (formatErr != paNoError) {
      TraceLog(LOG_ERROR, "Input format not supported: %s\n",
               Pa_GetErrorText(formatErr));
      return formatErr;
    }
  }

  std::atomic_store(&shared_rms, 0.0);
  std::atomic_store(&threshold_exceeded, 0);

  PaError err =
      Pa_OpenStream(stream, &inputParameters, NULL, sampleRate,
                    FRAMES_PER_BUFFER, paNoFlag, paCallback, NULL);
  if (err != paNoError) {
    TraceLog(LOG_ERROR, "Failed to open audio stream: %s\n",
             Pa_GetErrorText(err));
    return err;
  }

  err = Pa_StartStream(*stream);
  if (err != paNoError) {
    TraceLog(LOG_ERROR, "Failed to start audio stream: %s\n",
             Pa_GetErrorText(err));
    Pa_CloseStream(*stream);
    *stream = nullptr;
    return err;
  }
  return err;
}

void StopAudioCapture(PaStream *stream) {
  if (stream == nullptr) {
    return;
  }
  Pa_StopStream(stream);
  Pa_CloseStream(stream);
  Pa_Terminate();
}

int CheckAndResetThreshold(double *out_rms) {
  if (std::atomic_load(&threshold_exceeded) == 1) {
    *out_rms = std::atomic_load(&shared_rms);
    std::atomic_store(&threshold_exceeded, 0);
    return 1;
  }
  return 0;
}

float GetRMS() { return std::atomic_load(&shared_rms); }

float GetThreshold() { return std::atomic_load(&rms_threshold); }

void SetThreshold(float threshold) {
  if (threshold < 0.0f) {
    threshold = 0.0f;
  }
  std::atomic_store(&rms_threshold, threshold);
}