# file      BootLoader.asm








# date      2008/11/27
# author    kkamagui 
#           Copyright(c)2008 All rights reserved by kkamagui
# brief     MINT64 OS�� ��Ʈ �δ� �ҽ� ����

[ORG 0x00]          ; �ڵ��� ���� ��巹���� 0x00���� ����
[BITS 16]           ; ������ �ڵ�� 16��Ʈ �ڵ�� ����
TOTALSECTORCOUNT : dw 1024
SECTION .text       ; text ����(���׸�Ʈ)�� ����

jmp 0x07c0:START



    cent:    dw 00

    year:    dw 00

    fyear : dw 00

    mont:    dw 00

    day:    dw 00

    week:    dw 0

 

START:

    mov ax, 0x07C0

    mov ds, ax ; start address

    mov ax, 0xB800

    mov es, ax ; video address

    mov si, 0
 
	mov ax,0x0000
	mov ss,ax
	mov sp,0xFFFE
	mov bp,0xFFFE





    mov ah,04h

    int 1Ah



;month

    mov bh,dh ;copy contents of month (dh) to bh

    shr bh,4

    mov ax, 10

    mul bh


    mov [mont], ax

    add bh,30h ;add 30h to convert to ascii

    mov ah,bh
    shr ax,8
    push ax;;;;month 10

    mov bh,dh

    and bh,0fh

    add [mont], bh

    add bh,30h
    mov ah,bh
   shr ax,8
    push ax;; month 1
    
;day

    mov bh,dl ;copy contents of day (dl) to bh

    shr bh,4

    mov ax, 10

    mul bh


    mov [day], ax

    add bh,30h ;add 30h to convert to ascii

    mov ah,bh
   shr ax,8
    push ax ;;;;; day10
    mov bh,dl

    and bh,0fh

    add [day], bh

    add bh,30h

   mov ah,bh
   shr ax,8
    push ax ;;;;;;;;; day 1
;cent

    mov bh,ch ;copy contents of century (ch) to bh

    shr bh,4

    mov ax, 10

    mul bh
	    

    mov [cent], ax

    add bh,30h ;add 30h to convert to ascii
   mov ah,bh
   shr ax,8
    push ax ;;;;;;;;;;;;;; cent 10
    mov bh,ch

    and bh,0fh

    add [cent], bh

    add bh,30h
    mov ah,bh
   shr ax,8
    push ax ;;;;;;;;;;; cent 1
;year

    mov bh,cl ;copy contents of year (cl) to bh

    shr bh,4

    mov ax, 10

    mul bh


    mov [year], ax

    add bh,30h ;add 30h to convert to ascii

   mov ah,bh
   shr ax,8
    push ax ;;;;;;;;;;;;;;;;;;;;;;year 10
    mov bh,cl

    and bh,0fh

    add [year], bh

    add bh,30h
    mov ah,bh
   shr ax,8
    push ax ;;;;;;;;;;;;;;;;;;;;; year 1


 



    


    mov ax, 100

    mov bx, [cent]

    mul bx

    add ax, [year]

    mov [fyear], ax
    
    
	    


    mov ax, [day]

    mov [week], ax ;;;; day

    

    xor ax, ax

    mov ax, [mont]

    cmp ax, 2

    Ja .nocvt

 

    xor ax, ax

    mov ax, [fyear]

    sub ax, 1

    mov [fyear], ax

    xor ax, ax

    mov ax, [mont]

    add ax, 12

 

.nocvt:

    add ax, 1

    mov bx, 13

    mul bx

    mov bl, 5

    div bl

    add [week], al ;;;; 13*(month + 1)/5

 

    mov bl, 100

    mov ax, [fyear]

    div bl

    mov [year], ah

    mov [cent], al    

 

    add [week], ah ;;;; year%100

 

    mov ax, [year]

    mov bl, 4

    div bl

    add [week], al ;;;; (year%100)/4

 

    mov ax, [cent]

    mov bl, 4

    div bl

    mov ah, 0

    add [week], ax ;;;; year/400

 

    mov ax, [cent]

    mov bl, 5

    mul bl

    add [week], ax ;;;; 5*(year/100)

 

   

    mov ax, [week]

    mov bl, 7

    div bl
    mov al, 0
    shr ax,8
    push ax


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

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; �ε��� ���� OS �̹��� ����    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    jmp 0x1000:0x0000
    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   �Լ� �ڵ� ����
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ��ũ ������ ó���ϴ� �Լ�   
HANDLEDISKERROR:
    
    jmp $                   ; ���� ��ġ���� ���� ���� ����

; �޽����� ����ϴ� �Լ�
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   ������ ����
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ��Ʈ �δ� ���� �޽���
             
    

 


 


 
 


 


 


 

 



    


DISKERRORMESSAGE:       db  'DISK Error~!!', 0
IMAGELOADINGMESSAGE:    db  'OS.', 0
LOADINGCOMPLETEMESSAGE: db  'Complete~!!', 0

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
