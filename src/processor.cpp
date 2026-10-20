#include "../includes/stack.h"
#include "../includes/commands.h"
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "raylib.h"

const double EPSILON         = 1e-6;
const size_t COUNT_REGISTERS = 8;
const size_t HEADER_SIZE     = 23;
const size_t RAM_SIZE        = 121;

struct cmd_val_s {
    command_s command;
    double    value;
};

struct cpu_s {
    stack_s    stack;
    stack_s    ret_ln;
    double*    registers;
    size_t     pc;
    cmd_val_s* commands;
    size_t     commands_size;
    unsigned char* ram;
};

typedef error_code_e(cmd_fun_t)(cpu_s* const);

struct cmd_to_fun_s {
    cmd_fun_t* const fun;
};

error_code_e processor_start           (cpu_s* const cpu);
error_code_e processor_load_program    (cpu_s* const cpu, const char* const bytecode_file_name);
error_code_e processor_execute_commands(cpu_s* const cpu);
error_code_e processor_push            (cpu_s* const cpu);
error_code_e processor_pop             (cpu_s* const cpu);
error_code_e processor_add             (cpu_s* const cpu);
error_code_e processor_mul             (cpu_s* const cpu);
error_code_e processor_div             (cpu_s* const cpu);
error_code_e processor_sqrt            (cpu_s* const cpu);
error_code_e processor_sin             (cpu_s* const cpu);
error_code_e processor_cos             (cpu_s* const cpu);
error_code_e processor_out             (cpu_s* const cpu);
error_code_e processor_in              (cpu_s* const cpu);
error_code_e processor_jmp             (cpu_s* const cpu);
error_code_e processor_jb              (cpu_s* const cpu);
error_code_e processor_ja              (cpu_s* const cpu);
error_code_e processor_je              (cpu_s* const cpu);
error_code_e processor_jae             (cpu_s* const cpu);
error_code_e processor_jbe             (cpu_s* const cpu);
error_code_e processor_jne             (cpu_s* const cpu);
error_code_e processor_cal             (cpu_s* const cpu);
error_code_e processor_ret             (cpu_s* const cpu);
error_code_e processor_dmp             (cpu_s* const cpu);
error_code_e processor_hlc             (cpu_s* const cpu);
error_code_e incorrect_cmd             (cpu_s* const cpu);

void         log_dump_cpu     (cpu_s* const cpu,       const char* const reason);
void         dump_ram         (cpu_s* const cpu);
error_code_e processor_destroy(cpu_s* const cpu);
error_code_e stat_get_size    (const char*  const file_name, size_t* const file_size);

const cmd_to_fun_s cmd_funcs[] = {{incorrect_cmd}, {processor_push}, {processor_pop},
                                  {processor_add}, {processor_mul},  {processor_div},      {processor_out},
                                  {processor_in},  {processor_sin},  {processor_cos},      {processor_sqrt},
                                  {processor_jmp}, {processor_ja},   {processor_jb},       {processor_je},
                                  {processor_jae}, {processor_jbe},  {processor_jne},      {processor_cal},
                                  {processor_ret}, {processor_dmp},  {processor_hlc}};

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
    cpu->ram           = (unsigned char*)calloc(RAM_SIZE, sizeof(unsigned char));

    if (cpu->registers == NULL) {
        log_print_error(ALLOCATION_ERROR, "processor_start: ERROR during calloc registers\n");
        return ALLOCATION_ERROR;
    }

    if (cpu->ram == NULL) {
        log_print_error(ALLOCATION_ERROR, "processor_start: ERROR during calloc ram\n");
        return ALLOCATION_ERROR;
    }

    return SUCCESS;
}


error_code_e processor_load_program(cpu_s* const cpu, const char* const bytecode_file_name) {
    command_s    command    = {};
    double       value      = NAN;
    size_t       file_size  = 0;
    error_code_e error_code = INIT_VALUE;
    char         buffer[HEADER_SIZE]   = "";

    error_code = stat_get_size(bytecode_file_name, &file_size);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_load_program: ERROR during get_stat_size\n");
        return error_code;
    }

    if (file_size == 0) {
        log_print_error(ALLOCATION_ERROR, "processor_load_program: ERROR void file\n");
        return ALLOCATION_ERROR;
    }

    cpu->commands = (cmd_val_s*)calloc(file_size, sizeof(cmd_val_s));

    if (cpu->commands == NULL) {
        log_print_error(ALLOCATION_ERROR, "processor_load_program: ERROR during allocation\n");
        return ALLOCATION_ERROR;
    }

    int bytecode_file = open(bytecode_file_name, O_RDONLY);
    if (bytecode_file == -1) {
        log_print_error(ERROR_DURING_OPEN, "processor_load_program: ERROR during open\n");
        return ERROR_DURING_OPEN;
    }

    ssize_t bytes_read = read(bytecode_file, buffer, HEADER_SIZE);

    while (cpu->commands_size < file_size && (bytes_read = read(bytecode_file, &command, sizeof(command))) ) {
        if (bytes_read == -1) {
            log_print_error(ERROR_DURING_READ, "processor_load_program: ERROR during read\n");
            return ERROR_DURING_READ;
        }


        cpu->commands[cpu->commands_size].command = command;

        if ((PUSH <= command.num && command.num <= POP) || (JMP <= command.num && command.num <= CAL)) {
            if (read(bytecode_file, &value, sizeof(value)) <= 0) {
                PRINT_ERROR(INCORRECT_PARAM);
                return INCORRECT_PARAM;
            }
            cpu->commands[cpu->commands_size].value = value;
        }

        cpu->commands_size++;
    }

    if (close(bytecode_file) == -1) {
        log_print_error(ERROR_DURING_CLOSE, "processor_load_program: ERROR during close\n");
        return ERROR_DURING_CLOSE;
    }

    cmd_val_s* reduced_commands = (cmd_val_s*)realloc(cpu->commands, sizeof(cmd_val_s) * cpu->commands_size);
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
        log_dump_cpu(cpu, "");

        if (cpu->commands[cpu->pc].command.num == HLC) {
            break;
        }

        error_code = cmd_funcs[(size_t)cpu->commands[cpu->pc].command.num].fun(cpu);
        if (error_code != SUCCESS) {
            log_print_error(error_code, "processor_execute_commands: ERROR during push\n");
            return error_code;
        }
    }

    return SUCCESS;
}


error_code_e processor_push(cpu_s* const cpu) {
    error_code_e error_code = INIT_VALUE;
    double       value      = 0;

    if (cpu == NULL) {
        log_print_error(NULL_PARAM, "add: ERROR null stack ptr\n");
        return NULL_PARAM;
    }

    if (cpu->commands[cpu->pc].command.konst) {
        value += cpu->commands[cpu->pc].value;
    }

    if (cpu->commands[cpu->pc].command.reg) {
        value += cpu->registers[(size_t)cpu->commands[cpu->pc].value];
    }

    if (cpu->commands[cpu->pc].command.ram) {
        value += cpu->ram[(size_t)cpu->commands[cpu->pc].value];
    }

    error_code = stack_push(&cpu->stack, value);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_push: ERROR during stack_push\n");
        return error_code;
    }

    return SUCCESS;
}


error_code_e processor_pop(cpu_s* const cpu) {
    error_code_e error_code = INIT_VALUE;
    double value = NAN;

    error_code = stack_verify(&cpu->stack);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_pop: ERROR during stack_verify\n");
        return error_code;
    }

    if (cpu->commands[cpu->pc].command.konst) {
        if (cpu->stack.size < 1) {
            log_print_error(INCORRECT_SIZE, "processor_pop: ERROR stack->size < 2\n");
            return INCORRECT_SIZE;
        }

        error_code = stack_pop(&cpu->stack, &value);
        if (error_code != SUCCESS) {
            log_print_error(error_code, "processor_pop: ERROR during stack_pop\n");
        }
    }

    if (cpu->commands[cpu->pc].command.reg) {
        if (cpu->stack.size < 1) {
            log_print_error(INCORRECT_SIZE, "processor_pop: ERROR stack->size < 2\n");
            return INCORRECT_SIZE;
        }

        error_code = stack_pop(&cpu->stack, &value);
        if (error_code != SUCCESS) {
            log_print_error(error_code, "processor_pop: ERROR during stack_pop\n");
        }
        cpu->registers[(size_t)cpu->commands[cpu->pc].value] = value;
    }

    if (cpu->commands[cpu->pc].command.ram) {

        if (cpu->stack.size < 1) {
            log_print_error(INCORRECT_SIZE, "processor_pop: ERROR stack->size < 2\n");
            return INCORRECT_SIZE;
        }

        error_code = stack_pop(&cpu->stack, &value);
        if (error_code != SUCCESS) {
            log_print_error(error_code, "processor_pop: ERROR during stack_pop\n");
        }
        cpu->ram[(size_t)cpu->registers[(size_t)cpu->commands[cpu->pc].value]] = (unsigned char)value;
    }

    return SUCCESS;
}


error_code_e processor_add(cpu_s* const cpu) {
    error_code_e error_code = INIT_VALUE;

    error_code = stack_verify(&cpu->stack);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_add: ERROR during stack_verify\n");
        return error_code;
    }

    if (cpu->stack.size < 2) {
        log_print_error(INCORRECT_SIZE, "processor_add: ERROR stack->size < 2\n");
        return INCORRECT_SIZE;
    }

    stack_element  first_elem = 0;
    stack_element second_elem = 0;

    error_code = stack_pop(&cpu->stack,  &first_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_add: ERROR during stack_pop(first_elem)\n");
        return error_code;
    }

    error_code = stack_pop(&cpu->stack, &second_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_add: ERROR during stack_pop(second_elem)\n");
        return error_code;
    }

    error_code = stack_push(&cpu->stack, first_elem + second_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code,  "processor_add: ERROR during stack_push(first_elem + second_elem)\n");
    }

    return SUCCESS;
}


error_code_e processor_mul(cpu_s* const cpu) {
    error_code_e error_code = INIT_VALUE;

    error_code = stack_verify(&cpu->stack);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_multiplying: ERROR during stack_verify\n");
        return error_code;
    }

    if (cpu->stack.size < 2) {
        log_print_error(INCORRECT_SIZE, "processor_multiplying: ERROR stack->size < 2\n");
        return INCORRECT_SIZE;
    }

    stack_element  first_elem = 0;
    stack_element second_elem = 0;

    error_code = stack_pop(&cpu->stack,  &first_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_multiplying: ERROR during stack_pop(first_elem)\n");
        return error_code;
    }

    error_code = stack_pop(&cpu->stack, &second_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_multiplying: ERROR during stack_pop(second_elem)\n");
        return error_code;
    }

    error_code = stack_push(&cpu->stack, first_elem * second_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code,  "processor_multiplying: ERROR during stack_push(first_elem * second_elem)\n");
    }

    return SUCCESS;
}


error_code_e processor_div(cpu_s* const cpu) {
    error_code_e error_code = INIT_VALUE;

    error_code = stack_verify(&cpu->stack);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_div: ERROR during stack_verify\n");
        return error_code;
    }

    if (cpu->stack.size < 2) {
        log_print_error(INCORRECT_SIZE, "processor_div: ERROR stack->size < 2\n");
        return INCORRECT_SIZE;
    }

    stack_element  first_elem = 0;
    stack_element second_elem = 0;

    error_code = stack_pop(&cpu->stack,  &first_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_div: ERROR during stack_pop(first_elem)\n");
        return error_code;
    }

    error_code = stack_pop(&cpu->stack, &second_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_div: ERROR during stack_pop(second_elem)\n");
        return error_code;
    }

    if (fabs(first_elem) <= EPSILON) {
        return DIVISION_BY_ZERO;
    }

    error_code = stack_push(&cpu->stack,  second_elem / first_elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code,  "processor_div: ERROR during stack_push(first_elem / second_elem)\n");
    }

    return SUCCESS;
}


error_code_e processor_sqrt(cpu_s* const cpu) {
    error_code_e error_code = INIT_VALUE;

    error_code = stack_verify(&cpu->stack);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_sqrt: ERROR during stack_verify\n");
        return error_code;
    }

    if (cpu->stack.size < 1) {
        log_print_error(INCORRECT_SIZE, "processor_sqrt: ERROR stack->size < 1\n");
        return INCORRECT_SIZE;
    }

    stack_element elem = 0;

    error_code = stack_pop(&cpu->stack,  &elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_sqrt: ERROR during stack_pop(first_elem)\n");
        return error_code;
    }

    error_code = stack_push(&cpu->stack, sqrt(elem));
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_sqrt: ERROR during stack_push(sqrt(elem))\n");
    }

    return SUCCESS;
}


error_code_e processor_sin(cpu_s* const cpu) {
    error_code_e error_code = INIT_VALUE;

    error_code = stack_verify(&cpu->stack);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_sin: ERROR during stack_verify\n");
        return error_code;
    }

    if (cpu->stack.size < 1) {
        log_print_error(INCORRECT_SIZE, "processor_sin: ERROR stack->size < 1\n");
        return INCORRECT_SIZE;
    }

    stack_element elem = 0;

    error_code = stack_pop(&cpu->stack,  &elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_sin: ERROR during stack_pop(first_elem)\n");
        return error_code;
    }

    error_code = stack_push(&cpu->stack, sin(elem));
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_sin: ERROR during stack_push(sin(elem))\n");
    }

    return SUCCESS;
}


error_code_e processor_cos(cpu_s* const cpu) {
    error_code_e error_code = INIT_VALUE;

    error_code = stack_verify(&cpu->stack);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_cos: ERROR during stack_verify\n");
        return error_code;
    }

    if (cpu->stack.size < 1) {
        log_print_error(INCORRECT_SIZE, "processor_cos: ERROR stack->size < 1\n");
        return INCORRECT_SIZE;
    }

    stack_element elem = 0;

    error_code = stack_pop(&cpu->stack,  &elem);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_cos: ERROR during stack_pop(first_elem)\n");
        return error_code;
    }

    error_code = stack_push(&cpu->stack, cos(elem));
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_cos: ERROR during stack_push(cos(elem))\n");
    }

    return SUCCESS;
}


error_code_e processor_out(cpu_s* const cpu) {

    error_code_e error_code = stack_verify(&cpu->stack);
    if (error_code != SUCCESS) {
        log_print_error(error_code, "processor_out: ERROR during stack verify\n");
        return error_code;
    }

    if (cpu->stack.size < 1) {
        printf("Stack is void\n");
        return SUCCESS;
    }

    printf("%lg\n", cpu->stack.data[cpu->stack.size - 1]);

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


error_code_e processor_jmp(cpu_s* const cpu) {
    assert(cpu != NULL);

    cpu->pc = (size_t)cpu->commands[cpu->pc].value - 1;

    return SUCCESS;
}


error_code_e processor_jb(cpu_s* const cpu) {
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
        cpu->pc =  (size_t)cpu->commands[cpu->pc].value - 1;
    }

    return SUCCESS;
}


error_code_e processor_ja(cpu_s* const cpu) {
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
        cpu->pc = (size_t)cpu->commands[cpu->pc].value - 1;
    }

    return SUCCESS;
}


error_code_e processor_je(cpu_s* const cpu) {
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
        cpu->pc = (size_t)cpu->commands[cpu->pc].value - 1;
    }

    return SUCCESS;
}


error_code_e processor_jae(cpu_s* const cpu) {
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
        cpu->pc = (size_t)cpu->commands[cpu->pc].value - 1;
    }

    return SUCCESS;
}


error_code_e processor_jbe(cpu_s* const cpu) {
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
        cpu->pc = (size_t)cpu->commands[cpu->pc].value - 1;
    }

    return SUCCESS;
}


error_code_e processor_jne(cpu_s* const cpu) {
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
        cpu->pc = (size_t)cpu->commands[cpu->pc].value - 1;
    }

    return SUCCESS;
}


error_code_e processor_cal(cpu_s* const cpu) {
    assert(cpu != NULL);

    error_code_e error_code = stack_push(&cpu->ret_ln, cpu->pc + 1);
    if (error_code != SUCCESS) {
        return error_code;
    }
    cpu->pc = (size_t)cpu->commands[cpu->pc].value - 1;

    return SUCCESS;
}


error_code_e processor_ret(cpu_s* const cpu) {
    assert(cpu != NULL);
    double value = 0;

    error_code_e error_code = stack_pop(&cpu->ret_ln, &value);
    if (error_code != SUCCESS) {
        return error_code;
    }
    cpu->pc = (size_t)value - 1;

    return SUCCESS;
}


error_code_e processor_dmp(cpu_s* const cpu) {
    assert(cpu != NULL);

    dump_ram(cpu);

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


error_code_e incorrect_cmd(cpu_s* const cpu) {
    assert(cpu != NULL);

    return INCORRECT_PARAM;
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
        fprintf(log_file, "\t\t[%zu] = %d%d%d %d", ind, cpu->commands[ind].command.konst, cpu->commands[ind].command.reg, cpu->commands[ind].command.ram, cpu->commands[ind].command.num);

        if ((PUSH <= cpu->commands[ind].command.num && cpu->commands[ind].command.num <= POP) || (JMP <= cpu->commands[ind].command.num && cpu->commands[ind].command.num <= CAL)) {
            fprintf(log_file, " %lg", cpu->commands[ind].value);
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


void dump_ram(cpu_s* const cpu) {
    const size_t hight  = 11;
    const size_t weight = 11;

    for (size_t hi = 0; hi < hight; hi++) {
        for (size_t we = 0; we < weight; we++) {
            printf("%d", cpu->ram[hi * weight + we]);
        }
        printf("\n");
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
