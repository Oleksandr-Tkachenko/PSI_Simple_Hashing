#include "psi_cuckoo_hashing.h"

void psi_cuckoo_hashing(PSI_CUCKOO_HASHING_CTX *ctx) {
    self_configuration(ctx);
    show_settings(ctx);

    psi_cuckoo_tree_iterate(ctx);

    fclose(ctx->f_source);
    fclose(ctx->f_stash);
    fclose(ctx->f_dest);

    printf("There are %zu elements in stash\n", fsize(ctx->path_stash) / 16);
}

static void psi_cuckoo_tree_iterate(PSI_CUCKOO_HASHING_CTX *ctx) {
    uint8_t buffer[16];
    while (psi_cuckoo_read(ctx->f_source, buffer)) {
        uint8_t * new_elem = (uint8_t*) calloc(19 * sizeof*new_elem, 1);
        uint64_t * new_hash = (uint64_t*) malloc(sizeof*new_hash);
        memcpy(new_elem, buffer, 16);
        psi_get_64bit_sha256_with_seed(ctx->seed[new_elem[16]], new_elem, new_hash);
        *new_hash /= ctx->divisor;
        if (g_tree_lookup(ctx->tree, new_hash)) {
            PSI_Cuckoo_wle * wle = (PSI_Cuckoo_wle*) malloc(sizeof*wle);
            wle->buffer = new_elem;
            wle->hash_val = new_hash;
            ctx->wait_list = g_slist_prepend(ctx->wait_list, wle);
        } else
            g_tree_insert(ctx->tree, new_hash, new_elem);
        if (g_tree_nnodes(ctx->tree) >= ctx->read_buffer_size)
            psi_cuckoo_tree_save_all_once(ctx);
    }
    psi_cuckoo_tree_save_all(ctx->f_dest, ctx->f_stash, ctx);
}

static gboolean psi_cuckoo_is_empty(uint8_t * buf) {
    for (uint8_t i = 0; i < 19; i++)
        if (buf[i] != 0)return FALSE;
    return TRUE;
}

static void self_configuration(PSI_CUCKOO_HASHING_CTX *ctx) {
    char adr[64];
    snprintf(adr, 32, "%d", (int) pow(10, ctx->element_pow));
    strncat(ctx->path_source, adr, 32);

    psi_mkdir(ctx->path_dest);
    strncpy(adr, ctx->path_dest, 64);
    snprintf(ctx->path_stash, 128, "%sstash", adr);
    snprintf(ctx->path_dest, 128, "%stable", adr);

    ctx->f_source = psi_try_fopen(ctx->path_source, "rb");
    ctx->f_stash = psi_try_fopen(ctx->path_stash, "ab");
    ctx->f_dest = psi_try_fopen(ctx->path_dest, "ab");

    ctx->size_source = fsize(ctx->path_source);
    ctx->size_table = (ctx->size_source / 16)*19 * ctx->d_mult_size_table;
    ctx->divisor = (0xFFFFFFFFFFFFFFFF / ctx-> size_table) * 19;

    uint8_t zero_buf[19] = {0};
    for (off_t i = 0; i < ctx->size_table;) {
        fwrite(zero_buf, 19, 1, ctx->f_dest);
        i += 19;
    }
    fclose(ctx->f_dest);
    ctx->f_dest = psi_try_fopen(ctx->path_dest, "rb+");
    ctx->tree = g_tree_new_full(psi_cuckoo_tree_compare, NULL, g_free, g_free);
}

static void show_settings(PSI_CUCKOO_HASHING_CTX *ctx) {
    printf("Source path : %s\n", ctx->path_source);
    printf("Destination path : %s\n", ctx->path_dest);
    printf("Stash path : %s\n", ctx->path_stash);
    printf("Size of the table : %zu(%zu * 19/16 * %0.1f) Bytes\n", ctx->size_table, ctx->size_source, ctx->d_mult_size_table);
    printf("Read buffer size : %zu KB (%zu elements)\n", (size_t) (ctx->read_buffer_size * sizeof (PSI_Cuckoo_list_element) / 1000), ctx->read_buffer_size);
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

static gboolean psi_cuckoo_read(FILE * f, uint8_t * buffer) {
    if (fread(buffer, 16, 1, f) < 1) return FALSE;
    return TRUE;
}

static void psi_cuckoo_tree_move_to_stash(uint8_t * elem, FILE * f) {
    if (fwrite(elem, 16, 1, f) < 1)
        printf("Failed writing to stash\n");
}

static gboolean psi_cuckoo_tree_save_elem(gpointer hash, gpointer elem, gpointer ctx) {
    /*Current position*/
    off_t cur = ftello(((PSI_CUCKOO_HASHING_CTX*) ctx)->f_dest);
    /*Temp buffer*/
    uint8_t buf[19];
    /*Search needed position*/
    fseeko(((PSI_CUCKOO_HASHING_CTX*) ctx)->f_dest, (*((uint64_t*) hash)) * 19 - cur, SEEK_CUR);
    /*Read element at position we are going to write*/
    if (fread(buf, 19, 1, ((PSI_CUCKOO_HASHING_CTX*) ctx)->f_dest) < 1) printf("Error reading from table\n");
    /*Step 1 element back*/
    fseeko(((PSI_CUCKOO_HASHING_CTX*) ctx)->f_dest, -19, SEEK_CUR);
    /*Write element from buffer*/
    if ((fwrite((uint8_t*) elem, 19, 1, ((PSI_CUCKOO_HASHING_CTX*) ctx)->f_dest)) < 1) printf("Error writing to table\n");
    /*Check if pulled element is not all 0*/
    if (psi_cuckoo_is_empty(buf)) return FALSE;

    uint8_t * new_elem = (uint8_t*) malloc(19 * sizeof*new_elem);
    uint64_t * new_hash = (uint64_t*) malloc(sizeof*new_hash);

    memcpy(new_elem, buf, 19);
    new_elem[16] = (new_elem[16] + 1) % ((PSI_CUCKOO_HASHING_CTX*) ctx)->hash_n;
    psi_get_64bit_sha256_with_seed(((PSI_CUCKOO_HASHING_CTX*) ctx)->seed[new_elem[16]], new_elem, new_hash);
    *new_hash /= ((PSI_CUCKOO_HASHING_CTX*) ctx)->divisor;

    if (interpret_limiter(&new_elem[17], &new_elem[18], ((PSI_CUCKOO_HASHING_CTX*) ctx)->rec_limit)) {
        if (g_tree_lookup(((PSI_CUCKOO_HASHING_CTX*) ctx)->tree, new_hash)) {
            PSI_Cuckoo_wle * e = (PSI_Cuckoo_wle *) malloc(sizeof*e);
            e->buffer = new_elem;
            e->hash_val = new_hash;
            ((PSI_CUCKOO_HASHING_CTX*) ctx)->wait_list =
                    g_slist_prepend(((PSI_CUCKOO_HASHING_CTX*) ctx)->wait_list, e);
        } else
            g_tree_insert(((PSI_CUCKOO_HASHING_CTX*) ctx)->tree, new_hash, new_elem);
    } else {
        psi_cuckoo_tree_move_to_stash(new_elem, ((PSI_CUCKOO_HASHING_CTX*) ctx)->f_stash);
        free(new_elem);
        free(new_hash);
    }
    return FALSE;
}

static void psi_cuckoo_tree_save_all_once(PSI_CUCKOO_HASHING_CTX *ctx) {
    GTree * tree_to_save = ctx->tree;
    ctx->tree = g_tree_new_full(psi_cuckoo_tree_compare, NULL, g_free, g_free);
    g_tree_foreach(tree_to_save, psi_cuckoo_tree_save_elem, (gpointer) ctx);
    g_tree_destroy(tree_to_save);

    size_t i = 0;
    while (ctx->wait_list != NULL && g_tree_nnodes(ctx->tree) < ctx->read_buffer_size) {
        PSI_Cuckoo_wle * e = g_slist_nth_data(ctx->wait_list, i);
        if (e == NULL) break;
        if (g_tree_lookup(ctx->tree, e->hash_val)) i++;
        else {
            g_tree_insert(ctx->tree, e->hash_val, e->buffer);
            free(e);
            ctx->wait_list = g_slist_delete_link(ctx->wait_list, g_slist_nth(ctx->wait_list, i));
        }
    }
}

static void psi_cuckoo_tree_save_all(FILE * f_dest, FILE * f_stash, PSI_CUCKOO_HASHING_CTX *ctx) {
    while (ctx->tree != NULL && g_tree_height(ctx->tree) > 0) {
        psi_cuckoo_tree_save_all_once(ctx);
    }
}

static gint psi_cuckoo_tree_compare(gconstpointer a, gconstpointer b, gpointer user_data) {
    if (*((uint64_t*) a) > *((uint64_t*) b)) return 1;
    else if (*((uint64_t*) a) == *((uint64_t*) b)) return 0;
    else return -1;
}