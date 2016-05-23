/* 
 * File:   psi_simple_hashing.h
 * Author: Oleksandr Tkachenko <oleksandr.tkachenko at crisp-da.de>
 *
 * Created on May 14, 2016, 1:22 PM
 */

#ifndef PSI_SIMPLE_HASHING_H
#define PSI_SIMPLE_HASHING_H

#include <math.h>

#include "psi_misc.h"
#include "psi_hashing.h"
#include "psi_structures.h"

#ifdef __cplusplus
extern "C" {
#endif

    void psi_simple_hashing(PSI_SIMPLE_HASHING_CTX * ctx);
    static void show_settings(PSI_SIMPLE_HASHING_CTX *ctx);
    PSI_Queue ** psi_sh_allocate_queues(PSI_SIMPLE_HASHING_CTX * ctx);
    void psi_sh_free_queues(PSI_SIMPLE_HASHING_CTX * ctx);
    void parse_paths(PSI_SIMPLE_HASHING_CTX * ctx);
    static void handle_input(PSI_SIMPLE_HASHING_CTX * ctx);
    static void save_bucket(PSI_SIMPLE_HASHING_CTX * ctx, uint64_t * n);
    static void save_buffer(PSI_SIMPLE_HASHING_CTX * ctx, uint8_t ** buf, size_t n);
    static void add_to_bucket(PSI_SIMPLE_HASHING_CTX * ctx, PSI_Queue * q, uint8_t * buf, uint8_t hash_n, uint64_t * bucket);
    static void save_all_buckets(PSI_SIMPLE_HASHING_CTX * ctx);

#ifdef __cplusplus
}
#endif

#endif /* PSI_SIMPLE_HASHING_H */

