:start
    push 0
    pop rax
    push 0
    pop rbx
    :f_cycle
        :s_cycle

            push rax
            push -36
            add
            push rax
            push -36
            add
            mul

            push rbx
            push -27
            add
            push rbx
            push -27
            add
            mul

            add

            push 100

            jb :not_circle

            push rax
            push 72
            mul
            push rbx
            add
            pop rcx

            push 1
            pop [rcx]

            :not_circle

            push rax
            push 1
            add
            pop rax

            push 72
            push rax
            jb :s_cycle

        push 0
        pop rax

        push rbx
        push 1
        add
        pop rbx

        push 54
        push rbx
        jb :f_cycle

    dmp

    hlc
