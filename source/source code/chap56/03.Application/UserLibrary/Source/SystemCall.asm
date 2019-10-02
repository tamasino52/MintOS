# file      SystemCall.asm
# date      2009/12/12
# author    kkamagui 
#           Copyright(c)2008 All rights reserved by kkamagui
# brief     ���� �������� �ý��� ���� ȣ���ϴµ� ���õ� �ҽ� ����

[BITS 64]           ; ������ �ڵ�� 64��Ʈ �ڵ�� ����

SECTION .text       ; text ����(���׸�Ʈ)�� ����

; �ܺο��� ȣ���� �� �ֵ��� �̸��� ������(Export)
global _START, ExecuteSystemCall

; �ܺο��� ���ǵ� �Լ��� �� �� �ֵ��� ������(Import)
extern Main, exit

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   �ڵ� ����
;       �������α׷��� �ε��Ǿ��� �� ������ ������ ������ ��Ʈ�� ����Ʈ
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
_START:
    call Main       ; C ��� ��Ʈ�� ����Ʈ �Լ�(Main) ȣ��

    mov rdi, rax    ; �Ķ���Ϳ� Main �Լ��� ��ȯ ��(RAX ��������)�� ����
    call exit       ; ���μ��� ���� �Լ��� ȣ��

    jmp $           ; ������ ���μ����� ����ǹǷ� ����� ������� ����

    ret

; �ý��� ���� ����
;   PARAM: QWORD qwServiceNumber, PARAMETERTABLE* pstParameter
ExecuteSystemCall:
    push rcx        ; SYSCALL�� ȣ���� �� RCX �������Ϳ� RIP �������Ͱ� ����ǰ� 
    push r11        ; R11 �������Ϳ� RFLAGS �������Ͱ� ����ǹǷ� ���ÿ� ����
    
    syscall         ; SYSCALL ����
    
    pop r11         ; ���ÿ� ����� ������ RCX �������Ϳ� R11 �������͸� ����
    pop rcx
    ret
