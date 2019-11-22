/**
 *  file    InterruptHandler.c
 *  date    2009/01/24
 *  author  kkamagui
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   인터럽트 및 예외 핸들러에 관련된 소스 파일
 */

#include "InterruptHandler.h"
#include "PIC.h"
#include "Keyboard.h"
#include "Console.h"
#include "Utility.h"
#include "Task.h"
#include "Descriptor.h"

/**
 *  공통으로 사용하는 예외 핸들러
 */
void kCommonExceptionHandler( int iVectorNumber, QWORD qwErrorCode )
{
    char vcBuffer[ 3 ] = { 0, };

    // 인터럽트 벡터를 화면 오른쪽 위에 2자리 정수로 출력
    vcBuffer[ 0 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 1 ] = '0' + iVectorNumber % 10;
    
    kPrintStringXY( 0, 0, "====================================================" );
    kPrintStringXY( 0, 1, "                 Exception Occur~!!!!               " );
    kPrintStringXY( 0, 2, "                    Vector:                         " );
    kPrintStringXY( 27, 2, vcBuffer );
    kPrintStringXY( 0, 3, "====================================================" );

    while( 1 ) ;
}

/**
 *  공통으로 사용하는 인터럽트 핸들러
 */
void kCommonInterruptHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iCommonInterruptCount = 0;

    //=========================================================================
    // 인터럽트가 발생했음을 알리려고 메시지를 출력하는 부분
    // 인터럽트 벡터를 화면 오른쪽 위에 2자리 정수로 출력
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // 발생한 횟수 출력
    vcBuffer[ 8 ] = '0' + g_iCommonInterruptCount;
    g_iCommonInterruptCount = ( g_iCommonInterruptCount + 1 ) % 10;
    kPrintStringXY( 70, 0, vcBuffer );
    //=========================================================================
    
    // EOI 전송
    kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );
}

/**
 *  키보드 인터럽트의 핸들러
 */
void kKeyboardHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iKeyboardInterruptCount = 0;
    BYTE bTemp;

    //=========================================================================
    // 인터럽트가 발생했음을 알리려고 메시지를 출력하는 부분
    // 인터럽트 벡터를 화면 왼쪽 위에 2자리 정수로 출력
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // 발생한 횟수 출력
    vcBuffer[ 8 ] = '0' + g_iKeyboardInterruptCount;
    g_iKeyboardInterruptCount = ( g_iKeyboardInterruptCount + 1 ) % 10;
    kPrintStringXY( 0, 0, vcBuffer );
    //=========================================================================

    // 키보드 컨트롤러에서 데이터를 읽어서 ASCII로 변환하여 큐에 삽입
    if( kIsOutputBufferFull() == TRUE )
    {
        bTemp = kGetKeyboardScanCode();
        kConvertScanCodeAndPutQueue( bTemp );
    }

    // EOI 전송
    kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );
}


// 타이머 인터럽트 핸들러
void kTimerHandler(int iVectorNumber)
{
	char vcBuffer[] = "[INT:  , ]";
	static int g_iTimerInterruptCount = 0;

	//=====================================================================
	// 인터럽트가 발생했음을 알리려고 메시지률 출력하는 부분
	// 인터럽트 벡터를 화면 오른쪽 위에 2자리 정수로 출력
	vcBuffer[5] = '0' + iVectorNumber / 10;
	vcBuffer[6] = '0' + iVectorNumber % 10;
	// 발생 횟수 출력
	vcBuffer[8] = '0' + g_iTimerInterruptCount;
	g_iTimerInterruptCount = (g_iTimerInterruptCount + 1) % 10;
	kPrintStringXY(70, 0, vcBuffer);
	//=====================================================================

	kSendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);
	g_qwTickCount++;

	kDecreaseProcessorTime();
	if (kIsProcessorTimeExpired() == TRUE)
	{
		kScheduleInInterrupt();
	}
}




void getHex(char* hexBuffer, QWORD input)
{
	char reHex[16] = {0, };
	int position = 0;
	int j = 0;

	while(1)
	{
		int mod = input % 16;

		if(mod < 10)
		{
			reHex[position] = 48 + mod;
		}
		else
		{
			reHex[position] = 65 + (mod - 10);
		}

		input = input / 16;
		position++;

		if(input == 0)
		{
			break;
		}
	}

	for( int i = position -1; i >= 0; i--)
	{
		hexBuffer[j] = reHex[i];
		j++;
	}
}

void fixPageEntry(QWORD address)
{
	QWORD* page;
	QWORD entryPoint = address >> 21;
	QWORD entryAddress;

	if(entryPoint == 0)
	{
		entryPoint = address >> 12;
		entryAddress = 0x102000 + ( entryPoint * 8 );
		page = (QWORD*) entryAddress;
	}
	else
	{
		entryAddress = 0x103000 + ( entryPoint * 8 );
		page = (QWORD*) entryAddress;
	}

	*page = *page | 0x03;
}

void kPagingHandler( int iVectorNumber, QWORD qwErrorCode, QWORD address )
{
	char hexBuffer[16] = {0, };
	getHex(hexBuffer, address);

	char vcBuffer[3] = {0, };

	vcBuffer[0] = '0' + iVectorNumber / 10;
	vcBuffer[1] = '0' + iVectorNumber % 10;

	if( qwErrorCode == 3 )
	{
		kPrintStringXY( 0, 0, "==========================================================");
		kPrintStringXY( 0, 1, "              Protection Fault Occurs~!                   ");
		kPrintStringXY( 0, 2, "              Address :                                   ");
		kPrintStringXY( 0, 3, "==========================================================");
		kPrintStringXY( 23, 2, hexBuffer );
	}
	else if( qwErrorCode == 2 )
	{
		kPrintStringXY( 0, 0, "==========================================================");
		kPrintStringXY( 0, 1, "              Page Fault Occurs~!                         ");
		kPrintStringXY( 0, 2, "              Address :                                   ");
		kPrintStringXY( 0, 3, "==========================================================");
		kPrintStringXY( 23, 2, hexBuffer );
	}
	else
	{
		kPrintStringXY( 0, 0, "==========================================================");
		kPrintStringXY( 0, 1, "              Error Occurs~!                              ");
		kPrintStringXY( 0, 2, "              Vector :                                    ");
		kPrintStringXY( 0, 3, "==========================================================");
		kPrintStringXY( 23, 2, vcBuffer );
	}

	fixPageEntry(address);
}
