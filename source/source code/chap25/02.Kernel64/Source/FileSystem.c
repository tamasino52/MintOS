/**
 *  file    FileSystem.c
 *  date    2009/05/01
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief  ���� �ý��ۿ� ���õ� ��� ����
 */

#include "FileSystem.h"
#include "HardDisk.h"
#include "DynamicMemory.h"

// ���� �ý��� �ڷᱸ��
static FILESYSTEMMANAGER   gs_stFileSystemManager;
// ���� �ý��� �ӽ� ����
static BYTE gs_vbTempBuffer[ FILESYSTEM_SECTORSPERCLUSTER * 512 ];

// �ϵ� ��ũ ��� ���õ� �Լ� ������ ����
fReadHDDInformation gs_pfReadHDDInformation = NULL;
fReadHDDSector gs_pfReadHDDSector = NULL;
fWriteHDDSector gs_pfWriteHDDSector = NULL;

/**
 *  ���� �ý����� �ʱ�ȭ
 */
BOOL kInitializeFileSystem( void )
{
    // �ڷᱸ�� �ʱ�ȭ�� ����ȭ ��ü �ʱ�ȭ
    kMemSet( &gs_stFileSystemManager, 0, sizeof( gs_stFileSystemManager ) );
    kInitializeMutex( &( gs_stFileSystemManager.stMutex ) );
    
    // �ϵ� ��ũ�� �ʱ�ȭ
    if( kInitializeHDD() == TRUE )
    {
        // �ʱ�ȭ�� �����ϸ� �Լ� �����͸� �ϵ� ��ũ�� �Լ��� ����
        gs_pfReadHDDInformation = kReadHDDInformation;
        gs_pfReadHDDSector = kReadHDDSector;
        gs_pfWriteHDDSector = kWriteHDDSector;
    }
    else
    {
        return FALSE;
    }
    
    // ���� �ý��� ����
    if( kMount() == FALSE )
    {
        return FALSE;
    }
    
    return TRUE;
}

//==============================================================================
//  ������ �Լ�(Low Level Function)
//==============================================================================
/**
 *  �ϵ� ��ũ�� MBR�� �о MINT ���� �ý������� Ȯ��
 *      MINT ���� �ý����̶�� ���� �ý��ۿ� ���õ� ���� ������ �о
 *      �ڷᱸ���� ����
 */
BOOL kMount( void )
{
    MBR* pstMBR;
    
    // ����ȭ ó��
    kLock( &( gs_stFileSystemManager.stMutex ) );

    // MBR�� ����
    if( gs_pfReadHDDSector( TRUE, TRUE, 0, 1, gs_vbTempBuffer ) == FALSE )
    {
        // ����ȭ ó��
        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return FALSE;
    }
    
    // �ñ׳�ó�� Ȯ���Ͽ� ���ٸ� �ڷᱸ���� �� ������ ���� ���� ����
    pstMBR = ( MBR* ) gs_vbTempBuffer;
    if( pstMBR->dwSignature != FILESYSTEM_SIGNATURE )
    {
        // ����ȭ ó��
        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return FALSE;
    }
    
    // ���� �ý��� �ν� ����
    gs_stFileSystemManager.bMounted = TRUE;
    
    // �� ������ ���� LBA ��巹���� ���� ���� ���
    gs_stFileSystemManager.dwReservedSectorCount = pstMBR->dwReservedSectorCount;
    gs_stFileSystemManager.dwClusterLinkAreaStartAddress =
        pstMBR->dwReservedSectorCount + 1;
    gs_stFileSystemManager.dwClusterLinkAreaSize = pstMBR->dwClusterLinkSectorCount;
    gs_stFileSystemManager.dwDataAreaStartAddress = 
        pstMBR->dwReservedSectorCount + pstMBR->dwClusterLinkSectorCount + 1;
    gs_stFileSystemManager.dwTotalClusterCount = pstMBR->dwTotalClusterCount;

    // ����ȭ ó��
    kUnlock( &( gs_stFileSystemManager.stMutex ) );
    return TRUE;
}

/**
 *  �ϵ� ��ũ�� ���� �ý����� ����
 */
BOOL kFormat( void )
{
    HDDINFORMATION* pstHDD;
    MBR* pstMBR;
    DWORD dwTotalSectorCount, dwRemainSectorCount;
    DWORD dwMaxClusterCount, dwClsuterCount;
    DWORD dwClusterLinkSectorCount;
    DWORD i;
    
    // ����ȭ ó��
    kLock( &( gs_stFileSystemManager.stMutex ) );

    //==========================================================================
    //  �ϵ� ��ũ ������ �о ��Ÿ ������ ũ��� Ŭ�������� ������ ���
    //==========================================================================
    // �ϵ� ��ũ�� ������ �� �ϵ� ��ũ�� �� ���� ���� ����
    pstHDD = ( HDDINFORMATION* ) gs_vbTempBuffer;
    if( gs_pfReadHDDInformation( TRUE, TRUE, pstHDD ) == FALSE )
    {
        // ����ȭ ó��
        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return FALSE;
    }    
    dwTotalSectorCount = pstHDD->dwTotalSectors;
    
    // ��ü ���� ���� 4Kbyte, �� Ŭ������ ũ��� ������ �ִ� Ŭ������ ���� ���
    dwMaxClusterCount = dwTotalSectorCount / FILESYSTEM_SECTORSPERCLUSTER;
    
    // �ִ� Ŭ�������� ���� ���߾� Ŭ������ ��ũ ���̺��� ���� ���� ���
    // ��ũ �����ʹ� 4����Ʈ�̹Ƿ�, �� ���Ϳ��� 128���� ��. ���� �� ������
    // 128�� ���� �� �ø��Ͽ� Ŭ������ ��ũ�� ���� ���� ����
    dwClusterLinkSectorCount = ( dwMaxClusterCount + 127 ) / 128;
    
    // ����� ������ ���� ������� �����Ƿ�, ��ũ ��ü �������� MBR ������ Ŭ������
    // ��ũ ���̺� ������ ũ�⸦ �� �������� ���� ������ ������ ��
    // �ش� ������ Ŭ������ ũ��� ������ ���� Ŭ�������� ������ ����
    dwRemainSectorCount = dwTotalSectorCount - dwClusterLinkSectorCount - 1;
    dwClsuterCount = dwRemainSectorCount / FILESYSTEM_SECTORSPERCLUSTER;
    
    // ���� ��� ������ Ŭ������ ���� ���߾� �ٽ� �ѹ� ���
    dwClusterLinkSectorCount = ( dwClsuterCount + 127 ) / 128;

    //==========================================================================
    // ���� ������ MBR�� ���� ����, ��Ʈ ���͸� �������� ��� 0���� �ʱ�ȭ�Ͽ�
    // ���� �ý����� ����
    //==========================================================================
    // MBR ���� �б�
    if( gs_pfReadHDDSector( TRUE, TRUE, 0, 1, gs_vbTempBuffer ) == FALSE )
    {
        // ����ȭ ó��
        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return FALSE;
    }        
    
    // ��Ƽ�� ������ ���� �ý��� ���� ����    
    pstMBR = ( MBR* ) gs_vbTempBuffer;
    kMemSet( pstMBR->vstPartition, 0, sizeof( pstMBR->vstPartition ) );
    pstMBR->dwSignature = FILESYSTEM_SIGNATURE;
    pstMBR->dwReservedSectorCount = 0;
    pstMBR->dwClusterLinkSectorCount = dwClusterLinkSectorCount;
    pstMBR->dwTotalClusterCount = dwClsuterCount;
    
    // MBR ������ 1 ���͸� ��
    if( gs_pfWriteHDDSector( TRUE, TRUE, 0, 1, gs_vbTempBuffer ) == FALSE )
    {
        // ����ȭ ó��
        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return FALSE;
    }
    
    // MBR ���ĺ��� ��Ʈ ���͸����� ��� 0���� �ʱ�ȭ
    kMemSet( gs_vbTempBuffer, 0, 512 );
    for( i = 0 ; i < ( dwClusterLinkSectorCount + FILESYSTEM_SECTORSPERCLUSTER );
         i++ )
    {
        // ��Ʈ ���͸�(Ŭ������ 0)�� �̹� ���� �ý����� ����ϰ� �����Ƿ�,
        // �Ҵ�� ������ ǥ��
        if( i == 0 )
        {
            ( ( DWORD* ) ( gs_vbTempBuffer ) )[ 0 ] = FILESYSTEM_LASTCLUSTER;
        }
        else
        {
            ( ( DWORD* ) ( gs_vbTempBuffer ) )[ 0 ] = FILESYSTEM_FREECLUSTER;
        }
        
        // 1 ���;� ��
        if( gs_pfWriteHDDSector( TRUE, TRUE, i + 1, 1, gs_vbTempBuffer ) == FALSE )
        {
            // ����ȭ ó��
            kUnlock( &( gs_stFileSystemManager.stMutex ) );
            return FALSE;
        }
    }    
    
    // ����ȭ ó��
    kUnlock( &( gs_stFileSystemManager.stMutex ) );
    return TRUE;
}

/**
 *  ���� �ý��ۿ� ����� �ϵ� ��ũ�� ������ ��ȯ
 */
BOOL kGetHDDInformation( HDDINFORMATION* pstInformation)
{
    BOOL bResult;
    
    // ����ȭ ó��
    kLock( &( gs_stFileSystemManager.stMutex ) );
    
    bResult = gs_pfReadHDDInformation( TRUE, TRUE, pstInformation );
    
    // ����ȭ ó��
    kUnlock( &( gs_stFileSystemManager.stMutex ) );
    
    return bResult;
}

/**
 *  Ŭ������ ��ũ ���̺� ���� �����¿��� �� ���͸� ����
 */
BOOL kReadClusterLinkTable( DWORD dwOffset, BYTE* pbBuffer )
{
    // Ŭ������ ��ũ ���̺� ������ ���� ��巹���� ����
    return gs_pfReadHDDSector( TRUE, TRUE, dwOffset + 
              gs_stFileSystemManager.dwClusterLinkAreaStartAddress, 1, pbBuffer );
}

/**
 *  Ŭ������ ��ũ ���̺� ���� �����¿� �� ���͸� ��
 */
BOOL kWriteClusterLinkTable( DWORD dwOffset, BYTE* pbBuffer )
{
    // Ŭ������ ��ũ ���̺� ������ ���� ��巹���� ����
    return gs_pfWriteHDDSector( TRUE, TRUE, dwOffset + 
               gs_stFileSystemManager.dwClusterLinkAreaStartAddress, 1, pbBuffer );
}

/**
 *  ������ ������ �����¿��� �� Ŭ�����͸� ����
 */
BOOL kReadCluster( DWORD dwOffset, BYTE* pbBuffer )
{
    // ������ ������ ���� ��巹���� ����
    return gs_pfReadHDDSector( TRUE, TRUE, ( dwOffset * FILESYSTEM_SECTORSPERCLUSTER ) + 
              gs_stFileSystemManager.dwDataAreaStartAddress, 
              FILESYSTEM_SECTORSPERCLUSTER, pbBuffer );
}

/**
 *  ������ ������ �����¿� �� Ŭ�����͸� ��
 */
BOOL kWriteCluster( DWORD dwOffset, BYTE* pbBuffer )
{
    // ������ ������ ���� ��巹���� ����
    return gs_pfWriteHDDSector( TRUE, TRUE, ( dwOffset * FILESYSTEM_SECTORSPERCLUSTER ) + 
              gs_stFileSystemManager.dwDataAreaStartAddress, 
              FILESYSTEM_SECTORSPERCLUSTER, pbBuffer );
}

/**
 *  Ŭ������ ��ũ ���̺� �������� �� Ŭ�����͸� �˻���
 */
DWORD kFindFreeCluster( void )
{
    DWORD dwLinkCountInSector;
    DWORD dwLastSectorOffset, dwCurrentSectorOffset;
    DWORD i, j;
    
    // ���� �ý����� �ν����� �������� ����
    if( gs_stFileSystemManager.bMounted == FALSE )
    {
        return FILESYSTEM_LASTCLUSTER;
    }
    
    // ���������� Ŭ�����͸� �Ҵ��� Ŭ������ ��ũ ���̺��� ���� �������� ������
    dwLastSectorOffset = gs_stFileSystemManager.dwLastAllocatedClusterLinkSectorOffset;

    // ���������� �Ҵ��� ��ġ���� ������ ���鼭 �� Ŭ�����͸� �˻�
    for( i = 0 ; i < gs_stFileSystemManager.dwClusterLinkAreaSize ; i++ )
    {
        // Ŭ������ ��ũ ���̺��� ������ �����̸� ��ü ���͸�ŭ ���� ���� �ƴ϶�
        // ���� Ŭ�������� ����ŭ ������ ���ƾ� ��
        if( ( dwLastSectorOffset + i ) == 
            ( gs_stFileSystemManager.dwClusterLinkAreaSize - 1 ) )
        {
            dwLinkCountInSector = gs_stFileSystemManager.dwTotalClusterCount % 128; 
        }
        else
        {
            dwLinkCountInSector = 128;
        }
        
        // �̹��� �о�� �� Ŭ������ ��ũ ���̺��� ���� �������� ���ؼ� ����
        dwCurrentSectorOffset = ( dwLastSectorOffset + i ) % 
            gs_stFileSystemManager.dwClusterLinkAreaSize;
        if( kReadClusterLinkTable( dwCurrentSectorOffset, gs_vbTempBuffer ) == FALSE )
        {
            return FILESYSTEM_LASTCLUSTER;
        }
        
        // ���� ������ ������ ���鼭 �� Ŭ�����͸� �˻�
        for( j = 0 ; j < dwLinkCountInSector ; j++ )
        {
            if( ( ( DWORD* ) gs_vbTempBuffer )[ j ] == FILESYSTEM_FREECLUSTER )
            {
                break;
            }
        }
            
        // ã�Ҵٸ� Ŭ������ �ε����� ��ȯ
        if( j != dwLinkCountInSector )
        {
            // ���������� Ŭ�����͸� �Ҵ��� Ŭ������ ��ũ ���� ���� �������� ����
            gs_stFileSystemManager.dwLastAllocatedClusterLinkSectorOffset = 
                dwCurrentSectorOffset;
            
            // ���� Ŭ������ ��ũ ���̺��� �������� �����Ͽ� Ŭ������ �ε����� ���
            return ( dwCurrentSectorOffset * 128 ) + j;
        }
    }
    
    return FILESYSTEM_LASTCLUSTER;
}

/**
 *  Ŭ������ ��ũ ���̺� ���� ����
 */
BOOL kSetClusterLinkData( DWORD dwClusterIndex, DWORD dwData )
{
    DWORD dwSectorOffset;
    
    // ���� �ý����� �ν����� �������� ����
    if( gs_stFileSystemManager.bMounted == FALSE )
    {
        return FALSE;
    }
    
    // �� ���Ϳ� 128���� Ŭ������ ��ũ�� ���Ƿ� 128�� ������ ���� �������� 
    // ���� �� ����
    dwSectorOffset = dwClusterIndex / 128;

    // �ش� ���͸� �о ��ũ ������ ������ ��, �ٽ� ����
    if( kReadClusterLinkTable( dwSectorOffset, gs_vbTempBuffer ) == FALSE )
    {
        return FALSE;
    }    
    
    ( ( DWORD* ) gs_vbTempBuffer )[ dwClusterIndex % 128 ] = dwData;

    if( kWriteClusterLinkTable( dwSectorOffset, gs_vbTempBuffer ) == FALSE )
    {
        return FALSE;
    }

    return TRUE;
}

/**
 *  Ŭ������ ��ũ ���̺��� ���� ��ȯ
 */
BOOL kGetClusterLinkData( DWORD dwClusterIndex, DWORD* pdwData )
{
    DWORD dwSectorOffset;
    
    // ���� �ý����� �ν����� �������� ����
    if( gs_stFileSystemManager.bMounted == FALSE )
    {
        return FALSE;
    }
    
    // �� ���Ϳ� 128���� Ŭ������ ��ũ�� ���Ƿ� 128�� ������ ���� �������� 
    // ���� �� ����
    dwSectorOffset = dwClusterIndex / 128;
    
    if( dwSectorOffset > gs_stFileSystemManager.dwClusterLinkAreaSize )
    {
        return FALSE;
    }
    
    
    // �ش� ���͸� �о ��ũ ������ ��ȯ
    if( kReadClusterLinkTable( dwSectorOffset, gs_vbTempBuffer ) == FALSE )
    {
        return FALSE;
    }    

    *pdwData = ( ( DWORD* ) gs_vbTempBuffer )[ dwClusterIndex % 128 ];
    return TRUE;
}


/**
 *  ��Ʈ ���͸����� �� ���͸� ��Ʈ���� ��ȯ
 */
int kFindFreeDirectoryEntry( void )
{
    DIRECTORYENTRY* pstEntry;
    int i;

    // ���� �ý����� �ν����� �������� ����
    if( gs_stFileSystemManager.bMounted == FALSE )
    {
        return -1;
    }

    // ��Ʈ ���͸��� ����
    if( kReadCluster( 0, gs_vbTempBuffer ) == FALSE )
    {
        return -1;
    }
    
    // ��Ʈ ���͸� �ȿ��� ������ ���鼭 �� ��Ʈ��, �� ���� Ŭ������ ��ȣ�� 0��
    // ��Ʈ���� �˻�
    pstEntry = ( DIRECTORYENTRY* ) gs_vbTempBuffer;
    for( i = 0 ; i < FILESYSTEM_MAXDIRECTORYENTRYCOUNT ; i++ )
    {
        if( pstEntry[ i ].dwStartClusterIndex == 0 )
        {
            return i;
        }
    }
    return -1;
}

/**
 *  ��Ʈ ���͸��� �ش� �ε����� ���͸� ��Ʈ���� ����
 */
BOOL kSetDirectoryEntryData( int iIndex, DIRECTORYENTRY* pstEntry )
{
    DIRECTORYENTRY* pstRootEntry;
    
    // ���� �ý����� �ν����� ���߰ų� �ε����� �ùٸ��� ������ ����
    if( ( gs_stFileSystemManager.bMounted == FALSE ) ||
        ( iIndex < 0 ) || ( iIndex >= FILESYSTEM_MAXDIRECTORYENTRYCOUNT ) )
    {
        return FALSE;
    }

    // ��Ʈ ���͸��� ����
    if( kReadCluster( 0, gs_vbTempBuffer ) == FALSE )
    {
        return FALSE;
    }    
    
    // ��Ʈ ���͸��� �ִ� �ش� �����͸� ����
    pstRootEntry = ( DIRECTORYENTRY* ) gs_vbTempBuffer;
    kMemCpy( pstRootEntry + iIndex, pstEntry, sizeof( DIRECTORYENTRY ) );

    // ��Ʈ ���͸��� ��
    if( kWriteCluster( 0, gs_vbTempBuffer ) == FALSE )
    {
        return FALSE;
    }    
    return TRUE;
}

/**
 *  ��Ʈ ���͸��� �ش� �ε����� ��ġ�ϴ� ���͸� ��Ʈ���� ��ȯ
 */
BOOL kGetDirectoryEntryData( int iIndex, DIRECTORYENTRY* pstEntry )
{
    DIRECTORYENTRY* pstRootEntry;
    
    // ���� �ý����� �ν����� ���߰ų� �ε����� �ùٸ��� ������ ����
    if( ( gs_stFileSystemManager.bMounted == FALSE ) ||
        ( iIndex < 0 ) || ( iIndex >= FILESYSTEM_MAXDIRECTORYENTRYCOUNT ) )
    {
        return FALSE;
    }

    // ��Ʈ ���͸��� ����
    if( kReadCluster( 0, gs_vbTempBuffer ) == FALSE )
    {
        return FALSE;
    }    
    
    // ��Ʈ ���͸��� �ִ� �ش� �����͸� ����
    pstRootEntry = ( DIRECTORYENTRY* ) gs_vbTempBuffer;
    kMemCpy( pstEntry, pstRootEntry + iIndex, sizeof( DIRECTORYENTRY ) );
    return TRUE;
}

/**
 *  ��Ʈ ���͸����� ���� �̸��� ��ġ�ϴ� ��Ʈ���� ã�Ƽ� �ε����� ��ȯ
 */
int kFindDirectoryEntry( const char* pcFileName, DIRECTORYENTRY* pstEntry )
{
    DIRECTORYENTRY* pstRootEntry;
    int i;
    int iLength;

    // ���� �ý����� �ν����� �������� ����
    if( gs_stFileSystemManager.bMounted == FALSE )
    {
        return -1;
    }

    // ��Ʈ ���͸��� ����
    if( kReadCluster( 0, gs_vbTempBuffer ) == FALSE )
    {
        return -1;
    }
    
    iLength = kStrLen( pcFileName );
    // ��Ʈ ���͸� �ȿ��� ������ ���鼭 ���� �̸��� ��ġ�ϴ� ��Ʈ���� ��ȯ
    pstRootEntry = ( DIRECTORYENTRY* ) gs_vbTempBuffer;
    for( i = 0 ; i < FILESYSTEM_MAXDIRECTORYENTRYCOUNT ; i++ )
    {
        if( kMemCmp( pstRootEntry[ i ].vcFileName, pcFileName, iLength ) == 0 )
        {
            kMemCpy( pstEntry, pstRootEntry + i, sizeof( DIRECTORYENTRY ) );
            return i;
        }
    }
    return -1;
}

/**
 *  ���� �ý����� ������ ��ȯ
 */
void kGetFileSystemInformation( FILESYSTEMMANAGER* pstManager )
{
    kMemCpy( pstManager, &gs_stFileSystemManager, sizeof( gs_stFileSystemManager ) );
}
