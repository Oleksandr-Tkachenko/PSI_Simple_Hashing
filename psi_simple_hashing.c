#include "psi_simple_hashing.h"

static void show_settings(PSI_SIMPLE_HASHING_CTX *ctx);
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

void psi_simple_hashing(PSI_SIMPLE_HASHING_CTX * ctx) {
    show_settings(ctx);
    ctx->queues = psi_sh_allocate_queues(ctx);
    handle_input(ctx);
    psi_sh_free_queues(ctx);
    psi_sh_create_table(ctx);
}

static void show_settings(PSI_SIMPLE_HASHING_CTX *ctx) {
    if (ctx->path_buckets[strlen(ctx->path_buckets) - 1] != '/');
    strcat(ctx->path_buckets, "/");
    ctx->size_source = fsize(ctx->path_source);
    printf("Source path : %s\n", ctx->path_source);
    printf("Buckets path : %s\n", ctx->path_buckets);
    printf("Queues buffer size : %zu\n", ctx->queue_buffer_size);
    printf("Source size : %zu\n", ctx->size_source);
    if (ctx->fixed_table_size)
        printf("Table size : %zu\n", ctx->fixed_table_size);
    else
        printf("Table size : x%0.2f\n", ctx->table_size);
    printf("Result path : %s\n", ctx->path_result);
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

PSI_Queue ** psi_sh_allocate_queues(PSI_SIMPLE_HASHING_CTX * ctx) {
    PSI_Queue ** queues = (PSI_Queue**) malloc(ctx->bucket_n * sizeof (*queues));
    if (queues == NULL)
        perror("Error by memory allocation");
    for (size_t i = 0; i < ctx->bucket_n; i++) {
        queues[i] = (PSI_Queue*) malloc(sizeof (PSI_Queue));
        queues[i]->counter = 0;
        queues[i]->buffer = (uint8_t**) malloc((sizeof (uint8_t*)) * ctx->queue_buffer_size);
        if (queues[i] == NULL || queues[i]->buffer == NULL)
            perror("Error by memory allocation");
        for (size_t j = 0; j < ctx->queue_buffer_size; j++) {
            queues[i]->buffer[j] = g_slice_alloc(17);
            if (queues[i]->buffer[j] == NULL)
                perror("Error by memory allocation");
        }
    }
    return queues;
}

void psi_sh_free_queues(PSI_SIMPLE_HASHING_CTX * ctx) {
    for (size_t i = 0; i < ctx->bucket_n; i++) {
        for (size_t j = 0; j < ctx->queue_buffer_size; j++)
            g_slice_free1(17, ctx->queues[i]->buffer[j]);
        free(ctx->queues[i]->buffer);
        free(ctx->queues[i]);
    }
    free(ctx->queues);
}

static void handle_input(PSI_SIMPLE_HASHING_CTX * ctx) {
    psi_mkdir(ctx->path_buckets);
    ctx->divisor = 0xFFFFFFFFFFFFFFFF / ctx->bucket_n;
    FILE * f_source = psi_try_fopen(ctx->path_source, "rb");

    uint8_t **buf;
    buf = (uint8_t**) malloc((sizeof*buf) * ctx->read_buffer_size);
    for (size_t j = 0; j < ctx->read_buffer_size; j++)
        buf[j] = (uint8_t*) malloc(17);

    for (size_t i = 0; i < ctx->read_buffer_size; i++) {
        if (fread(buf[i], 16, 1, f_source) < 1) {
            save_buffer(ctx, buf, i);
            save_all_buckets(ctx);
            break;
        } else if (i == ctx->read_buffer_size - 1) {
            save_buffer(ctx, buf, i);
            i = 0;
        }
    }
    for (size_t i = 0; i < ctx->read_buffer_size; i++)
        free(buf[i]);
    free(buf);
    fclose(f_source);
}

static void save_bucket(PSI_SIMPLE_HASHING_CTX * ctx, uint64_t * n) {
    char tmp[128];
    strncpy(tmp, ctx->path_buckets, 110);
    snprintf(tmp + strlen(tmp), 16, "%"PRIu64, *n);

    FILE * f = psi_try_fopen(tmp, "ab");
    for (size_t i = 0; i < ctx->queues[*n]->counter; i++)
        if (fwrite(ctx->queues[*n]->buffer[i], 17, 1, f) < 1)
            printf("Error writing to bucket\n");
    ctx->queues[*n]->counter = 0;
    fclose(f);
}

static void save_buffer(PSI_SIMPLE_HASHING_CTX * ctx, uint8_t ** buf, size_t n) {
#pragma omp parallel for shared(n, buf, ctx) num_threads(ctx->threads)
    for (size_t i = 0; i < n; i++) {
        uint64_t bucket;
        for (uint8_t j = 0; j < ctx->hash_n; j++) {
            psi_get_64bit_sha256_with_seed(ctx->seed[j], buf[i], &bucket);
            bucket /= ctx->divisor;
            add_to_bucket(ctx, ctx->queues[bucket], buf[i], j, &bucket);
        }
    }
}

static void add_to_bucket(PSI_SIMPLE_HASHING_CTX * ctx, PSI_Queue * q, uint8_t * buf, uint8_t hash_n, uint64_t * bucket) {
    while (q->counter == ctx->queue_buffer_size);
#pragma omp critical
    {
        if (q->counter > ctx->queue_buffer_size)
            printf("Queue counter is out of bounds\n");

        memcpy(q->buffer[q->counter], buf, 16);
        q->buffer[q->counter][16] = hash_n;
        q->counter++;
        if (q->counter == ctx->queue_buffer_size) {
            save_bucket(ctx, bucket);
        } else if (q->counter > ctx->queue_buffer_size)
            printf("Queue counter is out of bounds\n");
    }
}

static void save_all_buckets(PSI_SIMPLE_HASHING_CTX * ctx) {
    for (uint64_t i = 0; i < ctx->bucket_n; i++)
        save_bucket(ctx, &i);
}

static void psi_sh_create_table(PSI_SIMPLE_HASHING_CTX * ctx) {
    uint8_t buf[17 * READ_BUF_SIZE];
    size_t read;
    int counter = 0, counter_result = -1;
    uint64_t table_divisor;
    size_t abs_elem_n = ctx->size_source / 16;
    size_t abs_table_size = 0;

    if (ctx->fixed_table_size) {
        table_divisor = (0xFFFFFFFFFFFFFFFF / (ctx->fixed_table_size));
        abs_table_size = ctx->fixed_table_size;
    } else {
        table_divisor = (0xFFFFFFFFFFFFFFFF / (
                (ctx->size_source / 16) * ctx->table_size));
        abs_table_size = (ctx->table_size * abs_elem_n);
    }
    char path_buf[128], remove_buf[150], path_sh_result[128];
    FILE * f_result = NULL;
    GList ** lists;
    size_t l_size = abs_table_size / ctx->bucket_n;

    while (counter < ctx->bucket_n) {
        lists = (GList**) malloc((sizeof*lists) * l_size);
        psi_sh_null_lists(lists, l_size);
        if (counter == 0 || counter % (ctx->bucket_n / 4) == 0) {
            counter_result++;
            strncpy(path_sh_result, ctx->path_result, 110);
            snprintf(path_sh_result + strlen(path_sh_result), 16, "_result%d", counter_result);
            if (f_result != NULL)
                fclose(f_result);
            f_result = psi_try_fopen(path_sh_result, "wb");
        }
        strncpy(path_buf, ctx->path_buckets, 110);
        snprintf(path_buf + strlen(path_buf), 16, "%d", counter);
        if (fsize(path_buf) == 0) {
            counter++;
            continue;
        }
        FILE * f = psi_try_fopen(path_buf, "rb");
        read = psi_sh_read_from_bucket(f, buf);
        while (read > 0) {
            if (read == 0)
                break;
#pragma omp parallel for shared(abs_elem_n, abs_table_size, ctx, buf, read, counter, l_size, lists, table_divisor)num_threads(ctx->threads)
            for (size_t i = 0; i < read; i++) {
                uint64_t pointer;
                uint8_t * elem = (uint8_t*) malloc(17 * sizeof*elem);
                memcpy(elem, buf + (i * 17), 17);
                psi_get_64bit_sha256_with_seed(ctx->seed[elem[16]], elem, &pointer);
                pointer /= table_divisor;
                if (pointer < counter * abs_table_size / ctx->bucket_n &&
                        pointer > (counter + 1) * abs_table_size / ctx->bucket_n)
                    printf("Error assigning element : pointer=%"PRIu64" expected=%"PRIu64"-%"PRIu64" \n",
                        pointer, (uint64_t) (counter * abs_table_size / ctx->bucket_n),
                        (uint64_t) ((counter + 1) * abs_table_size / ctx->bucket_n));
                pointer %= l_size;
                lists[pointer] = psi_sh_save_elem_to_list(lists[pointer], elem);
            }
            read = 0;
            read = psi_sh_read_from_bucket(f, buf);
        }
        counter++;

        if (remove(path_buf))
            printf("Error deleting bucket file\n");
        psi_sh_save_lists(lists, f_result, l_size);
        psi_sh_clear_lists(lists, l_size);
        free(lists);
    }
    snprintf(remove_buf, 149, "find \"%s\" -size 0 -delete", ctx->path_buckets);
    system(remove_buf);
    printf("PSI Simple Hashing : Done\n");
}

static size_t psi_sh_read_from_bucket(FILE * f, uint8_t buf[]) {

    return fread(buf, 17, READ_BUF_SIZE, f);
}

static GList * psi_sh_save_elem_to_list(GList * l, uint8_t * elem) {
    GList * ret = NULL;
#pragma omp critical
    {
        ret = g_list_append(l, elem);
    }
    return ret;
}

static void psi_sh_save_lists(GList ** lists, FILE * f, size_t l_size) {
    for (size_t i = 0; i < l_size; i++) {
        if (lists[i] == NULL) {

            lists[i] = psi_sh_add_empty(lists[i]);
        }
        GList * last = g_list_last(lists[i]);
        ((uint8_t*) last->data)[16] |= LAST_ELEM_FLAG;
        g_list_foreach(lists[i], psi_sh_save_elem_to_res, f);
    }
}

static void psi_sh_clear_lists(GList ** lists, size_t l_size) {
    for (size_t i = 0; i < l_size; i++) {

        g_list_free_full(lists[i], free);
    }
}

static void psi_sh_null_lists(GList ** lists, size_t l_size) {

    for (size_t i = 0; i < l_size; i++)
        lists[i] = NULL;
}

static void psi_sh_save_elem_to_res(gpointer elem, gpointer f) {

    if (fwrite((uint8_t*) elem, 17, 1, (FILE*) f) < 1)
        printf("Error writing result to file\n");
}

static GList * psi_sh_add_empty(GList * l) {
    uint8_t * elem = (uint8_t*) calloc(17 * sizeof*elem, 1);
    return g_list_append(l, elem);
}
