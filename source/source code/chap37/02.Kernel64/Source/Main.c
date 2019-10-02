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
#include "MPConfigurationTable.h"
#include "Mouse.h"

// Application Processor�� ���� Main �Լ�
void MainForApplicationProcessor( void );
// ��Ƽ �ھ� ���μ��� �Ǵ� ��Ƽ ���μ��� ���� ��ȯ�ϴ� �Լ�
BOOL kChangeToMultiCoreMode( void );
// �׷��� ��带 �׽�Ʈ�ϴ� �Լ�
void kStartGraphicModeTest( void );

/**
 *  Bootstrap Processor�� C ��� Ŀ�� ��Ʈ�� ����Ʈ
 *      �Ʒ� �Լ��� C ��� Ŀ���� ���� �κ���
 */
void Main( void )
{
    int iCursorX, iCursorY;    
    BYTE bButton;
    int iX;
    int iY;
    
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

    kPrintf( "Mouse Activate And Queue Initialize.........[    ]" );
    // ���콺�� Ȱ��ȭ
    if( kInitializeMouse() == TRUE )
    {
        kEnableMouseInterrupt();
        kSetCursor( 45, iCursorY++ );
        kPrintf( "Pass\n" );
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
    
    // ��Ƽ�ھ� ���μ��� ���� ��ȯ
    // Application Processor Ȱ��ȭ, I/O ��� Ȱ��ȭ, ���ͷ�Ʈ�� �½�ũ ���� �л�
    // ��� Ȱ��ȭ
    kPrintf( "Change To MultiCore Processor Mode..........[    ]" );
    if( kChangeToMultiCoreMode() == TRUE )
    {
        kSetCursor( 45, iCursorY++ );
        kPrintf( "Pass\n" );
    }
    else
    {
        kSetCursor( 45, iCursorY++ );
        kPrintf( "Fail\n" );
    }

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
    //kPrintf( "Application Processor[APIC ID: %d] Is Activated\n",
    //        kGetAPICID() );

    // ���� �½�ũ ����
    kIdleTask();
}

/**
 *  ��Ƽ�ھ� ���μ��� �Ǵ� ��Ƽ ���μ��� ���� ��ȯ�ϴ� �Լ�
 */
BOOL kChangeToMultiCoreMode( void )
{
    MPCONFIGRUATIONMANAGER* pstMPManager;
    BOOL bInterruptFlag;
    int i;

    // Application Processor Ȱ��ȭ
    if( kStartUpApplicationProcessor() == FALSE )
    {
        return FALSE;
    }

    //--------------------------------------------------------------------------
    // ��Ī I/O ���� ��ȯ
    //--------------------------------------------------------------------------
    // MP ���� �Ŵ����� ã�Ƽ� PIC ����ΰ� Ȯ��
    pstMPManager = kGetMPConfigurationManager();
    if( pstMPManager->bUsePICMode == TRUE )
    {
        // PIC ����̸� I/O ��Ʈ ��巹�� 0x22�� 0x70�� ���� �����ϰ� 
        // I/O ��Ʈ ��巹�� 0x23�� 0x01�� �����ϴ� ������� IMCR �������Ϳ� �����Ͽ�
        // PIC ��� ��Ȱ��ȭ
        kOutPortByte( 0x22, 0x70 );
        kOutPortByte( 0x23, 0x01 );
    }

    // PIC ��Ʈ�ѷ��� ���ͷ�Ʈ�� ��� ����ũ�Ͽ� ���ͷ�Ʈ�� �߻��� �� ������ ��
    kMaskPICInterrupt( 0xFFFF );

    // ���μ��� ��ü�� ���� APIC�� Ȱ��ȭ
    kEnableGlobalLocalAPIC();
    
    // ���� �ھ��� ���� APIC�� Ȱ��ȭ
    kEnableSoftwareLocalAPIC();

    // ���ͷ�Ʈ�� �Ұ��� ����
    bInterruptFlag = kSetInterruptFlag( FALSE );
    
    // ��� ���ͷ�Ʈ�� ������ �� �ֵ��� �½�ũ �켱 ���� �������͸� 0���� ����
    kSetTaskPriority( 0 );

    // ���� APIC�� ���� ���� ���̺��� �ʱ�ȭ
    kInitializeLocalVectorTable();

    // ��Ī I/O ���� ����Ǿ����� ����
    kSetSymmetricIOMode( TRUE );
    
    // I/O APIC �ʱ�ȭ
    kInitializeIORedirectionTable();
        
    // ���� ���ͷ�Ʈ �÷��׸� ����
    kSetInterruptFlag( bInterruptFlag );

    // ���ͷ�Ʈ ���� �л� ��� Ȱ��ȭ
    kSetInterruptLoadBalancing( TRUE );

    // �½�ũ ���� �л� ��� Ȱ��ȭ
    for( i = 0 ; i < MAXPROCESSORCOUNT ; i++ )
    {
        kSetTaskLoadBalancing( i, TRUE );
    }
    
    return TRUE;
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


// ���콺 Ŀ���� ���� �߰�
// ���콺�� �ʺ�� ����
#define MOUSE_CURSOR_WIDTH      20
#define MOUSE_CURSOR_HEIGHT     20

// ���콺 Ŀ���� �̹����� �����ϴ� ������
static BYTE gs_vwMouseBuffer[ MOUSE_CURSOR_WIDTH * MOUSE_CURSOR_HEIGHT ] =
{
    1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 2, 3, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0,
    0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1,
    0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 1, 1,
    0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 1, 1, 0, 0,
    0, 1, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 1, 1, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 2, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 2, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 2, 3, 3, 3, 2, 2, 3, 3, 3, 2, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 2, 3, 3, 2, 1, 1, 2, 3, 2, 2, 2, 1, 0, 0, 0, 0,
    0, 0, 0, 1, 2, 3, 2, 2, 1, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0, 0,
    0, 0, 0, 1, 2, 3, 2, 1, 0, 0, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0,
    0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 1, 0,
    0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 1,
    0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 1, 0,
    0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0,
    0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
};

// Ŀ�� �̹����� ����
#define MOUSE_CURSOR_OUTERLINE      RGB(0, 0, 0 )
#define MOUSE_CURSOR_OUTER          RGB( 79, 204, 11 )
#define MOUSE_CURSOR_INNER          RGB( 232, 255, 232 )

/**
 *  X, Y ��ġ�� ���콺 Ŀ���� ���
 */
void kDrawCursor( RECT* pstArea, COLOR* pstVideoMemory, int iX, int iY )
{
    int i;
    int j;
    BYTE* pbCurrentPos;
    
    // Ŀ�� �������� ���� ��ġ�� ����
    pbCurrentPos = gs_vwMouseBuffer;
    
    // Ŀ���� �ʺ�� ���̸�ŭ ������ ���鼭 �ȼ��� ȭ�鿡 ���
    for( j = 0 ; j < MOUSE_CURSOR_HEIGHT ; j++ )
    {
        for( i = 0 ; i < MOUSE_CURSOR_WIDTH ; i++ )
        {
            switch( *pbCurrentPos )
            {
                // 0�� ������� ����
            case 0:
                // nothing
                break;
                
                // ���� �ٱ��� �׵θ�, ���������� ���
            case 1:
                kInternalDrawPixel( pstArea, pstVideoMemory, i + iX, j + iY,
                                    MOUSE_CURSOR_OUTERLINE );
                break;
                
                // ���ʰ� �ٱ����� ���, ��ο� ������� ���
            case 2:
                kInternalDrawPixel( pstArea, pstVideoMemory, i + iX, j + iY,
                                    MOUSE_CURSOR_OUTER );
                break;
                
                // Ŀ���� ��, ���� ������ ���
            case 3:
                kInternalDrawPixel( pstArea, pstVideoMemory, i + iX, j + iY,
                                    MOUSE_CURSOR_INNER );
                break;
            }
            
            // Ŀ���� �ȼ��� ǥ�õʿ� ���� Ŀ�� �������� ��ġ�� ���� �̵�
            pbCurrentPos++;
        }
    }
}

/**
 *  �׷��� ��带 �׽�Ʈ�ϴ� �Լ�
 */
void kStartGraphicModeTest()
{
    VBEMODEINFOBLOCK* pstVBEMode;
    int iX, iY;
    COLOR* pstVideoMemory;
    RECT stScreenArea;
    int iRelativeX, iRelativeY;
    BYTE bButton;
    
    // VBE ��� ���� ����� ��ȯ
    pstVBEMode = kGetVBEModeInfoBlock();
    
    // ȭ�� ���� ����
    stScreenArea.iX1 = 0;
    stScreenArea.iY1 = 0;
    stScreenArea.iX2 = pstVBEMode->wXResolution - 1;
    stScreenArea.iY2 = pstVBEMode->wYResolution - 1;

    // �׷��� �޸� ��巹�� ����
    pstVideoMemory = ( COLOR* ) ( ( QWORD )pstVBEMode->dwPhysicalBasePointer & 0xFFFFFFFF );

    // ���콺�� �ʱ� ��ġ�� ȭ�� ����� ����
    iX = pstVBEMode->wXResolution / 2;
    iY = pstVBEMode->wYResolution / 2;
    
    //==========================================================================
    // ���콺 Ŀ���� ����ϰ� ���콺 �̵��� ó��
    //==========================================================================
    // ����� ���
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 
            stScreenArea.iX1, stScreenArea.iY1, stScreenArea.iX2, stScreenArea.iY2, 
            RGB( 232, 255, 232 ), TRUE );

    // ���� ��ġ�� ���콺 Ŀ���� ���
    kDrawCursor( &stScreenArea, pstVideoMemory, iX, iY );
    
    while( 1 )
    {
        // ���콺 �����Ͱ� ���ŵǱ⸦ ��ٸ�
        if( kGetMouseDataFromMouseQueue( &bButton, &iRelativeX, &iRelativeY ) ==
            FALSE )
        {
            kSleep( 0 );
            continue;
        }
        
        // ������ ���콺 Ŀ���� �ִ� ��ġ�� ����� ���
        kInternalDrawRect( &stScreenArea, pstVideoMemory, iX, iY, 
                iX + MOUSE_CURSOR_WIDTH, iY + MOUSE_CURSOR_HEIGHT, 
                RGB( 232, 255, 232 ), TRUE );
        
        // ���콺�� ������ �Ÿ��� ���� Ŀ�� ��ġ�� ���ؼ� ���� ��ǥ�� ���
        iX += iRelativeX;
        iY += iRelativeY;
        
        // ���콺 Ŀ���� ȭ���� ����� ���ϵ��� ����
        if( iX < stScreenArea.iX1 )
        {
            iX = stScreenArea.iX1;
        }
        else if( iX > stScreenArea.iX2 )
        {
            iX = stScreenArea.iX2;
        }
        
        if( iY < stScreenArea.iY1 )
        {
            iY = stScreenArea.iY1;
        }
        else if( iY > stScreenArea.iY2 )
        {
            iY = stScreenArea.iY2;
        }
        
        // ���� ��ư�� �������� ������ ������Ÿ�� ǥ��
        if( bButton & MOUSE_LBUTTONDOWN )
        {
            kDrawWindowFrame( iX - 10, iY - 10, 400, 200, "MINT64 OS Test Window" );
        }
        // ������ ��ư�� �������� ȭ�� ��ü�� �������� ä��
        else if( bButton & MOUSE_RBUTTONDOWN )
        {
            kInternalDrawRect( &stScreenArea, pstVideoMemory, 
                stScreenArea.iX1, stScreenArea.iY1, stScreenArea.iX2, 
                stScreenArea.iY2, RGB( 232, 255, 232 ), TRUE );
        }
        
        // ����� ���콺 Ŀ�� ��ġ�� ���콺 Ŀ�� �̹����� ���
        kDrawCursor( &stScreenArea, pstVideoMemory, iX, iY );
    }
}
