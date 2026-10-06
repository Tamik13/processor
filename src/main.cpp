#include "../includes/processor.h"


int main() {

    stack_s stack = {};

    processor_start(&stack);

    processor_push(&stack, 10);
    processor_push(&stack, 20);

    processor_add(&stack);

    log_dump_stack(&stack, "");

    return 0;
}
