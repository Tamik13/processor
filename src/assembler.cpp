#include "../includes/debugging.h"
#include "../includes/commands.h"
#include <math.h>
#include <fcntl.h>

#define INCORRECT_COMMAND(cmd, line)  fprintf(stderr, COLOR_TEXT("INCORRECT COMMAND ON",  RED_C) " %zu: <%s>\n", line, cmd);
#define INCORRECT_REGISTER(reg, line) fprintf(stderr, COLOR_TEXT("INCORRECT REGISTER ON", RED_C) " %zu: <%s>\n", line, reg);
#define INCORRECT_LABEL(label, line)  fprintf(stderr, COLOR_TEXT("INCORRECT LABEL ON",    RED_C) " %zu: <%s>\n", line, label);

const char* const bytecode_file_name = "bytecode";
const size_t      MAX_COMMAND_LEN    = 30;
const double      EPSILON  = 1e-6;

struct label_s {
    char*   name;
    ssize_t line;
};

struct context_s {
    const char* input_file_name;
    label_s*    labels_table;
    size_t      size;
    size_t      capacity;
};

struct asm_cmd_s {
    const char*   name;
    unsigned long hash;
    size_t        number;
};

error_code_e make_bytecode         (context_s* const context, const asm_cmd_s* const commands);
error_code_e write_in_bytecode_file(const command_s  command, const double num);
error_code_e clear_bytecode_file   ();

error_code_e context_init     (context_s* const  context, const char* const input_file_name);
error_code_e labels_table_init(context_s* const  context);
error_code_e context_push     (context_s*        context, label_s           label);
ssize_t      find_label       (context_s* const  context, const char* const name);
ssize_t      get_register_num (const char* const name);
void         comands_init     (asm_cmd_s* const  commands);

size_t bin_search(const asm_cmd_s* const commands, const unsigned long hash);
int    cmd_cmp   (const asm_cmd_s* first_elem, const asm_cmd_s *second_elem);
char*  int_to_str(const int        num);

int main(int argc, char* argv[]) {

    if (argc == 1) {
        printf(COLOR_TEXT("The compiled file is not specified\n", RED_C));
        return 1;
    }

    start_logs();

    context_s    context = {};
    context_init(&context, argv[1]);

    asm_cmd_s commands[COMMANDS_CNT] = {};
    comands_init(commands);

    qsort(commands, COMMANDS_CNT, sizeof(asm_cmd_s), (int(*) (const void *, const void *))cmd_cmp);

    error_code_e error_code = make_bytecode(&context, commands);
    if (error_code != SUCCESS) {
        return error_code;
    }

    return 0;
}



error_code_e make_bytecode(context_s* context, const asm_cmd_s* const commands) {
    char          command[MAX_COMMAND_LEN] = "";
    char          buff[MAX_COMMAND_LEN]    = "";
    error_code_e  error_code               = INIT_VALUE;
    double        val                      = NAN;
    size_t        line                     = 0;

    error_code = clear_bytecode_file();
    if (error_code != SUCCESS) {
        log_print_error(error_code, "make_bytecode: ERROR during clear\n");
        return error_code;
    }

    FILE* input_file = fopen(context->input_file_name, "r");

    if (input_file == NULL) {
        log_print_error(ERROR_DURING_OPEN, "make_bytecode: ERROR during open\n");
        return ERROR_DURING_OPEN;
    }



    while (fscanf(input_file, "%s", command) == 1) {
        line++;
        if (command[0] == ':') continue;

        unsigned long hash = djb2_str_hash((const unsigned char*)command);
        command_s     cmd  = {};
        cmd.num            = (unsigned char)bin_search(commands, hash);

        if (cmd.num == 0) {
            INCORRECT_COMMAND(command, line)
            return INCORRECT_PARAM;
        }

        if (cmd.num == PUSH || cmd.num == POP) {

            if (fscanf(input_file, "%lg", &val) == 1) {
                cmd.konst = 1;
            } else if (fscanf(input_file, "[%29[^]]]", buff) > 0) {
                val     = get_register_num(buff);
                cmd.ram = 1;

                if (fabs(val + 1) < EPSILON) {
                    INCORRECT_REGISTER(buff, line)
                    return INCORRECT_SIZE;
                }

            } else if (fscanf(input_file, "%s", buff) > 0) {

                val     = get_register_num(buff);
                cmd.reg = 1;

                if (fabs(val + 1) < EPSILON) {
                    INCORRECT_REGISTER(buff, line)
                    return INCORRECT_SIZE;
                }

            } else {
                INCORRECT_COMMAND(command, line)
                return INCORRECT_PARAM;
            }

        } else if (JMP <= cmd.num && cmd.num <= CAL) {

            fscanf(input_file, "%s",  buff);
            val = find_label(context, buff);

            if (fabs(val + 1) < EPSILON) {
                INCORRECT_LABEL(buff, line)
                return INCORRECT_SIZE;
            }

        } else {
            val = NAN;
        }

        error_code = write_in_bytecode_file(cmd, val);
        if (error_code != SUCCESS) {
            fclose(input_file);
            log_print_error(error_code, "processor_execute_commands: ERROR during write push\n");
            return error_code;
        }
    }

    if (fclose(input_file)) {
        log_print_error(ERROR_DURING_CLOSE, "processor_execute_commands: ERROR during close\n");
        return ERROR_DURING_CLOSE;
    }

    return SUCCESS;
}


error_code_e write_in_bytecode_file(const command_s command, const double num) {

    int bytecode_file = open(bytecode_file_name, O_WRONLY | O_CREAT |  O_APPEND, S_IREAD | S_IWRITE);

    if (bytecode_file == -1) {
        log_print_error(ERROR_DURING_OPEN, "write_in_bytecode_file: ERROR during open bytecode_file\n");
        return ERROR_DURING_OPEN;
    }

    if (!isnan(num)) {
        write(bytecode_file, &command, sizeof(command)); // TODO: check errors
        write(bytecode_file, &num,     sizeof(num));
    } else {
        write(bytecode_file, &command, sizeof(command));
    }

    if (close(bytecode_file) == -1) {
        log_print_error(ERROR_DURING_CLOSE, "write_in_bytecode_file: ERROR during close bytecode_file\n");
        return ERROR_DURING_CLOSE;
    }

    return SUCCESS;
}


error_code_e clear_bytecode_file() {
    int bytecode_file = open(bytecode_file_name, O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
    const char header[] = "bin file with bytecode";

    if (bytecode_file == -1) {
        log_print_error(ERROR_DURING_OPEN, "write_in_bytecode_file: ERROR during open bytecode_file\n");
        return ERROR_DURING_OPEN;
    }

    write(bytecode_file, &header, sizeof(header));

    if (close(bytecode_file) == -1) {
        log_print_error(ERROR_DURING_CLOSE, "write_in_bytecode_file: ERROR during close bytecode_file\n");
        return ERROR_DURING_CLOSE;
    }

    return SUCCESS;
}


error_code_e context_init(context_s* const context, const char* const input_file_name) {
    assert(context != NULL);

    context->input_file_name = input_file_name;
    context->labels_table    = (label_s*)calloc(2, sizeof(label_s));
    context->size            = 0;
    context->capacity        = 2;

    if (context->labels_table == NULL) {
        log_print_error(ALLOCATION_ERROR, "context_init: ERROR during allocation\n");
        return ALLOCATION_ERROR;
    }

    labels_table_init(context);

    return SUCCESS;
}


error_code_e labels_table_init(context_s* const context) {
    assert(context != NULL);
    char         line[MAX_COMMAND_LEN] = "";
    label_s      new_label             = {};
    ssize_t      line_num              = 0;

    new_label.name = (char*)calloc(MAX_COMMAND_LEN, sizeof(char));

    if (new_label.name == NULL) {
        log_print_error(ALLOCATION_ERROR, "labels_table_init: ERROR during allocation\n");
        return ALLOCATION_ERROR;
    }

    FILE* input_file = fopen(context->input_file_name, "r");
    if (input_file == NULL) {
        log_print_error(ERROR_DURING_OPEN, "labels_table_init: ERROR during open\n");
        return ERROR_DURING_OPEN;
    }

    fgets(line, MAX_COMMAND_LEN, input_file);
    int is_read = sscanf(line, "%s", new_label.name);

    while (!feof(input_file)) {

        if (is_read == 1 && new_label.name[0] == ':') {
            new_label.line = line_num;
            context_push(context, new_label);
        } else if (is_read == 1) {
            line_num++;
        }

        fgets(line, MAX_COMMAND_LEN, input_file);
        is_read = sscanf(line, "%s", new_label.name);
    }


    if (fclose(input_file) == EOF) {
        log_print_error(ERROR_DURING_CLOSE, "labels_table_init: ERROR during close\n");
        return ERROR_DURING_CLOSE;
    }

    return SUCCESS;
}


error_code_e context_push(context_s* context, label_s label) {
    assert(context != NULL);

    if (context->capacity == context->size) {
        label_s* new_table = (label_s*)realloc(context->labels_table, context->capacity * 2 * sizeof(label_s));
        if (new_table == NULL) {
            log_print_error(ALLOCATION_ERROR, "context_push: ERROR during allocation\n");
            return ALLOCATION_ERROR;
        }

        context->labels_table = new_table;
        context->capacity    *= 2;
    }

    context->labels_table[context->size] = label;
    context->labels_table[context->size].name = strdup(label.name);
    context->size++;

    if (context->labels_table[context->size - 1].name == NULL) {
        log_print_error(ALLOCATION_ERROR, "context_push: ERROR during strdup\n");
        return ALLOCATION_ERROR;
    }

    return SUCCESS;
}


ssize_t find_label(context_s* const context, const char* const name) {
    assert(context != NULL);
    assert(name    != NULL);

    for (size_t ind = 0; ind < context->size; ind++) {
        if (strcmp(context->labels_table[ind].name, name) == 0) {
            return context->labels_table[ind].line;
        }
    }

    return -1;
}


ssize_t get_register_num (const char* const name) {
    assert(name != NULL);

    if        (strncmp(name, "rax", MAX_COMMAND_LEN) == 0) {
        return 1;

    } else if (strncmp(name, "rbx", MAX_COMMAND_LEN) == 0) {
        return 2;

    } else if (strncmp(name, "rcx", MAX_COMMAND_LEN) == 0) {
        return 3;

    } else if (strncmp(name, "rdx", MAX_COMMAND_LEN) == 0) {
        return 4;

    } else {
        return -1;
    }
}


void comands_init(asm_cmd_s* const  commands) {
    assert(commands != NULL);

    const char* const names[COMMANDS_CNT] =  {"nac", "push",
    "pop", "add", "mul", "div", "out", "in", "sin",
    "cos", "sqr", "jmp", "ja",  "jb",  "je", "jae",
    "jbe", "jne", "cal", "ret", "dmp", "hlc"};

    for (size_t cmd_num = 0; cmd_num < COMMANDS_CNT; cmd_num++) {
        commands[cmd_num].name   = names[cmd_num];
        commands[cmd_num].number = cmd_num;
        commands[cmd_num].hash   = djb2_str_hash((const unsigned char*)commands[cmd_num].name);
    }
}


size_t bin_search(const asm_cmd_s* const commands, const unsigned long hash) {
    ssize_t l = 0, r = COMMANDS_CNT - 1;

    while (l < r) {
        assert(l <= r);
        ssize_t m = (l + r) / 2;

        if (commands[m].hash == hash) {
            return (size_t)commands[m].number;
        } else if (commands[m].hash < hash) {
            l = m + 1;
        } else {
            r = m - 1;
        }
    }

    if (commands[l].hash == hash) {
        return (size_t)commands[l].number;
    }

    return 0;
}


int cmd_cmp(const asm_cmd_s* first_elem, const asm_cmd_s* second_elem) {
    assert(first_elem  != NULL);
    assert(second_elem != NULL);

    return (first_elem->hash > second_elem->hash) - (first_elem->hash < second_elem->hash);
}
