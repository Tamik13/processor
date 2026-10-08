#include "../includes/processor.h"


int main() {
    stack_s stack = {};
    error_code_e error_code = INIT_VALUE;

    error_code = processor_start(&stack);
    if (error_code != SUCCESS) {
        PRINT_ERROR(error_code);
        return error_code;
    }

    error_code = processor_execute_commands(&stack, "bytecode");
    if (error_code != SUCCESS) {
        PRINT_ERROR(error_code);
        return error_code;
    }

    return 0;
}
