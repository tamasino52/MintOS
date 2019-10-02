/**
 *  file    MPConfigurationTable.h
 *  date    2009/06/20
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   MP ���� ���̺�(MP Configuration Table)�� ���õ� ��� ����
 */

#ifndef __MPCONFIGURATIONTABLE__
#define __MPCONFIGURATIONTABLE__

#include "Types.h"

////////////////////////////////////////////////////////////////////////////////
//
// ��ũ��
//
////////////////////////////////////////////////////////////////////////////////
// MP �÷��� �������� Ư�� ����Ʈ(Feature Byte)
#define MP_FLOATINGPOINTER_FEATUREBYTE1_USEMPTABLE  0x00
#define MP_FLOATINGPOINTER_FEATUREBYTE2_PICMODE     0x80

// ��Ʈ�� Ÿ��(Entry Type)
#define MP_ENTRYTYPE_PROCESSOR                  0
#define MP_ENTRYTYPE_BUS                        1
#define MP_ENTRYTYPE_IOAPIC                     2
#define MP_ENTRYTYPE_IOINTERRUPTASSIGNMENT      3
#define MP_ENTRYTYPE_LOCALINTERRUPTASSIGNMENT   4

// ���μ��� CPU �÷���
#define MP_PROCESSOR_CPUFLAGS_ENABLE            0x01
#define MP_PROCESSOR_CPUFLAGS_BSP               0x02

// ���� Ÿ�� ��Ʈ��(Bus Type String)
#define MP_BUS_TYPESTRING_ISA                   "ISA"
#define MP_BUS_TYPESTRING_PCI                   "PCI"
#define MP_BUS_TYPESTRING_PCMCIA                "PCMCIA"
#define MP_BUS_TYPESTRING_VESALOCALBUS          "VL"

// ���ͷ�Ʈ Ÿ��(Interrupt Type)
#define MP_INTERRUPTTYPE_INT                    0
#define MP_INTERRUPTTYPE_NMI                    1
#define MP_INTERRUPTTYPE_SMI                    2
#define MP_INTERRUPTTYPE_EXTINT                 3

// ���ͷ�Ʈ �÷���(Interrupt Flags)
#define MP_INTERRUPT_FLAGS_CONFORMPOLARITY      0x00
#define MP_INTERRUPT_FLAGS_ACTIVEHIGH           0x01
#define MP_INTERRUPT_FLAGS_ACTIVELOW            0x03
#define MP_INTERRUPT_FLAGS_CONFORMTRIGGER       0x00
#define MP_INTERRUPT_FLAGS_EDGETRIGGERED        0x04
#define MP_INTERRUPT_FLAGS_LEVELTRIGGERED       0x0C

////////////////////////////////////////////////////////////////////////////////
//
// ����ü
//
////////////////////////////////////////////////////////////////////////////////
// 1����Ʈ�� ����
#pragma pack( push, 1 )

// MP �÷��� ������ �ڷᱸ��(MP Floating Pointer Data Structure)
typedef struct kMPFloatingPointerStruct
{
    // �ñ׳�ó, _MP_
    char vcSignature[ 4 ]; 
    // MP ���� ���̺��� ��ġ�ϴ� ��巹��
    DWORD dwMPConfigurationTableAddress;
    // MP �÷��� ������ �ڷᱸ���� ����, 16 ����Ʈ
    BYTE bLength;
    // MultiProcessor Specification�� ����
    BYTE bRevision;
    // üũ��
    BYTE bCheckSum;
    // MP Ư�� ����Ʈ 1~5
    BYTE vbMPFeatureByte[ 5 ];
} MPFLOATINGPOINTER;

// MP ���� ���̺� ���(MP Configuration Table Header) �ڷᱸ��
typedef struct kMPConfigurationTableHeaderStruct
{
    // �ñ׳�ó, PCMP
    char vcSignature[ 4 ];
    // �⺻ ���̺� ����
    WORD wBaseTableLength;
    // MultiProcessor Specification�� ����
    BYTE bRevision;
    // üũ��
    BYTE bCheckSum;
    // OEM ID ���ڿ�
    char vcOEMIDString[ 8 ];
    // PRODUCT ID ���ڿ�
    char vcProductIDString[ 12 ];
    // OEM�� ������ ���̺��� ��巹��
    DWORD dwOEMTablePointerAddress;
    // OEM�� ������ ���̺��� ũ��
    WORD wOEMTableSize;
    // �⺻ MP ���� ���̺� ��Ʈ���� ����
    WORD wEntryCount;
    // ���� APIC�� �޸� �� I/O ��巹��
    DWORD dwMemoryMapIOAddressOfLocalAPIC;
    // Ȯ�� ���̺��� ����
    WORD wExtendedTableLength;
    // Ȯ�� ���̺��� üũ��
    BYTE bExtendedTableChecksum;
    // �����
    BYTE bReserved;
} MPCONFIGURATIONTABLEHEADER;

// ���μ��� ��Ʈ�� �ڷᱸ��(Processor Entry)
typedef struct kProcessorEntryStruct
{
    // ��Ʈ�� Ÿ�� �ڵ�, 0
    BYTE bEntryType;
    // ���μ����� ���Ե� ���� APIC�� ID
    BYTE bLocalAPICID;
    // ���� APIC�� ����
    BYTE bLocalAPICVersion;
    // CPU �÷���
    BYTE bCPUFlags;
    // CPU �ñ׳�ó
    BYTE vbCPUSignature[ 4 ];
    // Ư�� �÷���
    DWORD dwFeatureFlags;
    // �����
    DWORD vdwReserved[ 2 ];
} PROCESSORENTRY;

// ���� ��Ʈ�� �ڷᱸ��(Bus Entry)
typedef struct kBusEntryStruct
{
    // ��Ʈ�� Ÿ�� �ڵ�, 1
    BYTE bEntryType;
    // ���� ID
    BYTE bBusID;
    // ���� Ÿ�� ���ڿ�
    char vcBusTypeString[ 6 ];
} BUSENTRY;

// I/O APIC ��Ʈ�� �ڷᱸ��(I/O APIC Entry)
typedef struct kIOAPICEntryStruct
{
    // ��Ʈ�� Ÿ�� �ڵ�, 2
    BYTE bEntryType;
    // I/O APIC ID
    BYTE bIOAPICID;
    // I/O APIC ����
    BYTE bIOAPICVersion;
    // I/O APIC �÷���
    BYTE bIOAPICFlags;
    // �޸� �� I/O ��巹��
    DWORD dwMemoryMapAddress;
} IOAPICENTRY;

// I/O ���ͷ�Ʈ ���� ��Ʈ�� �ڷᱸ��(I/O Interrupt Assignment Entry)
typedef struct kIOInterruptAssignmentEntryStruct
{
    // ��Ʈ�� Ÿ�� �ڵ�, 3
    BYTE bEntryType;
    // ���ͷ�Ʈ Ÿ��
    BYTE bInterruptType;
    // I/O ���ͷ�Ʈ �÷���
    WORD wInterruptFlags;
    // �߻��� ���� ID
    BYTE bSourceBUSID;
    // �߻��� ���� IRQ
    BYTE bSourceBUSIRQ;
    // ������ I/O APIC ID
    BYTE bDestinationIOAPICID;
    // ������ I/O APIC INTIN
    BYTE bDestinationIOAPICINTIN;
} IOINTERRUPTASSIGNMENTENTRY;

// ���� ���ͷ�Ʈ ���� ��Ʈ�� �ڷᱸ��(Local Interrupt Assignment Entry)
typedef struct kLocalInterruptEntryStruct
{
    // ��Ʈ�� Ÿ�� �ڵ�, 4
    BYTE bEntryType;
    // ���ͷ�Ʈ Ÿ��
    BYTE bInterruptType;
    // ���� ���ͷ�Ʈ �÷���
    WORD wInterruptFlags;
    // �߻��� ���� ID
    BYTE bSourceBUSID;
    // �߻��� ���� IRQ
    BYTE bSourceBUSIRQ;
    // ������ ���� APIC ID
    BYTE bDestinationLocalAPICID;
    // ������ ���� APIC INTIN
    BYTE bDestinationLocalAPICLINTIN;
} LOCALINTERRUPTASSIGNMENTENTRY;

#pragma pack( pop)

// MP ���� ���̺��� �����ϴ� �ڷᱸ��
typedef struct kMPConfigurationManagerStruct
{
    // MP �÷��� ���̺�
    MPFLOATINGPOINTER* pstMPFloatingPointer;
    // MP ���� ���̺� ���
    MPCONFIGURATIONTABLEHEADER* pstMPConfigurationTableHeader;
    // �⺻ MP ���� ���̺� ��Ʈ���� ���� ��巹��
    QWORD qwBaseEntryStartAddress;
    // ���μ��� �Ǵ� �ھ��� ��
    int iProcessorCount;
    // PIC ��� ���� ����
    BOOL bUsePICMode;
    // ISA ������ ID
    BYTE bISABusID;
} MPCONFIGRUATIONMANAGER;


////////////////////////////////////////////////////////////////////////////////
//
// �Լ�
//
////////////////////////////////////////////////////////////////////////////////
BOOL kFindMPFloatingPointerAddress( QWORD* pstAddress );
BOOL kAnalysisMPConfigurationTable( void );
MPCONFIGRUATIONMANAGER* kGetMPConfigurationManager( void );
void kPrintMPConfigurationTable( void );
int kGetProcessorCount( void );
IOAPICENTRY* kFindIOAPICEntryForISA( void );

#endif /*__MPCONFIGURATIONTABLE__*/
