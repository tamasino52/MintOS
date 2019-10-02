/**
 *  file    WindowManager.c
 *  date    2009/10/04
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   ������ �Ŵ����� ���õ� �Լ��� ������ �ҽ� ����
 */
 
#include "Types.h"
#include "Window.h"
#include "WindowManagerTask.h"
#include "VBE.h"
#include "Mouse.h"
#include "Task.h"

/**
 *  ������ �Ŵ��� �½�ũ
 */
void kStartWindowManager( void )
{
    VBEMODEINFOBLOCK* pstVBEMode;
    int iRelativeX, iRelativeY;
    int iMouseX, iMouseY;
    BYTE bButton;
    QWORD qwWindowID;
    TCB* pstTask;
    char vcTempTitle[ WINDOW_TITLEMAXLENGTH ];
    int iWindowCount = 0;

    // ������ �Ŵ����� TCB�� ��ȯ
    pstTask = kGetRunningTask( kGetAPICID() );
    
    // GUI �ý����� �ʱ�ȭ
    kInitializeGUISystem();
    
    // ���� ���콺 ��ġ�� Ŀ���� ���
    kGetCursorPosition( &iMouseX, &iMouseY );
    kMoveCursor( iMouseX, iMouseY );
    
    //==========================================================================
    // ���콺 �̵��� ó���ϰ� ���콺 ��ư�� ���� ������ ������ ���� ����
    //==========================================================================
    while( 1 )
    {
        // ���콺 �����Ͱ� ���ŵǱ⸦ ��ٸ�
        if( kGetMouseDataFromMouseQueue( &bButton, &iRelativeX, &iRelativeY ) ==
            FALSE )
        {
            kSleep( 0 );
            continue;
        }
        
        // ���� ���콺 Ŀ�� ��ġ�� ��ȯ
        kGetCursorPosition( &iMouseX, &iMouseY );
        
        // ���콺�� ������ �Ÿ��� ���� Ŀ�� ��ġ�� ���ؼ� ���� ��ǥ�� ���
        iMouseX += iRelativeX;
        iMouseY += iRelativeY;
        
        // ���� ��ư�� �������� ������ ����
        if( bButton & MOUSE_LBUTTONDOWN )
        {
            // ���� ���콺 Ŀ���� �ִ� ��ġ�� �����츦 ����. �Ʒ� �ڵ忡�� ������ ���ο� 
            // ���ڸ� ����� �� �����츦 ǥ���Ϸ��� ������ �Ӽ����� WINDOW_FLAGS_SHOW��
            // ������
            kSPrintf( vcTempTitle, "MINT64 OS Test Window %d", iWindowCount++ );
            qwWindowID = kCreateWindow( iMouseX - 10, iMouseY - WINDOW_TITLEBAR_HEIGHT / 2,
                 400, 200, WINDOW_FLAGS_DRAWFRAME | WINDOW_FLAGS_DRAWTITLE, vcTempTitle );
            
            // ������ ���ο� �ؽ�Ʈ ����ϰ� �����츦 ȭ�鿡 ��Ÿ��
            kDrawText( qwWindowID, 10, WINDOW_TITLEBAR_HEIGHT + 10, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, "This is real window~!!", 22 );
            kDrawText( qwWindowID, 10, WINDOW_TITLEBAR_HEIGHT + 30, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, "No more prototype~!!", 18 );
            kShowWindow( qwWindowID, TRUE );
        }
        // ������ ��ư�� �������� ������ �Ŵ��� �½�ũ�� ������ ��� �����츦 ����
        else if( bButton & MOUSE_RBUTTONDOWN )
        {
            // �½�ũ�� ������ ��� �����츦 ����
            kDeleteAllWindowInTaskID( pstTask->stLink.qwID );
            iWindowCount = 0;
        }
        
        // ���ο� ��ġ�� ���콺 Ŀ���� �̵�
        kMoveCursor( iMouseX, iMouseY );
    }
}

