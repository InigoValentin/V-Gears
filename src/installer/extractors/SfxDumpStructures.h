#pragma once
#include <stdint.h>

#pragma pack(push, 1)

typedef struct
{
	int16_t iCoef1;
	int16_t iCoef2;
} ADPCMCOEFSET;

typedef struct
{
  uint16_t wFormatTag;
  uint16_t nChannels;
  uint32_t nSamplesPerSec;
  uint32_t nAvgBytesPerSec;
  uint16_t nBlockAlign;
  uint16_t wBitsPerSample;
  uint16_t cbSize;
} WAVEFORMATEX;

typedef struct
{
	WAVEFORMATEX wfx;
	uint16_t wSamplesPerBlock;
	uint16_t wNumCoef;
	ADPCMCOEFSET aCoef[7];
} ADPCMWAVEFORMAT;

typedef struct
{
	char id[4];
	uint32_t size;
	ADPCMWAVEFORMAT adpcm;
} Fmt_chunk;

typedef struct
{
	char id[4];
	uint32_t size;
	uint32_t start;
	uint32_t end;
} Loop_chunk;

typedef struct
{
	char id[4];
	uint32_t size;
	char data[];
} Data_chunk;

typedef struct
{
	char id[4];
	uint32_t size;
	char format[4];
} Riff_header;

typedef struct
{
	uint32_t len;
	uint32_t offset;
	uint32_t loop;
	uint32_t count;
	uint32_t start;
	uint32_t end;
} FfWav_header;

#pragma pack(pop)
