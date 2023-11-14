#include "structs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



Fmt_chunk read_Fmt_chunk(FILE* fmt)
{
	Fmt_chunk chunk = {.size = sizeof chunk.adpcm};

	memcpy(chunk.id, "fmt ", sizeof chunk.id);
	fread(&chunk.adpcm, sizeof chunk.adpcm, 1, fmt);

	return chunk;
}



Loop_chunk read_Loop_chunk(FfWav_header const* header)
{
	Loop_chunk chunk = {
		.size  = sizeof(uint32_t) * 2,
		.start = header->start,
		.end   = header->end
	};

	memcpy(chunk.id, "fflp", sizeof chunk.id);

	return chunk;
}



Data_chunk* read_Data_chunk(FfWav_header const* header, FILE* data)
{
	Data_chunk* chunk = malloc(sizeof *chunk * header->len);

	memcpy(chunk->id, "data", sizeof chunk->id);
	chunk->size = header->len;
	fread(chunk->data, header->len, 1, data);

	return chunk;
}



Riff_header init_riff_header()
{
	Riff_header riff;
	memcpy(&riff.id, "RIFF", sizeof riff.id);
	memcpy(&riff.format, "WAVE", sizeof riff.format);
	riff.size = 0;
	return riff;
}




int main(int argc, char* argv[])
{
	if (argc != 4) {
		printf("Usage: sfxdump fmt_path dat_path target_dir");
		return 1;
	}

	FILE* fmt  = fopen(argv[1], "rb");
	FILE* dat  = fopen(argv[2], "rb");

	if (!fmt || !dat) {
		printf("Could not open .fmt and / or .dat file");
		return 1;
	}

	//printf("Dumping sfx 0 - 750 to %s\n", argv[3]);

	for (int count = 0; count < 750; ++count) {

		FfWav_header header = {0};
		fread(&header, sizeof header, 1, fmt);

		if (!header.len) {
			fseek(fmt, sizeof(WAVEFORMATEX), SEEK_CUR);
			continue;
		}

		Riff_header riff = init_riff_header();
		Fmt_chunk format = read_Fmt_chunk(fmt);
		Loop_chunk loop  = read_Loop_chunk(&header);
		Data_chunk* data = read_Data_chunk(&header, dat);
		riff.size = sizeof riff.format + sizeof format + sizeof *data + data->size;

		if (header.loop)
			riff.size += sizeof loop;

		char path[260];
		sprintf(path, "%s/%d.wav", argv[3], count);
		FILE* out_wav = fopen(path, "wb");

		if (!out_wav) {
			//printf("Error opening %s\n", path);
			continue;
		}

		fwrite(&riff, sizeof riff, 1, out_wav);
		fwrite(&format, sizeof format, 1, out_wav);
		fwrite(data, sizeof *data + data->size, 1, out_wav);

		if (header.loop) {
			//printf("Appending loop data for %d\n", count);
			fwrite(&loop, sizeof loop, 1, out_wav);
		}

		fclose(out_wav);
		free(data);
	}

	fclose(fmt);
	fclose(dat);
}
