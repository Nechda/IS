#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

typedef struct WAV_HEADER {
    /* RIFF Chunk Descriptor */
    uint8_t RIFF[4];    // RIFF Header Magic header
    uint32_t ChunkSize; // RIFF Chunk Size
    uint8_t WAVE[4];    // WAVE Header
    /* "fmt" sub-chunk */
    uint8_t fmt[4];         // FMT header
    uint32_t Subchunk1Size; // Size of the fmt chunk
    uint16_t AudioFormat;   // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
    uint16_t NumOfChan;     // Number of channels 1=Mono 2=Sterio
    uint32_t SamplesPerSec; // Sampling Frequency in Hz
    uint32_t bytesPerSec;   // bytes per second
    uint16_t blockAlign;    // 2=16-bit mono, 4=16-bit stereo
    uint16_t bitsPerSample; // Number of bits per sample
    /* "data" sub-chunk */
    uint8_t Subchunk2ID[4]; // "data"  string
    uint32_t Subchunk2Size; // Sampled data length
} wav_hdr;

struct ByteStream {
    ByteStream(const char *filename) { fptr_ = fopen(filename, "r"); }

    void push_front(uint32_t data) {
        in_queue = sizeof(data);
        saved_value = data;
    }

    uint8_t extract_byte() {
        uint8_t ret = 0;
        uint32_t n_bytes = 0;
        if (in_queue) {
            ret = saved_value & 0xFF;
            saved_value >>= 8;
            n_bytes = 1;
            in_queue--;
        } else {
            n_bytes = fread(&ret, sizeof(ret), 1, fptr_);
        }
        return n_bytes == 0 ? 0 : ret;
    }
    ~ByteStream() { fclose(fptr_); }

  private:
    FILE *fptr_ = nullptr;
    uint32_t saved_value = 0;
    int in_queue = 0;
};

int main(int argc, char *argv[]) {
    wav_hdr wavHeader;

    if (argc != 4) {
        std::cout << "Usage:\n  encoder <input wav> <output wav> <hidden file>\n";
        return 1;
    }
    const char *input_filename = argv[1];
    const char *output_filename = argv[2];
    const char *data_filename = argv[3];

    FILE *in_file = fopen(input_filename, "r");
    FILE *out_file = fopen(output_filename, "w");
    ByteStream stream(data_filename);

    fread(&wavHeader, sizeof(wavHeader), 1, in_file);   // read the header
    fwrite(&wavHeader, sizeof(wavHeader), 1, out_file); // create header

    constexpr uint32_t BUFFER_SIZE = sizeof(float) * 1024;
    int8_t *buffer = new int8_t[BUFFER_SIZE];

    auto hidden_file_size = std::filesystem::file_size(data_filename);
    stream.push_front(hidden_file_size);

    uint32_t bytesRead = 0;
    while ((bytesRead = fread(buffer, sizeof(int8_t), BUFFER_SIZE, in_file)) > 0) {
        if (wavHeader.bitsPerSample == 32) {
            using sample_t = float;
            static_assert(sizeof(sample_t) * 8 == 32);
            sample_t *data = (sample_t *)buffer;
            int count = bytesRead / sizeof(data[0]);

            union {
                float f;
                struct {
                    unsigned int mantissa : 23;
                    unsigned int exponent : 8;
                    unsigned int sign : 1;

                } raw;
            } holder;

            for (int i = 0; i < count; i++) {
                uint8_t value = stream.extract_byte();
                holder.f = data[i];
                holder.raw.mantissa &= ((~0U) << 8);
                holder.raw.mantissa |= value & 0xFF;
                data[i] = holder.f;
            }
        }

        fwrite(buffer, sizeof(buffer[0]), bytesRead, out_file);
    }
    delete[] buffer;
    fclose(in_file);
    fclose(out_file);
    return 0;
}
