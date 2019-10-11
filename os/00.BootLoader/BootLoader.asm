# file      BootLoader.asm
# date      2008/11/27
# author    kkamagui 
#           Copyright(c)2008 All rights reserved by kkamagui
# brief     MINT64 OS�� ��Ʈ �δ� �ҽ� ����

[ORG 0x00]          ; �ڵ��� ���� ��巹���� 0x00���� ����
[BITS 16]           ; ������ �ڵ�� 16��Ʈ �ڵ�� ����

SECTION .text       ; text ����(���׸�Ʈ)�� ����

jmp 0x07C0:START    ; CS ���׸�Ʈ �������Ϳ� 0x07C0�� �����ϸ鼭, START ���̺��� �̵�

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   MINT64 OS�� ���õ� ȯ�� ���� ��
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
TOTALSECTORCOUNT:   dw  0x02    ; ��Ʈ �δ��� ������ MINT64 OS �̹����� ũ��
                                ; �ִ� 1152 ����(0x90000byte)���� ����
KERNEL32SECTORCOUNT: dw 0x02    ; ��ȣ ��� Ŀ���� �� ���� ��

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   �ڵ� ����
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
START:
    mov ax, 0x07C0  ; ��Ʈ �δ��� ���� ��巹��(0x7C00)�� ���׸�Ʈ �������� ������ ��ȯ
    mov ds, ax      ; DS ���׸�Ʈ �������Ϳ� ����
    mov ax, 0xB800  ; ���� �޸��� ���� ��巹��(0x7C00)�� ���׸�Ʈ �������� ������ ��ȯ
    mov es, ax      ; ES ���׸�Ʈ �������Ϳ� ����

    ; ������ 0x0000:0000~0x0000:FFFF ������ 64KB ũ��� ����
    mov ax, 0x0000  ; ���� ���׸�Ʈ�� ���� ��巹��(0x0000)�� ���׸�Ʈ �������� ������ ��ȯ
    mov ss, ax      ; SS ���׸�Ʈ �������Ϳ� ����
    mov sp, 0xFFFE  ; SP ���������� ��巹���� 0xFFFE�� ����
    mov bp, 0xFFFE  ; BP ���������� ��巹���� 0xFFFE�� ����

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; ȭ���� ��� �����, �Ӽ����� ������� ����
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov si,    0                    ; SI ��������(���ڿ� ���� �ε��� ��������)�� �ʱ�ȭ
        
.SCREENCLEARLOOP:                   ; ȭ���� ����� ����
    mov byte [ es: si ], 0          ; ���� �޸��� ���ڰ� ��ġ�ϴ� ��巹����
                                    ; 0�� �����Ͽ� ���ڸ� ����
    mov byte [ es: si + 1 ], 0x0A   ; ���� �޸��� �Ӽ��� ��ġ�ϴ� ��巹����
                                    ; 0x0A(���� ������ ���� ���)�� ����


    add si, 2                       ; ���ڿ� �Ӽ��� ���������Ƿ� ���� ��ġ�� �̵�

    cmp si, 80 * 25 * 2     ; ȭ���� ��ü ũ��� 80 ���� * 25 ������
                            ; ����� ������ ���� �ǹ��ϴ� SI �������Ϳ� ��
    jl .SCREENCLEARLOOP     ; SI �������Ͱ� 80 * 25 * 2���� �۴ٸ� ���� ������ 
                            ; ���� ������ �����Ƿ� .SCREENCLEARLOOP ���̺��� �̵�
    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; ȭ�� ��ܿ� ���� �޽����� ���
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    push MESSAGE1               ; ����� �޽����� ��巹���� ���ÿ� ����
    push 0                      ; ȭ�� Y ��ǥ(0)�� ���ÿ� ����
    push 0                      ; ȭ�� X ��ǥ(0)�� ���ÿ� ����
    call PRINTMESSAGE           ; PRINTMESSAGE �Լ� ȣ��
    add  sp, 6                  ; ������ �Ķ���� ����
    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; OS �̹����� �ε��Ѵٴ� �޽��� ���
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    push IMAGELOADINGMESSAGE    ; ����� �޽����� ��巹���� ���ÿ� ����           
    push 1                      ; ȭ�� Y ��ǥ(1)�� ���ÿ� ����                     
    push 0                      ; ȭ�� X ��ǥ(0)�� ���ÿ� ����                     
    call PRINTMESSAGE           ; PRINTMESSAGE �Լ� ȣ��                           
    add  sp, 6                  ; ������ �Ķ���� ����                             
        
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; ��ũ���� OS �̹����� �ε�
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; ��ũ�� �б� ���� ���� ����
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
RESETDISK:                          ; ��ũ�� �����ϴ� �ڵ��� ����
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; BIOS Reset Function ȣ��
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; ���� ��ȣ 0, ����̺� ��ȣ(0=Floppy)
    mov ax, 0
    mov dl, 0              
    int 0x13     
    ; ������ �߻��ϸ� ���� ó���� �̵�
    jc  HANDLEDISKERROR
        
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; ��ũ���� ���͸� ����
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; ��ũ�� ������ �޸𸮷� ������ ��巹��(ES:BX)�� 0x10000���� ����
    mov si, 0x1000                  ; OS �̹����� ������ ��巹��(0x10000)�� 
                                    ; ���׸�Ʈ �������� ������ ��ȯ
    mov es, si                      ; ES ���׸�Ʈ �������Ϳ� �� ����
    mov bx, 0x0000                  ; BX �������Ϳ� 0x0000�� �����Ͽ� ������ 
                                    ; ��巹���� 0x1000:0000(0x10000)���� ���� ����

    mov di, word [ TOTALSECTORCOUNT ] ; ������ OS �̹����� ���� ���� DI �������Ϳ� ����

READDATA:                           ; ��ũ�� �д� �ڵ��� ����
    ; ��� ���͸� �� �о����� Ȯ��
    cmp di, 0               ; ������ OS �̹����� ���� ���� 0�� ��
    je  READEND             ; ������ ���� ���� 0�̶�� �� ���� �����Ƿ� READEND�� �̵�
    sub di, 0x1             ; ������ ���� ���� 1 ����

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; BIOS Read Function ȣ��
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ah, 0x02                        ; BIOS ���� ��ȣ 2(Read Sector)
    mov al, 0x1                         ; ���� ���� ���� 1
    mov ch, byte [ TRACKNUMBER ]        ; ���� Ʈ�� ��ȣ ����
    mov cl, byte [ SECTORNUMBER ]       ; ���� ���� ��ȣ ����
    mov dh, byte [ HEADNUMBER ]         ; ���� ��� ��ȣ ����
    mov dl, 0x00                        ; ���� ����̺� ��ȣ(0=Floppy) ����
    int 0x13                            ; ���ͷ�Ʈ ���� ����
    jc HANDLEDISKERROR                  ; ������ �߻��ߴٸ� HANDLEDISKERROR�� �̵�
    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; ������ ��巹���� Ʈ��, ���, ���� ��巹�� ���
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    add si, 0x0020      ; 512(0x200)����Ʈ��ŭ �о����Ƿ�, �̸� ���׸�Ʈ ��������
                        ; ������ ��ȯ
    mov es, si          ; ES ���׸�Ʈ �������Ϳ� ���ؼ� ��巹���� �� ���� ��ŭ ����
    
    ; �� ���͸� �о����Ƿ� ���� ��ȣ�� ������Ű�� ������ ����(18)���� �о����� �Ǵ�
    ; ������ ���Ͱ� �ƴϸ� ���� �б�� �̵��ؼ� �ٽ� ���� �б� ����
    mov al, byte [ SECTORNUMBER ]       ; ���� ��ȣ�� AL �������Ϳ� ����
    add al, 0x01                        ; ���� ��ȣ�� 1 ����
    mov byte [ SECTORNUMBER ], al       ; ������Ų ���� ��ȣ�� SECTORNUMBER�� �ٽ� ����
    cmp al, 19                          ; ������Ų ���� ��ȣ�� 19�� ��
    jl READDATA                         ; ���� ��ȣ�� 19 �̸��̶�� READDATA�� �̵�
    
    ; ������ ���ͱ��� �о�����(���� ��ȣ�� 19�̸�) ��带 ���(0->1, 1->0)�ϰ�, 
    ; ���� ��ȣ�� 1�� ����
    xor byte [ HEADNUMBER ], 0x01       ; ��� ��ȣ�� 0x01�� XOR�Ͽ� ���(0->1, 1->1)
    mov byte [ SECTORNUMBER ], 0x01     ; ���� ��ȣ�� �ٽ� 1�� ����
    
    ; ���� ��尡 1->0�� �ٲ������ ���� ��带 ��� ���� ���̹Ƿ� �Ʒ��� �̵��Ͽ�
    ; Ʈ�� ��ȣ�� 1 ����
    cmp byte [ HEADNUMBER ], 0x00       ; ��� ��ȣ�� 0x00�� ��
    jne READDATA                        ; ��� ��ȣ�� 0�� �ƴϸ� READDATA�� �̵�
    
    ; Ʈ���� 1 ������Ų ��, �ٽ� ���� �б�� �̵�
    add byte [ TRACKNUMBER ], 0x01      ; Ʈ�� ��ȣ�� 1 ����
    jmp READDATA                        ; READDATA�� �̵�
READEND:

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; OS �̹����� �Ϸ�Ǿ��ٴ� �޽����� ���
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    push LOADINGCOMPLETEMESSAGE     ; ����� �޽����� ��巹���� ���ÿ� ����
    push 1                          ; ȭ�� Y ��ǥ(1)�� ���ÿ� ����
    push 20                         ; ȭ�� X ��ǥ(20)�� ���ÿ� ����
    call PRINTMESSAGE               ; PRINTMESSAGE �Լ� ȣ��
    add  sp, 6                      ; ������ �Ķ���� ����

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; ���� �޸� ������ ���
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	mov ax, 0E820h
	mov dx, 534D4150h
	mov cx, 20
	int 15h

	mov si, 5
	mov bl, 10
	mov ax, cx

	mov ax, 87
	.loop:
	
	; ah : 나머지     al : 몫
	div bl
	add ah, '0'
	mov byte [ MEMORYSIZE + si ], ah
	sub si, 1
	mov ah, 0

	mov ah, 0
	div bl
	add ah, '0'
	mov byte [ MEMORYSIZE + si ], ah
	sub si, 1

	
	
	push MEMORYSIZE					; ����� �޽����� ��巹���� ���ÿ� ����
    push 20                          ; ȭ�� Y ��ǥ(1)�� ���ÿ� ����
    push 0							; ȭ�� X ��ǥ(20)�� ���ÿ� ����
    call PRINTMESSAGE               ; PRINTMESSAGE �Լ� ȣ��
    add  sp, 6                      ; ������ �Ķ���� ����
	




    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; �ε��� ���� OS �̹��� ����    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    jmp 0x1000:0x0000
    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   �Լ� �ڵ� ����
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ��ũ ������ ó���ϴ� �Լ�   
HANDLEDISKERROR:
    push DISKERRORMESSAGE   ; ���� ���ڿ��� ��巹���� ���ÿ� ����
    push 1                  ; ȭ�� Y ��ǥ(1)�� ���ÿ� ����
    push 20                 ; ȭ�� X ��ǥ(20)�� ���ÿ� ����
    call PRINTMESSAGE       ; PRINTMESSAGE �Լ� ȣ��
    
    jmp $                   ; ���� ��ġ���� ���� ���� ����

; �޽����� ����ϴ� �Լ�
;   PARAM: x ��ǥ, y ��ǥ, ���ڿ�
PRINTMESSAGE:
    push bp         ; ���̽� ������ ��������(BP)�� ���ÿ� ����
    mov bp, sp      ; ���̽� ������ ��������(BP)�� ���� ������ ��������(SP)�� ���� ����
                    ; ���̽� ������ ��������(BP)�� �̿��ؼ� �Ķ���Ϳ� ������ ����

    push es         ; ES ���׸�Ʈ �������ͺ��� DX �������ͱ��� ���ÿ� ����
    push si         ; �Լ����� �ӽ÷� ����ϴ� �������ͷ� �Լ��� ������ �κп���
    push di         ; ���ÿ� ���Ե� ���� ���� ���� ������ ����
    push ax
    push cx
    push dx
    
    ; ES ���׸�Ʈ �������Ϳ� ���� ��� ��巹�� ����
    mov ax, 0xB800              ; ���� �޸� ���� ��巹��(0x0B8000)�� 
                                ; ���׸�Ʈ �������� ������ ��ȯ
    mov es, ax                  ; ES ���׸�Ʈ �������Ϳ� ����
    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; X, Y�� ��ǥ�� ���� �޸��� ��巹���� �����
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; Y ��ǥ�� �̿��ؼ� ���� ���� ��巹���� ����
    mov ax, word [ bp + 6 ]     ; �Ķ���� 2(ȭ�� ��ǥ Y)�� AX �������Ϳ� ����
    mov si, 160                 ; �� ������ ����Ʈ ��(2 * 80 �÷�)�� SI �������Ϳ� ����
    mul si                      ; AX �������Ϳ� SI �������͸� ���Ͽ� ȭ�� Y ��巹�� ���
    mov di, ax                  ; ���� ȭ�� Y ��巹���� DI �������Ϳ� ����
    
    ; X �·Ḧ �̿��ؼ� 2�� ���� �� ���� ��巹���� ����
    mov ax, word [ bp + 4 ]     ; �Ķ���� 1(ȭ�� ��ǥ X)�� AX �������Ϳ� ����
    mov si, 2                   ; �� ���ڸ� ��Ÿ���� ����Ʈ ��(2)�� SI �������Ϳ� ����
    mul si                      ; AX �������Ϳ� SI �������͸� ���Ͽ� ȭ�� X ��巹���� ���
    add di, ax                  ; ȭ�� Y ��巹���� ���� X ��巹���� ���ؼ�
                                ; ���� ���� �޸� ��巹���� ���
    
    ; ����� ���ڿ��� ��巹��      
    mov si, word [ bp + 8 ]     ; �Ķ���� 3(����� ���ڿ��� ��巹��)

.MESSAGELOOP:               ; �޽����� ����ϴ� ����
    mov cl, byte [ si ]     ; SI �������Ͱ� ����Ű�� ���ڿ� ��ġ���� �� ���ڸ� 
                            ; CL �������Ϳ� ����
                            ; CL �������ʹ� CX ���������� ���� 1����Ʈ�� �ǹ�
                            ; ���ڿ��� 1����Ʈ�� ����ϹǷ� CX ���������� ���� 1����Ʈ�� ���
    
    cmp cl, 0               ; ����� ���ڿ� 0�� ��
    je .MESSAGEEND          ; ������ ������ ���� 0�̸� ���ڿ��� ����Ǿ�����
                            ; �ǹ��ϹǷ� .MESSAGEEND�� �̵��Ͽ� ���� ��� ����

    mov byte [ es: di ], cl ; 0�� �ƴ϶�� ���� �޸� ��巹�� 0xB800:di�� ���ڸ� ���
    
    add si, 1               ; SI �������Ϳ� 1�� ���Ͽ� ���� ���ڿ��� �̵�
    add di, 2               ; DI �������Ϳ� 2�� ���Ͽ� ���� �޸��� ���� ���� ��ġ�� �̵�
                            ; ���� �޸𸮴� (����, �Ӽ�)�� ������ �����ǹǷ� ���ڸ� ����Ϸ���
                            ; 2�� ���ؾ� ��

    jmp .MESSAGELOOP        ; �޽��� ��� ������ �̵��Ͽ� ���� ���ڸ� ���

.MESSAGEEND:
    pop dx      ; �Լ����� ����� ���� DX �������ͺ��� ES �������ͱ����� ���ÿ�
    pop cx      ; ���Ե� ���� �̿��ؼ� ����
    pop ax      ; ������ ���� �������� �� �����Ͱ� ���� ���� ������ 
    pop di      ; �ڷᱸ��(Last-In, First-Out)�̹Ƿ� ����(push)�� ��������
    pop si      ; ����(pop) �ؾ� ��
    pop es
    pop bp      ; ���̽� ������ ��������(BP) ����
    ret         ; �Լ��� ȣ���� ���� �ڵ��� ��ġ�� ����
    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   ������ ����
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ��Ʈ �δ� ���� �޽���
MESSAGE1:    db 'MINT64 OS Boot Loader Start~!!', 0 ; ����� �޽��� ����
                                                    ; �������� 0���� �����Ͽ� .MESSAGELOOP���� 
                                                    ; ���ڿ��� ����Ǿ����� �� �� �ֵ��� ��
DISKERRORMESSAGE:       db  'DISK Error~!!', 0
IMAGELOADINGMESSAGE:    db  'OS Image Loading...', 0
LOADINGCOMPLETEMESSAGE: db  'Complete~!!', 0
MEMORYSIZE: db  '000000MB', 0

; ��ũ �б⿡ ���õ� ������
SECTORNUMBER:           db  0x02    ; OS �̹����� �����ϴ� ���� ��ȣ�� �����ϴ� ����
HEADNUMBER:             db  0x00    ; OS �̹����� �����ϴ� ��� ��ȣ�� �����ϴ� ����
TRACKNUMBER:            db  0x00    ; OS �̹����� �����ϴ� Ʈ�� ��ȣ�� �����ϴ� ����
    
times 510 - ( $ - $$ )    db    0x00    ; $ : ���� ������ ��巹��
                                        ; $$ : ���� ����(.text)�� ���� ��巹��
                                        ; $ - $$ : ���� ������ �������� �ϴ� ������
                                        ; 510 - ( $ - $$ ) : ������� ��巹�� 510����
                                        ; db 0x00 : 1����Ʈ�� �����ϰ� ���� 0x00
                                        ; time : �ݺ� ����
                                        ; ���� ��ġ���� ��巹�� 510���� 0x00���� ä��

db 0x55             ; 1����Ʈ�� �����ϰ� ���� 0x55
db 0xAA             ; 1����Ʈ�� �����ϰ� ���� 0xAA
                    ; ��巹�� 511, 512�� 0x55, 0xAA�� �Ἥ ��Ʈ ���ͷ� ǥ����
