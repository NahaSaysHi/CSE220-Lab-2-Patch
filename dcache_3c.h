#ifndef DCACHE_3C_H
#define DCACHE_3C_H

#ifdef __cplusplus
extern "C" {
#endif

void dcache_3c_init(int num_lines);
void dcache_3c_record_miss(unsigned long long line_addr);
void dcache_3c_record_hit(unsigned long long line_addr);

#ifdef __cplusplus
}
#endif

#endif
