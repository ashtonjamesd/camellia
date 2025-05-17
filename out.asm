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
  mov qword [rbp-8], 0
  mov rax, 1
  push rax
  mov rax, 2
  push rax
  mov rax, 3
  pop rbx
  imul rax, rbx
  mov rbx, rax
  pop rax
  sub rax, rbx
  mov qword [rbp-16], rax
  mov rax, [rbp-16]
  mov rsp, rbp
  pop rbp
  ret
