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

#ifdef __cplusplus
}
#endif

#endif /* PSI_CUCKOO_HASHING_H */

