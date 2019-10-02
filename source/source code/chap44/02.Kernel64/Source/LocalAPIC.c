/**
 *  file    LocalAPIC.c
 *  date    2009/06/28
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   ���� APIC(Local APIC)�� ���õ� �ҽ� ����
 */
#include "LocalAPIC.h"
#include "MPConfigurationTable.h"

/**
 *  ���� APIC�� �޸� �� I/O ��巹���� ��ȯ
 */
QWORD kGetLocalAPICBaseAddress( void )
{
    MPCONFIGURATIONTABLEHEADER* pstMPHeader;
    
    // MP ���� ���̺� ����� ����� ���� APIC�� �޸� �� I/O ��巹���� ���
    pstMPHeader = kGetMPConfigurationManager()->pstMPConfigurationTableHeader;
    return pstMPHeader->dwMemoryMapIOAddressOfLocalAPIC;
}

/**
 *  �ǻ� ���ͷ�Ʈ ���� ��������(Spurious Interrupt Vector Register)�� �ִ� 
 *  APIC ����Ʈ���� Ȱ��/��Ȱ�� �ʵ带 1�� �����Ͽ� ���� APIC�� Ȱ��ȭ��
 */
void kEnableSoftwareLocalAPIC( void )
{
    QWORD qwLocalAPICBaseAddress;
    
    // MP ���� ���̺� ����� ����� ���� APIC�� �޸� �� I/O ��巹���� ���
    qwLocalAPICBaseAddress = kGetLocalAPICBaseAddress();
    
    // �ǻ� ���ͷ�Ʈ ���� ��������(Spurious Interrupt Vector Register, 0xFEE000F0)�� 
    // APIC ����Ʈ���� Ȱ��/��Ȱ�� �ʵ�(��Ʈ 8)�� 1�� �����ؼ� ���� APIC�� Ȱ��ȭ
    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_SVR ) |= 0x100;
}

/**
 *  ���� APIC�� EOI(End of Interrupt)�� ����
 */
void kSendEOIToLocalAPIC( void )
{
    QWORD qwLocalAPICBaseAddress;
    
    // MP ���� ���̺� ����� ����� ���� APIC�� �޸� �� I/O ��巹���� ���
    qwLocalAPICBaseAddress = kGetLocalAPICBaseAddress();
    
    // EOI ��������(0xFEE000B0)�� 0x00�� ����Ͽ� EOI�� ����
    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_EOI ) = 0;
}

/*
 *  �½�ũ �켱 ���� ��������(Task Priority Register) ����
 */
void kSetTaskPriority( BYTE bPriority )
{
    QWORD qwLocalAPICBaseAddress;
    
    // MP ���� ���̺� ����� ����� ���� APIC�� �޸� �� I/O ��巹���� ���
    qwLocalAPICBaseAddress = kGetLocalAPICBaseAddress();
    
    // �½�ũ �켱 ���� ��������(0xFEE00080)�� �켱 ���� ���� ����
    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_TASKPRIORITY ) = bPriority;
}

/**
 *  ���� ���� ���̺� �ʱ�ȭ
 */
void kInitializeLocalVectorTable( void )
{
    QWORD qwLocalAPICBaseAddress;
    DWORD dwTempValue;
    
    // MP ���� ���̺� ����� ����� ���� APIC�� �޸� �� I/O ��巹���� ���
    qwLocalAPICBaseAddress = kGetLocalAPICBaseAddress();

    // Ÿ�̸� ���ͷ�Ʈ�� �߻����� �ʵ��� ���� ���� ����ũ ���� ���ؼ� 
    // LVT Ÿ�̸� ��������(0xFEE00320)�� ����
    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_TIMER ) |= APIC_INTERRUPT_MASK;
    
    // LINT0 ���ͷ�Ʈ�� �߻����� �ʵ��� ���� ���� ����ũ ���� ���ؼ�
    // LVT LINT0 ��������(0xFEE00350)�� ����
    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_LINT0 ) |= APIC_INTERRUPT_MASK;

    // LINT1 ���ͷ�Ʈ�� NMI�� �߻��ϵ��� NMI�� �����Ͽ� LVT LINT1 
    // ��������(0xFEE00360)�� ����
    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_LINT1 ) = APIC_TRIGGERMODE_EDGE | 
        APIC_POLARITY_ACTIVEHIGH | APIC_DELIVERYMODE_NMI;

    // ���� ���ͷ�Ʈ�� �߻����� �ʵ��� ���� ���� ����ũ ���� ���ؼ�
    // LVT ���� ��������(0xFEE00370)�� ����
    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_ERROR ) |= APIC_INTERRUPT_MASK;

    // ���� ����͸� ī���� ���ͷ�Ʈ�� �߻����� �ʵ��� ���� ���� ����ũ ���� ���ؼ�
    // LVT ���� ����͸� ī���� ��������(0xFEE00340)�� ����
    *( DWORD* ) ( qwLocalAPICBaseAddress + 
            APIC_REGISTER_PERFORMANCEMONITORINGCOUNTER ) |= APIC_INTERRUPT_MASK;

    // �µ� ���� ���ͷ�Ʈ�� �߻����� �ʵ��� ���� ���� ����ũ ���� ���ؼ�
    // LVT �µ� ���� ��������(0xFEE00330)�� ����
    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_THERMALSENSOR ) |= 
        APIC_INTERRUPT_MASK;
}
