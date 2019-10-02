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
