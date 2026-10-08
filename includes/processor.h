#include "../includes/stack.h"
#include <math.h>
#include <string.h>

error_code_e processor_start           (stack_s* const stack);
error_code_e processor_execute_commands(stack_s* const stack, const char* const bytecode_file_name);
error_code_e processor_push            (stack_s* const stack, const stack_element element);
error_code_e processor_add             (stack_s* const stack);
error_code_e processor_multiply        (stack_s* const stack);
error_code_e processor_div             (stack_s* const stack);
error_code_e processor_sqrt            (stack_s* const stack);
error_code_e processor_sin             (stack_s* const stack);
error_code_e processor_cos             (stack_s* const stack);
error_code_e processor_dump_stack(const stack_s* const stack);
error_code_e processor_print_top       (stack_s* const stack);
error_code_e processor_hlc             (stack_s* const stack);

