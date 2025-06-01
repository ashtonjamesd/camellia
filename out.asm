section .rodata
section .data
section .text
  global _start

_start:
  call main
  mov rdi, rax
  mov rax, 60
  syscall

main:
  push rbp
  mov rbp, rsp
  sub rsp, 24
section .rodata
  x db "Hello, World!"
section .text
  mov qword [rbp-16], 0
.Lwhile_start0:
  mov rax, qword [rbp-16]
  push rax
  mov rax, 10
  mov rbx, rax
  pop rax
  cmp rax, rbx
  jge .Lwhile_end1
  mov qword [rbp0], 1
  mov rax, 1
  cmp rax, 0
  je .Lelse1
  mov rax, 1
  mov rdi, 1
  lea rsi, [x]
  mov rdx, 13
  syscall
  jmp .Lend0
  .Lelse1:
.Lend0:
  mov rax, qword [rbp-16]
  push rax
  mov rax, 1
  pop rbx
  add rax, rbx
  mov qword [rbp-16], rax
  jmp .Lwhile_start0
.Lwhile_end1:
  mov rsp, rbp
  pop rbp
  ret
