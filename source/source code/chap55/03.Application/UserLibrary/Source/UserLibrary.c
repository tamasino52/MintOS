/**
 *  file    UserLibrary.c
 *  date    2009/12/13
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   ���� �������� ����ϴ� ���̺귯���� ���õ� �ҽ� ����
 */

#include "Types.h"
#include "UserLibrary.h"
#include <stdarg.h>

//==============================================================================
//  ȭ�� ����°� ǥ�� �Լ� ����
//==============================================================================
/** 
 *  �޸𸮸� Ư�� ������ ä��
 */
void memset( void* pvDestination, BYTE bData, int iSize )
{
    int i;
    QWORD qwData;
    int iRemainByteStartOffset;
    
    // 8 ����Ʈ �����͸� ä��
    qwData = 0;
    for( i = 0 ; i < 8 ; i++ )
    {
        qwData = ( qwData << 8 ) | bData;
    }
    
    // 8 ����Ʈ�� ���� ä��
    for( i = 0 ; i < ( iSize / 8 ) ; i++ )
    {
        ( ( QWORD* ) pvDestination )[ i ] = qwData;
    }
    
    // 8 ����Ʈ�� ä��� ���� �κ��� ������
    iRemainByteStartOffset = i * 8;
    for( i = 0 ; i < ( iSize % 8 ) ; i++ )
    {
        ( ( char* ) pvDestination )[ iRemainByteStartOffset++ ] = bData;
    }
}

/**
 *  �޸� ����
 */
int memcpy( void* pvDestination, const void* pvSource, int iSize )
{
    int i;
    int iRemainByteStartOffset;
    
    // 8 ����Ʈ�� ���� ����
    for( i = 0 ; i < ( iSize / 8 ) ; i++ )
    {
        ( ( QWORD* ) pvDestination )[ i ] = ( ( QWORD* ) pvSource )[ i ];
    }
    
    // 8 ����Ʈ�� ä��� ���� �κ��� ������
    iRemainByteStartOffset = i * 8;
    for( i = 0 ; i < ( iSize % 8 ) ; i++ )
    {
        ( ( char* ) pvDestination )[ iRemainByteStartOffset ] = 
            ( ( char* ) pvSource )[ iRemainByteStartOffset ];
        iRemainByteStartOffset++;
    }
    return iSize;
}

/**
 *  �޸� ��
 */
int memcmp( const void* pvDestination, const void* pvSource, int iSize )
{
    int i, j;
    int iRemainByteStartOffset;
    QWORD qwValue;
    char cValue;
    
    // 8 ����Ʈ�� ���� ��
    for( i = 0 ; i < ( iSize / 8 ) ; i++ )
    {
        qwValue = ( ( QWORD* ) pvDestination )[ i ] - ( ( QWORD* ) pvSource )[ i ];

        // Ʋ�� ��ġ�� ��Ȯ�ϰ� ã�Ƽ� �� ���� ��ȯ
        if( qwValue != 0 )
        {
            for( i = 0 ; i < 8 ; i++ )
            {
                if( ( ( qwValue >> ( i * 8 ) ) & 0xFF ) != 0 )
                {
                    return ( qwValue >> ( i * 8 ) ) & 0xFF;
                }
            }
        }
    }
    
    // 8 ����Ʈ�� ä��� ���� �κ��� ������
    iRemainByteStartOffset = i * 8;
    for( i = 0 ; i < ( iSize % 8 ) ; i++ )
    {
        cValue = ( ( char* ) pvDestination )[ iRemainByteStartOffset ] -
            ( ( char* ) pvSource )[ iRemainByteStartOffset ];
        if( cValue != 0 )
        {
            return cValue;
        }
        iRemainByteStartOffset++;
    }    
    return 0;
}

/**
 *  ���ڿ��� ����
 */
int strcpy( char* pcDestination, const char* pcSource )
{
    int i;
    
    for( i = 0 ; pcSource[ i ] != 0 ; i++ )
    {
        pcDestination[ i ] = pcSource[ i ];
    }
    return i;
}

/**
 *  ���ڿ��� ��
 */
int strcmp( char* pcDestination, const char* pcSource )
{
    int i;
    
    for( i = 0 ; ( pcDestination[ i ] != 0 ) && ( pcSource[ i ] != 0 ) ; i++ )
    {
        if( ( pcDestination[ i ] - pcSource[ i ] ) != 0 )
        {
            break;
        }
    }
    
    return ( pcDestination[ i ] - pcSource[ i ] );
}

/**
 *  ���ڿ��� ���̸� ��ȯ
 */
int strlen( const char* pcBuffer )
{
    int i;
    
    for( i = 0 ; ; i++ )
    {
        if( pcBuffer[ i ] == '\0' )
        {
            break;
        }
    }
    return i;
}

/**
 *  atoi() �Լ��� ���� ����
 */
long atoi( const char* pcBuffer, int iRadix )
{
    long lReturn;
    
    switch( iRadix )
    {
        // 16����
    case 16:
        lReturn = HexStringToQword( pcBuffer );
        break;
        
        // 10���� �Ǵ� ��Ÿ
    case 10:
    default:
        lReturn = DecimalStringToLong( pcBuffer );
        break;
    }
    return lReturn;
}

/**
 *  16���� ���ڿ��� QWORD�� ��ȯ 
 */
QWORD HexStringToQword( const char* pcBuffer )
{
    QWORD qwValue = 0;
    int i;
    
    // ���ڿ��� ���鼭 ���ʷ� ��ȯ
    for( i = 0 ; pcBuffer[ i ] != '\0' ; i++ )
    {
        qwValue *= 16;
        if( ( 'A' <= pcBuffer[ i ] )  && ( pcBuffer[ i ] <= 'Z' ) )
        {
            qwValue += ( pcBuffer[ i ] - 'A' ) + 10;
        }
        else if( ( 'a' <= pcBuffer[ i ] )  && ( pcBuffer[ i ] <= 'z' ) )
        {
            qwValue += ( pcBuffer[ i ] - 'a' ) + 10;
        }
        else 
        {
            qwValue += pcBuffer[ i ] - '0';
        }
    }
    return qwValue;
}

/**
 *  10���� ���ڿ��� long���� ��ȯ
 */
long DecimalStringToLong( const char* pcBuffer )
{
    long lValue = 0;
    int i;
    
    // �����̸� -�� �����ϰ� �������� ���� long���� ��ȯ
    if( pcBuffer[ 0 ] == '-' )
    {
        i = 1;
    }
    else
    {
        i = 0;
    }
    
    // ���ڿ��� ���鼭 ���ʷ� ��ȯ
    for( ; pcBuffer[ i ] != '\0' ; i++ )
    {
        lValue *= 10;
        lValue += pcBuffer[ i ] - '0';
    }
    
    // �����̸� - �߰�
    if( pcBuffer[ 0 ] == '-' )
    {
        lValue = -lValue;
    }
    return lValue;
}

/**
 *  itoa() �Լ��� ���� ����
 */
int itoa( long lValue, char* pcBuffer, int iRadix )
{
    int iReturn;
    
    switch( iRadix )
    {
        // 16����
    case 16:
        iReturn = HexToString( lValue, pcBuffer );
        break;
        
        // 10���� �Ǵ� ��Ÿ
    case 10:
    default:
        iReturn = DecimalToString( lValue, pcBuffer );
        break;
    }
    
    return iReturn;
}

/**
 *  16���� ���� ���ڿ��� ��ȯ
 */
int HexToString( QWORD qwValue, char* pcBuffer )
{
    QWORD i;
    QWORD qwCurrentValue;

    // 0�� ������ �ٷ� ó��
    if( qwValue == 0 )
    {
        pcBuffer[ 0 ] = '0';
        pcBuffer[ 1 ] = '\0';
        return 1;
    }
    
    // ���ۿ� 1�� �ڸ����� 16, 256, ...�� �ڸ� ������ ���� ����
    for( i = 0 ; qwValue > 0 ; i++ )
    {
        qwCurrentValue = qwValue % 16;
        if( qwCurrentValue >= 10 )
        {
            pcBuffer[ i ] = 'A' + ( qwCurrentValue - 10 );
        }
        else
        {
            pcBuffer[ i ] = '0' + qwCurrentValue;
        }
        
        qwValue = qwValue / 16;
    }
    pcBuffer[ i ] = '\0';
    
    // ���ۿ� ����ִ� ���ڿ��� ����� ... 256, 16, 1�� �ڸ� ������ ����
    ReverseString( pcBuffer );
    return i;
}

/**
 *  10���� ���� ���ڿ��� ��ȯ
 */
int DecimalToString( long lValue, char* pcBuffer )
{
    long i;

    // 0�� ������ �ٷ� ó��
    if( lValue == 0 )
    {
        pcBuffer[ 0 ] = '0';
        pcBuffer[ 1 ] = '\0';
        return 1;
    }
    
    // ���� �����̸� ��� ���ۿ� '-'�� �߰��ϰ� ����� ��ȯ
    if( lValue < 0 )
    {
        i = 1;
        pcBuffer[ 0 ] = '-';
        lValue = -lValue;
    }
    else
    {
        i = 0;
    }

    // ���ۿ� 1�� �ڸ����� 10, 100, 1000 ...�� �ڸ� ������ ���� ����
    for( ; lValue > 0 ; i++ )
    {
        pcBuffer[ i ] = '0' + lValue % 10;        
        lValue = lValue / 10;
    }
    pcBuffer[ i ] = '\0';
    
    // ���ۿ� ����ִ� ���ڿ��� ����� ... 1000, 100, 10, 1�� �ڸ� ������ ����
    if( pcBuffer[ 0 ] == '-' )
    {
        // ������ ���� ��ȣ�� �����ϰ� ���ڿ��� ������
        ReverseString( &( pcBuffer[ 1 ] ) );
    }
    else
    {
        ReverseString( pcBuffer );
    }
    
    return i;
}

/**
 *  ���ڿ��� ������ ������
 */
void ReverseString( char* pcBuffer )
{
   int iLength;
   int i;
   char cTemp;
   
   
   // ���ڿ��� ����� �߽����� ��/�츦 �ٲ㼭 ������ ������
   iLength = strlen( pcBuffer );
   for( i = 0 ; i < iLength / 2 ; i++ )
   {
       cTemp = pcBuffer[ i ];
       pcBuffer[ i ] = pcBuffer[ iLength - 1 - i ];
       pcBuffer[ iLength - 1 - i ] = cTemp;
   }
}

/**
 *  sprintf() �Լ��� ���� ����
 */
int sprintf( char* pcBuffer, const char* pcFormatString, ... )
{
    va_list ap;
    int iReturn;
    
    // ���� ���ڸ� ������ vsprintf() �Լ��� �Ѱ���
    va_start( ap, pcFormatString );
    iReturn = vsprintf( pcBuffer, pcFormatString, ap );
    va_end( ap );
    
    return iReturn;
}

/**
 *  vsprintf() �Լ��� ���� ����
 *      ���ۿ� ���� ���ڿ��� ���� �����͸� ����
 */
int vsprintf( char* pcBuffer, const char* pcFormatString, va_list ap )
{
    QWORD i, j, k;
    int iBufferIndex = 0;
    int iFormatLength, iCopyLength;
    char* pcCopyString;
    QWORD qwValue;
    int iValue;
    double dValue;
    
    // ���� ���ڿ��� ���̸� �о ���ڿ��� ���̸�ŭ �����͸� ��� ���ۿ� ���
    iFormatLength = strlen( pcFormatString );
    for( i = 0 ; i < iFormatLength ; i++ ) 
    {
        // %�� �����ϸ� ������ Ÿ�� ���ڷ� ó��
        if( pcFormatString[ i ] == '%' ) 
        {
            // % ������ ���ڷ� �̵�
            i++;
            switch( pcFormatString[ i ] ) 
            {
                // ���ڿ� ���  
            case 's':
                // ���� ���ڿ� ����ִ� �Ķ���͸� ���ڿ� Ÿ������ ��ȯ
                pcCopyString = ( char* ) ( va_arg(ap, char* ));
                iCopyLength = strlen( pcCopyString );
                // ���ڿ��� ���̸�ŭ�� ��� ���۷� �����ϰ� ����� ���̸�ŭ 
                // ������ �ε����� �̵�
                memcpy( pcBuffer + iBufferIndex, pcCopyString, iCopyLength );
                iBufferIndex += iCopyLength;
                break;
                
                // ���� ���
            case 'c':
                // ���� ���ڿ� ����ִ� �Ķ���͸� ���� Ÿ������ ��ȯ�Ͽ� 
                // ��� ���ۿ� �����ϰ� ������ �ε����� 1��ŭ �̵�
                pcBuffer[ iBufferIndex ] = ( char ) ( va_arg( ap, int ) );
                iBufferIndex++;
                break;

                // ���� ���
            case 'd':
            case 'i':
                // ���� ���ڿ� ����ִ� �Ķ���͸� ���� Ÿ������ ��ȯ�Ͽ�
                // ��� ���ۿ� �����ϰ� ����� ���̸�ŭ ������ �ε����� �̵�
                iValue = ( int ) ( va_arg( ap, int ) );
                iBufferIndex += itoa( iValue, pcBuffer + iBufferIndex, 10 );
                break;
                
                // 4����Ʈ Hex ���
            case 'x':
            case 'X':
                // ���� ���ڿ� ����ִ� �Ķ���͸� DWORD Ÿ������ ��ȯ�Ͽ�
                // ��� ���ۿ� �����ϰ� ����� ���̸�ŭ ������ �ε����� �̵�
                qwValue = ( DWORD ) ( va_arg( ap, DWORD ) ) & 0xFFFFFFFF;
                iBufferIndex += itoa( qwValue, pcBuffer + iBufferIndex, 16 );
                break;

                // 8����Ʈ Hex ���
            case 'q':
            case 'Q':
            case 'p':
                // ���� ���ڿ� ����ִ� �Ķ���͸� QWORD Ÿ������ ��ȯ�Ͽ�
                // ��� ���ۿ� �����ϰ� ����� ���̸�ŭ ������ �ε����� �̵�
                qwValue = ( QWORD ) ( va_arg( ap, QWORD ) );
                iBufferIndex += itoa( qwValue, pcBuffer + iBufferIndex, 16 );
                break;
            
                // �Ҽ��� ��° �ڸ����� �Ǽ��� ���
            case 'f':
                dValue = ( double) ( va_arg( ap, double ) );
                // ��° �ڸ����� �ݿø� ó��
                dValue += 0.005;
                // �Ҽ��� ��° �ڸ����� ���ʷ� �����Ͽ� ���۸� ������
                pcBuffer[ iBufferIndex ] = '0' + ( QWORD ) ( dValue * 100 ) % 10;
                pcBuffer[ iBufferIndex + 1 ] = '0' + ( QWORD ) ( dValue * 10 ) % 10;
                pcBuffer[ iBufferIndex + 2 ] = '.';
                for( k = 0 ; ; k++ )
                {
                    // ���� �κ��� 0�̸� ����
                    if( ( ( QWORD ) dValue == 0 ) && ( k != 0 ) )
                    {
                        break;
                    }
                    pcBuffer[ iBufferIndex + 3 + k ] = '0' + ( ( QWORD ) dValue % 10 );
                    dValue = dValue / 10;
                }
                pcBuffer[ iBufferIndex + 3 + k ] = '\0';
                // ���� ����� ���̸�ŭ ������ ���̸� ������Ŵ
                ReverseString( pcBuffer + iBufferIndex );
                iBufferIndex += 3 + k;
                break;
                
                // ���� �ش����� ������ ���ڸ� �״�� ����ϰ� ������ �ε�����
                // 1��ŭ �̵�
            default:
                pcBuffer[ iBufferIndex ] = pcFormatString[ i ];
                iBufferIndex++;
                break;
            }
        } 
        // �Ϲ� ���ڿ� ó��
        else
        {
            // ���ڸ� �״�� ����ϰ� ������ �ε����� 1��ŭ �̵�
            pcBuffer[ iBufferIndex ] = pcFormatString[ i ];
            iBufferIndex++;
        }
    }
    
    // NULL�� �߰��Ͽ� ������ ���ڿ��� ����� ����� ������ ���̸� ��ȯ
    pcBuffer[ iBufferIndex ] = '\0';
    return iBufferIndex;
}

/**
 *  printf �Լ��� ���� ����
 */
void printf( const char* pcFormatString, ... )
{
    va_list ap;
    char vcBuffer[ 1024 ];
    int iNextPrintOffset;

    // ���� ���� ����Ʈ�� ����ؼ� vsprintf()�� ó��
    va_start( ap, pcFormatString );
    vsprintf( vcBuffer, pcFormatString, ap );
    va_end( ap );
    
    // ���� ���ڿ��� ȭ�鿡 ���
    iNextPrintOffset = ConsolePrintString( vcBuffer );
    
    // Ŀ���� ��ġ�� ������Ʈ
    SetCursor( iNextPrintOffset % CONSOLE_WIDTH, iNextPrintOffset / CONSOLE_WIDTH );
}

// ������ �߻���Ű�� ���� ����
static volatile QWORD gs_qwRandomValue = 0;

/**
 *  ������ �ʱ갪(Seed) ����
 */
void srand( QWORD qwSeed )
{
    gs_qwRandomValue = qwSeed;
}

/**
 *  ������ ������ ��ȯ
 */
QWORD rand( void )
{
    gs_qwRandomValue = ( gs_qwRandomValue * 412153 + 5571031 ) >> 16;
    return gs_qwRandomValue;
}

//==============================================================================
// GUI �ý��� ����
//==============================================================================
/**
 *  (x, y)�� �簢�� ���� �ȿ� �ִ��� ���θ� ��ȯ
 */
BOOL IsInRectangle( const RECT* pstArea, int iX, int iY )
{
    // ȭ�鿡 ǥ�õǴ� ������ ����ٸ� �׸��� ����
    if( ( iX <  pstArea->iX1 ) || ( pstArea->iX2 < iX ) ||
        ( iY <  pstArea->iY1 ) || ( pstArea->iY2 < iY ) )
    {
        return FALSE;
    }
    
    return TRUE;
}

/**
 *  �簢���� �ʺ� ��ȯ
 */
int GetRectangleWidth( const RECT* pstArea )
{
    int iWidth;
    
    iWidth = pstArea->iX2 - pstArea->iX1 + 1;
    
    if( iWidth < 0 )
    {
        return -iWidth;
    }
    
    return iWidth;
}

/**
 *  �簢���� ���̸� ��ȯ
 */
int GetRectangleHeight( const RECT* pstArea )
{
    int iHeight;
    
    iHeight = pstArea->iY2 - pstArea->iY1 + 1;
    
    if( iHeight < 0 )
    {
        return -iHeight;
    }
    
    return iHeight;
}

/**
 *  ���� 1�� ���� 2�� ��ġ�� ������ ��ȯ
 */
BOOL GetOverlappedRectangle( const RECT* pstArea1, const RECT* pstArea2,
        RECT* pstIntersection  )
{
    int iMaxX1;
    int iMinX2;
    int iMaxY1;
    int iMinY2;

    // X���� �������� �� �� �߿��� ū ���� ã��
    iMaxX1 = MAX( pstArea1->iX1, pstArea2->iX1 );
    // X���� ������ �� �� �߿��� ���� ���� ã��
    iMinX2 = MIN( pstArea1->iX2, pstArea2->iX2 );
    // ����� �������� ��ġ�� ������ ��ġ���� ũ�ٸ� �� �簢���� ��ġ�� ����
    if( iMinX2 < iMaxX1 )
    {
        return FALSE;
    }

    // Y���� �������� �� �� �߿��� ū ���� ã��
    iMaxY1 = MAX( pstArea1->iY1, pstArea2->iY1 );
    // Y���� ������ �� �� �߿��� ���� ���� ã��
    iMinY2 = MIN( pstArea1->iY2, pstArea2->iY2 );
    // ����� �������� ��ġ�� ������ ��ġ���� ũ�ٸ� �� �簢���� ��ġ�� ����
    if( iMinY2 < iMaxY1 )
    {
        return FALSE;
    }

    // ��ġ�� ������ ���� ����
    pstIntersection->iX1 = iMaxX1;
    pstIntersection->iY1 = iMaxY1;
    pstIntersection->iX2 = iMinX2;
    pstIntersection->iY2 = iMinY2;

    return TRUE;
}

/**
 *  ��ü ȭ���� �������� �� X,Y ��ǥ�� ������ ���� ��ǥ�� ��ȯ
 */
BOOL ConvertPointScreenToClient( QWORD qwWindowID, const POINT* pstXY, 
        POINT* pstXYInWindow )
{
    RECT stArea;
    
    // ������ ������ ��ȯ
    if( GetWindowArea( qwWindowID, &stArea ) == FALSE )
    {
        return FALSE;
    }
    
    pstXYInWindow->iX = pstXY->iX - stArea.iX1;
    pstXYInWindow->iY = pstXY->iY - stArea.iY1;
    return TRUE;
}

/**
 *  ������ ���θ� �������� �� X,Y ��ǥ�� ȭ�� ��ǥ�� ��ȯ
 */
BOOL ConvertPointClientToScreen( QWORD qwWindowID, const POINT* pstXY, 
        POINT* pstXYInScreen )
{
    RECT stArea;
    
    // ������ ������ ��ȯ
    if( GetWindowArea( qwWindowID, &stArea ) == FALSE )
    {
        return FALSE;
    }
    
    pstXYInScreen->iX = pstXY->iX + stArea.iX1;
    pstXYInScreen->iY = pstXY->iY + stArea.iY1;
    return TRUE;
}

/**
 *  ��ü ȭ���� �������� �� �簢�� ��ǥ�� ������ ���� ��ǥ�� ��ȯ
 */
BOOL ConvertRectScreenToClient( QWORD qwWindowID, const RECT* pstArea, 
        RECT* pstAreaInWindow )
{
    RECT stWindowArea;
    
    // ������ ������ ��ȯ
    if( GetWindowArea( qwWindowID, &stWindowArea ) == FALSE )
    {
        return FALSE;
    }
    
    pstAreaInWindow->iX1 = pstArea->iX1 - stWindowArea.iX1;
    pstAreaInWindow->iY1 = pstArea->iY1 - stWindowArea.iY1;
    pstAreaInWindow->iX2 = pstArea->iX2 - stWindowArea.iX1;
    pstAreaInWindow->iY2 = pstArea->iY2 - stWindowArea.iY1;
    return TRUE;
}

/**
 *  ������ ���θ� �������� �� �簢�� ��ǥ�� ȭ�� ��ǥ�� ��ȯ
 */
BOOL ConvertRectClientToScreen( QWORD qwWindowID, const RECT* pstArea, 
        RECT* pstAreaInScreen )
{
    RECT stWindowArea;
    
    // ������ ������ ��ȯ
    if( GetWindowArea( qwWindowID, &stWindowArea ) == FALSE )
    {
        return FALSE;
    }
    
    pstAreaInScreen->iX1 = pstArea->iX1 + stWindowArea.iX1;
    pstAreaInScreen->iY1 = pstArea->iY1 + stWindowArea.iY1;
    pstAreaInScreen->iX2 = pstArea->iX2 + stWindowArea.iX1;
    pstAreaInScreen->iY2 = pstArea->iY2 + stWindowArea.iY1;
    return TRUE;
}

/**
 *  �簢�� �ڷᱸ���� ä��
 *      x1�� x2, y1�� y2�� ���ؼ� x1 < x2, y1 < y2�� �ǵ��� ����
 */
void SetRectangleData( int iX1, int iY1, int iX2, int iY2, RECT* pstRect )
{
    // x1 < x2�� �ǵ��� RECT �ڷᱸ���� X��ǥ�� ����
    if( iX1 < iX2 )
    {
        pstRect->iX1 = iX1;
        pstRect->iX2 = iX2;
    }
    else
    {
        pstRect->iX1 = iX2;
        pstRect->iX2 = iX1;
    }
    
    // y1 < y2�� �ǵ��� RECT �ڷᱸ���� Y��ǥ�� ����
    if( iY1 < iY2 )
    {
        pstRect->iY1 = iY1;
        pstRect->iY2 = iY2;
    }
    else
    {
        pstRect->iY1 = iY2;
        pstRect->iY2 = iY1;
    }
}

/**
 *  ���콺 �̺�Ʈ �ڷᱸ���� ����
 */
BOOL SetMouseEvent( QWORD qwWindowID, QWORD qwEventType, int iMouseX, int iMouseY, 
        BYTE bButtonStatus, EVENT* pstEvent )
{
    POINT stMouseXYInWindow;
    POINT stMouseXY;
    
    // �̺�Ʈ ������ Ȯ���Ͽ� ���콺 �̺�Ʈ ����
    switch( qwEventType )
    {
        // ���콺 �̺�Ʈ ó��
    case EVENT_MOUSE_MOVE:
    case EVENT_MOUSE_LBUTTONDOWN:
    case EVENT_MOUSE_LBUTTONUP:            
    case EVENT_MOUSE_RBUTTONDOWN:
    case EVENT_MOUSE_RBUTTONUP:
    case EVENT_MOUSE_MBUTTONDOWN:
    case EVENT_MOUSE_MBUTTONUP:
        // ���콺�� X, Y��ǥ�� ����
        stMouseXY.iX = iMouseX;
        stMouseXY.iY = iMouseY;
        
        // ���콺 X, Y��ǥ�� ������ ���� ��ǥ�� ��ȯ
        if( ConvertPointScreenToClient( qwWindowID, &stMouseXY, &stMouseXYInWindow ) 
                == FALSE )
        {
            return FALSE;
        }

        // �̺�Ʈ Ÿ�� ����
        pstEvent->qwType = qwEventType;
        // ������ ID ����
        pstEvent->stMouseEvent.qwWindowID = qwWindowID;    
        // ���콺 ��ư�� ���� ����
        pstEvent->stMouseEvent.bButtonStatus = bButtonStatus;
        // ���콺 Ŀ���� ��ǥ�� ������ ���� ��ǥ�� ��ȯ�� ���� ����
        memcpy( &( pstEvent->stMouseEvent.stPoint ), &stMouseXYInWindow, 
                sizeof( POINT ) );
        break;
        
    default:
        return FALSE;
        break;
    }    
    return TRUE;
}

/**
 *  ������ �̺�Ʈ �ڷᱸ���� ����
 */
BOOL SetWindowEvent( QWORD qwWindowID, QWORD qwEventType, EVENT* pstEvent )
{
    RECT stArea;
    
    // �̺�Ʈ ������ Ȯ���Ͽ� ������ �̺�Ʈ ����
    switch( qwEventType )
    {
        // ������ �̺�Ʈ ó��
    case EVENT_WINDOW_SELECT:
    case EVENT_WINDOW_DESELECT:
    case EVENT_WINDOW_MOVE:
    case EVENT_WINDOW_RESIZE:
    case EVENT_WINDOW_CLOSE:
        // �̺�Ʈ Ÿ�� ����
        pstEvent->qwType = qwEventType;
        // ������ ID ����
        pstEvent->stWindowEvent.qwWindowID = qwWindowID;
        // ������ ������ ��ȯ
        if( GetWindowArea( qwWindowID, &stArea ) == FALSE )
        {
            return FALSE;
        }
        
        // �������� ���� ��ǥ�� ����
        memcpy( &( pstEvent->stWindowEvent.stArea ), &stArea, sizeof( RECT ) );
        break;
        
    default:
        return FALSE;
        break;
    }    
    return TRUE;
}

/**
 *  Ű �̺�Ʈ �ڷᱸ���� ����
 */
void SetKeyEvent( QWORD qwWindow, const KEYDATA* pstKeyData, EVENT* pstEvent )
{
    // ���� �Ǵ� ������ ó��
    if( pstKeyData->bFlags & KEY_FLAGS_DOWN )
    {
        pstEvent->qwType = EVENT_KEY_DOWN;
    }
    else
    {
        pstEvent->qwType = EVENT_KEY_UP;
    }
    
    // Ű�� �� ������ ����
    pstEvent->stKeyEvent.bASCIICode = pstKeyData->bASCIICode;
    pstEvent->stKeyEvent.bScanCode = pstKeyData->bScanCode;
    pstEvent->stKeyEvent.bFlags = pstKeyData->bFlags;
}
