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
  mov qword [rbp-8], 1
  mov qword [rbp-8], 1
.Lwhile_start0:
  mov rax, qword [rbp-8]
  push rax
  mov rax, 255
  mov rbx, rax
  pop rax
  cmp rax, rbx
  jge .Lwhile_end1
  mov rax, qword [rbp-8]
  push rax
  mov rax, 1
  pop rbx
  add rax, rbx
  mov qword [rbp-8], rax
  jmp .Lwhile_start0
.Lwhile_end1:
  mov rax, [rbp-8]
  mov rsp, rbp
  pop rbp
  ret
