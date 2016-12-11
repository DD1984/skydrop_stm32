#ifndef __DUMP_H__
#define __DUMP_H__

#ifdef __cplusplus
extern "C" {
#endif

void _hex_dump(void *addr, int len, int line_len);
#define hex_dump(addr, len) _hex_dump(addr, len, 16);

#ifdef __cplusplus
}
#endif

#endif
