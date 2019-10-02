# file      SystemCallEntryPoint.asm
# date      2009/12/12
# author    kkamagui 
#           Copyright(c)2008 All rights reserved by kkamagui
# brief     �ý��� �� ��Ʈ�� ����Ʈ�� ���õ� �ҽ� ����

[BITS 64]           ; ������ �ڵ�� 64��Ʈ �ڵ�� ����

SECTION .text       ; text ����(���׸�Ʈ)�� ����

; �ܺο��� ���ǵ� �Լ��� �� �� �ֵ��� ������(Import)
extern kProcessSystemCall

; C ���� ȣ���� �� �ֵ��� �̸��� ������(Export)
global kSystemCallEntryPoint, kSystemCallTestTask


; ���� �������� SYSCALL�� �������� �� ȣ��Ǵ� �ý��� �� ��Ʈ�� ����Ʈ
;   PARAM: QWORD qwServiceNumber, PARAMETERTABLE* pstParameter
kSystemCallEntryPoint:
    push rcx        ; SYSRET�� �ǵ��ư� �� ����� RIP ���������� ���� RFLAGS ����������
    push r11        ; ���� ���� 
    mov cx, ds      ; CS�� SS ���׸�Ʈ �����͸� ������ ������ ���׸�Ʈ �������� 
    push cx         ; ���� ���ÿ� ����
    mov cx, es      ; DS ���׸�Ʈ �����Ϳ� ES ���׸�Ʈ �����ʹ� ���ÿ� �ٷ� ������ ��
    push cx         ; �����Ƿ� CX �������ͷ� �ű� �� ����
    push fs         
    push gs
    
    mov cx, 0x10    ; Ŀ�� ������ ���׸�Ʈ �������� �ε����� CX �������Ϳ� ����
    mov ds, cx      ; ���׸�Ʈ �����͸� Ŀ�� ������ ���׸�Ʈ �����ͷ� ��ü
    mov es, cx
    mov fs, cx
    mov gs, cx
    
    call kProcessSystemCall     ; ���� ��ȣ�� ���� Ŀ�� ���� �Լ��� ȣ��

    pop gs          ; ���ÿ� ����� ������ ���׸�Ʈ �������� ���� RCX, R11 
    pop fs          ; �������͸� ����
    pop cx
    mov es, cx
    pop cx
    mov ds, cx
    pop r11
    pop rcx

    o64 sysret      ; SYSCALL ��ɾ�� ȣ��Ǿ����Ƿ� SYSRET ��ɾ�� �����ؾ� ��

; �ý��� ���� �׽�Ʈ�ϴ� ���� ���� �½�ũ
;   �׽�Ʈ �ý��� �� ���񽺸� 3�� �������� ȣ���� �� exit() ���񽺸� ȣ���Ͽ� ����
;   PARAM: ����
kSystemCallTestTask:
    mov rdi, 0xFFFFFFFF ; ���� ��ȣ�� �׽�Ʈ �ý��� ��(SYSCALL_TEST, 0xFFFFFFFF)�� ����
    mov rsi, 0x00   ; �ý��� ���� ȣ���� �� ����ϴ� �Ķ���� ���̺� �����Ϳ� NULL�� ����
    syscall         ; �ý��� ���� ȣ��

    mov rdi, 0xFFFFFFFF ; ���� ��ȣ�� �׽�Ʈ �ý��� ��(SYSCALL_TEST, 0xFFFFFFFF)�� ����
    mov rsi, 0x00   ; �ý��� ���� ȣ���� �� ����ϴ� �Ķ���� ���̺� �����Ϳ� NULL�� ����
    syscall         ; �ý��� ���� ȣ��

    mov rdi, 0xFFFFFFFF ; ���� ��ȣ�� �׽�Ʈ �ý��� ��(SYSCALL_TEST, 0xFFFFFFFF)�� ����
    mov rsi, 0x00   ; �ý��� ���� ȣ���� �� ����ϴ� �Ķ���� ���̺� �����Ϳ� NULL�� ����
    syscall         ; �ý��� ���� ȣ��

    mov rdi, 24     ; ���� ��ȣ�� �½�ũ ����(SYSCALL_EXITTASK, 24)�� ����
    mov rsi, 0x00   ; �ý��� ���� ȣ���� �� ����ϴ� �Ķ���� ���̺� �����Ϳ� NULL�� ����
    syscall         ; �ý��� ���� ȣ��
    jmp $
