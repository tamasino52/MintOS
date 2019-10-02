/**
 *  file    MultiProcessor.h
 *  date    2009/06/29
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   ��Ƽ ���μ��� �Ǵ� ��Ƽ�ھ� ���μ����� ���õ� ��� ����
 */

#ifndef __MULTIPROCESSOR_H__
#define __MULTIPROCESSOR_H__

#include "Types.h"

////////////////////////////////////////////////////////////////////////////////
//
// ��ũ��
//
////////////////////////////////////////////////////////////////////////////////
// MultiProcessor ���� ��ũ��
#define BOOTSTRAPPROCESSOR_FLAGADDRESS      0x7C09
// ���� ������ �ִ� ���μ��� �Ǵ� �ھ��� ����
#define MAXPROCESSORCOUNT                   16


////////////////////////////////////////////////////////////////////////////////
//
//  �Լ�
//
////////////////////////////////////////////////////////////////////////////////
BOOL kStartUpApplicationProcessor( void );
BYTE kGetAPICID( void );
static BOOL kWakeUpApplicationProcessor( void );

#endif /*__MULTIPROCESSOR_H__*/
