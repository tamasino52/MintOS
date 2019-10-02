/**
 *  file    Task.c
 *  date    2009/02/19
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   �½�ũ�� ó���ϴ� �Լ��� ���õ� ����
 */

#include "Task.h"
#include "Descriptor.h"
#include "MultiProcessor.h"
#include "DynamicMemory.h"

// �����ٷ� ���� �ڷᱸ��
static SCHEDULER gs_vstScheduler[ MAXPROCESSORCOUNT ];
static TCBPOOLMANAGER gs_stTCBPoolManager;

//==============================================================================
//  �½�ũ Ǯ�� �½�ũ ����
//==============================================================================
/**
 *  �½�ũ Ǯ �ʱ�ȭ
 */
static void kInitializeTCBPool( void )
{
    int i;
    
    kMemSet( &( gs_stTCBPoolManager ), 0, sizeof( gs_stTCBPoolManager ) );
    
    // �½�ũ Ǯ�� ��巹���� �����ϰ� �ʱ�ȭ
    gs_stTCBPoolManager.pstStartAddress = ( TCB* ) TASK_TCBPOOLADDRESS;
    kMemSet( TASK_TCBPOOLADDRESS, 0, sizeof( TCB ) * TASK_MAXCOUNT );

    // TCB�� ID �Ҵ�
    for( i = 0 ; i < TASK_MAXCOUNT ; i++ )
    {
        gs_stTCBPoolManager.pstStartAddress[ i ].stLink.qwID = i;
    }
    
    // TCB�� �ִ� ������ �Ҵ�� Ƚ���� �ʱ�ȭ
    gs_stTCBPoolManager.iMaxCount = TASK_MAXCOUNT;
    gs_stTCBPoolManager.iAllocatedCount = 1;
    
    // ���ɶ� �ʱ�ȭ
    kInitializeSpinLock( &gs_stTCBPoolManager.stSpinLock );
}

/**
 *  TCB�� �Ҵ� ����
 */
static TCB* kAllocateTCB( void )
{
    TCB* pstEmptyTCB;
    int i;
    
    // ����ȭ ó��
    kLockForSpinLock( &gs_stTCBPoolManager.stSpinLock );
    
    if( gs_stTCBPoolManager.iUseCount == gs_stTCBPoolManager.iMaxCount )
    {
        // ����ȭ ó��
        kUnlockForSpinLock( &gs_stTCBPoolManager.stSpinLock );
        return NULL;
    }

    for( i = 0 ; i < gs_stTCBPoolManager.iMaxCount ; i++ )
    {
        // ID�� ���� 32��Ʈ�� 0�̸� �Ҵ���� ���� TCB
        if( ( gs_stTCBPoolManager.pstStartAddress[ i ].stLink.qwID >> 32 ) == 0 )
        {
            pstEmptyTCB = &( gs_stTCBPoolManager.pstStartAddress[ i ] );
            break;
        }
    }

    // ���� 32��Ʈ�� 0�� �ƴ� ������ �����ؼ� �Ҵ�� TCB�� ����
    pstEmptyTCB->stLink.qwID = ( ( QWORD ) gs_stTCBPoolManager.iAllocatedCount << 32 ) | i;
    gs_stTCBPoolManager.iUseCount++;
    gs_stTCBPoolManager.iAllocatedCount++;
    if( gs_stTCBPoolManager.iAllocatedCount == 0 )
    {
        gs_stTCBPoolManager.iAllocatedCount = 1;
    }

    // ����ȭ ó��
    kUnlockForSpinLock( &gs_stTCBPoolManager.stSpinLock );

    return pstEmptyTCB;
}

/**
 *  TCB�� ������
 */
static void kFreeTCB( QWORD qwID )
{
    int i;
    
    // �½�ũ ID�� ���� 32��Ʈ�� �ε��� ������ ��
    i = GETTCBOFFSET( qwID );
    
    // TCB�� �ʱ�ȭ�ϰ� ID ����
    kMemSet( &( gs_stTCBPoolManager.pstStartAddress[ i ].stContext ), 0, sizeof( CONTEXT ) );

    // ����ȭ ó��
    kLockForSpinLock( &gs_stTCBPoolManager.stSpinLock );
    
    gs_stTCBPoolManager.pstStartAddress[ i ].stLink.qwID = i;
    
    gs_stTCBPoolManager.iUseCount--;

    // ����ȭ ó��
    kUnlockForSpinLock( &gs_stTCBPoolManager.stSpinLock );
}

/**
 *  �½�ũ�� ����
 *      �½�ũ ID�� ���� ���� Ǯ���� ���� �ڵ� �Ҵ�
 *      ���μ��� �� ������ ��� ���� ����
 *      bAffinity�� �½�ũ�� �����ϰ� ���� �ھ��� ID�� ���� ����
 */
TCB* kCreateTask( QWORD qwFlags, void* pvMemoryAddress, QWORD qwMemorySize, 
                  QWORD qwEntryPointAddress, BYTE bAffinity )
{
    TCB* pstTask, * pstProcess;
    void* pvStackAddress;
    BYTE bCurrentAPICID;
    
    // ���� �ھ��� ���� APIC ID�� Ȯ��
    bCurrentAPICID = kGetAPICID();
    
    // �½�ũ �ڷᱸ�� �Ҵ�
    pstTask = kAllocateTCB();
    if( pstTask == NULL )
    {
        return NULL;
    }
    
    // ���� �޸� �������� ���� �Ҵ�
    pvStackAddress = kAllocateMemory( TASK_STACKSIZE );
    if( pvStackAddress == NULL )
    {
        kFreeTCB( pstTask->stLink.qwID );
        return NULL;
    }

    // �Ӱ� ���� ����
    kLockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
    
    // ���� ���μ��� �Ǵ� �����尡 ���� ���μ����� �˻�
    pstProcess = kGetProcessByThread( kGetRunningTask( bCurrentAPICID ) );
    // ���� ���μ����� ���ٸ� �ƹ��� �۾��� ���� ����
    if( pstProcess == NULL )
    {
        kFreeTCB( pstTask->stLink.qwID );
        kFreeMemory( pvStackAddress );
        // �Ӱ� ���� ��
        kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
        return NULL;
    }

    // �����带 �����ϴ� ����� ���� ���� ���μ����� �ڽ� ������ ����Ʈ�� ������
    if( qwFlags & TASK_FLAGS_THREAD )
    {
        // ���� �������� ���μ����� ã�Ƽ� ������ �����忡 ���μ��� ������ ���
        pstTask->qwParentProcessID = pstProcess->stLink.qwID;
        pstTask->pvMemoryAddress = pstProcess->pvMemoryAddress;
        pstTask->qwMemorySize = pstProcess->qwMemorySize;
        
        // �θ� ���μ����� �ڽ� ������ ����Ʈ�� �߰�
        kAddListToTail( &( pstProcess->stChildThreadList ), &( pstTask->stThreadLink ) );
    }
    // ���μ����� �Ķ���ͷ� �Ѿ�� ���� �״�� ����
    else
    {
        pstTask->qwParentProcessID = pstProcess->stLink.qwID;
        pstTask->pvMemoryAddress = pvMemoryAddress;
        pstTask->qwMemorySize = qwMemorySize;
    }
    
    // �������� ID�� �½�ũ ID�� �����ϰ� ����
    pstTask->stThreadLink.qwID = pstTask->stLink.qwID;    
    // �Ӱ� ���� ��
    kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
    
    
    // TCB�� ������ �� �غ� ����Ʈ�� �����Ͽ� �����ٸ��� �� �ֵ��� ��
    kSetUpTask( pstTask, qwFlags, qwEntryPointAddress, pvStackAddress, 
            TASK_STACKSIZE );

    // �ڽ� ������ ����Ʈ�� �ʱ�ȭ
    kInitializeList( &( pstTask->stChildThreadList ) );
    
    // FPU ��� ���θ� ������� ���� ������ �ʱ�ȭ
    pstTask->bFPUUsed = FALSE;
    
    // ���� �ھ��� ���� APIC ID�� �½�ũ�� ����
    pstTask->bAPICID = bCurrentAPICID;
    
    // ���μ��� ģȭ��(Affinity)�� ����
    pstTask->bAffinity = bAffinity;

    // ���� �л��� ����Ͽ� �����ٷ��� �½�ũ�� �߰� 
    kAddTaskToSchedulerWithLoadBalancing( pstTask );
    return pstTask;
}

/**
 *  �Ķ���͸� �̿��ؼ� TCB�� ����
 */
static void kSetUpTask( TCB* pstTCB, QWORD qwFlags, QWORD qwEntryPointAddress,
                 void* pvStackAddress, QWORD qwStackSize )
{
    // ���ؽ�Ʈ �ʱ�ȭ
    kMemSet( pstTCB->stContext.vqRegister, 0, sizeof( pstTCB->stContext.vqRegister ) );
    
    // ���ÿ� ���õ� RSP, RBP �������� ����
    pstTCB->stContext.vqRegister[ TASK_RSPOFFSET ] = ( QWORD ) pvStackAddress + 
            qwStackSize - 8;
    pstTCB->stContext.vqRegister[ TASK_RBPOFFSET ] = ( QWORD ) pvStackAddress + 
            qwStackSize - 8;
    
    // Return Address ������ kExitTask() �Լ��� ��巹���� �����Ͽ� �½�ũ�� ��Ʈ��
    // ����Ʈ �Լ��� ���������� ���ÿ� kExitTask() �Լ��� �̵��ϵ��� ��
    *( QWORD * ) ( ( QWORD ) pvStackAddress + qwStackSize - 8 ) = ( QWORD ) kExitTask;

    // ���׸�Ʈ ������ ����
    // Ŀ�� �½�ũ�� ���� Ŀ�� ���� ���׸�Ʈ ��ũ���͸� ����
    if( ( qwFlags & TASK_FLAGS_USERLEVEL ) == 0 )
    {
        pstTCB->stContext.vqRegister[ TASK_CSOFFSET ] = GDT_KERNELCODESEGMENT | SELECTOR_RPL_0;
        pstTCB->stContext.vqRegister[ TASK_DSOFFSET ] = GDT_KERNELDATASEGMENT | SELECTOR_RPL_0;
        pstTCB->stContext.vqRegister[ TASK_ESOFFSET ] = GDT_KERNELDATASEGMENT | SELECTOR_RPL_0;
        pstTCB->stContext.vqRegister[ TASK_FSOFFSET ] = GDT_KERNELDATASEGMENT | SELECTOR_RPL_0;
        pstTCB->stContext.vqRegister[ TASK_GSOFFSET ] = GDT_KERNELDATASEGMENT | SELECTOR_RPL_0;
        pstTCB->stContext.vqRegister[ TASK_SSOFFSET ] = GDT_KERNELDATASEGMENT | SELECTOR_RPL_0;
    }
    // ���� �½�ũ�� ���� ���� ���� ���׸�Ʈ ��ũ���͸� ����
    else
    {
        pstTCB->stContext.vqRegister[ TASK_CSOFFSET ] = GDT_USERCODESEGMENT | SELECTOR_RPL_3;
        pstTCB->stContext.vqRegister[ TASK_DSOFFSET ] = GDT_USERDATASEGMENT | SELECTOR_RPL_3;
        pstTCB->stContext.vqRegister[ TASK_ESOFFSET ] = GDT_USERDATASEGMENT | SELECTOR_RPL_3;
        pstTCB->stContext.vqRegister[ TASK_FSOFFSET ] = GDT_USERDATASEGMENT | SELECTOR_RPL_3;
        pstTCB->stContext.vqRegister[ TASK_GSOFFSET ] = GDT_USERDATASEGMENT | SELECTOR_RPL_3;
        pstTCB->stContext.vqRegister[ TASK_SSOFFSET ] = GDT_USERDATASEGMENT | SELECTOR_RPL_3;
    }
    
    // RIP �������Ϳ� ���ͷ�Ʈ �÷��� ����
    pstTCB->stContext.vqRegister[ TASK_RIPOFFSET ] = qwEntryPointAddress;

    // RFLAGS ���������� IF ��Ʈ(��Ʈ 9)�� 1�� �����Ͽ� ���ͷ�Ʈ Ȱ��ȭ�ϰ�
    // IOPL ��Ʈ(��Ʈ 12~13)�� 3���� �����Ͽ� ���� ���������� I/O ��Ʈ�� ������ �� �ֵ��� ��
    pstTCB->stContext.vqRegister[ TASK_RFLAGSOFFSET ] |= 0x3200;
    
    // ���ð� �÷��� ����
    pstTCB->pvStackAddress = pvStackAddress;
    pstTCB->qwStackSize = qwStackSize;
    pstTCB->qwFlags = qwFlags;
}

//==============================================================================
//  �����ٷ� ����
//==============================================================================
/**
 *  �����ٷ��� �ʱ�ȭ
 *      �����ٷ��� �ʱ�ȭ�ϴµ� �ʿ��� TCB Ǯ�� init �½�ũ�� ���� �ʱ�ȭ
 */
void kInitializeScheduler( void )
{
    int i;
    int j;
    BYTE bCurrentAPICID;
    TCB* pstTask;
    
    // ���� �ھ��� ���� APIC ID Ȯ��
    bCurrentAPICID = kGetAPICID();

    // Bootstrap Processor�� �½�ũ Ǯ�� �����ٷ� �ڷᱸ���� ��� �ʱ�ȭ
    if( bCurrentAPICID == 0 )
    {
        // �½�ũ Ǯ �ʱ�ȭ
        kInitializeTCBPool();
        
        // �غ� ����Ʈ�� �켱 ������ ���� Ƚ���� �ʱ�ȭ�ϰ� ��� ����Ʈ�� ���ɶ��� �ʱ�ȭ
        for( j = 0 ; j < MAXPROCESSORCOUNT ; j++ )
        {
            // �غ� ����Ʈ �ʱ�ȭ
            for( i = 0 ; i < TASK_MAXREADYLISTCOUNT ; i++ )
            {
                kInitializeList( &( gs_vstScheduler[ j ].vstReadyList[ i ] ) );
                gs_vstScheduler[ j ].viExecuteCount[ i ] = 0;
            }    
            // ��� ����Ʈ �ʱ�ȭ
            kInitializeList( &( gs_vstScheduler[ j ].stWaitList ) );

            // ���ɶ� �ʱ�ȭ
            kInitializeSpinLock( &( gs_vstScheduler[ j ].stSpinLock ) );
        }
    }    
    
    // TCB�� �Ҵ� �޾� ������ ������ �½�ũ�� Ŀ�� ������ ���μ����� ����
    pstTask = kAllocateTCB();
    gs_vstScheduler[ bCurrentAPICID ].pstRunningTask = pstTask;
    
    // BSP�� �ܼ� ���̳� AP�� ���� �½�ũ(Idle Task)�� ��� ���� �ھ���� �����ϵ���
    // ���� APIC ID�� ���μ��� ģȭ���� ���� �ھ��� ���� APIC ID�� ����
    pstTask->bAPICID = bCurrentAPICID;
    pstTask->bAffinity = bCurrentAPICID;
    
    // Bootstrap Processor�� �ܼ� ���� ����
    if( bCurrentAPICID == 0 )
    {
        pstTask->qwFlags = TASK_FLAGS_HIGHEST | TASK_FLAGS_PROCESS | TASK_FLAGS_SYSTEM;
    }
    // Application Processor�� Ư���� ����� �½�ũ�� �����Ƿ� ����(Idle) �½�ũ�� ����
    else
    {
        pstTask->qwFlags = TASK_FLAGS_LOWEST | TASK_FLAGS_PROCESS | TASK_FLAGS_SYSTEM | TASK_FLAGS_IDLE;
    }
    
    pstTask->qwParentProcessID = pstTask->stLink.qwID;
    pstTask->pvMemoryAddress = ( void* ) 0x100000;
    pstTask->qwMemorySize = 0x500000;
    pstTask->pvStackAddress = ( void* ) 0x600000;
    pstTask->qwStackSize = 0x100000;
    
    // ���μ��� ������ ����ϴµ� ����ϴ� �ڷᱸ�� �ʱ�ȭ
    gs_vstScheduler[ bCurrentAPICID ].qwSpendProcessorTimeInIdleTask = 0;
    gs_vstScheduler[ bCurrentAPICID ].qwProcessorLoad = 0;
    
    // FPU�� ����� �½�ũ ID�� ��ȿ���� ���� ������ �ʱ�ȭ
    gs_vstScheduler[ bCurrentAPICID ].qwLastFPUUsedTaskID = TASK_INVALIDID;
}

/**
 *  ���� ���� ���� �½�ũ�� ����
 */
void kSetRunningTask( BYTE bAPICID, TCB* pstTask )
{
    // �Ӱ� ���� ����
    kLockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );

    gs_vstScheduler[ bAPICID ].pstRunningTask = pstTask;

    // �Ӱ� ���� ��
    kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );
}

/**
 *  ���� ���� ���� �½�ũ�� ��ȯ
 */
TCB* kGetRunningTask( BYTE bAPICID )
{
    TCB* pstRunningTask;
    
    // �Ӱ� ���� ����
    kLockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );
    
    pstRunningTask = gs_vstScheduler[ bAPICID ].pstRunningTask;
    
    // �Ӱ� ���� ��
    kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );

    return pstRunningTask;
}

/**
 *  �½�ũ ����Ʈ���� �������� ������ �½�ũ�� ����
 */
static TCB* kGetNextTaskToRun( BYTE bAPICID )
{
    TCB* pstTarget = NULL;
    int iTaskCount, i, j;
    
    // ť�� �½�ũ�� ������ ��� ť�� �½�ũ�� 1ȸ�� ����� ���, ��� ť�� ���μ�����
    // �纸�Ͽ� �½�ũ�� �������� ���� �� ������ NULL�� ��� �ѹ� �� ����
    for( j = 0 ; j < 2 ; j++ )
    {
        // ���� �켱 �������� ���� �켱 �������� ����Ʈ�� Ȯ���Ͽ� �����ٸ��� �½�ũ�� ����
        for( i = 0 ; i < TASK_MAXREADYLISTCOUNT ; i++ )
        {
            iTaskCount = kGetListCount( &( gs_vstScheduler[ bAPICID ].
                    vstReadyList[ i ] ) );
            
            // ���� ������ Ƚ������ ����Ʈ�� �½�ũ ���� �� ������ ���� �켱 ������
            // �½�ũ�� ������
            if( gs_vstScheduler[ bAPICID ].viExecuteCount[ i ] < iTaskCount )
            {
                pstTarget = ( TCB* ) kRemoveListFromHeader( 
                    &( gs_vstScheduler[ bAPICID ].vstReadyList[ i ] ) );
                gs_vstScheduler[ bAPICID ].viExecuteCount[ i ]++;
                break;            
            }
            // ���� ������ Ƚ���� �� ������ ���� Ƚ���� �ʱ�ȭ�ϰ� ���� �켱 ������ �纸��
            else
            {
                gs_vstScheduler[ bAPICID ].viExecuteCount[ i ] = 0;
            }
        }
        
        // ���� ������ �½�ũ�� ã������ ����
        if( pstTarget != NULL )
        {
            break;
        }
    }    
    return pstTarget;
}

/**
 *  �½�ũ�� �����ٷ��� �غ� ����Ʈ�� ����
 */
static BOOL kAddTaskToReadyList( BYTE bAPICID, TCB* pstTask )
{
    BYTE bPriority;
    
    bPriority = GETPRIORITY( pstTask->qwFlags );
    if( bPriority == TASK_FLAGS_WAIT )
    {
        kAddListToTail( &( gs_vstScheduler[ bAPICID ].stWaitList ), pstTask );
        return TRUE;
    }
    else if( bPriority >= TASK_MAXREADYLISTCOUNT )
    {
        return FALSE;
    }
    
    kAddListToTail( &( gs_vstScheduler[ bAPICID ].vstReadyList[ bPriority ] ), 
            pstTask );
    return TRUE;
}

/**
 *  �غ� ť���� �½�ũ�� ����
 */
static TCB* kRemoveTaskFromReadyList( BYTE bAPICID, QWORD qwTaskID )
{
    TCB* pstTarget;
    BYTE bPriority;
    
    // �½�ũ ID�� ��ȿ���� ������ ����
    if( GETTCBOFFSET( qwTaskID ) >= TASK_MAXCOUNT )
    {
        return NULL;
    }
    
    // TCB Ǯ���� �ش� �½�ũ�� TCB�� ã�� ������ ID�� ��ġ�ϴ°� Ȯ��
    pstTarget = &( gs_stTCBPoolManager.pstStartAddress[ GETTCBOFFSET( qwTaskID ) ] );
    if( pstTarget->stLink.qwID != qwTaskID )
    {
        return NULL;
    }
    
    // �½�ũ�� �����ϴ� �غ� ����Ʈ���� �½�ũ ����
    bPriority = GETPRIORITY( pstTarget->qwFlags );
    if( bPriority >= TASK_MAXREADYLISTCOUNT )
    {
        return NULL;
    }    

    pstTarget = kRemoveList( &( gs_vstScheduler[ bAPICID ].vstReadyList[ bPriority ]), 
                     qwTaskID );
    return pstTarget;
}

/**
 *  �½�ũ�� ���Ե� �����ٷ��� ID�� ��ȯ�ϰ�, �ش� �����ٷ��� ���ɶ��� ���
 */
static BOOL kFindSchedulerOfTaskAndLock( QWORD qwTaskID, BYTE* pbAPICID )
{
    TCB* pstTarget;
    BYTE bAPICID;
    
    while( 1 )
    {
        // �½�ũ ID�� �½�ũ �ڷᱸ���� ã�Ƽ� ��� �����ٷ����� ���� ������ Ȯ��
        pstTarget = &( gs_stTCBPoolManager.pstStartAddress[ GETTCBOFFSET( qwTaskID ) ] );
        if( ( pstTarget == NULL ) || ( pstTarget->stLink.qwID != qwTaskID ) )
        {
            return FALSE;
        }
    
        // ���� �½�ũ�� ����Ǵ� �ھ��� ID�� Ȯ��
        bAPICID = pstTarget->bAPICID;
        
        // �Ӱ� ���� ����
        kLockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );

        // ���ɶ��� ȹ���� ���� �ٽ� Ȯ���Ͽ� ���� �ھ�� ����Ǵ��� Ȯ��
        // �½�ũ�� ����Ǵ� �ھ ã�� �� ��Ȯ�ϰ� ���ɶ��� �ɱ� ���� 2������ �˻�
        pstTarget = &( gs_stTCBPoolManager.pstStartAddress[ GETTCBOFFSET( qwTaskID ) ] );
        if( pstTarget->bAPICID == bAPICID )
        {
            break;
        }
        
        // �½�ũ �ڷᱸ���� ����� ���� APIC ID�� ���� ���ɶ��� ȹ���ϱ� ���� �İ� 
        // �ٸ��ٸ�, ���ɶ��� ȹ���ϴ� ���� �½�ũ�� �ٸ� �ھ�� �Űܰ� ����
        // ���� �ٽ� ���ɶ��� �����ϰ� �Ű��� �ھ��� ���ɶ��� ȹ���ؾ� ��
        // �Ӱ� ���� ��
        kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );
    }  
    
    *pbAPICID = bAPICID;
    return TRUE;
}

/**
 *  �½�ũ�� �켱 ������ ����
 */
BOOL kChangePriority( QWORD qwTaskID, BYTE bPriority )
{
    TCB* pstTarget;
    BYTE bAPICID;
    
    if( bPriority > TASK_MAXREADYLISTCOUNT )
    {
        return FALSE;
    }
    
    // �½�ũ�� ���Ե� �ھ��� ���� APIC ID�� ã�� ��, ���ɶ��� ���
    if( kFindSchedulerOfTaskAndLock( qwTaskID, &bAPICID ) == FALSE )
    {
        return FALSE;
    }
    
    // �������� �½�ũ�̸� �켱 ������ ����
    // PIT ��Ʈ�ѷ��� ���ͷ�Ʈ(IRQ 0)�� �߻��Ͽ� �½�ũ ��ȯ�� ����� �� ����� 
    // �켱 ������ ����Ʈ�� �̵�
    pstTarget = gs_vstScheduler[ bAPICID ].pstRunningTask;
    if( pstTarget->stLink.qwID == qwTaskID )
    {
        SETPRIORITY( pstTarget->qwFlags, bPriority );
    }
    // �������� �½�ũ�� �ƴϸ� �غ� ����Ʈ���� ã�Ƽ� �ش� �켱 ������ ����Ʈ�� �̵�
    else
    {
        // �غ� ����Ʈ���� �½�ũ�� ã�� ���ϸ� ���� �½�ũ�� ã�Ƽ� �켱 ������ ����
        pstTarget = kRemoveTaskFromReadyList( bAPICID, qwTaskID );
        if( pstTarget == NULL )
        {
            // �½�ũ ID�� ���� ã�Ƽ� ����
            pstTarget = kGetTCBInTCBPool( GETTCBOFFSET( qwTaskID ) );
            if( pstTarget != NULL )
            {
                // �켱 ������ ����
                SETPRIORITY( pstTarget->qwFlags, bPriority );
            }
        }
        else
        {
            // �켱 ������ �����ϰ� �غ� ����Ʈ�� �ٽ� ����
            SETPRIORITY( pstTarget->qwFlags, bPriority );
            kAddTaskToReadyList( bAPICID, pstTarget );
        }
    }
    // �Ӱ� ���� ��
    kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );
    
    return TRUE;    
}

/**
 *  �ٸ� �½�ũ�� ã�Ƽ� ��ȯ
 *      ���ͷ�Ʈ�� ���ܰ� �߻����� �� ȣ���ϸ� �ȵ�
 *      ��, ���ͷ�Ʈ�� ���ܿ��� �½�ũ�� �����ϴ� ���� ��� ����
 */
BOOL kSchedule( void )
{
    TCB* pstRunningTask, * pstNextTask;
    BOOL bPreviousInterrupt;
    BYTE bCurrentAPICID;
    
    // ��ȯ�ϴ� ���� ���ͷ�Ʈ�� �߻��Ͽ� �½�ũ ��ȯ�� �� �Ͼ�� ����ϹǷ� ��ȯ�ϴ� 
    // ���� ���ͷ�Ʈ�� �߻����� ���ϵ��� ����
    bPreviousInterrupt = kSetInterruptFlag( FALSE );
    
    // ���� ���� APIC ID Ȯ��
    bCurrentAPICID = kGetAPICID();
    
    // ��ȯ�� �½�ũ�� �־�� ��
    if( kGetReadyTaskCount( bCurrentAPICID ) < 1 )
    {
        kSetInterruptFlag( bPreviousInterrupt );
        return FALSE;
    }
    
    // �Ӱ� ���� ����
    kLockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );

    // ������ ���� �½�ũ�� ����
    pstNextTask = kGetNextTaskToRun( bCurrentAPICID );
    if( pstNextTask == NULL )
    {
        // �Ӱ� ���� ��
        kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
        kSetInterruptFlag( bPreviousInterrupt );
        return FALSE;
    }
    
    // ���� �������� �½�ũ�� ������ ������ �� ���ؽ�Ʈ ��ȯ
    pstRunningTask = gs_vstScheduler[ bCurrentAPICID ].pstRunningTask; 
    gs_vstScheduler[ bCurrentAPICID ].pstRunningTask = pstNextTask;

    // ���� �½�ũ���� ��ȯ�Ǿ��ٸ� ����� ���μ��� �ð��� ������Ŵ
    if( ( pstRunningTask->qwFlags & TASK_FLAGS_IDLE ) == TASK_FLAGS_IDLE )
    {
        gs_vstScheduler[ bCurrentAPICID ].qwSpendProcessorTimeInIdleTask += 
            TASK_PROCESSORTIME - gs_vstScheduler[ bCurrentAPICID ].iProcessorTime;
    }

    // ������ ������ �½�ũ�� FPU�� �� �½�ũ�� �ƴ϶�� TS ��Ʈ�� ����
    if( gs_vstScheduler[ bCurrentAPICID ].qwLastFPUUsedTaskID != 
        pstNextTask->stLink.qwID )
    {
        kSetTS();
    }
    else
    {
        kClearTS();
    }

    // �½�ũ ���� �÷��װ� ������ ��� ���ؽ�Ʈ�� ������ �ʿ䰡 �����Ƿ�, ��� ����Ʈ��
    // �����ϰ� ���ؽ�Ʈ ��ȯ
    if( pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK )
    {
        kAddListToTail( &( gs_vstScheduler[ bCurrentAPICID ].stWaitList ), pstRunningTask );

        // �Ӱ� ���� ��
        kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );

        // �½�ũ ��ȯ
        kSwitchContext( NULL, &( pstNextTask->stContext ) );
    }
    else
    {
        kAddTaskToReadyList( bCurrentAPICID, pstRunningTask );
        
        // �Ӱ� ���� ��
        kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );

        // �½�ũ ��ȯ
        kSwitchContext( &( pstRunningTask->stContext ), &( pstNextTask->stContext ) );
    }

    // ���μ��� ��� �ð��� ������Ʈ
    gs_vstScheduler[ bCurrentAPICID ].iProcessorTime = TASK_PROCESSORTIME;
    
    // ���ͷ�Ʈ �÷��� ����
    kSetInterruptFlag( bPreviousInterrupt );
    return FALSE;
}

/**
 *  ���ͷ�Ʈ�� �߻����� ��, �ٸ� �½�ũ�� ã�� ��ȯ
 *      �ݵ�� ���ͷ�Ʈ�� ���ܰ� �߻����� �� ȣ���ؾ� ��
 */
BOOL kScheduleInInterrupt( void )
{
    TCB* pstRunningTask, * pstNextTask;
    char* pcContextAddress;
    BYTE bCurrentAPICID;
    QWORD qwISTStartAddress;
        
    // ���� ���� APIC ID Ȯ��
    bCurrentAPICID = kGetAPICID();
    
    // �Ӱ� ���� ����
    kLockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
        
    // ��ȯ�� �½�ũ�� ������ ����
    pstNextTask = kGetNextTaskToRun( bCurrentAPICID );
    if( pstNextTask == NULL )
    {
        // �Ӱ� ���� ��
        kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
        return FALSE;
    }
    
    //==========================================================================
    //  �½�ũ ��ȯ ó��   
    //      ���ͷ�Ʈ �ڵ鷯���� ������ ���ؽ�Ʈ�� �ٸ� ���ؽ�Ʈ�� ����� ������� ó��
    //==========================================================================
    // IST�� ���κк��� �ھ� 0 -> �ھ� 15 ������ 64Kbyte�� ���� �����Ƿ�, ���� APIC ID��
    // �̿��ؼ� IST ��巹���� ���
    qwISTStartAddress = IST_STARTADDRESS + IST_SIZE - 
                        ( IST_SIZE / MAXPROCESSORCOUNT * bCurrentAPICID );
    pcContextAddress = ( char* ) qwISTStartAddress - sizeof( CONTEXT );
    
    pstRunningTask = gs_vstScheduler[ bCurrentAPICID ].pstRunningTask;
    gs_vstScheduler[ bCurrentAPICID ].pstRunningTask = pstNextTask;

    // ���� �½�ũ���� ��ȯ�Ǿ��ٸ� ����� Tick Count�� ������Ŵ
    if( ( pstRunningTask->qwFlags & TASK_FLAGS_IDLE ) == TASK_FLAGS_IDLE )
    {
        gs_vstScheduler[ bCurrentAPICID ].qwSpendProcessorTimeInIdleTask += TASK_PROCESSORTIME;
    }
    
    // �½�ũ ���� �÷��װ� ������ ���, ���ؽ�Ʈ�� �������� �ʰ� ��� ����Ʈ���� ����
    if( pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK )
    {    
        kAddListToTail( &( gs_vstScheduler[ bCurrentAPICID ].stWaitList ), 
                        pstRunningTask );
    }
    // �½�ũ�� ������� ������ IST�� �ִ� ���ؽ�Ʈ�� �����ϰ�, ���� �½�ũ�� �غ� ����Ʈ��
    // �ű�
    else
    {
        kMemCpy( &( pstRunningTask->stContext ), pcContextAddress, sizeof( CONTEXT ) );
    }
    
    // ������ ������ �½�ũ�� FPU�� �� �½�ũ�� �ƴ϶�� TS bit ����
    if( gs_vstScheduler[ bCurrentAPICID ].qwLastFPUUsedTaskID != 
        pstNextTask->stLink.qwID )
    {
        kSetTS();
    }
    else
    {
        kClearTS();
    }
    
    // �Ӱ� ���� ��
    kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
    
    // ��ȯ�ؼ� ������ �½�ũ�� Running Task�� �����ϰ� ���ؽ�Ʈ�� IST�� �����ؼ�
    // �ڵ����� �½�ũ ��ȯ�� �Ͼ���� ��
    kMemCpy( pcContextAddress, &( pstNextTask->stContext ), sizeof( CONTEXT ) );
    
    // �����ϴ� �½�ũ�� �ƴϸ� �����ٷ��� �½�ũ �߰�
    if( ( pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK ) != TASK_FLAGS_ENDTASK )
    {
        // �����ٷ��� �½�ũ�� �߰�, ���� �л��� �����
        kAddTaskToSchedulerWithLoadBalancing( pstRunningTask );
    }    
    
    // ���μ��� ��� �ð��� ������Ʈ
    gs_vstScheduler[ bCurrentAPICID ].iProcessorTime = TASK_PROCESSORTIME;
    
    return TRUE;
}

/**
 *  ���μ����� ����� �� �ִ� �ð��� �ϳ� ����
 */
void kDecreaseProcessorTime( BYTE bAPICID )
{
    gs_vstScheduler[ bAPICID ].iProcessorTime--;
}

/**
 *  ���μ����� ����� �� �ִ� �ð��� �� �Ǿ����� ���θ� ��ȯ
 */
BOOL kIsProcessorTimeExpired( BYTE bAPICID )
{
    if( gs_vstScheduler[ bAPICID ].iProcessorTime <= 0 )
    {
        return TRUE;
    }
    return FALSE;
}

/**
 *  �½�ũ�� ����
 */
BOOL kEndTask( QWORD qwTaskID )
{
    TCB* pstTarget;
    BYTE bPriority;
    BYTE bAPICID;
    
    // �½�ũ�� ���Ե� �ھ��� ���� APIC ID�� ã�� ��, ���ɶ��� ���
    if( kFindSchedulerOfTaskAndLock( qwTaskID, &bAPICID ) == FALSE )
    {
        return FALSE;
    }
    
    // ���� �������� �½�ũ�̸� EndTask ��Ʈ�� �����ϰ� �½�ũ�� ��ȯ
    pstTarget = gs_vstScheduler[ bAPICID ].pstRunningTask;
    if( pstTarget->stLink.qwID == qwTaskID )
    {
        pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
        SETPRIORITY( pstTarget->qwFlags, TASK_FLAGS_WAIT );
        
        // �Ӱ� ���� ��
        kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );
        
        // ���� �����ٷ����� �������� �½�ũ�� ��츸 �Ʒ��� ����
        if( kGetAPICID() == bAPICID )
        {
            kSchedule();
            
            // �½�ũ�� ��ȯ �Ǿ����Ƿ� �Ʒ� �ڵ�� ���� ������� ����
            while( 1 ) 
            {
                ;
            }
        }
        
        return TRUE;
    }
    
    // ���� ���� �½�ũ�� �ƴϸ� �غ� ť���� ���� ã�Ƽ� ��� ����Ʈ�� ����
    // �غ� ����Ʈ���� �½�ũ�� ã�� ���ϸ� ���� �½�ũ�� ã�Ƽ� �½�ũ ���� ��Ʈ��
    // ����
    pstTarget = kRemoveTaskFromReadyList( bAPICID, qwTaskID );
    if( pstTarget == NULL )
    {
        // �½�ũ ID�� ���� ã�Ƽ� ����
        pstTarget = kGetTCBInTCBPool( GETTCBOFFSET( qwTaskID ) );
        if( pstTarget != NULL )
        {
            pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
            SETPRIORITY( pstTarget->qwFlags, TASK_FLAGS_WAIT );
        }
        
        // �Ӱ� ���� ��
        kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );
        return TRUE;
    }
    
    pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
    SETPRIORITY( pstTarget->qwFlags, TASK_FLAGS_WAIT );
    kAddListToTail( &( gs_vstScheduler[ bAPICID ].stWaitList ), pstTarget );
    
    // �Ӱ� ���� ��
    kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );
    return TRUE;
}

/**
 *  �½�ũ�� �ڽ��� ������
 */
void kExitTask( void )
{
    kEndTask( gs_vstScheduler[ kGetAPICID() ].pstRunningTask->stLink.qwID );
}

/**
 *  �غ� ť�� �ִ� ��� �½�ũ�� ���� ��ȯ
 */
int kGetReadyTaskCount( BYTE bAPICID )
{
    int iTotalCount = 0;
    int i;

    // �Ӱ� ���� ����
    kLockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );

    // ��� �غ� ť�� Ȯ���Ͽ� �½�ũ ������ ����
    for( i = 0 ; i < TASK_MAXREADYLISTCOUNT ; i++ )
    {
        iTotalCount += kGetListCount( &( gs_vstScheduler[ bAPICID ].
                vstReadyList[ i ] ) );
    }
    
    // �Ӱ� ���� ��
    kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );
    return iTotalCount ;
}

/**
 *  ��ü �½�ũ�� ���� ��ȯ
 */ 
int kGetTaskCount( BYTE bAPICID )
{
    int iTotalCount;
    
    // �غ� ť�� �½�ũ ���� ���� ��, ��� ť�� �½�ũ ���� ���� ���� ���� �½�ũ ���� ����
    iTotalCount = kGetReadyTaskCount( bAPICID );
    
    // �Ӱ� ���� ����
    kLockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );
    
    iTotalCount += kGetListCount( &( gs_vstScheduler[ bAPICID ].stWaitList ) ) + 1;

    // �Ӱ� ���� ��
    kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );
    return iTotalCount;
}

/**
 *  TCB Ǯ���� �ش� �������� TCB�� ��ȯ
 */
TCB* kGetTCBInTCBPool( int iOffset )
{
    if( ( iOffset < -1 ) && ( iOffset > TASK_MAXCOUNT ) )
    {
        return NULL;
    }
    
    return &( gs_stTCBPoolManager.pstStartAddress[ iOffset ] );
}

/**
 *  �½�ũ�� �����ϴ��� ���θ� ��ȯ
 */
BOOL kIsTaskExist( QWORD qwID )
{
    TCB* pstTCB;
    
    // ID�� TCB�� ��ȯ
    pstTCB = kGetTCBInTCBPool( GETTCBOFFSET( qwID ) );
    // TCB�� ���ų� ID�� ��ġ���� ������ �������� �ʴ� ����
    if( ( pstTCB == NULL ) || ( pstTCB->stLink.qwID != qwID ) )
    {
        return FALSE;
    }
    return TRUE;
}

/**
 *  ���μ����� ������ ��ȯ
 */
QWORD kGetProcessorLoad( BYTE bAPICID )
{
    return gs_vstScheduler[ bAPICID ].qwProcessorLoad;
}

/**
 *  �����尡 �Ҽӵ� ���μ����� ��ȯ
 */
static TCB* kGetProcessByThread( TCB* pstThread )
{
    TCB* pstProcess;
    
    // ���� ���� ���μ����̸� �ڽ��� ��ȯ
    if( pstThread->qwFlags & TASK_FLAGS_PROCESS )
    {
        return pstThread;
    }
    
    // ���� ���μ����� �ƴ϶��, �θ� ���μ����� ������ �½�ũ ID�� ���� 
    // TCB Ǯ���� �½�ũ �ڷᱸ�� ����
    pstProcess = kGetTCBInTCBPool( GETTCBOFFSET( pstThread->qwParentProcessID ) );

    // ���� ���μ����� ���ų�, �½�ũ ID�� ��ġ���� �ʴ´ٸ� NULL�� ��ȯ
    if( ( pstProcess == NULL ) || ( pstProcess->stLink.qwID != pstThread->qwParentProcessID ) )
    {
        return NULL;
    }
    
    return pstProcess;
}

/**
 *  �� �����ٷ��� �½�ũ ���� �̿��Ͽ� ������ �����ٷ��� �½�ũ �߰�
 *      ���� �л� ����� ������� �ʴ� ��� ���� �ھ ����
 *      ���� �л��� ������� �ʴ� ���, �½�ũ�� ���� ����Ǵ� �ھ�� ��� �����ϹǷ�
 *      pstTask���� ��� APIC ID�� �����Ǿ� �־�� ��
 */
void kAddTaskToSchedulerWithLoadBalancing( TCB* pstTask )
{
    BYTE bCurrentAPICID;
    BYTE bTargetAPICID;
    
    // �½�ũ�� �����ϴ� �ھ��� APIC�� Ȯ��
    bCurrentAPICID = pstTask->bAPICID;
    
    // ���� �л� ����� ����ϰ�, ���μ��� ģȭ��(Affinity)�� ��� �ھ�(0xFF)�� 
    // �����Ǿ����� ���� �л� ����
    if( ( gs_vstScheduler[ bCurrentAPICID ].bUseLoadBalancing == TRUE ) &&
        ( pstTask->bAffinity == TASK_LOADBALANCINGID ) )
    {
        // �½�ũ�� �߰��� �����ٷ��� ����
        bTargetAPICID = kFindSchedulerOfMinumumTaskCount( pstTask );
    }
    // �½�ũ ���� �л� ��ɰ� ���� ���� ���μ��� ģȭ�� �ʵ忡 �ٸ� �ھ��� APIC ID�� 
    // ��������� �ش� �����ٷ��� �Ű���
    else if( ( pstTask->bAffinity != bCurrentAPICID ) &&
             ( pstTask->bAffinity != TASK_LOADBALANCINGID ) )
    {
        bTargetAPICID = pstTask->bAffinity;
    }
    // ���� �л� ����� ������� �ʴ� ���� ���� �����ٷ��� �ٽ� ����
    else
    {
        bTargetAPICID = bCurrentAPICID;
    }
    
    // �Ӱ� ���� ����
    kLockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
    // �½�ũ�� �߰��� �����ٷ��� ���� �����ٷ��� �ٸ��ٸ� �½�ũ�� �̵���.
    // FPU�� �������� �����Ƿ� ���� �½�ũ�� FPU�� ���������� ��ٸ� FPU ���ؽ�Ʈ�� 
    // �޸𸮿� �����ؾ� ��
    if( ( bCurrentAPICID != bTargetAPICID ) &&
        ( pstTask->stLink.qwID == 
            gs_vstScheduler[ bCurrentAPICID ].qwLastFPUUsedTaskID ) )
    {
        // FPU�� �����ϱ� ���� TS bit�� ���� ������, ���� 7(Device Not Available)��
        // �߻��ϹǷ� �����ؾ� ��
        kClearTS();
        kSaveFPUContext( pstTask->vqwFPUContext );
        gs_vstScheduler[ bCurrentAPICID ].qwLastFPUUsedTaskID = TASK_INVALIDID;
    }
    // �Ӱ� ���� ��
    kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );

    // �Ӱ� ���� ����
    kLockForSpinLock( &( gs_vstScheduler[ bTargetAPICID ].stSpinLock ) );    
 
    // �½�ũ�� ������ �ھ��� APIC ID�� �����ϰ�, �ش� �����ٷ��� �½�ũ ����
    pstTask->bAPICID = bTargetAPICID;
    kAddTaskToReadyList( bTargetAPICID, pstTask );
    
    // �Ӱ� ���� ��
    kUnlockForSpinLock( &( gs_vstScheduler[ bTargetAPICID ].stSpinLock ) );
}

/**
 *  �½�ũ�� �߰��� �����ٷ��� ID�� ��ȯ
 *      �Ķ���ͷ� ���޵� �½�ũ �ڷᱸ������ ��� �÷��׿� ���μ��� ģȭ��(Affinity) �ʵ尡
 *      ä�����־�� ��
 */
static BYTE kFindSchedulerOfMinumumTaskCount( const TCB* pstTask )
{
    BYTE bPriority;
    BYTE i;
    int iCurrentTaskCount;
    int iMinTaskCount;
    BYTE bMinCoreIndex;
    int iTempTaskCount;
    int iProcessorCount;
    
    // �ھ��� ������ Ȯ��
    iProcessorCount = kGetProcessorCount();
    
    // �ھ �ϳ���� ���� �ھ�� ��� ����
    if( iProcessorCount == 1 )
    {
        return pstTask->bAPICID;
    }
    
    // �켱 ���� ����
    bPriority = GETPRIORITY( pstTask->qwFlags );

    // �½�ũ�� ���Ե� �����ٷ����� �½�ũ�� ���� �켱 ������ �½�ũ ���� Ȯ��
    iCurrentTaskCount = kGetListCount( &( gs_vstScheduler[ pstTask->bAPICID ].
            vstReadyList[ bPriority ] ) );
    
    // ������ �ھ�� ���� ���� �½�ũ�� ���� ������ �˻�
    // �ڽŰ� �½�ũ�� ���� ��� 2 �̻� ���� ���� �� �߿��� ���� �½�ũ ���� ����
    // �����ٷ��� ID�� ��ȯ
    iMinTaskCount = TASK_MAXCOUNT;
    bMinCoreIndex = pstTask->bAPICID;
    for( i = 0 ; i < iProcessorCount ; i++ )
    {
        if( i == pstTask->bAPICID )
        {
            continue;
        }
        
        // ��� �����ٷ��� ���鼭 Ȯ��
        iTempTaskCount = kGetListCount( &( gs_vstScheduler[ i ].vstReadyList[ 
            bPriority ] ) );
        
        // ���� �ھ�� �½�ũ ���� 2�� �̻� ���̰� ���� �������� �½�ũ ���� ���� �۾Ҵ�
        // �ھ�� �� �۴ٸ� ������ ������
        if( ( iTempTaskCount + 2 <= iCurrentTaskCount ) &&
            ( iTempTaskCount < iMinTaskCount ) )
        {
            bMinCoreIndex = i;
            iMinTaskCount = iTempTaskCount;
        }
    }
    
    return bMinCoreIndex;
}

/**
 *  �Ķ���ͷ� ���޵� �ھ �½�ũ ���� �л� ��� ��� ���θ� ����
 */
BYTE kSetTaskLoadBalancing( BYTE bAPICID, BOOL bUseLoadBalancing )
{
    gs_vstScheduler[ bAPICID ].bUseLoadBalancing = bUseLoadBalancing;
}

/**
 *  ���μ��� ģȭ���� ����
 */
BOOL kChangeProcessorAffinity( QWORD qwTaskID, BYTE bAffinity )
{
    TCB* pstTarget;
    BYTE bAPICID;
    
    // �½�ũ�� ���Ե� �ھ��� ���� APIC ID�� ã�� ��, ���ɶ��� ���
    if( kFindSchedulerOfTaskAndLock( qwTaskID, &bAPICID ) == FALSE )
    {
        return FALSE;
    }
    
    // ���� �������� �½�ũ�̸� ���μ��� ģȭ���� ����. ���� �½�ũ�� �Ű����� ������
    // �½�ũ ��ȯ�� ����� ����
    pstTarget = gs_vstScheduler[ bAPICID ].pstRunningTask;
    if( pstTarget->stLink.qwID == qwTaskID )
    {
        // ���μ��� ģȭ�� ����
        pstTarget->bAffinity = bAffinity;

        // �Ӱ� ���� ��
        kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );
    }
    // �������� �½�ũ�� �ƴϸ� �غ� ����Ʈ���� ã�Ƽ� ��� �̵�
    else
    {
        // �غ� ����Ʈ���� �½�ũ�� ã�� ���ϸ� ���� �½�ũ�� ã�Ƽ� ģȭ���� ����
        pstTarget = kRemoveTaskFromReadyList( bAPICID, qwTaskID );
        if( pstTarget == NULL )
        {
            pstTarget = kGetTCBInTCBPool( GETTCBOFFSET( qwTaskID ) );
            if( pstTarget != NULL )
            {
                // ���μ��� ģȭ�� ����
                pstTarget->bAffinity = bAffinity;
            }
        }
        else
        {
            // ���μ��� ģȭ�� ����
            pstTarget->bAffinity = bAffinity;
        }

        // �Ӱ� ���� ��
        kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );
        
        // ���μ��� ���� �л��� ����ؼ� �����ٷ��� ���
        kAddTaskToSchedulerWithLoadBalancing( pstTarget );
    }
    
    return TRUE;
}

//==============================================================================
//  ���� �½�ũ ����
//==============================================================================
/**
 *  ���� �½�ũ
 *      ��� ť�� ���� ������� �½�ũ�� ����
 */
void kIdleTask( void )
{
    TCB* pstTask, * pstChildThread, * pstProcess;
    QWORD qwLastMeasureTickCount, qwLastSpendTickInIdleTask;
    QWORD qwCurrentMeasureTickCount, qwCurrentSpendTickInIdleTask;
    QWORD qwTaskID, qwChildThreadID;
    int i, iCount;
    void* pstThreadLink;
    BYTE bCurrentAPICID;
    BYTE bProcessAPICID;
    
    // ���� �ھ��� ���� APIC ID�� Ȯ��
    bCurrentAPICID = kGetAPICID();
    
    // ���μ��� ��뷮 ����� ���� ���� ������ ����
    qwLastSpendTickInIdleTask = 
        gs_vstScheduler[ bCurrentAPICID ].qwSpendProcessorTimeInIdleTask;
    qwLastMeasureTickCount = kGetTickCount();
    
    while( 1 )
    {
        // ���� ���¸� ����
        qwCurrentMeasureTickCount = kGetTickCount();
        qwCurrentSpendTickInIdleTask = 
            gs_vstScheduler[ bCurrentAPICID ].qwSpendProcessorTimeInIdleTask;
        
        // ���μ��� ��뷮�� ���
        // 100 - ( ���� �½�ũ�� ����� ���μ��� �ð� ) * 100 / ( �ý��� ��ü���� 
        // ����� ���μ��� �ð� )
        if( qwCurrentMeasureTickCount - qwLastMeasureTickCount == 0 )
        {
            gs_vstScheduler[ bCurrentAPICID ].qwProcessorLoad = 0;
        }
        else
        {
            gs_vstScheduler[ bCurrentAPICID ].qwProcessorLoad = 100 - 
                ( qwCurrentSpendTickInIdleTask - qwLastSpendTickInIdleTask ) * 
                100 /( qwCurrentMeasureTickCount - qwLastMeasureTickCount );
        }
        
        // ���� ���¸� ���� ���¿� ����
        qwLastMeasureTickCount = qwCurrentMeasureTickCount;
        qwLastSpendTickInIdleTask = qwCurrentSpendTickInIdleTask;

        // ���μ����� ���Ͽ� ���� ���� ��
        kHaltProcessorByLoad( bCurrentAPICID );
        
        // ��� ť�� ������� �½�ũ�� ������ �½�ũ�� ������
        if( kGetListCount( &( gs_vstScheduler[ bCurrentAPICID ].stWaitList ) ) 
                > 0 )
        {
            while( 1 )
            {
                // �Ӱ� ���� ����
                kLockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
                pstTask = kRemoveListFromHeader( 
                    &( gs_vstScheduler[ bCurrentAPICID ].stWaitList ) );
                // �Ӱ� ���� ��
                kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
                
                if( pstTask == NULL )
                {
                    break;
                }
                
                if( pstTask->qwFlags & TASK_FLAGS_PROCESS )
                {
                    // ���μ����� ������ �� �ڽ� �����尡 �����ϸ� �����带 ��� 
                    // �����ϰ�, �ٽ� �ڽ� ������ ����Ʈ�� ����
                    iCount = kGetListCount( &( pstTask->stChildThreadList ) );
                    for( i = 0 ; i < iCount ; i++ )
                    {
                        // �Ӱ� ���� ����
                        kLockForSpinLock( 
                            &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
                        // ������ ��ũ�� ��巹������ ���� �����带 �����Ŵ
                        pstThreadLink = ( TCB* ) kRemoveListFromHeader( 
                                &( pstTask->stChildThreadList ) );
                        if( pstThreadLink == NULL )
                        {
                            // �Ӱ� ���� ��
                            kUnlockForSpinLock( 
                                &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
                            break;
                        }
                        
                        // �ڽ� ������ ����Ʈ�� ����� ������ �½�ũ �ڷᱸ���� �ִ� 
                        // stThreadLink�� ���� ��巹���̹Ƿ�, �½�ũ �ڷᱸ���� ����
                        // ��巹���� ���Ϸ��� ������ ����� �ʿ���
                        pstChildThread = GETTCBFROMTHREADLINK( pstThreadLink );                        

                        // �ٽ� �ڽ� ������ ����Ʈ�� �����Ͽ� �ش� �����尡 ����� ��
                        // �ڽ� �����尡 ���μ����� ã�� ������ ����Ʈ���� �����ϵ��� ��
                        kAddListToTail( &( pstTask->stChildThreadList ),
                                &( pstChildThread->stThreadLink ) );
                        qwChildThreadID = pstChildThread->stLink.qwID;
                        // �Ӱ� ���� ��
                        kUnlockForSpinLock( 
                            &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
                        
                        // �ڽ� �����带 ã�Ƽ� ����
                        kEndTask( qwChildThreadID );
                    }
                    
                    // ���� �ڽ� �����尡 �����ִٸ� �ڽ� �����尡 �� ����� ������
                    // ��ٷ��� �ϹǷ� �ٽ� ��� ����Ʈ�� ����
                    if( kGetListCount( &( pstTask->stChildThreadList ) ) > 0 )
                    {
                        // �Ӱ� ���� ����
                        kLockForSpinLock( 
                            &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
                        kAddListToTail( 
                             &( gs_vstScheduler[ bCurrentAPICID ].stWaitList ), 
                             pstTask );
                        // �Ӱ� ���� ��
                        kUnlockForSpinLock( 
                            &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
                        continue;
                    }
                    // ���μ����� �����ؾ� �ϹǷ� �Ҵ� ���� �޸� ������ ����
                    else
                    {
                        // TODO: ���Ŀ� �ڵ� ����
                    }
                }                
                else if( pstTask->qwFlags & TASK_FLAGS_THREAD )
                {
                    // �������� ���μ����� �ڽ� ������ ����Ʈ���� ����
                    pstProcess = kGetProcessByThread( pstTask );
                    if( pstProcess != NULL )
                    {
                        // ���μ��� ID�� ���μ����� ���� �����ٷ��� ID�� ã�� ���ɶ� ���
                        if( kFindSchedulerOfTaskAndLock( pstProcess->stLink.qwID, 
                                &bProcessAPICID ) == TRUE )
                        {
                            kRemoveList( &( pstProcess->stChildThreadList ), 
                                         pstTask->stLink.qwID );
                            kUnlockForSpinLock( &( gs_vstScheduler[ 
                                bProcessAPICID ].stSpinLock ) );
                        }
                    }
                }
                
                // ������� �Դٸ� �½�ũ�� ���������� ����� ���̹Ƿ�, 
                // �½�ũ �ڷᱸ��(TCB)�� ������ ��ȯ
                qwTaskID = pstTask->stLink.qwID;

                // ������ ��ȯ
                kFreeMemory( pstTask->pvStackAddress );
                 
                // �½�ũ �ڷᱸ��(TCB)�� ��ȯ
                kFreeTCB( qwTaskID );                
                kPrintf( "IDLE: Task ID[0x%q] is completely ended.\n", 
                        qwTaskID );
            }
        }
        
        kSchedule();
    }
}

/**
 *  ������ ���μ��� ���Ͽ� ���� ���μ����� ���� ��
 */
void kHaltProcessorByLoad( BYTE bAPICID )
{
    if( gs_vstScheduler[ bAPICID ].qwProcessorLoad < 40 )
    {
        kHlt();
        kHlt();
        kHlt();
    }
    else if( gs_vstScheduler[ bAPICID ].qwProcessorLoad < 80 )
    {
        kHlt();
        kHlt();
    }
    else if( gs_vstScheduler[ bAPICID ].qwProcessorLoad < 95 )
    {
        kHlt();
    }
}


//==============================================================================
//  FPU ����
//==============================================================================
/**
 *  ���������� FPU�� ����� �½�ũ ID�� ��ȯ
 */
QWORD kGetLastFPUUsedTaskID( BYTE bAPICID )
{
    return gs_vstScheduler[ bAPICID ].qwLastFPUUsedTaskID;
}

/**
 *  ���������� FPU�� ����� �½�ũ ID�� ����
 */
void kSetLastFPUUsedTaskID( BYTE bAPICID, QWORD qwTaskID )
{
    gs_vstScheduler[ bAPICID ].qwLastFPUUsedTaskID = qwTaskID;
}


