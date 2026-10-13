#include "../includes/debugging.h"
#include <math.h>

const char* const bytecode_file_name = "bytecode";
const size_t      MAX_COMMAND_LEN    = 15;

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

error_code_e make_bytecode         (context_s* const context);
error_code_e write_in_bytecode_file(const int        command, const double num);
error_code_e clear_bytecode_file   ();

error_code_e context_init     (context_s* const context, const char* const input_file_name);
error_code_e labels_table_init(context_s* const context);
error_code_e context_push     (context_s*       context, label_s label);
ssize_t      find_label       (context_s* const context, const char* const name);
ssize_t      get_register_num (const char* const name);

int main(int argc, char* argv[]) {

    if (argc == 1) {
        printf(COLOR_TEXT("The compiled file is not specified\n", RED));
        return 1;
    }

    context_s context = {};
    context_init(&context, argv[1]);


    error_code_e error_code = make_bytecode(&context);
    if (error_code != SUCCESS) {
        PRINT_ERROR(error_code);
        return error_code;
    }

    return 0;
}



error_code_e make_bytecode(context_s* context) {
    char         command[MAX_COMMAND_LEN] = "";
    char         line[MAX_COMMAND_LEN]    = "";
    double       num                      = NAN;
    char         arg[MAX_COMMAND_LEN]     = "";
    int          count_reads              = 0;
    error_code_e error_code               = INIT_VALUE;

    clear_bytecode_file();

    FILE* input_file = fopen(context->input_file_name, "r");

    if (input_file == NULL) {
        log_print_error(ERROR_DURING_OPEN, "make_bytecode: ERROR during open\n");
        return ERROR_DURING_OPEN;
    }

    count_reads = fscanf(input_file, "%s", command);

    while (!feof(input_file)) {
        if        (strncmp(command, "push",     MAX_COMMAND_LEN) == 0) {
            int is_read = fscanf(input_file, "%lg", &num);

            if (is_read == 1) {
                error_code = write_in_bytecode_file(1, num);

            } else if (is_read == 0) {
                fscanf(input_file, "%s", line);
                ssize_t reg = get_register_num(line);
                error_code = write_in_bytecode_file(13, reg);
            } else {
                log_print_error(END_OF_FILE, "processor_execute_commands: ERROR EOF\n");
                return END_OF_FILE;
            }

            if (error_code != SUCCESS) {
                fclose(input_file);
                log_print_error(error_code, "processor_execute_commands: ERROR during write push\n");
                return error_code;
            }

        } else if (strncmp(command, "add",      MAX_COMMAND_LEN) == 0) {

            error_code = write_in_bytecode_file(2, NAN);
            if (error_code != SUCCESS) {
                fclose(input_file);
                log_print_error(error_code, "processor_execute_commands: ERROR during write add\n");
                return error_code;
            }

        } else if (strncmp(command, "multiply", MAX_COMMAND_LEN) == 0) {

            error_code = write_in_bytecode_file(3, NAN);
            if (error_code != SUCCESS) {
                fclose(input_file);
                log_print_error(error_code, "processor_execute_commands: ERROR during write multiply\n");
                return error_code;
            }

        } else if (strncmp(command, "div",      MAX_COMMAND_LEN) == 0) {

            error_code = write_in_bytecode_file(4, NAN);
            if (error_code != SUCCESS) {
                fclose(input_file);
                log_print_error(error_code, "processor_execute_commands: ERROR during write div\n");
                return error_code;
            }

        } else if (strncmp(command, "sqrt",     MAX_COMMAND_LEN) == 0) {

            error_code = write_in_bytecode_file(5, NAN);
            if (error_code != SUCCESS) {
                fclose(input_file);
                log_print_error(error_code, "processor_execute_commands: ERROR during write sqrt\n");
                return error_code;
            }

        } else if (strncmp(command, "sin",      MAX_COMMAND_LEN) == 0) {

            error_code = write_in_bytecode_file(6, NAN);
            if (error_code != SUCCESS) {
                fclose(input_file);
                log_print_error(error_code, "processor_execute_commands: ERROR during write sin\n");
                return error_code;
            }

        } else if (strncmp(command, "cos",      MAX_COMMAND_LEN) == 0) {

            error_code = write_in_bytecode_file(7, NAN);
            if (error_code != SUCCESS) {
                fclose(input_file);
                log_print_error(error_code, "processor_execute_commands: ERROR during write cos\n");
                return error_code;
            }

        } else if (strncmp(command, "top",      MAX_COMMAND_LEN) == 0) {

            error_code = write_in_bytecode_file(8, NAN);
            if (error_code != SUCCESS) {
                fclose(input_file);
                log_print_error(error_code, "processor_execute_commands: ERROR during write top\n");
                return error_code;
            }

        } else if (strncmp(command, "dump",     MAX_COMMAND_LEN) == 0) {

            error_code = write_in_bytecode_file(9, NAN);
            if (error_code != SUCCESS) {
                fclose(input_file);
                log_print_error(error_code, "processor_execute_commands: ERROR during write dump\n");
                return error_code;
            }

        } else if (strncmp(command, "hlc",      MAX_COMMAND_LEN) == 0) {

            error_code = write_in_bytecode_file(10, NAN);
            if (error_code != SUCCESS) {
                fclose(input_file);
                log_print_error(error_code, "processor_execute_commands: ERROR during write hlc\n");
                return error_code;
            }
            break;

        } else if (strncmp(command, "pop",      MAX_COMMAND_LEN) == 0) {

            fscanf(input_file, "%s", arg);

            ssize_t reg = get_register_num(arg);
            error_code  = write_in_bytecode_file(11, reg);

            if (error_code != SUCCESS) {
                fclose(input_file);
                log_print_error(error_code, "processor_execute_commands: ERROR during write pop\n");
                return error_code;
            }

        } else if (strncmp(command, "jmp", MAX_COMMAND_LEN) == 0) {

            fscanf(input_file, "%s", arg);
            ssize_t line_num = -1;
            line_num = find_label(context, arg);

            if (line_num == -1) {
                fprintf(stderr, "Incorrect label: %s\n", line);
                break;
            }

            error_code = write_in_bytecode_file(12, line_num);
            if (error_code != SUCCESS) {
                fclose(input_file);
                log_print_error(error_code, "processor_execute_commands: ERROR during write dump\n");
                return error_code;
            }

        } else if (command[0] == ':') {
            count_reads = fscanf(input_file, "%s", command);
            continue;
        } else {
            fprintf(stderr, "Incorrect command: %s\n", line);
            break;
        }

        count_reads = fscanf(input_file, "%s", command);
    }

    fclose(input_file);
    return SUCCESS;
}


error_code_e write_in_bytecode_file(const int command, const double num) {

    FILE* bytecode_file = fopen(bytecode_file_name, "a");

    if (bytecode_file == NULL) {
        log_print_error(ERROR_DURING_OPEN, "write_in_bytecode_file: ERROR during open bytecode_file\n");
        return ERROR_DURING_OPEN;
    }

    if (command == 1 || command == 11 || command == 12 || command == 13) {
        fprintf(bytecode_file ,"%d %lg\n", command, num);
    } else {
        fprintf(bytecode_file ,"%d\n",     command);
    }

    if (fclose(bytecode_file) == EOF) {
        log_print_error(ERROR_DURING_CLOSE, "write_in_bytecode_file: ERROR during close bytecode_file\n");
        return ERROR_DURING_CLOSE;
    }

    return SUCCESS;
}


error_code_e clear_bytecode_file() {
    FILE* bytecode_file = fopen(bytecode_file_name, "w");

    if (bytecode_file == NULL) {
        log_print_error(ERROR_DURING_OPEN, "write_in_bytecode_file: ERROR during open bytecode_file\n");
        return ERROR_DURING_OPEN;
    }

    if (fclose(bytecode_file) == EOF) {
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

    while (!feof(input_file)) {

        if (line[0] == ':') {
            sscanf(line, "%s", new_label.name);
            new_label.line = line_num;
            context_push(context, new_label);
        } else {
            line_num++;
        }

        fgets(line, MAX_COMMAND_LEN, input_file);
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
        label_s* new_labels_table = (label_s*)realloc(context->labels_table, context->capacity * 2 * sizeof(label_s));
        if (new_labels_table == NULL) {
            log_print_error(ALLOCATION_ERROR, "context_push: ERROR during allocation\n");
            return ALLOCATION_ERROR;
        }

        context->labels_table = new_labels_table;
        context->capacity    *= 2;
    }

    context->labels_table[context->size] = label;
    context->size++;

    context->labels_table[context->size].name = strdup(context->labels_table[context->size].name);

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

    if (strncmp(name, "rax", MAX_COMMAND_LEN) == 0) {
        return 1;

    } else if (strncmp(name, "rbx", MAX_COMMAND_LEN) == 0) {
        return 2;

    } else if (strncmp(name, "rcx", MAX_COMMAND_LEN) == 0) {
        return 3;

    } else if (strncmp(name, "rdx", MAX_COMMAND_LEN) == 0) {
        return 4;

    } else {
        fprintf(stderr, "Incorrect register: %s\n", name);
        return -1;
    }
}

