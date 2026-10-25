:start
    push 0
    pop rax
    push 0
    pop rbx
    :f_cycle
        :s_cycle

            push rax
            push -270
            add
            push rax
            push -270
            add
            mul

            push rbx
            push -360
            add
            push rbx
            push -360
            add
            mul

            add

            push 50000

            jb :not_circle

            push rax
            push 720
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

            push 540
            push rax
            jb :s_cycle

        push 0
        pop rax

        push rbx
        push 1
        add
        pop rbx

        push 720
        push rbx
        jb :f_cycle

    dmp
    wait

    hlc
