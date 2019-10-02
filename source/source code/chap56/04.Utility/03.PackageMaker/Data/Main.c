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
