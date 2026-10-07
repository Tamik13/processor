#include "../includes/stack.h"
#include <math.h>
#include <string.h>

const stack_element epsilon = 1e-6;
const size_t MAX_COMMAND_LEN = 15;

error_code_e processor_start           (stack_s* const stack);
error_code_e processor_execute_commands(stack_s* const stack);
error_code_e processor_push            (stack_s* const stack, const stack_element element);
error_code_e processor_add             (stack_s* const stack);
error_code_e processor_multiply        (stack_s* const stack);
error_code_e processor_div             (stack_s* const stack);
error_code_e processor_sqrt            (stack_s* const stack);
error_code_e processor_sin             (stack_s* const stack);
error_code_e processor_cos             (stack_s* const stack);
error_code_e processor_dump_stack(const stack_s* const stack);
error_code_e processor_print_top       (stack_s* const stack);

