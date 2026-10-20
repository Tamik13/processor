:start
  push 0
  pop rax
  in
  pop rdx

  :loop
    push rax
    push 1
    add
    pop rax

    push rax
    push rax
    mul
    out

    pop  rbx
    push rax
    push rdx

  ja :loop
  hlc
