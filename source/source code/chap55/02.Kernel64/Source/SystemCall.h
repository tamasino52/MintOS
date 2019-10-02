/**
 *  file    SystemCall.h
 *  date    2009/12/08
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   �ý��� �ݿ� ���õ� �Լ��� ������ ��� ����
 */

#ifndef __SYSTEMCALL_H__
#define __SYSTEMCALL_H__

////////////////////////////////////////////////////////////////////////////////
//
// ��ũ��
//
////////////////////////////////////////////////////////////////////////////////
// �Ķ���ͷ� ������ �� �ִ� �ִ� ����
#define SYSTEMCALL_MAXPARAMETERCOUNT    20

////////////////////////////////////////////////////////////////////////////////
//
// ����ü
//
////////////////////////////////////////////////////////////////////////////////
// 1 ����Ʈ�� ����
#pragma pack( push, 1 )

// �ý��� ���� ȣ���� �� �����ϴ� �Ķ���͸� �����ϴ� �ڷᱸ��
typedef struct kSystemCallParameterTableStruct
{
    QWORD vqwValue[ SYSTEMCALL_MAXPARAMETERCOUNT ];
} PARAMETERTABLE;

#pragma pack( pop )

// �Ķ���� �ڷᱸ������ N ��°�� ����Ű�� ��ũ��
#define PARAM( x )   ( pstParameter->vqwValue[ ( x ) ] )

////////////////////////////////////////////////////////////////////////////////
//
// �Լ�
//
////////////////////////////////////////////////////////////////////////////////
void kSystemCallEntryPoint( QWORD qwServiceNumber, PARAMETERTABLE* pstParameter );
QWORD kProcessSystemCall( QWORD qwServiceNumber, PARAMETERTABLE* pstParameter );

void kSystemCallTestTask( void );

#endif /*__SYSTEMCALL_H__*/
