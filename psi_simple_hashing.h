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

#define LAST_ELEM_FLAG 0x80
#define READ_BUF_SIZE 10000

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
    static void psi_sh_create_table(PSI_SIMPLE_HASHING_CTX * ctx);
    static size_t psi_sh_read_from_bucket(FILE * f, uint8_t buf[]);
    static GList * psi_sh_save_elem_to_list(GList * l, uint8_t * elem);
    static void psi_sh_save_lists(GList ** lists, FILE * f, size_t l_size);
    static void psi_sh_clear_lists(GList ** lists, size_t l_size);
    static void psi_sh_null_lists(GList ** lists, size_t l_size);
    static void psi_sh_save_elem_to_res(gpointer elem, gpointer f);
    static GList * psi_sh_add_empty(GList * l);
    
#ifdef __cplusplus
}
#endif

#endif /* PSI_SIMPLE_HASHING_H */

