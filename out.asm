section .data
section .text
  global _start

_start:
  call main
  mov ebx, eax
  mov eax, 1
  int 0x80

main:
  call x
  ret

x:
  mov eax, 5
  ret
