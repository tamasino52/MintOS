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

// Application Processor�� ���� Main �Լ�
void MainForApplicationProcessor( void );

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
            ( QWORD ) kIdleTask );
    kStartConsoleShell();
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

    // 1�ʸ��� �ѹ��� �޽����� ���
    qwTickCount = kGetTickCount();
    while( 1 )
    {
        if( kGetTickCount() - qwTickCount > 1000 )
        {
            qwTickCount = kGetTickCount();
            
            kPrintf( "Application Processor[APIC ID: %d] Is Activated\n",
                    kGetAPICID() );
        }
    }
}
