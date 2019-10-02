/**
 *  file    Main.c
 *  date    2010/03/23
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   C ���� �ۼ��� �������α׷��� ��Ʈ�� ����Ʈ ����
 */

#include "MINTOSLibrary.h"
#include "Main.h"

/**
 *  �������α׷��� C ��� ��Ʈ�� ����Ʈ
 */
int Main( char* pcArgument )
{
    QWORD qwWindowID;
    int iX;
    int iY;
    int iWidth;
    int iHeight;
    TEXTINFO stInfo;
    int iMoveLine;
    EVENT stReceivedEvent;
    KEYEVENT* pstKeyEvent;
    WINDOWEVENT* pstWindowEvent;
    DWORD dwFileOffset;
    RECT stScreenArea;
    
    //--------------------------------------------------------------------------
    // �׷��� ��� �Ǵ�
    //--------------------------------------------------------------------------
    // MINT64 OS�� �׷��� ���� �����ߴ��� Ȯ��
    if( IsGraphicMode() == FALSE )
    {        
        // MINT64 OS�� �׷��� ���� �������� �ʾҴٸ� ����
        printf( "This task can run only GUI mode~!!!\n" );
        return -1;
    }
    
    //--------------------------------------------------------------------------
    // ������ ������ ���� ���ۿ� �����ϰ� ���� �� ���� ������ ����� ���۸� ����
    //--------------------------------------------------------------------------
    // ���ڿ� �ƹ��͵� ���޵��� ������ ����
    if( strlen( pcArgument ) == 0 )
    {
        printf( "ex) exec hanviwer.elf abc.txt\n" );
        return 0;
    }
        
    // ������ ���͸����� ã�� �ڿ� ������ ũ�⸸ŭ �޸𸮸� �Ҵ��Ͽ� ������ ����
    // ���� �� ���� �������� ������ ���۵� ���� ����
    if( ReadFileToBuffer( pcArgument, &stInfo ) == FALSE )
    {
        printf( "%s file is not found\n", pcArgument );
        return 0;
    }

    //--------------------------------------------------------------------------
    // ������� ���� �ε����� ������ �� ù ��° ���κ��� ȭ�鿡 ���
    //--------------------------------------------------------------------------
    // �����츦 ȭ�� ����� ���� 500 �ȼ� X ���� 500 �ȼ��� ����
    GetScreenArea( &stScreenArea );
    iWidth = 500;
    iHeight = 500;
    iX = ( GetRectangleWidth( &stScreenArea ) - iWidth ) / 2;
    iY = ( GetRectangleHeight( &stScreenArea ) - iHeight ) / 2;
    qwWindowID = CreateWindow(iX, iY, iWidth, iHeight, WINDOW_FLAGS_DEFAULT | 
                              WINDOW_FLAGS_RESIZABLE, "�ѱ� ���(Hangul Viewer)" );
    
    // ���� �� ���� �������� ����ϰ� ���� ȭ�鿡 ����ϴ� ���� �ε����� 0���� ����
    CalculateFileOffsetOfLine( iWidth, iHeight, &stInfo );
    stInfo.iCurrentLineIndex = 0;
    
    // ���� ���κ��� ȭ�� ��ü ũ�⸸ŭ�� ǥ��
    DrawTextBuffer( qwWindowID, &stInfo );
        
    //--------------------------------------------------------------------------
    // GUI �½�ũ�� �̺�Ʈ ó�� ����
    //--------------------------------------------------------------------------
    while( 1 )
    {
        // �̺�Ʈ ť���� �̺�Ʈ�� ����
        if( ReceiveEventFromWindowQueue( qwWindowID, &stReceivedEvent ) == FALSE )
        {
            Sleep( 10 );
            continue;
        }
        
        // ���ŵ� �̺�Ʈ�� Ÿ�Կ� ���� ������ ó��
        switch( stReceivedEvent.qwType )
        {
            // Ű ���� ó��
        case EVENT_KEY_DOWN:
            pstKeyEvent = &( stReceivedEvent.stKeyEvent );
            if( pstKeyEvent->bFlags & KEY_FLAGS_DOWN )
            {
                // Ű ���� ���� ���� ���� ���� �� ����
                switch( pstKeyEvent->bASCIICode )
                {
                    // Page Up Ű�� Page Down Ű�� ȭ�鿡 ��� ������ ���� ������ �̵�
                case KEY_PAGEUP:
                    iMoveLine = -stInfo.iRowCount;
                    break;
                case KEY_PAGEDOWN:
                    iMoveLine = stInfo.iRowCount;
                    break;
                    // Up Ű�� Down Ű�� �� ���� ������ �̵�
                case KEY_UP:
                    iMoveLine = -1;
                    break;
                case KEY_DOWN:
                    iMoveLine = 1;
                    break;
                    
                    // ��Ÿ Ű�̰ų� ���� ��ġ���� ������ �ʿ䰡 ������ ����
                default:
                    iMoveLine = 0;
                    break;
                }
                
                // �ִ� �ּ� ���� ������ ����� ���� ���� �ε����� ����
                if( stInfo.iCurrentLineIndex + iMoveLine < 0 )
                {
                    iMoveLine = -stInfo.iCurrentLineIndex;
                }
                else if( stInfo.iCurrentLineIndex + iMoveLine >= stInfo.iMaxLineCount )
                {
                    iMoveLine = stInfo.iMaxLineCount - stInfo.iCurrentLineIndex - 1;
                }
                                
                // ��Ÿ Ű�̰ų� ������ �ʿ䰡 ������ ����
                if( iMoveLine == 0 )
                {
                    break;
                }
                        
                // ���� ������ �ε����� �����ϰ� ȭ�鿡 ���
                stInfo.iCurrentLineIndex += iMoveLine;
                DrawTextBuffer( qwWindowID, &stInfo );
            }
            break;
            
            // ������ ũ�� ���� ó��
        case EVENT_WINDOW_RESIZE:
            pstWindowEvent = &( stReceivedEvent.stWindowEvent );
            iWidth = GetRectangleWidth( &( pstWindowEvent->stArea ) );
            iHeight = GetRectangleHeight( &( pstWindowEvent->stArea ) );
            
            // ���� ������ �ִ� ���� �������� ����
            dwFileOffset = stInfo.pdwFileOffsetOfLine[ stInfo.iCurrentLineIndex ];
            
            // ����� ȭ�� ũ��� �ٽ� ���� ���� ���� �� ���� ��, �׸��� ���� �������� 
            // ����ϰ� �� ���� ������ ������ ���� �������� �̿��Ͽ� ���� ������ �ٽ� ���
            CalculateFileOffsetOfLine( iWidth, iHeight, &stInfo );
            stInfo.iCurrentLineIndex = dwFileOffset / stInfo.iColumnCount;
            
            // ���� ���κ��� ȭ�鿡 ���
            DrawTextBuffer( qwWindowID, &stInfo );
            break;
            
            // ������ �ݱ� ��ư ó��
        case EVENT_WINDOW_CLOSE:
            // �����츦 �����ϰ� �޸𸮸� ����
            DeleteWindow( qwWindowID );
            free( stInfo.pbFileBuffer );
            free( stInfo.pdwFileOffsetOfLine );
            return 0;
            break;
            
            // �� �� ����
        default:
            break;
        }
    }
    
    return 0;
}


/**
 *  ������ ã�Ƽ� ������ ũ�⸸ŭ ���۸� �Ҵ��ϰ� ���� �� ���� ������ ���۸� �Ҵ��� �ڿ�
 *  ������ ������ �о �޸𸮿� ����
 */
BOOL ReadFileToBuffer( const char* pcFileName, TEXTINFO* pstInfo )
{
    DIR* pstDirectory;
    struct dirent* pstEntry;
    DWORD dwFileSize;
    FILE* pstFile;
    DWORD dwReadSize;
    
    //--------------------------------------------------------------------------
    // ��Ʈ ���͸��� ��� ������ �˻�
    //--------------------------------------------------------------------------
    pstDirectory = opendir( "/" );
    dwFileSize = 0;
    
    // ���͸����� ������ �˻�
    while( 1 )
    {
        // ���͸����� ��Ʈ�� �ϳ��� ����
        pstEntry = readdir( pstDirectory );
        // �� �̻� ������ ������ ����
        if( pstEntry == NULL )
        {
            break;
        }
        
        // ���� �̸��� ���̿� ������ ���� ���� �˻�
        if( ( strlen( pstEntry->d_name ) == strlen( pcFileName ) ) &&
            ( memcmp( pstEntry->d_name, pcFileName, strlen( pcFileName ) ) 
                    == 0 ) )
        {
            dwFileSize = pstEntry->dwFileSize;
            break;
        }
    }
    // ���͸� �ڵ��� ��ȯ, �ڵ��� ��ȯ���� ������ �޸𸮰� �������� �ʰ� �����Ƿ�
    // �� �����ؾ� ��
    closedir( pstDirectory );

    if( dwFileSize == 0 )
    {
        printf( "%s file doesn't exist or size is zero\n", 
                pcFileName );
        return FALSE;
    }
    
    // ���� �̸��� ����
    memcpy( &( pstInfo->vcFileName ), pcFileName, sizeof( pstInfo->vcFileName ) );
    pstInfo->vcFileName[ sizeof( pstInfo->vcFileName ) - 1 ] = '\0';
    
    //--------------------------------------------------------------------------
    // ������ ��ü�� ���� �� �ִ� �ӽ� ���ۿ� ���� �� ���� �������� ������ ���۸� 
    // �Ҵ� �޾Ƽ� ������ ������ ��� ����
    //--------------------------------------------------------------------------
    // ���� ���� ���� �������� ������ ���۸� �Ҵ�
    pstInfo->pdwFileOffsetOfLine = malloc( MAXLINECOUNT * sizeof( DWORD ) );
    if( pstInfo->pdwFileOffsetOfLine == NULL )
    {
        printf( "Memory allocation fail\n" );
        return FALSE;
    }
    
    // ������ ������ ������ ���۸� �Ҵ�
    pstInfo->pbFileBuffer = ( BYTE* ) malloc( dwFileSize );
    if( pstInfo->pbFileBuffer == NULL )
    {
        printf( "Memory %dbytes allocate fail\n", dwFileSize );
        free( pstInfo->pdwFileOffsetOfLine );
        return FALSE;
    }
    
    // ������ ��� ��� �޸𸮿� ����
    pstFile = fopen( pcFileName, "r" );
    if( ( pstFile != NULL ) && 
        ( fread( pstInfo->pbFileBuffer, 1, dwFileSize, pstFile ) == dwFileSize ) )
    {
        fclose( pstFile );
        printf( "%s file read success\n", pcFileName );
    }
    else
    {
        printf( "%s file read fail\n", pcFileName );
        free( pstInfo->pdwFileOffsetOfLine );
        free( pstInfo->pbFileBuffer );
        fclose( pstFile );
        return FALSE;
    }

    // ������ ũ�� ����
    pstInfo->dwFileSize = dwFileSize;
    return TRUE;
}

/**
 *  ���� ������ ������ �м��Ͽ� ���� �� ���� �������� ���
 */
void CalculateFileOffsetOfLine( int iWidth, int iHeight, TEXTINFO* pstInfo )
{
    DWORD i;
    int iLineIndex;
    int iColumnIndex;
    BOOL bHangul;
    
    // ���� ������ ���� ǥ������ ���̸� ����ؼ� ���� �� ���� ���� ����� �� �ִ� 
    // ���� ���� ���
    pstInfo->iColumnCount = ( iWidth - MARGIN * 2 ) / FONT_ENGLISHWIDTH;
    pstInfo->iRowCount = ( iHeight - ( WINDOW_TITLEBAR_HEIGHT * 2 ) - 
            ( MARGIN * 2 ) ) / FONT_ENGLISHHEIGHT;
    
    // ������ ó������ ������ ���� ��ȣ�� ����ؼ� ���� �������� ����
    iLineIndex = 0;
    iColumnIndex = 0;
    pstInfo->pdwFileOffsetOfLine[ 0 ] = 0;
    for( i = 0 ; i < pstInfo->dwFileSize ; i++ )
    {
        // ���� �ǵ� ���ڴ� ����
        if( pstInfo->pbFileBuffer[ i ] == '\r' )
        {
            continue;
        }
        else if( pstInfo->pbFileBuffer[ i ] == '\t' )
        {
            // �� �����̸� �� ������ ũ�� ������ ����� �������� ����
            iColumnIndex = iColumnIndex + TABSPACE;
            iColumnIndex -= iColumnIndex % TABSPACE;
        }
        // �ѱ��� ��� ó��
        else if( pstInfo->pbFileBuffer[ i ] & 0x80 )
        {
            bHangul = TRUE;
            iColumnIndex += 2;
            i++;
        }
        else
        {
            bHangul = FALSE;
            iColumnIndex++;
        }
            
        // ����� ��ġ�� ���� �� ���� ���� �Ѱų� �� ���ڸ� ����� ������ ���� ���, 
        // �Ǵ� ���� ���ڰ� ����Ǹ� ���� ����
        if( ( iColumnIndex >= pstInfo->iColumnCount ) ||
            ( pstInfo->pbFileBuffer[ i ] == '\n' ) )
        {
            iLineIndex++;
            iColumnIndex = 0;
            
            // ���� ����� ���ڰ� �ѱ��̸� �ٸ� ���ο� ���ڸ� ���
            if( bHangul == TRUE )
            {
                i -= 2;
            }

            // ���� �ε��� ���ۿ� ������ ����
            if( i + 1 < pstInfo->dwFileSize )
            {
                pstInfo->pdwFileOffsetOfLine[ iLineIndex ] = i + 1;
            }
            
            // �ؽ�Ʈ �� �����ϴ� �ִ� ���� ���� �Ѿ�� ����
            if( iLineIndex >= MAXLINECOUNT )
            {
                break;
            }
        }
    }
    
    // ���� ������ ���� ��ȣ�� ����
    pstInfo->iMaxLineCount = iLineIndex;
}

/**
 *  ������ ȭ�� ���ۿ� ���� ���κ��� ȭ�鿡 ���
 */
BOOL DrawTextBuffer( QWORD qwWindowID, TEXTINFO* pstInfo )
{
    DWORD i;
    DWORD j;
    DWORD dwBaseOffset;
    BYTE bTemp;
    int iXOffset;
    int iYOffset;
    int iLineCountToPrint;
    int iColumnCountToPrint;
    char vcBuffer[ 100 ];
    RECT stWindowArea;
    int iLength;
    int iWidth;
    int iColumnIndex;

    // ��ǥ�� ���ذ�
    iXOffset = MARGIN;
    iYOffset = WINDOW_TITLEBAR_HEIGHT;
    GetWindowArea( qwWindowID, &stWindowArea );

    //--------------------------------------------------------------------------
    // ���� ���� ǥ�� ������ ������ ���
    //--------------------------------------------------------------------------
    // ���� �̸��� ���� ����, �׸��� ��ü ���� ���� ���
    iWidth = GetRectangleWidth( &stWindowArea );
    DrawRect( qwWindowID, 2, iYOffset, iWidth - 3, WINDOW_TITLEBAR_HEIGHT * 2, 
            RGB( 55, 215, 47 ), TRUE );
    // �ӽ� ���ۿ� ������ ����
    sprintf( vcBuffer, "����: %s, �� ��ȣ: %d/%d\n", pstInfo->vcFileName,
            pstInfo->iCurrentLineIndex + 1, pstInfo->iMaxLineCount );
    iLength = strlen( vcBuffer );
    // ����� ������ ���� ���� ǥ�� ������ ����� ���
    DrawText( qwWindowID, ( iWidth - iLength * FONT_ENGLISHWIDTH ) / 2, 
            WINDOW_TITLEBAR_HEIGHT + 2, RGB( 255, 255, 255 ), RGB( 55, 215, 47 ), 
            vcBuffer, strlen( vcBuffer ) );
    
    //--------------------------------------------------------------------------
    // ���� ���� ǥ�� ������ ���� ������ ���
    //--------------------------------------------------------------------------
    // �����͸� ����� �κ��� ��� ������� ��� �ڿ� ������ ���
    iYOffset = ( WINDOW_TITLEBAR_HEIGHT * 2 ) + MARGIN;
    DrawRect( qwWindowID, iXOffset, iYOffset, iXOffset + FONT_ENGLISHWIDTH * 
            pstInfo->iColumnCount, iYOffset + FONT_ENGLISHHEIGHT * pstInfo->iRowCount,
            RGB( 255, 255, 255 ), TRUE );
    
    //--------------------------------------------------------------------------
    // ������ �����ϸ鼭 ���� ������ ȭ�鿡 ���
    //--------------------------------------------------------------------------
    // ���� ���ο��� ���� ���� ���� �� ȭ�鿡 ����� �� �ִ� ���� ���� ���Ͽ�
    // ���� ���� ����
    iLineCountToPrint = MIN( pstInfo->iRowCount, 
                 ( pstInfo->iMaxLineCount - pstInfo->iCurrentLineIndex ) );
    for( j = 0 ; j < iLineCountToPrint ; j++ )
    {
        // ����� ������ ���� ������
        dwBaseOffset = pstInfo->pdwFileOffsetOfLine[ pstInfo->iCurrentLineIndex + j ];

        //----------------------------------------------------------------------
        // ������ �����ϸ鼭 ���� ���ο� ���ڸ� ���
        //----------------------------------------------------------------------
        // ���� ���ο��� ���� ���� ���� �� ȭ�鿡 ����� �� �ִ� ���� ���� ���Ͽ�
        // ���� ���� ����
        iColumnCountToPrint = MIN( pstInfo->iColumnCount,
                                   ( pstInfo->dwFileSize - dwBaseOffset ) );
        iColumnIndex = 0;
        for( i = 0 ; ( i < iColumnCountToPrint ) && 
                     ( iColumnIndex < pstInfo->iColumnCount ) ; i++ )
        {
            bTemp = pstInfo->pbFileBuffer[ i + dwBaseOffset ];
            // ���� ���ڰ� ���̸� ����
            if( bTemp == '\n' )
            {
                break;
            }
            // �� �����̸� �� ������ ũ�� ������ ����� �������� ����
            else if( bTemp == '\t')
            {
                // �� ���ڰ� �����ϴ� ���� ������ ����� �������� ����
                iColumnIndex = iColumnIndex + TABSPACE;
                iColumnIndex -= iColumnIndex % TABSPACE;
            }
            // ���� �ǵ� �����̸� ����
            else if( bTemp == '\r' )
            {
                // Nothing
            }
            // ��Ÿ ���ڴ� ȭ�鿡 ���
            else
            {
                // �����ڴ� �״�� ó��
                if( ( bTemp & 0x80 ) == 0 )
                {
                    // ����� ��ġ�� ���ڸ� ����ϰ� ���� ��ġ�� �̵�
                    DrawText( qwWindowID, iColumnIndex * FONT_ENGLISHWIDTH + iXOffset,
                            iYOffset + ( j * FONT_ENGLISHHEIGHT ), RGB( 0, 0, 0 ),
                            RGB( 255, 255, 255 ), &bTemp, 1 );
                    iColumnIndex++;
                }
                // �ѱ��� ��� �ѱ��� ������� �� �ִ� �÷��� ���� �ʴ� ��츸 ó��
                else if( ( iColumnIndex + 2 ) < pstInfo->iColumnCount )
                {
                    // ����� ��ġ�� ���ڸ� ����ϰ� ���� ��ġ�� �̵�
                    DrawText( qwWindowID, iColumnIndex * FONT_ENGLISHWIDTH + iXOffset,
                            iYOffset + ( j * FONT_ENGLISHHEIGHT ), RGB( 0, 0, 0 ),
                            RGB( 255, 255, 255 ), &pstInfo->pbFileBuffer[ i + dwBaseOffset ],
                            2 );
                    // �ѱ��� ��������Ƿ� �������� 2�踸ŭ �̵�
                    iColumnIndex += 2;
                    i++;
                }
            }
        }
    }
    
    // ������ ��ü�� �����Ͽ� ����� ȭ���� ������Ʈ
    ShowWindow( qwWindowID, TRUE );
    
    return TRUE;
}
