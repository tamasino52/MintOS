# file      EntryPoint.s
# date      2008/11/27
# author    kkamagui 
#           Copyright(c)2008 All rights reserved by kkamagui
# brief     보호 모드 커널 엔트리 포인트에 관련된 소스 파일

[ORG 0x00]          ; 코드의 시작 어드레스를 0x00으로 설정
[BITS 16]           ; 이하의 코드는 16비트 코드로 설정

SECTION .text       ; text 섹션(세그먼트)을 정의

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   코드 영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
START:
    mov ax, 0x1000  ; 보호 모드 엔트리 포인트의 시작 어드레스(0x10000)를 
                    ; 세그먼트 레지스터 값으로 변환
    mov ds, ax      ; DS 세그먼트 레지스터에 설정
    mov es, ax      ; ES 세그먼트 레지스터에 설정
    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; A20 게이트를 활성화
    ; BIOS를 이용한 전환이 실패했을 때 시스템 컨트롤 포트로 전환 시도
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; BIOS 서비스를 사용해서 A20 게이트를 활성화
    mov ax, 0x2401          ; A20 게이트 활성화 서비스 설정
    int 0x15                ; BIOS 인터럽트 서비스 호출

    jc .A20GATEERROR        ; A20 게이트 활성화가 성공했는지 확인
    jmp .A20GATESUCCESS

.A20GATEERROR:
    ; 에러 발생 시, 시스템 컨트롤 포트로 전환 시도
    in al, 0x92     ; 시스템 컨트롤 포트(0x92)에서 1 바이트를 읽어 AL 레지스터에 저장
    or al, 0x02     ; 읽은 값에 A20 게이트 비트(비트 1)를 1로 설정
    and al, 0xFE    ; 시스템 리셋 방지를 위해 0xFE와 AND 연산하여 비트 0를 0으로 설정
    out 0x92, al    ; 시스템 컨트롤 포트(0x92)에 변경된 값을 1 바이트 설정
    
.A20GATESUCCESS:
    cli             ; 인터럽트가 발생하지 못하도록 설정
    lgdt [ GDTR ]   ; GDTR 자료구조를 프로세서에 설정하여 GDT 테이블을 로드

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; 보호모드로 진입
    ; Disable Paging, Disable Cache, Internal FPU, Disable Align Check, 
    ; Enable ProtectedMode
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov eax, 0x4001003B ; PG=0, CD=1, NW=0, AM=0, WP=1!!!!, NE=1, ET=1, TS=1, EM=0, MP=1, PE=1
    mov cr0, eax        ; CR0 컨트롤 레지스터에 위에서 저장한 플래그를 설정하여 
                        ; 보호 모드로 전환

    ; 커널 코드 세그먼트를 0x00을 기준으로 하는 것으로 교체하고 EIP의 값을 0x00을 기준으로 재설정
    ; CS 세그먼트 셀렉터 : EIP
    jmp dword 0x18: ( PROTECTEDMODE - $$ + 0x10000 )

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; 보호 모드로 진입
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
[BITS 32]               ; 이하의 코드는 32비트 코드로 설정
PROTECTEDMODE:
    mov ax, 0x20        ; 보호 모드 커널용 데이터 세그먼트 디스크립터를 AX 레지스터에 저장
    mov ds, ax          ; DS 세그먼트 셀렉터에 설정
    mov es, ax          ; ES 세그먼트 셀렉터에 설정
    mov fs, ax          ; FS 세그먼트 셀렉터에 설정
    mov gs, ax          ; GS 세그먼트 셀렉터에 설정
    
    ; 스택을 0x00000000~0x0000FFFF 영역에 64KB 크기로 생성
    mov ss, ax          ; SS 세그먼트 셀렉터에 설정
    mov esp, 0xFFFE     ; ESP 레지스터의 어드레스를 0xFFFE로 설정
    mov ebp, 0xFFFE     ; EBP 레지스터의 어드레스를 0xFFFE로 설정
    
    ; 화면에 보호 모드로 전환되었다는 메시지를 찍는다.
    push ( SWITCHSUCCESSMESSAGE - $$ + 0x10000 )    ; 출력할 메시지의 어드레스르 스택에 삽입
    push 2                                          ; 화면 Y 좌표(2)를 스택에 삽입
    push 0                                          ; 화면 X 좌표(0)를 스택에 삽입
    call PRINTMESSAGE                               ; PRINTMESSAGE 함수 호출
    add esp, 12                                     ; 삽입한 파라미터 제거

	call E820

    jmp dword 0x18: 0x10200 ; C 언어 커널이 존재하는 0x10200 어드레스로 이동하여 C 언어 커널 수행


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   함수 코드 영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 데이터

Date_Mod db '14 May 2010 v. 2r5 ',0
Signon db '*** Testing INT 15h AX=E820h Big Memory '
       db 'Services - System Memory Map ***',0
Fn_na db 'Sorry, unsupported Fn, Int 15h EAX=E820',0
Mbase db ' <-Base  ',0
Mlen  db ' <-Length  ',0
Mtype db ' <-Type  ',0
Mtypes db ' Types: 1=Avail. to OS, 2=Not Avail., '
   db '3=ACPI Avail. to OS, 4=NVS Not Avail.',0

Fn_nak_E801 db 'Sorry, unsupported Fn, Int 15h EAX=E801',0
Fn_ack_E801 db ' Supported Fn, Int 15h EAX=E801',0

Fn_nak_E881 db 'Sorry, unsupported Fn, Int 15h EAX=E881',0
Fn_ack_E881 db ' Supported Fn, Int 15h EAX=E881',0

strEAX db 'EAX=',0
strEBX db 'EBX=',0
strECX db 'ECX=',0
strEDX db 'EDX=',0

valEAX dd 0
valEBX dd 0
valECX dd 0
valEDX dd 0


; 버퍼사이즈 인터럽트 함수
;;--------------------------------------------------------60
E820:
  mov  edx, Signon
  call putstr
  call newline

  clc				;; just in case..
  mov  edi, buffer	;; offset re dseg
  xor  ebx, ebx
  xor  esi, esi		; counter
.top:
  mov  eax, 0E820h
  mov  edx, 534D4150h	; 'SMAP'
  mov  ecx, 20		;; length of packet
  int  15h
  jc   error
  inc  esi			; bump counter
  add  edi, ecx
  or   ebx, ebx		;; test continuation value=0
  jz   done
  jmp  short .top		;; loop, building map table
done:
  mov  ecx, esi		; count
  mov  esi, buffer

show:		;;retrieve values from table & print them.
  mov  eax, [esi + mm_ent.base + 4]
  call showeax
  call separator
  mov  eax, [esi + mm_ent.base]
  call showeax
  mov  edx, Mbase
  call putstr

  mov  eax, [esi + mm_ent.len + 4]
  call showeax
  call separator
  mov  eax, [esi + mm_ent.len]
  call showeax
  mov  edx, Mlen
  call putstr

  mov  eax, [esi + mm_ent.type]
  call showeax
  mov  edx, Mtype
  call putstr
  call newline

  add  esi, mm_ent_size
  loop show
  ;;---eoshow---

  mov  edx, Mtypes
  call putstr

  jmp  exit

error:	;; report unsupported Fn
  mov  edx, Fn_na
  call putstr
  call newline

exit:
  RET


;;--------------------------------------------------------60
;; 유틸리티
;;--------------------------------------------------------60
dmp4regs:

  mov  edx, strEAX
  call putstr
  mov  eax, [valEAX]
  call showeax
  call virtbar

  mov  edx, strEBX
  call putstr
  mov  eax, [valEBX]
  call showeax
  call virtbar

  mov  edx, strECX
  call putstr
  mov  eax, [valECX]
  call showeax
  call virtbar

  mov  edx, strEDX
  call putstr
  mov  eax, [valEDX]
  call showeax
  call newline
  call newline

  RET
;------------------
showeax:
  push cx
  mov  cx, 8
.top:
  rol  eax, 4
  push eax
  and  al, 0Fh
  cmp  al, 0Ah
  sbb  al, 69h
  das
  call putc
  pop  eax
  loop .top
  pop  cx
  RET
;-------------------
separator:
  push ax
  mov  al, 27h
  call putc
  pop  ax
  RET
;-------------------
virtbar:
  push ax
  mov  al, 7Ch
  call putc
  pop  ax
  RET
;-------------------
newline:
  push ax
  mov  al, 13
  call putc
  mov  al, 10
  call putc
  pop  ax
  RET
;--------------------
tab:
  push ax
  mov  al,9
  call putc
  pop  ax
  RET
;--------------------
putc:  ;; chr arrives in AL
  pusha

  mov  ah, 0Eh ;; Fn tty
  mov  ebx, 0  ;; BH pg no
  int  10h

  popa
  RET
;------------------
putstr:  ;;Str Offset supplied in EDX
  mov  al, [edx]
  cmp  al, 0
  jz   endstr
  call putc
  inc  edx
  jmp  short putstr
endstr:
  RET


buffer:  TIMES 200h db 0
stkbase: TIMES 80h dw 0
stktop:




; 메시지를 출력하는 함수
;   스택에 x 좌표, y 좌표, 문자열
PRINTMESSAGE:
    push ebp        ; 베이스 포인터 레지스터(BP)를 스택에 삽입
    mov ebp, esp    ; 베이스 포인터 레지스터(BP)에 스택 포인터 레지스터(SP)의 값을 설정
    push esi        ; 함수에서 임시로 사용하는 레지스터로 함수의 마지막 부분에서
    push edi        ; 스택에 삽입된 값을 꺼내 원래 값으로 복원
    push eax
    push ecx
    push edx
    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; X, Y의 좌표로 비디오 메모리의 어드레스를 계산함
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; Y 좌표를 이용해서 먼저 라인 어드레스를 구함
    mov eax, dword [ ebp + 12 ] ; 파라미터 2(화면 좌표 Y)를 EAX 레지스터에 설정
    mov esi, 160                ; 한 라인의 바이트 수(2 * 80 컬럼)를 ESI 레지스터에 설정
    mul esi                     ; EAX 레지스터와 ESI 레지스터를 곱하여 화면 Y 어드레스 계산
    mov edi, eax                ; 계산된 화면 Y 어드레스를 EDI 레지스터에 설정
    
    ; X 좌료를 이용해서 2를 곱한 후 최종 어드레스를 구함
    mov eax, dword [ ebp + 8 ]  ; 파라미터 1(화면 좌표 X)를 EAX 레지스터에 설정
    mov esi, 2                  ; 한 문자를 나타내는 바이트 수(2)를 ESI 레지스터에 설정
    mul esi                     ; EAX 레지스터와 ESI 레지스터를 곱하여 화면 X 어드레스를 계산
    add edi, eax                ; 화면 Y 어드레스와 계산된 X 어드레스를 더해서
                                ; 실제 비디오 메모리 어드레스를 계산
                                
    ; 출력할 문자열의 어드레스      
    mov esi, dword [ ebp + 16 ] ; 파라미터 3(출력할 문자열의 어드레스)

.MESSAGELOOP:               ; 메시지를 출력하는 루프
    mov cl, byte [ esi ]    ; ESI 레지스터가 가리키는 문자열 위치에서 한 문자를 
                            ; CL 레지스터에 복사
                            ; CL 레지스터는 ECX 레지스터의 하위 1바이트를 의미
                            ; 문자열은 1바이트면 충분하므로 ECX 레지스터의 하위 1바이트만 사용

    cmp cl, 0               ; 복사된 문자와 0을 비교
    je .MESSAGEEND          ; 복사한 문자의 값이 0이면 문자열이 종료되었음을
                            ; 의미하므로 .MESSAGEEND로 이동하여 문자 출력 종료

    mov byte [ edi + 0xB8000 ], cl  ; 0이 아니라면 비디오 메모리 어드레스 
                                    ; 0xB8000 + EDI 에 문자를 출력
    
    add esi, 1              ; ESI 레지스터에 1을 더하여 다음 문자열로 이동
    add edi, 2              ; EDI 레지스터에 2를 더하여 비디오 메모리의 다음 문자 위치로 이동
                            ; 비디오 메모리는 (문자, 속성)의 쌍으로 구성되므로 문자만 출력하려면
                            ; 2를 더해야 함

    jmp .MESSAGELOOP        ; 메시지 출력 루프로 이동하여 다음 문자를 출력

.MESSAGEEND:
    pop edx     ; 함수에서 사용이 끝난 EDX 레지스터부터 EBP 레지스터까지를 스택에
    pop ecx     ; 삽입된 값을 이용해서 복원
    pop eax     ; 스택은 가장 마지막에 들어간 데이터가 가장 먼저 나오는
    pop edi     ; 자료구조(Last-In, First-Out)이므로 삽입(push)의 역순으로
    pop esi     ; 제거(pop) 해야 함
    pop ebp     ; 베이스 포인터 레지스터(BP) 복원
    ret         ; 함수를 호출한 다음 코드의 위치로 복귀

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   데이터 영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 아래의 데이터들을 8byte에 맞춰 정렬하기 위해 추가
align 8, db 0

; GDTR의 끝을 8byte로 정렬하기 위해 추가
dw 0x0000
; GDTR 자료구조 정의
GDTR:
    dw GDTEND - GDT - 1         ; 아래에 위치하는 GDT 테이블의 전체 크기
    dd ( GDT - $$ + 0x10000 )   ; 아래에 위치하는 GDT 테이블의 시작 어드레스

; GDT 테이블 정의
GDT:
    ; 널(NULL) 디스크립터, 반드시 0으로 초기화해야 함
    NULLDescriptor:
        dw 0x0000
        dw 0x0000
        db 0x00
        db 0x00
        db 0x00
        db 0x00

    ; IA-32e 모드 커널용 코드 세그먼트 디스크립터
    IA_32eCODEDESCRIPTOR:     
        dw 0xFFFF       ; Limit [15:0]
        dw 0x0000       ; Base [15:0]
        db 0x00         ; Base [23:16]
        db 0x9A         ; P=1, DPL=0, Code Segment, Execute/Read
        db 0xAF         ; G=1, D=0, L=1, Limit[19:16]
        db 0x00         ; Base [31:24]  
        
    ; IA-32e 모드 커널용 데이터 세그먼트 디스크립터
    IA_32eDATADESCRIPTOR:
        dw 0xFFFF       ; Limit [15:0]
        dw 0x0000       ; Base [15:0]
        db 0x00         ; Base [23:16]
        db 0x92         ; P=1, DPL=0, Data Segment, Read/Write
        db 0xAF         ; G=1, D=0, L=1, Limit[19:16]
        db 0x00         ; Base [31:24]
        
    ; 보호 모드 커널용 코드 세그먼트 디스크립터
    CODEDESCRIPTOR:     
        dw 0xFFFF       ; Limit [15:0]
        dw 0x0000       ; Base [15:0]
        db 0x00         ; Base [23:16]
        db 0x9A         ; P=1, DPL=0, Code Segment, Execute/Read
        db 0xCF         ; G=1, D=1, L=0, Limit[19:16]
        db 0x00         ; Base [31:24]  
        
    ; 보호 모드 커널용 데이터 세그먼트 디스크립터
    DATADESCRIPTOR:
        dw 0xFFFF       ; Limit [15:0]
        dw 0x0000       ; Base [15:0]
        db 0x00         ; Base [23:16]
        db 0x92         ; P=1, DPL=0, Data Segment, Read/Write
        db 0xCF         ; G=1, D=1, L=0, Limit[19:16]
        db 0x00         ; Base [31:24]
GDTEND:

; 보호 모드로 전환되었다는 메시지
SWITCHSUCCESSMESSAGE: db 'Switch To Protected Mode Success~!!', 0

times 512 - ( $ - $$ )  db  0x00    ; 512바이트를 맞추기 위해 남은 부분을 0으로 채움
