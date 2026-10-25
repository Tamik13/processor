#include "../includes/debugging.h"


void start_logs(void) {
    FILE* log_file = fopen(LOG_FILE_NAME, "w");

    if (log_file == NULL) {
        PRINT_ERROR(ERROR_DURING_OPEN);
        abort();
    }
}


void log_print(const char* const message) {
    assert(message != NULL);

    FILE* log_file = fopen(LOG_FILE_NAME, "a");

    if (log_file == NULL) {
        PRINT_ERROR(ERROR_DURING_OPEN);
        abort();
    }

    time_t date = 0;
    date = time(NULL);

    fprintf(log_file, "\nFUNCTION: log_print DATE:%s", ctime(&date));
    fputs  (message,  log_file);

    if (fclose(log_file) == EOF) {
        PRINT_ERROR(ERROR_DURING_CLOSE);
        abort();
    }
}


void log_print_error(error_code_e error_code, const char* const message) {
    FILE* log_file = fopen(LOG_FILE_NAME, "a");

    if (log_file == NULL) {
        PRINT_ERROR(ERROR_DURING_OPEN);
        abort();
    }

    time_t date = 0;
    date = time(NULL);

    fprintf(log_file, "\nFUNCTION: log_print_error DATE:%s",   ctime(&date));
    fprintf(log_file, "%s"                                 ,   message);
    fprintf(log_file, "ERROR error_code: %s errno_code: %s\n", my_str_error(error_code), strerror(errno));

    if (fclose(log_file) == EOF) {
        PRINT_ERROR(ERROR_DURING_CLOSE);
        abort();
    }
}


unsigned long djb2_hash(const unsigned char* str, const size_t size) {
    assert(str != NULL);

    unsigned long hash = 5381;

    for (size_t ind = 0; ind < size; ind++) {
        hash = ((hash << 5) + hash) + str[ind];
    }

    return hash;
}


unsigned long djb2_str_hash(const unsigned char* str) {
    assert(str != NULL);

    unsigned long hash = 5381;

    while (*str) {
        hash = ((hash << 5) + hash) + *str++;
    }

    return hash;
}


void print_strptr_arr(const char* const arr[], const size_t size) {
    assert(arr != NULL);

    for (size_t block_ind = 0; block_ind < size; block_ind++) {
        ASSERT_FOR_ARR(block_ind, size);
        printf("<%s>\n", arr[block_ind]);
    }
}


void print_str_matrix(const char* const arr, const size_t size_x, const size_t size_y) {
    assert(arr != NULL);

    for (size_t x = 0; x < size_x; x++) {
        ASSERT_FOR_ARR(x, size_x);
        printf("<%s>\n", ((arr + size_y * x)));
    }
}


void print_int_arr(const int int_array[], const size_t size) {
    assert(int_array != NULL);

    for (size_t x = 0; x < size; x++) {
        ASSERT_FOR_ARR(x, size);
        printf("%d ", int_array[x]);
    }
    printf("\n");
}


void print_intptr_arr(const int* const int_array[], const size_t size) {
    assert(int_array != NULL);

    for (size_t x = 0; x < size; x++) {
        ASSERT_FOR_ARR(x, size);

        printf("%d ", *int_array[x]);
    }
    printf("\n");
}


const char* my_str_error(const error_code_e error_code) {
    switch (error_code) {
        case SUCCESS:
            return "SUCCESS: 0";

        case INCORRECT_SIZE:
            return "INCORRECT SIZE: 1";

        case NULL_PARAM:
            return "NULL PARAM: 2";

        case ALLOCATION_ERROR:
            return "ALLOCATION ERROR: 3";

        case POP_VOID_STACK:
            return "POP VOID STACK: 4";

        case ERROR_DURING_OPEN:
            return "ERROR DURING OPEN: 5";

        case ERROR_DURING_CLOSE:
            return "ERROR DURING CLOSE: 6";

        case SIZE_HIGHER_CAPACITY:
            return "SIZE HIGHER CAPACITY: 7";

        case ZERO_CAPACITY:
            return "ZERO CAPACITY: 8";

        case NULL_STACK:
            return "NULL STACK: 9";

        case CANARY_IS_DEAD:
            return "CANARY IS DEAD: 10";

        case REINITIALIZATION:
            return "REINITIALIZATION: 11";

        case HASH_CHANGED:
            return "HASH CHANGED: 12";

        case SEG_FAULT:
            return "SEG FAULT: 13";

        case ERROR_DURING_READ:
            return "ERROR DURING READ: 14";

        case DIVISION_BY_ZERO:
            return "DIVISION BY ZERO: 15";

        case STAT_READ_ERROR:
            return "STAT READ ERROR: 16";

        case END_OF_FILE:
            return "END_OF_FILE: 17";

        case INCORRECT_PARAM:
            return "INCORRECT_PARAM: 18";

        case INIT_VALUE:
            return "INIT VALUE: -1";

        case UNEXPECTED_ERROR:
        default:
            return "UNEXPECTED ERROR: -2";
    }
}


error_code_e is_readble_ptr(const void* ptr) {
    int file_descriptor[2] = {0, 0};



    if (pipe(file_descriptor) == -1) {
        log_print_error(ERROR_DURING_OPEN, "stack_readble_ptr: ERROR cant open file descriptor\n");
        return ERROR_DURING_OPEN;
    }

    ssize_t is_file_open = 0;



    do {
        is_file_open = write(file_descriptor[1], ptr, 1);
    } while (is_file_open == -1 && errno == EINTR);

    int saved_errno = errno;



    if (close(file_descriptor[0])) {
        log_print_error(ERROR_DURING_CLOSE, "is_readble: ERROR FILE_DESCRIPTOR[0] CANT BE CLOSE\n");
    }

    if (close(file_descriptor[1])) {
        log_print_error(ERROR_DURING_CLOSE, "is_readble: ERROR FILE_DESCRIPTOR[1] CANT BE CLOSE\n");
    }



    if (is_file_open == 1) {
        return SUCCESS;
    }



    errno = saved_errno;



    if (is_file_open == -1 && saved_errno == EFAULT) {

        log_print_error(SEG_FAULT, "is_readble: ERROR pointer cant be read\n");

        return SEG_FAULT;
    }



    log_print_error(UNEXPECTED_ERROR, "is_readble_ptr: ERROR unexpected error\n");
    return UNEXPECTED_ERROR;
}

error_code_e stat_get_size(const char* const file_name, size_t* const file_size) {
    struct stat buff = {};

    if (stat(file_name, &buff) == -1) {
        log_print_error(STAT_READ_ERROR, "STAT\n");
        return STAT_READ_ERROR;
    }

    *file_size = (size_t)buff.st_size;

    return SUCCESS;
}

void _$(int       num)         {$int(num);}
void _$(double    num)         {$double(num);}
void _$(size_t    num)         {$size_t(num);}
void _$(long long num)         {$llint(num);}
void _$(void**    num)         {$ptr(num);}
void _$(const char* const str) {$string(str);};

