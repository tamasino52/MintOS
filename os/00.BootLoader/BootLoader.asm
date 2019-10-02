# file      BootLoader.asm








# date      2008/11/27
# author    kkamagui 
#           Copyright(c)2008 All rights reserved by kkamagui
# brief     MINT64 OS의 부트 로더 소스 파일

[ORG 0x00]          ; 코드의 시작 어드레스를 0x00으로 설정
[BITS 16]           ; 이하의 코드는 16비트 코드로 설정
TOTALSECTORCOUNT : dw 1024
SECTION .text       ; text 섹션(세그먼트)을 정의

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
    ; 디스크에서 OS 이미지를 로딩
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; 디스크를 읽기 전에 먼저 리셋
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
RESETDISK:                          ; 디스크를 리셋하는 코드의 시작
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; BIOS Reset Function 호출
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; 서비스 번호 0, 드라이브 번호(0=Floppy)
    
    mov ax, 0
    mov dl, 0              
    int 0x13     
    ; 에러가 발생하면 에러 처리로 이동
    jc  HANDLEDISKERROR
        
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; 디스크에서 섹터를 읽음
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; 디스크의 내용을 메모리로 복사할 어드레스(ES:BX)를 0x10000으로 설정
    mov si, 0x1000                  ; OS 이미지를 복사할 어드레스(0x10000)를 
                                    ; 세그먼트 레지스터 값으로 변환
    mov es, si                      ; ES 세그먼트 레지스터에 값 설정
    mov bx, 0x0000                  ; BX 레지스터에 0x0000을 설정하여 복사할 
                                    ; 어드레스를 0x1000:0000(0x10000)으로 최종 설정

    mov di, word [ TOTALSECTORCOUNT ] ; 복사할 OS 이미지의 섹터 수를 DI 레지스터에 설정

READDATA:                           ; 디스크를 읽는 코드의 시작
    ; 모든 섹터를 다 읽었는지 확인
    cmp di, 0               ; 복사할 OS 이미지의 섹터 수를 0과 비교
    je  READEND             ; 복사할 섹터 수가 0이라면 다 복사 했으므로 READEND로 이동
    sub di, 0x1             ; 복사할 섹터 수를 1 감소

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; BIOS Read Function 호출
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ah, 0x02                        ; BIOS 서비스 번호 2(Read Sector)
    mov al, 0x1                         ; 읽을 섹터 수는 1
    mov ch, byte [ TRACKNUMBER ]        ; 읽을 트랙 번호 설정
    mov cl, byte [ SECTORNUMBER ]       ; 읽을 섹터 번호 설정
    mov dh, byte [ HEADNUMBER ]         ; 읽을 헤드 번호 설정
    mov dl, 0x00                        ; 읽을 드라이브 번호(0=Floppy) 설정
    int 0x13                            ; 인터럽트 서비스 수행
    jc HANDLEDISKERROR                  ; 에러가 발생했다면 HANDLEDISKERROR로 이동
    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; 복사할 어드레스와 트랙, 헤드, 섹터 어드레스 계산
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    add si, 0x0020      ; 512(0x200)바이트만큼 읽었으므로, 이를 세그먼트 레지스터
                        ; 값으로 변환
    mov es, si          ; ES 세그먼트 레지스터에 더해서 어드레스를 한 섹터 만큼 증가
    
    ; 한 섹터를 읽었으므로 섹터 번호를 증가시키고 마지막 섹터(18)까지 읽었는지 판단
    ; 마지막 섹터가 아니면 섹터 읽기로 이동해서 다시 섹터 읽기 수행
    mov al, byte [ SECTORNUMBER ]       ; 섹터 번호를 AL 레지스터에 설정
    add al, 0x01                        ; 섹터 번호를 1 증가
    mov byte [ SECTORNUMBER ], al       ; 증가시킨 섹터 번호를 SECTORNUMBER에 다시 설정
    cmp al, 19                          ; 증가시킨 섹터 번호를 19와 비교
    jl READDATA                         ; 섹터 번호가 19 미만이라면 READDATA로 이동
    
    ; 마지막 섹터까지 읽었으면(섹터 번호가 19이면) 헤드를 토글(0->1, 1->0)하고, 
    ; 섹터 번호를 1로 설정
    xor byte [ HEADNUMBER ], 0x01       ; 헤드 번호를 0x01과 XOR하여 토글(0->1, 1->1)
    mov byte [ SECTORNUMBER ], 0x01     ; 섹터 번호를 다시 1로 설정
    
    ; 만약 헤드가 1->0로 바뀌었으면 양쪽 헤드를 모두 읽은 것이므로 아래로 이동하여
    ; 트랙 번호를 1 증가
    cmp byte [ HEADNUMBER ], 0x00       ; 헤드 번호를 0x00과 비교
    jne READDATA                        ; 헤드 번호가 0이 아니면 READDATA로 이동
    
    ; 트랙을 1 증가시킨 후, 다시 섹터 읽기로 이동
    add byte [ TRACKNUMBER ], 0x01      ; 트랙 번호를 1 증가
    jmp READDATA                        ; READDATA로 이동


READEND:

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; OS 이미지가 완료되었다는 메시지를 출력
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; 로딩한 가상 OS 이미지 실행    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    jmp 0x1000:0x0000
    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   함수 코드 영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 디스크 에러를 처리하는 함수   
HANDLEDISKERROR:
    
    jmp $                   ; 현재 위치에서 무한 루프 수행

; 메시지를 출력하는 함수
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   데이터 영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 부트 로더 시작 메시지
             
    

 


 


 
 


 


 


 

 



    


DISKERRORMESSAGE:       db  'DISK Error~!!', 0
IMAGELOADINGMESSAGE:    db  'OS.', 0
LOADINGCOMPLETEMESSAGE: db  'Complete~!!', 0

; 디스크 읽기에 관련된 변수들
SECTORNUMBER:           db  0x02    ; OS 이미지가 시작하는 섹터 번호를 저장하는 영역
HEADNUMBER:             db  0x00    ; OS 이미지가 시작하는 헤드 번호를 저장하는 영역
TRACKNUMBER:            db  0x00    ; OS 이미지가 시작하는 트랙 번호를 저장하는 영역
times 510 - ( $ - $$ )    db    0x00    ; $ : 현재 라인의 어드레스
                                        ; $$ : 현재 섹션(.text)의 시작 어드레스
                                        ; $ - $$ : 현재 섹션을 기준으로 하는 오프셋
                                        ; 510 - ( $ - $$ ) : 현재부터 어드레스 510까지
                                        ; db 0x00 : 1바이트를 선언하고 값은 0x00
                                        ; time : 반복 수행
                                        ; 현재 위치에서 어드레스 510까지 0x00으로 채움

db 0x55             ; 1바이트를 선언하고 값은 0x55
db 0xAA             ; 1바이트를 선언하고 값은 0xAA
                    ; 어드레스 511, 512에 0x55, 0xAA를 써서 부트 섹터로 표기함
