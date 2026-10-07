#include "../includes/processor.h"


int main() {

    stack_s stack = {};

    processor_start(&stack);

    log_dump_stack(&stack, "");

    processor_execute_commands(&stack);

    log_dump_stack(&stack, "");

    return 0;
}
