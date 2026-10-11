#include "../includes/debugging.h"
#include <math.h>

const char* const bytecode_file_name = "bytecode";
const size_t      MAX_COMMAND_LEN    = 15;

struct asm_context_s {
    const char* input_file_name;
    ssize_t*    labels_table;
};

error_code_e make_bytecode         (asm_context_s* const context);
error_code_e write_in_bytecode_file(const int            command, const double num);
error_code_e clear_bytecode_file   ();
error_code_e context_init          (asm_context_s* const context, const char* const input_file_name);


int main(int argc, char* argv[]) {

    if (argc == 1) {
        printf(COLOR_TEXT("The compiled file is not specified\n", RED));
        return 1;
    }

    asm_context_s context = {};
    context_init(&context, argv[1]);

    clear_bytecode_file();

    error_code_e error_code = make_bytecode(&context);
    if (error_code != SUCCESS) {
        PRINT_ERROR(error_code);
        return error_code;
    }

    return 0;
}


error_code_e make_bytecode(asm_context_s* context) {
    char         command[MAX_COMMAND_LEN] = "";
    char         line[MAX_COMMAND_LEN]    = "";
    double       num                      = NAN;
    char         reg[MAX_COMMAND_LEN]     = "";
    int          count_reads              = 0;
    error_code_e error_code               = INIT_VALUE;

    FILE* input_file = fopen(context->input_file_name, "r");

    if (input_file == NULL) {
        log_print_error(ERROR_DURING_OPEN, "make_bytecode: ERROR during open\n");
        return ERROR_DURING_OPEN;
    }

    count_reads = fscanf(input_file, "%s", command);

    while (count_reads) {
        if        (strncmp(command, "push",     MAX_COMMAND_LEN) == 0) {

            fscanf(input_file, "%lg", &num);

            error_code = write_in_bytecode_file(1, num);
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

            fscanf(input_file, "%s", reg);

            if (strncmp(reg, "rax", MAX_COMMAND_LEN) == 0) {
                error_code = write_in_bytecode_file(11, 1);

            } else if (strncmp(reg, "rbx", MAX_COMMAND_LEN) == 0) {
                error_code = write_in_bytecode_file(11, 2);

            } else if (strncmp(reg, "rcx", MAX_COMMAND_LEN) == 0) {
                error_code = write_in_bytecode_file(11, 3);

            } else if (strncmp(reg, "rdx", MAX_COMMAND_LEN) == 0) {
                error_code = write_in_bytecode_file(11, 4);

            } else {
                fprintf(stderr, "Incorrect command: %s\n", line);
                break;
            }

            if (error_code != SUCCESS) {
                fclose(input_file);
                log_print_error(error_code, "processor_execute_commands: ERROR during write pop\n");
                return error_code;
            }

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

    if (command == 1 || command == 11) {
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


error_code_e context_init(asm_context_s* const context, const char* const input_file_name) {
    assert(context != NULL);

    context->input_file_name = input_file_name;


    return SUCCESS;
}

