/**
 *  file    PackageMaker.c
 *  date    2010/03/31
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   �������α׷��� ���� ������ ������ ���� ��Ű�� ���·� ������ִ�
 *          PackageMaker�� �ҽ� ����
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

// ��ũ�� ����
// �� ������ ����Ʈ ��
#define BYTESOFSECTOR       512

// ��Ű���� �ñ׳�ó
#define PACKAGESIGNATURE    "MINT64OSPACKAGE "

// ���� �̸��� �ִ� ����, Ŀ���� FILESYSTEM_MAXFILENAMELENGTH�� ����
#define MAXFILENAMELENGTH   24

// DWORD Ÿ���� ����
#define DWORD               unsigned int


// �ڷ� ���� ����
// 1����Ʈ�� ����
#pragma pack( push, 1 )

// ��Ű�� ��� ������ �� ���� ������ �����ϴ� �ڷᱸ��
typedef struct PackageItemStruct
{
    // ���� �̸�
    char vcFileName[ MAXFILENAMELENGTH ];

    // ������ ũ��
    DWORD dwFileLength;
} PACKAGEITEM;

// ��Ű�� ��� �ڷᱸ��
typedef struct PackageHeaderStruct
{
    // MINT64 OS�� ��Ű�� ������ ��Ÿ���� �ñ׳�ó
    char vcSignature[ 16 ];

    // ��Ű�� ����� ��ü ũ��
    DWORD dwHeaderSize;

    // ��Ű�� �������� ���� ��ġ
    PACKAGEITEM vstItem[ 0 ];
} PACKAGEHEADER;

#pragma pack( pop )


// �Լ� ����
int AdjustInSectorSize( int iFd, int iSourceSize );
int CopyFile( int iSourceFd, int iTargetFd );

/**
 *  Main �Լ�
*/
int main(int argc, char* argv[])
{
    int iSourceFd;
    int iTargetFd;
    int iSourceSize;
    int i;
    struct stat stFileData;
    PACKAGEHEADER stHeader;
    PACKAGEITEM stItem;
        
    // Ŀ�ǵ� ���� �ɼ� �˻�
    if( argc < 2 )
    {
        fprintf( stderr, "[ERROR] PackageMaker.exe app1.elf app2.elf data.txt ...\n" );
        exit( -1 );
    }
    
    // Package.img ������ ����
    if( ( iTargetFd = open( "Package.img", O_RDWR | O_CREAT |  O_TRUNC |
            O_BINARY, S_IREAD | S_IWRITE ) ) == -1 )
    {
        fprintf( stderr , "[ERROR] Package.img open fail.\n" );
        exit( -1 );
    }

    //--------------------------------------------------------------------------
    //  ���ڷ� ���޵� ���� �̸����� ��Ű�� ����� ���� ����
    //--------------------------------------------------------------------------
    printf( "[INFO] Create package header...\n" );

    // �ñ׳�ó�� �����ϰ� ����� ũ�⸦ ���
    memcpy( stHeader.vcSignature, PACKAGESIGNATURE, sizeof( stHeader.vcSignature ) );
    stHeader.dwHeaderSize = sizeof( PACKAGEHEADER ) +
        ( argc - 1 ) * sizeof( PACKAGEITEM );
    // ���Ͽ� ����
    if( write( iTargetFd, &stHeader, sizeof( stHeader ) ) != sizeof( stHeader ) )
    {
        fprintf( stderr, "[ERROR] Data write fail\n" );
        exit( -1 );
    }

    // ���ڸ� ���鼭 ��Ű�� ����� ������ ä�� ����
    for( i = 1 ; i < argc ; i++ )
    {
        // ���� ������ Ȯ��
        if( stat( argv[ i ], &stFileData ) != 0 )
        {
            fprintf( stderr, "[ERROR] %s file open fail\n" );
            exit( -1 );
        }

        // ���� �̸��� ���̸� ����
        memset( stItem.vcFileName, 0, sizeof( stItem.vcFileName ) );
        strncpy( stItem.vcFileName, argv[ i ], sizeof( stItem.vcFileName ) );
        stItem.vcFileName[ sizeof( stItem.vcFileName ) - 1 ] = '\0';
        stItem.dwFileLength = stFileData.st_size;

        // ���Ͽ� ��
        if( write( iTargetFd, &stItem, sizeof( stItem ) ) != sizeof( stItem ) )
        {
            fprintf( stderr, "[ERROR] Data write fail\n" );
            exit( -1 );
        }

        printf( "[%d] file: %s, size: %d Byte\n", i, argv[ i ], stFileData.st_size );
    }
    printf( "[INFO] Create complete\n" );

    //--------------------------------------------------------------------------
    //  ������ ��Ű�� ��� �ڿ� ������ ������ ����
    //--------------------------------------------------------------------------
    printf( "[INFO] Copy data file to package...\n" );
    // ���ڸ� ���鼭 ������ ä�� ����
    iSourceSize = 0;
    for( i = 1 ; i < argc ; i++ )
    {
        // ������ ������ ��
        if( ( iSourceFd = open( argv[ i ], O_RDONLY | O_BINARY ) ) == -1 )
        {
            fprintf( stderr, "[ERROR] %s open fail\n", argv[ 1 ] );
            exit( -1 );
        }

        // ������ ������ ��Ű�� ���Ͽ� �� �ڿ� ������ ����
        iSourceSize += CopyFile( iSourceFd, iTargetFd );
        close( iSourceFd );
    }
    
    // ���� ũ�⸦ ���� ũ���� 512����Ʈ�� ���߱� ���� ������ �κ��� 0x00���� ä��
    AdjustInSectorSize( iTargetFd , iSourceSize + stHeader.dwHeaderSize );

    // ���� �޽��� ���
    printf( "[INFO] Total %d Byte copy complete\n", iSourceSize);
    printf( "[INFO] Package file create complete\n" );

    close( iTargetFd );
    return 0;
}

/**
 *  ���� ��ġ���� 512����Ʈ ��� ��ġ���� ���߾� 0x00���� ä��
*/
int AdjustInSectorSize( int iFd, int iSourceSize )
{
    int i;
    int iAdjustSizeToSector;
    char cCh;
    int iSectorCount;

    iAdjustSizeToSector = iSourceSize % BYTESOFSECTOR;
    cCh = 0x00;
    
    if( iAdjustSizeToSector != 0 )
    {
        iAdjustSizeToSector = 512 - iAdjustSizeToSector;
        for( i = 0 ; i < iAdjustSizeToSector ; i++ )
        {
            write( iFd , &cCh , 1 );
        }
    }
    else
    {
        printf( "[INFO] File size is aligned 512 byte\n" );
    }
    
    // ���� ���� �ǵ�����
    iSectorCount = ( iSourceSize + iAdjustSizeToSector ) / BYTESOFSECTOR;
    return iSectorCount;
}

/**
 *  �ҽ� ����(Source FD)�� ������ ��ǥ ����(Target FD)�� �����ϰ� �� ũ�⸦ �ǵ�����
*/
int CopyFile( int iSourceFd, int iTargetFd )
{
    int iSourceFileSize;
    int iRead;
    int iWrite;
    char vcBuffer[ BYTESOFSECTOR ];

    iSourceFileSize = 0;
    while( 1 )
    {
        iRead   = read( iSourceFd, vcBuffer, sizeof( vcBuffer ) );
        iWrite  = write( iTargetFd, vcBuffer, iRead );

        if( iRead != iWrite )
        {
            fprintf( stderr, "[ERROR] iRead != iWrite.. \n" );
            exit(-1);
        }
        iSourceFileSize += iRead;
        
        if( iRead != sizeof( vcBuffer ) )
        {
            break;
        }
    }
    return iSourceFileSize;
} 
