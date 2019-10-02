# file      BootLoader.asm
# date      2008/11/27
# author    kkamagui 
#           Copyright(c)2008 All rights reserved by kkamagui
# brief     MINT64		;Bootloader Source file of OS

[ORG 0x00]			;Start Address of Code
[BITS 16]			;16Bits Code Under this line
TOTALSECTORCOUNT : dw 1024
SECTION .text			;text Segment define

jmp 0x07c0:START


;---Variables-----------;
    cent:    dw 00
    year:    dw 00
    fyear : dw 00
    mont:    dw 00
    day:    dw 00
    week:    dw 0

 
;---BootLoader Main function-----------;
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

	;---Interrupt----------;
	mov ah,04h
	int 1Ah

	;---Month data---------;

	mov bh,dh	;copy contents of month (dh) to bh
	shr bh,4
	mov ax, 10
	mul bh

	mov [mont], ax
	add bh,30h	;add 30h to convert to ascii

	mov ah,bh
	shr ax,8
	push ax		;month 10

	mov bh,dh
	and bh,0fh
	add [mont], bh

	add bh,30h
	mov ah,bh
	shr ax,8
	push ax		;month 1
	    
	;---Day data----------;

	mov bh,dl	;copy contents of day (dl) to bh
	shr bh,4
	mov ax, 10
	mul bh

	mov [day], ax

	add bh,30h	;add 30h to convert to ascii
	mov ah,bh
	shr ax,8
	push ax		;day 10
	mov bh,dl
	and bh,0fh
	add [day], bh
	add bh,30h
	mov ah,bh
	shr ax,8
	push ax 	;day 1

	;---Cent data---------;

	mov bh,ch	;copy contents of century (ch) to bh
	shr bh,4
	mov ax, 10
	mul bh
		    
	mov [cent], ax
	add bh,30h	;add 30h to convert to ascii
	mov ah,bh
	shr ax,8
	push ax		;cent 10
	mov bh,ch

	and bh,0fh
	add [cent], bh
	add bh,30h
	mov ah,bh
	shr ax,8
	push ax		;cent 1

	;---Year data---------;
	mov bh,cl	;copy contents of year (cl) to bh
	shr bh,4
	mov ax, 10
	mul bh

	mov [year], ax
	add bh,30h	;add 30h to convert to ascii

	mov ah,bh
	shr ax,8
	push ax		;year 10
	mov bh,cl

	and bh,0fh
	add [year], bh
	add bh,30h
	mov ah,bh
	shr ax,8
	push ax		;year 1


 
	;---Adding Year to Cent ---------;

	mov ax, 100
	mov bx, [cent]
	mul bx
	add ax, [year]
	mov [fyear], ax

	mov ax, [day]
	mov [week], ax 	;day

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
	add [week], al		;13*(month + 1)/5

	mov bl, 100
	mov ax, [fyear]
	div bl
	mov [year], ah
	mov [cent], al    

	add [week], ah		;year%100
	mov ax, [year]
	mov bl, 4
	div bl
	add [week], al		;(year%100)/4

	mov ax, [cent]
	mov bl, 4
	div bl
	mov ah, 0
	add [week], ax		;year/400

	mov ax, [cent]
	mov bl, 5
	mul bl
	add [week], ax		;5*(year/100)

	mov ax, [week]
	mov bl, 7
	div bl
	mov al, 0
	shr ax,8
	push ax


;--Load OS Image from Disk---------------;
;--Reset Disk before reading-------------;
RESETDISK:

	;---BIOS Reset Function Call-------------;
	;---Service number 0, Drive number (0=Floppy)-------;
	mov ax, 0
	mov dl, 0              
	int 0x13     

	;---Error Exception, if error occurs then jump Error Handler--------;
	jc  HANDLEDISKERROR
        
	;---Read Sector from disk--------;
	;---Memory address to copy contents of disk to memory is 0x10000---------;
	mov si, 0x1000                  ; Change address(0x10000) to copy OS image
					; into segment register value
	mov es, si                      ; Set ES segment register value
	mov bx, 0x0000                  ; Copy address 0x0000 to BX register
					; Final setting :: Address 0x1000:0000(0x10000)  
	mov di, word [ TOTALSECTORCOUNT ]

;---Read Disk Part-----------;
READDATA:                       
	cmp di, 0             
	je  READEND            
	sub di, 0x1            

	;---BIOS Read Function Call----------------;
	mov ah, 0x02                        ; BIOS service num 2 (Read Sector)
	mov al, 0x1                         ; Number of read sector is 1
	mov ch, byte [ TRACKNUMBER ]        ; Track num setting
	mov cl, byte [ SECTORNUMBER ]       ; Sector num setting
	mov dh, byte [ HEADNUMBER ]         ; Head num setting
	mov dl, 0x00                        ; Drive num setting (0=Floppy)
	int 0x13                            ; Interrupt Service Activate
	jc HANDLEDISKERROR                  ; Error Exception
    
	;---Track, Head, Sector Address-----------------;
	add si, 0x0020				
                        
	mov es, si          
    
	mov al, byte [ SECTORNUMBER ]       
	add al, 0x01                        
	mov byte [ SECTORNUMBER ], al       
	cmp al, 19                          
	jl READDATA                         

	xor byte [ HEADNUMBER ], 0x01       
	mov byte [ SECTORNUMBER ], 0x01     
    
	cmp byte [ HEADNUMBER ], 0x00       
	jne READDATA                        
    
	add byte [ TRACKNUMBER ], 0x01      
	jmp READDATA                        


;---Print message "complete to load OS image"-----------;
;---Excute loaded OS image-----------;
READEND:
	jmp 0x1000:0x0000
    
;Error Exception handler
HANDLEDISKERROR:
	jmp $
        
    

 


 


 
 


 


 


 

 



    


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
