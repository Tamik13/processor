#include "debugging.h"
#include <math.h>

const char* const bytecode_file_name = "bytecode";
const size_t      MAX_COMMAND_LEN    = 15;

error_code_e make_bytecode         (const char* const input_file_name);
error_code_e write_in_bytecode_file(const int         command,        const double num);
error_code_e clear_bytecode_file   ();
