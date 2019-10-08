/**
 *  file    Types.h
 *  date    2008/12/28
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   ����¡�� ���õ� ���� ������ ������ ����
 */

#include "Page.h"

/**
 *	IA-32e ��� Ŀ���� ���� ������ ���̺� ����
 */
void kInitializePageTables( void )
{
	PML4TENTRY* pstPML4TEntry;
	PDPTENTRY* pstPDPTEntry;
	PDENTRY* pstPDEntry;
	PDTMPENTRY* pstPDTMPEntry; // PDTMP Table Entry
	DWORD dwMappingAddress;
	int i;

	// PML4 ���̺� ����
	// ù ��° ��Ʈ�� �ܿ� �������� ��� 0���� �ʱ�ȭ
	pstPML4TEntry = ( PML4TENTRY* ) 0x100000;
	kSetPageEntryData( &( pstPML4TEntry[ 0 ] ), 0x00, 0x101000, PAGE_FLAGS_DEFAULT,
			0 );
	for( i = 1 ; i < PAGE_MAXENTRYCOUNT ; i++ )
	{
		kSetPageEntryData( &( pstPML4TEntry[ i ] ), 0, 0, 0, 0 );
	}
	
	// ������ ���͸� ������ ���̺� ����
	// �ϳ��� PDPT�� 512GByte���� ���� �����ϹǷ� �ϳ��� �����
	// 64���� ��Ʈ���� �����Ͽ� 64GByte���� ������
	pstPDPTEntry = ( PDPTENTRY* ) 0x101000;
	for( i = 0 ; i < 64 ; i++ )
	{
		kSetPageEntryData( &( pstPDPTEntry[ i ] ), 0, 0x103000 + ( i * PAGE_TABLESIZE ), 
				PAGE_FLAGS_DEFAULT, 0 );
	}
	for( i = 64 ; i < PAGE_MAXENTRYCOUNT ; i++ )
	{
		kSetPageEntryData( &( pstPDPTEntry[ i ] ), 0, 0, 0, 0 );
	}

	// Make PDTMP Table
	// One PDTMP can map 2MByte and We make 512 Entry that can map 4KByte Page
	pstPDTMPEntry = (PDTMPENTRY*) 0x102000;
	dwMappingAddress = 0;
	for(i=0; i<PAGE_MAXENTRYCOUNT; i++){
		if(i == 511){ // 0x1FF000 Read Only (Just Present Flag)
			kSetPageEntryData(&(pstPDTMPEntry[i]), 0, dwMappingAddress, 0x01, 0);
		}
		else if(i == 510){ // 0x1FE000 Read & Write (Present + Read&Write Flag)
			kSetPageEntryData(&(pstPDTMPEntry[i]), 0, dwMappingAddress, PAGE_FLAGS_DEFAULT, 0);
		}
		else{
			kSetPageEntryData(&(pstPDTMPEntry[i]), 0, dwMappingAddress, PAGE_FLAGS_DEFAULT, 0);
		}
		dwMappingAddress += 0x1000;
	}
	
	// ������ ���͸� ���̺� ����
	// �ϳ��� ������ ���͸��� 1GByte���� ���� ���� 
	// �����ְ� 64���� ������ ���͸��� �����Ͽ� �� 64GB���� ����
	pstPDEntry = ( PDENTRY* ) 0x103000;
	dwMappingAddress = 0;
	for( i = 0 ; i < PAGE_MAXENTRYCOUNT * 64 ; i++ )
	{
		// 32��Ʈ�δ� ���� ��巹���� ǥ���� �� �����Ƿ�, Mbyte ������ ����� ����
		// ���� ����� �ٽ� 4Kbyte�� ������ 32��Ʈ �̻��� ��巹���� �����
		if(i == 0){ // First Entry reference PDTMP Table 0x102000, PS Flag Set 0x00 So it's 4KB Paging Part
			kSetPageEntryData( &( pstPDEntry[ i ] ), ( i * ( PAGE_DEFAULTSIZE >> 20 ) ) >> 12, 0x102000, PAGE_FLAGS_DEFAULT | 0x00, 0 );
		}
		else if(i == 5){ // 0xAB8000 Video Memory Double Mapping with 0xB8000 (0xA00000~0xC00000) -> (0~0x200000)
			kSetPageEntryData( &( pstPDEntry[ i ] ), ( i * ( PAGE_DEFAULTSIZE >> 20 ) ) >> 12, 0, PAGE_FLAGS_DEFAULT | PAGE_FLAGS_PS, 0 );
		}
		else{
			kSetPageEntryData( &( pstPDEntry[ i ] ), ( i * ( PAGE_DEFAULTSIZE >> 20 ) ) >> 12, dwMappingAddress, PAGE_FLAGS_DEFAULT | PAGE_FLAGS_PS, 0 );
		}
		dwMappingAddress += PAGE_DEFAULTSIZE;
	}	
}

/**
 *	������ ��Ʈ���� ���� �ּҿ� �Ӽ� �÷��׸� ����
 */
void kSetPageEntryData( PTENTRY* pstEntry, DWORD dwUpperBaseAddress,
		DWORD dwLowerBaseAddress, DWORD dwLowerFlags, DWORD dwUpperFlags )
{
	pstEntry->dwAttributeAndLowerBaseAddress = dwLowerBaseAddress | dwLowerFlags;
	pstEntry->dwUpperBaseAddressAndEXB = ( dwUpperBaseAddress & 0xFF ) | 
		dwUpperFlags;
}
