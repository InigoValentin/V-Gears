#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "SfxDump.h"
#include "SfxDumpStructures.h"

BOOST_AUTO_TEST_CASE(SfxDumpWritesLoopedWaveFile){
    const boost::filesystem::path temp_dir = boost::filesystem::temp_directory_path()
      / boost::filesystem::unique_path("vgears-sfxdump-%%%%-%%%%");
    boost::filesystem::create_directories(temp_dir);

    const boost::filesystem::path fmt_path = temp_dir / "audio.fmt";
    const boost::filesystem::path data_path = temp_dir / "audio.dat";
    const boost::filesystem::path sound_dir = temp_dir / "sounds";
    boost::filesystem::create_directories(sound_dir);

    {
        std::ofstream fmt_file(fmt_path.string().c_str(), std::ios::binary);
        FfWav_header empty_header = {0};
        WAVEFORMATEX empty_format = {0};
        FfWav_header sound_header = {4, 0, 1, 0, 2, 3};
        ADPCMWAVEFORMAT adpcm_format = {0};

        fmt_file.write(reinterpret_cast<const char*>(&empty_header), sizeof empty_header);
        fmt_file.write(reinterpret_cast<const char*>(&empty_format), sizeof empty_format);
        fmt_file.write(reinterpret_cast<const char*>(&sound_header), sizeof sound_header);
        fmt_file.write(reinterpret_cast<const char*>(&adpcm_format), sizeof adpcm_format);
        BOOST_REQUIRE(fmt_file.good());
    }
    {
        std::ofstream data_file(data_path.string().c_str(), std::ios::binary);
        const unsigned char sample_data[] = {0x10, 0x20, 0x30, 0x40};
        data_file.write(reinterpret_cast<const char*>(sample_data), sizeof sample_data);
        BOOST_REQUIRE(data_file.good());
    }

    BOOST_REQUIRE_EQUAL(
      SfxDumpConvert(fmt_path.string().c_str(), data_path.string().c_str(), sound_dir.string().c_str()),
      0
    );

    const boost::filesystem::path wav_path = sound_dir / "1.wav";
    BOOST_REQUIRE(boost::filesystem::exists(wav_path));
    BOOST_CHECK(!boost::filesystem::exists(sound_dir / "0.wav"));

    std::ifstream wav_file(wav_path.string().c_str(), std::ios::binary);
    std::vector<char> wav_bytes(
      (std::istreambuf_iterator<char>(wav_file)), std::istreambuf_iterator<char>()
    );
    const size_t expected_size = sizeof(Riff_header) + sizeof(Fmt_chunk)
      + sizeof(Data_chunk) + 4 + sizeof(Loop_chunk);
    BOOST_REQUIRE_EQUAL(wav_bytes.size(), expected_size);
    BOOST_CHECK_EQUAL(std::string(wav_bytes.data(), 4), "RIFF");
    BOOST_CHECK_EQUAL(std::string(wav_bytes.data() + 8, 4), "WAVE");
    BOOST_CHECK_EQUAL(std::string(wav_bytes.data() + 70, 4), "data");
    BOOST_CHECK_EQUAL(std::string(wav_bytes.data() + 82, 4), "fflp");

    uint32_t riff_size = 0;
    std::memcpy(&riff_size, wav_bytes.data() + 4, sizeof riff_size);
    BOOST_CHECK_EQUAL(riff_size, expected_size - 8);

    boost::filesystem::remove_all(temp_dir);
}