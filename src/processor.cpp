#include "../includes/stack.h"
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <inttypes.h>

struct command_s {
    uint8_t command = 0;
    double  value_1 = NAN;
};

struct cpu_s {
    stack_s    stack;
    double*    registers;
    size_t     pc;
    command_s* commands;
    size_t     commands_size;
};

#define STAT_GET_SIZE(file_size, file_name)                \
    struct stat buff = {};                      \
                                                \
    if (stat(file_name, &buff) == -1) {   \
        log_print_error(STAT_READ_ERROR, "STAT\n"); \
        return STAT_READ_ERROR;       \
    }                                                            \
                                                \
    file_size = (size_t)buff.st_size; // TODO: func

error_code_e processor_start           (cpu_s*   const cpu);
error_code_e processor_load_program    (cpu_s*   const cpu,   const char* const bytecode_file_name);
error_code_e processor_execute_commands(cpu_s*   const cpu);
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
error_code_e processor_pop             (stack_s* const stack, double* const value);

void         log_dump_cpu        (cpu_s*   const cpu,  const char* const reason);


const stack_element EPSILON  = 1e-6;
const size_t COUNT_REGISTERS = 8;


int main() {
    cpu_s        cpu        = {};
    error_code_e error_code = INIT_VALUE;

    error_code = processor_start(&cpu);
    if (error_code != SUCCESS) {
        PRINT_ERROR(error_code);
        return error_code;
    }

    error_code = processor_load_program(&cpu, "../PROCESSOR/bytecode");
    if (error_code != SUCCESS) {
        PRINT_ERROR(error_code);
        return error_code;
    }

    log_dump_cpu(&cpu, "qwerty");

    if (error_code != SUCCESS) {
        PRINT_ERROR(error_code);
    }

    error_code = processor_execute_commands(&cpu);
    if (error_code != SUCCESS) {
        PRINT_ERROR(error_code);
        return error_code;
    }

    log_dump_cpu(&cpu, "qwerty");

    return 0;
}


error_code_e processor_start(cpu_s* const cpu) {
    start_logs();

    STACK_INIT(&cpu->stack, 5);
    cpu->registers     = (double*)calloc(COUNT_REGISTERS, sizeof(double));
    cpu->pc            = 0;
    cpu->commands_size = 0;
    cpu->commands      = NULL;

    return SUCCESS;
}


error_code_e processor_load_program(cpu_s* const cpu, const char* const bytecode_file_name) {
    uint8_t      command               = 0;
    double       value                 = NAN;
    size_t       file_size             = 0;

    STAT_GET_SIZE(file_size, bytecode_file_name);

    cpu->commands = (command_s*)calloc(file_size, sizeof(command_s));

    if (cpu->commands == NULL) {
        log_print_error(ALLOCATION_ERROR, "processor_load_program: ERROR during allocation\n");
        return ALLOCATION_ERROR;
    }

    FILE* bytecode_file = fopen(bytecode_file_name, "r");
    if (bytecode_file == NULL) {
        log_print_error(ERROR_DURING_OPEN, "processor_load_program: ERROR during open\n");
        return ERROR_DURING_OPEN;
    }

    while (!feof(bytecode_file)) {
        fscanf(bytecode_file, "%" PRIu8, &command);

        cpu->commands[cpu->commands_size].command = command;

        if (command == 1 || command == 11) {
            fscanf(bytecode_file, "%lg", &value);
            cpu->commands[cpu->commands_size].value_1 = value;
        }

        cpu->commands_size++;
    }

    if (fclose(bytecode_file) == EOF) {
        log_print_error(ERROR_DURING_CLOSE, "processor_load_program: ERROR during close\n");
        return ERROR_DURING_CLOSE;
    }

    command_s* reduced_commands = (command_s*)realloc(cpu->commands, sizeof(command_s) * cpu->commands_size);
    if (reduced_commands == NULL) {
        log_print_error(ALLOCATION_ERROR, "processor_load_program: ERROR during recalloc\n");
        return ALLOCATION_ERROR;
    }

    cpu->commands = reduced_commands;

    return SUCCESS;
}


error_code_e processor_execute_commands(cpu_s* const cpu) {
    error_code_e error_code = INIT_VALUE;
    double       value      = NAN;

    for (;cpu->pc < cpu->commands_size; cpu->pc++) {
        switch(cpu->commands[cpu->pc].command) {
        case 1:

            error_code = processor_push(&cpu->stack, cpu->commands[cpu->pc].value_1);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during push\n");
                return error_code;
            }
            break;

        case 2:

            error_code = processor_add(&cpu->stack);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during add\n");
                return error_code;
            }
            break;

        case 3:

            error_code = processor_multiply(&cpu->stack);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during multiply\n");
                return error_code;
            }
            break;

        case 4:

            error_code = processor_div(&cpu->stack);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during div\n");
                return error_code;
            }
            break;

        case 5:

            error_code = processor_sqrt(&cpu->stack);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during sqrt\n");
                return error_code;
            }
            break;

        case 6:

            error_code = processor_sin(&cpu->stack);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during sin\n");
                return error_code;
            }
            break;

        case 7:

            error_code = processor_cos(&cpu->stack);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during cos\n");
                return error_code;
            }
            break;

        case 8:

            error_code = processor_print_top(&cpu->stack);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during top\n");
                return error_code;
            }
            break;

        case 9:

            error_code = processor_dump_stack(&cpu->stack);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during dump\n");
                return error_code;
            }
            break;

        case 10:

            error_code = processor_hlc(&cpu->stack);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during write hlc\n");
                return error_code;
            }

            return SUCCESS;

        case 11:

            error_code = processor_pop(&cpu->stack, &value);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during write pop\n");
                return error_code;
            }

            if ((uint8_t)cpu->commands[cpu->pc].value_1 < 0 || COUNT_REGISTERS <= (uint8_t)cpu->commands[cpu->pc].value_1) {
                log_print_error(INCORRECT_SIZE, "processor_execute_commands: ERROR during write pop, incorrect size\n");
                return INCORRECT_SIZE;
            }

            cpu->registers[(uint8_t)cpu->commands[cpu->pc].value_1] = value;

            break;

        default:
            printf("%d" COLOR_TEXT(" Incorrect command ", RED) "%d\n", __LINE__, cpu->commands[cpu->pc].command);
            break;
        }
    }

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

    if (fabs(second_elem) <= EPSILON) {
        return DIVISION_BY_ZERO;
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


error_code_e processor_hlc(stack_s* const stack) {

    error_code_e error_code = stack_destroy(stack);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_hlc: ERROR during stack destroy\n");
        return error_code;
    }

    return SUCCESS;
}


error_code_e processor_pop(stack_s* const stack, double* const value) {
    error_code_e error_code = INIT_VALUE;

    error_code = stack_verify(stack);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_pop: ERROR during stack_verify\n");
        return error_code;
    }

    if (stack->size < 1) {
        log_print_error(INCORRECT_SIZE, "processor_pop: ERROR stack->size < 2\n");
        return INCORRECT_SIZE;
    }

    error_code = stack_pop(stack, value);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_pop: ERROR during stack_pop\n");
    }

    return SUCCESS;
}


void log_dump_cpu(cpu_s* const cpu, const char* const reason) {
    assert(cpu != NULL);
    error_code_e error_code = INIT_VALUE;

    FILE* log_file = fopen(LOG_FILE_NAME, "a");

    if (log_file == NULL) {
        PRINT_ERROR(ERROR_DURING_OPEN);
        abort();
    }

    time_t tm = 0;
    tm = time(NULL);

    fprintf(log_file, "\nFUNCTION: log_dump_cpu DATE:%s REASON: %s", ctime(&tm), reason);

    fprintf(log_file, "{\n");

    error_code = stack_verify(&cpu->stack);
    if (error_code == SUCCESS) {
        log_dump_stack(&cpu->stack, "");
    } else {
        fprintf(log_file, "\tstack cant be printed\n");
    }

    fprintf(log_file, "\tip            = %zu\n", cpu->pc);
    fprintf(log_file, "\tcommands_size = %zu\n", cpu->commands_size);
    fprintf(log_file, "\tcommands[%p] {\n",      cpu->commands);

    for (size_t ind = 0; ind < cpu->commands_size; ind++) {
        ASSERT_FOR_ARR(ind, cpu->commands_size);
        fprintf(log_file, "\t\t[%zu] = %" PRIu8, ind, cpu->commands[ind].command);

        if (cpu->commands[ind].command == 1 || cpu->commands[ind].command == 11) {
            fprintf(log_file, " %lg", cpu->commands[ind].value_1);
        } else {
            fprintf(log_file, " NAN");
        }

        if (ind == cpu->pc) {
            fprintf(log_file, " <[");
        }

        fprintf(log_file, "\n");
    }

    fprintf(log_file, "\t}\n");
    fprintf(log_file, "\tregisters[%p] {\n", cpu->registers);

    for (size_t ind = 0; ind < COUNT_REGISTERS; ind++) {
        ASSERT_FOR_ARR(ind, COUNT_REGISTERS);
        fprintf(log_file, "\t\t[%zu] = %lg\n", ind, cpu->registers[ind]);
    }

    fprintf(log_file, "\t}\n");
    fprintf(log_file, "}\n");

    if (fclose(log_file) == EOF) {
        PRINT_ERROR(ERROR_DURING_CLOSE);
        abort();
    }
}
