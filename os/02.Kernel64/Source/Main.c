/**
 *  file    Main.c
 *  date    2009/01/02
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   C 언어로 작성된 커널의 엔트리 포인트 파일
 */

#include "Types.h"
#include "Keyboard.h"

// 함수 선언
void kPrintString( int iX, int iY, const char* pcString );
void kAB8000PrintString(int iX, int iY, const char* pcString);

/**
 *  아래 함수는 C 언어 커널의 시작 부분임
 */
void Main( void )
{

	char vcTemp[2] = { 0, };
	BYTE bFlags;
	BYTE bTemp;
	int i = 0;


    kPrintString( 0, 10, "Switch To IA-32e Mode Success~!!" );
    kPrintString( 0, 11, "IA-32e C Language Kernel Start..............[Pass]" );
	kAB8000PrintString(0, 12, "This message is printed through the video memory relocated to 0xAB8000");
	kPrintString(0, 13, "Read from 0x1fe000 [     ]");
	char * tmp = (char *) 0x1FE000;
	kPrintString(22, 13, "OK");

	kPrintString(0, 14, "Write to 0x1fe000 [     ]");
	*tmp = 'p';
	kPrintString(21, 14, "OK");

	kPrintString(0, 15, "Read from 0x1ff000 [     ]");
	tmp = (char *) 0x1FF000;
	kPrintString(22, 15, "OK");

	kPrintString(0, 16, "Keyboard Activate..............[     ]");

	// 키보드 활성화
	if (kActivateKeyboard() == TRUE)
	{
		kPrintString(34, 16, "Pass");
		kChangeKeyboardLED(FALSE, FALSE, FALSE);
	}
	else
	{
		kPrintString(34, 16, "Fail");
		while (1);
	}

	while (1)
	{
		// 출력 버퍼가 차있으면 스캔코드를 읽을 수 없음
		if (kIsOutputBufferFull() == TRUE)
		{
			//출력 버퍼에서 스캔코드를 읽어서 저장
			bTemp = kGetKeyboardScanCode();

			//스캔코드를 아스키 코드로 변환하는 함수를 호출하여 아스키 코드와 눌림 또는 떨어짐 정보를 반환
			if (kConvertScanCodeToASCIICode(bTemp, &(vcTemp[0]), &bFlags) == TRUE)
			{
				//키가 눌러졌으면 키의 아스키코드값을 화면에 출력
				if (bFlags & KEY_FLAGS_DOWN)
				{
					kPrintString(i++, 17,  vcTemp);
				}
			}
		}
	}
}

/**
 *  문자열을 X, Y 위치에 출력
 */
void kPrintString( int iX, int iY, const char* pcString )
{
    CHARACTER* pstScreen = ( CHARACTER* ) 0xB8000;
    int i;
    
    // X, Y 좌표를 이용해서 문자열을 출력할 어드레스를 계산
    pstScreen += ( iY * 80 ) + iX;

    // NULL이 나올 때까지 문자열 출력
    for( i = 0 ; pcString[ i ] != 0 ; i++ )
    {
        pstScreen[ i ].bCharactor = pcString[ i ];
    }
}

void kAB8000PrintString(int iX, int iY, const char* pcString)
{
	CHARACTER* pstScreen = (CHARACTER*) 0xAB8000;
	int i;

	pstScreen += (iY * 80) + iX;

	for(i = 0; pcString[i] != 0; i++){
		pstScreen[i].bCharactor = pcString[i];
	}
}

void PageFaultException() {

}

void ProtectionFaultException() {

}