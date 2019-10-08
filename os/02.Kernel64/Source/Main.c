/**
 *  file    Main.c
 *  date    2009/01/02
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   C ���� �ۼ��� Ŀ���� ��Ʈ�� ����Ʈ ����
 */

#include "Types.h"

// �Լ� ����
void kPrintString( int iX, int iY, const char* pcString );
void kAB8000PrintString(int iX, int iY, const char* pcString);

/**
 *  �Ʒ� �Լ��� C ��� Ŀ���� ���� �κ���
 */
void Main( void )
{
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
}

/**
 *  ���ڿ��� X, Y ��ġ�� ���
 */
void kPrintString( int iX, int iY, const char* pcString )
{
    CHARACTER* pstScreen = ( CHARACTER* ) 0xB8000;
    int i;
    
    // X, Y ��ǥ�� �̿��ؼ� ���ڿ��� ����� ��巹���� ���
    pstScreen += ( iY * 80 ) + iX;

    // NULL�� ���� ������ ���ڿ� ���
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
