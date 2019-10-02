/**
 *  file    Main.c
 *  date    2010/03/10
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
    KEYEVENT *pstKeyEvent;
    QWORD qwLastTickCount;
    char* pcStartMessage = "Please LButton Down To Start~!";
    RECT stScreenArea;
    int iX;
    int iY;
    BYTE bBlockKind;
    
    //--------------------------------------------------------------------------
    // �����츦 ȭ�� ����� ����
    //--------------------------------------------------------------------------
    GetScreenArea( &stScreenArea );
    iX = ( GetRectangleWidth( &stScreenArea ) - WINDOW_WIDTH ) / 2;
    iY = ( GetRectangleHeight( &stScreenArea ) - WINDOW_HEIGHT ) / 2;
    qwWindowID = CreateWindow( iX, iY, WINDOW_WIDTH, WINDOW_HEIGHT,
                               WINDOW_FLAGS_DEFAULT, "Hexa" );
    if( qwWindowID == WINDOW_INVALIDID )
    {
        printf( "Window create fail\n" );
        return -1;
    }

    //--------------------------------------------------------------------------
    // ���ӿ� ���õ� ������ �ʱ�ȭ�ϰ� ����� ���۸� �Ҵ�
    //--------------------------------------------------------------------------
    // ���� ������ �ʱ�ȭ
    Initialize();

    // ���� �ʱ갪(Random Seed) ����
    srand( GetTickCount() );

    //--------------------------------------------------------------------------
    // ���� ������ ���� ������ ����ϰ� ���� ���� ��� �޽����� ǥ��
    //--------------------------------------------------------------------------
    DrawInformation( qwWindowID );
    DrawGameArea( qwWindowID );
    DrawText( qwWindowID, 7, 200, RGB( 255, 255, 255 ), RGB( 0, 0, 0 ),
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
                break;

                // Ű���� ���� ó��
            case EVENT_KEY_DOWN:
                pstKeyEvent = &( stEvent.stKeyEvent );
                if( g_stGameInfo.bGameStart == FALSE )
                {
                    break;
                }

                switch( pstKeyEvent->bASCIICode )
                {
                    // �������� �̵�
                case KEY_LEFT:
                    if( IsMovePossible( g_stGameInfo.iBlockX - 1,
                                        g_stGameInfo.iBlockY ) == TRUE )
                    {
                        g_stGameInfo.iBlockX -= 1;
                        DrawGameArea( qwWindowID );
                    }
                    break;

                    // ���������� �̵�
                case KEY_RIGHT:
                    if( IsMovePossible( g_stGameInfo.iBlockX + 1,
                                        g_stGameInfo.iBlockY ) == TRUE )
                    {
                        g_stGameInfo.iBlockX += 1;
                        DrawGameArea( qwWindowID );
                    }
                    break;

                    // �����̴� ����� �����ϴ� ���� ����� ������ ����
                case KEY_UP:
                    bBlockKind = g_stGameInfo.vbBlock[ 0 ];
                    memcpy( &( g_stGameInfo.vbBlock ), &( g_stGameInfo.vbBlock[ 1 ] ),
                            BLOCKCOUNT - 1 );
                    g_stGameInfo.vbBlock[ BLOCKCOUNT - 1 ] = bBlockKind;

                    DrawGameArea( qwWindowID );
                    break;

                    // ����� �Ʒ��� �̵�
                case KEY_DOWN:
                    if( IsMovePossible( g_stGameInfo.iBlockX,
                                        g_stGameInfo.iBlockY + 1 ) == TRUE )
                    {
                        g_stGameInfo.iBlockY += 1;
                    }
                    DrawGameArea( qwWindowID );
                    break;

                    // ����� �Ʒ��� ������ �̵�
                case ' ':
                    while( IsMovePossible( g_stGameInfo.iBlockX,
                                           g_stGameInfo.iBlockY + 1 ) == TRUE )
                    {
                        g_stGameInfo.iBlockY += 1;
                    }
                    DrawGameArea( qwWindowID );
                    break;
                }

                // ����� ������ ȭ�鿡 ǥ��
                ShowWindow( qwWindowID, TRUE );
                break;

                // ������ �ݱ� ��ư ó��
            case EVENT_WINDOW_CLOSE:
                // �����츦 ����
                DeleteWindow( qwWindowID );
                return 0;
                break;
            }
        }

        //----------------------------------------------------------------------
        // ���� ���� ó�� �κ�
        //----------------------------------------------------------------------
        // ������ ���� �Ǿ��ٸ� ������ ���� ���� �ð� ����� �ڿ� ����� �Ʒ��� �̵�
        if( ( g_stGameInfo.bGameStart == TRUE ) &&
            ( ( GetTickCount() - qwLastTickCount ) >
              ( 300 - ( g_stGameInfo.qwLevel * 10 ) ) ) )
        {
            qwLastTickCount = GetTickCount();

            // ����� �� ĭ �Ʒ��� ������ �� �̻� ���� �� ���ٸ� ����� ����
            if( IsMovePossible( g_stGameInfo.iBlockX, g_stGameInfo.iBlockY + 1 ) ==
                    FALSE )
            {
                // ��� ������ �� ������ ���� ����
                if( FreezeBlock( g_stGameInfo.iBlockX, g_stGameInfo.iBlockY ) ==
                        FALSE )
                {
                    g_stGameInfo.bGameStart = FALSE;

                    // ���� ���� �޽����� ���
                    DrawText( qwWindowID, 82, 230, RGB( 255, 255, 255 ), RGB( 0, 0, 0 ),
                            "Game Over~!!!", 13 );
                    DrawText( qwWindowID, 7, 250, RGB( 255, 255, 255 ), RGB( 0, 0, 0 ),
                            pcStartMessage, strlen( pcStartMessage ) );
                }

                // ���忡 ����� �˻��Ͽ� 3�� �̻� ���ӵ� ����� �����ϰ� ȭ�鿡 ǥ��
                EraseAllContinuousBlockOnBoard( qwWindowID );

                // ���ο� ����� ����
                CreateBlock();
            }
            else
            {
                g_stGameInfo.iBlockY++;

                // ���� ������ ���� �׸�
                DrawGameArea( qwWindowID );
            }

            // ����� ������ ȭ�鿡 ǥ��
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
 *  ���� ������ �ʱ�ȭ
 */
void Initialize( void )
{
    // ���� ���� �ڷᱸ�� ��ü�� �ʱ�ȭ
    memset( &g_stGameInfo, 0, sizeof( g_stGameInfo ) );

    // ����� ������ ����
    g_stGameInfo.vstBlockColor[ 1 ] = RGB( 230, 0, 0 );
    g_stGameInfo.vstBlockColor[ 2 ] = RGB( 0, 230, 0 );
    g_stGameInfo.vstBlockColor[ 3 ] = RGB( 230, 0, 230 );
    g_stGameInfo.vstBlockColor[ 4 ] = RGB( 230, 230, 0 );
    g_stGameInfo.vstBlockColor[ 5 ] = RGB( 0, 230, 230 );
    g_stGameInfo.vstEdgeColor[ 1 ] = RGB( 150, 0, 0 );
    g_stGameInfo.vstEdgeColor[ 2 ] = RGB( 0, 150, 0 );
    g_stGameInfo.vstEdgeColor[ 3 ] = RGB( 150, 0, 150 );
    g_stGameInfo.vstEdgeColor[ 4 ] = RGB( 150, 150, 0 );
    g_stGameInfo.vstEdgeColor[ 5 ] = RGB( 0, 150, 150 );

    // �����̴� ����� ��ġ�� ����
    g_stGameInfo.iBlockX = -1;
    g_stGameInfo.iBlockX = -1;
}

/**
 *  �����̴� ����� ����
 */
void CreateBlock( void )
{
    int i;

    // ����� ���� ��ǥ�� ���� ������� ���� �Ʒ� ��Ϻ��� ȭ�鿡 ǥ�õǵ��� ��
    g_stGameInfo.iBlockX = BOARDWIDTH / 2;
    g_stGameInfo.iBlockY = -BLOCKCOUNT;

    // �����̴� ����� �����ϴ� ���� ����� ������ ����
    for( i = 0 ; i < BLOCKCOUNT ; i++ )
    {
        g_stGameInfo.vbBlock[ i ] = ( rand() % BLOCKKIND ) + 1;
    }
}

/**
 *  �����̴� ����� Ư�� ��ġ�� �ű� �� �ִ��� Ȯ��
 */
BOOL IsMovePossible( int iBlockX, int iBlockY )
{
    // ����� ��ǥ�� ���� ���� �ȿ� �ִ����� Ȯ��
    if( ( iBlockX < 0 ) || ( iBlockX >= BOARDWIDTH ) ||
        ( ( iBlockY + BLOCKCOUNT ) > BOARDHEIGHT ) )
    {
        return FALSE;
    }

    // ������ ����� ��ġ�� ���� Ȯ���Ͽ� �����ǿ� ������ ������ ������� ������ ����
    if( g_stGameInfo.vvbBoard[ iBlockY + BLOCKCOUNT - 1 ][ iBlockX ] != EMPTYBLOCK )
    {
        return FALSE;
    }

    return TRUE;
}

/**
 *  ����� �����ǿ� ����
 */
BOOL FreezeBlock( int iBlockX, int iBlockY )
{
    int i;

    // ����� �����ϴ� ��ġ�� 0���� ������ ���� ��ġ�� ����� ���� á�ٴ� ���̹Ƿ� ����
    if( iBlockY < 0 )
    {
        return FALSE;
    }

    // �����̴� ����� ���� ��ǥ �״�� �����ǿ� ����
    for( i = 0 ; i < BLOCKCOUNT ; i++ )
    {
        g_stGameInfo.vvbBoard[ iBlockY + i ][ iBlockX ] = g_stGameInfo.vbBlock[ i ];
    }

    // ����� �����Ǿ����Ƿ� ����� X�� ��ǥ�� -1�� ����
    g_stGameInfo.iBlockX = -1;
    return TRUE;
}

/**
 * ���� �������� ��ġ�ϴ� ����� ã�Ƽ� ǥ��
 */
BOOL MarkContinuousHorizonBlockOnBoard( void )
{
    int iMatchCount;
    BYTE bBlockKind;
    int i;
    int j;
    int k;
    BOOL bMarked;

    bMarked = FALSE;

    // ������ ��ü�� �˻��Ͽ� ���� �������� 3�� �̻��� ���� ã�Ƽ� ǥ��
    for( j = 0 ; j < BOARDHEIGHT ; j++ )
    {
        iMatchCount = 0;
        bBlockKind = 0xFF;

        for( i = 0 ; i < BOARDWIDTH ; i++ )
        {
            // ù ��°�̸� ��� ������ ����
            if( ( iMatchCount == 0 ) &&
                ( g_stGameInfo.vvbBoard[ j ][ i ] != EMPTYBLOCK ) )
            {
                bBlockKind = g_stGameInfo.vvbBoard[ j ][ i ];
                iMatchCount++;
            }
            else
            {
                // ������ ��ġ�ϸ� ��ġ�� ����� ���� ����
                if( g_stGameInfo.vvbBoard[ j ][ i ] == bBlockKind )
                {
                    iMatchCount++;
                    // ���ӵ� ����� 3���̸� ��Ƶ� ���� 3����� ���� ������� ǥ��
                    if( iMatchCount == BLOCKCOUNT )
                    {
                        for( k = 0 ; k < iMatchCount ; k++ )
                        {
                            g_stGameInfo.vvbEraseBlock[ j ][ i - k ] = ERASEBLOCK;
                        }

                        // ǥ�õ� ���� �ִ� ������ ����
                        bMarked = TRUE;
                    }
                    // ���ӵ� ����� 4�� �̻��̸� ��� ���� ������� ǥ��
                    else if( iMatchCount > BLOCKCOUNT )
                    {
                        g_stGameInfo.vvbEraseBlock[ j ][ i ] = ERASEBLOCK;
                    }
                }
                // ��ġ���� ������ ���ο� ������� ����
                else
                {
                    if( g_stGameInfo.vvbBoard[ j ][ i ] != EMPTYBLOCK )
                    {
                        // ���ο� ������� ����
                        iMatchCount = 1;
                        bBlockKind = g_stGameInfo.vvbBoard[ j ][ i ];
                    }
                    else
                    {
                        iMatchCount = 0;
                        bBlockKind = 0xFF;
                    }
                }
            }
        }
    }

    return bMarked;
}

/**
 * ���� �������� ��ġ�ϴ� ����� ã�Ƽ� ǥ��
 */
BOOL MarkContinuousVerticalBlockOnBoard( void )
{
    int iMatchCount;
    BYTE bBlockKind;
    int i;
    int j;
    int k;
    BOOL bMarked;

    bMarked = FALSE;
    // ������ ��ü�� �˻��Ͽ� ���� �������� 3�� �̻��� ���� ã�Ƽ� ǥ��
    for( i = 0 ; i < BOARDWIDTH ; i++ )
    {
        iMatchCount = 0;
        bBlockKind = 0xFF;

        for( j = 0 ; j < BOARDHEIGHT ; j++ )
        {
            // ù ��°�̸� ��� ������ ����
            if( ( iMatchCount == 0 ) &&
                ( g_stGameInfo.vvbBoard[ j ][ i ] != EMPTYBLOCK ) )
            {
                bBlockKind = g_stGameInfo.vvbBoard[ j ][ i ];
                iMatchCount++;
            }
            else
            {
                // ������ ��ġ�ϸ� ��ġ�� ����� ���� ����
                if( g_stGameInfo.vvbBoard[ j ][ i ] == bBlockKind )
                {
                    iMatchCount++;
                    // ���ӵ� ����� 3���̸� ��Ƶ� ���� 3����� ���� ������� ǥ��
                    if( iMatchCount == BLOCKCOUNT )
                    {
                        for( k = 0 ; k < iMatchCount ; k++ )
                        {
                            g_stGameInfo.vvbEraseBlock[ j - k ][ i ] = ERASEBLOCK;
                        }

                        bMarked = TRUE;
                    }
                    // ���ӵ� ����� 4�� �̻��̸� ��� ���� ������� ǥ��
                    else if( iMatchCount > BLOCKCOUNT )
                    {
                        g_stGameInfo.vvbEraseBlock[ j ][ i ] = ERASEBLOCK;
                    }
                }
                // ��ġ���� ������ ���ο� ������� ����
                else
                {
                    if( g_stGameInfo.vvbBoard[ j ][ i ] != EMPTYBLOCK )
                    {
                        // ���ο� ������� ����
                        iMatchCount = 1;
                        bBlockKind = g_stGameInfo.vvbBoard[ j ][ i ];
                    }
                    else
                    {
                        iMatchCount = 0;
                        bBlockKind = 0xFF;
                    }
                }
            }
        }
    }

    return bMarked;
}

/**
 * �밢�� �������� ��ġ�ϴ� ����� ã�Ƽ� ǥ��
 */
BOOL MarkContinuousDiagonalBlockInBoard( void )
{
    int iMatchCount;
    BYTE bBlockKind;
    int i;
    int j;
    int k;
    int l;
    BOOL bMarked;

    bMarked = FALSE;

    //--------------------------------------------------------------------------
    // ������ ��ü�� �˻��Ͽ� ������ �Ʒ��� �밢�� �������� 3�� �̻��� ����
    // ã�Ƽ� ǥ��
    //--------------------------------------------------------------------------
    for( i = 0 ; i < BOARDWIDTH ; i++ )
    {
        for( j = 0 ; j < BOARDHEIGHT ; j++ )
        {
            iMatchCount = 0;
            bBlockKind = 0xFF;

            for( k = 0 ; ( ( i + k ) < BOARDWIDTH ) && ( ( j + k ) < BOARDHEIGHT ) ; k++ )
            {
                // ù ��°�̸� ��� ������ ����
                if( ( iMatchCount == 0 ) &&
                    ( g_stGameInfo.vvbBoard[ j + k ][ i + k ] != EMPTYBLOCK ) )
                {
                    bBlockKind = g_stGameInfo.vvbBoard[ j + k ][ i + k ];
                    iMatchCount++;
                }
                else
                {
                    // ������ ��ġ�ϸ� ��ġ�� ����� ���� ����
                    if( g_stGameInfo.vvbBoard[ j + k ][ i + k ] == bBlockKind )
                    {
                        iMatchCount++;
                        // ���ӵ� ����� 3���̸� ��Ƶ� ���� 3����� ���� ������� ǥ��
                        if( iMatchCount == BLOCKCOUNT )
                        {
                            for( l = 0 ; l < iMatchCount ; l++ )
                            {
                                g_stGameInfo.vvbEraseBlock[ j + k - l ][ i + k - l] =
                                        ERASEBLOCK;
                            }
                            bMarked = TRUE;
                        }
                        // ���ӵ� ����� 4�� �̻��̸� ��� ���� ������� ǥ��
                        else if( iMatchCount > BLOCKCOUNT )
                        {
                            g_stGameInfo.vvbEraseBlock[ j + k ][ i + k ] = ERASEBLOCK;
                        }
                    }
                    // ��ġ���� ������ ���ο� ������� ����
                    else
                    {
                        if( g_stGameInfo.vvbBoard[ j + k ][ i + k ] != EMPTYBLOCK )
                        {
                            // ���ο� ������� ����
                            iMatchCount = 1;
                            bBlockKind = g_stGameInfo.vvbBoard[ j + k ][ i + k ];
                        }
                        else
                        {
                            iMatchCount = 0;
                            bBlockKind = 0xFF;
                        }
                    }
                }
            }
        }
    }

    //--------------------------------------------------------------------------
    // ������ ��ü�� �˻��Ͽ� �Ʒ����� ���� �밢�� �������� 3�� �̻��� ����
    // ã�Ƽ� ǥ��
    //--------------------------------------------------------------------------
    for( i = 0 ; i < BOARDWIDTH ; i++ )
    {
        for( j = 0 ; j < BOARDHEIGHT ; j++ )
        {
            iMatchCount = 0;
            bBlockKind = 0xFF;

            for( k = 0 ; ( ( i + k ) < BOARDWIDTH ) && ( ( j - k ) >= 0 ) ; k++ )
            {
                // ù ��°�̸� ��� ������ ����
                if( ( iMatchCount == 0 ) &&
                    ( g_stGameInfo.vvbBoard[ j - k ][ i + k ] != EMPTYBLOCK ) )
                {
                    bBlockKind = g_stGameInfo.vvbBoard[ j - k ][ i + k ];
                    iMatchCount++;
                }
                else
                {
                    // ������ ��ġ�ϸ� ��ġ�� ����� ���� ����
                    if( g_stGameInfo.vvbBoard[ j - k ][ i + k ] == bBlockKind )
                    {
                        iMatchCount++;
                        // ���ӵ� ����� 3���̸� ��Ƶ� ���� 3����� ���� ������� ǥ��
                        if( iMatchCount == BLOCKCOUNT )
                        {
                            for( l = 0 ; l < iMatchCount ; l++ )
                            {
                                g_stGameInfo.vvbEraseBlock[ j - k + l ][ i + k - l ] =
                                        ERASEBLOCK;
                            }
                            bMarked = TRUE;
                        }
                        // ���ӵ� ����� 4�� �̻��̸� ��� ���� ������� ǥ��
                        else if( iMatchCount > BLOCKCOUNT )
                        {
                            g_stGameInfo.vvbEraseBlock[ j - k ][ i + k ] = ERASEBLOCK;
                        }
                    }
                    // ��ġ���� ������ ���ο� ������� ����
                    else
                    {
                        if( g_stGameInfo.vvbBoard[ j - k ][ i + k ] != EMPTYBLOCK )
                        {
                            // ���ο� ������� ����
                            iMatchCount = 1;
                            bBlockKind = g_stGameInfo.vvbBoard[ j - k ][ i + k ];
                        }
                        else
                        {
                            iMatchCount = 0;
                            bBlockKind = 0xFF;
                        }
                    }
                }
            }
        }
    }

    return bMarked;
}

/**
 *  ������ ������� ǥ�õ� ����� �����ǿ��� �����ϰ� ������ ����
 */
void EraseMarkedBlock( void )
{
    int i;
    int j;

    for( j = 0 ; j < BOARDHEIGHT ; j++ )
    {
        for( i = 0 ; i < BOARDWIDTH ; i++ )
        {
            // ���� ����̸� �����ǿ��� ����
            if( g_stGameInfo.vvbEraseBlock[ j ][ i ] == ERASEBLOCK )
            {
                // �����ǿ� ����� �� ������ ����
                g_stGameInfo.vvbBoard[ j ][ i ] = EMPTYBLOCK;

                // ������ ����
                g_stGameInfo.qwScore++;
            }
        }
    }
}

/**
 *  �� ������ ����� �Ʒ��� �̵�
 */
void CompactBlockOnBoard( void )
{
    int i;
    int j;
    int iEmptyPosition;

    // �������� ��� ������ ���鼭 �� ������ ����� ä��
    for( i = 0 ; i < BOARDWIDTH ; i++ )
    {
        iEmptyPosition = -1;

        // �Ʒ����� ���� �ö󰡸鼭 �� ������ ã�� ����� ä��
        for( j = BOARDHEIGHT - 1 ; j >= 0 ; j-- )
        {
            // �� ����̸� ���� ��ġ�� �����صξ��ٰ� ����� �ű� �� ���
            if( ( iEmptyPosition == -1 ) &&
                ( g_stGameInfo.vvbBoard[ j ][ i ] == EMPTYBLOCK ) )
            {
                iEmptyPosition = j;
            }
            // �߰��� �� ����� ����Ǿ��� ���� ��ġ�� ����� ������ �Ʒ��� �̵�
            else if( ( iEmptyPosition != -1 ) &&
                     ( g_stGameInfo.vvbBoard[ j ][ i ] != EMPTYBLOCK ) )
            {
                g_stGameInfo.vvbBoard[ iEmptyPosition ][ i ] =
                        g_stGameInfo.vvbBoard[ j ][ i ];

                // �� ����� Y��ǥ�� ���� �� ĭ �÷��� ��� �׾� �ø� �� �ֵ��� ��
                iEmptyPosition--;

                // ���� ��ġ�� ����� �Ű������Ƿ� �� ������� ����
                g_stGameInfo.vvbBoard[ j ][ i ] = EMPTYBLOCK;
            }
        }
    }
}


/**
 *  ���̻� ������ ����� ���� ������ �ݺ��Ͽ� �������� ����� �����ϰ� ����
 */
void EraseAllContinuousBlockOnBoard( QWORD qwWindowID )
{
    BOOL bMarked;

    // ���� ���� �ڷᱸ���� �ִ� ������ ��� �ʵ带 �ʱ�ȭ
    memset( g_stGameInfo.vvbEraseBlock, 0, sizeof( g_stGameInfo.vvbEraseBlock ) );

    while( 1 )
    {
        // ����� �����ϱ� ���� ��� ����Ͽ� ���� �������� ���¿� ����� ���¸� ����
        Sleep( 300 );

        bMarked = FALSE;

        // ���� �������� ������ ����� ǥ��
        bMarked |= MarkContinuousHorizonBlockOnBoard();
        // ���� �������� ������ ����� ǥ��
        bMarked |= MarkContinuousVerticalBlockOnBoard();
        // �밢�� �������� ������ ����� ǥ��
        bMarked |= MarkContinuousDiagonalBlockInBoard();

        // ������ ����� ������ �� �̻� ������ �ʿ䰡 ����
        if( bMarked == FALSE )
        {
            break;
        }

        // ǥ�õ� ����� ����
        EraseMarkedBlock();

        // ����� ������ �Ʒ��� �̵����� �� ������ ä��
        CompactBlockOnBoard();

        // ������ ������ 30�� ������ ����
        g_stGameInfo.qwLevel = ( g_stGameInfo.qwScore / 30 ) + 1;

        // ���� ����� ������ ���� ���� ������ ���� ������ �ٽ� �׸�
        DrawGameArea( qwWindowID );
        DrawInformation( qwWindowID );

        // �����츦 ȭ�鿡 ǥ��
        ShowWindow( qwWindowID, TRUE );
    }
}

/**
 *  ���� ������ ȭ�鿡 ���
 */
void DrawInformation( QWORD qwWindowID )
{
    char vcBuffer[ 200 ];
    int iLength;

    // ���� ���� ������ ����� ���
    DrawRect( qwWindowID, 1, WINDOW_TITLEBAR_HEIGHT - 1, WINDOW_WIDTH - 2,
              WINDOW_TITLEBAR_HEIGHT + INFORMATION_HEIGHT, RGB( 55, 215, 47 ), TRUE );

    // �ӽ� ���ۿ� ����� ������ ����
    sprintf( vcBuffer, "Level: %d, Score: %d\n", g_stGameInfo.qwLevel,
            g_stGameInfo.qwScore );
    iLength = strlen( vcBuffer );

    // ����� ������ ���� ���� ǥ�� ������ ����� ���
    DrawText( qwWindowID, ( WINDOW_WIDTH - iLength * FONT_ENGLISHWIDTH ) / 2,
            WINDOW_TITLEBAR_HEIGHT + 2, RGB( 255, 255, 255 ), RGB( 55, 215, 47 ),
            vcBuffer, strlen( vcBuffer ) );
}

/**
 *  ���� ������ ȭ�鿡 ���
 */
void DrawGameArea( QWORD qwWindowID )
{
    COLOR stColor;
    int i;
    int j;
    int iY;

    // ���� ������ ���۵Ǵ� ��ġ
    iY = WINDOW_TITLEBAR_HEIGHT + INFORMATION_HEIGHT;

    // ���� ������ ����� ���
    DrawRect( qwWindowID, 0, iY, BLOCKSIZE * BOARDWIDTH, iY + BLOCKSIZE * BOARDHEIGHT,
            RGB( 0, 0, 0 ), TRUE );

    // �������� ������ ȭ�鿡 ǥ��
    for( j = 0 ; j < BOARDHEIGHT ; j++ )
    {
        for( i = 0 ; i < BOARDWIDTH ; i++ )
        {
            // �� ����� �ƴϸ� ����� ǥ����
            if( g_stGameInfo.vvbBoard[ j ][ i ] != EMPTYBLOCK )
            {
                // ����� ���θ� �׸�
                stColor = g_stGameInfo.vstBlockColor[ g_stGameInfo.vvbBoard[ j ][ i ] ];
                DrawRect( qwWindowID, i * BLOCKSIZE, iY + ( j * BLOCKSIZE ),
                        ( i + 1 ) * BLOCKSIZE, iY + ( ( j + 1 ) * BLOCKSIZE ),
                        stColor, TRUE );

                // ����� �ܺ� �׵θ��� �׸�
                stColor = g_stGameInfo.vstEdgeColor[ g_stGameInfo.vvbBoard[ j ][ i ] ];
                DrawRect( qwWindowID, i * BLOCKSIZE, iY + ( j * BLOCKSIZE ),
                        ( i + 1 ) * BLOCKSIZE, iY + ( ( j + 1 ) * BLOCKSIZE ),
                        stColor, FALSE );
                stColor = g_stGameInfo.vstEdgeColor[ g_stGameInfo.vvbBoard[ j ][ i ] ];
                DrawRect( qwWindowID, i * BLOCKSIZE + 1, iY + ( j * BLOCKSIZE ) + 1,
                        ( i + 1 ) * BLOCKSIZE - 1, iY + ( ( j + 1 ) * BLOCKSIZE ) - 1,
                        stColor, FALSE );
            }
        }
    }

    // ���� �����̴� ����� ȭ�鿡 ǥ��
    if( g_stGameInfo.iBlockX != -1 )
    {
        for( i = 0 ; i < BLOCKCOUNT ; i++ )
        {
            // ���� ǥ���� �Ʒ��� ����� ǥ�õ� ���� ǥ��
            if( WINDOW_TITLEBAR_HEIGHT <
                    ( iY + ( ( g_stGameInfo.iBlockY + i ) * BLOCKSIZE ) ) )
            {
                // ����� ���θ� �׸�
                stColor = g_stGameInfo.vstBlockColor[ g_stGameInfo.vbBlock[ i ] ];
                DrawRect( qwWindowID, g_stGameInfo.iBlockX * BLOCKSIZE,
                    iY + ( ( g_stGameInfo.iBlockY + i ) * BLOCKSIZE ),
                    ( g_stGameInfo.iBlockX + 1 ) * BLOCKSIZE,
                    iY + ( ( g_stGameInfo.iBlockY + i + 1 ) * BLOCKSIZE ),
                    stColor, TRUE );

                // ����� �ܺ� �׵θ��� �׸�
                stColor = g_stGameInfo.vstEdgeColor[ g_stGameInfo.vbBlock[ i ] ];
                DrawRect( qwWindowID, g_stGameInfo.iBlockX * BLOCKSIZE,
                    iY + ( ( g_stGameInfo.iBlockY + i ) * BLOCKSIZE ),
                    ( g_stGameInfo.iBlockX + 1 ) * BLOCKSIZE,
                    iY + ( ( g_stGameInfo.iBlockY + i + 1 ) * BLOCKSIZE ),
                    stColor, FALSE );
                DrawRect( qwWindowID, g_stGameInfo.iBlockX * BLOCKSIZE + 1,
                    iY + ( ( g_stGameInfo.iBlockY + i ) * BLOCKSIZE ) + 1,
                    ( g_stGameInfo.iBlockX + 1 ) * BLOCKSIZE - 1,
                    iY + ( ( g_stGameInfo.iBlockY + i + 1 ) * BLOCKSIZE ) - 1,
                    stColor, FALSE );
            }
        }
    }

    // ���� ������ �׵θ��� �׸�
    DrawRect( qwWindowID, 0, iY, BLOCKSIZE * BOARDWIDTH - 1,
            iY + BLOCKSIZE * BOARDHEIGHT - 1, RGB( 0, 255, 0 ), FALSE );
}
