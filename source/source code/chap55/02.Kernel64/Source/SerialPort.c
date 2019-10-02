/**
 *  file    SerialPort.c
 *  date    2009/06/06
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   �ø��� ��Ʈ ��Ʈ�ѷ��� ���õ� �ҽ� ����
 */

#include "SerialPort.h"
#include "Utility.h"

// �ø��� ��Ʈ�� ����ϴ� �ڷᱸ��
static SERIALMANAGER gs_stSerialManager;

/**
 *  �ø��� ��Ʈ �ʱ�ȭ
 */
void kInitializeSerialPort( void )
{
    WORD wPortBaseAddress;

    // ���ؽ� �ʱ�ȭ
    kInitializeMutex( &( gs_stSerialManager.stLock ) );
    
    // COM1 �ø��� ��Ʈ(��Ʈ 0x3F8)�� �����Ͽ� �ʱ�ȭ
    wPortBaseAddress = SERIAL_PORT_COM1;

    // ���ͷ�Ʈ Ȱ��ȭ ��������(��Ʈ 0x3F9)�� 0�� �����Ͽ� ��� ���ͷ�Ʈ�� ��Ȱ��ȭ
    kOutPortByte( wPortBaseAddress + SERIAL_PORT_INDEX_INTERRUPTENABLE, 0 );

    // ��� �ӵ��� 115200���� ����
    // ���� ���� ��������(��Ʈ 0x3FB)�� DLAB ��Ʈ(��Ʈ 7)�� 1�� �����Ͽ� 
    // ���� ��ġ �������Ϳ� ����
    kOutPortByte( wPortBaseAddress + SERIAL_PORT_INDEX_LINECONTROL, 
            SERIAL_LINECONTROL_DLAB );
    // LSB ���� ��ġ ��������(��Ʈ 0x3F8)�� ����� ���� 8��Ʈ�� ����
    kOutPortByte( wPortBaseAddress + SERIAL_PORT_INDEX_DIVISORLATCHLSB, 
            SERIAL_DIVISORLATCH_115200 );
    // MSB ���� ��ġ ��������(��Ʈ 0x3F9)�� ����� ���� 8��Ʈ�� ����
    kOutPortByte( wPortBaseAddress + SERIAL_PORT_INDEX_DIVISORLATCHMSB, 
            SERIAL_DIVISORLATCH_115200 >> 8 );
    
    // �ۼ��� ����� ���� 
    // ���� ���� ��������(��Ʈ 0x3FB)�� ��� ����� 8��Ʈ, �з�Ƽ ����(No Parity),
    // 1 Stop ��Ʈ�� �����ϰ�, ���� ��ġ �������� ����� �������Ƿ� DLAB ��Ʈ�� 
    // 0���� ����
    kOutPortByte( wPortBaseAddress + SERIAL_PORT_INDEX_LINECONTROL, 
            SERIAL_LINECONTROL_8BIT | SERIAL_LINECONTROL_NOPARITY | 
            SERIAL_LINECONTROL_1BITSTOP );
    
    // FIFO�� ���ͷ�Ʈ �߻� ������ 14����Ʈ�� ����
    kOutPortByte( wPortBaseAddress + SERIAL_PORT_INDEX_FIFOCONTROL, 
            SERIAL_FIFOCONTROL_FIFOENABLE | SERIAL_FIFOCONTROL_14BYTEFIFO );
}

/**
 *  �۽� FIFO�� ����ִ����� ��ȯ
 */
static BOOL kIsSerialTransmitterBufferEmpty( void )
{
    BYTE bData;
    
    // ���� ���� ��������(��Ʈ 0x3FD)�� ���� �� TBE ��Ʈ(��Ʈ 1)�� Ȯ���Ͽ� 
    // �۽� FIFO�� ����ִ��� Ȯ��
    bData = kInPortByte( SERIAL_PORT_COM1 + SERIAL_PORT_INDEX_LINESTATUS );
    if( ( bData & SERIAL_LINESTATUS_TRANSMITBUFFEREMPTY ) == 
        SERIAL_LINESTATUS_TRANSMITBUFFEREMPTY )
    {
        return TRUE;
    }
    return FALSE;
}

/**
 *  �ø��� ��Ʈ�� �����͸� �۽�
 */
void kSendSerialData( BYTE* pbBuffer, int iSize )
{
    int iSentByte;
    int iTempSize;
    int j;
    
    // ����ȭ
    kLock( &( gs_stSerialManager.stLock ) );
    
    // ��û�� ����Ʈ ����ŭ ���� ������ �ݺ�
    iSentByte = 0;
    while( iSentByte < iSize )
    {
        // �۽� FIFO�� �����Ͱ� �����ִٸ� �� ���۵� ������ ���
        while( kIsSerialTransmitterBufferEmpty() == FALSE )
        {
            kSleep( 0 );
        }
        
        // ������ ������ �߿��� ���� ũ��� FIFO�� �ִ� ũ��(16 ����Ʈ)�� 
        // ���� ��, ���� ���� �����Ͽ� �۽� �ø��� ��Ʈ�� ä��
        iTempSize = MIN( iSize - iSentByte, SERIAL_FIFOMAXSIZE );
        for( j = 0 ; j < iTempSize ; j++ )
        {
            // �۽� ���� ��������(��Ʈ 0x3F8)�� �� ����Ʈ�� ����
            kOutPortByte( SERIAL_PORT_COM1 + SERIAL_PORT_INDEX_TRANSMITBUFFER, 
                    pbBuffer[ iSentByte + j ] );
        }
        iSentByte += iTempSize;
    }

    // ����ȭ
    kUnlock( &( gs_stSerialManager.stLock ) );
}

/**
 *  ���� FIFO�� �����Ͱ� �ִ����� ��ȯ
 */
static BOOL kIsSerialReceiveBufferFull( void )
{
    BYTE bData;
    
    // ���� ���� ��������(��Ʈ 0x3FD)�� ���� �� RxRD ��Ʈ(��Ʈ 0)�� Ȯ���Ͽ� 
    // ���� FIFO�� �����Ͱ� �ִ��� Ȯ��
    bData = kInPortByte( SERIAL_PORT_COM1 + SERIAL_PORT_INDEX_LINESTATUS );
    if( ( bData & SERIAL_LINESTATUS_RECEIVEDDATAREADY ) == 
        SERIAL_LINESTATUS_RECEIVEDDATAREADY )
    {
        return TRUE;
    }
    return FALSE;
}

/**
 *  �ø��� ��Ʈ���� �����͸� ����
 */
int kReceiveSerialData( BYTE* pbBuffer, int iSize )
{
    int i;
    
    // ����ȭ
    kLock( &( gs_stSerialManager.stLock ) );
    
    // ������ ���鼭 ���� ���ۿ� �ִ� �����͸� �о ��ȯ
    for( i = 0 ; i < iSize ; i++ )
    {
        // ���ۿ� �����Ͱ� ������ ����
        if( kIsSerialReceiveBufferFull() == FALSE )
        {
            break;
        }
        // ���� ���� ��������(��Ʈ 0x3F8)���� �� ����Ʈ�� ����
        pbBuffer[ i ] = kInPortByte( SERIAL_PORT_COM1 + 
                                     SERIAL_PORT_INDEX_RECEIVEBUFFER );
    }
    
    // ����ȭ
    kUnlock( &( gs_stSerialManager.stLock ) );

    // ���� �������� ������ ��ȯ
    return i;
}

/**
 *  �ø��� ��Ʈ ��Ʈ�ѷ��� FIFO�� �ʱ�ȭ
 */
void kClearSerialFIFO( void )
{
    // ����ȭ
    kLock( &( gs_stSerialManager.stLock ) );
    
    // �ۼ��� FIFO�� ��� ���� ���ۿ� �����Ͱ� 14����Ʈ á�� �� ���ͷ�Ʈ�� 
    // �߻��ϵ��� FIFO ���� ��������(��Ʈ 0x3FA)�� ���� ���� ����
    kOutPortByte( SERIAL_PORT_COM1 + SERIAL_PORT_INDEX_FIFOCONTROL, 
        SERIAL_FIFOCONTROL_FIFOENABLE | SERIAL_FIFOCONTROL_14BYTEFIFO |
        SERIAL_FIFOCONTROL_CLEARRECEIVEFIFO | SERIAL_FIFOCONTROL_CLEARTRANSMITFIFO );
    
    // ����ȭ
    kUnlock( &( gs_stSerialManager.stLock ) );
}
