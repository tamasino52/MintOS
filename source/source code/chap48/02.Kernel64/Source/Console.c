/**
 *  file    Console.c
 *  date    2009/01/31
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   �ֿܼ� ���õ� �ҽ� ����
 */

#include <stdarg.h>
#include "Console.h"
#include "Keyboard.h"

// �ܼ��� ������ �����ϴ� �ڷᱸ��
CONSOLEMANAGER gs_stConsoleManager = { 0, };

// �׷��� ���� �������� �� ����ϴ� ȭ�� ���� ����
static CHARACTER gs_vstScreenBuffer[ CONSOLE_WIDTH * CONSOLE_HEIGHT ];

// �׷��� ���� �������� �� GUI �ܼ� �� ������� ���޵� Ű �̺�Ʈ�� �ܼ� �� �½�ũ�� 
// �����ϴ� ť ����
static KEYDATA gs_vstKeyQueueBuffer[ CONSOLE_GUIKEYQUEUE_MAXCOUNT ];

/**
 *  �ܼ� �ʱ�ȭ
 */
void kInitializeConsole( int iX, int iY )
{
    // �ܼ� �ڷᱸ�� �ʱ�ȭ
    kMemSet( &gs_stConsoleManager, 0, sizeof( gs_stConsoleManager ) );
    // ȭ�� ���� �ʱ�ȭ
    kMemSet( &gs_vstScreenBuffer, 0, sizeof( gs_vstScreenBuffer ) );
    
    if( kIsGraphicMode() == FALSE )
    {
        // �׷��� ���� ������ ���� �ƴϸ� ���� �޸𸮸� ȭ�� ���۷� ����
        gs_stConsoleManager.pstScreenBuffer = ( CHARACTER* ) CONSOLE_VIDEOMEMORYADDRESS;
    }
    else
    {
        // �׷��� ���� ���������� �׷��� ���� ȭ�� ���۸� ����
        gs_stConsoleManager.pstScreenBuffer = gs_vstScreenBuffer;
        
        // �׷��� ��忡�� ����� Ű ť�� ���ؽ��� �ʱ�ȭ
        kInitializeQueue( &( gs_stConsoleManager.stKeyQueueForGUI ), gs_vstKeyQueueBuffer, 
                CONSOLE_GUIKEYQUEUE_MAXCOUNT, sizeof( KEYDATA ) );
        kInitializeMutex( &( gs_stConsoleManager.stLock ) );
    }
    
    // Ŀ�� ��ġ ����
    kSetCursor( iX, iY );
}

/**
 *  Ŀ���� ��ġ�� ����
 *      ���ڸ� ����� ��ġ�� ���� ����
 */
void kSetCursor( int iX, int iY ) 
{
    int iLinearValue;
    int iOldX;
    int iOldY;
    int i;
    
    // Ŀ���� ��ġ�� ���
    iLinearValue = iY * CONSOLE_WIDTH + iX;

    // �ؽ�Ʈ ���� ���������� CRT ��Ʈ�ѷ��� Ŀ�� ��ġ�� ����
    if( kIsGraphicMode() == FALSE )
    {
        // CRTC ��Ʈ�� ��巹�� ��������(��Ʈ 0x3D4)�� 0x0E�� �����Ͽ�
        // ���� Ŀ�� ��ġ �������͸� ����
        kOutPortByte( VGA_PORT_INDEX, VGA_INDEX_UPPERCURSOR );
        // CRTC ��Ʈ�� ������ ��������(��Ʈ 0x3D5)�� Ŀ���� ���� ����Ʈ�� ���
        kOutPortByte( VGA_PORT_DATA, iLinearValue >> 8 );
    
        // CRTC ��Ʈ�� ��巹�� ��������(��Ʈ 0x3D4)�� 0x0F�� �����Ͽ�
        // ���� Ŀ�� ��ġ �������͸� ����
        kOutPortByte( VGA_PORT_INDEX, VGA_INDEX_LOWERCURSOR );
        // CRTC ��Ʈ�� ������ ��������(��Ʈ 0x3D5)�� Ŀ���� ���� ����Ʈ�� ���
        kOutPortByte( VGA_PORT_DATA, iLinearValue & 0xFF );
    }
    // �׷��� ���� ���������� ȭ�� ���ۿ� ����� Ŀ���� ��ġ�� �Ű���
    else
    {
        // ������ Ŀ���� �ִ� ��ġ�� �״�� Ŀ���� ���������� Ŀ���� ����
        for( i = 0 ; i < CONSOLE_WIDTH * CONSOLE_HEIGHT ; i++ )
        {
            // Ŀ���� ������ ����
            if( ( gs_stConsoleManager.pstScreenBuffer[ i ].bCharactor == '_' ) &&
                ( gs_stConsoleManager.pstScreenBuffer[ i ].bAttribute == 0x00 ) )
            {
                gs_stConsoleManager.pstScreenBuffer[ i ].bCharactor = ' ';
                gs_stConsoleManager.pstScreenBuffer[ i ].bAttribute = 
                    CONSOLE_DEFAULTTEXTCOLOR;
                break;
            }
        }
        
        // ���ο� ��ġ�� Ŀ���� ���
        gs_stConsoleManager.pstScreenBuffer[ iLinearValue ].bCharactor = '_';
        gs_stConsoleManager.pstScreenBuffer[ iLinearValue ].bAttribute = 0x00;
    }
    // ���ڸ� ����� ��ġ ������Ʈ
    gs_stConsoleManager.iCurrentPrintOffset = iLinearValue;
}

/**
 *  ���� Ŀ���� ��ġ�� ��ȯ
 */
void kGetCursor( int *piX, int *piY )
{
    *piX = gs_stConsoleManager.iCurrentPrintOffset % CONSOLE_WIDTH;
    *piY = gs_stConsoleManager.iCurrentPrintOffset / CONSOLE_WIDTH;
}

/**
 *  printf �Լ��� ���� ����
 */
void kPrintf( const char* pcFormatString, ... )
{
    va_list ap;
    char vcBuffer[ 1024 ];
    int iNextPrintOffset;

    // ���� ���� ����Ʈ�� ����ؼ� vsprintf()�� ó��
    va_start( ap, pcFormatString );
    kVSPrintf( vcBuffer, pcFormatString, ap );
    va_end( ap );
    
    // ���� ���ڿ��� ȭ�鿡 ���
    iNextPrintOffset = kConsolePrintString( vcBuffer );
    
    // Ŀ���� ��ġ�� ������Ʈ
    kSetCursor( iNextPrintOffset % CONSOLE_WIDTH, iNextPrintOffset / CONSOLE_WIDTH );
}

/**
 *  \n, \t�� ���� ���ڰ� ���Ե� ���ڿ��� ����� ��, ȭ����� ���� ����� ��ġ�� 
 *  ��ȯ
 */
int kConsolePrintString( const char* pcBuffer )
{
    CHARACTER* pstScreen;
    int i, j;
    int iLength;
    int iPrintOffset;

    // ȭ�� ���۸� ����
    pstScreen = gs_stConsoleManager.pstScreenBuffer;
    
    // ���ڿ��� ����� ��ġ�� ����
    iPrintOffset = gs_stConsoleManager.iCurrentPrintOffset;

    // ���ڿ��� ���̸�ŭ ȭ�鿡 ���
    iLength = kStrLen( pcBuffer );    
    for( i = 0 ; i < iLength ; i++ )
    {
        // ���� ó��
        if( pcBuffer[ i ] == '\n' )
        {
            // ����� ��ġ�� 80�� ��� �÷����� �ű�
            // ���� ������ ���� ���ڿ��� ����ŭ ���ؼ� ���� �������� ��ġ��Ŵ
            iPrintOffset += ( CONSOLE_WIDTH - ( iPrintOffset % CONSOLE_WIDTH ) );
        }
        // �� ó��
        else if( pcBuffer[ i ] == '\t' )
        {
            // ����� ��ġ�� 8�� ��� �÷����� �ű�
            iPrintOffset += ( 8 - ( iPrintOffset % 8 ) );
        }
        // �Ϲ� ���ڿ� ���
        else
        {
            // ���� �޸𸮿� ���ڿ� �Ӽ��� �����Ͽ� ���ڸ� ����ϰ�
            // ����� ��ġ�� �������� �̵�
            pstScreen[ iPrintOffset ].bCharactor = pcBuffer[ i ];
            pstScreen[ iPrintOffset ].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
            iPrintOffset++;
        }
        
        // ����� ��ġ�� ȭ���� �ִ�(80 * 25)�� ������� ��ũ�� ó��
        if( iPrintOffset >= ( CONSOLE_HEIGHT * CONSOLE_WIDTH ) )
        {
            // ���� ������ ������ �������� �� �� ���� ����
            kMemCpy( pstScreen, pstScreen + CONSOLE_WIDTH,
                     ( CONSOLE_HEIGHT - 1 ) * CONSOLE_WIDTH * sizeof( CHARACTER ) );

            // ���� ������ ������ �������� ä��
            for( j = ( CONSOLE_HEIGHT - 1 ) * ( CONSOLE_WIDTH ) ; 
                 j < ( CONSOLE_HEIGHT * CONSOLE_WIDTH ) ; j++ )
            {
                // ���� ���
                pstScreen[ j ].bCharactor = ' ';
                pstScreen[ j ].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
            }
            
            // ����� ��ġ�� ���� �Ʒ��� ������ ó������ ����
            iPrintOffset = ( CONSOLE_HEIGHT - 1 ) * CONSOLE_WIDTH;
        }
    }
    return iPrintOffset;
}

/**
 *  ��ü ȭ���� ����
 */
void kClearScreen( void )
{
    CHARACTER* pstScreen;
    int i;
    
    // ȭ�� ���۸� ����
    pstScreen = gs_stConsoleManager.pstScreenBuffer;
    
    // ȭ�� ��ü�� �������� ä���, Ŀ���� ��ġ�� 0, 0���� �ű�
    for( i = 0 ; i < CONSOLE_WIDTH * CONSOLE_HEIGHT ; i++ )
    {
        pstScreen[ i ].bCharactor = ' ';
        pstScreen[ i ].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
    }
    
    // Ŀ���� ȭ�� ������� �̵�
    kSetCursor( 0, 0 );
}

/**
 *  getch() �Լ��� ����
 */
BYTE kGetCh( void )
{
    KEYDATA stData;
    
    // Ű�� ������ ������ �����
    while( 1 )
    {
        // �׷��� ��尡 �ƴ� ���� Ŀ���� Ű ť���� ���� ������
        if( kIsGraphicMode() == FALSE )
        {
            // Ű ť�� �����Ͱ� ���ŵ� ������ ���
            while( kGetKeyFromKeyQueue( &stData ) == FALSE )
            {
                kSchedule();
            }
        }
        // �׷��� ����� ���� �׷��� ���� Ű ť���� ���� ������
        else
        {
            while( kGetKeyFromGUIKeyQueue( &stData ) == FALSE )
            {
                // �׷��� ��忡�� �����ϴ� �߿� �� �½�ũ�� �����ؾߵ� ��� ������ ����
                if( gs_stConsoleManager.bExit == TRUE )
                {
                    return 0xFF;
                }
                kSchedule();
            }
        }
        
        // Ű�� ���ȴٴ� �����Ͱ� ���ŵǸ� ASCII �ڵ带 ��ȯ
        if( stData.bFlags & KEY_FLAGS_DOWN )
        {
            return stData.bASCIICode;
        }
    }
}

/**
 *  ���ڿ��� X, Y ��ġ�� ���
 */
void kPrintStringXY( int iX, int iY, const char* pcString )
{
    CHARACTER* pstScreen;
    int i;
    
    // ȭ�� ���۸� ����
    pstScreen = gs_stConsoleManager.pstScreenBuffer;
  
    // ���� ����� ��ġ�� ���
    pstScreen += ( iY * CONSOLE_WIDTH ) + iX;
    // ���ڿ��� ���̸�ŭ ������ ���鼭 ���ڿ� �Ӽ��� ����
    for( i = 0 ; pcString[ i ] != 0 ; i++ )
    {
        pstScreen[ i ].bCharactor = pcString[ i ];
        pstScreen[ i ].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
    }
}

/**
 *  �ܼ��� �����ϴ� �ڷᱸ���� ��ȯ
 */
CONSOLEMANAGER* kGetConsoleManager( void )
{
    return &gs_stConsoleManager;
}

/**
 *  �׷��� ���� Ű ť���� Ű �����͸� ����
 */
BOOL kGetKeyFromGUIKeyQueue( KEYDATA* pstData )
{
    BOOL bResult;
    
    // ť�� �����Ͱ� ������ ����
    if( kIsQueueEmpty( &( gs_stConsoleManager.stKeyQueueForGUI ) ) == TRUE )
    {
        return FALSE;
    }
    
    // ����ȭ ó��
    kLock( &( gs_stConsoleManager.stLock ) );

    // ť���� �����͸� ����
    bResult = kGetQueue( &( gs_stConsoleManager.stKeyQueueForGUI ), pstData );
    
    // ����ȭ ó��
    kUnlock( &( gs_stConsoleManager.stLock ) );
    
    return bResult;
}

/**
 *  �׷��� ���� Ű ť�� �����͸� ����
 */
BOOL kPutKeyToGUIKeyQueue( KEYDATA* pstData )
{
    BOOL bResult;
    
    // ť�� �����Ͱ� ���� á���� ����
    if( kIsQueueFull( &( gs_stConsoleManager.stKeyQueueForGUI ) ) == TRUE )
    {
        return FALSE;
    }
    
    // ����ȭ ó��
    kLock( &( gs_stConsoleManager.stLock ) );

    // ť�� �����͸� ����
    bResult = kPutQueue( &( gs_stConsoleManager.stKeyQueueForGUI ), pstData );
    
    // ����ȭ ó��
    kUnlock( &( gs_stConsoleManager.stLock ) );
    
    return bResult;
}

/**
 *  �ܼ� �� �½�ũ ���� �÷��׸� ����
 */
void kSetConsoleShellExitFlag( BOOL bFlag )
{
    gs_stConsoleManager.bExit = bFlag;
}
