/**
 *  file    Main.h
 *  date    2010/03/10
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
// �������� �ʺ�� ����
#define BOARDWIDTH      8
#define BOARDHEIGHT     12

// ��� �ϳ��� ũ��
#define BLOCKSIZE       32
// �����̴� ����� ����
#define BLOCKCOUNT      3
// �� ����� ��Ÿ���� ��
#define EMPTYBLOCK      0
// ���� ����� ��Ÿ���� ��
#define ERASEBLOCK      0xFF
// ����� ����
#define BLOCKKIND       5

// �������� �ʺ�� ����
#define WINDOW_WIDTH        ( BOARDWIDTH * BLOCKSIZE )
#define WINDOW_HEIGHT       ( WINDOW_TITLEBAR_HEIGHT + INFORMATION_HEIGHT + \
                              BOARDHEIGHT * BLOCKSIZE )

// ���� ���� ������ ����
#define INFORMATION_HEIGHT  20


////////////////////////////////////////////////////////////////////////////////
//
// ����ü
//
////////////////////////////////////////////////////////////////////////////////
// ���� ������ �����ϴ� �ڷᱸ��
typedef struct GameInfoStruct
{
    //-------------------------------------------------------------------------
    // ���� �����̴� ��ϰ� �����ǿ� ������ ����� �����ϴµ� �ʿ��� �ʵ�
    //-------------------------------------------------------------------------
    // ��� ������ ���� ����(��� ���� ����� �׵θ� ����)
    COLOR vstBlockColor[ BLOCKKIND + 1 ];
    COLOR vstEdgeColor[ BLOCKKIND + 1 ];

    // ���� �����̴� ����� ��ġ
    int iBlockX;
    int iBlockY;

    // �����ǿ� ������ ����� ���¸� �����ϴ� ����
    BYTE vvbBoard[ BOARDHEIGHT ][ BOARDWIDTH ];

    // �����ǿ� ������ ��� �߿��� �����ؾ� �� ����� �����ϴ� ����
    BYTE vvbEraseBlock[ BOARDHEIGHT ][ BOARDWIDTH ];

    // ���� �����̴� ����� ������ �����ϴ� ����
    BYTE vbBlock[ BLOCKCOUNT ];

    //-------------------------------------------------------------------------
    // ������ �����ϴµ� �ʿ��� �ʵ�
    //-------------------------------------------------------------------------
    // ������ ���� ����
    BOOL bGameStart;

    // ������ ����
    QWORD qwScore;

    // ������ ����
    QWORD qwLevel;
} GAMEINFO;


////////////////////////////////////////////////////////////////////////////////
//
// �Լ�
//
////////////////////////////////////////////////////////////////////////////////
void Initialize( void );
void CreateBlock( void );
BOOL IsMovePossible( int iBlockX, int iBlockY );
BOOL FreezeBlock( int iBlockX, int iBlockY );
void EraseAllContinuousBlockOnBoard( QWORD qwWindowID );

BOOL MarkContinuousVerticalBlockOnBoard( void );
BOOL MarkContinuousHorizonBlockOnBoard( void );
BOOL MarkContinuousDiagonalBlockInBoard( void );
void EraseMarkedBlock( void );
void CompactBlockOnBoard( void );

void DrawInformation( QWORD qwWindowID );
void DrawGameArea( QWORD qwWindowID );

#endif /* __MAIN_H__ */
