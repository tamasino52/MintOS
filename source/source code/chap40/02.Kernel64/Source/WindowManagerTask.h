/**
 *  file    WindowManager.h
 *  date    2009/10/04
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   ������ �Ŵ����� ���õ� �Լ��� ������ ��� ����
 */

#ifndef __WINDOWMANAGER_H__
#define __WINDOWMANAGER_H__

////////////////////////////////////////////////////////////////////////////////
//
// �Լ�
//
////////////////////////////////////////////////////////////////////////////////
void kStartWindowManager( void );
BOOL kProcessMouseData( void );
BOOL kProcessKeyData( void );
BOOL kProcessEventQueueData( void );

#endif /*__WINDOWMANAGER_H__*/
