enum commands_e {
    NAC  = -1,
    PUSH = 1,   // d
    PUR  = 2,   // s
    POP  = 3,   // s
    ADD  = 4,
    MUL  = 5,
    DIV  = 6,
    OUT  = 7,
    IN   = 8,
    SIN  = 9,
    COS  = 10,
    SQR  = 11,
    JMP  = 12,  // s
    JA   = 13,  // s
    JB   = 14,  // s
    JE   = 15,  // s
    JAE  = 16,  // s
    JBE  = 17,  // s
    JNE  = 18,  // s
    CAL  = 19,  // s
    RET  = 20,
    HLC  = 21
};

#define COMMANDS_CNT 22
