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
#include "GUITask.h"
#include "Font.h"

/**
 *  ������ �Ŵ��� �½�ũ
 */
void kStartWindowManager( void )
{
    int iMouseX, iMouseY;
    BOOL bMouseDataResult;
    BOOL bKeyDataResult;
    BOOL bEventQueueResult;
    //-------------------------------------------------------------------------
    QWORD qwLastTickCount;
    QWORD qwPreviousLoopExecutionCount;
    QWORD qwLoopExecutionCount;
    QWORD qwMinLoopExecutionCount;
    char vcTemp[ 40 ];
    RECT stLoopCountArea;
    QWORD qwBackgroundWindowID;
    //-------------------------------------------------------------------------

    // GUI �ý����� �ʱ�ȭ
    kInitializeGUISystem();
    
    // ���� ���콺 ��ġ�� Ŀ���� ���
    kGetCursorPosition( &iMouseX, &iMouseY );
    kMoveCursor( iMouseX, iMouseY );
    
    //-------------------------------------------------------------------------
    // ���� ���� Ƚ�� ������ ���� �ʱ�ȭ
    qwLastTickCount = kGetTickCount();
    qwPreviousLoopExecutionCount = 0;
    qwLoopExecutionCount = 0;
    qwMinLoopExecutionCount = 0xFFFFFFFFFFFFFFFF;
    qwBackgroundWindowID = kGetBackgroundWindowID();
    //-------------------------------------------------------------------------
    
    // ������ �Ŵ��� �½�ũ ����
    while( 1 )
    {
        //-------------------------------------------------------------------------
        // 1�ʸ��� ������ �Ŵ��� �½�ũ ������ ������ Ƚ���� �����Ͽ� �ּڰ��� ���
        if( kGetTickCount() - qwLastTickCount > 1000 )
        {
            qwLastTickCount = kGetTickCount();
            // 1�� ���� ������ �½�ũ ������ ���� ���� �½�ũ ������ ���� ����
            // �ּ� ���� ���� Ƚ���� ���Ͽ� �ּ� ���� ���� Ƚ���� ������Ʈ
            if( ( qwLoopExecutionCount - qwPreviousLoopExecutionCount ) < 
                    qwMinLoopExecutionCount )
            {
                qwMinLoopExecutionCount = 
                    qwLoopExecutionCount - qwPreviousLoopExecutionCount;
            }
            qwPreviousLoopExecutionCount = qwLoopExecutionCount;
            
            // ������ �ּ� ���� Ƚ���� 1�ʸ��� ������Ʈ
            kSPrintf( vcTemp, "MIN Loop Execution Count:%d   ", qwMinLoopExecutionCount );
            kDrawText( qwBackgroundWindowID, 0, 0, RGB( 0, 0, 0 ), RGB( 255, 255, 255 ), 
                    vcTemp, kStrLen( vcTemp ) );
            
            // ��� ������ ��ü�� ������Ʈ�ϸ� �ð��� ���� �ɸ��Ƿ� ��� �����쿡
            // ���� ���� Ƚ���� ��µ� �κи� ������Ʈ
            kSetRectangleData( 0, 0, kStrLen( vcTemp ) * FONT_ENGLISHWIDTH, 
                    FONT_ENGLISHHEIGHT, &stLoopCountArea );
            kRedrawWindowByArea( &stLoopCountArea, qwBackgroundWindowID );
        }
        qwLoopExecutionCount++;
        //-------------------------------------------------------------------------
        
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
    int i;
    
    // ������ �Ŵ����� ��ȯ
    pstWindowManager = kGetWindowManager();

    //--------------------------------------------------------------------------
    // ���콺 �̺�Ʈ�� �����ϴ� �κ�
    //--------------------------------------------------------------------------
    for( i = 0 ; i < WINDOWMANAGER_DATAACCUMULATECOUNT ; i++ )
    {
        // ���콺 �����Ͱ� ���ŵǱ⸦ ��ٸ�
        if( kGetMouseDataFromMouseQueue( &bButtonStatus, &iRelativeX, &iRelativeY ) ==
            FALSE )
        {
            // ó������ Ȯ���ߴµ� �����Ͱ� ���ٸ� ����
            if( i == 0 )
            {
                return FALSE;
            }
            // ó���� �ƴ� ���� ���� �������� ���콺 �̺�Ʈ�� ���������Ƿ� 
            // ������ �̺�Ʈ�� ó��
            else
            {
                break;
            }
        }
        
        // ���� ���콺 Ŀ�� ��ġ�� ��ȯ
        kGetCursorPosition( &iMouseX, &iMouseY );
        
        // ó�� ���콺 �̺�Ʈ�� ���ŵ� ���̸� ���� ��ǥ�� ���� ���콺�� ��ġ�� �����ص�
        if( i == 0 )
        {
            // �����̱� ������ ��ǥ�� ����
            iPreviousMouseX = iMouseX;
            iPreviousMouseY = iMouseY;
        }
        
        // ���콺�� ������ �Ÿ��� ���� Ŀ�� ��ġ�� ���ؼ� ���� ��ǥ�� ���
        iMouseX += iRelativeX;
        iMouseY += iRelativeY;
        
        // ���ο� ��ġ�� ���콺 Ŀ���� �̵��ϰ� �ٽ� ���� Ŀ���� ��ġ�� ��ȯ
        // ���콺 Ŀ���� ȭ���� ����� �ʵ��� ó���� Ŀ�� ��ǥ�� ����Ͽ� ȭ���� ���
        // Ŀ���� ���� �߻��ϴ� ������ ����
        kMoveCursor( iMouseX, iMouseY );
        kGetCursorPosition( &iMouseX, &iMouseY );
        
        // ��ư ���´� ���� ��ư ���¿� ���� ��ư ���¸� XOR�Ͽ� 1�� �����ƴ����� Ȯ��
        bChangedButton = pstWindowManager->bPreviousButtonStatus ^ bButtonStatus;

        // ���콺�� ���������� ��ư�� ��ȭ�� �ִٸ� �ٷ� �̺�Ʈ ó��
        if( bChangedButton != 0 )
        {
            break;
        }
    }
    
    //--------------------------------------------------------------------------
    // ���콺 �̺�Ʈ�� ó���ϴ� �κ�
    //--------------------------------------------------------------------------
    // ���� ���콺 Ŀ�� �Ʒ��� �ִ� �����츦 �˻�
    qwWindowIDUnderMouse = kFindWindowByPoint( iMouseX, iMouseY );
    
    //--------------------------------------------------------------------------
    // ��ư ���°� ���ߴ��� Ȯ���ϰ� ��ư ���¿� ���� ���콺 �޽����� ������ �޽�����
    // ����
    //--------------------------------------------------------------------------
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
            // �׽�Ʈ�� ���� ������ ��ư�� ������ GUI �½�ũ�� ����
            kCreateTask( TASK_FLAGS_LOW | TASK_FLAGS_THREAD, NULL, NULL, ( QWORD ) kHelloWorldGUITask, 
                    TASK_LOADBALANCINGID );
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
    EVENT vstEvent[ WINDOWMANAGER_DATAACCUMULATECOUNT ];
    int iEventCount;
    WINDOWEVENT* pstWindowEvent;
    WINDOWEVENT* pstNextWindowEvent;
    QWORD qwWindowID;
    RECT stArea;
    RECT stOverlappedArea;
    int i;
    int j;
    
    //--------------------------------------------------------------------------
    // ������ �Ŵ��� �½�ũ�� �̺�Ʈ ť�� ���ŵ� �̺�Ʈ�� �����ϴ� �κ�
    //--------------------------------------------------------------------------
    for( i = 0 ; i < WINDOWMANAGER_DATAACCUMULATECOUNT ; i++ )
    {
        // �̺�Ʈ�� ���ŵǱ⸦ ��ٸ�
        if( kReceiveEventFromWindowManagerQueue( &( vstEvent[ i ] ) ) == FALSE )
        {
            // ó������ �̺�Ʈ�� ���ŵ��� �ʾ����� ����
            if( i == 0 )
            {
                return FALSE;
            }
            else
            {
                break;
            }
        }
        
        pstWindowEvent = &( vstEvent[ i ].stWindowEvent );
        // ������ ID�� ������Ʈ�ϴ� �̺�Ʈ�� ���ŵǸ� ������ ������ �̺�Ʈ �����Ϳ� ����
        if( vstEvent[ i ].qwType == EVENT_WINDOWMANAGER_UPDATESCREENBYID )
        {
            // �������� ũ�⸦ �̺�Ʈ �ڷᱸ���� ����
            if( kGetWindowArea( pstWindowEvent->qwWindowID, &stArea ) == FALSE )
            {
                kSetRectangleData( 0, 0, 0, 0, &( pstWindowEvent->stArea ) );
            }
            else
            {
                kSetRectangleData( 0, 0, kGetRectangleWidth( &stArea ) - 1, 
                    kGetRectangleHeight( &stArea ) - 1, &( pstWindowEvent->stArea ) );
            }
        }
    }
    
    // ����� �̺�Ʈ�� �˻��ϸ鼭 ��ĥ �� �ִ� �̺�Ʈ�� �ϳ��� ����
    iEventCount = i;
    
    for( j = 0 ; j < iEventCount ; j++ )
    {
        // ���ŵ� �̺�Ʈ �߿� �̺�Ʈ �߿��� �̹��� ó���� �Ͱ� ���� �����쿡��
        // �߻��ϴ� ������ �̺�Ʈ�� �˻�
        pstWindowEvent = &( vstEvent[ j ].stWindowEvent );
        if( ( vstEvent[ j ].qwType != EVENT_WINDOWMANAGER_UPDATESCREENBYID ) &&
            ( vstEvent[ j ].qwType != EVENT_WINDOWMANAGER_UPDATESCREENBYWINDOWAREA ) &&
            ( vstEvent[ j ].qwType != EVENT_WINDOWMANAGER_UPDATESCREENBYSCREENAREA ) )
        {
            continue;
        }
        
        // ������ �̺�Ʈ�� ������ ������ �����ϸ鼭 ���ŵ� �̺�Ʈ�� �˻�
        for( i = j + 1 ; i < iEventCount ; i++ )
        {
            pstNextWindowEvent = &( vstEvent[ i ].stWindowEvent );
            // ȭ�� ������Ʈ�� �ƴϰų� ������ ID�� ��ġ���� ������ ����
            if( ( ( vstEvent[ i ].qwType != EVENT_WINDOWMANAGER_UPDATESCREENBYID ) &&
                  ( vstEvent[ i ].qwType != EVENT_WINDOWMANAGER_UPDATESCREENBYWINDOWAREA ) &&
                  ( vstEvent[ i ].qwType != EVENT_WINDOWMANAGER_UPDATESCREENBYSCREENAREA ) ) ||
                ( pstWindowEvent->qwWindowID != pstNextWindowEvent->qwWindowID ) )
            {
                continue;
            }
        
            // ��ġ�� ������ ����Ͽ� ��ġ�� ������ ����
            if( kGetOverlappedRectangle( &( pstWindowEvent->stArea ), 
                &( pstNextWindowEvent->stArea ), &stOverlappedArea ) == FALSE )
            {
                continue;
            }
            
            // �� ������ ��ġ�ϰų� ��� ������ ���ԵǸ� �̺�Ʈ�� ����
            if( kMemCmp( &( pstWindowEvent->stArea ), &stOverlappedArea,
                    sizeof( RECT ) ) == 0 )
            {
                // ���� �̺�Ʈ�� �������� ������ ��ġ�� ������ ��ġ�Ѵٸ� 
                // ���� �̺�Ʈ�� ������ ������ ���� ������ ������ ���ų� ������
                // ���� ���� �̺�Ʈ�� ���� �̺�Ʈ�� ������ ������ �����ϰ�
                // ���� �̺�Ʈ�� ����
                kMemCpy( &( pstWindowEvent->stArea ), &( pstNextWindowEvent->stArea ), 
                         sizeof( RECT ) );
                vstEvent[ i ].qwType = EVENT_UNKNOWN;
            }
            else if( kMemCmp( &( pstNextWindowEvent->stArea ), &stOverlappedArea,
                     sizeof( RECT ) ) == 0 )
            {
                // ���� �̺�Ʈ�� �������� ������ ��ġ�� ������ ��ġ�Ѵٸ� 
                // ���� �̺�Ʈ�� ������ ������ ���� ������ ������ ���ų� ������
                // ���� ������ ������ �������� �ʰ� ���� �̺�Ʈ�� ����
                vstEvent[ i ].qwType = EVENT_UNKNOWN;
            }
        }
    }
    
    // ���յ� �̺�Ʈ�� ��� ó��
    for( i = 0 ; i < iEventCount ; i++ )
    {
        pstWindowEvent = &( vstEvent[ i ].stWindowEvent );
        
        // Ÿ�� ���� ó��
        switch( vstEvent[ i ].qwType )
        {
            // ���� �����찡 �ִ� ������ ȭ�鿡 ������Ʈ
        case EVENT_WINDOWMANAGER_UPDATESCREENBYID:
            // �������� ���� ������ ȭ�鿡 ������Ʈ
        case EVENT_WINDOWMANAGER_UPDATESCREENBYWINDOWAREA:
            // �����츦 �������� �� ��ǥ�� ȭ�� ��ǥ�� ��ȯ�Ͽ� ������Ʈ ó��
            if( kConvertRectClientToScreen( pstWindowEvent->qwWindowID, 
                    &( pstWindowEvent->stArea ), &stArea ) == TRUE )
            {
                // ������ ������ ������ �����Ƿ� �״�� ȭ�� ������Ʈ �Լ��� ȣ��
                kRedrawWindowByArea( &stArea, pstWindowEvent->qwWindowID );        
            }
            break;
    
            // ȭ�� ��ǥ�� ���޵� ������ ȭ�鿡 ������Ʈ
        case EVENT_WINDOWMANAGER_UPDATESCREENBYSCREENAREA:
            kRedrawWindowByArea( &( pstWindowEvent->stArea ), WINDOW_INVALIDID );
            break;
    
        default:
            break;
        }
    }

    return TRUE;
}
