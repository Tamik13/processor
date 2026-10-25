#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>

#define PRINT_ERROR(error)    \
    fprintf(stderr, "%s:%d " COLOR_TEXT("ERROR CODE: ", RED_C) "%s   " COLOR_TEXT("ERRNO: ", RED_C) "%s  %s\n",  __FILE__, __LINE__, my_str_error(error), strerror(errno), __FUNCTION__);

#define ASSERT_FOR_ARR(ind, size) assert(0 <= (size_t)ind && (size_t)ind < (size_t)size);

#define $ANCHOR         fprintf(stderr, "%s:%d " COLOR_TEXT("ANCHOR",         VIOLET_C) " %s\n", __FILE__, __LINE__, __FUNCTION__);
#define $START_FUNCTION fprintf(stderr, "%s:%d " COLOR_TEXT("START FUNCTION", BLUE_C)   " %s\n", __FILE__, __LINE__, __FUNCTION__);
#define $END_FUNCTION   fprintf(stderr, "%s:%d " COLOR_TEXT("END FUNCTION",   YELLOW_C) " %s\n", __FILE__, __LINE__, __FUNCTION__);

#define $PRINT_STR_ARR(arr, size)                                                                   \
    assert(arr != NULL);                                                                            \
    $ANCHOR                                                                                         \
                                                                                                    \
    printf(COLOR_TEXT("%s:%d " #arr " %s", VIOLET_C) "\n", __FILE__,  __LINE__, __FUNCTION__);        \
    print_str_matrix(arr, size);                                                                    \
    getchar();                                                                                      \
                                                                                                    \
    fprintf(stderr, "\n");

#define $PRINT_PTR_ARR(arr, size)                                                                   \
    assert(arr != NULL);                                                                            \
    $ANCHOR                                                                                         \
                                                                                                    \
    for (size_t x = 0; x < size; x++) {                                                             \
        ASSERT_FOR_ARR(x, size);                                                                    \
        fprintf(stderr, COLOR_TEXT(#arr, VIOLET_C) "[%zu] = %p\n", x, ((void**)arr)[x]);              \
    }                                                                                               \
                                                                                                    \
    printf("\n");

#define $PRINT_INTPTR_ARR(int_arr, size)                                                            \
    assert(int_arr != NULL);                                                                        \
    $ANCHOR                                                                                         \
                                                                                                    \
    for (size_t x = 0; x < size; x++) {                                                             \
        ASSERT_FOR_ARR(x, size);                                                                    \
                                                                                                    \
        fprintf(stderr, COLOR_TEXT(#int_arr, VIOLET_C) "[%zu] = %d\n", x, *((int**)int_arr)[x]);      \
    }                                                                                               \
                                                                                                    \
    fprintf(stderr, "\n");

#define RED_C    "91"
#define GREEN_C  "92"
#define BLUE_C   "94"
#define YELLOW_C "33"
#define VIOLET_C "35"

#define COLOR_TEXT(STR, COLOR)  "\033[" COLOR "m" STR "\033[0m"
#define COLOR_TEXT_START(COLOR) "\033[" COLOR "m"
#define COLOR_TEXT_END          "\033[0m"

#define $int(num)     fprintf(stderr, COLOR_TEXT(#num,    VIOLET_C) " = %d\n\n",       num)
#define $double(num)  fprintf(stderr, COLOR_TEXT(#num,    VIOLET_C) " = %lg\n\n",      num)
#define $luint(num)   fprintf(stderr, COLOR_TEXT(#num,    VIOLET_C) " = %lu\n\n",      num)
#define $llint(num)   fprintf(stderr, COLOR_TEXT(#num,    VIOLET_C) " = %lld\n\n",     num)
#define $uint(num)    fprintf(stderr, COLOR_TEXT(#num,    VIOLET_C) " = %u\n\n",       num)
#define $char(symbol) fprintf(stderr, COLOR_TEXT(#symbol, VIOLET_C) " = <%c>, %d\n\n", symbol, symbol)
#define $string(str)  fprintf(stderr, COLOR_TEXT(#str,    VIOLET_C) " = <%s>\n\n",     str)
#define $size_t(num)  fprintf(stderr, COLOR_TEXT(#num,    VIOLET_C) " = %zu\n\n",      num)
#define $ptr(ptr)     fprintf(stderr, COLOR_TEXT(#ptr,    VIOLET_C) " = %p\n\n",       ptr)
#define $(param)      $ANCHOR _$(param);

enum error_code_e {
    SUCCESS              = 0,
    INCORRECT_SIZE       = 1,
    NULL_PARAM           = 2,
    ALLOCATION_ERROR     = 3,
    POP_VOID_STACK       = 4,
    ERROR_DURING_OPEN    = 5,
    ERROR_DURING_CLOSE   = 6,
    SIZE_HIGHER_CAPACITY = 7,
    ZERO_CAPACITY        = 8,
    NULL_STACK           = 9,
    CANARY_IS_DEAD       = 10,
    REINITIALIZATION     = 11,
    HASH_CHANGED         = 12,
    SEG_FAULT            = 13,
    ERROR_DURING_READ    = 14,
    DIVISION_BY_ZERO     = 15,
    STAT_READ_ERROR      = 16,
    END_OF_FILE          = 17,
    INCORRECT_PARAM      = 18,
    INIT_VALUE           = -1,
    UNEXPECTED_ERROR     = -2,
};

const char* const LOG_FILE_NAME = "log.txt";

void start_logs     ();
void log_print      (const char* const message);
void log_print_error(error_code_e error,        const char* const message);

unsigned long djb2_hash    (const unsigned char* str, const size_t size);
unsigned long djb2_str_hash(const unsigned char* str);

void $print_strptr_arr(const char* const arr[],       const size_t size);
void $print_str_matrix(const char* const arr,         const size_t size_x, const size_t size_y);
void $print_int_arr   (const int         int_array[], const size_t size);
void $print_intptr_arr(const int* const  int_array[], const size_t size);

const char* my_str_error(const error_code_e error_code);

error_code_e is_readble_ptr(const void* ptr);
error_code_e stat_get_size (const char*  const file_name, size_t* const file_size);

void _$(int    num);
void _$(double num);
void _$(size_t num);
void _$(long long num);
void _$(const char* const str);
