/**
 *  file    VBE.h
 *  date    2009/08/29
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   VBE�� ���õ� �Լ��� ������ �ҽ� ����
 */

#include "VBE.h"

// ��� ���� ��� �ڷᱸ��
static VBEMODEINFOBLOCK* gs_pstVBEModeBlockInfo = 
    ( VBEMODEINFOBLOCK* ) VBE_MODEINFOBLOCKADDRESS;

/**
 *  VBE ��� ���� ����� ��ȯ
 */
inline VBEMODEINFOBLOCK* kGetVBEModeInfoBlock( void )
{
    return gs_pstVBEModeBlockInfo;
}
