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
#include <glib-2.0/glib.h>

#include "psi_hashing.h"
#include "psi_structures.h"
#include "psi_misc.h"


#ifdef __cplusplus
extern "C" {
#endif

    void psi_cuckoo_hashing(PSI_CUCKOO_HASHING_CTX *ctx);
    static void psi_cuckoo_tree_iterate(PSI_CUCKOO_HASHING_CTX *ctx);
    
    /*Checks if cuckoo element is empty*/
    static gboolean psi_cuckoo_is_empty(uint8_t * buf);
    static void self_configuration(PSI_CUCKOO_HASHING_CTX *ctx);
    static void show_settings(PSI_CUCKOO_HASHING_CTX *ctx);

    /*Interprets 2 byte recursive depth limiter*/
    static gboolean interpret_limiter(uint8_t * a, uint8_t * b, uint16_t limit);
    static gboolean psi_cuckoo_read(FILE * f, uint8_t * buffer);

    static void psi_cuckoo_tree_move_to_stash(uint8_t * elem, FILE * f);
    static gboolean psi_cuckoo_tree_save_elem(gpointer hash, gpointer elem, gpointer ctx);
    static void psi_cuckoo_tree_save_all_once(PSI_CUCKOO_HASHING_CTX *ctx);
    static void psi_cuckoo_tree_save_all(FILE * f_dest, FILE * f_stash, PSI_CUCKOO_HASHING_CTX *ctx);

    static gint psi_cuckoo_tree_compare(gconstpointer a, gconstpointer b, gpointer user_data);

#ifdef __cplusplus
}
#endif

#endif /* PSI_CUCKOO_HASHING_H */

