/**
 *  file    2DGraphics.h
 *  date    2009/09/5
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   2D Graphic�� ���� �ҽ� ����
 */

#include "2DGraphics.h"
#include "VBE.h"
#include "Font.h"
#include "Utility.h"

/**
 *  �� �׸���
 */
inline void kDrawPixel( int iX, int iY, COLOR stColor )
{
    VBEMODEINFOBLOCK* pstModeInfo;
    
    // ��� ���� ��� ��ȯ
    pstModeInfo = kGetVBEModeInfoBlock();
    
    // Physical Address�� COLOR Ÿ������ �ϸ� �ȼ� ���������� ��� ����
    *( ( ( COLOR* ) ( QWORD ) pstModeInfo->dwPhysicalBasePointer ) + 
            pstModeInfo->wXResolution * iY + iX ) = stColor;
}

/**
 *  ���� �׸���
 */
void kDrawLine( int iX1, int iY1, int iX2, int iY2, COLOR stColor )
{
    int iDeltaX, iDeltaY;
    int iError = 0;
    int iDeltaError;
    int iX, iY;
    int iStepX, iStepY;
    
    // ��ȭ�� ���
    iDeltaX = iX2 - iX1;
    iDeltaY = iY2 - iY1;

    // X�� ��ȭ���� ���� X�� ���� ���� ���
    if( iDeltaX < 0 ) 
    {
        iDeltaX = -iDeltaX; 
        iStepX = -1; 
    } 
    else 
    { 
        iStepX = 1; 
    }

    // Y�� ��ȭ���� ���� Y�� ���� ���� ��� 
    if( iDeltaY < 0 ) 
    {
        iDeltaY = -iDeltaY; 
        iStepY = -1; 
    } 
    else 
    {
        iStepY = 1; 
    }

    // X�� ��ȭ���� Y�� ��ȭ������ ũ�ٸ� X���� �߽����� ������ �׸�
    if( iDeltaX > iDeltaY )
    {
        // ����� �� �ȼ����� ������ ����, Y�� ��ȭ���� 2��
        // ����Ʈ �������� * 2�� ��ü
        iDeltaError = iDeltaY << 1;
        iY = iY1;
        for( iX = iX1 ; iX != iX2 ; iX += iStepX )
        {
            // �� �׸���
            kDrawPixel( iX, iY, stColor );

            // ���� ����
            iError += iDeltaError;

            // ������ ������ X�� ��ȭ������ ũ�� ���� ���� �����ϰ� ������ ���� ����
            // �������� ����
            if( iError >= iDeltaX )
            {
                iY += iStepY;
                // X���� ��ȭ���� 2�踦 ����
                // ����Ʈ �������� *2�� ��ü
                iError -= iDeltaX << 1;
            }
        }
        // iX == iX2�� ���� ��ġ�� �� �׸���
        kDrawPixel( iX, iY, stColor );
    }
    // Y�� ��ȭ���� X�� ��ȭ������ ũ�ų� ���ٸ� Y���� �߽����� ������ �׸�
    else
    {
        // ����� �� �ȼ����� ������ ����, X�� ��ȭ���� 2��
        // ����Ʈ �������� * 2�� ��ü
        iDeltaError = iDeltaX << 1;
        iX = iX1;
        for( iY = iY1 ; iY != iY2 ; iY += iStepY )
        {
            // �� �׸���
            kDrawPixel( iX, iY, stColor );

            // ���� ����
            iError += iDeltaError;

            // ������ ������ Y�� ��ȭ������ ũ�� ���� ���� �����ϰ� ������ ���� ����
            // �������� ����
            if( iError >= iDeltaY )
            {
                iX += iStepX;
                // Y���� ��ȭ���� 2�踦 ����
                // ����Ʈ �������� *2�� ��ü
                iError -= iDeltaY << 1;
            }
        }

        // iY == iY2�� ���� ��ġ�� �� �׸���
        kDrawPixel( iX, iY, stColor );
    }
}

/**
 *  �簢�� �׸���
 */
void kDrawRect( int iX1, int iY1, int iX2, int iY2, COLOR stColor, BOOL bFill )
{
    int iWidth;
    int iTemp;
    int iY;
    int iStepY;
    COLOR* pstVideoMemoryAddress;
    VBEMODEINFOBLOCK* pstModeInfo;

    // ä�� ���ο� ���� �ڵ带 �и�
    if( bFill == FALSE )
    {
        // �� ���� �̿��� �ͳ��� �������� ����
        kDrawLine( iX1, iY1, iX2, iY1, stColor );
        kDrawLine( iX1, iY1, iX1, iY2, stColor );
        kDrawLine( iX2, iY1, iX2, iY2, stColor );
        kDrawLine( iX1, iY2, iX2, iY2, stColor );
    }
    else
    {
        // VBE ��� ���� ����� ��ȯ
        pstModeInfo = kGetVBEModeInfoBlock();
        
        // ���� �޸� ��巹�� ���
        pstVideoMemoryAddress = 
            ( COLOR* ) ( ( QWORD ) pstModeInfo->dwPhysicalBasePointer );
        
        // kMemSetWord() �Լ��� X�� ���� �����ϴ� �������� �����͸� ä��Ƿ�
        // x1�� x2�� ���Ͽ� �� ���� ��ȯ
        if( iX2 < iX1 )
        {
            iTemp = iX1;
            iX1 = iX2;
            iX2 = iTemp;
            
            iTemp = iY1;
            iY1 = iY2;
            iY2 = iTemp;
        }
        
        // �簢���� X�� ���̸� ���
        iWidth = iX2 - iX1 + 1;
        
        // y1�� y2�� ���Ͽ� �� ȸ���� ������ų Y ���� ����
        if( iY1 <= iY2 )
        {
            iStepY = 1;         
        }
        else
        {
            iStepY = -1;
        }
        
        // ����� ������ ���� �޸� ��巹���� ���
        pstVideoMemoryAddress += iY1 * pstModeInfo->wXResolution + iX1;
        
        // ������ ���鼭 �� Y�ึ�� ���� ä��
        for( iY = iY1 ; iY != iY2 ; iY += iStepY )
        {
            // ���� �޸𸮿� �簢���� �ʺ�ŭ ���
            kMemSetWord( pstVideoMemoryAddress, stColor, iWidth );
            
            // ����� ���� �޸� ��巹�� ����
            // x, y ��ǥ�� �Ź� ���� �޸� ��巹���� ����ϴ� ���� ���Ϸ���
            // X�� �ػ󵵸� �̿��Ͽ� ���� Y�� ��巹���� ��� 
            if( iStepY >= 0 )
            {
                pstVideoMemoryAddress += pstModeInfo->wXResolution;
            }
            else
            {
                pstVideoMemoryAddress -= pstModeInfo->wXResolution;
            }
        }
        
        // ���� �޸𸮿� �簢���� �ʺ�ŭ ���, ������ �� ���
        kMemSetWord( pstVideoMemoryAddress, stColor, iWidth );
    }
}

/**
 *  �� �׸���
 */
void kDrawCircle( int iX, int iY, int iRadius, COLOR stColor, BOOL bFill )
{
    int iCircleX, iCircleY;
    int iDistance;
    
    // �������� 0���� �۴ٸ� �׸� �ʿ� ����
    if( iRadius < 0 )
    {
        return ;
    }
    
    // (0, R)�� ��ǥ���� ����
    iCircleY = iRadius;

    // ä�� ���ο� ���� �������� �׸�
    if( bFill == FALSE )
    {
        // �������� �� ���� ��� �׸�
        kDrawPixel( 0 + iX, iRadius + iY, stColor );
        kDrawPixel( 0 + iX, -iRadius + iY, stColor );
        kDrawPixel( iRadius + iX, 0 + iY, stColor );
        kDrawPixel( -iRadius + iX, 0 + iY, stColor );
    }
    else
    {
        // ���� ������ X��� Y�� ��� �׸�
        kDrawLine( 0 + iX, iRadius + iY, 0 + iX, -iRadius + iY, stColor );
        kDrawLine( iRadius + iX, 0 + iY, -iRadius + iX, 0 + iY, stColor );
    }
    
    // ���� �������� �߽����� ���� �Ÿ�
    iDistance = -iRadius;

    // �� �׸���
    for( iCircleX = 1 ; iCircleX <= iCircleY ; iCircleX++ )
    {
        // ������ ������ �Ÿ� ���
        // ����Ʈ �������� * 2�� ��ü
        iDistance += ( iCircleX << 1 ) - 1;  //2 * iCircleX - 1;
                    
        // �߽����� ���� �ܺο� ������ �Ʒ��� �ִ� �� ����
        if( iDistance >= 0 )
        {
            iCircleY--;
            
            // ���ο� ������ �ٽ� ���� �Ÿ� ���
            // ����Ʈ �������� * 2�� ��ü
            iDistance += ( -iCircleY << 1 ) + 2; //-2 * iCircleY + 2;
        }
        
        // ä�� ���ο� ���� �׸�
        if( bFill == FALSE )
        {
            // 8 ���� ��� �� �׸�
            kDrawPixel( iCircleX + iX, iCircleY + iY, stColor );
            kDrawPixel( iCircleX + iX, -iCircleY + iY, stColor );
            kDrawPixel( -iCircleX + iX, iCircleY + iY, stColor );
            kDrawPixel( -iCircleX + iX, -iCircleY + iY, stColor );
            kDrawPixel( iCircleY + iX, iCircleX + iY, stColor );
            kDrawPixel( iCircleY + iX, -iCircleX + iY, stColor );
            kDrawPixel( -iCircleY + iX, iCircleX + iY, stColor );
            kDrawPixel( -iCircleY + iX, -iCircleX + iY, stColor );
        }
        else
        {
            // ��Ī�Ǵ� ���� ã�� X�࿡ ������ ������ �׾� ä���� ���� �׸�
            // ���༱�� �׸��� ���� �簢�� �׸��� �Լ��� ������ ó���� �� ����
            kDrawRect( -iCircleX + iX, iCircleY + iY, 
                iCircleX + iX, iCircleY + iY, stColor, TRUE );
            kDrawRect( -iCircleX + iX, -iCircleY + iY, 
                iCircleX + iX, -iCircleY + iY, stColor, TRUE );
            kDrawRect( -iCircleY + iX, iCircleX + iY, 
                iCircleY + iX, iCircleX + iY, stColor, TRUE );
            kDrawRect( -iCircleY + iX, -iCircleX + iY, 
                iCircleY + iX, -iCircleX + iY, stColor, TRUE );
        }
    }
}

/**
 *  ���� ���
 */
void kDrawText( int iX, int iY, COLOR stTextColor, COLOR stBackgroundColor, 
        const char* pcString, int iLength )
{
    int iCurrentX, iCurrentY;
    int i, j, k;
    BYTE bBitmask;
    int iBitmaskStartIndex;
    VBEMODEINFOBLOCK* pstModeInfo;
    COLOR* pstVideoMemoryAddress;

    // VBE ��� ���� ����� ��ȯ
    pstModeInfo = kGetVBEModeInfoBlock();
    
    // ���� �޸� ��巹�� ���
    pstVideoMemoryAddress = 
        ( COLOR* ) ( ( QWORD ) pstModeInfo->dwPhysicalBasePointer );
    
    // ���ڸ� ����ϴ� X��ǥ
    iCurrentX = iX;
    
    // ������ ������ŭ �ݺ�
    for( k = 0 ; k < iLength ; k++ )
    {
        // ���ڸ� ����� ��ġ�� Y��ǥ�� ����
        iCurrentY = iY * pstModeInfo->wXResolution;

        // ��Ʈ�� ��Ʈ �����Ϳ��� ���� ��Ʈ���� �����ϴ� ��ġ�� ���
        // 1����Ʈ * FONT_HEIGHT�� �����Ǿ� �����Ƿ� ������ ��Ʈ�� ��ġ��
        // �Ʒ��� ���� ��� ����
        iBitmaskStartIndex = pcString[ k ] * FONT_ENGLISHHEIGHT;
        
        // ���� ���
        for( j = 0 ; j < FONT_ENGLISHHEIGHT ; j++ )
        {
            // �̹� ���ο��� ����� ��Ʈ ��Ʈ�� 
            bBitmask = g_vucEnglishFont[ iBitmaskStartIndex++ ];
            
            // ���� ���� ���
            for( i = 0 ; i < FONT_ENGLISHWIDTH ; i++ )
            {
                // ��Ʈ�� �����Ǿ������� ȭ�鿡 ���ڻ��� ǥ��
                if( bBitmask & ( 0x01 << ( FONT_ENGLISHWIDTH - i - 1 ) ) )
                {
                    pstVideoMemoryAddress[ iCurrentY + iCurrentX + i ] = stTextColor;
                }
                // ��Ʈ�� �����Ǿ����� ������ ȭ�鿡 ������ ǥ��
                else
                {
                    pstVideoMemoryAddress[ iCurrentY + iCurrentX + i ] = stBackgroundColor;
                }
            }
            
            // ���� �������� �̵��ؾ� �ϹǷ�, ���� Y��ǥ�� ȭ���� �ʺ�ŭ ������
            iCurrentY += pstModeInfo->wXResolution;
        }
        
        // ���� �ϳ��� �� ��������� ��Ʈ�� �ʺ�ŭ X ��ǥ�� �̵��Ͽ� ���� ���ڸ� ���
        iCurrentX += FONT_ENGLISHWIDTH;
    }
}
