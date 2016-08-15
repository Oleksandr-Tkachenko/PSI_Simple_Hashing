/* 
 * File:   main.c
 * Author: Oleksandr Tkachenko <oleksandr.tkachenko at crisp-da.de>
 *
 * Created on May 14, 2016, 1:14 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>

#include "psi_simple_hashing.h"
#include "psi_structures.h"

int parse_argv(int argc, char** argv, PSI_SIMPLE_HASHING_CTX* ctx);

int main(int argc, char** argv) {
if(argc==1){
printf("PSI Simple Hashing\n");
return(EXIT_FAILURE);
}
    PSI_SIMPLE_HASHING_CTX ctx[1];
    parse_argv(argc, argv, ctx);
    psi_simple_hashing(ctx);
    return (EXIT_SUCCESS);
}

int parse_argv(int argc, char** argv, PSI_SIMPLE_HASHING_CTX* ctx) {
    int index, c;
    opterr = 0;
    ctx->hash_n = 0;
    while ((c = getopt(argc, argv, "1:2:3:p:e:b:q:s:r:i:t:d:")) != -1)
        switch (c) {
            case '1':
                atob(optarg, ctx->seed[ctx->hash_n]);
                ctx->hash_n++;
                break;
            case '2':
                atob(optarg, ctx->seed[ctx->hash_n]);
                ctx->hash_n++;
                break;
            case '3':
                atob(optarg, ctx->seed[ctx->hash_n]);
                ctx->hash_n++;
                break;
            case 'd':
                ctx->table_size = atof(optarg);
                break;
            case 'i':
                ctx->read_buffer_size = atoi(optarg);
                break;
            case 'e':
                ctx->element_pow = atoi(optarg);
                break;
            case 'p':
                strncpy(ctx->path_root, optarg, 128);
                break;
            case 'b':
                ctx->bucket_n = atoi(optarg);
                break;
            case 'q':
                ctx->queue_buffer_size = atoi(optarg);
                break;
            case 's':
                strncpy(ctx->path_buckets, optarg, 128);
                break;
            case 't':
                ctx->threads = atoi(optarg);
                break;
            case 'r':
                strncpy(ctx->path_source, optarg, 128);
                break;
            case '?':
                if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr,
                        "Unknown option character `\\x%x'.\n",
                        optopt);
                exit(EXIT_FAILURE);
            default:
                abort();
        }

    for (index = optind; index < argc; index++)
        printf("Non-option argument %s\n", argv[index]);

return(EXIT_SUCCESS);
}
