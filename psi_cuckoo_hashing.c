#include "psi_cuckoo_hashing.h"

void psi_cuckoo_hashing(PSI_CUCKOO_HASHING_CTX *ctx) {
    char adr[64];
    snprintf(adr, 32, "%d", (int) pow(10, ctx->element_pow));
    strncat(ctx->path_source, adr, 32);

    psi_mkdir(ctx->path_dest);
    strncpy(adr, ctx->path_dest, 64);
    snprintf(ctx->path_stash, 128, "%sstash", adr);
    snprintf(ctx->path_dest, 128, "%stable", adr);

    FILE * f_source = psi_try_fopen(ctx->path_source, "rb");
    FILE * f_stash = psi_try_fopen(ctx->path_stash, "ab");
    FILE * f_dest = psi_try_fopen(ctx->path_dest, "ab");
    
    ctx->size_source = fsize(ctx->path_source);
    ctx->size_table = (ctx->size_source / 16)*19 * ctx->d_mult_size_table;
    ctx->divisor = (0xFFFFFFFFFFFFFFFF / ctx-> size_table) * 19;

    show_settings(ctx);

    uint8_t zero_buf[16] = {0};
    for (off_t i = 0; i < ctx->size_table;) {
        fwrite(zero_buf, 16, 1, f_dest);
        i += 16;
    }
    fclose(f_dest);
    f_dest = psi_try_fopen(ctx->path_dest, "rb+");
    ctx->l = psi_cuckoo_alloc_list();
    psi_cuckoo_list_iterate(ctx->l, f_source, f_dest, f_stash, ctx);

    fclose(f_source);
    fclose(f_stash);
    fclose(f_dest);
    
    printf("There are %zu elements in stash\n", fsize(ctx->path_stash)/16);
}

PSI_Cuckoo_list* psi_cuckoo_alloc_list() {
    PSI_Cuckoo_list *l = (PSI_Cuckoo_list*) malloc(sizeof *l);
    if (l == NULL) {
        printf("Could not allocate enough memory\n");
        exit(EXIT_FAILURE);
    }
    l->root = NULL;
    l->size = 0;
    return l;
}

PSI_Cuckoo_element * psi_cuckoo_alloc_elem() {
    PSI_Cuckoo_element * e = (PSI_Cuckoo_element*) malloc(sizeof *e);
    e->buffer = malloc(19 * sizeof *e->buffer);
    if (e == NULL) {
        printf("Could not allocate enough memory\n");
        return NULL;
    }
    e->next = NULL;
    e->prev = NULL;
    e->buffer[16] = 0;
    e->buffer[17] = 0;
    e->buffer[18] = 0;
    return e;
}

static void free_elem(PSI_Cuckoo_element * e) {
    free(e->buffer);
    e->prev = NULL;
    e->next = NULL;
    free(e);
}

void psi_cuckoo_list_insert_sorted(PSI_Cuckoo_list * l, PSI_Cuckoo_element * e) {
    if (l == NULL) {
        printf("Cuckoo list is NULL\n");
        exit(EXIT_FAILURE);
    }
    if (e == NULL) {
        printf("Cuckoo element is NULL\n");
    }
    if (l->root == NULL)
        l->root = e;
    else if (l->root->hash_val >= e->hash_val) {
        e->next = l->root;
        l->root->prev = e;
        l->root = e;
    } else {
        PSI_Cuckoo_element * tmp = l->root;
        while (!psi_cuckoo_list_try_to_insert(tmp, e)) {
            tmp = tmp->next;
            if (tmp == NULL) {
                printf("Error by inserting element in cuckoo list\n");
            }
        }
    }
    l->size++;
}

gboolean psi_cuckoo_list_try_to_insert(PSI_Cuckoo_element * actual, PSI_Cuckoo_element * to_insert) {
    if (actual->hash_val <= to_insert->hash_val && (actual->next == NULL || actual->next->hash_val > to_insert->hash_val)) {
        if (actual->next != NULL)
            actual->next->prev = to_insert;
        to_insert->prev = actual;
        to_insert->next = actual->next;
        actual->next = to_insert;
        return TRUE;
    } else
        return FALSE;
}

void psi_cuckoo_list_remove_node(PSI_Cuckoo_list * l, PSI_Cuckoo_element * e) {
    if (e == NULL) {
        printf("Trying to remove node with address NULL\n");
        return;
    } else if (e->prev == NULL) {
        l->root = l->root->next;
        if (l->root)
            l->root->prev = NULL;
    } else if (e->next == NULL) {
        e->prev->next = NULL;
    } else {
        e->prev->next = e->next;
        e->next->prev = e->prev;
    }
    free_elem(e);
    l->size--;
}

void psi_cuckoo_list_iterate(PSI_Cuckoo_list * l, FILE * f_source, FILE * f_dest, FILE * f_stash, PSI_CUCKOO_HASHING_CTX *ctx) {
    uint8_t buffer[16];
    while (psi_cuckoo_read(f_source, buffer)) {
        PSI_Cuckoo_element * e = psi_cuckoo_alloc_elem();
        memcpy(e->buffer, buffer, 16);
        psi_get_64bit_sha256_with_seed(ctx->seed[e->buffer[16]], e->buffer, &e->hash_val);
        psi_cuckoo_list_insert_sorted(l, e);
        if (l->size >= ctx->read_buffer_size)
            psi_cuckoo_save_all_one_iter(l, f_dest, f_stash, ctx);
    }
    psi_cuckoo_save_all(l, f_dest, f_stash, ctx);
}

void psi_cuckoo_save(PSI_Cuckoo_list * l, PSI_Cuckoo_element * e, FILE * f_dest, FILE * f_stash, PSI_CUCKOO_HASHING_CTX *ctx) {
    off_t cur = ftello(f_dest);
    uint8_t buf[20];
    if (l->root->prev != NULL)
        printf("Memory corruption in root?\n");
    fseeko(f_dest, (e->hash_val / ctx->divisor) * 19 - cur, SEEK_CUR);
    if (fread(buf, 19, 1, f_dest) < 1) printf("Error reading from table\n");
    fseeko(f_dest, -19, SEEK_CUR);
    if (fwrite(e->buffer, 19, 1, f_dest) < 1) printf("Error writing to table\n");
    psi_cuckoo_list_remove_node(l, e);
    if (psi_cuckoo_is_empty(buf)) return;
    else {
        PSI_Cuckoo_element * ret = psi_cuckoo_alloc_elem();
        memcpy(ret->buffer, buf, 19);
        ret->buffer[16] = (ret->buffer[16] + 1) % ctx->hash_n;
        psi_get_64bit_sha256_with_seed(ctx->seed[ret->buffer[16]], ret->buffer, &ret->hash_val);
        if (interpret_limiter(&ret->buffer[17], &ret->buffer[18], ctx->rec_limit)) {
            psi_cuckoo_list_insert_sorted(l, ret);
        } else {
            psi_cuckoo_move_to_stash(l, ret, f_stash);
        }
    }
}

gboolean psi_cuckoo_is_empty(uint8_t * buf) {
    for (uint8_t i = 0; i < 19; i++)
        if (buf[i] != 0)return FALSE;
    return TRUE;
}

static void show_settings(PSI_CUCKOO_HASHING_CTX *ctx) {
    printf("Source path : %s\n", ctx->path_source);
    printf("Destination path : %s\n", ctx->path_dest);
    printf("Stash path : %s\n", ctx->path_stash);
    printf("Size of the table : %zu(%zu * 19/16 * %0.1f) Bytes\n", ctx->size_table, ctx->size_source, ctx->d_mult_size_table);
    printf("Read buffer size : %zu KB (%zu elements)\n", (size_t) (ctx->read_buffer_size * sizeof (PSI_Cuckoo_element) / 1000), ctx->read_buffer_size);
    printf("Recursive deepness limit : %zu\n", ctx->rec_limit);
    printf("Seeds :\n");
    for (uint8_t i = 0; i < ctx->hash_n; i++) {
        printf("%02x%02x%02x%02x%02x%02x%02x%02x",
                ctx->seed[i][0], ctx->seed[i][1], ctx->seed[i][2],
                ctx->seed[i][3], ctx->seed[i][4], ctx->seed[i][5],
                ctx->seed[i][6], ctx->seed[i][7]);

        printf("%02x%02x%02x%02x%02x%02x%02x%02x\n",
                ctx->seed[i][8], ctx->seed[i][9], ctx->seed[i][10],
                ctx->seed[i][11], ctx->seed[i][12], ctx->seed[i][13],
                ctx->seed[i][14], ctx->seed[i][15]);
    }
}

static gboolean interpret_limiter(uint8_t * a, uint8_t * b, uint16_t limit) {
    uint16_t tmp = (*a << 8) + *b;
    tmp++;
    *a = (tmp >> 8) & 0xFF;
    *b = tmp & 0xFF;
    if (tmp >= limit)
        return FALSE;
    return TRUE;
}

static void psi_cuckoo_move_to_stash(PSI_Cuckoo_list *l, PSI_Cuckoo_element * e, FILE * f_stash) {
    fwrite(e->buffer, 16, 1, f_stash);
    psi_cuckoo_list_remove_node(l, e);
}

static gboolean psi_cuckoo_read(FILE * f, uint8_t * buffer) {
    if (fread(buffer, 16, 1, f) < 1) return FALSE;
    return TRUE;
}

static void psi_cuckoo_save_all_one_iter(PSI_Cuckoo_list * l, FILE * f_dest, FILE * f_stash, PSI_CUCKOO_HASHING_CTX *ctx) {
    PSI_Cuckoo_element * e1 = l->root, * e2 = l->root;
    while (e2 != NULL) {
        e1 = e2;
        e2 = e2->next;
        psi_cuckoo_save(l, e1, f_dest, f_stash, ctx);
    }
}

static void psi_cuckoo_save_all(PSI_Cuckoo_list * l, FILE * f_dest, FILE * f_stash, PSI_CUCKOO_HASHING_CTX *ctx) {
    PSI_Cuckoo_element * e1 = l->root, * e2 = l->root;
    while (l->size > 1) {
        e1 = e2;
        e2 = e1->next;
        psi_cuckoo_save(l, e1, f_dest, f_stash, ctx);
        if (e2 == NULL)
            e2 = l->root;
    }
}