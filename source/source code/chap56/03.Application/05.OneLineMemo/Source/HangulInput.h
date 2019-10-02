/**
 *  file    Main.c
 *  date    2010/03/30
 *  author  kkamagui
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   �ѱ� �Է¿� ���õ� ��� ����
 */

#ifndef __HANGULINPUT_H__
#define __HANGULINPUT_H__

////////////////////////////////////////////////////////////////////////////////
//
// ��ũ��
//
////////////////////////////////////////////////////////////////////////////////
// ���� �빮�ڸ� �ҹ��ڷ� �ٲ��ִ� ��ũ��
#define TOLOWER( x ) ( ( ( 'A' <= ( x ) ) && ( ( x ) <= 'Z' ) ) ? \
    ( ( x ) - 'A' + 'a' ) : ( x ) )

////////////////////////////////////////////////////////////////////////////////
//
// ����ü
//
////////////////////////////////////////////////////////////////////////////////
// �ѱ� �Է� ���̺��� �����ϴ� ����
typedef struct HangulInputTableItemStruct
{
    // �ѱ�
    char* pcHangul;

    // �ѱۿ� �����ϴ� Ű �Է�
    char* pcInput;
} HANGULINPUTITEM;

// �ѱ� �Է� ���̺��� �ε��� ���̺��� �����ϴ� ����
typedef struct HangulIndexTableItemStruct
{
    // �ѱ� ���ڸ� �Է��� �� ����ϴ� ù ��° Ű
    char cStartCharactor;

    // �ѱ� �Է� ���̺��� ���� �ε���
    DWORD dwIndex;
} HANGULINDEXITEM;


////////////////////////////////////////////////////////////////////////////////
//
// �Լ�
//
////////////////////////////////////////////////////////////////////////////////
BOOL IsJaum( char cInput );
BOOL IsMoum( char cInput );
BOOL FindLongestHangulInTable( const char* pcInputBuffer, int iBufferCount,
                        int* piMatchIndex, int* piMatchLength );
BOOL ComposeHangul( char* pcInputBuffer, int* piInputBufferLength,
    char* pcOutputBufferForProcessing, char* pcOutputBufferForComplete );

#endif /* __HANGULINPUT_H__ */
