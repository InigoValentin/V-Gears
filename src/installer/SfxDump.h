#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Converts FFVII audio data into individual WAV files.
 *
 * @param fmt_path Path to the audio format table.
 * @param data_path Path to the audio data archive.
 * @param target_dir Directory where WAV files are written.
 * @return 0 on success, nonzero if conversion fails.
 */
int SfxDumpConvert(const char* fmt_path, const char* data_path, const char* target_dir);

#ifdef __cplusplus
}
#endif