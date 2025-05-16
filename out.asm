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
  sub rsp, 16
  mov rax, 0
  mov rsp, rbp
  pop rbp
  ret
