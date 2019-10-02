/**
 *  file    Main.h
 *  date    2010/02/20
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
// ������� �ִ� ����
#define MAXBUBBLECOUNT      50
// ������� ������
#define RADIUS              16
// ������� �⺻ �ӵ�
#define DEFAULTSPEED        3
// �÷��̾��� �ִ� ����
#define MAXLIFE             20

// �������� �ʺ�� ����
#define WINDOW_WIDTH        250
#define WINDOW_HEIGHT       350

// ���� ���� ������ ����
#define INFORMATION_HEIGHT  20


////////////////////////////////////////////////////////////////////////////////
//
// ����ü
//
////////////////////////////////////////////////////////////////////////////////
// ������� ������ �����ϴ� �ڷᱸ��
typedef struct BubbleStruct
{
    // X, Y ��ǥ
    QWORD qwX;
    QWORD qwY;

    // �������� �ӵ�(Y�� ��ȭ��)
    QWORD qwSpeed;

    // ����� ����
    COLOR stColor;

    // ����ִ��� ����
    BOOL bAlive;
} BUBBLE;

// ���� ������ �����ϴ� �ڷᱸ��
typedef struct GameInfoStruct
{
    //-------------------------------------------------------------------------
    // ������� �����ϴµ� �ʿ��� �ʵ�
    //-------------------------------------------------------------------------
    // ������� ������ �����ϴ� ����
    BUBBLE* pstBubbleBuffer;

    // ��� �ִ� ������� ��
    int iAliveBubbleCount;

    //-------------------------------------------------------------------------
    // ������ �����ϴµ� �ʿ��� �ʵ�
    //-------------------------------------------------------------------------
    // �÷��̾��� ����
    int iLife;

    // ������ ����
    QWORD qwScore;

    // ������ ���� ����
    BOOL bGameStart;
} GAMEINFO;

////////////////////////////////////////////////////////////////////////////////
//
// �Լ�
//
////////////////////////////////////////////////////////////////////////////////
BOOL Initialize( void );
BOOL CreateBubble( void );
void MoveBubble( void );
void DeleteBubbleUnderMouse( POINT* pstMouseXY );
void DrawInformation( QWORD qwWindowID );
void DrawGameArea( QWORD qwWindowID, POINT* pstMouseXY );

#endif /* __MAIN_H__ */
