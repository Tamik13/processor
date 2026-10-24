#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


const size_t WIDTH  = 720;
const size_t HEIGHT = 540;

int main() {
    const char* video_path = "/Users/tamerlannagaev/prodva/PROCESSOR/examples/bad_apple/01858.NicoNico.Douga-Bad.Apple.SD.amvnews.ru.mp4";
    char cmd[512] = "";

    snprintf(cmd, sizeof(cmd),
            "ffmpeg -loglevel quiet -i \"%s\" -f rawvideo -pix_fmt rgb24 pipe:1",
            video_path);

    FILE* ffmpeg_pipe = popen(cmd, "r");
    if (!ffmpeg_pipe) {
        perror("ERROR during start ffmpeg\n");
        return 1;
    }

    FILE* output_file = fopen("/Users/tamerlannagaev/prodva/PROCESSOR/examples/bad_apple/bad_apple.asm", "w");
    if (output_file == NULL) {
        perror("ERROR during open output file\n");
        return 1;
    }

    char last_color[WIDTH * HEIGHT] = "";

    size_t   frame_bytes  = WIDTH * HEIGHT * 3;
    uint8_t *frame_buffer = (uint8_t*)calloc(frame_bytes, sizeof(uint8_t));

    while (fread(frame_buffer, 1, frame_bytes, ffmpeg_pipe) == frame_bytes) {

        for (size_t y = 0; y < HEIGHT; y++) {
            for (size_t x = 0; x < WIDTH; x++) {

                size_t index = (y * WIDTH + x) * 3;
                long long avg = frame_buffer[index] + frame_buffer[index + 1] + frame_buffer[index + 2];
                size_t  cell_index = y * WIDTH + x;
                uint8_t cur_color  = (avg > 384) ? 1 : 0;

                if (cur_color == last_color[cell_index]) {
                    continue;
                }

                last_color[cell_index] = cur_color;

                fprintf(output_file,
                        "push %u\n"
                        "pop [%zu]\n", cur_color, cell_index);

            }
        }
        fprintf(output_file, "dmp\n");
    }

    free  (frame_buffer);
    pclose(ffmpeg_pipe);
    fclose(output_file);

    return 0;
}
