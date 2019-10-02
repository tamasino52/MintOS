/**
 *  file    GUITask.h
 *  date    2009/10/20
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   GUI �½�ũ�� ���õ� �Լ��� ������ ��� ����
 */

#ifndef __GUITASK_H__
#define __GUITASK_H__

#include "Types.h"

////////////////////////////////////////////////////////////////////////////////
//
// ��ũ��
//
////////////////////////////////////////////////////////////////////////////////
// �½�ũ�� ������ ���� �̺�Ʈ Ÿ�� ����
#define EVENT_USER_TESTMESSAGE          0x80000001

// �ý��� ����� �½�ũ ��ũ��
// ���μ��� ������ ǥ���ϴ� ������ �ʺ�
#define SYSTEMMONITOR_PROCESSOR_WIDTH       150
// ���μ��� ������ ǥ���ϴ� ������ ���� ������ ����
#define SYSTEMMONITOR_PROCESSOR_MARGIN      20
// ���μ��� ������ ǥ���ϴ� ������ ����
#define SYSTEMMONITOR_PROCESSOR_HEIGHT      150
// �ý��� ����� �������� ����
#define SYSTEMMONITOR_WINDOW_HEIGHT         310
// �޸� ������ ǥ���ϴ� ������ ����
#define SYSTEMMONITOR_MEMORY_HEIGHT         100
// ������ ����
#define SYSTEMMONITOR_BAR_COLOR             RGB( 55, 215, 47 )

////////////////////////////////////////////////////////////////////////////////
//
//  �Լ�
//
////////////////////////////////////////////////////////////////////////////////
// �⺻ GUI �½�ũ�� Hello World GUI �½�ũ
void kBaseGUITask( void );
void kHelloWorldGUITask( void );

// �ý��� ����� �½�ũ �Լ�
void kSystemMonitorTask( void );
static void kDrawProcessorInformation( QWORD qwWindowID, int iX, int iY, 
        BYTE bAPICID );
static void kDrawMemoryInformation( QWORD qwWindowID, int iY, int iWindowWidth );

#endif /*__GUITASK_H__*/
