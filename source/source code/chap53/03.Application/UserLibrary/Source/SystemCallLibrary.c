/**
 *  file    SystemCallLibrary.c
 *  date    2009/12/13
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   MINT64 OS���� �����ϴ� �ý��� �ݿ� ���õ� �ҽ� ����
 */

#include "SystemCallLibrary.h"

//==============================================================================
//  �ܼ� I/O ����
//==============================================================================
/**
 *  �ֿܼ� ���ڿ��� ���
 *      printf() �Լ� ���ο��� ��� 
 *      \n, \t�� ���� ���ڰ� ���Ե� ���ڿ��� ����� ��, ȭ����� ���� ����� ��ġ�� 
 *      ��ȯ
 */
int ConsolePrintString( const char* pcBuffer )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) pcBuffer;

    // �ý��� �� ȣ��
    return ExecuteSystemCall( SYSCALL_CONSOLEPRINTSTRING, &stParameter );
}

/**
 *  Ŀ���� ��ġ�� ����
 *      ���ڸ� ����� ��ġ�� ���� ����
 */
BOOL SetCursor( int iX, int iY )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) iX;
    PARAM( 1 ) = ( QWORD ) iY;

    // �ý��� �� ȣ��
    return ExecuteSystemCall( SYSCALL_SETCURSOR, &stParameter );
}

/**
 *  ���� Ŀ���� ��ġ�� ��ȯ
 */
BOOL GetCursor( int *piX, int *piY )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) piX;
    PARAM( 1 ) = ( QWORD ) piY;

    // �ý��� �� ȣ��
    return ExecuteSystemCall( SYSCALL_GETCURSOR, &stParameter );
}

/**
 *  ��ü ȭ���� ����
 */
BOOL ClearScreen( void )
{
    // �ý��� �� ȣ��
    return ExecuteSystemCall( SYSCALL_CLEARSCREEN, NULL );
}

/**
 *  getch() �Լ��� ����
 */
BYTE getch( void )
{
    // �ý��� �� ȣ��
    return ExecuteSystemCall( SYSCALL_GETCH, NULL );
}

//==============================================================================
// ���� �޸� ����
//==============================================================================
/**
 *  �޸𸮸� �Ҵ�
 */
void* malloc( QWORD qwSize )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwSize;

    // �ý��� �� ȣ��
    return ( void* ) ExecuteSystemCall( SYSCALL_MALLOC, &stParameter );
}

/**
 *  �Ҵ� ���� �޸𸮸� ����
 */
BOOL free( void* pvAddress )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) pvAddress;

    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_FREE, &stParameter );    
}

//==============================================================================
// ���ϰ� ���͸� I/O ����
//==============================================================================
/**
 *  ������ ���ų� ���� 
 */
FILE* fopen( const char* pcFileName, const char* pcMode )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) pcFileName;
    PARAM( 1 ) = ( QWORD ) pcMode;

    // �ý��� �� ȣ��
    return ( FILE* ) ExecuteSystemCall( SYSCALL_FOPEN, &stParameter );      
}

/**
 *  ������ �о� ���۷� ����
 */
DWORD fread( void* pvBuffer, DWORD dwSize, DWORD dwCount, FILE* pstFile )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) pvBuffer;
    PARAM( 1 ) = ( QWORD ) dwSize;
    PARAM( 2 ) = ( QWORD ) dwCount;
    PARAM( 3 ) = ( QWORD ) pstFile;

    // �ý��� �� ȣ��
    return ExecuteSystemCall( SYSCALL_FREAD, &stParameter );      
}

/**
 *  ������ �����͸� ���Ͽ� ��
 */
DWORD fwrite( const void* pvBuffer, DWORD dwSize, DWORD dwCount, FILE* pstFile )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) pvBuffer;
    PARAM( 1 ) = ( QWORD ) dwSize;
    PARAM( 2 ) = ( QWORD ) dwCount;
    PARAM( 3 ) = ( QWORD ) pstFile;

    // �ý��� �� ȣ��
    return ExecuteSystemCall( SYSCALL_FWRITE, &stParameter );    
}

/**
 *  ���� �������� ��ġ�� �̵�
 */
int fseek( FILE* pstFile, int iOffset, int iOrigin )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) pstFile;
    PARAM( 1 ) = ( QWORD ) iOffset;
    PARAM( 2 ) = ( QWORD ) iOrigin;

    // �ý��� �� ȣ��
    return ( int ) ExecuteSystemCall( SYSCALL_FSEEK, &stParameter );     
}

/**
 *  ������ ����
 */
int fclose( FILE* pstFile )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) pstFile;

    // �ý��� �� ȣ��
    return ( int ) ExecuteSystemCall( SYSCALL_FCLOSE, &stParameter );      
}

/**
 *  ������ ����
 */
int remove( const char* pcFileName )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) pcFileName;

    // �ý��� �� ȣ��
    return ( int ) ExecuteSystemCall( SYSCALL_REMOVE, &stParameter );          
}

/**
 *  ���͸��� ��
 */
DIR* opendir( const char* pcDirectoryName )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) pcDirectoryName;

    // �ý��� �� ȣ��
    return ( DIR* ) ExecuteSystemCall( SYSCALL_OPENDIR, &stParameter );         
}

/**
 *  ���͸� ��Ʈ���� ��ȯ�ϰ� �������� �̵�
 */
struct dirent* readdir( DIR* pstDirectory )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) pstDirectory;

    // �ý��� �� ȣ��
    return ( struct dirent* ) ExecuteSystemCall( SYSCALL_READDIR, 
                                                               &stParameter );       
}

/**
 *  ���͸� �����͸� ���͸��� ó������ �̵�
 */
BOOL rewinddir( DIR* pstDirectory )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) pstDirectory;

    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_REWINDDIR, &stParameter );          
}

/**
 *  ���� ���͸��� ����
 */
int closedir( DIR* pstDirectory )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) pstDirectory;

    // �ý��� �� ȣ��
    return ( int ) ExecuteSystemCall( SYSCALL_CLOSEDIR, &stParameter );       
}

/**
 *  �ڵ� Ǯ�� �˻��Ͽ� ������ �����ִ����� Ȯ��
 */
BOOL IsFileOpened( const struct dirent* pstEntry )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) pstEntry;

    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_ISFILEOPENED, &stParameter );    
}

//==============================================================================
// �ϵ� ��ũ I/O ����
//==============================================================================
/**
 *  �ϵ� ��ũ�� ���͸� ����
 *      �ִ� 256���� ���͸� ���� �� ����
 *      ������ ���� ���� ���� ��ȯ
 */
int ReadHDDSector( BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, 
        char* pcBuffer )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) bPrimary;
    PARAM( 1 ) = ( QWORD ) bMaster;
    PARAM( 2 ) = ( QWORD ) dwLBA;
    PARAM( 3 ) = ( QWORD ) iSectorCount;
    PARAM( 4 ) = ( QWORD ) pcBuffer;

    // �ý��� �� ȣ��
    return ( int ) ExecuteSystemCall( SYSCALL_READHDDSECTOR, &stParameter );     
}

/**
 *  �ϵ� ��ũ�� ���͸� ��
 *      �ִ� 256���� ���͸� �� �� ����
 *      ������ �� ���� ���� ��ȯ
 */
int WriteHDDSector( BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, 
        char* pcBuffer )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) bPrimary;
    PARAM( 1 ) = ( QWORD ) bMaster;
    PARAM( 2 ) = ( QWORD ) dwLBA;
    PARAM( 3 ) = ( QWORD ) iSectorCount;
    PARAM( 4 ) = ( QWORD ) pcBuffer;

    // �ý��� �� ȣ��
    return ( int ) ExecuteSystemCall( SYSCALL_WRITEHDDSECTOR, &stParameter );      
}

//==============================================================================
// �½�ũ�� �����ٷ� ����
//==============================================================================
/**
 *  �½�ũ�� ����
 */
QWORD CreateTask( QWORD qwFlags, void* pvMemoryAddress, QWORD qwMemorySize, 
                  QWORD qwEntryPointAddress, BYTE bAffinity )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) qwFlags;
    PARAM( 1 ) = ( QWORD ) pvMemoryAddress;
    PARAM( 2 ) = ( QWORD ) qwMemorySize;
    PARAM( 3 ) = ( QWORD ) qwEntryPointAddress;
    PARAM( 4 ) = ( QWORD ) bAffinity;

    // �ý��� �� ȣ��
    return ExecuteSystemCall( SYSCALL_CREATETASK, &stParameter );     
}

/**
 *  �ٸ� �½�ũ�� ã�Ƽ� ��ȯ
 */
BOOL Schedule( void )
{
    // �ý��� �� ȣ��
    return ( BOOL) ExecuteSystemCall( SYSCALL_SCHEDULE, NULL );
}

/**
 *  �½�ũ�� �켱 ������ ����
 */
BOOL ChangePriority( QWORD qwID, BYTE bPriority, BOOL bExecutedInInterrupt )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwID;
    PARAM( 1 ) = ( QWORD ) bPriority;
    PARAM( 2 ) = ( QWORD ) bExecutedInInterrupt;

    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_CHANGEPRIORITY, &stParameter );        
}

/**
 *  �½�ũ�� ����
 */
BOOL EndTask( QWORD qwTaskID )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwTaskID;

    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_ENDTASK, &stParameter );     
}

/**
 *  �½�ũ�� �ڽ��� ������
 */
void exit( int iExitValue )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) iExitValue;

    // �ý��� �� ȣ��
    ExecuteSystemCall( SYSCALL_EXITTASK, &stParameter );      
}

/**
 *  ��ü �½�ũ�� ���� ��ȯ
 */ 
int GetTaskCount( BYTE bAPICID )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) bAPICID;

    // �ý��� �� ȣ��
    return ( int ) ExecuteSystemCall( SYSCALL_GETTASKCOUNT, &stParameter );     
}

/**
 *  �½�ũ�� �����ϴ��� ���θ� ��ȯ
 */
BOOL IsTaskExist( QWORD qwID )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwID;

    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_ISTASKEXIST, &stParameter );       
}

/**
 *  ���μ����� ������ ��ȯ
 */
QWORD GetProcessorLoad( BYTE bAPICID )
{
    PARAMETERTABLE stParameter;
     
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) bAPICID;

    // �ý��� �� ȣ��
    return ExecuteSystemCall( SYSCALL_GETPROCESSORLOAD, &stParameter );   
}

/**
 *  ���μ��� ģȭ���� ����
 */
BOOL ChangeProcessorAffinity( QWORD qwTaskID, BYTE bAffinity )
{
    PARAMETERTABLE stParameter;
     
    // �Ķ���� ����
    PARAM( 0 ) = qwTaskID;
    PARAM( 1 ) = ( QWORD ) bAffinity;

    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_CHANGEPROCESSORAFFINITY, &stParameter );      
}

/**
 *  �������α׷� ����
 */
QWORD ExecuteProgram( const char* pcFileName, const char* pcArgumentString, 
        BYTE bAffinity )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) pcFileName;
    PARAM( 1 ) = ( QWORD ) pcArgumentString;
    PARAM( 2 ) = ( QWORD ) bAffinity;
    
    // �ý��� �� ȣ��
    return ExecuteSystemCall( SYSCALL_EXECUTEPROGRAM, &stParameter );
}

/**
 *  ������ ����
 */
QWORD CreateThread( QWORD qwEntryPoint, QWORD qwArgument, BYTE bAffinity ) 
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) qwEntryPoint;
    PARAM( 1 ) = ( QWORD ) qwArgument;
    PARAM( 2 ) = ( QWORD ) bAffinity;
    // ������ �� ȣ��Ǵ� �Լ��� exit�� �����Ͽ� �����尡 ������ �����ϵ��� ��
    PARAM( 3 ) = ( QWORD ) exit;
    
    // �ý��� �� ȣ��
    return ExecuteSystemCall( SYSCALL_EXECUTEPROGRAM, &stParameter );
}

//==============================================================================
// GUI �ý��� ����
//==============================================================================
/**
 *  ��� �������� ID�� ��ȯ
 */
QWORD GetBackgroundWindowID( void )
{
    // �ý��� �� ȣ��
    return ExecuteSystemCall( SYSCALL_GETBACKGROUNDWINDOWID, NULL );         
}

/**
 *  ȭ�� ������ ũ�⸦ ��ȯ
 */
void GetScreenArea( RECT* pstScreenArea )
{
    PARAMETERTABLE stParameter;
     
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) pstScreenArea;

    // �ý��� �� ȣ��
    ExecuteSystemCall( SYSCALL_GETSCREENAREA, &stParameter );     
}

/**
 *  �����츦 ����
 */
QWORD CreateWindow( int iX, int iY, int iWidth, int iHeight, DWORD dwFlags,
        const char* pcTitle )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) iX;
    PARAM( 1 ) = ( QWORD ) iY;
    PARAM( 2 ) = ( QWORD ) iWidth;
    PARAM( 3 ) = ( QWORD ) iHeight;
    PARAM( 4 ) = ( QWORD ) dwFlags;
    PARAM( 5 ) = ( QWORD ) pcTitle;
    
    // �ý��� �� ȣ��
    return ExecuteSystemCall( SYSCALL_CREATEWINDOW, &stParameter );
}

/**
 *  �����츦 ����
 */
BOOL DeleteWindow( QWORD qwWindowID )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwWindowID;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DELETEWINDOW, &stParameter );    
}

/**
 *  �����츦 ȭ�鿡 ��Ÿ���ų� ����
 */
BOOL ShowWindow( QWORD qwWindowID, BOOL bShow )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) bShow;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_SHOWWINDOW, &stParameter );    
}

/**
 *  Ư�� ��ġ�� �����ϴ� ������ �߿��� ���� ���� �ִ� �����츦 ��ȯ
 */
QWORD FindWindowByPoint( int iX, int iY )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) iX;
    PARAM( 1 ) = ( QWORD ) iY;
    
    // �ý��� �� ȣ��
    return ExecuteSystemCall( SYSCALL_FINDWINDOWBYPOINT, &stParameter );    
}

/**
 *  ������ ������ ��ġ�ϴ� �����츦 ��ȯ
 */
QWORD FindWindowByTitle( const char* pcTitle )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) pcTitle;
    
    // �ý��� �� ȣ��
    return ExecuteSystemCall( SYSCALL_FINDWINDOWBYTITLE, &stParameter );
}

/**
 *  �����찡 �����ϴ��� ���θ� ��ȯ
 */
BOOL IsWindowExist( QWORD qwWindowID )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwWindowID;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_ISWINDOWEXIST, &stParameter );    
}

/**
 *  �ֻ��� �������� ID�� ��ȯ
 */
QWORD GetTopWindowID( void )
{
    // �ý��� �� ȣ��
    return ExecuteSystemCall( SYSCALL_GETTOPWINDOWID, NULL );     
}

/**
 *  �������� Z ������ �ֻ����� ����
 */
BOOL MoveWindowToTop( QWORD qwWindowID )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwWindowID;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_MOVEWINDOWTOTOP, &stParameter );      
}

/**
 *  X, Y��ǥ�� �������� ���� ǥ���� ��ġ�� �ִ����� ��ȯ
 */
BOOL IsInTitleBar( QWORD qwWindowID, int iX, int iY )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX;
    PARAM( 2 ) = ( QWORD ) iY;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_ISINTITLEBAR, &stParameter );     
}

/**
 *  X, Y��ǥ�� �������� �ݱ� ��ư ��ġ�� �ִ����� ��ȯ
 */
BOOL IsInCloseButton( QWORD qwWindowID, int iX, int iY )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX;
    PARAM( 2 ) = ( QWORD ) iY;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_ISINCLOSEBUTTON, &stParameter );     
}

/**
 *  �����츦 �ش� ��ġ�� �̵�
 */
BOOL MoveWindow( QWORD qwWindowID, int iX, int iY )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX;
    PARAM( 2 ) = ( QWORD ) iY;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_MOVEWINDOW, &stParameter );      
}

/**
 *  �������� ũ�⸦ ����
 */
BOOL ResizeWindow( QWORD qwWindowID, int iX, int iY, int iWidth, int iHeight )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX;
    PARAM( 2 ) = ( QWORD ) iY;
    PARAM( 3 ) = ( QWORD ) iWidth;
    PARAM( 4 ) = ( QWORD ) iHeight;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_RESIZEWINDOW, &stParameter );    
}

/**
 *  ������ ������ ��ȯ
 */
BOOL GetWindowArea( QWORD qwWindowID, RECT* pstArea )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) pstArea;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_GETWINDOWAREA, &stParameter );    
}

/**
 *  �����츦 ȭ�鿡 ������Ʈ
 *      �½�ũ���� ����ϴ� �Լ�
 */
BOOL UpdateScreenByID( QWORD qwWindowID )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwWindowID;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_UPDATESCREENBYID, &stParameter );      
}

/**
 *  �������� ���θ� ȭ�鿡 ������Ʈ
 *      �½�ũ���� ����ϴ� �Լ�
 */
BOOL UpdateScreenByWindowArea( QWORD qwWindowID, const RECT* pstArea )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) pstArea;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_UPDATESCREENBYWINDOWAREA, &stParameter );     
}

/**
 *  ȭ�� ��ǥ�� ȭ���� ������Ʈ
 *      �½�ũ���� ����ϴ� �Լ�
 */
BOOL UpdateScreenByScreenArea( const RECT* pstArea )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) pstArea;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_UPDATESCREENBYSCREENAREA, &stParameter );      
}

/**
 *  ������� �̺�Ʈ�� ����
 */
BOOL SendEventToWindow( QWORD qwWindowID, const EVENT* pstEvent )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) pstEvent;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_SENDEVENTTOWINDOW, &stParameter );      
}

/**
 *  �������� �̺�Ʈ ť�� ����� �̺�Ʈ�� ����
 */
BOOL ReceiveEventFromWindowQueue( QWORD qwWindowID, EVENT* pstEvent )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) pstEvent;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_RECEIVEEVENTFROMWINDOWQUEUE, &stParameter );         
}

/**
 *  ������ ȭ�� ���ۿ� ������ �׵θ� �׸���
 */
BOOL DrawWindowFrame( QWORD qwWindowID )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwWindowID;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWWINDOWFRAME, &stParameter );      
}

/**
 *  ������ ȭ�� ���ۿ� ��� �׸���
 */
BOOL DrawWindowBackground( QWORD qwWindowID )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwWindowID;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWWINDOWBACKGROUND, &stParameter );     
}

/**
 *  ������ ȭ�� ���ۿ� ������ ���� ǥ���� �׸���
 */
BOOL DrawWindowTitle( QWORD qwWindowID, const char* pcTitle, BOOL bSelectedTitle )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) pcTitle;
    PARAM( 2 ) = ( QWORD ) bSelectedTitle;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWWINDOWTITLE, &stParameter );        
}

/**
 *  ������ ���ο� ��ư �׸���
 */
BOOL DrawButton( QWORD qwWindowID, RECT* pstButtonArea, COLOR stBackgroundColor,
        const char* pcText, COLOR stTextColor )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) pstButtonArea;
    PARAM( 2 ) = ( QWORD ) stBackgroundColor;
    PARAM( 3 ) = ( QWORD ) pcText;
    PARAM( 4 ) = ( QWORD ) stTextColor;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWBUTTON, &stParameter );      
}

/**
 *  ������ ���ο� �� �׸���
 */
BOOL DrawPixel( QWORD qwWindowID, int iX, int iY, COLOR stColor )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX;
    PARAM( 2 ) = ( QWORD ) iY;
    PARAM( 3 ) = ( QWORD ) stColor;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWPIXEL, &stParameter );     
}

/**
 *  ������ ���ο� ���� �׸���
 */
BOOL DrawLine( QWORD qwWindowID, int iX1, int iY1, int iX2, int iY2, COLOR stColor )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX1;
    PARAM( 2 ) = ( QWORD ) iY1;
    PARAM( 3 ) = ( QWORD ) iX2;
    PARAM( 4 ) = ( QWORD ) iY2;
    PARAM( 5 ) = ( QWORD ) stColor;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWLINE, &stParameter );     
}

/**
 *  ������ ���ο� �簢�� �׸���
 */
BOOL DrawRect( QWORD qwWindowID, int iX1, int iY1, int iX2, int iY2,
        COLOR stColor, BOOL bFill )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX1;
    PARAM( 2 ) = ( QWORD ) iY1;
    PARAM( 3 ) = ( QWORD ) iX2;
    PARAM( 4 ) = ( QWORD ) iY2;
    PARAM( 5 ) = ( QWORD ) stColor;
    PARAM( 6 ) = ( QWORD ) bFill;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWRECT, &stParameter );       
}

/**
 *  ������ ���ο� �� �׸���
 */
BOOL DrawCircle( QWORD qwWindowID, int iX, int iY, int iRadius, COLOR stColor,
        BOOL bFill )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX;
    PARAM( 2 ) = ( QWORD ) iY;
    PARAM( 3 ) = ( QWORD ) iRadius;
    PARAM( 4 ) = ( QWORD ) stColor;
    PARAM( 5 ) = ( QWORD ) bFill;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWCIRCLE, &stParameter );      
}

/**
 *  ������ ���ο� ���� ���
 */
BOOL DrawText( QWORD qwWindowID, int iX, int iY, COLOR stTextColor,
        COLOR stBackgroundColor, const char* pcString, int iLength )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX;
    PARAM( 2 ) = ( QWORD ) iY;
    PARAM( 3 ) = ( QWORD ) stTextColor;
    PARAM( 4 ) = ( QWORD ) stBackgroundColor;
    PARAM( 5 ) = ( QWORD ) pcString;
    PARAM( 6 ) = ( QWORD ) iLength;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWTEXT, &stParameter );     
}

/**
 *  ���콺 Ŀ���� �ش� ��ġ�� �̵��ؼ� �׷���
 */
void MoveCursor( int iX, int iY )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) iX;
    PARAM( 1 ) = ( QWORD ) iY;
    
    // �ý��� �� ȣ��
    ExecuteSystemCall( SYSCALL_MOVECURSOR, &stParameter );        
}

/**
 *  ���� ���콺 Ŀ���� ��ġ�� ��ȯ
 */
void GetCursorPosition( int* piX, int* piY )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) piX;
    PARAM( 1 ) = ( QWORD ) piY;
    
    // �ý��� �� ȣ��
    ExecuteSystemCall( SYSCALL_GETCURSORPOSITION, &stParameter );       
}

/**
 *  ������ ȭ�� ���ۿ� ������ ������ �ѹ��� ����
 *      X, Y ��ǥ�� ������ ���� ���� ����
 */
BOOL BitBlt( QWORD qwWindowID, int iX, int iY, COLOR* pstBuffer, int iWidth, 
        int iHeight )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX;
    PARAM( 2 ) = ( QWORD ) iY;
    PARAM( 3 ) = ( QWORD ) pstBuffer;
    PARAM( 4 ) = ( QWORD ) iWidth;
    PARAM( 5 ) = ( QWORD ) iHeight;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_BITBLT, &stParameter );        
}

//==============================================================================
// JPEG ��� ����
//==============================================================================
/**
 *  JPEG �̹��� ������ ��ü�� ��� ���� ���ۿ� ũ�⸦ �̿��ؼ� JPEG �ڷᱸ���� �ʱ�ȭ
 *      ���� ������ ������ �м��Ͽ� �̹��� ��ü�� ũ��� ��Ÿ ������ JPEG �ڷᱸ���� ���� 
 */
BOOL JPEGInit(JPEG *jpeg, BYTE* pbFileBuffer, DWORD dwFileSize)
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) jpeg;
    PARAM( 1 ) = ( QWORD ) pbFileBuffer;
    PARAM( 2 ) = ( QWORD ) dwFileSize;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_JPEGINIT, &stParameter );     
}

/**
 *  JPEG �ڷᱸ���� ����� ������ �̿��Ͽ� ���ڵ��� ����� ��� ���ۿ� ����
 */
BOOL JPEGDecode(JPEG *jpeg, COLOR* rgb)
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) jpeg;
    PARAM( 1 ) = ( QWORD ) rgb;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_JPEGDECODE, &stParameter );       
}

//==============================================================================
// RTC ����
//==============================================================================
/**
 *  CMOS �޸𸮿��� RTC ��Ʈ�ѷ��� ������ ���� �ð��� ����
 */
BOOL ReadRTCTime( BYTE* pbHour, BYTE* pbMinute, BYTE* pbSecond )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) pbHour;
    PARAM( 1 ) = ( QWORD ) pbMinute;
    PARAM( 2 ) = ( QWORD ) pbSecond;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_READRTCTIME, &stParameter );        
}

/**
 *  CMOS �޸𸮿��� RTC ��Ʈ�ѷ��� ������ ���� ���ڸ� ����
 */
BOOL ReadRTCDate( WORD* pwYear, BYTE* pbMonth, BYTE* pbDayOfMonth, 
                  BYTE* pbDayOfWeek )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) pwYear;
    PARAM( 1 ) = ( QWORD ) pbMonth;
    PARAM( 2 ) = ( QWORD ) pbDayOfMonth;
    PARAM( 3 ) = ( QWORD ) pbDayOfWeek;
    
    // �ý��� �� ȣ��
    return ( BOOL ) ExecuteSystemCall( SYSCALL_READRTCDATE, &stParameter );      
}

//==============================================================================
// �ø��� ��� ����
//==============================================================================
/**
 *  �ø��� ��Ʈ�� �����͸� �۽�
 */
void SendSerialData( BYTE* pbBuffer, int iSize )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) pbBuffer;
    PARAM( 1 ) = ( QWORD ) iSize;
    
    // �ý��� �� ȣ��
    ExecuteSystemCall( SYSCALL_SENDSERIALDATA, &stParameter );
}

/**
 *  �ø��� ��Ʈ���� �����͸� ����
 */
int ReceiveSerialData( BYTE* pbBuffer, int iSize )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = ( QWORD ) pbBuffer;
    PARAM( 1 ) = ( QWORD ) iSize;
    
    // �ý��� �� ȣ��
    return ( int ) ExecuteSystemCall( SYSCALL_RECEIVESERIALDATA, &stParameter );
}

/**
 *  �ø��� ��Ʈ ��Ʈ�ѷ��� FIFO�� �ʱ�ȭ
 */
void ClearSerialFIFO( void )
{
    // �ý��� �� ȣ��
    ExecuteSystemCall( SYSCALL_CLEARSERIALFIFO, NULL );
}

//==============================================================================
// ��Ÿ
//==============================================================================
/**
 *  RAM ũ�⸦ ��ȯ
 */
QWORD GetTotalRAMSize( void )
{
    // �ý��� �� ȣ��
    return ExecuteSystemCall( SYSCALL_GETTOTALRAMSIZE, NULL );
}

/**
 *  Tick Count�� ��ȯ
 */
QWORD GetTickCount( void )
{
    // �ý��� �� ȣ��
    return ExecuteSystemCall( SYSCALL_GETTICKCOUNT, NULL );
}

/**
 *  �и�������(milisecond) ���� ���
 */
void Sleep( QWORD qwMillisecond )
{
    PARAMETERTABLE stParameter;
    
    // �Ķ���� ����
    PARAM( 0 ) = qwMillisecond;
    
    // �ý��� �� ȣ��
    ExecuteSystemCall( SYSCALL_SLEEP, &stParameter );    
}

/**
 *  �׷��� ������� ���θ� ��ȯ
 */
BOOL IsGraphicMode( void )
{
    // �ý��� �� ȣ��
    ExecuteSystemCall( SYSCALL_ISGRAPHICMODE, NULL );    
}
