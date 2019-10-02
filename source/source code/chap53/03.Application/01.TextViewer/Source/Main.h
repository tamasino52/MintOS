/**
 *  file    Main.h
 *  date    2010/01/03
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   C ���� �ۼ��� �������α׷��� ��Ʈ�� ����Ʈ ����
 */

#ifndef __MAIN_H__
#define __MAIN_H__

////////////////////////////////////////////////////////////////////////////////
//
// ��ũ��
//
////////////////////////////////////////////////////////////////////////////////
// �ִ�� ǥ���� �� �ִ� ������ ��
#define MAXLINECOUNT        ( 256 * 1024 )
// ������ ������ ���� ���� ǥ�� ���� ������ ���� ����
#define MARGIN              5
// ���� �����ϴ� ũ��
#define TABSPACE            4

////////////////////////////////////////////////////////////////////////////////
//
// ����ü
//
////////////////////////////////////////////////////////////////////////////////
// �ؽ�Ʈ ������ �����ϴ� ����ü
typedef struct TextInformationStruct
{
    // ���� ���ۿ� ������ ũ��
    BYTE* pbFileBuffer;
    DWORD dwFileSize;
    
    // ���� ���� ǥ�� ������ ����� �� �ִ� ���� ���� ���� �� ���� ��
    int iColumnCount;
    int iRowCount;

    // ���� ��ȣ�� ���� ���� �������� �����ϴ� ����
    DWORD* pdwFileOffsetOfLine;

    // ������ �ִ� ���� ��
    int iMaxLineCount;
    // ���� ������ �ε���
    int iCurrentLineIndex;

    // ���� �̸�
    char vcFileName[ 100 ];    
    
} TEXTINFO;

////////////////////////////////////////////////////////////////////////////////
//
// �Լ�
//
////////////////////////////////////////////////////////////////////////////////
BOOL ReadFileToBuffer( const char* pcFileName, TEXTINFO* pstInfo );
void CalculateFileOffsetOfLine( int iWidth, int iHeight, TEXTINFO* pstInfo );
BOOL DrawTextBuffer( QWORD qwWindowID, TEXTINFO* pstInfo );

#endif /*__MAIN_H__*/
