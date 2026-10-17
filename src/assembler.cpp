#include "../includes/debugging.h"
#include "../includes/commands.h"
#include <math.h>
#include <fcntl.h>

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
error_code_e write_in_bytecode_file(const int        command, const double num);
error_code_e clear_bytecode_file   ();

error_code_e context_init     (context_s* const  context, const char* const input_file_name);
error_code_e labels_table_init(context_s* const  context);
error_code_e context_push     (context_s*        context, label_s           label);
ssize_t      find_label       (context_s* const  context, const char* const name);
ssize_t      get_register_num (const char* const name);
void         comands_init     (asm_cmd_s* const  commands);

ssize_t bin_search(const asm_cmd_s* const commands, const unsigned long hash);
int     cmd_cmp   (const asm_cmd_s* first_elem, const asm_cmd_s *second_elem);

int main(int argc, char* argv[]) {

    if (argc == 1) {
        printf(COLOR_TEXT("The compiled file is not specified\n", RED));
        return 1;
    }

    context_s    context = {};
    context_init(&context, argv[1]);

    asm_cmd_s commands[COMMANDS_CNT] = {};
    comands_init(commands);

    qsort(commands, COMMANDS_CNT, sizeof(asm_cmd_s), (int(*) (const void *, const void *))cmd_cmp);

    error_code_e error_code = make_bytecode(&context, commands);
    if (error_code != SUCCESS) {
        PRINT_ERROR(error_code);
        return error_code;
    }

    return 0;
}



error_code_e make_bytecode(context_s* context, const asm_cmd_s* const commands) {
    char          command[MAX_COMMAND_LEN] = "";
    char          buff[MAX_COMMAND_LEN]    = "";
    error_code_e  error_code               = INIT_VALUE;
    double        num                      = NAN;

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
        if (command[0] == ':') continue;

        unsigned long hash    = djb2_str_hash((const unsigned char*)command);
        ssize_t       cmd_num = bin_search(commands, hash);

        if (cmd_num == PUSH) {
            fscanf(input_file, "%lg", &num);

        } else if (PUR <= cmd_num && cmd_num <= POP) {
            fscanf(input_file, "%s", buff);
            num  =  get_register_num(buff);

            if (fabs(num + 1) < EPSILON) {
                return INCORRECT_SIZE; // TODO: printerror
            }
        } else if (JMP <= cmd_num && cmd_num <= CAL) {
            fscanf(input_file, "%s",  buff);
            num = find_label(context, buff);

            if (fabs(num + 1) < EPSILON) {
                return INCORRECT_SIZE; // TODO: printerror
            }
        } else {
            num = NAN;
        }

        error_code = write_in_bytecode_file((int)cmd_num, num);
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


error_code_e write_in_bytecode_file(const int command, const double num) {

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
        fprintf(stderr, "Incorrect register: <%s>\n", name);
        return -1;
    }
}


void comands_init(asm_cmd_s* const  commands) {
    assert(commands != NULL);

    const char* const names[COMMANDS_CNT] =  {"nac", "push", "pur",
    "pop", "add", "mul", "div", "out", "in", "sin",
    "cos", "sqr", "jmp", "ja",  "jb",  "je", "jae",
    "jbe", "jne", "cal", "ret", "hlc"};

    for (size_t cmd_num = 0; cmd_num < COMMANDS_CNT; cmd_num++) {
        commands[cmd_num].name   = names[cmd_num];
        commands[cmd_num].number = cmd_num;
        commands[cmd_num].hash   = djb2_str_hash((const unsigned char*)commands[cmd_num].name);
    }
}


ssize_t bin_search(const asm_cmd_s* const commands, const unsigned long hash) {
    size_t l = 0, r = COMMANDS_CNT - 1;

    while (l < r) {
        assert(l <= r);
        size_t m = (l + r) / 2;

        if (commands[m].hash == hash) {
            return (ssize_t)commands[m].number;
        } else if (commands[m].hash < hash) {
            l = m + 1;
        } else {
            r = m - 1;
        }
    }

    if (commands[l].hash == hash) {
        return (ssize_t)commands[l].number;
    }

    return -1;
}


int cmd_cmp(const asm_cmd_s* first_elem, const asm_cmd_s* second_elem) {
    assert(first_elem  != NULL);
    assert(second_elem != NULL);

    return (int)(first_elem->hash - second_elem->hash);
}
