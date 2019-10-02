/**
 *  file    Main.c
 *  date    2010/02/20
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   C ���� �ۼ��� �������α׷��� ��Ʈ�� ����Ʈ ����
 */

#include "MINTOSLibrary.h"
#include "Main.h"

// ���� ���� ������ �����ϴ� �ڷᱸ��
GAMEINFO g_stGameInfo = { 0, };

/**
 *  �������α׷��� C ��� ��Ʈ�� ����Ʈ
 */
int Main( char* pcArgument )
{
    QWORD qwWindowID;
    EVENT stEvent;
    QWORD qwLastTickCount;
    char* pcStartMessage = "Please LButton Down To Start~!";
    POINT stMouseXY;
    RECT stScreenArea;
    int iX;
    int iY;
    
    //--------------------------------------------------------------------------
    // �����츦 ȭ�� ����� ����x���� 250x350 ũ��� ����
    //--------------------------------------------------------------------------
    GetScreenArea( &stScreenArea );
    iX = ( GetRectangleWidth( &stScreenArea ) - WINDOW_WIDTH ) / 2;
    iY = ( GetRectangleHeight( &stScreenArea ) - WINDOW_HEIGHT ) / 2;
    qwWindowID = CreateWindow( iX, iY, WINDOW_WIDTH, WINDOW_HEIGHT,
                               WINDOW_FLAGS_DEFAULT, "Bubble Shooter" );
    if( qwWindowID == WINDOW_INVALIDID )
    {
        printf( "Window create fail\n" );
        return -1;
    }

    //--------------------------------------------------------------------------
    // ���ӿ� ���õ� ������ �ʱ�ȭ�ϰ� ����� ���۸� �Ҵ�
    //--------------------------------------------------------------------------
    // ���콺 ��ǥ ����
    stMouseXY.iX = WINDOW_WIDTH / 2;
    stMouseXY.iY = WINDOW_HEIGHT / 2;

    // ���� ������ �ʱ�ȭ
    if( Initialize() == FALSE )
    {
        // �ʱ�ȭ�� �����ϸ� �����츦 ����
        DeleteWindow( qwWindowID );
        return -1;
    }

    // ���� �ʱ갪(Random Seed) ����
    srand( GetTickCount() );

    //--------------------------------------------------------------------------
    // ���� ������ ���� ������ ����ϰ� ���� ���� ��� �޽����� ǥ��
    //--------------------------------------------------------------------------
    DrawInformation( qwWindowID );
    DrawGameArea( qwWindowID, &stMouseXY );
    DrawText( qwWindowID, 5, 150, RGB( 255, 255, 255 ), RGB( 0, 0, 0 ),
            pcStartMessage, strlen( pcStartMessage ) );

    // ��µ� �޽����� ȭ�鿡 ǥ��
    ShowWindow( qwWindowID, TRUE );

    //--------------------------------------------------------------------------
    // GUI �½�ũ�� �̺�Ʈ�� ���� ������ ó���ϴ� �κ�
    //--------------------------------------------------------------------------
    qwLastTickCount = GetTickCount();
    while( 1 )
    {
        //----------------------------------------------------------------------
        // �̺�Ʈ ó�� �κ�
        //----------------------------------------------------------------------
        // �̺�Ʈ ť���� �̺�Ʈ�� ����
        if( ReceiveEventFromWindowQueue( qwWindowID, &stEvent ) == TRUE )
        {
            // ���ŵ� �̺�Ʈ�� Ÿ�Կ� ���� ������ ó��
            switch( stEvent.qwType )
            {
                // ���콺 Ŭ�� ó��
            case EVENT_MOUSE_LBUTTONDOWN:
                // ���� ������ ���ϴ� Ŭ���̸� ������ ����
                if( g_stGameInfo.bGameStart == FALSE )
                {
                    // ���� ������ �ʱ�ȭ
                    Initialize();

                    // ���� ���� �÷��׸� ����
                    g_stGameInfo.bGameStart = TRUE;
                    break;
                }

                // ���콺�� Ŭ���� ���� �ִ� ������� ����
                DeleteBubbleUnderMouse( &( stEvent.stMouseEvent.stPoint ) );

                // ���콺 ��ġ ����
                memcpy( &stMouseXY, &( stEvent.stMouseEvent.stPoint ),
                        sizeof( stMouseXY ) );
                break;

                // ���콺 �̵�  ����
            case EVENT_MOUSE_MOVE:
                // ���콺 ��ġ ����
                memcpy( &stMouseXY, &( stEvent.stMouseEvent.stPoint ),
                        sizeof( stMouseXY ) );
                break;

                // ������ �ݱ� ��ư ó��
            case EVENT_WINDOW_CLOSE:
                // �����츦 �����ϰ� �޸𸮸� ����
                DeleteWindow( qwWindowID );
                free( g_stGameInfo.pstBubbleBuffer );
                return 0;
                break;
            }
        }

        //----------------------------------------------------------------------
        // ���� ���� ó�� �κ�
        //----------------------------------------------------------------------
        // ������ ���� �Ǿ��ٸ� 50ms���� ������ ������� �Ʒ��� �̵�
        if( ( g_stGameInfo.bGameStart == TRUE ) &&
            ( ( GetTickCount() - qwLastTickCount ) > 50 ) )
        {
            qwLastTickCount = GetTickCount();

            // ������� ����
            if( ( rand() % 7 ) == 1 )
            {
                CreateBubble();
            }

            // ������� �̵�
            MoveBubble();

            // ���� ������ ǥ��
            DrawGameArea( qwWindowID, &stMouseXY );

            // ���� ������ ǥ��
            DrawInformation( qwWindowID );

            // �÷��̾��� ������ 0�̶�� ���� ����
            if( g_stGameInfo.iLife <= 0 )
            {
                g_stGameInfo.bGameStart = FALSE;

                // ���� ���� �޽����� ���
                DrawText( qwWindowID, 80, 130, RGB( 255, 255, 255 ), RGB( 0, 0, 0 ),
                        "Game Over~!!!", 13 );
                DrawText( qwWindowID, 5, 150, RGB( 255, 255, 255 ), RGB( 0, 0, 0 ),
                        pcStartMessage, strlen( pcStartMessage ) );
            }

            // ����� �������� ���θ� ȭ�鿡 ������Ʈ
            ShowWindow( qwWindowID, TRUE );
        }
        else
        {
            Sleep( 0 );
        }
    }

    return 0;
}

/**
 *  ���ӿ� ���õ� ������ �ʱ�ȭ
 */
BOOL Initialize( void )
{
    // ������� �ִ� ������ŭ �޸𸮸� �Ҵ�
    if( g_stGameInfo.pstBubbleBuffer == NULL )
    {
        g_stGameInfo.pstBubbleBuffer = malloc( sizeof( BUBBLE ) * MAXBUBBLECOUNT );
        if( g_stGameInfo.pstBubbleBuffer == NULL )
        {
            printf( "Memory allocate fail\n" );
            return FALSE;
        }
    }

    // ������� ������ �ʱ�ȭ
    memset( g_stGameInfo.pstBubbleBuffer, 0, sizeof( BUBBLE ) *
            MAXBUBBLECOUNT );
    g_stGameInfo.iAliveBubbleCount = 0;

    // ������ ���۵Ǿ��ٴ� ������ ����, �׸��� ���� ����
    g_stGameInfo.bGameStart = FALSE;
    g_stGameInfo.qwScore = 0;
    g_stGameInfo.iLife = MAXLIFE;

    return TRUE;
}

/**
 *  ������� ����
 */
BOOL CreateBubble( void )
{
    BUBBLE* pstTarget;
    int i;

    // ������� �ִ� ������ ����ִ� ������� ������ ���Ͽ� �������� ���θ� ����
    if( g_stGameInfo.iAliveBubbleCount >= MAXBUBBLECOUNT )
    {
        return FALSE;
    }

    // �� ����� �ڷᱸ���� �˻�
    for( i = 0 ; i < MAXBUBBLECOUNT ; i++ )
    {
        // ������� ������� ������ �ٽ� �Ҵ��ؼ� ���
        if( g_stGameInfo.pstBubbleBuffer[ i ].bAlive == FALSE )
        {
            // ���õ� ����� �ڷᱸ��
            pstTarget = &( g_stGameInfo.pstBubbleBuffer[ i ] );

            // ������� ��� �ִٰ� �����ϰ� ������� �̵� �ӵ��� �ʱ�ȭ
            pstTarget->bAlive = TRUE;
            pstTarget->qwSpeed = ( rand() % 8 ) + DEFAULTSPEED;

            // X��ǥ�� Y��ǥ�� ������� ���� ���� ���ο� ��ġ�ϵ��� ����
            pstTarget->qwX = rand() % ( WINDOW_WIDTH - 2 * RADIUS ) + RADIUS;
            pstTarget->qwY = INFORMATION_HEIGHT + WINDOW_TITLEBAR_HEIGHT + RADIUS + 1;

            // ������� ���� ����
            pstTarget->stColor = RGB( rand() % 256, rand() % 256, rand() % 256 );

            // ����ִ� ������� ���� ����
            g_stGameInfo.iAliveBubbleCount++;
            return TRUE;
        }
    }

    return FALSE;
}

/**
 *  ������� �̵�
 */
void MoveBubble( void )
{
    BUBBLE* pstTarget;
    int i;

    // ����ִ� ��� ������� �̵�
    for( i = 0 ; i < MAXBUBBLECOUNT ; i++ )
    {
        // ������� ��������� �̵�
        if( g_stGameInfo.pstBubbleBuffer[ i ].bAlive == TRUE )
        {
            // ���� ����� �ڷᱸ��
            pstTarget = &( g_stGameInfo.pstBubbleBuffer[ i ] );

            // ������� Y��ǥ�� �̵� �ӵ��� ����
            pstTarget->qwY += pstTarget->qwSpeed;

            // ���� ���� ���� ������ ������� �����ϰ� �÷��̾ ������� ����
            // �������Ƿ� ������ �ϳ� ����
            if( ( pstTarget->qwY + RADIUS ) >= WINDOW_HEIGHT )
            {
                pstTarget->bAlive = FALSE;

                // ��� �ִ� ������� ���� ���̰� ���� �ϳ� ����
                g_stGameInfo.iAliveBubbleCount--;
                if( g_stGameInfo.iLife > 0 )
                {
                    g_stGameInfo.iLife--;
                }
            }
        }
    }
}

/**
 *  ���콺 �Ʒ��� �ִ� ������� �����ϰ� ������ ����
 */
void DeleteBubbleUnderMouse( POINT* pstMouseXY )
{
    BUBBLE* pstTarget;
    int i;
    QWORD qwDistance;

    // ����ִ� ��� ������� �˻��Ͽ� ���콺 �Ʒ��� �ִ� ������� ����
    for( i = MAXBUBBLECOUNT - 1 ; i >= 0 ; i-- )
    {
        // ������� ��������� �Ÿ��� ����ؼ� ���� ���θ� ����
        if( g_stGameInfo.pstBubbleBuffer[ i ].bAlive == TRUE )
        {
            // ���� ����� �ڷᱸ��
            pstTarget = &( g_stGameInfo.pstBubbleBuffer[ i ] );

            // ���콺�� Ŭ���� ��ġ�� ���� �߽��� ������ �Ÿ� �����̸� ����
            qwDistance = ( ( pstMouseXY->iX - pstTarget->qwX ) *
                           ( pstMouseXY->iX - pstTarget->qwX ) ) +
                         ( ( pstMouseXY->iY - pstTarget->qwY ) *
                           ( pstMouseXY->iY - pstTarget->qwY ) );
            // ������� �߽ɰ� ���콺 Ŭ�� ��ġ ������ �Ÿ��� ����� ��������
            // ���Ͽ� �۴ٸ� ����� ���ο� Ŭ���� ���̹Ƿ� ����
            if( qwDistance < ( RADIUS * RADIUS ) )
            {
                pstTarget->bAlive = FALSE;

                // ����ִ� ������� ���� ���̰� ������ ����
                g_stGameInfo.iAliveBubbleCount--;
                g_stGameInfo.qwScore++;
                break;
            }
        }
    }
}

/**
 *  ���� ������ ȭ�鿡 ���
 */
void DrawInformation( QWORD qwWindowID )
{
    char vcBuffer[ 200 ];
    int iLength;

    // ���� ���� ������ ǥ��
    DrawRect( qwWindowID, 1, WINDOW_TITLEBAR_HEIGHT - 1, WINDOW_WIDTH - 2,
              WINDOW_TITLEBAR_HEIGHT + INFORMATION_HEIGHT, RGB( 55, 215, 47 ), TRUE );

    // �ӽ� ���ۿ� ����� ������ ����
    sprintf( vcBuffer, "Life: %d, Score: %d\n", g_stGameInfo.iLife,
            g_stGameInfo.qwScore );
    iLength = strlen( vcBuffer );

    // ����� ������ ���� ���� ǥ�� ������ ����� ���
    DrawText( qwWindowID, ( WINDOW_WIDTH - iLength * FONT_ENGLISHWIDTH ) / 2,
            WINDOW_TITLEBAR_HEIGHT + 2, RGB( 255, 255, 255 ), RGB( 55, 215, 47 ),
            vcBuffer, strlen( vcBuffer ) );
}

/**
 *  ���� ������ ������� ǥ��
 */
void DrawGameArea( QWORD qwWindowID, POINT* pstMouseXY )
{
    BUBBLE* pstTarget;
    int i;

    // ���� ������ ����� �ʱ�ȭ
    DrawRect( qwWindowID, 0, WINDOW_TITLEBAR_HEIGHT + INFORMATION_HEIGHT,
            WINDOW_WIDTH - 1, WINDOW_HEIGHT - 1, RGB( 0, 0, 0 ), TRUE );

    // ����ִ� ��� ������� ǥ��
    for( i = 0 ; i < MAXBUBBLECOUNT ; i++ )
    {
        // ������� ��������� ȭ�鿡 ǥ��
        if( g_stGameInfo.pstBubbleBuffer[ i ].bAlive == TRUE )
        {
            // ���� ����� �ڷᱸ��
            pstTarget = &( g_stGameInfo.pstBubbleBuffer[ i ] );

            // ������� ���ο� �ܺθ� �׸�
            DrawCircle( qwWindowID, pstTarget->qwX, pstTarget->qwY, RADIUS,
                    pstTarget->stColor, TRUE );
            DrawCircle( qwWindowID, pstTarget->qwX, pstTarget->qwY, RADIUS,
                    ~pstTarget->stColor, FALSE );
        }
    }

    // ���콺�� �ִ� ��ġ�� �˻��Ͽ� ���ؼ� ǥ��
    if( pstMouseXY->iY < ( WINDOW_TITLEBAR_HEIGHT + RADIUS ) )
    {
        pstMouseXY->iY = WINDOW_TITLEBAR_HEIGHT + RADIUS;
    }

    // ���ؼ��� +�� ǥ��
    DrawLine( qwWindowID, pstMouseXY->iX, pstMouseXY->iY - RADIUS,
              pstMouseXY->iX, pstMouseXY->iY + RADIUS, RGB( 255, 0, 0 ) );
    DrawLine( qwWindowID, pstMouseXY->iX - RADIUS, pstMouseXY->iY,
              pstMouseXY->iX + RADIUS, pstMouseXY->iY, RGB( 255, 0, 0 ) );


    // ���� ������ �׵θ��� ǥ��
    DrawRect( qwWindowID, 0, WINDOW_TITLEBAR_HEIGHT  + INFORMATION_HEIGHT,
            WINDOW_WIDTH - 1, WINDOW_HEIGHT - 1, RGB( 0, 255, 0 ), FALSE );
}
