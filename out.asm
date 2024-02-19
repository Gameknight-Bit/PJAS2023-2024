slope:
    push QWORD [rsp + 8]
    push QWORD [rsp + 24]
    pop rax
    pop rbx
    sub rax, rbx
    push rax
    push QWORD [rsp + 32]
    push QWORD [rsp + 48]
    pop rax
    pop rbx
    sub rax, rbx
    push rax
    pop rax
    pop rbx
    div rbx
    push rax
    pop rax
    ret
    add rsp, 0
    mov rax, 0 ;;return 0 at end
    ret

global _start:
_start:
    mov rax, 2
    push rax
    mov rax, 0
    push rax
    mov rax, 5
    push rax
    mov rax, 4
    push rax
    call slope
    push rax
    mov rax, 60
    pop rdi
    syscall
    mov rax, 60
    mov rdi, 0
    syscall
