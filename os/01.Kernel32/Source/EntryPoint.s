# file      EntryPoint.s
# date      2008/11/27
# author    kkamagui 
#           Copyright(c)2008 All rights reserved by kkamagui
# brief     ��ȣ ��� Ŀ�� ��Ʈ�� ����Ʈ�� ���õ� �ҽ� ����

[ORG 0x00]          ; �ڵ��� ���� ��巹���� 0x00���� ����
[BITS 16]           ; ������ �ڵ�� 16��Ʈ �ڵ�� ����

SECTION .text       ; text ����(���׸�Ʈ)�� ����

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   �ڵ� ����
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
START:

    mov ax, 0x1000  ; ��ȣ ��� ��Ʈ�� ����Ʈ�� ���� ��巹��(0x10000)�� 
                    ; ���׸�Ʈ �������� ������ ��ȯ
    mov ds, ax      ; DS ���׸�Ʈ �������Ϳ� ����
    mov es, ax      ; ES ���׸�Ʈ �������Ϳ� ����

	;mov DWORD [seax], eax
	;mov DWORD [sebx], ebx
	mov DWORD [secx], ecx
	mov DWORD [sedx], edx
    mov DWORD [sebp], ebp

	call GETMEMORY

	;mov eax, DWORD [seax]
	;mov ebx, DWORD [sebx]
	mov ecx, DWORD [secx]
	mov edx, DWORD [sedx]
	mov ebp, DWORD [sebp]




    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; A20 ����Ʈ�� Ȱ��ȭ
    ; BIOS�� �̿��� ��ȯ�� �������� �� �ý��� ��Ʈ�� ��Ʈ�� ��ȯ �õ�
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; BIOS ���񽺸� ����ؼ� A20 ����Ʈ�� Ȱ��ȭ
    mov ax, 0x2401          ; A20 ����Ʈ Ȱ��ȭ ���� ����
    int 0x15                ; BIOS ���ͷ�Ʈ ���� ȣ��

    jc .A20GATEERROR        ; A20 ����Ʈ Ȱ��ȭ�� �����ߴ��� Ȯ��
    jmp .A20GATESUCCESS

.A20GATEERROR:
    ; ���� �߻� ��, �ý��� ��Ʈ�� ��Ʈ�� ��ȯ �õ�
    in al, 0x92     ; �ý��� ��Ʈ�� ��Ʈ(0x92)���� 1 ����Ʈ�� �о� AL �������Ϳ� ����
    or al, 0x02     ; ���� ���� A20 ����Ʈ ��Ʈ(��Ʈ 1)�� 1�� ����
    and al, 0xFE    ; �ý��� ���� ������ ���� 0xFE�� AND �����Ͽ� ��Ʈ 0�� 0���� ����
    out 0x92, al    ; �ý��� ��Ʈ�� ��Ʈ(0x92)�� ����� ���� 1 ����Ʈ ����
    
.A20GATESUCCESS:
    cli             ; ���ͷ�Ʈ�� �߻����� ���ϵ��� ����
    lgdt [ GDTR ]   ; GDTR �ڷᱸ���� ���μ����� �����Ͽ� GDT ���̺��� �ε�

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; ��ȣ���� ����
    ; Disable Paging, Disable Cache, Internal FPU, Disable Align Check, 
    ; Enable ProtectedMode
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov eax, 0x4001003B ; PG=0, CD=1, NW=0, AM=0, WP=1!!!!, NE=1, ET=1, TS=1, EM=0, MP=1, PE=1
    mov cr0, eax        ; CR0 ��Ʈ�� �������Ϳ� ������ ������ �÷��׸� �����Ͽ� 
                        ; ��ȣ ���� ��ȯ

    ; Ŀ�� �ڵ� ���׸�Ʈ�� 0x00�� �������� �ϴ� ������ ��ü�ϰ� EIP�� ���� 0x00�� �������� �缳��
    ; CS ���׸�Ʈ ������ : EIP
    jmp dword 0x18: ( PROTECTEDMODE - $$ + 0x10000 )

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; �޸� ������ ��� ���� 
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

GETMEMORY:
	;FS will be used to write into the text buffer
	push 0b800h
	pop fs

	;SI is the pointer in the text buffer 
	xor si, si 

	;These are for the INT 15 service
	mov di, baseAddress                    ;Offset in ES where to save the result
	xor ebx, ebx                           ;Start from beginning
	mov ecx, 18h                           ;Length of the output buffer (One descriptor at a time)
	

._get_memory_range:
	;Set up the rest of the registers for INT 15 
	mov eax, 0e820h 
	mov edx, 534D4150h
	int 15h

	;Has somethig been returned actually?
	test ecx, ecx
	jz ._next_memory_range

	;Add length (just the lower 32 bits) to EBP if type = 1 or 3 
	mov eax, DWORD [length]

	;Avoid a branch (just for the sake of less typing)

	mov edx, DWORD [type]         ;EDX = 1        | 2        | 3        | 4   (1 and 3 are available memory)
	and dx, 01h                   ;EDX = 1        | 0        | 1        | 0 
	dec edx                       ;EDX = 0        | ffffffff | 0        | ffffffff 
	not edx                       ;EDX = ffffffff | 0        | ffffffff | 0 
	and eax, edx                  ;EAX = length   | 0        | length   | 0 

	add ebp, eax

._next_memory_range:
	test ebx, ebx 
	jnz ._get_memory_range

	;Print empty line
	;push WORD strNL 
	;call .print
	;push WORD strNL 
	;call .print

	push ebp
	push WORD strTotal 
	call .print

	ret

;Show a 32 bit hex number
.itoa16:
	push cx
	push ebx

	mov cl, 28d

.digits:
	mov ebx, eax
	shr ebx, cl
	and bx, 0fh                     ;Get current nibble

	;Translate nibble (digit to digital)
	mov bl, BYTE [bx + hexDigits]

	;Show it 
	mov bh, 0ch
	mov WORD [fs:si], bx
	add si, 02h   

	sub cl, 04h
	jnc .digits

	pop ebx
	pop cx
	ret

	;This function is a primitive printf, where the only format is % to show a 32 bit 
	;hex number 
	;The "cursor" is kept by SI.
	;SI is always aligned to lines, so 1) never print anything bigger than 80 chars
	;2) successive calls automatically print into their own lines 
	;3) SI is assumed at the beginning of a line 

	;Args
	;Format
.print:
   push bp
   mov bp, sp

   push di
   push cx

   mov di, WORD [bp+04h]      ;String 
   mov cx, 80*2               ;How much to add to SI to reach the next line 

   add bp, 06h                ;Pointer to var arg 

  .scan:

    ;Read cur char 
    mov al, [di]
    inc di

    ;Format?
    cmp al, '%'
    jne .print2

    ;Get current arg and advance index 
    mov eax, DWORD [bp]
    add bp, 04h
    ;Show the number 
    call .itoa16

    ;We printed 8 chars (16 bytes) 
    sub cx, 10h

   jmp .scan    

  .print2:
    ;End of string?
    test al, al
    je .end

    ;Normal char, print it 
    mov ah, 0ch
    mov WORD [fs:si], ax
    add si, 02h
    sub cx, 02h

   jmp .scan   

  .end:
    add si, cx

	pop cx
	pop di

	pop bp
	ret

  	hexDigits db "0123456789abcdef"

  	;Memory descriptor returned by INT 15 
	baseAddress dq 0
	length      dq 0
	type        dd 0
	extAttr     dd 0

	seax		dq 0
	sebx		dq 0
	secx		dq 0
	sedx		dq 0
	sebp		dq 0

  	;Strings, here % denote a 32 bit argument printed as hex 

	strTotal  db "Total amount of memory: %", 0 
	;This is tricky, see below 
	strNL     db 0


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; ��ȣ ���� ����
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
[BITS 32]               ; ������ �ڵ�� 32��Ʈ �ڵ�� ����
PROTECTEDMODE:


    mov ax, 0x20        ; ��ȣ ��� Ŀ�ο� ������ ���׸�Ʈ ��ũ���͸� AX �������Ϳ� ����
    mov ds, ax          ; DS ���׸�Ʈ �����Ϳ� ����
    mov es, ax          ; ES ���׸�Ʈ �����Ϳ� ����
    mov fs, ax          ; FS ���׸�Ʈ �����Ϳ� ����
    mov gs, ax          ; GS ���׸�Ʈ �����Ϳ� ����
    
    ; ������ 0x00000000~0x0000FFFF ������ 64KB ũ��� ����
    mov ss, ax          ; SS ���׸�Ʈ �����Ϳ� ����
    mov esp, 0xFFFE     ; ESP ���������� ��巹���� 0xFFFE�� ����
    mov ebp, 0xFFFE     ; EBP ���������� ��巹���� 0xFFFE�� ����
   
    
    ; ȭ�鿡 ��ȣ ���� ��ȯ�Ǿ��ٴ� �޽����� ��´�.
    push ( SWITCHSUCCESSMESSAGE - $$ + 0x10000 )    ; ����� �޽����� ��巹���� ���ÿ� ����
    push 2                                          ; ȭ�� Y ��ǥ(2)�� ���ÿ� ����
    push 0                                          ; ȭ�� X ��ǥ(0)�� ���ÿ� ����
    call PRINTMESSAGE                               ; PRINTMESSAGE �Լ� ȣ��
    add esp, 12                                     ; ������ �Ķ���� ����
	


    jmp dword 0x18: 0x10200 ; C ��� Ŀ���� �����ϴ� 0x10200 ��巹���� �̵��Ͽ� C ��� Ŀ�� ����

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   �Լ� �ڵ� ����
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; �޽����� ����ϴ� �Լ�
;   ���ÿ� x ��ǥ, y ��ǥ, ���ڿ�
PRINTMESSAGE:
    push ebp        ; ���̽� ������ ��������(BP)�� ���ÿ� ����
    mov ebp, esp    ; ���̽� ������ ��������(BP)�� ���� ������ ��������(SP)�� ���� ����
    push esi        ; �Լ����� �ӽ÷� ����ϴ� �������ͷ� �Լ��� ������ �κп���
    push edi        ; ���ÿ� ���Ե� ���� ���� ���� ������ ����
    push eax
    push ecx
    push edx
    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; X, Y�� ��ǥ�� ���� �޸��� ��巹���� �����
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; Y ��ǥ�� �̿��ؼ� ���� ���� ��巹���� ����
    mov eax, dword [ ebp + 12 ] ; �Ķ���� 2(ȭ�� ��ǥ Y)�� EAX �������Ϳ� ����
    mov esi, 160                ; �� ������ ����Ʈ ��(2 * 80 �÷�)�� ESI �������Ϳ� ����
    mul esi                     ; EAX �������Ϳ� ESI �������͸� ���Ͽ� ȭ�� Y ��巹�� ���
    mov edi, eax                ; ���� ȭ�� Y ��巹���� EDI �������Ϳ� ����
    
    ; X �·Ḧ �̿��ؼ� 2�� ���� �� ���� ��巹���� ����
    mov eax, dword [ ebp + 8 ]  ; �Ķ���� 1(ȭ�� ��ǥ X)�� EAX �������Ϳ� ����
    mov esi, 2                  ; �� ���ڸ� ��Ÿ���� ����Ʈ ��(2)�� ESI �������Ϳ� ����
    mul esi                     ; EAX �������Ϳ� ESI �������͸� ���Ͽ� ȭ�� X ��巹���� ���
    add edi, eax                ; ȭ�� Y ��巹���� ���� X ��巹���� ���ؼ�
                                ; ���� ���� �޸� ��巹���� ���
                                
    ; ����� ���ڿ��� ��巹��      
    mov esi, dword [ ebp + 16 ] ; �Ķ���� 3(����� ���ڿ��� ��巹��)

.MESSAGELOOP:               ; �޽����� ����ϴ� ����
    mov cl, byte [ esi ]    ; ESI �������Ͱ� ����Ű�� ���ڿ� ��ġ���� �� ���ڸ� 
                            ; CL �������Ϳ� ����
                            ; CL �������ʹ� ECX ���������� ���� 1����Ʈ�� �ǹ�
                            ; ���ڿ��� 1����Ʈ�� ����ϹǷ� ECX ���������� ���� 1����Ʈ�� ���

    cmp cl, 0               ; ����� ���ڿ� 0�� ��
    je .MESSAGEEND          ; ������ ������ ���� 0�̸� ���ڿ��� ����Ǿ�����
                            ; �ǹ��ϹǷ� .MESSAGEEND�� �̵��Ͽ� ���� ��� ����

    mov byte [ edi + 0xB8000 ], cl  ; 0�� �ƴ϶�� ���� �޸� ��巹�� 
                                    ; 0xB8000 + EDI �� ���ڸ� ���
    
    add esi, 1              ; ESI �������Ϳ� 1�� ���Ͽ� ���� ���ڿ��� �̵�
    add edi, 2              ; EDI �������Ϳ� 2�� ���Ͽ� ���� �޸��� ���� ���� ��ġ�� �̵�
                            ; ���� �޸𸮴� (����, �Ӽ�)�� ������ �����ǹǷ� ���ڸ� ����Ϸ���
                            ; 2�� ���ؾ� ��

    jmp .MESSAGELOOP        ; �޽��� ��� ������ �̵��Ͽ� ���� ���ڸ� ���

.MESSAGEEND:
    pop edx     ; �Լ����� ����� ���� EDX �������ͺ��� EBP �������ͱ����� ���ÿ�
    pop ecx     ; ���Ե� ���� �̿��ؼ� ����
    pop eax     ; ������ ���� �������� �� �����Ͱ� ���� ���� ������
    pop edi     ; �ڷᱸ��(Last-In, First-Out)�̹Ƿ� ����(push)�� ��������
    pop esi     ; ����(pop) �ؾ� ��
    pop ebp     ; ���̽� ������ ��������(BP) ����
    ret         ; �Լ��� ȣ���� ���� �ڵ��� ��ġ�� ����

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   ������ ����
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; �Ʒ��� �����͵��� 8byte�� ���� �����ϱ� ���� �߰�
align 8, db 0

; GDTR�� ���� 8byte�� �����ϱ� ���� �߰�
dw 0x0000
; GDTR �ڷᱸ�� ����
GDTR:
    dw GDTEND - GDT - 1         ; �Ʒ��� ��ġ�ϴ� GDT ���̺��� ��ü ũ��
    dd ( GDT - $$ + 0x10000 )   ; �Ʒ��� ��ġ�ϴ� GDT ���̺��� ���� ��巹��

; GDT ���̺� ����
GDT:
    ; ��(NULL) ��ũ����, �ݵ�� 0���� �ʱ�ȭ�ؾ� ��
    NULLDescriptor:
        dw 0x0000
        dw 0x0000
        db 0x00
        db 0x00
        db 0x00
        db 0x00

    ; IA-32e ��� Ŀ�ο� �ڵ� ���׸�Ʈ ��ũ����
    IA_32eCODEDESCRIPTOR:     
        dw 0xFFFF       ; Limit [15:0]
        dw 0x0000       ; Base [15:0]
        db 0x00         ; Base [23:16]
        db 0x9A         ; P=1, DPL=0, Code Segment, Execute/Read
        db 0xAF         ; G=1, D=0, L=1, Limit[19:16]
        db 0x00         ; Base [31:24]  
        
    ; IA-32e ��� Ŀ�ο� ������ ���׸�Ʈ ��ũ����
    IA_32eDATADESCRIPTOR:
        dw 0xFFFF       ; Limit [15:0]
        dw 0x0000       ; Base [15:0]
        db 0x00         ; Base [23:16]
        db 0x92         ; P=1, DPL=0, Data Segment, Read/Write
        db 0xAF         ; G=1, D=0, L=1, Limit[19:16]
        db 0x00         ; Base [31:24]
        
    ; ��ȣ ��� Ŀ�ο� �ڵ� ���׸�Ʈ ��ũ����
    CODEDESCRIPTOR:     
        dw 0xFFFF       ; Limit [15:0]
        dw 0x0000       ; Base [15:0]
        db 0x00         ; Base [23:16]
        db 0x9A         ; P=1, DPL=0, Code Segment, Execute/Read
        db 0xCF         ; G=1, D=1, L=0, Limit[19:16]
        db 0x00         ; Base [31:24]  
        
    ; ��ȣ ��� Ŀ�ο� ������ ���׸�Ʈ ��ũ����
    DATADESCRIPTOR:
        dw 0xFFFF       ; Limit [15:0]
        dw 0x0000       ; Base [15:0]
        db 0x00         ; Base [23:16]
        db 0x92         ; P=1, DPL=0, Data Segment, Read/Write
        db 0xCF         ; G=1, D=1, L=0, Limit[19:16]
        db 0x00         ; Base [31:24]
GDTEND:

; ��ȣ ���� ��ȯ�Ǿ��ٴ� �޽���
SWITCHSUCCESSMESSAGE: db 'Switch To Protected Mode Success~!!', 0

times 512 - ( $ - $$ )  db  0x00    ; 512����Ʈ�� ���߱� ���� ���� �κ��� 0���� ä��
