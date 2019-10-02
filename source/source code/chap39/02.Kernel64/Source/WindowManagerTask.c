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
    int iMouseX, iMouseY;
    BOOL bMouseDataResult;
    BOOL bKeyDataResult;
    BOOL bEventQueueResult;

    // GUI �ý����� �ʱ�ȭ
    kInitializeGUISystem();
    
    // ���� ���콺 ��ġ�� Ŀ���� ���
    kGetCursorPosition( &iMouseX, &iMouseY );
    kMoveCursor( iMouseX, iMouseY );
        
    // ������ �Ŵ��� �½�ũ ����
    while( 1 )
    {
        // ���콺 �����͸� ó��
        bMouseDataResult = kProcessMouseData();
        
        // Ű �����͸� ó��
        bKeyDataResult = kProcessKeyData();

        // ������ �Ŵ����� �̺�Ʈ ť�� ���ŵ� �����͸� ó��. ���ŵ� ��� �̺�Ʈ�� ó����
        bEventQueueResult = FALSE;
        while( kProcessEventQueueData() == TRUE )
        {
            bEventQueueResult = TRUE;
        }
        
        // ó���� �����Ͱ� �ϳ��� ���ٸ� Sleep()�� �����Ͽ� ���μ����� �纸��
        if( ( bMouseDataResult == FALSE ) &&
            ( bKeyDataResult == FALSE ) &&
            ( bEventQueueResult == FALSE ) )
        {
            kSleep( 0 );
        }
    }
}

/**
 *  ���ŵ� ���콺 �����͸� ó��
 */
BOOL kProcessMouseData( void )
{
    QWORD qwWindowIDUnderMouse;
    BYTE bButtonStatus;
    int iRelativeX, iRelativeY;
    int iMouseX, iMouseY;
    int iPreviousMouseX, iPreviousMouseY;
    BYTE bChangedButton;
    RECT stWindowArea;
    EVENT stEvent;
    WINDOWMANAGER* pstWindowManager;
    char vcTempTitle[ WINDOW_TITLEMAXLENGTH ];
    static int iWindowCount = 0;
    QWORD qwWindowID;

    // ���콺 �����Ͱ� ���ŵǱ⸦ ��ٸ�
    if( kGetMouseDataFromMouseQueue( &bButtonStatus, &iRelativeX, &iRelativeY ) ==
        FALSE )
    {
        return FALSE;
    }
    
    // ������ �Ŵ����� ��ȯ
    pstWindowManager = kGetWindowManager();

    // ���� ���콺 Ŀ�� ��ġ�� ��ȯ
    kGetCursorPosition( &iMouseX, &iMouseY );
    
    // �����̱� ������ ��ǥ�� ����
    iPreviousMouseX = iMouseX;
    iPreviousMouseY = iMouseY;
    
    // ���콺�� ������ �Ÿ��� ���� Ŀ�� ��ġ�� ���ؼ� ���� ��ǥ�� ���
    iMouseX += iRelativeX;
    iMouseY += iRelativeY;
    
    // ���ο� ��ġ�� ���콺 Ŀ���� �̵��ϰ� �ٽ� ���� Ŀ���� ��ġ�� ��ȯ.
    // ���콺 Ŀ���� ȭ���� ����� �ʵ��� ó���� Ŀ�� ��ǥ�� ����Ͽ� ȭ���� ���
    // Ŀ���� ���� �߻��ϴ� ������ ����
    kMoveCursor( iMouseX, iMouseY );
    kGetCursorPosition( &iMouseX, &iMouseY );
    
    // ���� ���콺 Ŀ�� �Ʒ��� �ִ� �����츦 �˻�
    qwWindowIDUnderMouse = kFindWindowByPoint( iMouseX, iMouseY );
    
    //--------------------------------------------------------------------------
    // ��ư ���°� ���ߴ��� Ȯ���ϰ� ��ư ���¿� ���� ���콺 �޽����� ������ �޽�����
    // ����
    //--------------------------------------------------------------------------
    // ��ư ���´� ���� ��ư ���¿� ���� ��ư ���¸� XOR�Ͽ� 1�� �����ƴ����� Ȯ��
    bChangedButton = pstWindowManager->bPreviousButtonStatus ^ bButtonStatus;
    
    // ���콺 ���� ��ư�� ��ȭ�� ���� ��� ó��
    if( bChangedButton & MOUSE_LBUTTONDOWN )
    {
        // ���� ��ư�� ���� ��� ó��
        if( bButtonStatus & MOUSE_LBUTTONDOWN )
        {
            // ���콺�� �����츦 ������ ���̹Ƿ�, ���콺 �Ʒ��� �ִ� �����찡
            // ��� �����찡 �ƴ� ��� �ֻ����� �÷���
            if( qwWindowIDUnderMouse != pstWindowManager->qwBackgoundWindowID )
            {
                // ���õ� �����츦 �ֻ����� ����
                // �����츦 �ֻ����� ����鼭 ���ÿ� ������ ���ð� ���� ���� �̺�Ʈ��
                // ���� ����
                kMoveWindowToTop( qwWindowIDUnderMouse );
            }
            
            //------------------------------------------------------------------
            // ���� ��ư�� ���� ��ġ�� ���� ǥ������ ��ġ�̸� ������ �̵����� �Ǵ�
            // �ݱ� ��ư ��ġ���� ���ȴ��� Ȯ���Ͽ� ó��
            //------------------------------------------------------------------
            if( kIsInTitleBar( qwWindowIDUnderMouse, iMouseX, iMouseY ) == TRUE )
            {
                // �ݱ� ��ư���� ���������� �����쿡 �ݱ� ����
                if( kIsInCloseButton( qwWindowIDUnderMouse, iMouseX, iMouseY ) 
                        == TRUE )
                {
                    // ������ �ݱ� �̺�Ʈ�� ����
                    kSetWindowEvent( qwWindowIDUnderMouse, EVENT_WINDOW_CLOSE,
                            &stEvent );
                    kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );
                    
                    //----------------------------------------------------------
                    // �׽�Ʈ�� ���� �Ͻ������� �߰��� �κ�
                    //----------------------------------------------------------
                    kDeleteWindow( qwWindowIDUnderMouse );                        
                }
                // �ݱ� ��ư�� �ƴϸ� ������ �̵� ���� ����
                else
                {
                    // ������ �̵� ��� ����
                    pstWindowManager->bWindowMoveMode = TRUE;
                    
                    // ���� �����츦 �̵��ϴ� ������� ����
                    pstWindowManager->qwMovingWindowID = qwWindowIDUnderMouse;
                }
            }
            // ������ ���ο��� ���� ���̸� ���� ��ư�� ���������� ����
            else
            {
                // ���� ��ư ���� �̺�Ʈ�� ����
                kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_LBUTTONDOWN,
                        iMouseX, iMouseY, bButtonStatus, &stEvent );
                kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );
            }
        }
        // ���� ��ư�� ������ ��� ó��
        else
        {
            // �����찡 �̵� ���̾����� ��常 ����
            if( pstWindowManager->bWindowMoveMode == TRUE )
            {
                // �̵� ���̶�� �÷��׸� ����
                pstWindowManager->bWindowMoveMode = FALSE;
                pstWindowManager->qwMovingWindowID = WINDOW_INVALIDID;
            }
            // �����찡 �̵� ���� �ƴϾ����� ������� ���� ��ư ������ �̺�Ʈ�� ����
            else
            {
                // ���� ��ư ������ �̺�Ʈ�� ����
                kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_LBUTTONUP,
                        iMouseX, iMouseY, bButtonStatus, &stEvent );
                kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );
            }
        }
    }            
    // ���콺 ������ ��ư�� ��ȭ�� ���� ��� ó��
    else if( bChangedButton & MOUSE_RBUTTONDOWN )
    {
        // ������ ��ư�� ���� ��� ó��
        if( bButtonStatus & MOUSE_RBUTTONDOWN )
        {
            // ������ ��ư ���� �̺�Ʈ�� ����
            kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_RBUTTONDOWN,
                    iMouseX, iMouseY, bButtonStatus, &stEvent );
            kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );
            
            //------------------------------------------------------------------
            // �׽�Ʈ�� ���� �Ͻ������� �߰��� �κ�
            //------------------------------------------------------------------
            // �׽�Ʈ�� ���� ������ ��ư�� ������ ������ ����
            kSPrintf( vcTempTitle, "MINT64 OS Test Window %d", iWindowCount++ );
            qwWindowID = kCreateWindow( iMouseX - 10, iMouseY - WINDOW_TITLEBAR_HEIGHT / 2,
                 400, 200, WINDOW_FLAGS_DRAWFRAME | WINDOW_FLAGS_DRAWTITLE, vcTempTitle );                    
            
            // ������ ���ο� �ؽ�Ʈ�� ����ϰ� �����츦 ȭ�鿡 ��Ÿ��
            kDrawText( qwWindowID, 10, WINDOW_TITLEBAR_HEIGHT + 10, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, "This is real window~!!", 22 );
            kDrawText( qwWindowID, 10, WINDOW_TITLEBAR_HEIGHT + 30, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, "Now you can move and select window~!!", 
                    37 );
            kShowWindow( qwWindowID, TRUE );
        }
        else
        {
            // ������ ��ư ������ �̺�Ʈ�� ����
            kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_RBUTTONUP,
                    iMouseX, iMouseY, bButtonStatus, &stEvent );
            kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );
        }
    }
    // ���콺 ��� ��ư�� ��ȭ�� ���� ��� ó��
    else if( bChangedButton & MOUSE_MBUTTONDOWN )
    {
        // ��� ��ư�� ���� ��� ó��
        if( bButtonStatus & MOUSE_MBUTTONDOWN )
        {
            // ��� ��ư ���� �̺�Ʈ�� ����
            kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_MBUTTONDOWN,
                    iMouseX, iMouseY, bButtonStatus, &stEvent );
            kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );
        }
        else
        {
            // ��� ��ư ������ �̺�Ʈ�� ����
            kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_MBUTTONUP,
                    iMouseX, iMouseY, bButtonStatus, &stEvent );
            kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );
        }
    }        
    // ���콺 ��ư�� ������� �ʾ����� ���콺 �̵� ó���� ����
    else
    {
        // ���콺 �̵� �̺�Ʈ�� ����
        kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_MOVE, iMouseX, iMouseY, 
                    bButtonStatus, &stEvent );
        kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );
    }

    // �����찡 �̵� ���̾����� ������ �̵� ó��
    if( pstWindowManager->bWindowMoveMode == TRUE )
    {
        // �������� ��ġ�� ����
        if( kGetWindowArea( pstWindowManager->qwMovingWindowID, &stWindowArea )
                == TRUE )
        {
            // �������� ���� ��ġ�� �� ���콺�� �̵��� ��ŭ �Ű���
            // ������ �̵� �̺�Ʈ�� �Լ� ���ο��� ����
            kMoveWindow( pstWindowManager->qwMovingWindowID, 
                         stWindowArea.iX1 + iMouseX - iPreviousMouseX, 
                         stWindowArea.iY1 + iMouseY - iPreviousMouseY ); 
        }
        // �������� ��ġ�� ���� �� ������ �����찡 �������� �ʴ� ���̹Ƿ�
        // ������ �̵� ��� ����
        else
        {
            // �̵� ���̶�� �÷��׸� ����
            pstWindowManager->bWindowMoveMode = FALSE;
            pstWindowManager->qwMovingWindowID = WINDOW_INVALIDID;
        }
    }
    
    // ���� ó���� ����Ϸ��� ���� ��ư ���¸� ������
    pstWindowManager->bPreviousButtonStatus = bButtonStatus;
    return TRUE;
}


/**
 *  ���ŵ� Ű �����͸� ó��
 */
BOOL kProcessKeyData( void )
{
    KEYDATA stKeyData;
    EVENT stEvent;
    QWORD qwAcitveWindowID;
    
    // Ű���� �����Ͱ� ���ŵǱ⸦ ��ٸ�
    if( kGetKeyFromKeyQueue( &stKeyData ) == FALSE )
    {
        return FALSE;
    }

    // �ֻ��� ������, �� ���õ� ������� �޽����� ����
    qwAcitveWindowID = kGetTopWindowID();
    kSetKeyEvent( qwAcitveWindowID, &stKeyData, &stEvent );
    return kSendEventToWindow( qwAcitveWindowID, &stEvent );
}    

/**
 *  �̺�Ʈ ť�� ���ŵ� �̺�Ʈ ó��
 */
BOOL kProcessEventQueueData( void )
{
    EVENT stEvent;
    WINDOWEVENT* pstWindowEvent;
    QWORD qwWindowID;
    RECT stArea;
    
    // ������ �Ŵ����� �̺�Ʈ ť�� �̺�Ʈ�� ���ŵǱ⸦ ��ٸ�
    if( kReceiveEventFromWindowManagerQueue( &stEvent ) == FALSE )
    {
        return FALSE;
    }
    
    pstWindowEvent = &( stEvent.stWindowEvent );
    
    // Ÿ�� ���� ó��
    switch( stEvent.qwType )
    {
        // ���� �����찡 �ִ� ������ ȭ�鿡 ������Ʈ
    case EVENT_WINDOWMANAGER_UPDATESCREENBYID:
        if( kGetWindowArea( pstWindowEvent->qwWindowID, &stArea ) == TRUE )
        {
            kRedrawWindowByArea( &stArea );        
        }
        break;

        // �������� ���� ������ ȭ�鿡 ������Ʈ
    case EVENT_WINDOWMANAGER_UPDATESCREENBYWINDOWAREA:
        // �����츦 �������� �� ��ǥ�� ȭ�� ��ǥ�� ��ȯ�Ͽ� ������Ʈ ó��
        if( kConvertRectClientToScreen( pstWindowEvent->qwWindowID, 
                &( pstWindowEvent->stArea ), &stArea ) == TRUE )
        {
            kRedrawWindowByArea( &stArea );
        }
        break;

        // ȭ�� ��ǥ�� ���޵� ������ ȭ�鿡 ������Ʈ
    case EVENT_WINDOWMANAGER_UPDATESCREENBYSCREENAREA:
        kRedrawWindowByArea( &( pstWindowEvent->stArea ) );
        break;

    default:
        break;
    }
    
    return TRUE;
}
