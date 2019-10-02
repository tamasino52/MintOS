# file      EntryPoint.s
# date      2009/01/01
# author    kkamagui 
#           Copyright(c)2008 All rights reserved by kkamagui
# brief     IA-32e ��� Ŀ�� ��Ʈ�� ����Ʈ�� ���õ� �ҽ� ����

[BITS 64]           ; ������ �ڵ�� 64��Ʈ �ڵ�� ����

SECTION .text       ; text ����(���׸�Ʈ)�� ����

; �ܺο��� ���ǵ� �Լ��� �� �� �ֵ��� ������(Import)
extern Main
; APIC ID ���������� ��巹���� ��� �ھ��� ����
extern g_qwAPICIDAddress, g_iWakeUpApplicationProcessorCount

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   �ڵ� ����
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
START:
    mov ax, 0x10        ; IA-32e ��� Ŀ�ο� ������ ���׸�Ʈ ��ũ���͸� AX �������Ϳ� ����
    mov ds, ax          ; DS ���׸�Ʈ �����Ϳ� ����
    mov es, ax          ; ES ���׸�Ʈ �����Ϳ� ����
    mov fs, ax          ; FS ���׸�Ʈ �����Ϳ� ����
    mov gs, ax          ; GS ���׸�Ʈ �����Ϳ� ����

    ; ������ 0x600000~0x6FFFFF ������ 1MB ũ��� ����
    mov ss, ax          ; SS ���׸�Ʈ �����Ϳ� ����
    mov rsp, 0x6FFFF8   ; RSP ���������� ��巹���� 0x6FFFF8�� ����
    mov rbp, 0x6FFFF8   ; RBP ���������� ��巹���� 0x6FFFF8�� ����
    
    ; ��Ʈ �δ� ������ Bootstrap Processor �÷��׸� Ȯ���Ͽ�, Bootstrap Processor�̸�
    ; �ٷ� Main �Լ��� �̵�
    cmp byte [ 0x7C09 ], 0x01
    je .BOOTSTRAPPROCESSORSTARTPOINT
    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; Application Processor�� �����ϴ� ����
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; ������ �����(Top)�� APIC ID�� �̿��ؼ� 0x700000 ���Ϸ� �̵�
    ; �ִ� 16�� �ھ���� ���� �����ϹǷ� ���� ������ 1M�� 16���� ���� ���� 
    ; 64Kbyte(0x10000)��ŭ�� �Ʒ��� �̵��ϸ鼭 ���� 
    ; ���� APIC�� APIC ID �������Ϳ��� ID�� ����. ID�� Bit 24~31�� ��ġ��
    mov rax, 0                              ; RAX �������� �ʱ�ȭ
    mov rbx, qword [ g_qwAPICIDAddress ]    ; APIC ID ���������� ��巹���� ����
    mov eax, dword [ rbx ] ; APIC ID �������Ϳ��� APIC ID�� ����(��Ʈ 24~31)
    shr rax, 24            ; ��Ʈ 24~31�� �����ϴ� APIC ID�� ����Ʈ ���Ѽ� ��Ʈ 0~7�� �̵�
    
    ; ������ APIC ID�� 64Kbyte(0x10000)�� ���Ͽ� ������ ����⸦ �̵���ų �Ÿ��� ���
    mov rbx, 0x10000       ; RBX �������Ϳ� ������ ũ��(64Kbyte)�� ����
    mul rbx                ; RAX �������Ϳ� ����� APIC ID�� RBX ���������� ���� ���� ����
    
    sub rsp, rax   ; RSP�� RBP �������Ϳ��� RAX �������Ϳ� ����� ��(������ ����⸦
    sub rbp, rax   ; �̵���ų �Ÿ�)�� ���� �� �ھ� �� ������ �Ҵ�����

    ; ��� Application Processor ���� 1 ������Ŵ. lock ��ɾ ����Ͽ� ������
    ; ��Ÿ��(Exclusive) ������ �����ϵ��� ��
    lock inc dword [ g_iWakeUpApplicationProcessorCount ]
    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; Bootstrap Processor�� Application Processor�� �������� �����ϴ� ����
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.BOOTSTRAPPROCESSORSTARTPOINT:
    call Main           ; C ��� ��Ʈ�� ����Ʈ �Լ�(Main) ȣ��
    
    jmp $
