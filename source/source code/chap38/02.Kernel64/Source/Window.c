/**
 *  file    Window.c
 *  date    2009/09/28
 *  author  kkamagui
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   GUI �ý��ۿ� ���õ� �Լ��� ������ �ҽ� ����
 */
#include "Window.h"
#include "VBE.h"
#include "Task.h"
#include "Font.h"
#include "DynamicMemory.h"

// GUI �ý��� ���� �ڷᱸ��
static WINDOWPOOLMANAGER gs_stWindowPoolManager;
// ������ �Ŵ��� ���� �ڷᱸ��
static WINDOWMANAGER gs_stWindowManager;

//==============================================================================
//  ������ Ǯ ����
//==============================================================================
/**
 *  ������ Ǯ�� �ʱ�ȭ
 */
static void kInitializeWindowPool( void )
{
    int i;
    void* pvWindowPoolAddress;
    
    // �ڷᱸ�� �ʱ�ȭ
    kMemSet( &gs_stWindowPoolManager, 0, sizeof( gs_stWindowPoolManager ) );
    
    // ������ Ǯ�� �޸𸮸� �Ҵ�
    pvWindowPoolAddress = ( void* ) kAllocateMemory( sizeof( WINDOW ) * WINDOW_MAXCOUNT );
    if( pvWindowPoolAddress == NULL )
    {
        kPrintf( "Window Pool Allocate Fail\n" );
        while( 1 )
        {
            ;
        }
    }
    
    // ������ Ǯ�� ��巹���� �����ϰ� �ʱ�ȭ
    gs_stWindowPoolManager.pstStartAddress = ( WINDOW* ) pvWindowPoolAddress;
    kMemSet( pvWindowPoolAddress, 0, sizeof( WINDOW ) * WINDOW_MAXCOUNT );

    // ������ Ǯ�� ID�� �Ҵ�
    for( i = 0 ; i < WINDOW_MAXCOUNT ; i++ )
    {
        gs_stWindowPoolManager.pstStartAddress[ i ].stLink.qwID = i;
    }
    
    // �������� �ִ� ������ �Ҵ�� Ƚ���� �ʱ�ȭ
    gs_stWindowPoolManager.iMaxCount = WINDOW_MAXCOUNT;
    gs_stWindowPoolManager.iAllocatedCount = 1;
    
    // ���ؽ� �ʱ�ȭ
    kInitializeMutex( &( gs_stWindowPoolManager.stLock ) );
}


/**
 *  ������ �ڷᱸ���� �Ҵ�
 */
static WINDOW* kAllocateWindow( void )
{
    WINDOW* pstEmptyWindow;
    int i;

    // ����ȭ ó��
    kLock( &( gs_stWindowPoolManager.stLock ) );

    // �����찡 ��� �Ҵ�Ǿ����� ����
    if( gs_stWindowPoolManager.iUseCount == gs_stWindowPoolManager.iMaxCount )
    {
        // ����ȭ ó��
        kUnlock( &gs_stWindowPoolManager.stLock );
        return NULL;
    }

    // ������ Ǯ�� ��� ���鼭 �� ������ �˻�
    for( i = 0 ; i < gs_stWindowPoolManager.iMaxCount ; i++ )
    {
        // ID�� ���� 32��Ʈ�� 0�̸� ����ִ� ������ �ڷᱸ����
        if( ( gs_stWindowPoolManager.pstStartAddress[ i ].stLink.qwID >> 32 ) == 0 )
        {
            pstEmptyWindow = &( gs_stWindowPoolManager.pstStartAddress[ i ] );
            break;
        }
    }

    // ���� 32��Ʈ�� 0�� �ƴ� ������ �����ؼ� �Ҵ�� ������� ����
    pstEmptyWindow->stLink.qwID =
        ( ( QWORD ) gs_stWindowPoolManager.iAllocatedCount << 32 ) | i;

    // �ڷᱸ���� ��� ���� ������ �Ҵ�� Ƚ���� ����
    gs_stWindowPoolManager.iUseCount++;
    gs_stWindowPoolManager.iAllocatedCount++;
    if( gs_stWindowPoolManager.iAllocatedCount == 0 )
    {
        gs_stWindowPoolManager.iAllocatedCount = 1;
    }

    // ����ȭ ó��
    kUnlock( &( gs_stWindowPoolManager.stLock ) );

    // �������� ���ؽ� �ʱ�ȭ
    kInitializeMutex( &( pstEmptyWindow->stLock ) );

    return pstEmptyWindow;
}

/**
 *  ������ �ڷᱸ���� ����
 */
static void kFreeWindow( QWORD qwID )
{
    int i;

    // ������ ID�� ������ Ǯ�� �������� ���, ������ ID�� ���� 32��Ʈ�� �ε��� ������ ��
    i = GETWINDOWOFFSET( qwID );

    // ����ȭ ó��
    kLock( &( gs_stWindowPoolManager.stLock ) );
    
    // ������ �ڷᱸ���� �ʱ�ȭ�ϰ� ID ����
    kMemSet( &( gs_stWindowPoolManager.pstStartAddress[ i ] ), 0, sizeof( WINDOW ) );
    gs_stWindowPoolManager.pstStartAddress[ i ].stLink.qwID = i;

    // ��� ���� �ڷᱸ���� ������ ����
    gs_stWindowPoolManager.iUseCount--;

    // ����ȭ ó��
    kUnlock( &( gs_stWindowPoolManager.stLock ) );
}

//==============================================================================
//  ������� ������ �Ŵ��� ����
//==============================================================================
/**
 *  GUI �ý����� �ʱ�ȭ
 */
void kInitializeGUISystem( void )
{
    VBEMODEINFOBLOCK* pstModeInfo;
    QWORD qwBackgroundWindowID;

    // ������ Ǯ�� �ʱ�ȭ
    kInitializeWindowPool();

    // VBE ��� ���� ����� ��ȯ
    pstModeInfo = kGetVBEModeInfoBlock();

    // ���� �޸� ��巹�� ����
    gs_stWindowManager.pstVideoMemory = ( COLOR* )
        ( ( QWORD ) pstModeInfo->dwPhysicalBasePointer & 0xFFFFFFFF );

    // ���콺 Ŀ���� �ʱ� ��ġ ����
    gs_stWindowManager.iMouseX = pstModeInfo->wXResolution / 2;
    gs_stWindowManager.iMouseY = pstModeInfo->wYResolution / 2;

    // ȭ�� ������ ���� ����
    gs_stWindowManager.stScreenArea.iX1 = 0;
    gs_stWindowManager.stScreenArea.iY1 = 0;
    gs_stWindowManager.stScreenArea.iX2 = pstModeInfo->wXResolution - 1;
    gs_stWindowManager.stScreenArea.iY2 = pstModeInfo->wYResolution - 1;

    // ���ؽ� �ʱ�ȭ
    kInitializeMutex( &( gs_stWindowManager.stLock ) );

    // ������ ����Ʈ �ʱ�ȭ
    kInitializeList( &( gs_stWindowManager.stWindowList ) );

    //--------------------------------------------------------------------------
    // ��� ������ ����
    //--------------------------------------------------------------------------
    // �÷��׿� 0�� �Ѱܼ� ȭ�鿡 �����츦 �׸��� �ʵ��� ��. ��� ������� ������ ���� 
    // ������ ��� ĥ�� �� ��Ÿ��
    qwBackgroundWindowID = kCreateWindow( 0, 0, pstModeInfo->wXResolution, 
            pstModeInfo->wYResolution, 0, WINDOW_BACKGROUNDWINDOWTITLE );
    gs_stWindowManager.qwBackgoundWindowID = qwBackgroundWindowID; 
    // ��� ������ ���ο� ������ ä��
    kDrawRect( qwBackgroundWindowID, 0, 0, pstModeInfo->wXResolution - 1, 
            pstModeInfo->wYResolution - 1, WINDOW_COLOR_SYSTEMBACKGROUND, TRUE );
    // ��� �����츦 ȭ�鿡 ��Ÿ��
    kShowWindow( qwBackgroundWindowID, TRUE );
}

/**
 *  ������ �Ŵ����� ��ȯ
 */
WINDOWMANAGER* kGetWindowManager( void )
{
    return &gs_stWindowManager;
}

/**
 *  ��� �������� ID�� ��ȯ
 */
QWORD kGetBackgroundWindowID( void )
{
    return gs_stWindowManager.qwBackgoundWindowID;
}

/**
 *  ȭ�� ������ ũ�⸦ ��ȯ
 */
void kGetScreenArea( RECT* pstScreenArea )
{
    kMemCpy( pstScreenArea, &( gs_stWindowManager.stScreenArea ), sizeof( RECT ) );
}

/**
 *  �����츦 ����
 */
QWORD kCreateWindow( int iX, int iY, int iWidth, int iHeight, DWORD dwFlags,
        const char* pcTitle )
{
    WINDOW* pstWindow;
    TCB* pstTask;

    // ũ�Ⱑ 0�� ������� ���� �� ����
    if( ( iWidth <= 0 ) || ( iHeight <= 0 ) )
    {
        return WINDOW_INVALIDID;
    }

    // ������ �ڷᱸ���� �Ҵ�
    pstWindow = kAllocateWindow();
    if( pstWindow == NULL )
    {
        return WINDOW_INVALIDID;
    }

    // ������ ���� ����
    pstWindow->stArea.iX1 = iX;
    pstWindow->stArea.iY1 = iY;
    pstWindow->stArea.iX2 = iX + iWidth - 1;
    pstWindow->stArea.iY2 = iY + iHeight - 1;
    
    // ������ ���� ����
    kMemCpy( pstWindow->vcWindowTitle, pcTitle, WINDOW_TITLEMAXLENGTH );
    pstWindow->vcWindowTitle[ WINDOW_TITLEMAXLENGTH ] = '\0';

    // ������ ȭ�� ���� �Ҵ�
    pstWindow->pstWindowBuffer = ( COLOR* ) kAllocateMemory( iWidth * iHeight *
            sizeof( COLOR ) );
    if( pstWindow == NULL )
    {
        // ������ ȭ�� ���� �Ҵ翡 �����ϸ� ������ �ڷᱸ�� ��ȯ
        kFreeWindow( pstWindow->stLink.qwID );
        return WINDOW_INVALIDID;
    }

    // �����츦 ������ �½�ũ�� ID�� ����
    pstTask = kGetRunningTask( kGetAPICID() );
    pstWindow->qwTaskID =  pstTask->stLink.qwID;

    // ������ �Ӽ� ����
    pstWindow->dwFlags = dwFlags;

    // ������ ��� �׸���
    kDrawWindowBackground( pstWindow->stLink.qwID );

    // ������ �׵θ� �׸���
    if( dwFlags & WINDOW_FLAGS_DRAWFRAME )
    {
        kDrawWindowFrame( pstWindow->stLink.qwID );
    }

    // ������ ���� ǥ���� �׸���
    if( dwFlags & WINDOW_FLAGS_DRAWTITLE )
    {
        kDrawWindowTitle( pstWindow->stLink.qwID, pcTitle );
    }

    // ����ȭ ó��
    kLock( &( gs_stWindowManager.stLock ) );

    // ������ ����Ʈ�� ���� �������� �߰��Ͽ� �ֻ��� ������� ����
    kAddListToTail( &gs_stWindowManager.stWindowList, pstWindow );

    // ����ȭ ó��
    kUnlock( &( gs_stWindowManager.stLock ) );

    // �����츦 �׸��� �ɼ��� ��������� �ش� �����츦 �׸�
    if( dwFlags & WINDOW_FLAGS_SHOW )
    {
        // ������ ������ŭ ȭ�鿡 ������Ʈ
        kRedrawWindowByArea( &( pstWindow->stArea ) );
    }

    return pstWindow->stLink.qwID;
}

/**
 *  �����츦 ����
 */
BOOL kDeleteWindow( QWORD qwWindowID )
{
    WINDOW* pstWindow;
    RECT stArea;

    // ����ȭ ó��
    kLock( &( gs_stWindowManager.stLock ) );
    
    // ������ �˻�
    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL )
    {
        // ����ȭ ó��
        kUnlock( &( gs_stWindowManager.stLock ) );
        return FALSE;
    }

    // �����츦 �����ϱ� ���� ������ �����ص�
    kMemCpy( &stArea, &( pstWindow->stArea ), sizeof( RECT ) );

    // ������ ����Ʈ���� ������ ����
    if( kRemoveList( &( gs_stWindowManager.stWindowList ), qwWindowID ) == NULL )
    {
        // ����ȭ ó��
        kUnlock( &( pstWindow->stLock ) );
        kUnlock( &( gs_stWindowManager.stLock ) );
        return FALSE;
    }

    // ����ȭ ó��
    kLock( &( pstWindow->stLock ) );

    // ������ ȭ�� ���۸� ��ȯ
    kFreeMemory( pstWindow->pstWindowBuffer );

    // ����ȭ ó��
    kUnlock( &( pstWindow->stLock ) );

    // ������ �ڷᱸ���� ��ȯ
    kFreeWindow( qwWindowID );
    
    // ����ȭ ó��
    kUnlock( &( gs_stWindowManager.stLock ) );

    // �����Ǳ� ���� �����찡 �ִ� ������ ȭ�鿡 �ٽ� ������Ʈ
    kRedrawWindowByArea( &stArea );
    return TRUE;
}

/**
 *  �½�ũ ID�� ��ġ�ϴ� ��� �����츦 ����
 */
BOOL kDeleteAllWindowInTaskID( QWORD qwTaskID )
{
    WINDOW* pstWindow;
    WINDOW* pstNextWindow;

    // ����ȭ ó��
    kLock( &( gs_stWindowManager.stLock ) );

    // ����Ʈ���� ù ��° �����츦 ��ȯ
    pstWindow = kGetHeaderFromList( &( gs_stWindowManager.stWindowList ) );
    while( pstWindow != NULL )
    {
        // ���� �����츦 �̸� ����
        pstNextWindow = kGetNextFromList( &( gs_stWindowManager.stWindowList ),
                pstWindow );

        // ��� �����찡 �ƴϰ� �½�ũ ID�� ��ġ�ϸ� ������ ����
        if( ( pstWindow->stLink.qwID != gs_stWindowManager.qwBackgoundWindowID ) &&
            ( pstWindow->qwTaskID == qwTaskID ) )
        {
            kDeleteWindow( pstWindow->stLink.qwID );
        }

        // �̸� ���ص� ���� �������� ���� ����
        pstWindow = pstNextWindow;
    }

    // ����ȭ ó��
    kUnlock( &( gs_stWindowManager.stLock ) );
}

/**
 *  ������ ID�� ������ �����͸� ��ȯ
 */
WINDOW* kGetWindow( QWORD qwWindowID )
{
    WINDOW* pstWindow;

    // ������ ID�� ��ȿ ���� �˻�
    if( GETWINDOWOFFSET( qwWindowID ) >= WINDOW_MAXCOUNT )
    {
        return NULL;
    }

    // ID�� ������ �����͸� ã�� �� ID�� ��ġ�ϸ� ��ȯ
    pstWindow = &gs_stWindowPoolManager.pstStartAddress[ GETWINDOWOFFSET( qwWindowID )];
    if( pstWindow->stLink.qwID == qwWindowID )
    {
        return pstWindow;
    }

    return NULL;
}

/**
 *  ������ ID�� ������ �����͸� ã�� ������ ���ؽ��� ��� �� ��ȯ
 */
WINDOW* kGetWindowWithWindowLock( QWORD qwWindowID )
{
    WINDOW* pstWindow;
    BOOL bResult;

    // �����츦 �˻�
    pstWindow = kGetWindow( qwWindowID );
    if( pstWindow == NULL )
    {
        return NULL;
    }
    
    // ����ȭ ó���� �� �ٽ� ������ ID�� ������ �˻�
    kLock( &(pstWindow->stLock ) );
    // ������ ����ȭ�� �� �ڿ� ������ ID�� �����츦 �˻��� �� ���ٸ� ���߿� �����찡
    // �ٲ� ���̹Ƿ� NULL ��ȯ
    pstWindow = kGetWindow( qwWindowID );
    if( pstWindow == NULL )
    {
        // ����ȭ ó��
        kUnlock( &(pstWindow->stLock ) );
        return NULL;
    }
    
    return pstWindow;
}

/**
 *  �����츦 ȭ�鿡 ��Ÿ���ų� ����
 */
BOOL kShowWindow( QWORD qwWindowID, BOOL bShow )
{
    WINDOW* pstWindow;

    // ������ �˻��� ����ȭ ó��
    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL )
    {
        return FALSE;
    }
    
    // ������ �Ӽ� ����
    if( bShow == TRUE )
    {
        pstWindow->dwFlags |= WINDOW_FLAGS_SHOW;
    }
    else
    {
        pstWindow->dwFlags &= ~WINDOW_FLAGS_SHOW;
    }

    // ����ȭ ó��
    kUnlock( &( pstWindow->stLock ) );
    
    // �����찡 �ִ� ������ �ٽ� ������Ʈ�����ν� �����츦 ��Ÿ���ų� ����
    kRedrawWindowByArea( &( pstWindow->stArea ) );
    return TRUE;
}

/**
 *  Ư�� ������ �����ϴ� ������� ��� �׸�
 */
BOOL kRedrawWindowByArea( const RECT* pstArea )
{
    WINDOW* pstWindow;
    WINDOW* pstTargetWindow = NULL;
    RECT stOverlappedArea;
    RECT stCursorArea;

    // ȭ�� ������ ��ġ�� ������ ������ �׸� �ʿ䰡 ����
    if( kGetOverlappedRectangle( &( gs_stWindowManager.stScreenArea ), pstArea,
            &stOverlappedArea ) == FALSE )
    {
        return FALSE;
    }

    //--------------------------------------------------------------------------
    // Z ������ ������, �� ������ ����Ʈ�� ù ��°���� ���������� ������ ���鼭 
    // ������Ʈ�� ������ ��ġ�� �����츦 ã�� ���� �޸𸮷� ����
    //--------------------------------------------------------------------------
    // ����ȭ ó��
    kLock( &( gs_stWindowManager.stLock ) );

    // ���� ������ ����Ʈ�� Z ������ ������, �� ó���� �ִ� �����찡 Z ������ ������
    // �����찡 �ǰ� ������ �����찡 �ֻ��� �����찡 �ǵ��� ���ĵǾ� ���� 
    // ���� ������ ����Ʈ�� ó������ ���󰡸鼭 �׸� ������ �����ϴ� �����츦 ã�� 
    // �� ��������� �ֻ��� ��������� ȭ�鿡 �����ϸ� ��
    pstWindow = kGetHeaderFromList( &( gs_stWindowManager.stWindowList ) );
    while( pstWindow != NULL )
    {
        // �����츦 ȭ�鿡 ��Ÿ���� �ɼ��� �����Ǿ�������,
        // ������Ʈ�� �κа� �����찡 �����ϴ� ������ ��ġ�� ��ġ�� ��ŭ�� ȭ�鿡 ����
        if( ( pstWindow->dwFlags & WINDOW_FLAGS_SHOW ) &&
            ( kIsRectangleOverlapped( &( pstWindow->stArea ), &stOverlappedArea )
                == TRUE ) )
        {
            // ����ȭ ó��
            kLock( &( pstWindow->stLock ) );

            // ������ ���� �޸𸮷� �����ϴ� �Լ�
            kCopyWindowBufferToFrameBuffer( pstWindow, &stOverlappedArea );

            // ����ȭ ó��
            kUnlock( &( pstWindow->stLock ) );
        }

        // ���� �����츦 ã��
        pstWindow = kGetNextFromList( &( gs_stWindowManager.stWindowList ),
                pstWindow );
    }

    // ����ȭ ó��
    kUnlock( &( gs_stWindowManager.stLock ) );

    //--------------------------------------------------------------------------
    // ���콺 Ŀ�� ������ ���ԵǸ� ���콺 Ŀ���� ���� �׸�
    //--------------------------------------------------------------------------
    // ���콺 ������ RECT �ڷᱸ���� ����
    kSetRectangleData( gs_stWindowManager.iMouseX, gs_stWindowManager.iMouseY,
            gs_stWindowManager.iMouseX + MOUSE_CURSOR_WIDTH,
            gs_stWindowManager.iMouseY + MOUSE_CURSOR_HEIGHT, &stCursorArea );
    
    // ��ġ���� Ȯ���Ͽ� ��ģ�ٸ� ���콺 Ŀ���� �׸�
    if( kIsRectangleOverlapped( &stOverlappedArea, &stCursorArea ) == TRUE )
    {
        kDrawCursor( gs_stWindowManager.iMouseX, gs_stWindowManager.iMouseY );
    }
}

/**
 *  ������ ȭ�� ������ �Ϻ� �Ǵ� ��ü�� ������ ���۷� ����
 */
static void kCopyWindowBufferToFrameBuffer( const WINDOW* pstWindow,
        const RECT* pstCopyArea )
{
    RECT stTempArea;
    RECT stOverlappedArea;
    int iOverlappedWidth;
    int iOverlappedHeight;
    int iScreenWidth;
    int iWindowWidth;
    int i;
    COLOR* pstCurrentVideoMemoryAddress;
    COLOR* pstCurrentWindowBufferAddress;

    // �����ؾ� �ϴ� ������ ȭ�� ������ ��ġ�� �κ��� �ӽ÷� ���
    if( kGetOverlappedRectangle( &( gs_stWindowManager.stScreenArea ), pstCopyArea,
            &stTempArea ) == FALSE )
    {
        return ;
    }

    // ������ ������ �ӽ÷� ����� ������ ��ġ�� �κ��� �ٽ� ���
    // �� ������ ��ġ�� �ʴ´ٸ� ���� �޸𸮷� ������ �ʿ� ����
    if( kGetOverlappedRectangle( &stTempArea, &( pstWindow->stArea ),
            &stOverlappedArea ) == FALSE )
    {
        return ;
    }

    // �� ������ �ʺ�� ���̸� ���
    iScreenWidth = kGetRectangleWidth( &( gs_stWindowManager.stScreenArea ) );
    iWindowWidth = kGetRectangleWidth( &( pstWindow->stArea ) );
    iOverlappedWidth = kGetRectangleWidth( &stOverlappedArea );
    iOverlappedHeight = kGetRectangleHeight( &stOverlappedArea );

    // ������ ������ ���� �޸� ��巹���� ������ ȭ�� ������ ��巹���� ���
    pstCurrentVideoMemoryAddress = gs_stWindowManager.pstVideoMemory +
        stOverlappedArea.iY1 * iScreenWidth + stOverlappedArea.iX1;

    // ������ ȭ�� ���۴� ȭ�� ��ü�� �ƴ� �����츦 �������� �� ��ǥ�̹Ƿ�,
    // ��ġ�� ������ ������ ���� ��ǥ �������� ��ȯ
    pstCurrentWindowBufferAddress = pstWindow->pstWindowBuffer +
        ( stOverlappedArea.iY1 - pstWindow->stArea.iY1 ) * iWindowWidth +
        ( stOverlappedArea.iX1 - pstWindow->stArea.iX1 );

    // ������ ���鼭 ������ ȭ�� ������ ������ ���� �޸𸮷� ����
    for( i = 0 ; i < iOverlappedHeight ; i++ )
    {
        // ���� ���� �ѹ��� ����
        kMemCpy( pstCurrentVideoMemoryAddress, pstCurrentWindowBufferAddress,
                iOverlappedWidth * sizeof( COLOR ) );

        // ���� �������� �޸� ��巹�� �̵�
        pstCurrentVideoMemoryAddress += iScreenWidth;
        pstCurrentWindowBufferAddress += iWindowWidth;
    }
}


//==============================================================================
//  ������ ���ο� �׸��� �Լ��� ���콺 Ŀ�� ����
//==============================================================================
/**
 *  ������ ȭ�� ���ۿ� ������ �׵θ� �׸���
 */
BOOL kDrawWindowFrame( QWORD qwWindowID )
{
    WINDOW* pstWindow;
    RECT stArea;
    int iWidth;
    int iHeight;

    // ������ �˻��� ����ȭ ó��
    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL )
    {
        return FALSE;
    }
    
    // �������� �ʺ�� ���̸� ���
    iWidth = kGetRectangleWidth( &( pstWindow->stArea ) );
    iHeight = kGetRectangleHeight( &( pstWindow->stArea ) );
    // Ŭ���� ���� ����
    kSetRectangleData( 0, 0, iWidth - 1, iHeight - 1, &stArea );

    // ������ �������� �����ڸ��� �׸�, 2 �ȼ� �β�
    kInternalDrawRect( &stArea, pstWindow->pstWindowBuffer,
            0, 0, iWidth - 1, iHeight - 1, WINDOW_COLOR_FRAME, FALSE );

    kInternalDrawRect( &stArea, pstWindow->pstWindowBuffer,
            1, 1, iWidth - 2, iHeight - 2, WINDOW_COLOR_FRAME, FALSE );

    // ����ȭ ó��
    kUnlock( &( pstWindow->stLock ) );

    return TRUE;
}


/**
 *  ������ ȭ�� ���ۿ� ��� �׸���
 */
BOOL kDrawWindowBackground( QWORD qwWindowID )
{
    WINDOW* pstWindow;
    int iWidth;
    int iHeight;
    RECT stArea;
    int iX;
    int iY;

    // ������ �˻��� ����ȭ ó��
    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL )
    {
        return FALSE;
    }

    // �������� �ʺ�� ���̸� ���
    iWidth = kGetRectangleWidth( &( pstWindow->stArea ) );
    iHeight = kGetRectangleHeight( &( pstWindow->stArea ) );
    // Ŭ���� ���� ����
    kSetRectangleData( 0, 0, iWidth - 1, iHeight - 1, &stArea );

    // �����쿡 ���� ǥ������ ������ �� �Ʒ����� ä��
    if( pstWindow->dwFlags & WINDOW_FLAGS_DRAWTITLE )
    {
        iY = WINDOW_TITLEBAR_HEIGHT;
    }
    else
    {
        iY = 0;
    }

    // ������ �׵θ��� �׸��� �ɼ��� �����Ǿ� ������ �׵θ��� ������ ������ ä��
    if( pstWindow->dwFlags & WINDOW_FLAGS_DRAWFRAME )
    {
        iX = 2;
    }
    else
    {
        iX = 0;
    }

    // �������� ���θ� ä��
    kInternalDrawRect( &stArea, pstWindow->pstWindowBuffer,
            iX, iY, iWidth - 1 - iX, iHeight - 1 - iX, WINDOW_COLOR_BACKGROUND, 
            TRUE );
    
    // ����ȭ ó��
    kUnlock( &( pstWindow->stLock ) );

    return TRUE;
}

/**
 *  ������ ȭ�� ���ۿ� ������ ���� ǥ���� �׸���
 */
BOOL kDrawWindowTitle( QWORD qwWindowID, const char* pcTitle )
{
    WINDOW* pstWindow;
    int iWidth;
    int iHeight;
    int iX;
    int iY;
    RECT stArea;
    RECT stButtonArea;

    // ������ �˻��� ����ȭ ó��
    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL )
    {
        return FALSE;
    }
    
    // �������� �ʺ�� ���̸� ���
    iWidth = kGetRectangleWidth( &( pstWindow->stArea ) );
    iHeight = kGetRectangleHeight( &( pstWindow->stArea ) );
    // Ŭ���� ���� ����
    kSetRectangleData( 0, 0, iWidth - 1, iHeight - 1, &stArea );

    //--------------------------------------------------------------------------
    // ���� ǥ���� �׸���
    //--------------------------------------------------------------------------
    // ���� ǥ������ ä��
    kInternalDrawRect( &stArea, pstWindow->pstWindowBuffer,
            0, 3, iWidth - 1, WINDOW_TITLEBAR_HEIGHT - 1,
            WINDOW_COLOR_TITLEBARBACKGROUND, TRUE );

    // ������ ������ ǥ��
    kInternalDrawText( &stArea, pstWindow->pstWindowBuffer,
            6, 3, WINDOW_COLOR_TITLEBARTEXT, WINDOW_COLOR_TITLEBARBACKGROUND,
            pcTitle, kStrLen( pcTitle ) );

    // ���� ǥ������ ��ü�� ���̰� ������ ���� �׸�, 2 �ȼ� �β�
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            1, 1, iWidth - 1, 1, WINDOW_COLOR_TITLEBARBRIGHT1 );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            1, 2, iWidth - 1, 2, WINDOW_COLOR_TITLEBARBRIGHT2 );

    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            1, 2, 1, WINDOW_TITLEBAR_HEIGHT - 1, WINDOW_COLOR_TITLEBARBRIGHT1 );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            2, 2, 2, WINDOW_TITLEBAR_HEIGHT - 1, WINDOW_COLOR_TITLEBARBRIGHT2 );

    // ���� ǥ������ �Ʒ��ʿ� ���� �׸�
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            2, WINDOW_TITLEBAR_HEIGHT - 2, iWidth - 2, WINDOW_TITLEBAR_HEIGHT - 2,
            WINDOW_COLOR_TITLEBARUNDERLINE );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            2, WINDOW_TITLEBAR_HEIGHT - 1, iWidth - 2, WINDOW_TITLEBAR_HEIGHT - 1,
            WINDOW_COLOR_TITLEBARUNDERLINE );

    // ����ȭ ó��
    kUnlock( &( pstWindow->stLock ) );

    //--------------------------------------------------------------------------
    // �ݱ� ��ư �׸���
    //--------------------------------------------------------------------------
    // �ݱ� ��ư�� �׸�, ������ ���� ǥ��
    stButtonArea.iX1 = iWidth - WINDOW_XBUTTON_SIZE - 1;
    stButtonArea.iY1 = 1;
    stButtonArea.iX2 = iWidth - 2;
    stButtonArea.iY2 = WINDOW_XBUTTON_SIZE - 1;
    kDrawButton( qwWindowID, &stButtonArea, WINDOW_COLOR_BACKGROUND, "", 
            WINDOW_COLOR_BACKGROUND );

    // ������ �˻��� ����ȭ ó��
    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL )
    {
        return FALSE;
    }
    
    // �ݱ� ��ư ���ο� �밢�� X�� 3 �ȼ��� �׸�
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            iWidth - 2 - 18 + 4, 1 + 4, iWidth - 2 - 4,
            WINDOW_TITLEBAR_HEIGHT - 6, WINDOW_COLOR_XBUTTONLINECOLOR );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            iWidth - 2 - 18 + 5, 1 + 4, iWidth - 2 - 4,
            WINDOW_TITLEBAR_HEIGHT - 7, WINDOW_COLOR_XBUTTONLINECOLOR );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            iWidth - 2 - 18 + 4, 1 + 5, iWidth - 2 - 5,
            WINDOW_TITLEBAR_HEIGHT - 6, WINDOW_COLOR_XBUTTONLINECOLOR );

    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            iWidth - 2 - 18 + 4, 19 - 4, iWidth - 2 - 4, 1 + 4,
            WINDOW_COLOR_XBUTTONLINECOLOR );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            iWidth - 2 - 18 + 5, 19 - 4, iWidth - 2 - 4, 1 + 5,
            WINDOW_COLOR_XBUTTONLINECOLOR );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            iWidth - 2 - 18 + 4, 19 - 5, iWidth - 2 - 5, 1 + 4,
            WINDOW_COLOR_XBUTTONLINECOLOR );

    // ����ȭ ó��
    kUnlock( &( pstWindow->stLock ) );
    
    return TRUE;
}

/**
 *  ������ ���ο� ��ư �׸���
 */
BOOL kDrawButton( QWORD qwWindowID, RECT* pstButtonArea, COLOR stBackgroundColor,
        const char* pcText, COLOR stTextColor )
{
    WINDOW* pstWindow;
    RECT stArea;
    int iWindowWidth;
    int iWindowHeight;
    int iTextLength;
    int iTextWidth;
    int iButtonWidth;
    int iButtonHeight;
    int iTextX;
    int iTextY;

    // ������ �˻��� ����ȭ ó��
    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL )
    {
        return FALSE;
    }
    
    // �������� �ʺ�� ���̸� ���
    iWindowWidth = kGetRectangleWidth( &( pstWindow->stArea ) );
    iWindowHeight = kGetRectangleHeight( &( pstWindow->stArea ) );
    // Ŭ���� ���� ����
    kSetRectangleData( 0, 0, iWindowWidth - 1, iWindowHeight - 1, &stArea );

    // ��ư�� ������ ǥ��
    kInternalDrawRect( &stArea, pstWindow->pstWindowBuffer,
            pstButtonArea->iX1, pstButtonArea->iY1, pstButtonArea->iX2,
            pstButtonArea->iY2, stBackgroundColor, TRUE );

    // ��ư�� �ؽ�Ʈ�� �ʺ�� ���̸� ���
    iButtonWidth = kGetRectangleWidth( pstButtonArea );
    iButtonHeight = kGetRectangleHeight( pstButtonArea );
    iTextLength = kStrLen( pcText );
    iTextWidth = iTextLength * FONT_ENGLISHWIDTH;
    
    // �ؽ�Ʈ�� ��ư�� ����� ��ġ�ϵ��� �����
    iTextX = ( pstButtonArea->iX1 + iButtonWidth / 2 ) - iTextWidth / 2;
    iTextY = ( pstButtonArea->iY1 + iButtonHeight / 2 ) - FONT_ENGLISHHEIGHT / 2;
    kInternalDrawText( &stArea, pstWindow->pstWindowBuffer, iTextX, iTextY, 
            stTextColor, stBackgroundColor, pcText, iTextLength );      
    
    // ��ư�� ��ü�� ���̰� �׵θ��� �׸�, 2 �ȼ� �β��� �׸�
    // ��ư�� ���ʰ� ���� ��� ǥ��
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            pstButtonArea->iX1, pstButtonArea->iY1, pstButtonArea->iX2,
            pstButtonArea->iY1, WINDOW_COLOR_BUTTONBRIGHT );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            pstButtonArea->iX1, pstButtonArea->iY1 + 1, pstButtonArea->iX2 - 1,
            pstButtonArea->iY1 + 1, WINDOW_COLOR_BUTTONBRIGHT );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            pstButtonArea->iX1, pstButtonArea->iY1, pstButtonArea->iX1,
            pstButtonArea->iY2, WINDOW_COLOR_BUTTONBRIGHT );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            pstButtonArea->iX1 + 1, pstButtonArea->iY1, pstButtonArea->iX1 + 1,
            pstButtonArea->iY2 - 1, WINDOW_COLOR_BUTTONBRIGHT );

    // ��ư�� �����ʰ� �Ʒ��� ��Ӱ� ǥ��
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            pstButtonArea->iX1 + 1, pstButtonArea->iY2, pstButtonArea->iX2,
            pstButtonArea->iY2, WINDOW_COLOR_BUTTONDARK );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            pstButtonArea->iX1 + 2, pstButtonArea->iY2 - 1, pstButtonArea->iX2,
            pstButtonArea->iY2 - 1, WINDOW_COLOR_BUTTONDARK );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            pstButtonArea->iX2, pstButtonArea->iY1 + 1, pstButtonArea->iX2,
            pstButtonArea->iY2, WINDOW_COLOR_BUTTONDARK );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            pstButtonArea->iX2 - 1, pstButtonArea->iY1 + 2, pstButtonArea->iX2 -1,
            pstButtonArea->iY2, WINDOW_COLOR_BUTTONDARK );
    
    // ����ȭ ó��
    kUnlock( &( pstWindow->stLock ) );

    return TRUE;
}

// ���콺 Ŀ���� �̹����� �����ϴ� ������
static BYTE gs_vwMouseBuffer[ MOUSE_CURSOR_WIDTH * MOUSE_CURSOR_HEIGHT ] =
{
    1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 2, 3, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0,
    0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1,
    0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 1, 1,
    0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 1, 1, 0, 0,
    0, 1, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 1, 1, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 2, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 2, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 2, 3, 3, 3, 2, 2, 3, 3, 3, 2, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 2, 3, 3, 2, 1, 1, 2, 3, 2, 2, 2, 1, 0, 0, 0, 0,
    0, 0, 0, 1, 2, 3, 2, 2, 1, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0, 0,
    0, 0, 0, 1, 2, 3, 2, 1, 0, 0, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0,
    0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 1, 0,
    0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 1,
    0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 1, 0,
    0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0,
    0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
};

/**
 *  X, Y ��ġ�� ���콺 Ŀ���� ���
 */
static void kDrawCursor( int iX, int iY )
{
    int i;
    int j;
    BYTE* pbCurrentPos;

    // Ŀ�� �������� ���� ��ġ�� ����
    pbCurrentPos = gs_vwMouseBuffer;

    // Ŀ���� �ʺ�� ���̸�ŭ ������ ���鼭 �ȼ��� ȭ�鿡 ���
    for( j = 0 ; j < MOUSE_CURSOR_HEIGHT ; j++ )
    {
        for( i = 0 ; i < MOUSE_CURSOR_WIDTH ; i++ )
        {
            switch( *pbCurrentPos )
            {
                // 0�� ������� ����
            case 0:
                // nothing
                break;

                // ���� �ٱ��� �׵θ�, ���������� ���
            case 1:
                kInternalDrawPixel( &( gs_stWindowManager.stScreenArea ),
                        gs_stWindowManager.pstVideoMemory, i + iX, j + iY,
                        MOUSE_CURSOR_OUTERLINE );
                break;

                // ���ʰ� �ٱ����� ���, ��ο� ������� ���
            case 2:
                kInternalDrawPixel( &( gs_stWindowManager.stScreenArea ),
                        gs_stWindowManager.pstVideoMemory, i + iX, j + iY,
                        MOUSE_CURSOR_OUTER );
                break;

                // Ŀ���� ��, ���� ������ ���
            case 3:
                kInternalDrawPixel( &( gs_stWindowManager.stScreenArea ),
                        gs_stWindowManager.pstVideoMemory, i + iX, j + iY,
                        MOUSE_CURSOR_INNER );
                break;
            }

            // Ŀ���� �ȼ��� ǥ�õʿ� ���� Ŀ�� �������� ��ġ�� ���� �̵�
            pbCurrentPos++;
        }
    }
}

/**
 *  ���콺 Ŀ���� �ش� ��ġ�� �̵��ؼ� �׷���
 */
void kMoveCursor( int iX, int iY )
{
    RECT stPreviousArea;

    // ���콺 Ŀ���� ȭ���� ����� ���ϵ��� ����
    if( iX < gs_stWindowManager.stScreenArea.iX1 )
    {
        iX = gs_stWindowManager.stScreenArea.iX1;
    }
    else if( iX > gs_stWindowManager.stScreenArea.iX2 )
    {
        iX = gs_stWindowManager.stScreenArea.iX2;
    }

    if( iY < gs_stWindowManager.stScreenArea.iY1 )
    {
        iY = gs_stWindowManager.stScreenArea.iY1;
    }
    else if( iY > gs_stWindowManager.stScreenArea.iY2 )
    {
        iY = gs_stWindowManager.stScreenArea.iY2;
    }

    // ����ȭ ó��
    kLock( &( gs_stWindowManager.stLock ) );
    
    // ������ ���콺 Ŀ���� �ִ� �ڸ��� ����
    stPreviousArea.iX1 = gs_stWindowManager.iMouseX;
    stPreviousArea.iY1 = gs_stWindowManager.iMouseY;
    stPreviousArea.iX2 = gs_stWindowManager.iMouseX + MOUSE_CURSOR_WIDTH - 1;
    stPreviousArea.iY2 = gs_stWindowManager.iMouseY + MOUSE_CURSOR_HEIGHT - 1;
    
    // ���콺 Ŀ���� �� ��ġ�� ����
    gs_stWindowManager.iMouseX = iX;
    gs_stWindowManager.iMouseY = iY;

    // ����ȭ ó��
    kUnlock( &( gs_stWindowManager.stLock ) );
    
    // ���콺�� ������ �ִ� ������ �ٽ� �׸�
    kRedrawWindowByArea( &stPreviousArea );

    // ���ο� ��ġ�� ���콺 Ŀ���� ���
    kDrawCursor( iX, iY );
}

/**
 *  ���� ���콺 Ŀ���� ��ġ�� ��ȯ
 */
void kGetCursorPosition( int* piX, int* piY )
{
    *piX = gs_stWindowManager.iMouseX;
    *piY = gs_stWindowManager.iMouseY;
}

/**
 *  ������ ���ο� �� �׸���
 */
BOOL kDrawPixel( QWORD qwWindowID, int iX, int iY, COLOR stColor )
{
    WINDOW* pstWindow;
    RECT stArea;

    // ������ �˻��� ����ȭ ó��
    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL )
    {
        return FALSE;
    }
    
    // ������ ���� ��ǥ�� 0,0���� �ϴ� ��ǥ�� ������ ��ȯ
    kSetRectangleData( 0, 0, pstWindow->stArea.iX2 - pstWindow->stArea.iX1, 
            pstWindow->stArea.iY2 - pstWindow->stArea.iY1, &stArea );

    // ���� �Լ��� ȣ��
    kInternalDrawPixel( &stArea, pstWindow->pstWindowBuffer, iX, iY,
            stColor );

    // ����ȭ ó��
    kUnlock( &pstWindow->stLock );

    return TRUE;
}


/**
 *  ������ ���ο� ���� �׸���
 */
BOOL kDrawLine( QWORD qwWindowID, int iX1, int iY1, int iX2, int iY2, COLOR stColor )
{
    WINDOW* pstWindow;
    RECT stArea;

    // ������ �˻��� ����ȭ ó��
    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL )
    {
        return FALSE;
    }
    
    // ������ ���� ��ǥ�� 0,0���� �ϴ� ������ ���� ��ǥ�� ������ ��ȯ
    kSetRectangleData( 0, 0, pstWindow->stArea.iX2 - pstWindow->stArea.iX1, 
            pstWindow->stArea.iY2 - pstWindow->stArea.iY1, &stArea );
    
    // ���� �Լ��� ȣ��
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer, iX1, iY1,
            iX2, iY2, stColor );

    // ����ȭ ó��
    kUnlock( &pstWindow->stLock );
    return TRUE;
}

/**
 *  ������ ���ο� �簢�� �׸���
 */
BOOL kDrawRect( QWORD qwWindowID, int iX1, int iY1, int iX2, int iY2,
        COLOR stColor, BOOL bFill )
{
    WINDOW* pstWindow;
    RECT stArea;
    
    // ������ �˻��� ����ȭ ó��
    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL )
    {
        return FALSE;
    }

    // ������ ���� ��ǥ�� 0,0���� �ϴ� ������ ���� ��ǥ�� ������ ��ȯ
    kSetRectangleData( 0, 0, pstWindow->stArea.iX2 - pstWindow->stArea.iX1, 
            pstWindow->stArea.iY2 - pstWindow->stArea.iY1, &stArea );
    
    // ���� �Լ��� ȣ��
    kInternalDrawRect( &stArea, pstWindow->pstWindowBuffer, iX1, iY1,
            iX2, iY2, stColor, bFill );

    // ����ȭ ó��
    kUnlock( &pstWindow->stLock );
    return TRUE;
}

/**
 *  ������ ���ο� �� �׸���
 */
BOOL kDrawCircle( QWORD qwWindowID, int iX, int iY, int iRadius, COLOR stColor,
        BOOL bFill )
{
    WINDOW* pstWindow;
    RECT stArea;
    
    // ������ �˻��� ����ȭ ó��
    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL )
    {
        return FALSE;
    }
    
    // ������ ���� ��ǥ�� 0,0���� �ϴ� ������ ���� ��ǥ�� ������ ��ȯ
    kSetRectangleData( 0, 0, pstWindow->stArea.iX2 - pstWindow->stArea.iX1, 
            pstWindow->stArea.iY2 - pstWindow->stArea.iY1, &stArea );
    
    // ���� �Լ��� ȣ��
    kInternalDrawCircle( &stArea, pstWindow->pstWindowBuffer,
            iX, iY, iRadius, stColor, bFill );

    // ����ȭ ó��
    kUnlock( &pstWindow->stLock );
    return TRUE;
}

/**
 *  ������ ���ο� ���� ���
 */
BOOL kDrawText( QWORD qwWindowID, int iX, int iY, COLOR stTextColor,
        COLOR stBackgroundColor, const char* pcString, int iLength )
{
    WINDOW* pstWindow;
    RECT stArea;

    // ������ �˻��� ����ȭ ó��
    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL )
    {
        return FALSE;
    }
    
    // ������ ���� ��ǥ�� 0,0���� �ϴ� ������ ���� ��ǥ�� ������ ��ȯ
    kSetRectangleData( 0, 0, pstWindow->stArea.iX2 - pstWindow->stArea.iX1, 
            pstWindow->stArea.iY2 - pstWindow->stArea.iY1, &stArea );
    
    // ���� �Լ��� ȣ��
    kInternalDrawText( &stArea, pstWindow->pstWindowBuffer, iX, iY,
            stTextColor, stBackgroundColor, pcString, iLength );

    // ����ȭ ó��
    kUnlock( &pstWindow->stLock );
    return TRUE;
}

