#include "SfxDump.h"
#include "SfxDumpStructures.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SFX_DUMP_SOUND_COUNT 750

static int read_exact(FILE* file, void* buffer, size_t size){
	return fread(buffer, 1, size, file) == size;
}

static int write_exact(FILE* file, const void* buffer, size_t size){
	return fwrite(buffer, 1, size, file) == size;
}

static int convert_sound(
	FILE* fmt_file,
	FILE* data_file,
	const FfWav_header* header,
	const char* target_dir,
	int sound_index
){
	Fmt_chunk format = {0};
	Loop_chunk loop = {0};
	Riff_header riff = {0};
	Data_chunk* data_chunk = NULL;
	FILE* output_file = NULL;
	char* output_path = NULL;
	int result = 1;
	size_t dir_length = strlen(target_dir);
	int add_separator = dir_length > 0
		&& target_dir[dir_length - 1] != '/'
		&& target_dir[dir_length - 1] != '\\';
	size_t path_capacity;
	uint64_t riff_size;

	if (dir_length > SIZE_MAX - 16 - (size_t)add_separator) {
		fprintf(stderr, "SfxDump: output directory path is too long.\n");
		return 1;
	}
	path_capacity = dir_length + (size_t)add_separator + 16;
	output_path = malloc(path_capacity);
	if (!output_path) {
		fprintf(stderr, "SfxDump: unable to allocate output path.\n");
		return 1;
	}

	if (snprintf(
		output_path, path_capacity, "%s%s%d.wav", target_dir,
		add_separator ? "/" : "", sound_index
	) >= (int)path_capacity){
		fprintf(stderr, "SfxDump: output path is too long.\n");
		goto cleanup;
	}

	memcpy(format.id, "fmt ", sizeof format.id);
	format.size = sizeof format.adpcm;
	if (!read_exact(fmt_file, &format.adpcm, sizeof format.adpcm)){
		fprintf(stderr, "SfxDump: truncated format record for sound %d.\n", sound_index);
		goto cleanup;
	}

	memcpy(loop.id, "fflp", sizeof loop.id);
	loop.size = sizeof loop.start + sizeof loop.end;
	loop.start = header->start;
	loop.end = header->end;

	if ((uint64_t)header->len > (uint64_t)SIZE_MAX - sizeof *data_chunk){
		fprintf(stderr, "SfxDump: sound %d is too large.\n", sound_index);
		goto cleanup;
	}
	data_chunk = malloc(sizeof *data_chunk + (size_t)header->len);
	if (!data_chunk){
		fprintf(stderr, "SfxDump: unable to allocate sound %d.\n", sound_index);
		goto cleanup;
	}
	memcpy(data_chunk->id, "data", sizeof data_chunk->id);
	data_chunk->size = header->len;
	if (!read_exact(data_file, data_chunk->data, header->len)){
		fprintf(stderr, "SfxDump: truncated audio data for sound %d.\n", sound_index);
		goto cleanup;
	}

	memcpy(riff.id, "RIFF", sizeof riff.id);
	memcpy(riff.format, "WAVE", sizeof riff.format);
	riff_size = sizeof riff.format + sizeof format + sizeof *data_chunk + header->len;
	if (header->loop) riff_size += sizeof loop;
	if (riff_size > UINT32_MAX){
		fprintf(stderr, "SfxDump: WAV file for sound %d is too large.\n", sound_index);
		goto cleanup;
	}
	riff.size = (uint32_t)riff_size;

	output_file = fopen(output_path, "wb");
	if (!output_file){
		fprintf(stderr, "SfxDump: unable to create %s.\n", output_path);
		goto cleanup;
	}
	if (!write_exact(output_file, &riff, sizeof riff)
		|| !write_exact(output_file, &format, sizeof format)
		|| !write_exact(output_file, data_chunk, sizeof *data_chunk + header->len)
		|| (header->loop && !write_exact(output_file, &loop, sizeof loop))){
		fprintf(stderr, "SfxDump: failed writing %s.\n", output_path);
		goto cleanup;
	}
	if (fclose(output_file) != 0){
		output_file = NULL;
		fprintf(stderr, "SfxDump: failed closing %s.\n", output_path);
		goto cleanup;
	}
	output_file = NULL;
	result = 0;

cleanup:
	if (output_file) fclose(output_file);
	free(data_chunk);
	free(output_path);
	return result;
}

int SfxDumpConvert(const char* fmt_path, const char* data_path, const char* target_dir){
	FILE* fmt_file;
	FILE* data_file;
	int result = 0;

	if (!fmt_path || !data_path || !target_dir || target_dir[0] == '\0'){
		fprintf(stderr, "SfxDump: invalid conversion path.\n");
		return 1;
	}

	fmt_file = fopen(fmt_path, "rb");
	if (!fmt_file){
		fprintf(stderr, "SfxDump: unable to open %s.\n", fmt_path);
		return 1;
	}
	data_file = fopen(data_path, "rb");
	if (!data_file){
		fprintf(stderr, "SfxDump: unable to open %s.\n", data_path);
		fclose(fmt_file);
		return 1;
	}

	for (int sound_index = 0; sound_index < SFX_DUMP_SOUND_COUNT; ++sound_index){
		FfWav_header header = {0};
		size_t header_size = fread(&header, 1, sizeof header, fmt_file);

		if (header_size == 0 && feof(fmt_file)) break;
		if (header_size != sizeof header){
			fprintf(stderr, "SfxDump: truncated format table at sound %d.\n", sound_index);
			result = 1;
			break;
		}

		if (!header.len){
			if (fseek(fmt_file, (long)sizeof(WAVEFORMATEX), SEEK_CUR) != 0){
				fprintf(stderr, "SfxDump: truncated empty format record at sound %d.\n", sound_index);
				result = 1;
				break;
			}
			continue;
		}

		if (convert_sound(fmt_file, data_file, &header, target_dir, sound_index) != 0){
			result = 1;
			break;
		}
	}

	if (fclose(fmt_file) != 0) result = 1;
	if (fclose(data_file) != 0) result = 1;
	return result;
}
