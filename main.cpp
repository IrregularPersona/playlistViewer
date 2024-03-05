#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }

    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

    (void)pInput;
}

void play_mp3_files_in_folder(const char* folderPath)
{
    ma_result result;
    ma_decoder decoder;
    ma_device_config deviceConfig;
    ma_device device;

    DIR* dir = opendir(folderPath);
    if (dir == NULL) {
        printf("Could not open folder: %s\n", folderPath);
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        const char* extension = strrchr(entry->d_name, '.');
        if (extension != NULL && strcmp(extension, ".mp3") == 0) {
            char filePath[256];
            snprintf(filePath, sizeof(filePath), "%s/%s", folderPath, entry->d_name);

            result = ma_decoder_init_file(filePath, NULL, &decoder);
            if (result != MA_SUCCESS) {
                printf("Could not load file: %s. Error code: %d\n", filePath, result);
                continue;
            }

            deviceConfig = ma_device_config_init(ma_device_type_playback);
            deviceConfig.playback.format   = decoder.outputFormat;
            deviceConfig.playback.channels = decoder.outputChannels;
            deviceConfig.sampleRate        = decoder.outputSampleRate;
            deviceConfig.dataCallback      = data_callback;
            deviceConfig.pUserData         = &decoder;

            if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
                printf("Failed to open playback device for file: %s\n", filePath);
                ma_decoder_uninit(&decoder);
                continue;
            }

            if (ma_device_start(&device) != MA_SUCCESS) {
                printf("Failed to start playback device for file: %s\n", filePath);
                ma_device_uninit(&device);
                ma_decoder_uninit(&decoder);
                continue;
            }

            printf("Now playing: %s. Press Enter to continue...\n", filePath);
            getchar();

            ma_device_uninit(&device);
            ma_decoder_uninit(&decoder);
        }
    }

    closedir(dir);
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        printf("Usage: %s <folder_path>\n", argv[0]);
        return -1;
    }

    play_mp3_files_in_folder(argv[1]);

    return 0;
}
