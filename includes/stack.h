#include "debugging.h"
#include <string.h>
#include <time.h>

typedef double stack_element;              // Введите между typedef и stack_element тип данных стека
#define STK_MODIFIER "%lg"                 // Введите после stk модификатор вывода типа данных стека
#define POISON_MODIFIER "%llx"
const   stack_element POISON = 0xBAADF00D; // Введите редко (желательно никогда не) встречающиеся значение в стеке



#define STACK_DEBUG                        // Закомментируйте для отключения DEBUG режима



#ifdef STACK_DEBUG
    #define ON_DBG(...) __VA_ARGS__
#else
    #define ON_DBG(...)
#endif

#define LOWER_COEF  4
#define HIGHER_COEF 2

const size_t             COUNT_CANARY      = 2;
const size_t             COUNT_LEFT_CANARY = 1;
const size_t             CANARY_SIZE       = 1;  // единица измерения - sizeof(stack_elemnet)
const unsigned long long LEFT_CANARY       = 0xDEADBABE;
const unsigned long long RIGHT_CANARY      = 0xBADCAFE;

#define TO_STR(val) #val

#define STACK_INIT(stack, capacity) stack_init(stack, capacity ON_DBG(,#stack, __FILE__, __FUNCTION__, __LINE__));

struct stack_s {
    ON_DBG(stack_element left_canary = LEFT_CANARY;)

    ON_DBG(
        const char*    name        = NULL;
        const char*    file        = NULL;
        const char*    function    = NULL;
        size_t         line        = 0;
        unsigned long  struct_hash = 0;
        unsigned long  data_hash   = 0;
        stack_element* canary_data = NULL;
    )

    stack_element* data       = NULL;
    size_t         size       = 0;
    size_t         capacity   = 0;

    ON_DBG(stack_element right_canary = RIGHT_CANARY;)
};

error_code_e stack_init    (stack_s* const stack, const size_t capacity ON_DBG(, const char* const name, const char* const file, const char* const function, const size_t line));
error_code_e stack_push    (stack_s* const stack, const stack_element  value);
error_code_e stack_pop     (stack_s* const stack, stack_element* const value);
error_code_e stack_destroy (stack_s* const stack);
error_code_e stack_recalloc(stack_s* const stack, const size_t new_capacity);

error_code_e stack_verify(stack_s* const stack);
void print_stack         (const stack_s* const stack);
bool is_stack_init       (const stack_s* const stack);

void log_dump_stack(const stack_s* const stack, const char* const reason);
