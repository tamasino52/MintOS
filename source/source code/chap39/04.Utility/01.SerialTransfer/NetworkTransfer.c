/**
 *  file    NetworkTransfer.c
 *  date    2009/06/06
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   ��Ʈ��ũ�� �����͸� �����ϴµ� ���α׷��� �ҽ� ����
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

// ��Ÿ ��ũ��
#define DWORD               unsigned int
#define BYTE                unsigned char
#define MIN( x, y )         ( ( ( x ) < ( y ) ) ? ( x ) : ( y ) )

// �ø��� ��Ʈ FIFO�� �ִ� ũ��
#define SERIAL_FIFOMAXSIZE  16

/**
 *  main �Լ�
 */
int main( int argc, char** argv )
{
    char vcFileName[ 256 ];
    char vcDataBuffer[ SERIAL_FIFOMAXSIZE ];
    struct sockaddr_in stSocketAddr;
    int iSocket;
    BYTE bAck;
    DWORD dwDataLength;
    DWORD dwSentSize;
    DWORD dwTemp;
    FILE* fp;
    
    //--------------------------------------------------------------------------
    // ���� ����
    //--------------------------------------------------------------------------
    // ���� �̸��� �Է����� �ʾ����� ���� �̸��� �Է� ����
    if( argc < 2 )
    {
        fprintf( stderr, "Input File Name: " );
        gets( vcFileName );
    }
    // ���� �̸��� ���� �ÿ� �Է��ߴٸ� ������
    else
    {
        strcpy( vcFileName, argv[ 1 ] );
    }

    // ���� ���� �õ�
    fp = fopen( vcFileName, "rb" );
    if( fp == NULL )
    {
        fprintf( stderr, "%s File Open Error\n", vcFileName );
        return 0;
    }
    
    // fseek�� ���� ������ �̵��Ͽ� ������ ���̸� ������ ��, �ٽ� ������ ó������ �̵�
    fseek( fp, 0, SEEK_END );
    dwDataLength = ftell( fp );
    fseek( fp, 0, SEEK_SET );
    fprintf( stderr, "File Name %s, Data Length %d Byte\n", vcFileName, 
            dwDataLength );
    
    //--------------------------------------------------------------------------
    // ��Ʈ��ũ ����
    //--------------------------------------------------------------------------
    // ������ QEMU�� Address�� ����
    stSocketAddr.sin_family = AF_INET;
    stSocketAddr.sin_port = htons( 4444 );
    stSocketAddr.sin_addr.s_addr = inet_addr( "127.0.0.1" );

    // ���� ���� ��, QEMU�� ���� �õ�
    iSocket = socket( AF_INET, SOCK_STREAM, 0 );
    if( connect( iSocket, ( struct sockaddr* ) &stSocketAddr, 
                 sizeof( stSocketAddr ) ) == -1 )
    {
        fprintf( stderr, "Socket Connect Error, IP 127.0.0.1, Port 4444\n" );
        return 0;
    }
    else
    {
        fprintf( stderr, "Socket Connect Success, IP 127.0.0.1, Port 4444\n" );
    }
    
    //--------------------------------------------------------------------------
    // ������ ����
    //--------------------------------------------------------------------------
    // ������ ���̸� ����
    if( send( iSocket, &dwDataLength, 4, 0 ) != 4 )
    {
        fprintf( stderr, "Data Length Send Fail, [%d] Byte\n", dwDataLength );
        return 0;
    }
    else
    {
        fprintf( stderr, "Data Length Send Success, [%d] Byte\n", dwDataLength );
    }
    // Ack�� ������ ������ ���
    if( recv( iSocket, &bAck, 1, 0 ) != 1 )
    {
        fprintf( stderr, "Ack Receive Error\n" );
        return 0;
    }
    
    // �����͸� ����
    fprintf( stderr, "Now Data Transfer..." );
    dwSentSize = 0;
    while( dwSentSize < dwDataLength )
    {
        // ���� ũ��� FIFO�� �ִ� ũ�� �߿��� ���� ���� ����
        dwTemp = MIN( dwDataLength - dwSentSize, SERIAL_FIFOMAXSIZE );
        dwSentSize += dwTemp;
        
        if( fread( vcDataBuffer, 1, dwTemp, fp ) != dwTemp )
        {
            fprintf( stderr, "File Read Error\n" );
            return 0;
        }
        
        // �����͸� ����
        if( send( iSocket, vcDataBuffer, dwTemp, 0 ) != dwTemp )
        {
            fprintf( stderr, "Socket Send Error\n" );
            return 0;
        }
        
        // Ack�� ���ŵ� ������ ���
        if( recv( iSocket, &bAck, 1, 0 ) != 1 )
        {
            fprintf( stderr, "Ack Receive Error\n" );
            return 0;
        }
        // ���� ��Ȳ ǥ��
        fprintf( stderr, "#" );
    }
    
    // ���ϰ� ������ ����
    fclose( fp );
    close( iSocket );
    
    // ������ �Ϸ�Ǿ����� ǥ���ϰ� ���� Ű�� ���
    fprintf( stderr, "\nSend Complete. Total Size [%d] Byte\n", dwSentSize );
    fprintf( stderr, "Press Enter Key To Exit\n" );
    getchar();
    
    return 0;
}
