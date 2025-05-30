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
  mov qword [rbp-8], 10
  mov qword [rbp-16], 5
  mov rax, 1
  cmp rax, 0
  je .Lelse1
  mov rax, qword [rbp-8]
  push rax
  mov rax, 2
  mov rbx, rax
  pop rax
  sub rax, rbx
  mov qword [rbp-8], rax
  mov rax, 0
  cmp rax, 0
  je .Lelse3
  mov rax, qword [rbp-8]
  push rax
  mov rax, 2
  mov rbx, rax
  pop rax
  sub rax, rbx
  mov qword [rbp-8], rax
  jmp .Lend2
  .Lelse3:
  mov rax, qword [rbp-8]
  push rax
  mov rax, 2
  pop rbx
  add rax, rbx
  mov qword [rbp-8], rax
.Lend2:
  mov rax, 1
  cmp rax, 0
  je .Lelse5
  mov rax, qword [rbp-8]
  push rax
  mov rax, qword [rbp-16]
  pop rbx
  add rax, rbx
  mov qword [rbp-8], rax
  jmp .Lend4
  .Lelse5:
.Lend4:
  mov rax, qword [rbp-8]
  push rax
  mov rax, 1
  pop rbx
  add rax, rbx
  mov qword [rbp-8], rax
  jmp .Lend0
  .Lelse1:
  mov rax, qword [rbp-8]
  push rax
  mov rax, 2
  pop rbx
  add rax, rbx
  mov qword [rbp-8], rax
.Lend0:
  mov rax, qword [rbp-8]
  push rax
  mov rax, qword [rbp-16]
  pop rbx
  add rax, rbx
  mov rsp, rbp
  pop rbp
  ret
