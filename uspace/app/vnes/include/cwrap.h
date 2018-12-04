#ifndef _CWRAPPER_H_
#define _CWRAPPER_H_

#ifdef __cplusplus
extern "C" {
#endif

void new_frame(uint32_t *frame);
int sha1_chksum(uint8_t *data, size_t data_size, uint8_t *hash);

#ifdef __cplusplus
}
#endif

#endif
