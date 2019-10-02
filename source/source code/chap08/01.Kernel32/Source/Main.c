/**
 *  file    Main.c
 *  date    2008/12/14
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   C ���� �ۼ��� Ŀ���� ��Ʈ�� ����Ʈ ����
 */

#include "Types.h"

// �Լ� ����
void kPrintString( int iX, int iY, const char* pcString );
BOOL kInitializeKernel64Area( void );
BOOL kIsMemoryEnough( void );

/**
 *  �Ʒ� �Լ��� C ��� Ŀ���� ���� �κ���
 *      �ݵ�� �ٸ� �Լ��� ���� ���� ���ʿ� �����ؾ� ��
 */
void Main( void )
{
    DWORD i;
    
    kPrintString( 0, 3, "C Language Kernel Start.....................[Pass]" );
    
    // �ּ� �޸� ũ�⸦ �����ϴ� �� �˻�
    kPrintString( 0, 4, "Minimum Memory Size Check...................[    ]" );
    if( kIsMemoryEnough() == FALSE )
    {
        kPrintString( 45, 4, "Fail" );
        kPrintString( 0, 5, "Not Enough Memory~!! MINT64 OS Requires Over "
                "64Mbyte Memory~!!" );
        while( 1 ) ;
    }
    else
    {
        kPrintString( 45, 4, "Pass" );
    }
    
    // IA-32e ����� Ŀ�� ������ �ʱ�ȭ
    kPrintString( 0, 5, "IA-32e Kernel Area Initialize...............[    ]" );
    if( kInitializeKernel64Area() == FALSE )
    {
        kPrintString( 45, 5, "Fail" );
        kPrintString( 0, 6, "Kernel Area Initialization Fail~!!" );
        while( 1 ) ;
    }
    kPrintString( 45, 5, "Pass" );

    while( 1 ) ;
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

/**
 *  IA-32e ���� Ŀ�� ������ 0���� �ʱ�ȭ
 *      1Mbyte ~ 6Mbyte���� ������ �ʱ�ȭ
 */
BOOL kInitializeKernel64Area( void )
{
    DWORD* pdwCurrentAddress;
    
    // �ʱ�ȭ�� ������ ��巹���� 0x100000(1MB)�� ����
    pdwCurrentAddress = ( DWORD* ) 0x100000;
    
    // ������ ��巹���� 0x600000(6MB)���� ������ ���鼭 4����Ʈ�� 0���� ä��
    while( ( DWORD ) pdwCurrentAddress < 0x600000 )
    {        
        *pdwCurrentAddress = 0x00;

        // 0���� ������ �� �ٽ� �о��� �� 0�� ������ ������ �ش� ��巹���� 
        // ����ϴµ� ������ ���� ���̹Ƿ� ���̻� �������� �ʰ� ����
        if( *pdwCurrentAddress != 0 )
        {
            return FALSE;
        }
        
        // ���� ��巹���� �̵�
        pdwCurrentAddress++;
    }
    
    return TRUE;
}

/**
 *  MINT64 OS�� �����ϱ⿡ ����� �޸𸮸� ������ �ִ��� üũ
 *      64Mbyte �̻��� �޸𸮸� ������ �ִ��� �˻�
 */
BOOL kIsMemoryEnough( void )
{
    DWORD* pdwCurrentAddress;
   
    // 0x100000(1MB)���� �˻� ����
    pdwCurrentAddress = ( DWORD* ) 0x100000;
    
    // 0x4000000(64MB)���� ������ ���鼭 Ȯ��
    while( ( DWORD ) pdwCurrentAddress < 0x4000000 )
    {
        *pdwCurrentAddress = 0x12345678;
        
        // 0x12345678�� ������ �� �ٽ� �о��� �� 0x12345678�� ������ ������ 
        // �ش� ��巹���� ����ϴµ� ������ ���� ���̹Ƿ� ���̻� �������� �ʰ� ����
        if( *pdwCurrentAddress != 0x12345678 )
        {
           return FALSE;
        }
        
        // 1MB�� �̵��ϸ鼭 Ȯ��
        pdwCurrentAddress += ( 0x100000 / 4 );
    }
    return TRUE;
}

