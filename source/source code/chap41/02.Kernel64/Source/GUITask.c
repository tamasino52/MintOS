/**
 *  file    GUITask.c
 *  date    2009/10/20
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   GUI �½�ũ�� ���õ� �Լ��� ������ �ҽ� ����
 */
#include "GUITask.h"
#include "Window.h"

//------------------------------------------------------------------------------
//  �⺻ GUI �½�ũ
//------------------------------------------------------------------------------
/**
 *  �⺻ GUI �½�ũ�� �ڵ�
 *      GUI �½�ũ�� ���� �� �����Ͽ� �⺻ �ڵ�� ���
 */
void kBaseGUITask( void )
{
    QWORD qwWindowID;
    int iMouseX, iMouseY;
    int iWindowWidth, iWindowHeight;
    EVENT stReceivedEvent;
    MOUSEEVENT* pstMouseEvent;
    KEYEVENT* pstKeyEvent;
    WINDOWEVENT* pstWindowEvent;

    //--------------------------------------------------------------------------
    // �׷��� ��� �Ǵ�
    //--------------------------------------------------------------------------
    // MINT64 OS�� �׷��� ���� �����ߴ��� Ȯ��
    if( kIsGraphicMode() == FALSE )
    {        
        // MINT64 OS�� �׷��� ���� �������� �ʾҴٸ� ����
        kPrintf( "This task can run only GUI mode~!!!\n" );
        return ;
    }
    
    //--------------------------------------------------------------------------
    // �����츦 ����
    //--------------------------------------------------------------------------
    // ���콺�� ���� ��ġ�� ��ȯ
    kGetCursorPosition( &iMouseX, &iMouseY );

    // �������� ũ��� ���� ����
    iWindowWidth = 500;
    iWindowHeight = 200;
    
    // ������ ���� �Լ� ȣ��, ���콺�� �ִ� ��ġ�� �������� ����
    qwWindowID = kCreateWindow( iMouseX - 10, iMouseY - WINDOW_TITLEBAR_HEIGHT / 2,
        iWindowWidth, iWindowHeight, WINDOW_FLAGS_DEFAULT, "Hello World Window" );
    // �����츦 �������� �������� ����
    if( qwWindowID == WINDOW_INVALIDID )
    {
        return ;
    }
    
    //--------------------------------------------------------------------------
    // GUI �½�ũ�� �̺�Ʈ ó�� ����
    //--------------------------------------------------------------------------
    while( 1 )
    {
        // �̺�Ʈ ť���� �̺�Ʈ�� ����
        if( kReceiveEventFromWindowQueue( qwWindowID, &stReceivedEvent ) == FALSE )
        {
            kSleep( 0 );
            continue;
        }
        
        // ���ŵ� �̺�Ʈ�� Ÿ�Կ� ���� ������ ó��
        switch( stReceivedEvent.qwType )
        {
            // ���콺 �̺�Ʈ ó��
        case EVENT_MOUSE_MOVE:
        case EVENT_MOUSE_LBUTTONDOWN:
        case EVENT_MOUSE_LBUTTONUP:            
        case EVENT_MOUSE_RBUTTONDOWN:
        case EVENT_MOUSE_RBUTTONUP:
        case EVENT_MOUSE_MBUTTONDOWN:
        case EVENT_MOUSE_MBUTTONUP:
            // ���⿡ ���콺 �̺�Ʈ ó�� �ڵ� �ֱ�
            pstMouseEvent = &( stReceivedEvent.stMouseEvent );
            break;

            // Ű �̺�Ʈ ó��
        case EVENT_KEY_DOWN:
        case EVENT_KEY_UP:
            // ���⿡ Ű���� �̺�Ʈ ó�� �ڵ� �ֱ�
            pstKeyEvent = &( stReceivedEvent.stKeyEvent );
            break;

            // ������ �̺�Ʈ ó��
        case EVENT_WINDOW_SELECT:
        case EVENT_WINDOW_DESELECT:
        case EVENT_WINDOW_MOVE:
        case EVENT_WINDOW_RESIZE:
        case EVENT_WINDOW_CLOSE:
            // ���⿡ ������ �̺�Ʈ ó�� �ڵ� �ֱ�
            pstWindowEvent = &( stReceivedEvent.stWindowEvent );

            //------------------------------------------------------------------
            // ������ �ݱ� �̺�Ʈ�̸� �����츦 �����ϰ� ������ �������� �½�ũ�� ����
            //------------------------------------------------------------------
            if( stReceivedEvent.qwType == EVENT_WINDOW_CLOSE )
            {
                // ������ ����
                kDeleteWindow( qwWindowID );
                return ;
            }
            break;
            
            // �� �� ����
        default:
            // ���⿡ �� �� ���� �̺�Ʈ ó�� �ڵ� �ֱ�
            break;
        }
    }
}

//------------------------------------------------------------------------------
//  Hello World GUI �½�ũ
//------------------------------------------------------------------------------
/**
 *  Hello World GUI �½�ũ
 */
void kHelloWorldGUITask( void )
{
    QWORD qwWindowID;
    int iMouseX, iMouseY;
    int iWindowWidth, iWindowHeight;
    EVENT stReceivedEvent;
    MOUSEEVENT* pstMouseEvent;
    KEYEVENT* pstKeyEvent;
    WINDOWEVENT* pstWindowEvent;
    int iY;
    char vcTempBuffer[ 50 ];
    static int s_iWindowCount = 0;
    // �̺�Ʈ Ÿ�� ���ڿ�
    char* vpcEventString[] = { 
            "Unknown",
            "MOUSE_MOVE       ",
            "MOUSE_LBUTTONDOWN",
            "MOUSE_LBUTTONUP  ",
            "MOUSE_RBUTTONDOWN",
            "MOUSE_RBUTTONUP  ",
            "MOUSE_MBUTTONDOWN",
            "MOUSE_MBUTTONUP  ",
            "WINDOW_SELECT    ",
            "WINDOW_DESELECT  ",
            "WINDOW_MOVE      ",
            "WINDOW_RESIZE    ",
            "WINDOW_CLOSE     ",            
            "KEY_DOWN         ",
            "KEY_UP           " };
    RECT stButtonArea;
    QWORD qwFindWindowID;
    EVENT stSendEvent;
    int i;

    //--------------------------------------------------------------------------
    // �׷��� ��� �Ǵ�
    //--------------------------------------------------------------------------
    // MINT64 OS�� �׷��� ���� �����ߴ��� Ȯ��
    if( kIsGraphicMode() == FALSE )
    {        
        // MINT64 OS�� �׷��� ���� �������� �ʾҴٸ� ����
        kPrintf( "This task can run only GUI mode~!!!\n" );
        return ;
    }
    
    //--------------------------------------------------------------------------
    // �����츦 ����
    //--------------------------------------------------------------------------
    // ���콺�� ���� ��ġ�� ��ȯ
    kGetCursorPosition( &iMouseX, &iMouseY );

    // �������� ũ��� ���� ����
    iWindowWidth = 500;
    iWindowHeight = 200;
    
    // ������ ���� �Լ� ȣ��, ���콺�� �ִ� ��ġ�� �������� �����ϰ� ��ȣ�� �߰��Ͽ�
    // �����츶�� �������� �̸��� �Ҵ�
    kSPrintf( vcTempBuffer, "Hello World Window %d", s_iWindowCount++ );
    qwWindowID = kCreateWindow( iMouseX - 10, iMouseY - WINDOW_TITLEBAR_HEIGHT / 2,
        iWindowWidth, iWindowHeight, WINDOW_FLAGS_DEFAULT, vcTempBuffer );
    // �����츦 �������� �������� ����
    if( qwWindowID == WINDOW_INVALIDID )
    {
        return ;
    }
    
    //--------------------------------------------------------------------------
    // ������ �Ŵ����� ������� �����ϴ� �̺�Ʈ�� ǥ���ϴ� ������ �׸�
    //--------------------------------------------------------------------------
    // �̺�Ʈ ������ ����� ��ġ ����
    iY = WINDOW_TITLEBAR_HEIGHT + 10;
    
    // �̺�Ʈ ������ ǥ���ϴ� ������ �׵θ��� ������ ID�� ǥ��
    kDrawRect( qwWindowID, 10, iY + 8, iWindowWidth - 10, iY + 70, RGB( 0, 0, 0 ),
            FALSE );
    kSPrintf( vcTempBuffer, "GUI Event Information[Window ID: 0x%Q]", qwWindowID );
    kDrawText( qwWindowID, 20, iY, RGB( 0, 0, 0 ), RGB( 255, 255, 255 ), 
               vcTempBuffer, kStrLen( vcTempBuffer ) );    
    
    //--------------------------------------------------------------------------
    // ȭ�� �Ʒ��� �̺�Ʈ ���� ��ư�� �׸�
    //--------------------------------------------------------------------------
    // ��ư ������ ����
    kSetRectangleData( 10, iY + 80, iWindowWidth - 10, iWindowHeight - 10, 
            &stButtonArea );
    // ����� �������� �������� �����ϰ� ���ڴ� ���������� �����Ͽ� ��ư�� �׸�
    kDrawButton( qwWindowID, &stButtonArea, WINDOW_COLOR_BACKGROUND, 
            "User Message Send Button(Up)", RGB( 0, 0, 0 ) );
    // �����츦 ȭ�鿡 ǥ��
    kShowWindow( qwWindowID, TRUE );
    
    //--------------------------------------------------------------------------
    // GUI �½�ũ�� �̺�Ʈ ó�� ����
    //--------------------------------------------------------------------------
    while( 1 )
    {
        // �̺�Ʈ ť���� �̺�Ʈ�� ����
        if( kReceiveEventFromWindowQueue( qwWindowID, &stReceivedEvent ) == FALSE )
        {
            kSleep( 0 );
            continue;
        }
        
        // ������ �̺�Ʈ ������ ǥ�õ� ������ �������� ĥ�Ͽ� ������ ǥ���� �����͸�
        // ��� ����
        kDrawRect( qwWindowID, 11, iY + 20, iWindowWidth - 11, iY + 69, 
                   WINDOW_COLOR_BACKGROUND, TRUE );        
        
        // ���ŵ� �̺�Ʈ�� Ÿ�Կ� ���� ������ ó��
        switch( stReceivedEvent.qwType )
        {
            // ���콺 �̺�Ʈ ó��
        case EVENT_MOUSE_MOVE:
        case EVENT_MOUSE_LBUTTONDOWN:
        case EVENT_MOUSE_LBUTTONUP:            
        case EVENT_MOUSE_RBUTTONDOWN:
        case EVENT_MOUSE_RBUTTONUP:
        case EVENT_MOUSE_MBUTTONDOWN:
        case EVENT_MOUSE_MBUTTONUP:
            // ���⿡ ���콺 �̺�Ʈ ó�� �ڵ� �ֱ�
            pstMouseEvent = &( stReceivedEvent.stMouseEvent );

            // ���콺 �̺�Ʈ�� Ÿ���� ���
            kSPrintf( vcTempBuffer, "Mouse Event: %s", 
                      vpcEventString[ stReceivedEvent.qwType ] );
            kDrawText( qwWindowID, 20, iY + 20, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, vcTempBuffer, kStrLen( vcTempBuffer ) );
            
            // ���콺 �����͸� ���
            kSPrintf( vcTempBuffer, "Data: X = %d, Y = %d, Button = %X", 
                     pstMouseEvent->stPoint.iX, pstMouseEvent->stPoint.iY,
                     pstMouseEvent->bButtonStatus );
            kDrawText( qwWindowID, 20, iY + 40, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, vcTempBuffer, kStrLen( vcTempBuffer ) );

            //------------------------------------------------------------------
            // ���콺 ���� �Ǵ� ������ �̺�Ʈ�̸� ��ư�� ������ �ٽ� �׸�
            //------------------------------------------------------------------
            // ���콺 ���� ��ư�� ������ �� ��ư ó��
            if( stReceivedEvent.qwType == EVENT_MOUSE_LBUTTONDOWN )
            {
                // ��ư ������ ���콺 ���� ��ư�� ���ȴ����� �Ǵ�
                if( kIsInRectangle( &stButtonArea, pstMouseEvent->stPoint.iX, 
                                    pstMouseEvent->stPoint.iY ) == TRUE )
                {
                    // ��ư�� ����� ���� ������� �����Ͽ� �������� ǥ��
                    kDrawButton( qwWindowID, &stButtonArea, 
                                 RGB( 79, 204, 11 ), "User Message Send Button(Down)",
                                 RGB( 255, 255, 255 ) );
                    kUpdateScreenByID( qwWindowID );
                    
                    //----------------------------------------------------------
                    // �ٸ� ������� ���� �̺�Ʈ�� ����
                    //----------------------------------------------------------
                    // ������ ��� �����츦 ã�Ƽ� �̺�Ʈ�� ����
                    stSendEvent.qwType = EVENT_USER_TESTMESSAGE;
                    stSendEvent.vqwData[ 0 ] = qwWindowID;
                    stSendEvent.vqwData[ 1 ] = 0x1234;
                    stSendEvent.vqwData[ 2 ] = 0x5678;
                    
                    // ������ �������� �� ��ŭ ������ �����ϸ鼭 �̺�Ʈ�� ����
                    for( i = 0 ; i < s_iWindowCount ; i++ )
                    {
                        // ������ �������� �����츦 �˻�
                        kSPrintf( vcTempBuffer, "Hello World Window %d", i );
                        qwFindWindowID = kFindWindowByTitle( vcTempBuffer );
                        // �����찡 �����ϸ� ������ �ڽ��� �ƴ� ���� �̺�Ʈ�� ����
                        if( ( qwFindWindowID != WINDOW_INVALIDID ) &&
                            ( qwFindWindowID != qwWindowID ) )
                        {
                            // ������� �̺�Ʈ ����
                            kSendEventToWindow( qwFindWindowID, &stSendEvent );
                        }
                    }
                }
            }
            // ���콺 ���� ��ư�� �������� �� ��ư ó��
            else if( stReceivedEvent.qwType == EVENT_MOUSE_LBUTTONUP )
            {
                // ��ư�� ����� ������� �����Ͽ� ������ �ʾ����� ǥ��
                kDrawButton( qwWindowID, &stButtonArea, 
                    WINDOW_COLOR_BACKGROUND, "User Message Send Button(Up)", 
                    RGB( 0, 0, 0 ) );
            }            
            break;

            // Ű �̺�Ʈ ó��
        case EVENT_KEY_DOWN:
        case EVENT_KEY_UP:
            // ���⿡ Ű���� �̺�Ʈ ó�� �ڵ� �ֱ�
            pstKeyEvent = &( stReceivedEvent.stKeyEvent );

            // Ű �̺�Ʈ�� Ÿ���� ���
            kSPrintf( vcTempBuffer, "Key Event: %s", 
                      vpcEventString[ stReceivedEvent.qwType ] );
            kDrawText( qwWindowID, 20, iY + 20, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, vcTempBuffer, kStrLen( vcTempBuffer ) );
            
            // Ű �����͸� ���
            kSPrintf( vcTempBuffer, "Data: Key = %c, Flag = %X", 
                    pstKeyEvent->bASCIICode, pstKeyEvent->bFlags );
            kDrawText( qwWindowID, 20, iY + 40, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, vcTempBuffer, kStrLen( vcTempBuffer ) );
            break;

            // ������ �̺�Ʈ ó��
        case EVENT_WINDOW_SELECT:
        case EVENT_WINDOW_DESELECT:
        case EVENT_WINDOW_MOVE:
        case EVENT_WINDOW_RESIZE:
        case EVENT_WINDOW_CLOSE:
            // ���⿡ ������ �̺�Ʈ ó�� �ڵ� �ֱ�
            pstWindowEvent = &( stReceivedEvent.stWindowEvent );

            // ������ �̺�Ʈ�� Ÿ���� ���
            kSPrintf( vcTempBuffer, "Window Event: %s", 
                      vpcEventString[ stReceivedEvent.qwType ] );
            kDrawText( qwWindowID, 20, iY + 20, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, vcTempBuffer, kStrLen( vcTempBuffer ) );
            
            // ������ �����͸� ���
            kSPrintf( vcTempBuffer, "Data: X1 = %d, Y1 = %d, X2 = %d, Y2 = %d", 
                    pstWindowEvent->stArea.iX1, pstWindowEvent->stArea.iY1, 
                    pstWindowEvent->stArea.iX2, pstWindowEvent->stArea.iY2 );
            kDrawText( qwWindowID, 20, iY + 40, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, vcTempBuffer, kStrLen( vcTempBuffer ) );
            
            //------------------------------------------------------------------
            // ������ �ݱ� �̺�Ʈ�̸� �����츦 �����ϰ� ������ �������� �½�ũ�� ����
            //------------------------------------------------------------------
            if( stReceivedEvent.qwType == EVENT_WINDOW_CLOSE )
            {
                // ������ ����
                kDeleteWindow( qwWindowID );
                return ;
            }
            break;
            
            // �� �� ����
        default:
            // ���⿡ �� �� ���� �̺�Ʈ ó�� �ڵ� �ֱ�
            // �� �� ���� �̺�Ʈ�� ���
            kSPrintf( vcTempBuffer, "Unknown Event: 0x%X", stReceivedEvent.qwType );
            kDrawText( qwWindowID, 20, iY + 20, RGB( 0, 0, 0 ), WINDOW_COLOR_BACKGROUND,
                       vcTempBuffer, kStrLen( vcTempBuffer ) );
            
            // �����͸� ���
            kSPrintf( vcTempBuffer, "Data0 = 0x%Q, Data1 = 0x%Q, Data2 = 0x%Q",
                      stReceivedEvent.vqwData[ 0 ], stReceivedEvent.vqwData[ 1 ], 
                      stReceivedEvent.vqwData[ 2 ] );
            kDrawText( qwWindowID, 20, iY + 40, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, vcTempBuffer, kStrLen( vcTempBuffer ) );
            break;
        }

        // �����츦 ȭ�鿡 ������Ʈ
        kShowWindow( qwWindowID, TRUE );
    }
}
