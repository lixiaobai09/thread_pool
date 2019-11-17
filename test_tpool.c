#include <getopt.h>
#include "tpool.h"
#define data_type double

struct option opt_options[] = {
    {"threads", required_argument, NULL, 'n'},
    {"help",    no_argument,       NULL, 'h'},
    {NULL,      no_argument,       NULL, 0}
};
const char* opt_string = "n:h";
void usage(char* process_name) {
    printf("\
usage: %s option [arguments] \n \
       --help -h print this help infomation. \n \
       --threads -n [thread number] set the thread number. \n \
        \n", process_name);
    exit(0);
}

data_type* matrix;
data_type* vec;
data_type* res;
int thread_count = -1, opt_index;
int row, col;
void* thread_func_mul_matrix_vector(void* arg) {
    long arg_data = (long) arg;
    int seg_len = (row + thread_count - 1) / thread_count;
    int start_row = arg_data * seg_len;
    int end_row = start_row + seg_len;
    if (row < end_row) {
        end_row = row;
    }
    for (int i = start_row; i < end_row; ++i) {
        data_type t = 0.0;
        for (int j = 0; j < col; ++j) {
            t += matrix[i * col + j] * vec[j];
        }
        res[i] = t;
    }
    return 0;
}

int main(int argc, char** argv) {
    struct tpool pool;
    int opt = getopt_long(argc, argv, opt_string, opt_options, &opt_index);
    if (opt == -1) {
        usage(argv[0]);
    }
    while (~opt) {
        switch(opt) {
            case 'n':
                thread_count = atoi(optarg);
                break;
            case 'h':
            default:
                usage(argv[0]);
        }
        opt = getopt_long(argc, argv, opt_string, opt_options, &opt_index);
    }
    if (thread_count == -1) {
        fprintf(stderr, "please give the run threads.\n");
        return -1;
    }
    tpool_create(&pool, thread_count);
    scanf("%d %d", &row, &col);
    matrix = malloc(row * col * sizeof(data_type));
    vec = malloc(col * sizeof(data_type));
    res = malloc(row * sizeof(data_type));
    printf("input the matrix:\n");
    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < col; ++j) {
            scanf("%lf", matrix + i * col + j);
        }
    }
    printf("input the vector x:\n");
    for (int i = 0; i < col; ++i) {
        scanf("%lf", vec + i);
    }
    for (int _t = 0; _t < 100000; ++_t) {
        for (long i = 0; i < thread_count; ++i) {
            tpool_add_task(&pool, 0, thread_func_mul_matrix_vector, (void*)i);
        }
    }
    tpool_wait(&pool);
    printf("%f ", res[233]);
    printf("\n");
    tpool_destroy(&pool);
    free(matrix);
    free(vec);
    free(res);
    return 0;
}
