#include <mono/metadata/appdomain.h>
#include <mono/mini/jit.h>

#include <string.h>

#include "../include/FAudio.h"
#include "../include/F3DAudio.h"
#include "../include/FACT.h"
#include "../include/FACT3D.h"
#include "../include/FAPO.h"
#include "../include/FAPOBase.h"
#include "../include/FAPOFX.h"
#include "../include/FAudioFX.h"

#include "../src/FAudio_internal.h"

/* stb_vorbis */

#define malloc FAudio_malloc
#define realloc FAudio_realloc
#define free FAudio_free
#ifdef memset /* Thanks, Apple! */
#undef memset
#endif
#define memset FAudio_memset
#ifdef memcpy /* Thanks, Apple! */
#undef memcpy
#endif
#define memcpy FAudio_memcpy
#define memcmp FAudio_memcmp

#define pow FAudio_pow
#define log(x) FAudio_log(x)
#define sin(x) FAudio_sin(x)
#define cos(x) FAudio_cos(x)
#define floor FAudio_floor
#define abs(x) FAudio_abs(x)
#define ldexp(v, e) FAudio_ldexp((v), (e))
#define exp(x) FAudio_exp(x)

#define qsort FAudio_qsort

#define assert FAudio_assert

#define FILE FAudioIOStream
#ifdef SEEK_SET
#undef SEEK_SET
#endif
#ifdef SEEK_END
#undef SEEK_END
#endif
#ifdef EOF
#undef EOF
#endif
#define SEEK_SET FAUDIO_SEEK_SET
#define SEEK_END FAUDIO_SEEK_END
#define EOF FAUDIO_EOF
#define fopen(path, mode) FAudio_fopen(path)
#define fopen_s(io, path, mode) (!(*io = FAudio_fopen(path)))
#define fclose(io) FAudio_close(io)
#define fread(dst, size, count, io) io->read(io->data, dst, size, count)
#define fseek(io, offset, whence) io->seek(io->data, offset, whence)
#define ftell(io) io->seek(io->data, 0, FAUDIO_SEEK_CUR)

/* stb_vorbis */

#define STB_VORBIS_NO_PUSHDATA_API 1
#define STB_VORBIS_NO_INTEGER_CONVERSION 1

typedef struct
{
   char *alloc_buffer;
   int   alloc_buffer_length_in_bytes;
} stb_vorbis_alloc;

typedef struct stb_vorbis stb_vorbis;

typedef struct
{
   unsigned int sample_rate;
   int channels;

   unsigned int setup_memory_required;
   unsigned int setup_temp_memory_required;
   unsigned int temp_memory_required;

   int max_frame_size;
} stb_vorbis_info;

typedef struct
{
   char *vendor;

   int comment_list_length;
   char **comment_list;
} stb_vorbis_comment;

FAUDIOAPI stb_vorbis_info stb_vorbis_get_info(stb_vorbis *f);
FAUDIOAPI stb_vorbis_comment stb_vorbis_get_comment(stb_vorbis *f);
FAUDIOAPI int stb_vorbis_get_error(stb_vorbis *f);
FAUDIOAPI void stb_vorbis_close(stb_vorbis *f);
FAUDIOAPI int stb_vorbis_get_sample_offset(stb_vorbis *f);
FAUDIOAPI unsigned int stb_vorbis_get_file_offset(stb_vorbis *f);
FAUDIOAPI stb_vorbis * stb_vorbis_open_memory(const unsigned char *data, int len,
                                  int *error, const stb_vorbis_alloc *alloc_buffer);
FAUDIOAPI stb_vorbis * stb_vorbis_open_filename(const char *filename,
                                  int *error, const stb_vorbis_alloc *alloc_buffer);
FAUDIOAPI stb_vorbis * stb_vorbis_open_file(FILE *f, int close_handle_on_close,
                                  int *error, const stb_vorbis_alloc *alloc_buffer);
FAUDIOAPI stb_vorbis * stb_vorbis_open_file_section(FILE *f, int close_handle_on_close, int *error, const stb_vorbis_alloc *alloc_buffer, unsigned int len);
FAUDIOAPI int stb_vorbis_seek_frame(stb_vorbis *f, unsigned int sample_number);
FAUDIOAPI int stb_vorbis_seek(stb_vorbis *f, unsigned int sample_number);
FAUDIOAPI int stb_vorbis_seek_start(stb_vorbis *f);
FAUDIOAPI unsigned int stb_vorbis_stream_length_in_samples(stb_vorbis *f);
FAUDIOAPI float        stb_vorbis_stream_length_in_seconds(stb_vorbis *f);
FAUDIOAPI int stb_vorbis_get_frame_float(stb_vorbis *f, int *channels, float ***output);
FAUDIOAPI int stb_vorbis_get_samples_float_interleaved(stb_vorbis *f, int channels, float *buffer, int num_floats);
FAUDIOAPI int stb_vorbis_get_samples_float(stb_vorbis *f, int channels, float **buffer, int num_samples);

/* qoa_decoder */

#define QOA_MIN_FILESIZE 16
#define QOA_MAX_CHANNELS 8

#define QOA_SLICE_LEN 20
#define QOA_SLICES_PER_FRAME 256
#define QOA_FRAME_LEN (QOA_SLICES_PER_FRAME * QOA_SLICE_LEN)
#define QOA_LMS_LEN 4
#define QOA_MAGIC 0x716f6166 /* 'qoaf' */

#define QOA_FRAME_SIZE(channels, slices) \
	(8 + QOA_LMS_LEN * 4 * channels + 8 * slices * channels)

typedef struct {
	int history[QOA_LMS_LEN];
	int weights[QOA_LMS_LEN];
} qoa_lms_t;

typedef struct {
	unsigned int channels;
	unsigned int samplerate;
	unsigned int samples;
	qoa_lms_t lms[QOA_MAX_CHANNELS];
	#ifdef QOA_RECORD_TOTAL_ERROR
		double error;
	#endif
} qoa_desc;

typedef struct {
	unsigned char *bytes;
	unsigned int size;
	unsigned int frame_index;
	unsigned int frame_size;
	unsigned short samples_per_channel_per_frame;
	int free_on_close;
	qoa_desc qoa;
} qoa_data;

typedef unsigned long long qoa_uint64_t;

typedef struct qoa qoa;

FAUDIOAPI qoa *qoa_open_from_memory(unsigned char *bytes, unsigned int size, int free_on_close);
FAUDIOAPI qoa *qoa_open_from_filename(const char *filename);
FAUDIOAPI void qoa_attributes(qoa *qoa, unsigned int *channels, unsigned int *samplerate, unsigned int *samples_per_channel_per_frame, unsigned int *total_samples_per_channel);
FAUDIOAPI unsigned int qoa_decode_next_frame(qoa *qoa, short *sample_data); /* decode the next frame into a preallocated buffer */
FAUDIOAPI void qoa_seek_frame(qoa *qoa, int frame_index);
FAUDIOAPI void qoa_decode_entire(qoa *qoa, short *sample_data); /* fill a buffer with the entire qoa data decoded */
FAUDIOAPI void qoa_close(qoa *qoa);

/* XNA song functions */
void XNA_SongInit();
void XNA_SongQuit();
float XNA_PlaySong(const char *name);
void XNA_PauseSong();
void XNA_ResumeSong();
void XNA_StopSong();
void XNA_SetSongVolume(float volume);
uint32_t XNA_GetSongEnded();
void XNA_EnableVisualization(uint32_t enable);
uint32_t XNA_VisualizationEnabled();
void XNA_GetSongVisualizationData(float *frequencies, float *samples, uint32_t count);

extern void** mono_aot_module_FAudio_CS_info;

extern void VMLFNAFAudioRegister()
{
	mono_aot_register_module(mono_aot_module_FAudio_CS_info);

    mono_add_internal_call("FAudio::FAudioLinkedVersion", FAudioLinkedVersion);
	mono_add_internal_call("FAudio::FAudioCreate", FAudioCreate);
	mono_add_internal_call("FAudio::FAudio_AddRef", FAudio_AddRef);
	mono_add_internal_call("FAudio::FAudio_Release", FAudio_Release);
	mono_add_internal_call("FAudio::FAudio_GetDeviceCount", FAudio_GetDeviceCount);
	mono_add_internal_call("FAudio::FAudio_GetDeviceDetails", FAudio_GetDeviceDetails);
	mono_add_internal_call("FAudio::FAudio_Initialize", FAudio_Initialize);
	mono_add_internal_call("FAudio::FAudio_RegisterForCallbacks", FAudio_RegisterForCallbacks);
	mono_add_internal_call("FAudio::FAudio_UnregisterForCallbacks", FAudio_UnregisterForCallbacks);
	mono_add_internal_call("FAudio::FAudio_CreateSourceVoice", FAudio_CreateSourceVoice);
	mono_add_internal_call("FAudio::FAudio_CreateSourceVoice", FAudio_CreateSourceVoice);
	mono_add_internal_call("FAudio::FAudio_CreateSubmixVoice", FAudio_CreateSubmixVoice);
	mono_add_internal_call("FAudio::FAudio_CreateMasteringVoice", FAudio_CreateMasteringVoice);
	mono_add_internal_call("FAudio::FAudio_StartEngine", FAudio_StartEngine);
	mono_add_internal_call("FAudio::FAudio_StopEngine", FAudio_StopEngine);
	mono_add_internal_call("FAudio::FAudio_CommitChanges", FAudio_CommitOperationSet);
	mono_add_internal_call("FAudio::FAudio_GetPerformanceData", FAudio_GetPerformanceData);
	mono_add_internal_call("FAudio::FAudio_SetDebugConfiguration", FAudio_SetDebugConfiguration);
	mono_add_internal_call("FAudio::FAudio_GetProcessingQuantum", FAudio_GetProcessingQuantum);
	mono_add_internal_call("FAudio::FAudioVoice_GetVoiceDetails", FAudioVoice_GetVoiceDetails);
	mono_add_internal_call("FAudio::FAudioVoice_SetOutputVoices", FAudioVoice_SetOutputVoices);
	mono_add_internal_call("FAudio::FAudioVoice_SetEffectChain", FAudioVoice_SetEffectChain);
	mono_add_internal_call("FAudio::FAudioVoice_EnableEffect", FAudioVoice_EnableEffect);
	mono_add_internal_call("FAudio::FAudioVoice_DisableEffect", FAudioVoice_DisableEffect);
	mono_add_internal_call("FAudio::FAudioVoice_GetEffectState", FAudioVoice_GetEffectState);
	mono_add_internal_call("FAudio::FAudioVoice_SetEffectParameters", FAudioVoice_SetEffectParameters);
	mono_add_internal_call("FAudio::FAudioVoice_GetEffectParameters", FAudioVoice_GetEffectParameters);
	mono_add_internal_call("FAudio::FAudioVoice_SetFilterParameters", FAudioVoice_SetFilterParameters);
	mono_add_internal_call("FAudio::FAudioVoice_GetFilterParameters", FAudioVoice_GetFilterParameters);
	mono_add_internal_call("FAudio::FAudioVoice_SetOutputFilterParameters", FAudioVoice_SetOutputFilterParameters);
	mono_add_internal_call("FAudio::FAudioVoice_GetOutputFilterParameters", FAudioVoice_GetOutputFilterParameters);
	mono_add_internal_call("FAudio::FAudioVoice_SetVolume", FAudioVoice_SetVolume);
	mono_add_internal_call("FAudio::FAudioVoice_GetVolume", FAudioVoice_GetVolume);
	mono_add_internal_call("FAudio::FAudioVoice_SetChannelVolumes", FAudioVoice_SetChannelVolumes);
	mono_add_internal_call("FAudio::FAudioVoice_GetChannelVolumes", FAudioVoice_GetChannelVolumes);
	mono_add_internal_call("FAudio::FAudioVoice_SetOutputMatrix", FAudioVoice_SetOutputMatrix);
	mono_add_internal_call("FAudio::FAudioVoice_GetOutputMatrix", FAudioVoice_GetOutputMatrix);
	mono_add_internal_call("FAudio::FAudioVoice_DestroyVoice", FAudioVoice_DestroyVoice);
	mono_add_internal_call("FAudio::FAudioVoice_DestroyVoiceSafeEXT", FAudioVoice_DestroyVoiceSafeEXT);
	mono_add_internal_call("FAudio::FAudioSourceVoice_Start", FAudioSourceVoice_Start);
	mono_add_internal_call("FAudio::FAudioSourceVoice_Stop", FAudioSourceVoice_Stop);
	mono_add_internal_call("FAudio::FAudioSourceVoice_SubmitSourceBuffer", FAudioSourceVoice_SubmitSourceBuffer);
	mono_add_internal_call("FAudio::FAudioSourceVoice_SubmitSourceBuffer", FAudioSourceVoice_SubmitSourceBuffer);
	mono_add_internal_call("FAudio::FAudioSourceVoice_FlushSourceBuffers", FAudioSourceVoice_FlushSourceBuffers);
	mono_add_internal_call("FAudio::FAudioSourceVoice_Discontinuity", FAudioSourceVoice_Discontinuity);
	mono_add_internal_call("FAudio::FAudioSourceVoice_ExitLoop", FAudioSourceVoice_ExitLoop);
	mono_add_internal_call("FAudio::FAudioSourceVoice_GetState", FAudioSourceVoice_GetState);
	mono_add_internal_call("FAudio::FAudioSourceVoice_SetFrequencyRatio", FAudioSourceVoice_SetFrequencyRatio);
	mono_add_internal_call("FAudio::FAudioSourceVoice_GetFrequencyRatio", FAudioSourceVoice_GetFrequencyRatio);
	mono_add_internal_call("FAudio::FAudioSourceVoice_SetSourceSampleRate", FAudioSourceVoice_SetSourceSampleRate);
	mono_add_internal_call("FAudio::FAudioCreateReverb", FAudioCreateReverb);
	mono_add_internal_call("FAudio::FAudioCreateReverb9", FAudioCreateReverb9);
	mono_add_internal_call("FAudio::FAPOBase_Release", FAPOBase_Release);
	mono_add_internal_call("FAudio::FACTCreateEngine", FACTCreateEngine);
	mono_add_internal_call("FAudio::FACTAudioEngine_AddRef", FACTAudioEngine_AddRef);
	mono_add_internal_call("FAudio::FACTAudioEngine_Release", FACTAudioEngine_Release);
	mono_add_internal_call("FAudio::FACTAudioEngine_GetRendererCount", FACTAudioEngine_GetRendererCount);
	mono_add_internal_call("FAudio::FACTAudioEngine_GetRendererDetails", FACTAudioEngine_GetRendererDetails);
	mono_add_internal_call("FAudio::FACTAudioEngine_GetFinalMixFormat", FACTAudioEngine_GetFinalMixFormat);
	mono_add_internal_call("FAudio::FACTAudioEngine_Initialize", FACTAudioEngine_Initialize);
	mono_add_internal_call("FAudio::FACTAudioEngine_ShutDown", FACTAudioEngine_ShutDown);
	mono_add_internal_call("FAudio::FACTAudioEngine_DoWork", FACTAudioEngine_DoWork);
	mono_add_internal_call("FAudio::FACTAudioEngine_CreateSoundBank", FACTAudioEngine_CreateSoundBank);
	mono_add_internal_call("FAudio::FACTAudioEngine_CreateInMemoryWaveBank", FACTAudioEngine_CreateInMemoryWaveBank);
	mono_add_internal_call("FAudio::FACTAudioEngine_CreateStreamingWaveBank", FACTAudioEngine_CreateStreamingWaveBank);
	mono_add_internal_call("FAudio::FACTAudioEngine_PrepareWave", FACTAudioEngine_PrepareWave);
	mono_add_internal_call("FAudio::FACTAudioEngine_PrepareInMemoryWave", FACTAudioEngine_PrepareInMemoryWave);
	mono_add_internal_call("FAudio::FACTAudioEngine_PrepareStreamingWave", FACTAudioEngine_PrepareStreamingWave);
	mono_add_internal_call("FAudio::FACTAudioEngine_RegisterNotification", FACTAudioEngine_RegisterNotification);
	mono_add_internal_call("FAudio::FACTAudioEngine_UnRegisterNotification", FACTAudioEngine_UnRegisterNotification);
	mono_add_internal_call("FAudio::FACTAudioEngine_GetCategory", FACTAudioEngine_GetCategory);
	mono_add_internal_call("FAudio::FACTAudioEngine_Stop", FACTAudioEngine_Stop);
	mono_add_internal_call("FAudio::FACTAudioEngine_SetVolume", FACTAudioEngine_SetVolume);
	mono_add_internal_call("FAudio::FACTAudioEngine_Pause", FACTAudioEngine_Pause);
	mono_add_internal_call("FAudio::FACTAudioEngine_GetGlobalVariableIndex", FACTAudioEngine_GetGlobalVariableIndex);
	mono_add_internal_call("FAudio::FACTAudioEngine_SetGlobalVariable", FACTAudioEngine_SetGlobalVariable);
	mono_add_internal_call("FAudio::FACTAudioEngine_GetGlobalVariable", FACTAudioEngine_GetGlobalVariable);
	mono_add_internal_call("FAudio::FACTSoundBank_GetCueIndex", FACTSoundBank_GetCueIndex);
	mono_add_internal_call("FAudio::FACTSoundBank_GetNumCues", FACTSoundBank_GetNumCues);
	mono_add_internal_call("FAudio::FACTSoundBank_GetCueProperties", FACTSoundBank_GetCueProperties);
	mono_add_internal_call("FAudio::FACTSoundBank_Prepare", FACTSoundBank_Prepare);
	mono_add_internal_call("FAudio::FACTSoundBank_Play", FACTSoundBank_Play);
	mono_add_internal_call("FAudio::FACTSoundBank_Play", FACTSoundBank_Play);
	mono_add_internal_call("FAudio::FACTSoundBank_Play3D", FACTSoundBank_Play3D);
	mono_add_internal_call("FAudio::FACTSoundBank_Stop", FACTSoundBank_Stop);
	mono_add_internal_call("FAudio::FACTSoundBank_Destroy", FACTSoundBank_Destroy);
	mono_add_internal_call("FAudio::FACTSoundBank_GetState", FACTSoundBank_GetState);
	mono_add_internal_call("FAudio::FACTWaveBank_Destroy", FACTWaveBank_Destroy);
	mono_add_internal_call("FAudio::FACTWaveBank_GetState", FACTWaveBank_GetState);
	mono_add_internal_call("FAudio::FACTWaveBank_GetNumWaves", FACTWaveBank_GetNumWaves);
	mono_add_internal_call("FAudio::FACTWaveBank_GetWaveIndex", FACTWaveBank_GetWaveIndex);
	mono_add_internal_call("FAudio::FACTWaveBank_GetWaveProperties", FACTWaveBank_GetWaveProperties);
	mono_add_internal_call("FAudio::FACTWaveBank_Prepare", FACTWaveBank_Prepare);
	mono_add_internal_call("FAudio::FACTWaveBank_Play", FACTWaveBank_Play);
	mono_add_internal_call("FAudio::FACTWaveBank_Stop", FACTWaveBank_Stop);
	mono_add_internal_call("FAudio::FACTWave_Destroy", FACTWave_Destroy);
	mono_add_internal_call("FAudio::FACTWave_Play", FACTWave_Play);
	mono_add_internal_call("FAudio::FACTWave_Stop", FACTWave_Stop);
	mono_add_internal_call("FAudio::FACTWave_Pause", FACTWave_Pause);
	mono_add_internal_call("FAudio::FACTWave_GetState", FACTWave_GetState);
	mono_add_internal_call("FAudio::FACTWave_SetPitch", FACTWave_SetPitch);
	mono_add_internal_call("FAudio::FACTWave_SetVolume", FACTWave_SetVolume);
	mono_add_internal_call("FAudio::FACTWave_SetMatrixCoefficients", FACTWave_SetMatrixCoefficients);
	mono_add_internal_call("FAudio::FACTWave_GetProperties", FACTWave_GetProperties);
	mono_add_internal_call("FAudio::FACTCue_Destroy", FACTCue_Destroy);
	mono_add_internal_call("FAudio::FACTCue_Play", FACTCue_Play);
	mono_add_internal_call("FAudio::FACTCue_Stop", FACTCue_Stop);
	mono_add_internal_call("FAudio::FACTCue_GetState", FACTCue_GetState);
	mono_add_internal_call("FAudio::FACTCue_SetMatrixCoefficients", FACTCue_SetMatrixCoefficients);
	mono_add_internal_call("FAudio::FACTCue_GetVariableIndex", FACTCue_GetVariableIndex);
	mono_add_internal_call("FAudio::FACTCue_SetVariable", FACTCue_SetVariable);
	mono_add_internal_call("FAudio::FACTCue_GetVariable", FACTCue_GetVariable);
	mono_add_internal_call("FAudio::FACTCue_Pause", FACTCue_Pause);
	mono_add_internal_call("FAudio::FACTCue_GetProperties", FACTCue_GetProperties);
	mono_add_internal_call("FAudio::FACTCue_SetOutputVoices", FACTCue_SetOutputVoices);
	mono_add_internal_call("FAudio::FACTCue_SetOutputVoiceMatrix", FACTCue_SetOutputVoiceMatrix);
	mono_add_internal_call("FAudio::F3DAudioInitialize", F3DAudioInitialize);
	mono_add_internal_call("FAudio::F3DAudioInitialize8", F3DAudioInitialize8);
	mono_add_internal_call("FAudio::F3DAudioCalculate", F3DAudioCalculate);
	mono_add_internal_call("FAudio::FACT3DInitialize", FACT3DInitialize);
	mono_add_internal_call("FAudio::FACT3DCalculate", FACT3DCalculate);
	mono_add_internal_call("FAudio::FACT3DApply", FACT3DApply);
	mono_add_internal_call("FAudio::XNA_SongInit", XNA_SongInit);
	mono_add_internal_call("FAudio::XNA_SongQuit", XNA_SongQuit);
	mono_add_internal_call("FAudio::XNA_PlaySong", XNA_PlaySong);
	mono_add_internal_call("FAudio::XNA_PauseSong", XNA_PauseSong);
	mono_add_internal_call("FAudio::XNA_ResumeSong", XNA_ResumeSong);
	mono_add_internal_call("FAudio::XNA_StopSong", XNA_StopSong);
	mono_add_internal_call("FAudio::XNA_SetSongVolume", XNA_SetSongVolume);
	mono_add_internal_call("FAudio::XNA_GetSongEnded", XNA_GetSongEnded);
	mono_add_internal_call("FAudio::XNA_EnableVisualization", XNA_EnableVisualization);
	mono_add_internal_call("FAudio::XNA_VisualizationEnabled", XNA_VisualizationEnabled);
	mono_add_internal_call("FAudio::XNA_GetSongVisualizationData", XNA_GetSongVisualizationData);
	mono_add_internal_call("FAudio::FAudio_fopen", FAudio_fopen);
	mono_add_internal_call("FAudio::FAudio_memopen", FAudio_memopen);
	mono_add_internal_call("FAudio::FAudio_memptr", FAudio_memptr);
	mono_add_internal_call("FAudio::FAudio_close", FAudio_close);
	mono_add_internal_call("FAudio::stb_vorbis_get_info", stb_vorbis_get_info);
	mono_add_internal_call("FAudio::stb_vorbis_get_comment", stb_vorbis_get_comment);
	mono_add_internal_call("FAudio::stb_vorbis_get_error", stb_vorbis_get_error);
	mono_add_internal_call("FAudio::stb_vorbis_close", stb_vorbis_close);
	mono_add_internal_call("FAudio::stb_vorbis_get_sample_offset", stb_vorbis_get_sample_offset);
	mono_add_internal_call("FAudio::stb_vorbis_get_file_offset", stb_vorbis_get_file_offset);
	mono_add_internal_call("FAudio::stb_vorbis_open_memory", stb_vorbis_open_memory);
	mono_add_internal_call("FAudio::stb_vorbis_open_filename", stb_vorbis_open_filename);
	mono_add_internal_call("FAudio::stb_vorbis_open_file", stb_vorbis_open_file);
	mono_add_internal_call("FAudio::stb_vorbis_open_file_section", stb_vorbis_open_file_section);
	mono_add_internal_call("FAudio::stb_vorbis_seek_frame", stb_vorbis_seek_frame);
	mono_add_internal_call("FAudio::stb_vorbis_seek", stb_vorbis_seek);
	mono_add_internal_call("FAudio::stb_vorbis_seek_start", stb_vorbis_seek_start);
	mono_add_internal_call("FAudio::stb_vorbis_stream_length_in_samples", stb_vorbis_stream_length_in_samples);
	mono_add_internal_call("FAudio::stb_vorbis_stream_length_in_seconds", stb_vorbis_stream_length_in_seconds);
	mono_add_internal_call("FAudio::stb_vorbis_get_frame_float", stb_vorbis_get_frame_float);
	mono_add_internal_call("FAudio::stb_vorbis_get_frame_float", stb_vorbis_get_frame_float);
	mono_add_internal_call("FAudio::stb_vorbis_get_samples_float_interleaved", stb_vorbis_get_samples_float_interleaved);
	mono_add_internal_call("FAudio::stb_vorbis_get_samples_float_interleaved", stb_vorbis_get_samples_float_interleaved);
	mono_add_internal_call("FAudio::stb_vorbis_get_samples_float", stb_vorbis_get_samples_float);
	mono_add_internal_call("FAudio::qoa_open_from_filename", qoa_open_from_filename);
	mono_add_internal_call("FAudio::qoa_attributes", qoa_attributes);
	mono_add_internal_call("FAudio::qoa_decode_next_frame", qoa_decode_next_frame);
	mono_add_internal_call("FAudio::qoa_seek_frame", qoa_seek_frame);
	mono_add_internal_call("FAudio::qoa_decode_entire", qoa_decode_entire);
	mono_add_internal_call("FAudio::qoa_close", qoa_close);
}