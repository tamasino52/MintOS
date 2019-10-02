/**
 *  file    2DGraphics.h
 *  date    2009/09/5
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   2D Graphic�� ���� ��� ����
 */

#ifndef __2DGRAPHICS_H__
#define __2DGRAPHICS_H__

#include "Types.h"

////////////////////////////////////////////////////////////////////////////////
//
// ��ũ��
//
////////////////////////////////////////////////////////////////////////////////
// ���� �����ϴµ� ����� �ڷᱸ��, 16��Ʈ ���� ����ϹǷ� WORD�� ����
typedef WORD                COLOR;

// 0~255 ������ R, G, B�� 16��Ʈ �� �������� ��ȯ�ϴ� ��ũ��
// 0~255�� ������ 0~31, 0~63���� ����Ͽ� ����ϹǷ� ���� 8�� 4�� ��������� ��
// ������ 8�� ������ 4�� >> 3�� >> 2�� ��ü
#define RGB( r, g, b )      ( ( ( BYTE )( r ) >> 3 ) << 11 | \
                ( ( ( BYTE )( g ) >> 2 ) ) << 5 |  ( ( BYTE )( b ) >> 3 ) )


////////////////////////////////////////////////////////////////////////////////
//
// ����ü
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// �Լ�
//
////////////////////////////////////////////////////////////////////////////////
inline void kDrawPixel( int iX, int iY, COLOR stColor );
void kDrawLine( int iX1, int iY1, int iX2, int iY2, COLOR stColor );
void kDrawRect( int iX1, int iY1, int iX2, int iY2, COLOR stColor, BOOL bFill );
void kDrawCircle( int iX, int iY, int iRadius, COLOR stColor, BOOL bFill );
void kDrawText( int iX, int iY, COLOR stTextColor, COLOR stBackgroundColor, 
        const char* pcString, int iLength );

#endif /*__2DGRAPHICS_H__*/
