slope:
    mov rcx, rsp ;; save pointer
    mov rax, 0
    push rax
    mov rax, 0
    push rax
    push QWORD [rsp + 48]
    push QWORD [rsp + 40]
    pop rax
    pop rbx
    cmp rax, rbx
    jge label0
    mov rax, 0
    jmp label1
label0:
    mov rax, 1
label1:
    push rax
    push QWORD [rsp + 48]
    push QWORD [rsp + 40]
    pop rax
    pop rbx
    cmp rax, rbx
    jge label2
    mov rax, 0
    jmp label3
label2:
    mov rax, 1
label3:
    push rax
    pop rax
    pop rbx
    cmp rax, rbx
    je label4
    mov rax, 0
    jmp label5
label4:
    mov rax, 1
label5:
    push rax
    pop rax
    test rax, rax
    jz label6
    push QWORD [rsp + 40]
    push QWORD [rsp + 32]
    pop rax
    pop rbx
    sub rax, rbx
    push rax
    pop rax
    mov [rsp + 8], rax 
    push QWORD [rsp + 48]
    push QWORD [rsp + 40]
    pop rax
    pop rbx
    sub rax, rbx
    push rax
    pop rax
    mov [rsp + 0], rax 
    push QWORD [rsp + 0]
    push QWORD [rsp + 16]
    pop rax
    pop rbx
    div rbx
    push rax
    pop rax
    jmp localend
    add rsp, 0
    jmp label7
label6:
    push QWORD [rsp + 32]
    push QWORD [rsp + 56]
    pop rax
    pop rbx
    cmp rax, rbx
    jg label8
    mov rax, 0
    jmp label9
label8:
    mov rax, 1
label9:
    push rax
    push QWORD [rsp + 32]
    push QWORD [rsp + 56]
    pop rax
    pop rbx
    cmp rax, rbx
    jg label10
    mov rax, 0
    jmp label11
label10:
    mov rax, 1
label11:
    push rax
    pop rax
    pop rbx
    cmp rax, rbx
    je label12
    mov rax, 0
    jmp label13
label12:
    mov rax, 1
label13:
    push rax
    pop rax
    test rax, rax
    jz label14
    push QWORD [rsp + 24]
    push QWORD [rsp + 48]
    pop rax
    pop rbx
    sub rax, rbx
    push rax
    pop rax
    mov [rsp + 8], rax 
    push QWORD [rsp + 32]
    push QWORD [rsp + 56]
    pop rax
    pop rbx
    sub rax, rbx
    push rax
    pop rax
    mov [rsp + 0], rax 
    push QWORD [rsp + 0]
    push QWORD [rsp + 16]
    pop rax
    pop rbx
    div rbx
    push rax
    pop rax
    jmp localend
    add rsp, 0
    jmp label7
label14:
    mov rax, 0
    push rax
    pop rax
    jmp localend
    add rsp, 0
label7:
    mov rax, 0 ;;return 0 at end
localend:
    add rsp, 16
    mov rsp, rcx ;; reinstate pointer
    ret

global _start:
_start:
    mov rax, 5
    push rax
    mov rax, 2
    push rax
    mov rax, 10
    push rax
    mov rax, 12
    push rax
    push QWORD [rsp + 24]
    push QWORD [rsp + 24]
    push QWORD [rsp + 24]
    push QWORD [rsp + 24]
    call slope
    push rax
    mov rax, 60
    pop rdi
    syscall
    mov rax, 60
    mov rdi, 0
    syscall
