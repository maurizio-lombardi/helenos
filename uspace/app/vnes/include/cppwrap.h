#ifndef _CPP_WRAP_H_
#define _CPP_WRAP_H_

#ifdef __cplusplus
extern "C" {
#endif

void sound_init(void);
void sound_new_samples(const int16_t* samples, size_t count);
void cpu_run_frame(void);
int cartridge_load_file(char *filename);
void *cartridge_dump(size_t *size);
void *cpu_dump(size_t *size);
void *ppu_dump(size_t *size);

#ifdef __cplusplus
}
#endif

#endif

