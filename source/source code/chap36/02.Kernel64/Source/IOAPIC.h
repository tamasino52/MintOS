/**
 *  file    IOAPIC.h
 *  date    2009/07/19
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   I/O APIC�� ���õ� ��� ����
 */

#ifndef __IOAPIC_H__
#define __IOAPIC_H__

#include "Types.h"

////////////////////////////////////////////////////////////////////////////////
//
//  ��ũ��
//
////////////////////////////////////////////////////////////////////////////////
// I/O APIC �������� ������ ���� ��ũ��
#define IOAPIC_REGISTER_IOREGISTERSELECTOR              0x00
#define IOAPIC_REGISTER_IOWINDOW                        0x10

// ���� �� �������ͷ� ������ �� ����ϴ� ���������� �ε���
#define IOAPIC_REGISTERINDEX_IOAPICID                   0x00
#define IOAPIC_REGISTERINDEX_IOAPICVERSION              0x01
#define IOAPIC_REGISTERINDEX_IOAPICARBID                0x02
#define IOAPIC_REGISTERINDEX_LOWIOREDIRECTIONTABLE      0x10
#define IOAPIC_REGISTERINDEX_HIGHIOREDIRECTIONTABLE     0x11

// IO �����̷��� ���̺��� �ִ� ����
#define IOAPIC_MAXIOREDIRECTIONTABLECOUNT               24

// ���ͷ�Ʈ ����ũ(Interrupt Mask) ���� ��ũ��
#define IOAPIC_INTERRUPT_MASK                           0x01

// Ʈ���� ���(Trigger Mode) ���� ��ũ��
#define IOAPIC_TRIGGERMODE_LEVEL                        0x80
#define IOAPIC_TRIGGERMODE_EDGE                         0x00

// ����Ʈ IRR(Remote IRR) ���� ��ũ��
#define IOAPIC_REMOTEIRR_EOI                            0x40
#define IOAPIC_REMOTEIRR_ACCEPT                         0x00

// ���ͷ�Ʈ �Է� �� �ؼ�(Interrupt Input Pin Polarity) ���� ��ũ��
#define IOAPIC_POLARITY_ACTIVELOW                       0x20
#define IOAPIC_POLARITY_ACTIVEHIGH                      0x00

// ���� ����(Delivery Status) ���� ��ũ��
#define IOAPIC_DELIFVERYSTATUS_SENDPENDING              0x10
#define IOAPIC_DELIFVERYSTATUS_IDLE                     0x00

// ������ ���(Destination Mode) ���� ��ũ��
#define IOAPIC_DESTINATIONMODE_LOGICALMODE              0x08
#define IOAPIC_DESTINATIONMODE_PHYSICALMODE             0x00

// ���� ���(Delivery Mode) ���� ��ũ��
#define IOAPIC_DELIVERYMODE_FIXED                       0x00
#define IOAPIC_DELIVERYMODE_LOWESTPRIORITY              0x01
#define IOAPIC_DELIVERYMODE_SMI                         0x02
#define IOAPIC_DELIVERYMODE_NMI                         0x04
#define IOAPIC_DELIVERYMODE_INIT                        0x05
#define IOAPIC_DELIVERYMODE_EXTINT                      0x07

// IRQ�� I/O APIC�� ���ͷ�Ʈ �Է� ��(INTIN)���� ������Ű�� ���̺��� �ִ� ũ��
#define IOAPIC_MAXIRQTOINTINMAPCOUNT                    16

////////////////////////////////////////////////////////////////////////////////
//
//  ����ü
//
////////////////////////////////////////////////////////////////////////////////
// 1����Ʈ�� ����
#pragma pack( push, 1 )

// IO �����̷��� ���̺��� �ڷᱸ��
typedef struct kIORedirectionTableStruct
{
    // ���ͷ�Ʈ ����
    BYTE bVector;  
    
    // Ʈ���� ���, ����Ʈ IRR, ���ͷ�Ʈ �Է� �� �ؼ�, ���� ����, ������ ���, 
    // ���� ��带 ����ϴ� �ʵ� 
    BYTE bFlagsAndDeliveryMode;
    
    // ���ͷ�Ʈ ����ũ
    BYTE bInterruptMask;
    
    // ����� ����
    BYTE vbReserved[ 4 ];
    
    // ���ͷ�Ʈ�� ������ APIC ID
    BYTE bDestination;
} IOREDIRECTIONTABLE;

// I/O APIC�� �����ϴ� �ڷᱸ��
typedef struct kIOAPICManagerStruct
{
    // ISA ������ ����� I/O APIC�� �޸� �� ��巹��
    QWORD qwIOAPICBaseAddressOfISA;
    
    // IRQ�� I/O APIC�� ���ͷ�Ʈ �Է� ��(INTIN)���� ���� ���踦 �����ϴ� ���̺�
    BYTE vbIRQToINTINMap[ IOAPIC_MAXIRQTOINTINMAPCOUNT ];    
} IOAPICMANAGER;

#pragma pack( pop )

////////////////////////////////////////////////////////////////////////////////
//
//  �Լ�
//
////////////////////////////////////////////////////////////////////////////////
QWORD kGetIOAPICBaseAddressOfISA( void );
void kSetIOAPICRedirectionEntry( IOREDIRECTIONTABLE* pstEntry, BYTE bAPICID,
        BYTE bInterruptMask, BYTE bFlagsAndDeliveryMode, BYTE bVector );
void kReadIOAPICRedirectionTable( int iINTIN, IOREDIRECTIONTABLE* pstEntry );
void kWriteIOAPICRedirectionTable( int iINTIN, IOREDIRECTIONTABLE* pstEntry );
void kMaskAllInterruptInIOAPIC( void );
void kInitializeIORedirectionTable( void );
void kPrintIRQToINTINMap( void );

#endif /*__IOAPIC_H__*/
