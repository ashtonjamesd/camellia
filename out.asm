section .data
section .text
  global _start

_start:
  call main
  mov rbx, rax
  mov rax, 1
  int 0x80

main:
  push rbp
  mov rbp, rsp
  sub rsp, 48
  mov qword [rbp-8], 2
  mov qword [rbp-16], 0
  mov qword [rbp-24], 0
  mov qword [rbp-32], 0
  mov qword [rbp-16], 0
  mov rsp, rbp
  pop rbp
  ret
