/**
 *  file    InterruptHandler.c
 *  date    2009/01/24
 *  author  kkamagui
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   ���ͷ�Ʈ �� ���� �ڵ鷯�� ���õ� �ҽ� ����
 */

#include "InterruptHandler.h"
#include "PIC.h"
#include "Keyboard.h"
#include "Console.h"

/**
 *  �������� ����ϴ� ���� �ڵ鷯
 */
void kCommonExceptionHandler( int iVectorNumber, QWORD qwErrorCode )
{
    char vcBuffer[ 3 ] = { 0, };

    // ���ͷ�Ʈ ���͸� ȭ�� ������ ���� 2�ڸ� ������ ���
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
 *  �������� ����ϴ� ���ͷ�Ʈ �ڵ鷯
 */
void kCommonInterruptHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iCommonInterruptCount = 0;

    //=========================================================================
    // ���ͷ�Ʈ�� �߻������� �˸����� �޽����� ����ϴ� �κ�
    // ���ͷ�Ʈ ���͸� ȭ�� ������ ���� 2�ڸ� ������ ���
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // �߻��� Ƚ�� ���
    vcBuffer[ 8 ] = '0' + g_iCommonInterruptCount;
    g_iCommonInterruptCount = ( g_iCommonInterruptCount + 1 ) % 10;
    kPrintStringXY( 70, 0, vcBuffer );
    //=========================================================================
    
    // EOI ����
    kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );
}

/**
 *  Ű���� ���ͷ�Ʈ�� �ڵ鷯
 */
void kKeyboardHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iKeyboardInterruptCount = 0;
    BYTE bTemp;

    //=========================================================================
    // ���ͷ�Ʈ�� �߻������� �˸����� �޽����� ����ϴ� �κ�
    // ���ͷ�Ʈ ���͸� ȭ�� ���� ���� 2�ڸ� ������ ���
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // �߻��� Ƚ�� ���
    vcBuffer[ 8 ] = '0' + g_iKeyboardInterruptCount;
    g_iKeyboardInterruptCount = ( g_iKeyboardInterruptCount + 1 ) % 10;
    kPrintStringXY( 0, 0, vcBuffer );
    //=========================================================================

    // Ű���� ��Ʈ�ѷ����� �����͸� �о ASCII�� ��ȯ�Ͽ� ť�� ����
    if( kIsOutputBufferFull() == TRUE )
    {
        bTemp = kGetKeyboardScanCode();
        kConvertScanCodeAndPutQueue( bTemp );
    }

    // EOI ����
    kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );
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
