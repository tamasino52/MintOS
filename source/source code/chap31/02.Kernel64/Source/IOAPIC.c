/**
 *  file    IOAPIC.c
 *  date    2009/07/19
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   I/O APIC�� ���õ� �ҽ� ����
 */

#include "IOAPIC.h"
#include "MPConfigurationTable.h"
#include "PIC.h"

// I/O APIC�� �����ϴ� �ڷᱸ��
static IOAPICMANAGER gs_stIOAPICManager;

/**
 *  ISA ������ ����� I/O APIC�� ���� ��巹���� ��ȯ
 */
QWORD kGetIOAPICBaseAddressOfISA( void )
{
    MPCONFIGRUATIONMANAGER* pstMPManager;
    IOAPICENTRY* pstIOAPICEntry;
    
    // I/O APIC�� ��巹���� ����Ǿ� ���� ������ ��Ʈ���� ã�Ƽ� ����
    if( gs_stIOAPICManager.qwIOAPICBaseAddressOfISA == NULL )
    {
        pstIOAPICEntry = kFindIOAPICEntryForISA();
        if( pstIOAPICEntry != NULL )
        {
            gs_stIOAPICManager.qwIOAPICBaseAddressOfISA = 
                pstIOAPICEntry->dwMemoryMapAddress & 0xFFFFFFFF;
        }
    }

    // I/O APIC�� ���� ��巹���� ã�Ƽ� ������ ���� ��ȯ
    return gs_stIOAPICManager.qwIOAPICBaseAddressOfISA;
}

/**
 *  I/O �����̷��� ���̺� �ڷᱸ���� ���� ����
 */
void kSetIOAPICRedirectionEntry( IOREDIRECTIONTABLE* pstEntry, BYTE bAPICID,
        BYTE bInterruptMask, BYTE bFlagsAndDeliveryMode, BYTE bVector )
{
    kMemSet( pstEntry, 0, sizeof( IOREDIRECTIONTABLE ) );
    
    pstEntry->bDestination = bAPICID;
    pstEntry->bFlagsAndDeliveryMode = bFlagsAndDeliveryMode;
    pstEntry->bInterruptMask = bInterruptMask;
    pstEntry->bVector = bVector;
}

/**
 *  ���ͷ�Ʈ �Է� ��(INTIN)�� �ش��ϴ� I/O �����̷��� ���̺��� ���� ����
 */
void kReadIOAPICRedirectionTable( int iINTIN, IOREDIRECTIONTABLE* pstEntry )
{
    QWORD* pqwData;
    QWORD qwIOAPICBaseAddress;
    
    // ISA ������ ����� I/O APIC�� �޸� �� I/O ��巹��
    qwIOAPICBaseAddress = kGetIOAPICBaseAddressOfISA();
    
    // I/O �����̷��� ���̺��� 8����Ʈ�̹Ƿ�, 8����Ʈ ������ ��ȯ�ؼ� ó��
    pqwData = ( QWORD* ) pstEntry;
    
    //--------------------------------------------------------------------------
    // I/O �����̷��� ���̺��� ���� 4����Ʈ�� ����
    // I/O �����̷��� ���̺��� ���� �������Ϳ� ���� �������Ͱ� �� ���̹Ƿ�, INTIN��
    // 2�� ���Ͽ� �ش� I/O �����̷��� ���̺� ���������� �ε����� ���
    //--------------------------------------------------------------------------
    // I/O �������� ���� ��������(0xFEC00000)�� ���� I/O �����̷��� ���̺� ����������
    // �ε����� ����
    *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOREGISTERSELECTOR ) = 
        IOAPIC_REGISTERINDEX_HIGHIOREDIRECTIONTABLE + iINTIN * 2;
    // I/O ������ ��������(0xFEC00010)���� ���� I/O �����̷��� ���̺� ����������
    // ���� ����
    *pqwData = *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOWINDOW );
    *pqwData = *pqwData << 32;
    
    //--------------------------------------------------------------------------
    // I/O �����̷��� ���̺��� ���� 4����Ʈ�� ����
    // I/O �����̷��� ���̺��� ���� �������Ϳ� ���� �������Ͱ� �� ���̹Ƿ�, INTIN��
    // 2�� ���Ͽ� �ش� I/O �����̷��� ���̺� ���������� �ε����� ���
    //--------------------------------------------------------------------------
    // I/O �������� ���� ��������(0xFEC00000)�� ���� I/O �����̷��� ���̺� ����������
    // �ε����� ����
    *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOREGISTERSELECTOR ) =
        IOAPIC_REGISTERINDEX_LOWIOREDIRECTIONTABLE + iINTIN * 2 ;
    // I/O ������ ��������(0xFEC00010)���� ���� I/O �����̷��� ���̺� ����������
    // ���� ����
    *pqwData |= *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOWINDOW );
}

/**
 *  ���ͷ�Ʈ �Է� ��(INTIN)�� �ش��ϴ� I/O �����̷��� ���̺� ���� ��
 */
void kWriteIOAPICRedirectionTable( int iINTIN, IOREDIRECTIONTABLE* pstEntry )
{
    QWORD* pqwData;
    QWORD qwIOAPICBaseAddress;
    
    // ISA ������ ����� I/O APIC�� �޸� �� I/O ��巹��
    qwIOAPICBaseAddress = kGetIOAPICBaseAddressOfISA();

    // I/O �����̷��� ���̺��� 8����Ʈ�̹Ƿ�, 8����Ʈ ������ ��ȯ�ؼ� ó��
    pqwData = ( QWORD* ) pstEntry;
    
    //--------------------------------------------------------------------------
    // I/O �����̷��� ���̺� ���� 4����Ʈ�� ��
    // I/O �����̷��� ���̺��� ���� �������Ϳ� ���� �������Ͱ� �� ���̹Ƿ�, INTIN��
    // 2�� ���Ͽ� �ش� I/O �����̷��� ���̺� ���������� �ε����� ���
    //--------------------------------------------------------------------------
    // I/O �������� ���� ��������(0xFEC00000)�� ���� I/O �����̷��� ���̺� ����������
    // �ε����� ����
    *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOREGISTERSELECTOR ) = 
        IOAPIC_REGISTERINDEX_HIGHIOREDIRECTIONTABLE + iINTIN * 2;
    // I/O ������ ��������(0xFEC00010)�� ���� I/O �����̷��� ���̺� ����������
    // ���� ��
    *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOWINDOW ) = *pqwData >> 32;
    
    //--------------------------------------------------------------------------
    // I/O �����̷��� ���̺� ���� 4����Ʈ�� ��
    // I/O �����̷��� ���̺��� ���� �������Ϳ� ���� �������Ͱ� �� ���̹Ƿ�, INTIN��
    // 2�� ���Ͽ� �ش� I/O �����̷��� ���̺� ���������� �ε����� ���
    //--------------------------------------------------------------------------
    // I/O �������� ���� ��������(0xFEC00000)�� ���� I/O �����̷��� ���̺� ����������
    // �ε����� ����
    *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOREGISTERSELECTOR ) =
        IOAPIC_REGISTERINDEX_LOWIOREDIRECTIONTABLE + iINTIN * 2 ;
    // I/O ������ ��������(0xFEC00010)�� ���� I/O �����̷��� ���̺� ����������
    // ���� ��
    *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOWINDOW ) = *pqwData;
}

/**
 *  I/O APIC�� ����� ��� ���ͷ�Ʈ ���� ����ũ�Ͽ� ���ͷ�Ʈ�� ���޵��� �ʵ��� ��
 */
void kMaskAllInterruptInIOAPIC( void )
{
    IOREDIRECTIONTABLE stEntry;
    int i;
    
    // ��� ���ͷ�Ʈ�� ��Ȱ��ȭ
    for( i = 0 ; i < IOAPIC_MAXIOREDIRECTIONTABLECOUNT ; i++ )
    {
        // I/O �����̷��� ���̺��� �о ���ͷ�Ʈ ����ũ �ʵ�(��Ʈ 0)�� 1�� 
        // �����Ͽ� �ٽ� ����
        kReadIOAPICRedirectionTable( i, &stEntry );
        stEntry.bInterruptMask = IOAPIC_INTERRUPT_MASK;
        kWriteIOAPICRedirectionTable( i, &stEntry );
    }
}

/**
 *  I/O APIC�� I/O �����̷��� ���̺��� �ʱ�ȭ
 */
void kInitializeIORedirectionTable( void )
{
    MPCONFIGRUATIONMANAGER* pstMPManager;
    MPCONFIGURATIONTABLEHEADER* pstMPHeader;
    IOINTERRUPTASSIGNMENTENTRY* pstIOAssignmentEntry;
    IOREDIRECTIONTABLE stIORedirectionEntry;
    QWORD qwEntryAddress;
    BYTE bEntryType;
    BYTE bDestination;
    int i;

    //==========================================================================
    // I/O APIC�� �����ϴ� �ڷᱸ���� �ʱ�ȭ
    //==========================================================================
    kMemSet( &gs_stIOAPICManager, 0, sizeof( gs_stIOAPICManager ) );
    
    // I/O APIC�� �޸� �� I/O ��巹�� ����, �Ʒ� �Լ����� ���������� ó����
    kGetIOAPICBaseAddressOfISA();
    
    // IRQ�� I/O APIC�� INTIN �ɰ� ������ ���̺�(IRQ->INTIN ���� ���̺�)�� �ʱ�ȭ
    for( i = 0 ; i < IOAPIC_MAXIRQTOINTINMAPCOUNT ; i++ )
    {
        gs_stIOAPICManager.vbIRQToINTINMap[ i ] = 0xFF;
    }
    
    //==========================================================================
    // I/O APIC�� ����ũ�Ͽ� ���ͷ�Ʈ�� �߻����� �ʵ��� �ϰ� I/O �����̷��� ���̺� �ʱ�ȭ
    //==========================================================================
    // ���� I/O APIC�� ���ͷ�Ʈ�� ����ũ�Ͽ� ���ͷ�Ʈ�� �߻����� �ʵ��� ��
    kMaskAllInterruptInIOAPIC();
    
    // IO ���ͷ�Ʈ ���� ��Ʈ�� �߿��� ISA ������ ���õ� ���ͷ�Ʈ�� �߷��� I/O �����̷���
    // ���̺� ����
    // MP ���� ���̺� ����� ���� ��巹���� ��Ʈ���� ���� ��巹���� ����
    pstMPManager = kGetMPConfigurationManager();
    pstMPHeader = pstMPManager->pstMPConfigurationTableHeader;
    qwEntryAddress = pstMPManager->qwBaseEntryStartAddress;
    
    // ��� ��Ʈ���� Ȯ���Ͽ� ISA ������ ���õ� I/O ���ͷ�Ʈ ���� ��Ʈ���� �˻�
    for( i = 0 ; i < pstMPHeader->wEntryCount ; i++ )
    {
        bEntryType = *( BYTE* ) qwEntryAddress;
        switch( bEntryType )
        {
            // IO ���ͷ�Ʈ ���� ��Ʈ���̸�, ISA �������� Ȯ���Ͽ� I/O �����̷���
            // ���̺� �����ϰ� IRQ->INITIN ���� ���̺��� ����
        case MP_ENTRYTYPE_IOINTERRUPTASSIGNMENT:
            pstIOAssignmentEntry = ( IOINTERRUPTASSIGNMENTENTRY* ) qwEntryAddress;

            // ���ͷ�Ʈ Ÿ���� ���ͷ�Ʈ(INT)�� �͸� ó��
            if( ( pstIOAssignmentEntry->bSourceBUSID == pstMPManager->bISABusID ) &&
                ( pstIOAssignmentEntry->bInterruptType == MP_INTERRUPTTYPE_INT ) )                        
            {
                // ������ �ʵ�� IRQ 0�� �����ϰ� 0x00���� �����Ͽ� Bootstrap Processor�� ����
                // IRQ 0�� �����ٷ��� ����ؾ� �ϹǷ� 0xFF�� �����Ͽ� ��� �ھ�� ����
                if( pstIOAssignmentEntry->bSourceBUSIRQ == 0 )
                {
                    bDestination = 0xFF;
                }
                else
                {
                    bDestination = 0x00;
                }
                
                // ISA ������ ���� Ʈ����(Edge Trigger)�� 1�� �� Ȱ��ȭ(Active High)��
                // ���
                // ������ ���� ���� ���, ���� ���� ����(Fixed)���� �Ҵ�
                // ���ͷ�Ʈ ���ʹ� PIC ��Ʈ�ѷ��� ���Ϳ� ���� 0x20 + IRQ�� ����
                kSetIOAPICRedirectionEntry( &stIORedirectionEntry, bDestination, 
                    0x00, IOAPIC_TRIGGERMODE_EDGE | IOAPIC_POLARITY_ACTIVEHIGH |
                    IOAPIC_DESTINATIONMODE_PHYSICALMODE | IOAPIC_DELIVERYMODE_FIXED, 
                    PIC_IRQSTARTVECTOR + pstIOAssignmentEntry->bSourceBUSIRQ );
                
                // ISA �������� ���޵� IRQ�� I/O APIC�� INTIN �ɿ� �����Ƿ�, INTIN ����
                // �̿��Ͽ� ó��
                kWriteIOAPICRedirectionTable( pstIOAssignmentEntry->bDestinationIOAPICINTIN, 
                        &stIORedirectionEntry );
                
                // IRQ�� ���ͷ�Ʈ �Է� ��(INTIN)�� ���踦 ����(IRQ->INTIN ���� ���̺� ����)
                gs_stIOAPICManager.vbIRQToINTINMap[ pstIOAssignmentEntry->bSourceBUSIRQ ] =
                    pstIOAssignmentEntry->bDestinationIOAPICINTIN;                
            }                    
            qwEntryAddress += sizeof( IOINTERRUPTASSIGNMENTENTRY );
            break;
        
            // ���μ��� ��Ʈ���� ����
        case MP_ENTRYTYPE_PROCESSOR:
            qwEntryAddress += sizeof( PROCESSORENTRY );
            break;
            
            // ���� ��Ʈ��, I/O APIC ��Ʈ��, ���� ���ͷ�Ʈ ���� ��Ʈ���� ����
        case MP_ENTRYTYPE_BUS:
        case MP_ENTRYTYPE_IOAPIC:
        case MP_ENTRYTYPE_LOCALINTERRUPTASSIGNMENT:
            qwEntryAddress += 8;
            break;
        }
    }  
}

/**
 *  IRQ�� I/O APIC�� ���ͷ�Ʈ ��(INT IN)���� ���� ���踦 ���
 */
void kPrintIRQToINTINMap( void )
{
    int i;
    
    kPrintf( "=========== IRQ To I/O APIC INT IN Mapping Table ===========\n" );
    
    for( i = 0 ; i < IOAPIC_MAXIRQTOINTINMAPCOUNT ; i++ )
    {
        kPrintf( "IRQ[%d] -> INTIN [%d]\n", i, gs_stIOAPICManager.vbIRQToINTINMap[ i ] );
    }
}
