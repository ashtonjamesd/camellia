section .data
section .text
  global _start

_start:
  call main
  mov ebx, eax
  mov eax, 1
  int 0x80

write:
  mov rax, 2
  mov rax, 5
  ret

main:
  mov eax, 0
  ret
