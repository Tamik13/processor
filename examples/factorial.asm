:start
    in
    pop rcx

    push 0
    pop rax

    push 1
    pop rbx

    cal :fact

    push

    hlc

:fact
    push rax
    push 1
    add
    pop rax

    push rbx
    push rax
    mul
    out
    pop rbx

    push rcx
    push rax
    jae :RT

    cal :fact

    :RT

    ret
