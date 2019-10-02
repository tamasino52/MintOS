/**
 *  file    InterruptHandler.h
 *  date    2009/01/24
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   ���ͷ�Ʈ �� ���� �ڵ鷯�� ���õ� ��� ����
 */

#ifndef __INTERRUPTHANDLER_H__
#define __INTERRUPTHANDLER_H__

#include "Types.h"
#include "MultiProcessor.h"

////////////////////////////////////////////////////////////////////////////////
//
//  ��ũ��
//
////////////////////////////////////////////////////////////////////////////////
// ���ͷ�Ʈ ������ �ִ� ����, ISA ������ ���ͷ�Ʈ�� ó���ϹǷ� 16
#define INTERRUPT_MAXVECTORCOUNT            16
// ���ͷ�Ʈ ���� �л��� �����ϴ� ����, ���ͷ�Ʈ ó�� Ƚ���� 10�� ����� �Ǵ� ����
#define INTERRUPT_LOADBALANCINGDIVIDOR      10

////////////////////////////////////////////////////////////////////////////////
//
//  ����ü
//
////////////////////////////////////////////////////////////////////////////////
// ���ͷ�Ʈ�� ���õ� ������ �����ϴ� �ڷᱸ��
typedef struct kInterruptManagerStruct
{
    // �ھ� �� ���ͷ�Ʈ ó�� Ƚ��, �ִ� �ھ� ���� X �ִ� ���ͷ�Ʈ ���� ������ ���ǵ� 2���� �迭
    QWORD vvqwCoreInterruptCount[ MAXPROCESSORCOUNT ][ INTERRUPT_MAXVECTORCOUNT ];
    
    // ���� �л� ��� ��� ����
    BOOL bUseLoadBalancing;
    
    // ��Ī I/O ���(Symmetric I/O Mode) ��� ����
    BOOL bSymmetricIOMode;
} INTERRUPTMANAGER;


////////////////////////////////////////////////////////////////////////////////
//
// �Լ�
//
////////////////////////////////////////////////////////////////////////////////
void kSetSymmetricIOMode( BOOL bSymmetricIOMode );
void kSetInterruptLoadBalancing( BOOL bUseLoadBalancing );
void kIncreaseInterruptCount( int iIRQ );
void kSendEOI( int iIRQ );
INTERRUPTMANAGER* kGetInterruptManager( void );
void kProcessLoadBalancing( int iIRQ );

void kCommonExceptionHandler( int iVectorNumber, QWORD qwErrorCode );
void kCommonInterruptHandler( int iVectorNumber );
void kKeyboardHandler( int iVectorNumber );
void kTimerHandler( int iVectorNumber );
void kDeviceNotAvailableHandler( int iVectorNumber );
void kHDDHandler( int iVectorNumber );
void kMouseHandler( int iVectorNumber );

#endif /*__INTERRUPTHANDLER_H__*/
