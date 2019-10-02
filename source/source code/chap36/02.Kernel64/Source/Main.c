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
#include "PIC.h"
#include "Console.h"
#include "ConsoleShell.h"
#include "Task.h"
#include "PIT.h"
#include "DynamicMemory.h"
#include "HardDisk.h"
#include "FileSystem.h"
#include "SerialPort.h"
#include "MultiProcessor.h"
#include "VBE.h"
#include "2DGraphics.h"

// Application Processor�� ���� Main �Լ�
void MainForApplicationProcessor( void );
// �׷��� ��带 �׽�Ʈ�ϴ� �Լ�
void kStartGraphicModeTest();

/**
 *  Bootstrap Processor�� C ��� Ŀ�� ��Ʈ�� ����Ʈ
 *      �Ʒ� �Լ��� C ��� Ŀ���� ���� �κ���
 */
void Main( void )
{
    int iCursorX, iCursorY;
    
    // ��Ʈ �δ��� �ִ� BSP �÷��׸� �о Application Processor�̸� 
    // �ش� �ھ�� �ʱ�ȭ �Լ��� �̵�
    if( *( ( BYTE* ) BOOTSTRAPPROCESSOR_FLAGADDRESS ) == 0 )
    {
        MainForApplicationProcessor();
    }
    
    // Bootstrap Processor�� ������ �Ϸ������Ƿ�, 0x7C09�� �ִ� Bootstrap Processor��
    // ��Ÿ���� �÷��׸� 0���� �����Ͽ� Application Processor������ �ڵ� ���� ��θ� ����
    *( ( BYTE* ) BOOTSTRAPPROCESSOR_FLAGADDRESS ) = 0;

    // �ܼ��� ���� �ʱ�ȭ�� ��, ���� �۾��� ����
    kInitializeConsole( 0, 10 );    
    kPrintf( "Switch To IA-32e Mode Success~!!\n" );
    kPrintf( "IA-32e C Language Kernel Start..............[Pass]\n" );
    kPrintf( "Initialize Console..........................[Pass]\n" );
    
    // ���� ��Ȳ�� ȭ�鿡 ���
    kGetCursor( &iCursorX, &iCursorY );
    kPrintf( "GDT Initialize And Switch For IA-32e Mode...[    ]" );
    kInitializeGDTTableAndTSS();
    kLoadGDTR( GDTR_STARTADDRESS );
    kSetCursor( 45, iCursorY++ );
    kPrintf( "Pass\n" );
    
    kPrintf( "TSS Segment Load............................[    ]" );
    kLoadTR( GDT_TSSSEGMENT );
    kSetCursor( 45, iCursorY++ );
    kPrintf( "Pass\n" );
    
    kPrintf( "IDT Initialize..............................[    ]" );
    kInitializeIDTTables();    
    kLoadIDTR( IDTR_STARTADDRESS );
    kSetCursor( 45, iCursorY++ );
    kPrintf( "Pass\n" );
    
    kPrintf( "Total RAM Size Check........................[    ]" );
    kCheckTotalRAMSize();
    kSetCursor( 45, iCursorY++ );
    kPrintf( "Pass], Size = %d MB\n", kGetTotalRAMSize() );
    
    kPrintf( "TCB Pool And Scheduler Initialize...........[Pass]\n" );
    iCursorY++;
    kInitializeScheduler();
    
    // ���� �޸� �ʱ�ȭ
    kPrintf( "Dynamic Memory Initialize...................[Pass]\n" );
    iCursorY++;
    kInitializeDynamicMemory();
    
    // 1ms�� �ѹ��� ���ͷ�Ʈ�� �߻��ϵ��� ����
    kInitializePIT( MSTOCOUNT( 1 ), 1 );
    
    kPrintf( "Keyboard Activate And Queue Initialize......[    ]" );
    // Ű���带 Ȱ��ȭ
    if( kInitializeKeyboard() == TRUE )
    {
        kSetCursor( 45, iCursorY++ );
        kPrintf( "Pass\n" );
        kChangeKeyboardLED( FALSE, FALSE, FALSE );
    }
    else
    {
        kSetCursor( 45, iCursorY++ );
        kPrintf( "Fail\n" );
        while( 1 ) ;
    }
    
    kPrintf( "PIC Controller And Interrupt Initialize.....[    ]" );
    // PIC ��Ʈ�ѷ� �ʱ�ȭ �� ��� ���ͷ�Ʈ Ȱ��ȭ
    kInitializePIC();
    kMaskPICInterrupt( 0 );
    kEnableInterrupt();
    kSetCursor( 45, iCursorY++ );
    kPrintf( "Pass\n" );
    
    // ���� �ý����� �ʱ�ȭ
    kPrintf( "File System Initialize......................[    ]" );
    if( kInitializeFileSystem() == TRUE )
    {
        kSetCursor( 45, iCursorY++ );
        kPrintf( "Pass\n" );
    }
    else
    {
        kSetCursor( 45, iCursorY++ );
        kPrintf( "Fail\n" );
    }

    // �ø��� ��Ʈ�� �ʱ�ȭ    
    kPrintf( "Serial Port Initialize......................[Pass]\n" );
    iCursorY++;
    kInitializeSerialPort();

    // ���� �½�ũ�� �ý��� ������� �����ϰ� ���� ����
    kCreateTask( TASK_FLAGS_LOWEST | TASK_FLAGS_THREAD | TASK_FLAGS_SYSTEM | TASK_FLAGS_IDLE, 0, 0, 
            ( QWORD ) kIdleTask, kGetAPICID() );

    // �׷��� ��尡 �ƴϸ� �ܼ� �� ����
    if( *( BYTE* ) VBE_STARTGRAPHICMODEFLAGADDRESS == 0 )
    {
        kStartConsoleShell();
    }
    // �׷��� ����̸� �׷��� ��� �׽�Ʈ �Լ� ����
    else
    {
        kStartGraphicModeTest();
    }
}

/**
 *  Application Processor�� C ��� Ŀ�� ��Ʈ�� ����Ʈ
 *      ��κ��� �ڷᱸ���� Bootstrap Processor�� ������ �������Ƿ� �ھ �����ϴ�
 *      �۾��� ��
 */
void MainForApplicationProcessor( void )
{
    QWORD qwTickCount;

    // GDT ���̺��� ����
    kLoadGDTR( GDTR_STARTADDRESS );

    // TSS ��ũ���͸� ����. TSS ���׸�Ʈ�� ��ũ���͸� Application Processor�� 
    // ����ŭ ���������Ƿ�, APIC ID�� �̿��Ͽ� TSS ��ũ���͸� �Ҵ�
    kLoadTR( GDT_TSSSEGMENT + ( kGetAPICID() * sizeof( GDTENTRY16 ) ) );

    // IDT ���̺��� ����
    kLoadIDTR( IDTR_STARTADDRESS );
    
    // �����ٷ� �ʱ�ȭ
    kInitializeScheduler();
    
    // ���� �ھ��� ���� APIC�� Ȱ��ȭ
    kEnableSoftwareLocalAPIC();

    // ��� ���ͷ�Ʈ�� ������ �� �ֵ��� �½�ũ �켱 ���� �������͸� 0���� ����
    kSetTaskPriority( 0 );

    // ���� APIC�� ���� ���� ���̺��� �ʱ�ȭ
    kInitializeLocalVectorTable();

    // ���ͷ�Ʈ�� Ȱ��ȭ
    kEnableInterrupt();    

    // ��Ī I/O ��� �׽�Ʈ�� ���� Application Processor�� ������ �� �ѹ��� ���
    kPrintf( "Application Processor[APIC ID: %d] Is Activated\n",
            kGetAPICID() );

    // ���� �½�ũ ����
    kIdleTask();
}

// x�� ���밪���� ��ȯ�ϴ� ��ũ��
#define ABS( x )    ( ( ( x ) >= 0 ) ? ( x ) : -( x ) )

/**
 *  ������ X, Y ��ǥ�� ��ȯ
 */
void kGetRandomXY( int* piX, int* piY )
{
    int iRandom;
    
    // X��ǥ�� ���
    iRandom = kRandom();
    *piX = ABS( iRandom ) % 1000;
    
    // Y��ǥ�� ���
    iRandom = kRandom();
    *piY = ABS( iRandom ) % 700;
}

/**
 *  ������ ���� ��ȯ
 */
COLOR kGetRandomColor( void )
{
    int iR, iG, iB;
    int iRandom;

    iRandom = kRandom();
    iR = ABS( iRandom ) % 256;

    iRandom = kRandom();
    iG = ABS( iRandom ) % 256;
    
    iRandom = kRandom();
    iB = ABS( iRandom ) % 256;
    
    return RGB( iR, iG, iB );
}

/**
 *  ������ �������� �׸�
 */
void kDrawWindowFrame( int iX, int iY, int iWidth, int iHeight, const char* pcTitle )
{
    char* pcTestString1 = "This is MINT64 OS's window prototype~!!!";
    char* pcTestString2 = "Coming soon~!!!";
    VBEMODEINFOBLOCK* pstVBEMode;
    COLOR* pstVideoMemory;
    RECT stScreenArea;

    // VBE ��� ���� ����� ��ȯ
    pstVBEMode = kGetVBEModeInfoBlock();
    
    // ȭ�� ���� ����
    stScreenArea.iX1 = 0;
    stScreenArea.iY1 = 0;
    stScreenArea.iX2 = pstVBEMode->wXResolution - 1;
    stScreenArea.iY2 = pstVBEMode->wYResolution - 1;
    
    // �׷��� �޸� ��巹�� ����
    pstVideoMemory = ( COLOR* ) ( ( QWORD )pstVBEMode->dwPhysicalBasePointer & 0xFFFFFFFF );
    
    // ������ �������� �����ڸ��� �׸�, 2 �ȼ� �β�
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 
            iX, iY, iX + iWidth, iY + iHeight, RGB( 109, 218, 22 ), FALSE );
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 
            iX + 1, iY + 1, iX + iWidth - 1, iY + iHeight - 1, RGB( 109, 218, 22 ),
            FALSE );

    // ���� ǥ������ ä��
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 
            iX, iY + 3, iX + iWidth - 1, iY + 21, RGB( 79, 204, 11 ), TRUE );

    // ������ ������ ǥ��
    kInternalDrawText( &stScreenArea, pstVideoMemory, 
            iX + 6, iY + 3, RGB( 255, 255, 255 ), RGB( 79, 204, 11 ),
            pcTitle, kStrLen( pcTitle ) );
    
    // ���� ǥ������ ��ü�� ���̰� ������ ���� �׸�, 2 �ȼ� �β�
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + 1, iY + 1, iX + iWidth - 1, iY + 1, RGB( 183, 249, 171 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + 1, iY + 2, iX + iWidth - 1, iY + 2, RGB( 150, 210, 140 ) );

    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + 1, iY + 2, iX + 1, iY + 20, RGB( 183, 249, 171 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + 2, iY + 2, iX + 2, iY + 20, RGB( 150, 210, 140 ) );
    
    // ���� ǥ������ �Ʒ��ʿ� ���� �׸�
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + 2, iY + 19, iX + iWidth - 2, iY + 19, RGB( 46, 59, 30 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + 2, iY + 20, iX + iWidth - 2, iY + 20, RGB( 46, 59, 30 ) );

    // �ݱ� ��ư�� �׸�, ������ ��ܿ� ǥ��
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 18, iY + 1, iX + iWidth - 2, iY + 19,
            RGB( 255, 255, 255 ), TRUE );

    // �ݱ� ��ư�� ��ü�� ���̰� ���� �׸�, 2 �ȼ� �β��� �׸�
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2, iY + 1, iX + iWidth - 2, iY + 19 - 1,
            RGB( 86, 86, 86 ), TRUE );
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 1, iY + 1, iX + iWidth - 2 - 1, iY + 19 - 1,
            RGB( 86, 86, 86 ), TRUE );
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 18 + 1, iY + 19, iX + iWidth - 2, iY + 19,
            RGB( 86, 86, 86 ), TRUE );
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 18 + 1, iY + 19 - 1, iX + iWidth - 2, iY + 19 - 1,
            RGB( 86, 86, 86 ), TRUE );

    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 18, iY + 1, iX + iWidth - 2 - 1, iY + 1,
            RGB( 229, 229, 229 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 18, iY + 1 + 1, iX + iWidth - 2 - 2, iY + 1 + 1,
            RGB( 229, 229, 229 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 18, iY + 1, iX + iWidth - 2 - 18, iY + 19,
            RGB( 229, 229, 229 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 18 + 1, iY + 1, iX + iWidth - 2 - 18 + 1, iY + 19 - 1,
            RGB( 229, 229, 229 ) );
    
    // �밢�� X�� �׸�, 3 �ȼ��� �׸�
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 18 + 4, iY + 1 + 4, iX + iWidth - 2 - 4, iY + 19 - 4, 
            RGB( 71, 199, 21 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 18 + 5, iY + 1 + 4, iX + iWidth - 2 - 4, iY + 19 - 5, 
            RGB( 71, 199, 21 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 18 + 4, iY + 1 + 5, iX + iWidth - 2 - 5, iY + 19 - 4, 
            RGB( 71, 199, 21 ) );
    
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 18 + 4, iY + 19 - 4, iX + iWidth - 2 - 4, iY + 1 + 4, 
            RGB( 71, 199, 21 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 18 + 5, iY + 19 - 4, iX + iWidth - 2 - 4, iY + 1 + 5, 
            RGB( 71, 199, 21 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 18 + 4, iY + 19 - 5, iX + iWidth - 2 - 5, iY + 1 + 4, 
            RGB( 71, 199, 21 ) );


    // ���θ� �׸�
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 
            iX + 2, iY + 21, iX + iWidth - 2, iY + iHeight - 2, 
            RGB( 255, 255, 255 ), TRUE );
    
    // �׽�Ʈ ���� ���
    kInternalDrawText( &stScreenArea, pstVideoMemory, 
            iX + 10, iY + 30, RGB( 0, 0, 0 ), RGB( 255, 255, 255 ), pcTestString1,
            kStrLen( pcTestString1 ) );
    kInternalDrawText( &stScreenArea, pstVideoMemory, 
            iX + 10, iY + 50, RGB( 0, 0, 0 ), RGB( 255, 255, 255 ), pcTestString2,
            kStrLen( pcTestString2 ) );
}

/**
 *  �׷��� ��带 �׽�Ʈ�ϴ� �Լ�
 */
void kStartGraphicModeTest()
{
    VBEMODEINFOBLOCK* pstVBEMode;
    int iX1, iY1, iX2, iY2;    
    COLOR stColor1, stColor2;
    int i;
    char* vpcString[] = { "Pixel", "Line", "Rectangle", "Circle", "MINT64 OS~!!!" };
    COLOR* pstVideoMemory;
    RECT stScreenArea;

    // VBE ��� ���� ����� ��ȯ
    pstVBEMode = kGetVBEModeInfoBlock();
    
    // ȭ�� ���� ����
    stScreenArea.iX1 = 0;
    stScreenArea.iY1 = 0;
    stScreenArea.iX2 = pstVBEMode->wXResolution - 1;
    stScreenArea.iY2 = pstVBEMode->wYResolution - 1;
    
    // �׷��� �޸� ��巹�� ����
    pstVideoMemory = ( COLOR* ) ( ( QWORD )pstVBEMode->dwPhysicalBasePointer & 0xFFFFFFFF );
    
    //==========================================================================
    // ��, ��, �簢��, ��, �׸��� ���ڸ� ������ ���
    //==========================================================================
    // (0, 0)�� Pixel�̶� ���ڿ��� ������ ������ ������� ���
    kInternalDrawText( &stScreenArea, pstVideoMemory, 
            0, 0, RGB( 255, 255, 255), RGB( 0, 0, 0 ), vpcString[ 0 ], 
            kStrLen( vpcString[ 0 ] ) );
    // �ȼ��� (1, 20), (2, 20)�� ������� ���
    kInternalDrawPixel( &stScreenArea, pstVideoMemory, 1, 20, RGB( 255, 255, 255 ) );
    kInternalDrawPixel( &stScreenArea, pstVideoMemory, 2, 20, RGB( 255, 255, 255 ) );
    
    // (0, 25)�� Line�̶� ���ڿ��� ������ ������ ���������� ���
    kInternalDrawText( &stScreenArea, pstVideoMemory, 
            0, 25, RGB( 255, 0, 0), RGB( 0, 0, 0 ), vpcString[ 1 ], 
            kStrLen( vpcString[ 1 ] ) );
    // (20, 50)�� �߽����� (1000, 50) (1000, 100), (1000, 150), (1000, 200), 
    // (1000, 250)���� ���������� ���
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 20, 50, 1000, 50, RGB( 255, 0, 0 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 20, 50, 1000, 100, RGB( 255, 0, 0 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 20, 50, 1000, 150, RGB( 255, 0, 0 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 20, 50, 1000, 200, RGB( 255, 0, 0 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 20, 50, 1000, 250, RGB( 255, 0, 0 ) );
    
    // (0, 180)�� Rectangle�̶� ���ڿ��� ������ ������ ������� ���
    kInternalDrawText( &stScreenArea, pstVideoMemory, 
            0, 180, RGB( 0, 255, 0), RGB( 0, 0, 0 ), vpcString[ 2 ], 
            kStrLen( vpcString[ 2 ] ) );
    // (20, 200)���� �����Ͽ� ���̰� ���� 50, 100, 150, 200�� �簢���� ������� ���
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 20, 200, 70, 250, RGB( 0, 255, 0 ), FALSE );
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 120, 200, 220, 300, RGB( 0, 255, 0 ), TRUE );
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 270, 200, 420, 350, RGB( 0, 255, 0 ), FALSE );
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 470, 200, 670, 400, RGB( 0, 255, 0 ), TRUE );
    
    // (0, 550)�� Circle�̶� ���ڿ��� ������ ������ �Ķ������� ���
    kInternalDrawText( &stScreenArea, pstVideoMemory, 
            0, 550, RGB( 0, 0, 255), RGB( 0, 0, 0 ), vpcString[ 3 ], 
            kStrLen( vpcString[ 3 ] ) );
    // (45, 600)���� �����Ͽ� �������� 25, 50, 75, 100�� ���� �Ķ������� ���
    kInternalDrawCircle( &stScreenArea, pstVideoMemory, 45, 600, 25, RGB( 0, 0, 255 ), FALSE ) ;
    kInternalDrawCircle( &stScreenArea, pstVideoMemory, 170, 600, 50, RGB( 0, 0, 255 ), TRUE ) ;
    kInternalDrawCircle( &stScreenArea, pstVideoMemory, 345, 600, 75, RGB( 0, 0, 255 ), FALSE ) ;
    kInternalDrawCircle( &stScreenArea, pstVideoMemory, 570, 600, 100, RGB( 0, 0, 255 ), TRUE ) ;
    
    // Ű �Է� ���
    kGetCh();
    
    //==========================================================================
    // ��, ��, �簢��, ��, �׸��� ���ڸ� �������� ���
    //==========================================================================
    // q Ű�� �Էµ� ������ �Ʒ��� �ݺ�
    do
    {
        // �� �׸���
        for( i = 0 ; i < 100 ; i++ )
        {
            // ������ X��ǥ�� ���� ��ȯ
            kGetRandomXY( &iX1, &iY1 );
            stColor1 = kGetRandomColor();
            
            // �� �׸���
            kInternalDrawPixel( &stScreenArea, pstVideoMemory, iX1, iY1, stColor1 );
        }        
        
        // �� �׸���
        for( i = 0 ; i < 100 ; i++ )
        {
            // ������ X��ǥ�� ���� ��ȯ
            kGetRandomXY( &iX1, &iY1 );
            kGetRandomXY( &iX2, &iY2 );
            stColor1 = kGetRandomColor();
            
            // �� �׸���
            kInternalDrawLine( &stScreenArea, pstVideoMemory, iX1, iY1, iX2, iY2, stColor1 );
        }

        // �簢�� �׸���
        for( i = 0 ; i < 20 ; i++ )
        {
            // ������ X��ǥ�� ���� ��ȯ
            kGetRandomXY( &iX1, &iY1 );
            kGetRandomXY( &iX2, &iY2 );
            stColor1 = kGetRandomColor();

            // �簢�� �׸���
            kInternalDrawRect( &stScreenArea, pstVideoMemory, 
                    iX1, iY1, iX2, iY2, stColor1, kRandom() % 2 );
        }
        
        // �� �׸���
        for( i = 0 ; i < 100 ; i++ )
        {
            // ������ X��ǥ�� ���� ��ȯ
            kGetRandomXY( &iX1, &iY1 );
            stColor1 = kGetRandomColor();

            // �� �׸���
            kInternalDrawCircle( &stScreenArea, pstVideoMemory, 
                    iX1, iY1, ABS( kRandom() % 50 + 1 ), stColor1, kRandom() % 2 );
        }
        
        // �ؽ�Ʈ ǥ��
        for( i = 0 ; i < 100 ; i++ )
        {
            // ������ X��ǥ�� ���� ��ȯ
            kGetRandomXY( &iX1, &iY1 );
            stColor1 = kGetRandomColor();
            stColor2 = kGetRandomColor();
            
            // �ؽ�Ʈ ���
            kInternalDrawText( &stScreenArea, pstVideoMemory, 
                    iX1, iY1, stColor1, stColor2, vpcString[ 4 ], 
                    kStrLen( vpcString[ 4 ] ) );
        }
    } while( kGetCh() != 'q' );
    
    //==========================================================================
    // ������ ������Ÿ���� ���
    //==========================================================================
    // q Ű�� ������ ���� ���Դٸ� ������ ������Ÿ���� ǥ����
    while( 1 )
    {
        // ����� ���
        kInternalDrawRect( &stScreenArea, pstVideoMemory, 
                stScreenArea.iX1, stScreenArea.iY1, stScreenArea.iX2,
                stScreenArea.iY2, RGB( 232, 255, 232 ), TRUE );

        // ������ �������� 3�� �׸�
        for( i = 0 ; i < 3 ; i++ )
        {
            kGetRandomXY( &iX1, &iY1 );
            kDrawWindowFrame( iX1, iY1, 400, 200, "MINT64 OS Test Window" );
        }

        kGetCh();
    }
}
