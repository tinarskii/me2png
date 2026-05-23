#define SAMPLE_RATE (44100)
#define FRAMES_PER_BUFFER (1024)
#include <string>
#include <vector>
#include <portaudio.h>

std::vector<std::string> GetMicrophoneList();
PaError StartAudioCapture(PaStream **stream, const std::string &micname);
void StopAudioCapture(PaStream *stream);
int CheckAndResetThreshold(double *out_rms);
float GetRMS();
float GetThreshold();
void SetThreshold(float threshold);