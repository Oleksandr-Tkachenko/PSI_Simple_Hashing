/* 
 * File:   psi_cuckoo_hashing.h
 * Author: Oleksandr Tkachenko <oleksandr.tkachenko at crisp-da.de>
 *
 * Created on May 7, 2016, 1:01 PM
 */

#ifndef PSI_CUCKOO_HASHING_H
#define PSI_CUCKOO_HASHING_H

#include <stdlib.h>
#include <math.h>

#include "psi_hashing.h"
#include "psi_structures.h"
#include "psi_misc.h"


#ifdef __cplusplus
extern "C" {
#endif

    void psi_cuckoo_hashing(PSI_CUCKOO_HASHING_CTX *ctx);
    PSI_Cuckoo_list * psi_cuckoo_alloc_list();
    static void free_elem(PSI_Cuckoo_element * e);
    PSI_Cuckoo_element * psi_cuckoo_alloc_elem();
    gboolean psi_cuckoo_list_try_to_insert(PSI_Cuckoo_element * actual, PSI_Cuckoo_element * to_insert);
    void psi_cuckoo_list_iterate(PSI_Cuckoo_list * l, FILE * f_source, FILE * f_dest, FILE * stash, PSI_CUCKOO_HASHING_CTX *ctx);
    void psi_cuckoo_list_remove_node(PSI_Cuckoo_list * l, PSI_Cuckoo_element * e);
    void psi_cuckoo_save(PSI_Cuckoo_list * l, PSI_Cuckoo_element * e, FILE * f_dest, FILE * f_stash, PSI_CUCKOO_HASHING_CTX *ctx);
    void psi_cuckoo_hashing(PSI_CUCKOO_HASHING_CTX *ctx);
    gboolean psi_cuckoo_is_empty(uint8_t * buf);
    static void show_settings(PSI_CUCKOO_HASHING_CTX *ctx);
    static gboolean interpret_limiter(uint8_t * a, uint8_t * b, uint16_t limit);
    static void psi_cuckoo_move_to_stash(PSI_Cuckoo_list *l, PSI_Cuckoo_element * e, FILE * f_stash);
    static gboolean psi_cuckoo_read(FILE * f, uint8_t * buffer);
    static void psi_cuckoo_save_all_one_iter(PSI_Cuckoo_list * l, FILE * f_dest, FILE * f_stash, PSI_CUCKOO_HASHING_CTX *ctx);
    static void psi_cuckoo_save_all(PSI_Cuckoo_list * l, FILE * f_dest, FILE * f_stash, PSI_CUCKOO_HASHING_CTX *ctx);

#ifdef __cplusplus
}
#endif

#endif /* PSI_CUCKOO_HASHING_H */

