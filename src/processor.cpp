#include "../includes/processor.h"

error_code_e processor_execute_commands(stack_s* const stack) {
    char          command[MAX_COMMAND_LEN] = {};
    char          line[MAX_COMMAND_LEN]    = {};
    stack_element num                      = NAN;
    int           count_reads              = 0;
    error_code_e  error_code               = INIT_VALUE;

    if (fgets(line, MAX_COMMAND_LEN, stdin) == NULL) {
        return ERROR_DURING_READ;
    }

    count_reads = sscanf(line, "%s %lg", command, &num);

    while (count_reads) {
        if (count_reads == 2 &&
                   strncmp(command, "push",     MAX_COMMAND_LEN) == 0) {

            error_code = processor_push(stack, num);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during push\n");
                return error_code;
            }

        } else if (strncmp(command, "add",      MAX_COMMAND_LEN) == 0) {

            error_code = processor_add(stack);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during add\n");
                return error_code;
            }

        } else if (strncmp(command, "multiply", MAX_COMMAND_LEN) == 0) {

            error_code = processor_multiply(stack);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during multiply\n");
                return error_code;
            }

        } else if (strncmp(command, "div",      MAX_COMMAND_LEN) == 0) {

            error_code = processor_div(stack);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during div\n");
                return error_code;
            }

        } else if (strncmp(command, "sqrt",     MAX_COMMAND_LEN) == 0) {

            error_code = processor_sqrt(stack);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during sqrt\n");
                return error_code;
            }

        } else if (strncmp(command, "sin",      MAX_COMMAND_LEN) == 0) {

            error_code = processor_sin(stack);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during sin\n");
                return error_code;
            }

        } else if (strncmp(command, "cos",      MAX_COMMAND_LEN) == 0) {

            error_code = processor_cos(stack);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during cos\n");
                return error_code;
            }

        } else if (strncmp(command, "top",      MAX_COMMAND_LEN) == 0) {

            error_code = processor_print_top(stack);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during top\n");
                return error_code;
            }

        } else if (strncmp(command, "dump",     MAX_COMMAND_LEN) == 0) {

        error_code = processor_dump_stack(stack);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during dump\n");
                return error_code;
            }

        } else {
            printf("Incorrect command\n");
        }


        if (fgets(line, MAX_COMMAND_LEN, stdin) == NULL) {
            return ERROR_DURING_READ;
        }

        count_reads = sscanf(line, "%s %lg", command, &num);
    }

    return SUCCESS;
}

error_code_e processor_start(stack_s* const stack) {
    start_logs();

    STACK_INIT(stack, 5);

    return SUCCESS;
}

error_code_e processor_push(stack_s* const stack, const stack_element element) {
    error_code_e error_code = INIT_VALUE;

    if (stack == NULL) {
        log_print_error(NULL_PARAM, "add: ERROR null stack ptr\n");
        return NULL_PARAM;
    }

    error_code = stack_push(stack, element);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_push: ERROR during stack_push\n");
        return error_code;
    }

    return SUCCESS;
}

error_code_e processor_add(stack_s* const stack) {
    error_code_e error_code = INIT_VALUE;

    error_code = stack_verify(stack);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_add: ERROR during stack_verify\n");
        return error_code;
    }

    if (stack->size < 2) {
        log_print_error(INCORRECT_SIZE, "processor_add: ERROR stack->size < 2\n");
        return INCORRECT_SIZE;
    }

    stack_element  first_elem = 0;
    stack_element second_elem = 0;

    error_code = stack_pop(stack,  &first_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_add: ERROR during stack_pop(first_elem)\n");
        return error_code;
    }

    error_code = stack_pop(stack, &second_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_add: ERROR during stack_pop(second_elem)\n");
        return error_code;
    }

    error_code = stack_push(stack, first_elem + second_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code,  "processor_add: ERROR during stack_push(first_elem + second_elem)\n");
    }

    return SUCCESS;
}


error_code_e processor_multiply(stack_s* const stack) {
    error_code_e error_code = INIT_VALUE;

    error_code = stack_verify(stack);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_multiplying: ERROR during stack_verify\n");
        return error_code;
    }

    if (stack->size < 2) {
        log_print_error(INCORRECT_SIZE, "processor_multiplying: ERROR stack->size < 2\n");
        return INCORRECT_SIZE;
    }

    stack_element  first_elem = 0;
    stack_element second_elem = 0;

    error_code = stack_pop(stack,  &first_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_multiplying: ERROR during stack_pop(first_elem)\n");
        return error_code;
    }

    error_code = stack_pop(stack, &second_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_multiplying: ERROR during stack_pop(second_elem)\n");
        return error_code;
    }

    error_code = stack_push(stack, first_elem * second_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code,  "processor_multiplying: ERROR during stack_push(first_elem * second_elem)\n");
    }

    return SUCCESS;
}


error_code_e processor_div(stack_s* const stack) {
    error_code_e error_code = INIT_VALUE;

    error_code = stack_verify(stack);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_div: ERROR during stack_verify\n");
        return error_code;
    }

    if (stack->size < 2) {
        log_print_error(INCORRECT_SIZE, "processor_div: ERROR stack->size < 2\n");
        return INCORRECT_SIZE;
    }

    stack_element  first_elem = 0;
    stack_element second_elem = 0;

    error_code = stack_pop(stack,  &first_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_div: ERROR during stack_pop(first_elem)\n");
        return error_code;
    }

    error_code = stack_pop(stack, &second_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_div: ERROR during stack_pop(second_elem)\n");
        return error_code;
    }

    error_code = stack_push(stack, first_elem / second_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code,  "processor_div: ERROR during stack_push(first_elem / second_elem)\n");
    }

    return SUCCESS;
}


error_code_e processor_sqrt(stack_s* const stack) {
    error_code_e error_code = INIT_VALUE;

    error_code = stack_verify(stack);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_sqrt: ERROR during stack_verify\n");
        return error_code;
    }

    if (stack->size < 1) {
        log_print_error(INCORRECT_SIZE, "processor_sqrt: ERROR stack->size < 2\n");
        return INCORRECT_SIZE;
    }

    stack_element elem = 0;

    error_code = stack_pop(stack,  &elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_sqrt: ERROR during stack_pop(first_elem)\n");
        return error_code;
    }

    error_code = stack_push(stack, sqrt(elem));
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_sqrt: ERROR during stack_push(sqrt(elem))\n");
    }

    return SUCCESS;
}


error_code_e processor_sin(stack_s* const stack) {
    error_code_e error_code = INIT_VALUE;

    error_code = stack_verify(stack);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_sin: ERROR during stack_verify\n");
        return error_code;
    }

    if (stack->size < 1) {
        log_print_error(INCORRECT_SIZE, "processor_sin: ERROR stack->size < 2\n");
        return INCORRECT_SIZE;
    }

    stack_element elem = 0;

    error_code = stack_pop(stack,  &elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_sin: ERROR during stack_pop(first_elem)\n");
        return error_code;
    }

    error_code = stack_push(stack, sin(elem * M_PI / 180.0));
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_sin: ERROR during stack_push(sin(elem))\n");
    }

    return SUCCESS;
}


error_code_e processor_cos(stack_s* const stack) {
    error_code_e error_code = INIT_VALUE;

    error_code = stack_verify(stack);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_cos: ERROR during stack_verify\n");
        return error_code;
    }

    if (stack->size < 1) {
        log_print_error(INCORRECT_SIZE, "processor_cos: ERROR stack->size < 2\n");
        return INCORRECT_SIZE;
    }

    stack_element elem = 0;

    error_code = stack_pop(stack,  &elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_cos: ERROR during stack_pop(first_elem)\n");
        return error_code;
    }

    error_code = stack_push(stack, cos(elem * M_PI / 180.0));
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_cos: ERROR during stack_push(cos(elem))\n");
    }

    return SUCCESS;
}


error_code_e processor_dump_stack(const stack_s* const stack) {

    log_dump_stack(stack, "processor_dump_stack\n");

    return SUCCESS;
}

error_code_e processor_print_top(stack_s* const stack) {

    error_code_e error_code = stack_verify(stack);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_print_top: ERROR during stack verify\n");
        return error_code;
    }

    if (stack->size < 1) {
        printf("Stack is void\n");
        return SUCCESS;
    }

    printf("%lg\n", stack->data[stack->size - 1]);

    return SUCCESS;
}
