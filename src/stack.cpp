#include "../includes/stack.h"

error_code_e stack_verify(stack_s* const stack) {
    if (stack == NULL) {
        log_print_error(NULL_STACK, "stack_verify: ERROR stack is null\n");
        return NULL_STACK;
    }

    ON_DBG(
    if (is_readble_ptr(stack)) {
        log_print_error(SEG_FAULT, "stack_verify: ERROR stack ptr cant be read\n");
        return SEG_FAULT;
    }
    )

    if (stack->data == NULL) {
        log_print_error(NULL_STACK, "stack_verify: ERROR stack->data is null\n");
        return NULL_STACK;
    }

    ON_DBG(
    if (stack->canary_data == NULL) {
        log_print_error(NULL_STACK, "stack_verify: ERROR stack->canary_data is null\n");
        return NULL_STACK;
    }

    if ((unsigned long long)stack->left_canary != LEFT_CANARY) {
        log_print_error(CANARY_IS_DEAD, "stack_verify: ERROR first canary in struct IS DEAD(((\n");
        log_dump_stack(stack, "");
        return CANARY_IS_DEAD;
    }

    if ((unsigned long long)stack->right_canary != RIGHT_CANARY) {
        log_print_error(CANARY_IS_DEAD, "stack_verify: ERROR second canary in struct IS DEAD(((\n");
        log_dump_stack(stack, "");
        return CANARY_IS_DEAD;
    }

    if (is_readble_ptr(stack->data)) {

        log_print_error(SEG_FAULT, "stack_verify: ERROR stack->data ptr cant be read\n");

        return SEG_FAULT;
    }

    if ((unsigned long long)stack->canary_data[0] != LEFT_CANARY) {
        log_print_error(CANARY_IS_DEAD, "stack_verify: ERROR first canary in data IS DEAD(((\n");
        log_dump_stack(stack, "");
        return CANARY_IS_DEAD;
    }

    if ((unsigned long long)stack->data[stack->capacity] != RIGHT_CANARY) {
        log_print_error(CANARY_IS_DEAD, "stack_verify: ERROR second canary in data IS DEAD(((\n");
        log_dump_stack(stack, "");
        return CANARY_IS_DEAD;
    }
    )

    if (stack->capacity == 0) {
        log_print_error(ZERO_CAPACITY, "stack_verify: ERROR zero capacity\n");
        log_dump_stack(stack, "");
        return ZERO_CAPACITY;
    }

    if (stack->capacity < stack->size) {
        log_print_error(SIZE_HIGHER_CAPACITY, "stack_verify: ERROR size higher capacity\n");
        log_dump_stack(stack,                 "stack_verify: ERROR size higher capacity\n");
        return SIZE_HIGHER_CAPACITY;
    }

    ON_DBG(
    unsigned long old_struct_hash = stack->struct_hash; // TODO: define
    unsigned long old_data_hash   = stack->data_hash;
    stack->       struct_hash     = 0;
    stack->       data_hash       = 0;
    unsigned long new_struct_hash = djb2_hash((const unsigned char*)stack, sizeof(*stack));

    if (old_struct_hash != new_struct_hash) {
        log_print_error(HASH_CHANGED, "stack_verify: ERROR new_hash != old_hash\n");
        log_dump_stack(stack, "");
        return HASH_CHANGED;
    }

    unsigned long new_data_hash   = djb2_hash((const unsigned char*)stack->canary_data, sizeof(stack_element) * (stack->capacity + COUNT_CANARY));
    stack->struct_hash = new_struct_hash;

    if (old_data_hash != new_data_hash) {
        log_print_error(HASH_CHANGED, "stack_verify: ERROR new_hash != old_hash\n");
        log_dump_stack(stack, "");
        return HASH_CHANGED;
    }

    stack->data_hash = new_data_hash;
    )

    return SUCCESS;
}


error_code_e stack_init(stack_s* const stack, const size_t capacity ON_DBG(, const char* const name, const char* const file, const char* const function, const size_t line)) {
    if (capacity == 0) {
        log_print_error(ZERO_CAPACITY, "stack_init: ERROR try to init stack with zero capacity\n");
        return ZERO_CAPACITY;
    }

    if (is_stack_init(stack)) {
        log_print_error(REINITIALIZATION, "stack_init: ERROR try to init already initialized stack\n");
        return REINITIALIZATION;
    }

    stack->data = (stack_element*)calloc(capacity ON_DBG(+ COUNT_CANARY), sizeof(stack_element));

    if (stack->data == NULL) {
        log_print_error(ALLOCATION_ERROR, "stack_init: ERROR during allocation\n");
        return ALLOCATION_ERROR;
    }

    stack->capacity = capacity;
    stack->size     = 0;

    ON_DBG(
        stack->canary_data =  stack->data;
        stack->data        += COUNT_LEFT_CANARY * CANARY_SIZE;
        stack->name        =  name;
        stack->file        =  file;
        stack->function    =  function;
        stack->line        =  line;
    )

    for (size_t ind = 0; ind < stack->capacity; ind++) {
        ASSERT_FOR_ARR(ind, stack->capacity); // TODO malloc_size
        stack->data[ind] = POISON;
    }

    ON_DBG(
    stack->canary_data[0]        = LEFT_CANARY;
    stack->data[stack->capacity] = RIGHT_CANARY;

    stack->left_canary           = LEFT_CANARY;
    stack->right_canary          = RIGHT_CANARY;

    stack->struct_hash = 0;
    stack->struct_hash = djb2_hash((const unsigned char*)stack, sizeof(*stack));

    stack->data_hash = 0;
    stack->data_hash = djb2_hash((const unsigned char*)stack->canary_data, sizeof(stack_element) * (stack->capacity + COUNT_CANARY));

    )

    return SUCCESS;
}


error_code_e stack_push(stack_s* const stack, const stack_element value) {
    error_code_e error_code = INIT_VALUE;

    error_code = stack_verify(stack);
    if (error_code) {
        log_print_error(error_code, "stack_push: ERROR before push during stack_verify\n");

        if (error_code != SEG_FAULT) {
            log_dump_stack (stack,  "stack_push: ERROR before push during stack_verify\n");
        }
        return error_code;
    }

    log_print("stack push\n");

    if (stack->size == stack->capacity) {
        error_code = stack_recalloc(stack, stack->size * HIGHER_COEF);

        if (error_code) {
            log_print_error(error_code, "stack_push: ERROR during recalloc\n");
            return error_code;
        }

        error_code = stack_verify(stack);
        if (error_code) {
            log_print_error(error_code, "stack_push: ERROR after recalloc during stack_verify\n");

            if (error_code != SEG_FAULT) {
                log_dump_stack (stack,  "stack_push: ERROR after recalloc during stack_verify\n");
            }
            return error_code;
        }
    }

    stack->data[stack->size++] = value;

    ON_DBG(
    stack->struct_hash = 0;
    stack->data_hash   = 0;

    stack->struct_hash = djb2_hash((const unsigned char*)stack, sizeof(*stack));
    stack->data_hash   = djb2_hash((const unsigned char*)stack->canary_data, sizeof(stack_element) * (stack->capacity + COUNT_CANARY));
    )

    error_code = stack_verify(stack);
    if (error_code) {
        log_print_error(error_code, "stack_push: ERROR after push during stack_verify\n");

        if (error_code != SEG_FAULT) {
            log_dump_stack (stack,  "stack_push: ERROR after push during stack_verify\n");
        }

        return error_code;
    }


    return SUCCESS;
}


error_code_e stack_pop(stack_s* const stack, stack_element* const value) {
    error_code_e error_code = INIT_VALUE;

    error_code = stack_verify(stack);
    if (error_code) {
        log_print_error(error_code, "stack_pop: ERROR before pop during stack_verify\n");

        if (error_code != SEG_FAULT) {
            log_dump_stack (stack,  "stack_pop: ERROR before pop during stack_verify\n");
        }
        return error_code;
    }

    if (stack->size == 0) {
        log_print_error(POP_VOID_STACK, "stack_pop: ERROR before pop try pop void stack\n");
        log_dump_stack (stack,          "stack_pop: ERROR before pop try pop void stack\n");
        return POP_VOID_STACK;
    }

    if (value == NULL) {
        log_print_error(NULL_PARAM, "stack_push: ERROR null value ptr\n");
        log_dump_stack (stack,      "stack_push: ERROR null value ptr\n");
        return NULL_PARAM;
    }

    log_print("stack pop\n");

    if (stack->size <= stack->capacity / LOWER_COEF && stack->capacity / LOWER_COEF ) {
        error_code = stack_recalloc(stack, stack->capacity / LOWER_COEF);

        if (error_code) {
            log_print_error(error_code, "stack_pop: ERROR during recalloc\n");
            return error_code;
        }

        error_code = stack_verify(stack);
        if (error_code) {
            log_print_error(error_code, "stack_pop: ERROR after recalloc during stack_verify\n");

            if (error_code != SEG_FAULT) {
                log_dump_stack (stack,  "stack_pop: ERROR after recalloc during stack_verify\n");
            }
            return error_code;
        }
    }

    stack->size--;
    *value = stack->data[stack->size];
    stack->data[stack->size] = POISON;

    ON_DBG(
    stack->struct_hash = 0;
    stack->data_hash   = 0;

    stack->struct_hash = djb2_hash((const unsigned char*)stack, sizeof(*stack));
    stack->data_hash   = djb2_hash((const unsigned char*)stack->canary_data, sizeof(stack_element) * (stack->capacity + COUNT_CANARY));
    )

    error_code = stack_verify(stack);
    if (error_code) {
        log_print_error(error_code, "stack_pop: ERROR after pop during stack_verify\n");

        if (error_code != SEG_FAULT) {
            log_dump_stack (stack,  "stack_pop: ERROR after pop during stack_verify\n");
        }
        return error_code;
    }

    return SUCCESS;
}


error_code_e stack_destroy(stack_s* const stack) {
    error_code_e error_code = stack_verify(stack);

    if (error_code == CANARY_IS_DEAD) {
        log_print_error(error_code, "stack_destroy: ERROR possible damage to the stack, including data. Stack cant be destroy\n");
        return error_code;
    }

    free(stack->data ON_DBG( - COUNT_LEFT_CANARY * CANARY_SIZE));

    stack->data       = NULL;
    stack->capacity   = 0;
    stack->size       = 0;

    ON_DBG(
        stack->canary_data = NULL;
        stack->struct_hash = 0;
        stack->data_hash   = 0;
        stack->name        = NULL;
        stack->file        = NULL;
        stack->function    = NULL;
        stack->line        = 0;
    )

    return error_code;
}


error_code_e stack_recalloc(stack_s* const stack, const size_t new_capacity) {
    error_code_e error_code = INIT_VALUE;

    error_code = stack_verify(stack);
    if (error_code) {
        log_print_error(error_code, "stack_recalloc: ERROR before recalloc during stack_verify\n");
        return error_code;
    }

    stack_element* new_canary_data = (stack_element*)realloc((void*)(stack->data ON_DBG( - COUNT_LEFT_CANARY * CANARY_SIZE)), (new_capacity ON_DBG(+ COUNT_CANARY)) * sizeof(stack_element));

    if (new_canary_data == NULL) {
        log_print_error(error_code, "stack_recalloc: ERROR during recalloc\n");
        return ALLOCATION_ERROR;
    }

    stack->data = new_canary_data;

    ON_DBG(
        stack->canary_data        = new_canary_data;
        stack->data               += COUNT_LEFT_CANARY * CANARY_SIZE;
        stack->canary_data[0]     = LEFT_CANARY;
        stack->data[new_capacity] = RIGHT_CANARY;
    )

    for (size_t ind = stack->size; ind < new_capacity; ind++) {
        ASSERT_FOR_ARR(ind, new_capacity);
        stack->data[ind] = POISON;
    }

    stack->capacity = new_capacity;

    ON_DBG(
    stack->struct_hash = 0;
    stack->data_hash   = 0;

    stack->struct_hash = djb2_hash((const unsigned char*)stack, sizeof(*stack));
    stack->data_hash   = djb2_hash((const unsigned char*)stack->canary_data, sizeof(stack_element) * (stack->capacity + COUNT_CANARY));
    )

    error_code = stack_verify(stack);
    if (error_code) {
        log_print_error(error_code, "stack_recalloc: ERROR after recalloc during stack_verify\n");
        return error_code;
    }

    return SUCCESS;
}



void print_stack(const stack_s* const stack) {
    assert(stack != NULL);

    ON_DBG(fprintf(stderr, "stack_s \"%s\"[%p]] created by %s() at %s:%zu)", stack->name, stack, stack->function, stack->file, stack->line));

    fprintf(stderr, "{\n");
    fprintf(stderr, "\tsize = %zu \n capacity = %zu\n", stack->size, stack->capacity);
    fprintf(stderr,"\tdata[%p]\n", stack->data);
    fprintf(stderr, "\t{\n");

    for (size_t ind = 0; ind < stack->size; ind++) {
        ASSERT_FOR_ARR(ind, stack->size);
        fprintf(stderr ,"\t\t*[%zu] = " STK_MODIFIER "\n", ind, stack->data[ind]);
    }

    for (size_t ind = stack->size; ind < stack->capacity; ind++) {
        ASSERT_FOR_ARR(ind, stack->capacity);
        fprintf(stderr ,"\t\t [%lx] = " STK_MODIFIER " (POISON) \n", ind, stack->data[ind]);
    }

    fprintf(stderr ,"\t}\n");
    fprintf(stderr, "}\n");
}


bool is_stack_init(const stack_s* const stack) {
    assert(stack != NULL);

    ON_DBG(
    if (stack->file != NULL) {
        return true;
    }

    if (stack->function != NULL) {
        return true;
    }

    if (stack->line != 0) {
        return true;
    }

    if (stack->name != NULL) {
        return true;
    }

    if (stack->canary_data != NULL) {
        return true;
    }
    )

    if (stack->data != NULL) {
        return true;
    }

    if (stack->capacity != 0) {
        return true;
    }

    if (stack->size != 0) {
        return true;
    }

    return false;
}


void start_logs(void) {
    FILE* log_file = fopen(LOG_FILE_NAME, "w");

    if (log_file == NULL) {
        PRINT_ERROR(ERROR_DURING_OPEN);
        abort();
    }

    if (fclose(log_file) == EOF) {
        PRINT_ERROR(ERROR_DURING_CLOSE);
        abort();
    }
}


void log_dump_stack(const stack_s* const stack, const char* const reason) {
    assert(stack != NULL);

    FILE* log_file = fopen(LOG_FILE_NAME, "a");

    if (log_file == NULL) {
        PRINT_ERROR(ERROR_DURING_OPEN);
        abort();
    }

    time_t tm = 0;
    tm = time(NULL);

    fprintf(log_file, "\nFUNCTION: log_dump_stack DATE:%s REASON: %s", ctime(&tm), reason);

    ON_DBG(fprintf(log_file, "stack_s \"%s\"[%p] created by %s() at %s:%zu\n", stack->name, stack, stack->function, stack->file, stack->line));

    fprintf(log_file, "{\n");
    fprintf(log_file, "\tsize     = %zu\n", stack->size);
    fprintf(log_file, "\tcapacity = %zu\n", stack->capacity);

    ON_DBG(
    fprintf(log_file, "\thash     = %lu\n", stack->struct_hash);

    fprintf(log_file,"\tcanary_data[%p]\n", stack->canary_data);
    fprintf(log_file, "\t{\n");

    fprintf(log_file, "\t\t [%zu] = " POISON_MODIFIER " (CANARY!!!)\n", (size_t)0, (unsigned long long)stack->canary_data[0]); // ???(size_t)0 почему компилятор думает что 0 это int ???

    for (size_t ind = 0; ind < stack->size; ind++) {
        ASSERT_FOR_ARR(ind, stack->size);
        fprintf(log_file ,"\t\t*[%zu] = " STK_MODIFIER "\n", ind + 1, stack->data[ind]);
    }

    for (size_t ind = stack->size; ind < stack->capacity; ind++) {
        ASSERT_FOR_ARR(ind, stack->capacity);
        fprintf(log_file ,"\t\t [%zu] = " POISON_MODIFIER " (POISON!!!) \n", ind + 1, (unsigned long long)stack->data[ind]);
    }

    fprintf(log_file, "\t\t [%zu] = " POISON_MODIFIER " (CANARY!!!)\n", stack->capacity + 1, (unsigned long long)stack->data[stack->capacity]);
    fprintf(log_file ,"\t}\n");
    )

    fprintf(log_file, "\tdata[%p]\n", stack->data);
    fprintf(log_file, "\t{\n");

    for (size_t ind = 0; ind < stack->size; ind++) {
        ASSERT_FOR_ARR(ind, stack->size);
        fprintf(log_file ,"\t\t*[%zu] = " STK_MODIFIER "\n", ind, stack->data[ind]);
    }

    for (size_t ind = stack->size; ind < stack->capacity; ind++) {
        ASSERT_FOR_ARR(ind, stack->capacity);
        fprintf(log_file ,"\t\t [%zu] = " POISON_MODIFIER " (POISON!!!) \n", ind, (unsigned long long)stack->data[ind]);
    }

    fprintf(log_file ,"\t}\n");
    fprintf(log_file, "}\n");

    if (fclose(log_file) == EOF) {
        PRINT_ERROR(ERROR_DURING_CLOSE);
        abort();
    }
}





