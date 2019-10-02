/**
 *  file    Main.c
 *  date    2009/01/02
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   C ���� �ۼ��� Ŀ���� ��Ʈ�� ����Ʈ ����
 */

#include "Types.h"
#include "Keyboard.h"
#include "Descriptor.h"

// �Լ� ����
void kPrintString( int iX, int iY, const char* pcString );

/**
 *  �Ʒ� �Լ��� C ��� Ŀ���� ���� �κ���
 */
void Main( void )
{
    char vcTemp[ 2 ] = { 0, };
    BYTE bFlags;
    BYTE bTemp;
    int i = 0;
    
    kPrintString( 0, 10, "Switch To IA-32e Mode Success~!!" );
    kPrintString( 0, 11, "IA-32e C Language Kernel Start..............[Pass]" );
    
    kPrintString( 0, 12, "GDT Initialize And Switch For IA-32e Mode...[    ]" );
    kInitializeGDTTableAndTSS();
    kLoadGDTR( GDTR_STARTADDRESS );
    kPrintString( 45, 12, "Pass" );
    
    kPrintString( 0, 13, "TSS Segment Load............................[    ]" );
    kLoadTR( GDT_TSSSEGMENT );
    kPrintString( 45, 13, "Pass" );
    
    kPrintString( 0, 14, "IDT Initialize..............................[    ]" );
    kInitializeIDTTables();    
    kLoadIDTR( IDTR_STARTADDRESS );
    kPrintString( 45, 14, "Pass" );
    
    kPrintString( 0, 15, "Keyboard Activate...........................[    ]" );
    
    // Ű���带 Ȱ��ȭ
    if( kActivateKeyboard() == TRUE )
    {
        kPrintString( 45, 15, "Pass" );
        kChangeKeyboardLED( FALSE, FALSE, FALSE );
    }
    else
    {
        kPrintString( 45, 15, "Fail" );
        while( 1 ) ;
    }
    
    while( 1 )
    {
        // ��� ����(��Ʈ 0x60)�� �� ������ ��ĵ �ڵ带 ���� �� ����
        if( kIsOutputBufferFull() == TRUE )
        {
            // ��� ����(��Ʈ 0x60)���� ��ĵ �ڵ带 �о ����
            bTemp = kGetKeyboardScanCode();
            
            // ��ĵ �ڵ带 ASCII �ڵ�� ��ȯ�ϴ� �Լ��� ȣ���Ͽ� ASCII �ڵ��
            // ���� �Ǵ� ������ ������ ��ȯ
            if( kConvertScanCodeToASCIICode( bTemp, &( vcTemp[ 0 ] ), &bFlags ) == TRUE )
            {
                // Ű�� ���������� Ű�� ASCII �ڵ� ���� ȭ�鿡 ���
                if( bFlags & KEY_FLAGS_DOWN )
                {
                    kPrintString( i++, 16, vcTemp );
                    // 0�� �ԷµǸ� ������ 0���� ������ Divide Error ����(���� 0��)��
                    // �߻���Ŵ
                    if( vcTemp[ 0 ] == '0' )
                    {
                        // �Ʒ� �ڵ带 �����ϸ� Divide Error ���ܰ� �߻��Ͽ�
                        // Ŀ���� �ӽ� �ڵ鷯�� �����
                        bTemp = bTemp / 0;
                    }
                }
            }
        }
    }
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
