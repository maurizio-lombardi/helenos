#ifndef _CPP_WRAP_H_
#define _CPP_WRAP_H_

#ifdef __cplusplus
extern "C" {
#endif

void sound_init(int mute);
void sound_new_samples(const int16_t* samples, size_t count);
void cpu_run_frame(void);
int cartridge_load_file(char *filename);
void *cartridge_dump(size_t *size);
uint8_t *cartridge_sha1_get(void);
void *cpu_dump(size_t *size);
void *ppu_dump(size_t *size);
void *apu_dump(size_t *size);
void cpu_restore(void *data);
void ppu_restore(void *data);
void apu_restore(void *data);
void cartridge_restore(void *data);

#ifdef __cplusplus
}
#endif

#endif

