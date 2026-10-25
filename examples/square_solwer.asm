:start
    in
    in
    in
    pop rcx
    pop rbx
    pop rax

    push rax
    push 0
    je  :ln

    cal :disc
    pop rdx

    push  rdx
    push 0

    jbe :upz

    push 0
    out

    hlc

:ln
    push 0
    push rcx
    je :inf_roots

    push 0
    push rbx
    je :z_roots

    push 1
    out

    push rcx
    push -1
    mul
    push rbx
    div

    out
    hlc

:disc
    push rbx
    push rbx
    mul

    push 4
    push -1
    push rax
    push rcx

    mul
    mul
    mul

    add
    ret

:upz
    push rbx
    push -1
    mul

    push rdx
    sqr

    add

    push rax
    push 2

    mul
    div


    push rdx
    push 0


    je :one_root

    out

    push 2
    out


    push rbx

    push rdx
    sqr

    add

    push rax
    push 2

    mul
    div

    push -1
    mul

    out

    hlc

    :one_root
    push 1
    out
    pop rax
    out

    hlc

:inf_roots
    push 67
    out
    hlc

:z_roots
    push 0
    out
    hlc
