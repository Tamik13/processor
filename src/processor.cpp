#include "../includes/stack.h"
#include "../includes/commands.h"
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <inttypes.h>

struct command_s {
    commands_e command = NAC;
    double  value_1    = NAN;
};

struct cpu_s {
    stack_s    stack;
    stack_s    ret_ln;
    double*    registers;
    size_t     pc;
    command_s* commands;
    size_t     commands_size;
};

error_code_e processor_start           (cpu_s*   const cpu);
error_code_e processor_load_program    (cpu_s*   const cpu,   const char* const bytecode_file_name);
error_code_e processor_execute_commands(cpu_s*   const cpu);
error_code_e processor_push            (stack_s* const stack, const stack_element element);
error_code_e processor_push_reg        (cpu_s*   const cpu,   const size_t reg);
error_code_e processor_add             (stack_s* const stack);
error_code_e processor_mul             (stack_s* const stack);
error_code_e processor_div             (stack_s* const stack);
error_code_e processor_sqrt            (stack_s* const stack);
error_code_e processor_sin             (stack_s* const stack);
error_code_e processor_cos             (stack_s* const stack);
error_code_e processor_out             (stack_s* const stack);
error_code_e processor_in              (cpu_s*   const cpu);
error_code_e processor_pop             (stack_s* const stack, double* const value);
error_code_e processor_jmp             (cpu_s*   const cpu,   const size_t line_num);
error_code_e processor_jb              (cpu_s*   const cpu,   const size_t line_num);
error_code_e processor_ja              (cpu_s*   const cpu,   const size_t line_num);
error_code_e processor_je              (cpu_s*   const cpu,   const size_t line_num);
error_code_e processor_jae             (cpu_s*   const cpu,   const size_t line_num);
error_code_e processor_jbe             (cpu_s*   const cpu,   const size_t line_num);
error_code_e processor_jne             (cpu_s*   const cpu,   const size_t line_num);
error_code_e processor_cal             (cpu_s*   const cpu,   const size_t line_num);
error_code_e processor_ret             (cpu_s*   const cpu);
error_code_e processor_hlc             (cpu_s*   const cpu);

void         log_dump_cpu           (cpu_s* const cpu,       const char* const reason);
error_code_e processor_destroy      (cpu_s* const cpu);
error_code_e stat_get_size    (const char*  const file_name, size_t* const file_size);

const stack_element EPSILON  = 1e-6;
const size_t COUNT_REGISTERS = 8;


int main() {
    cpu_s        cpu        = {};
    error_code_e error_code = INIT_VALUE;

    start_logs();

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


    error_code = processor_execute_commands(&cpu);
    if (error_code != SUCCESS) {
        PRINT_ERROR(error_code);
        return error_code;
    }

    return 0;
}


error_code_e processor_start(cpu_s* const cpu) {
    start_logs();

    STACK_INIT(&cpu->stack,  5);
    STACK_INIT(&cpu->ret_ln, 5);
    cpu->registers     = (double*)calloc(COUNT_REGISTERS, sizeof(double));
    cpu->pc            = 0;
    cpu->commands_size = 0;
    cpu->commands      = NULL;

    return SUCCESS;
}


error_code_e processor_load_program(cpu_s* const cpu, const char* const bytecode_file_name) {
    commands_e   command    = NAC;
    double       value      = NAN;
    size_t       file_size  = 0;
    error_code_e error_code = INIT_VALUE;

   error_code = stat_get_size(bytecode_file_name, &file_size);
   if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_load_program: ERROR during get_stat_size\n");
        return error_code;
   }

   file_size *= 8;

    if (file_size == 0) {
        log_print_error(ALLOCATION_ERROR, "processor_load_program: ERROR void file\n");
        return ALLOCATION_ERROR;
    }

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

    while (cpu->commands_size < file_size && fscanf(bytecode_file, "%d", &command) == 1) {
        cpu->commands[cpu->commands_size].command = command;

        if ((PUSH <= command && command <= POP) || (JMP <= command && command <= CAL)) {
            if (fscanf(bytecode_file, "%lg", &value) != 1) {
                PRINT_ERROR(INCORRECT_PARAM);
                return INCORRECT_PARAM;
            }
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

    for (;cpu->pc < cpu->commands_size; cpu->pc++) {
        log_dump_cpu(cpu, " ");
        switch(cpu->commands[cpu->pc].command) {
        case PUSH:

            error_code = processor_push(&cpu->stack, cpu->commands[cpu->pc].value_1);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during push\n");
                return error_code;
            }
            break;

        case PUR:

            if (0 > cpu->commands[cpu->pc].value_1 || cpu->commands[cpu->pc].value_1 > 8) {
                log_print_error(INCORRECT_SIZE, "processor_execute_commands: ERROR incorrect register\n");
                return INCORRECT_SIZE;
            }

            error_code = processor_push_reg(cpu, (size_t)cpu->commands[cpu->pc].value_1);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during push reg\n");
                return error_code;
            }
            break;

        case POP:

            error_code = processor_pop(&cpu->stack, &cpu->registers[(size_t)cpu->commands[cpu->pc].value_1]);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during pop\n");
                return error_code;
            }
            break;

        case ADD:

            error_code = processor_add(&cpu->stack);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during add\n");
                return error_code;
            }
            break;

        case MUL:

            error_code = processor_mul(&cpu->stack);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during mul\n");
                return error_code;
            }
            break;

        case DIV:

            error_code = processor_div(&cpu->stack);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during div\n");
                return error_code;
            }
            break;

        case OUT:

            error_code = processor_out(&cpu->stack);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during out\n");
                return error_code;
            }
            break;

        case IN:

            error_code = processor_in(cpu);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during processor_in\n");
                return error_code;
            }
            break;

        case SIN:

            error_code = processor_sin(&cpu->stack);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during sin\n");
                return error_code;
            }
            break;

        case COS:
            error_code = processor_cos(&cpu->stack);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during cos\n");
                return error_code;
            }
            break;

        case SQR:
            error_code = processor_sqrt(&cpu->stack);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during write sqr\n");
                return error_code;
            }
            break;

        case JMP:
            error_code = processor_jmp(cpu, (size_t)cpu->commands[cpu->pc].value_1);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during write jmp\n");
                return error_code;
            }
            break;

        case JA:
            error_code = processor_ja(cpu, (size_t)cpu->commands[cpu->pc].value_1);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during write ja\n");
                return error_code;
            }
            break;

        case JB:
            error_code = processor_jb(cpu, (size_t)cpu->commands[cpu->pc].value_1);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during write ja\n");
                return error_code;
            }
            break;

        case JE:
            error_code = processor_je(cpu, (size_t)cpu->commands[cpu->pc].value_1);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during write ja\n");
                return error_code;
            }
            break;

        case JAE:
            error_code = processor_jae(cpu, (size_t)cpu->commands[cpu->pc].value_1);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during write ja\n");
                return error_code;
            }
            break;

        case JBE:
            error_code = processor_jbe(cpu, (size_t)cpu->commands[cpu->pc].value_1);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during write ja\n");
                return error_code;
            }
            break;

        case JNE:
            error_code = processor_jne(cpu, (size_t)cpu->commands[cpu->pc].value_1);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during write ja\n");
                return error_code;
            }
            break;

        case CAL:

            error_code = processor_cal(cpu, (size_t)cpu->commands[cpu->pc].value_1);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during write ja\n");
                return error_code;
            }
            break;

        case RET:

            error_code = processor_ret(cpu);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during write ja\n");
                return error_code;
            }
            break;

        case HLC:

            error_code = processor_hlc(cpu);
            if (error_code != SUCCESS) {
                log_print_error(error_code, "processor_execute_commands: ERROR during write hlc\n");
                return error_code;
            }
            return SUCCESS;

        case NAC:
        default:
            printf("%d" COLOR_TEXT(" Incorrect command ", RED) "%d\n", __LINE__, cpu->commands[cpu->pc].command);
            return INCORRECT_PARAM;
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


error_code_e processor_push_reg(cpu_s* const cpu, const size_t reg) {

    error_code_e error_code = INIT_VALUE;

    if (cpu == NULL) {
        log_print_error(NULL_PARAM, "add: ERROR null cpu ptr\n");
        return NULL_PARAM;
    }

    error_code = stack_push(&cpu->stack, cpu->registers[reg]);
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


error_code_e processor_mul(stack_s* const stack) {
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

    error_code = stack_push(stack,  second_elem / first_elem);
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
        log_print_error(INCORRECT_SIZE, "processor_sqrt: ERROR stack->size < 1\n");
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
        log_print_error(INCORRECT_SIZE, "processor_sin: ERROR stack->size < 1\n");
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
        log_print_error(INCORRECT_SIZE, "processor_cos: ERROR stack->size < 1\n");
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


error_code_e processor_out(stack_s* const stack) {

    error_code_e error_code = stack_verify(stack);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_out: ERROR during stack verify\n");
        return error_code;
    }

    if (stack->size < 1) {
        printf("Stack is void\n");
        return SUCCESS;
    }

    printf("%lg\n", stack->data[stack->size - 1]);

    return SUCCESS;
}


error_code_e processor_in(cpu_s* const cpu) {
    assert(cpu != NULL);
    double value = NAN;

    printf("Enter num:");
    scanf("%lg", &value);
    error_code_e error_code = stack_push(&cpu->stack, value);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_in: ERROR during stack_push\n");
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


error_code_e processor_jmp(cpu_s* const cpu, size_t line_num) {
    assert(cpu != NULL);

    cpu->pc = line_num - 1;

    return SUCCESS;
}


error_code_e processor_jb(cpu_s* const cpu, size_t line_num) {
    assert(cpu != NULL);
    error_code_e error_code = INIT_VALUE;

    if (cpu->stack.size < 2) {
        log_print_error(INCORRECT_SIZE, "processor_jb: ERROR stack->size < 2\n");
        return INCORRECT_SIZE;
    }

    stack_element  first_elem = 0;
    stack_element second_elem = 0;

    error_code = stack_pop(&cpu->stack,  &first_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_jb: ERROR during stack_pop(first_elem)\n");
        return error_code;
    }

    error_code = stack_pop(&cpu->stack, &second_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_jb: ERROR during stack_pop(second_elem)\n");
        return error_code;
    }

    if (first_elem < second_elem) {
        cpu->pc = line_num - 1;
    }

    return SUCCESS;
}


error_code_e processor_ja(cpu_s* const cpu, size_t line_num) {
    assert(cpu != NULL);
    error_code_e error_code = INIT_VALUE;

    if (cpu->stack.size < 2) {
        log_print_error(INCORRECT_SIZE, "processor_ja: ERROR stack->size < 2\n");
        return INCORRECT_SIZE;
    }

    stack_element  first_elem = 0;
    stack_element second_elem = 0;

    error_code = stack_pop(&cpu->stack,  &first_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_ja: ERROR during stack_pop(first_elem)\n");
        return error_code;
    }

    error_code = stack_pop(&cpu->stack, &second_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_ja: ERROR during stack_pop(second_elem)\n");
        return error_code;
    }

    if (first_elem > second_elem) {
        cpu->pc = line_num - 1;
    }

    return SUCCESS;
}


error_code_e processor_je(cpu_s* const cpu, size_t line_num) {
    assert(cpu != NULL);
    error_code_e error_code = INIT_VALUE;

    if (cpu->stack.size < 2) {
        log_print_error(INCORRECT_SIZE, "processor_je: ERROR stack->size < 2\n");
        return INCORRECT_SIZE;
    }

    stack_element  first_elem = 0;
    stack_element second_elem = 0;

    error_code = stack_pop(&cpu->stack,  &first_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_je: ERROR during stack_pop(first_elem)\n");
        return error_code;
    }

    error_code = stack_pop(&cpu->stack, &second_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_je: ERROR during stack_pop(second_elem)\n");
        return error_code;
    }

    if (fabs(first_elem - second_elem) < EPSILON) {
        cpu->pc = line_num - 1;
    }

    return SUCCESS;
}


error_code_e processor_jae(cpu_s* const cpu, size_t line_num) {
    assert(cpu != NULL);
    error_code_e error_code = INIT_VALUE;

    if (cpu->stack.size < 2) {
        log_print_error(INCORRECT_SIZE, "processor_jae: ERROR stack->size < 2\n");
        return INCORRECT_SIZE;
    }

    stack_element  first_elem = 0;
    stack_element second_elem = 0;

    error_code = stack_pop(&cpu->stack,  &first_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_jae: ERROR during stack_pop(first_elem)\n");
        return error_code;
    }

    error_code = stack_pop(&cpu->stack, &second_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_jae: ERROR during stack_pop(second_elem)\n");
        return error_code;
    }

    if (first_elem >= second_elem) {
        cpu->pc = line_num - 1;
    }

    return SUCCESS;
}


error_code_e processor_jbe(cpu_s* const cpu, size_t line_num) {
    assert(cpu != NULL);
    error_code_e error_code = INIT_VALUE;

    if (cpu->stack.size < 2) {
        log_print_error(INCORRECT_SIZE, "processor_jbe: ERROR stack->size < 2\n");
        return INCORRECT_SIZE;
    }

    stack_element  first_elem = 0;
    stack_element second_elem = 0;

    error_code = stack_pop(&cpu->stack,  &first_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_jbe: ERROR during stack_pop(first_elem)\n");
        return error_code;
    }

    error_code = stack_pop(&cpu->stack, &second_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_jbe: ERROR during stack_pop(second_elem)\n");
        return error_code;
    }

    if (first_elem <= second_elem) {
        cpu->pc = line_num - 1;
    }

    return SUCCESS;
}


error_code_e processor_jne(cpu_s* const cpu, size_t line_num) {
    assert(cpu != NULL);
    error_code_e error_code = INIT_VALUE;

    if (cpu->stack.size < 2) {
        log_print_error(INCORRECT_SIZE, "processor_ja: ERROR stack->size < 2\n");
        return INCORRECT_SIZE;
    }

    stack_element  first_elem = 0;
    stack_element second_elem = 0;

    error_code = stack_pop(&cpu->stack,  &first_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_ja: ERROR during stack_pop(first_elem)\n");
        return error_code;
    }

    error_code = stack_pop(&cpu->stack, &second_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_ja: ERROR during stack_pop(second_elem)\n");
        return error_code;
    }

    if (fabs(first_elem - second_elem) > EPSILON) {
        cpu->pc = line_num - 1;
    }

    return SUCCESS;
}


error_code_e processor_cal(cpu_s* const cpu, const size_t line_num) {
    assert(cpu != NULL);

    stack_push(&cpu->ret_ln, cpu->pc + 1);
    cpu->pc = line_num - 1;

    return SUCCESS;
}


error_code_e processor_ret(cpu_s* const cpu) {
    assert(cpu != NULL);
    double value = 0;

    stack_pop(&cpu->ret_ln, &value);
    cpu->pc = (size_t)value - 1;

    return SUCCESS;
}


error_code_e processor_hlc(cpu_s* const cpu) {
    assert(cpu != NULL);

    error_code_e error_code = processor_destroy(cpu);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_hlc: ERROR during stack destroy\n");
        return error_code;
    }

    return SUCCESS;
}


error_code_e processor_destroy(cpu_s* const cpu) {
    assert(cpu != NULL);

    error_code_e error_code = stack_destroy(&cpu->stack);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_hlc: ERROR during stack destroy\n");
        return error_code;
    }

    error_code = stack_destroy(&cpu->ret_ln);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_hlc: ERROR during ret_ln destroy\n");
        return error_code;
    }

    free(cpu->registers);
    free(cpu->commands);
    cpu->pc            = 0;
    cpu->commands_size = 0;

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
        fprintf(log_file, "\t\t[%zu] = %" SCNu8, ind, cpu->commands[ind].command);

        if ((PUSH <= cpu->commands[ind].command && cpu->commands[ind].command <= POP) || (JMP <= cpu->commands[ind].command && cpu->commands[ind].command <= CAL)) {
            fprintf(log_file, " %lg", cpu->commands[ind].value_1);
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


error_code_e stat_get_size(const char* const file_name, size_t* const file_size) {
    struct stat buff = {};

    if (stat(file_name, &buff) == -1) {
        log_print_error(STAT_READ_ERROR, "STAT\n");
        return STAT_READ_ERROR;
    }

    *file_size = (size_t)buff.st_size;

    return SUCCESS;
}
