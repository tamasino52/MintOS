[ORG 0X00]
[BITS 16]

SECTION .text

jmp 0x1020:START

SECTORCOUNT:	dw 0x0000

TOTALSECTORCOUNT equ 1024


START:
	mov ax,cs
	mov ds,ax
	mov ax,0xB800
	mov es,ax

	;;;;;;;;;;;;;;;


		times (512 -($ - $$ )% 512) db 0x00





