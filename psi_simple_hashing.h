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
    PSI_Queue ** psi_sh_allocate_queues(PSI_SIMPLE_HASHING_CTX * ctx);
    void psi_sh_free_queues(PSI_SIMPLE_HASHING_CTX * ctx);
    void parse_paths(PSI_SIMPLE_HASHING_CTX * ctx);
    
#ifdef __cplusplus
}
#endif

#endif /* PSI_SIMPLE_HASHING_H */

