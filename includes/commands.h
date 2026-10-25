enum cmd_num_e {
    NAC  = 0,
    PUSH = 1,
    POP  = 2,
    ADD  = 3,
    MUL  = 4,
    DIV  = 5,
    OUT  = 6,
    IN   = 7,
    SIN  = 8,
    COS  = 9,
    SQR  = 10,
    JMP  = 11,
    JA   = 12,
    JB   = 13,
    JE   = 14,
    JAE  = 15,
    JBE  = 16,
    JNE  = 17,
    CAL  = 18,
    RET  = 19,
    DMP  = 20,
    WAIT = 21,
    HLC  = 22
};

struct command_s {
    unsigned char num:5;
    unsigned char konst:1;
    unsigned char reg:1;
    unsigned char ram:1;
};

#define COMMANDS_CNT 23
