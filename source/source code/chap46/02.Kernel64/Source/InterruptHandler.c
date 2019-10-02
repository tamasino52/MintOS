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
#include "Utility.h"
#include "Task.h"
#include "Descriptor.h"
#include "AssemblyUtility.h"
#include "HardDisk.h"
#include "Mouse.h"

// ���ͷ�Ʈ �ڵ鷯 �ڷᱸ��
static INTERRUPTMANAGER gs_stInterruptManager;

/**
 *  ���ͷ�Ʈ �ڷᱸ�� �ʱ�ȭ
 */
void kInitializeHandler( void )
{
    kMemSet( &gs_stInterruptManager, 0, sizeof( gs_stInterruptManager ) );
}

/**
 *  ���ͷ�Ʈ ó�� ��带 ����
 */
void kSetSymmetricIOMode( BOOL bSymmetricIOMode )
{
    gs_stInterruptManager.bSymmetricIOMode = bSymmetricIOMode;
}

/**
 *  ���ͷ�Ʈ ���� �л� ����� ������� ���θ� ����
 */
void kSetInterruptLoadBalancing( BOOL bUseLoadBalancing )
{
    gs_stInterruptManager.bUseLoadBalancing = bUseLoadBalancing;
}

/**
 *  �ھ� �� ���ͷ�Ʈ ó�� Ƚ���� ����
 */
void kIncreaseInterruptCount( int iIRQ )
{
    // �ھ��� ���ͷ�Ʈ ī��Ʈ�� ����
    gs_stInterruptManager.vvqwCoreInterruptCount[ kGetAPICID() ][ iIRQ ]++;
}

/**
 *  ���� ���ͷ�Ʈ ��忡 ���߾� EOI�� ����
 */
void kSendEOI( int iIRQ )
{
    // ��Ī I/O ��尡 �ƴϸ� PIC ����̹Ƿ�, PIC ��Ʈ�ѷ��� EOI�� �����ؾ� ��
    if( gs_stInterruptManager.bSymmetricIOMode == FALSE )
    {
        kSendEOIToPIC( iIRQ );
    }
    // ��Ī I/O ����̸� ���� APIC�� EOI�� �����ؾ� ��
    else
    {
        kSendEOIToLocalAPIC();
    }
}

/**
 *  ���ͷ�Ʈ �ڵ鷯 �ڷᱸ���� ��ȯ
 */
INTERRUPTMANAGER* kGetInterruptManager( void )
{
    return &gs_stInterruptManager;
}

/**
 *  ���ͷ�Ʈ ���� �л�(Interrupt Load Balancing) ó��
 */
void kProcessLoadBalancing( int iIRQ )
{
    QWORD qwMinCount = 0xFFFFFFFFFFFFFFFF;
    int iMinCountCoreIndex;
    int iCoreCount;
    int i;
    BOOL bResetCount = FALSE;
    BYTE bAPICID;
    
    bAPICID = kGetAPICID();

    // ���� �л� ����� ���� �ְų�, ���� �л��� ó���� ������ �ƴϸ� �� �ʿ䰡 ����
    if( ( gs_stInterruptManager.vvqwCoreInterruptCount[ bAPICID ][ iIRQ ] == 0 ) ||
        ( ( gs_stInterruptManager.vvqwCoreInterruptCount[ bAPICID ][ iIRQ ] % 
            INTERRUPT_LOADBALANCINGDIVIDOR ) != 0 ) ||
        ( gs_stInterruptManager.bUseLoadBalancing == FALSE ) )
    {
        return ;
    }
    
    // �ھ��� ������ ���ؼ� ������ �����ϸ� ���ͷ�Ʈ ó�� Ƚ���� ���� ���� �ھ 
    // ����
    iMinCountCoreIndex = 0;
    iCoreCount = kGetProcessorCount();
    for( i = 0 ; i < iCoreCount ; i++ )
    {
        if( ( gs_stInterruptManager.vvqwCoreInterruptCount[ i ][ iIRQ ] <
                qwMinCount ) )
        {
            qwMinCount = gs_stInterruptManager.vvqwCoreInterruptCount[ i ][ iIRQ ];
            iMinCountCoreIndex = i;
        }
        // ��ü ī��Ʈ�� ���� �ִ� ���� �����ߴٸ� ���߿� ī��Ʈ�� ��� 0���� ����
        else if( gs_stInterruptManager.vvqwCoreInterruptCount[ i ][ iIRQ ] >=
            0xFFFFFFFFFFFFFFFE )
        {
            bResetCount = TRUE;
        }
    }
    
    // I/O �����̷��� ���̺��� �����Ͽ� ���� ���ͷ�Ʈ�� ó���� Ƚ���� ���� ���� APIC�� ����
    kRoutingIRQToAPICID( iIRQ, iMinCountCoreIndex );
    
    // ó���� �ھ��� ī��Ʈ�� �ִ񰪿� �����ߴٸ� ��ü ī��Ʈ�� �ٽ� 0���� �����ϵ���
    // ����
    if( bResetCount == TRUE )
    {
        for( i = 0 ; i < iCoreCount ; i++ )
        {
            gs_stInterruptManager.vvqwCoreInterruptCount[ i ][ iIRQ ] = 0;
        }
    }
}

/**
 *  �������� ����ϴ� ���� �ڵ鷯
 */
void kCommonExceptionHandler( int iVectorNumber, QWORD qwErrorCode )
{
    char vcBuffer[ 3 ] = { 0, };

    kPrintStringXY( 0, 0, "====================================================" );
    kPrintStringXY( 0, 1, "                 Exception Occur~!!!!               " );
    kPrintStringXY( 0, 2, "              Vector:           Core ID:            " );
    // ���� ���͸� ȭ�� ������ ���� 2�ڸ� ������ ���
    vcBuffer[ 0 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 1 ] = '0' + iVectorNumber % 10;
    kPrintStringXY( 21, 2, vcBuffer );
    kSPrintf( vcBuffer, "0x%X", kGetAPICID() );
    kPrintStringXY( 40, 2, vcBuffer );
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
    int iIRQ;

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
    
    // ���ͷ�Ʈ ���Ϳ��� IRQ ��ȣ ����
    iIRQ = iVectorNumber - PIC_IRQSTARTVECTOR;

    // EOI ����
    kSendEOI( iIRQ );
    
    // ���ͷ�Ʈ �߻� Ƚ���� ������Ʈ
    kIncreaseInterruptCount( iIRQ );
    
    // ���� �л�(Load Balancing) ó��
    kProcessLoadBalancing( iIRQ );
}

/**
 *  Ű���� ���ͷ�Ʈ�� �ڵ鷯
 */
void kKeyboardHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iCommonInterruptCount = 0;
    BYTE bTemp;
    int iIRQ;
    
    //=========================================================================
    // ���ͷ�Ʈ�� �߻������� �˸����� �޽����� ����ϴ� �κ�
    // ���ͷ�Ʈ ���͸� ȭ�� ���� ���� 2�ڸ� ������ ���
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // �߻��� Ƚ�� ���
    vcBuffer[ 8 ] = '0' + g_iCommonInterruptCount;
    g_iCommonInterruptCount = ( g_iCommonInterruptCount + 1 ) % 10;
    kPrintStringXY( 0, 0, vcBuffer );
    //=========================================================================

    // ��� ����(��Ʈ 0x60)�� ���ŵ� �����Ͱ� �ִ��� ���θ� Ȯ���Ͽ� ���� �����͸� 
    // Ű ť �Ǵ� ���콺 ť�� ����
    if( kIsOutputBufferFull() == TRUE )
    {
        // ���콺 �����Ͱ� �ƴϸ� Ű ť�� ����
        if( kIsMouseDataInOutputBuffer() == FALSE )
        {
            // ��� ����(��Ʈ 0x60)���� Ű ��ĵ �ڵ带 �д� �뵵�� �Լ����� Ű����� ���콺
            // �����ʹ� ��� ���۸� �������� ����ϹǷ� ���콺 �����͸� �дµ��� ��� ����
            bTemp = kGetKeyboardScanCode();
            // Ű ť�� ����
            kConvertScanCodeAndPutQueue( bTemp );
        }
        // ���콺 �������̸� ���콺 ť�� ����
        else
        {
            // ��� ����(��Ʈ 0x60)���� Ű ��ĵ �ڵ带 �д� �뵵�� �Լ����� Ű����� ���콺
            // �����ʹ� ��� ���۸� �������� ����ϹǷ� ���콺 �����͸� �дµ��� ��� ����
            bTemp = kGetKeyboardScanCode();
            // ���콺 ť�� ����
            kAccumulateMouseDataAndPutQueue( bTemp );
        } 
    }
    
    // ���ͷ�Ʈ ���Ϳ��� IRQ ��ȣ ����
    iIRQ = iVectorNumber - PIC_IRQSTARTVECTOR;

    // EOI ����
    kSendEOI( iIRQ );
    
    // ���ͷ�Ʈ �߻� Ƚ���� ������Ʈ
    kIncreaseInterruptCount( iIRQ );
    
    // ���� �л�(Load Balancing) ó��
    kProcessLoadBalancing( iIRQ );
}

/**
 *  Ÿ�̸� ���ͷ�Ʈ�� �ڵ鷯
 */
void kTimerHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iTimerInterruptCount = 0;
    int iIRQ;
    BYTE bCurrentAPICID;

    //=========================================================================
    // ���ͷ�Ʈ�� �߻������� �˸����� �޽����� ����ϴ� �κ�
    // ���ͷ�Ʈ ���͸� ȭ�� ������ ���� 2�ڸ� ������ ���
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // �߻��� Ƚ�� ���
    vcBuffer[ 8 ] = '0' + g_iTimerInterruptCount;
    g_iTimerInterruptCount = ( g_iTimerInterruptCount + 1 ) % 10;
    kPrintStringXY( 70, 0, vcBuffer );
    //=========================================================================
    
    // ���ͷ�Ʈ ���Ϳ��� IRQ ��ȣ ����
    iIRQ = iVectorNumber - PIC_IRQSTARTVECTOR;

    // EOI ����
    kSendEOI( iIRQ );
    
    // ���ͷ�Ʈ �߻� Ƚ���� ������Ʈ
    kIncreaseInterruptCount( iIRQ );

    // IRQ 0 ���ͷ�Ʈ ó���� Bootstrap Processor�� ó��
    bCurrentAPICID = kGetAPICID();
    if( bCurrentAPICID == 0 )
    {
        // Ÿ�̸� �߻� Ƚ���� ����
        g_qwTickCount++;
    }

    // �½�ũ�� ����� ���μ����� �ð��� ����
    kDecreaseProcessorTime( bCurrentAPICID );
    // ���μ����� ����� �� �ִ� �ð��� �� ��ٸ� �½�ũ ��ȯ ����
    if( kIsProcessorTimeExpired( bCurrentAPICID ) == TRUE )
    {
        kScheduleInInterrupt();
    }
}

/**
 *  Device Not Available ������ �ڵ鷯
 */
void kDeviceNotAvailableHandler( int iVectorNumber )
{
    TCB* pstFPUTask, * pstCurrentTask;
    QWORD qwLastFPUTaskID;
    BYTE bCurrentAPICID;

    //=========================================================================
    // FPU ���ܰ� �߻������� �˸����� �޽����� ����ϴ� �κ�
    char vcBuffer[] = "[EXC:  , ]";
    static int g_iFPUInterruptCount = 0;

    // ���� ���͸� ȭ�� ������ ���� 2�ڸ� ������ ���
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // �߻��� Ƚ�� ���
    vcBuffer[ 8 ] = '0' + g_iFPUInterruptCount;
    g_iFPUInterruptCount = ( g_iFPUInterruptCount + 1 ) % 10;
    kPrintStringXY( 0, 0, vcBuffer );    
    //=========================================================================
    
    // ���� �ھ��� ���� APIC ID�� Ȯ��
    bCurrentAPICID = kGetAPICID();
    
    // CR0 ��Ʈ�� ���������� TS ��Ʈ�� 0���� ����
    kClearTS();

    // ������ FPU�� ����� �½�ũ�� �ִ��� Ȯ���Ͽ�, �ִٸ� FPU�� ���¸� �½�ũ�� ����
    qwLastFPUTaskID = kGetLastFPUUsedTaskID( bCurrentAPICID );
    pstCurrentTask = kGetRunningTask( bCurrentAPICID );
    
    // ������ FPU�� ����� ���� �ڽ��̸� �ƹ��͵� �� ��
    if( qwLastFPUTaskID == pstCurrentTask->stLink.qwID )
    {
        return ;
    }
    // FPU�� ����� �½�ũ�� ������ FPU ���¸� ����
    else if( qwLastFPUTaskID != TASK_INVALIDID )
    {
        pstFPUTask = kGetTCBInTCBPool( GETTCBOFFSET( qwLastFPUTaskID ) );
        if( ( pstFPUTask != NULL ) && ( pstFPUTask->stLink.qwID == qwLastFPUTaskID ) )
        {
            kSaveFPUContext( pstFPUTask->vqwFPUContext );
        }
    }
    
    // ���� �½�ũ�� FPU�� ����� ���� �ִ� �� Ȯ���Ͽ� FPU�� ����� ���� ���ٸ� 
    // �ʱ�ȭ�ϰ�, ��������� �ִٸ� ����� FPU ���ؽ�Ʈ�� ����
    if( pstCurrentTask->bFPUUsed == FALSE )
    {
        kInitializeFPU();
        pstCurrentTask->bFPUUsed = TRUE;
    }
    else
    {
        kLoadFPUContext( pstCurrentTask->vqwFPUContext );
    }
    
    // FPU�� ����� �½�ũ ID�� ���� �½�ũ�� ����
    kSetLastFPUUsedTaskID( bCurrentAPICID, pstCurrentTask->stLink.qwID );
}

/**
 *  �ϵ� ��ũ���� �߻��ϴ� ���ͷ�Ʈ�� �ڵ鷯
 */
void kHDDHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iHDDInterruptCount = 0;
    BYTE bTemp;
    int iIRQ;

    //=========================================================================
    // ���ͷ�Ʈ�� �߻������� �˸����� �޽����� ����ϴ� �κ�
    // ���ͷ�Ʈ ���͸� ȭ�� ���� ���� 2�ڸ� ������ ���
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // �߻��� Ƚ�� ���
    vcBuffer[ 8 ] = '0' + g_iHDDInterruptCount;
    g_iHDDInterruptCount = ( g_iHDDInterruptCount + 1 ) % 10;
    // ���� ���� �ִ� �޽����� ��ġ�� �ʵ��� (10, 0)�� ���
    kPrintStringXY( 10, 0, vcBuffer );
    //=========================================================================

    // ù ��° PATA ��Ʈ�� ���ͷ�Ʈ �߻� ���θ� TRUE�� ����
    kSetHDDInterruptFlag( TRUE, TRUE );
    
    // ���ͷ�Ʈ ���Ϳ��� IRQ ��ȣ ����
    iIRQ = iVectorNumber - PIC_IRQSTARTVECTOR;

    // EOI ����
    kSendEOI( iIRQ );
    
    // ���ͷ�Ʈ �߻� Ƚ���� ������Ʈ
    kIncreaseInterruptCount( iIRQ );
    
    // ���� �л�(Load Balancing) ó��
    kProcessLoadBalancing( iIRQ );
}

/**
 *  ���콺 ���ͷ�Ʈ�� �ڵ鷯
 */
void kMouseHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iMouseInterruptCount = 0;
    BYTE bTemp;
    int iIRQ;
    
    //=========================================================================
    // ���ͷ�Ʈ�� �߻������� �˸����� �޽����� ����ϴ� �κ�
    // ���ͷ�Ʈ ���͸� ȭ�� ���� ���� 2�ڸ� ������ ���
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // �߻��� Ƚ�� ���
    vcBuffer[ 8 ] = '0' + g_iMouseInterruptCount;
    g_iMouseInterruptCount = ( g_iMouseInterruptCount + 1 ) % 10;
    kPrintStringXY( 0, 0, vcBuffer );
    //=========================================================================

    // ��� ����(��Ʈ 0x60)�� ���ŵ� �����Ͱ� �ִ��� ���θ� Ȯ���Ͽ� ���� �����͸� 
    // Ű ť �Ǵ� ���콺 ť�� ����
    if( kIsOutputBufferFull() == TRUE )
    {
        // ���콺 �����Ͱ� �ƴϸ� Ű ť�� ����
        if( kIsMouseDataInOutputBuffer() == FALSE )
        {
            // ��� ����(��Ʈ 0x60)���� Ű ��ĵ �ڵ带 �д� �뵵�� �Լ����� Ű����� ���콺
            // �����ʹ� ��� ���۸� �������� ����ϹǷ� ���콺 �����͸� �дµ��� ��� ����
            bTemp = kGetKeyboardScanCode();
            // Ű ť�� ����
            kConvertScanCodeAndPutQueue( bTemp );
        }
        // ���콺 �������̸� ���콺 ť�� ����
        else
        {
            // ��� ����(��Ʈ 0x60)���� Ű ��ĵ �ڵ带 �д� �뵵�� �Լ����� Ű����� ���콺
            // �����ʹ� ��� ���۸� �������� ����ϹǷ� ���콺 �����͸� �дµ��� ��� ����
            bTemp = kGetKeyboardScanCode();
            // ���콺 ť�� ����
            kAccumulateMouseDataAndPutQueue( bTemp );
        } 
    }
    
    // ���ͷ�Ʈ ���Ϳ��� IRQ ��ȣ ����
    iIRQ = iVectorNumber - PIC_IRQSTARTVECTOR;

    // EOI ����
    kSendEOI( iIRQ );
    
    // ���ͷ�Ʈ �߻� Ƚ���� ������Ʈ
    kIncreaseInterruptCount( iIRQ );
    
    // ���� �л�(Load Balancing) ó��
    kProcessLoadBalancing( iIRQ );
}

