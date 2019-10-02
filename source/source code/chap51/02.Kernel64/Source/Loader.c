/**
 *  file    ApplicationLoader.c
 *  date    2009/12/26
 *  author  kkamagui
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   �������α׷��� �ε��Ͽ� �����ϴ� �δ�(Loader)�� ���õ� �Լ��� ������ �ҽ� ����
 */

#include "Loader.h"
#include "FileSystem.h"
#include "DynamicMemory.h"

/**
 *  �������α׷��� ����
 */
QWORD kExecuteProgram( const char* pcFileName, const char* pcArgumentString, 
        BYTE bAffinity )
{
    DIR* pstDirectory;
    struct dirent* pstEntry;
    DWORD dwFileSize;
    BYTE* pbTempFileBuffer;
    FILE* pstFile;
    DWORD dwReadSize;
    QWORD qwEntryPointAddress;
    QWORD qwApplicationMemory;
    QWORD qwMemorySize;
    TCB* pstTask;
    
    //--------------------------------------------------------------------------
    // ��Ʈ ���͸��� ��� ������ �˻�
    //--------------------------------------------------------------------------
    pstDirectory = opendir( "/" );
    dwFileSize = 0;
    
    // ���͸����� ������ �˻�
    while( 1 )
    {
        // ���͸����� ��Ʈ�� �ϳ��� ����
        pstEntry = readdir( pstDirectory );
        // ���̻� ������ ������ ����
        if( pstEntry == NULL )
        {
            break;
        }
        
        // ���� �̸��� ���̿� ������ ���� ���� �˻�
        if( ( kStrLen( pstEntry->d_name ) == kStrLen( pcFileName ) ) &&
            ( kMemCmp( pstEntry->d_name, pcFileName, kStrLen( pcFileName ) ) 
                    == 0 ) )
        {
            dwFileSize = pstEntry->dwFileSize;
            break;
        }
    }
    // ���͸� �ڵ��� ��ȯ, �ڵ��� ��ȯ���� ������ �޸𸮰� �������� �ʰ� �����Ƿ�
    // �� �����ؾ� ��
    closedir( pstDirectory );

    if( dwFileSize == 0 )
    {
        kPrintf( "%s file doesn't exist or size is zero\n", 
                pcFileName );
        return TASK_INVALIDID;
    }
    
    //--------------------------------------------------------------------------
    // ���� ��ü�� ������ �� �ִ� �ӽ� ���۸� �Ҵ� �޾Ƽ� ������ ������ ��� ����
    //--------------------------------------------------------------------------
    // �޸� �Ҵ�
    pbTempFileBuffer = ( BYTE* ) kAllocateMemory( dwFileSize );
    if( pbTempFileBuffer == NULL )
    {
        kPrintf( "Memory %dbytes allocate fail\n", 
                 dwFileSize );
        return TASK_INVALIDID;
    }
    
    // ������ ��� ��� �о� �޸𸮿� ����
    pstFile = fopen( pcFileName, "r" );
    if( ( pstFile != NULL ) && 
        ( fread( pbTempFileBuffer, 1, dwFileSize, pstFile ) == dwFileSize ) )
    {
        fclose( pstFile );
        kPrintf( "%s file read success\n", pcFileName );
    }
    else
    {
        kPrintf( "%s file read fail\n", pcFileName );
        kFreeMemory( pbTempFileBuffer );
        fclose( pstFile );
        return TASK_INVALIDID;
    }

    //--------------------------------------------------------------------------
    // ������ ������ �м��Ͽ� ������ �ε��ϰ� ���ġ�� ����
    //--------------------------------------------------------------------------
    if( kLoadProgramAndRelocation( pbTempFileBuffer, &qwApplicationMemory, 
            &qwMemorySize, &qwEntryPointAddress ) == FALSE )
    {
        kPrintf( "%s file is invalid application file or loading fail\n", 
                 pcFileName );
        kFreeMemory( pbTempFileBuffer );
        return TASK_INVALIDID;
    }
    
    // �޸� ����
    kFreeMemory( pbTempFileBuffer );
    
    //--------------------------------------------------------------------------
    // �½�ũ�� �����ϰ� ���ÿ� ���� ���ڿ��� ����
    //--------------------------------------------------------------------------
    // ���� ���� �������α׷� �½�ũ ����
    pstTask = kCreateTask( TASK_FLAGS_USERLEVEL | TASK_FLAGS_PROCESS, 
        ( void* ) qwApplicationMemory, qwMemorySize, qwEntryPointAddress, 
        bAffinity );
    if( pstTask == NULL )
    {
        kFreeMemory( ( void* ) qwApplicationMemory );
        return TASK_INVALIDID;
    }
    
    // ���� ���ڿ� ����
    kAddArgumentStringToTask( pstTask, pcArgumentString );
    
    return pstTask->stLink.qwID;
}

/**
 *  �������α׷��� ������ �ε��ϰ� ���ġ�� ����
 */
static BOOL kLoadProgramAndRelocation( BYTE* pbFileBuffer, 
        QWORD* pqwApplicationMemoryAddress, QWORD* pqwApplicationMemorySize, 
        QWORD* pqwEntryPointAddress )
{
    Elf64_Ehdr* pstELFHeader;
    Elf64_Shdr* pstSectionHeader;
    Elf64_Shdr* pstSectionNameTableHeader;
    Elf64_Xword qwLastSectionSize;
    Elf64_Addr qwLastSectionAddress;
    int i;
    QWORD qwMemorySize;
    QWORD qwStackAddress;
    BYTE* pbLoadedAddress;

    //--------------------------------------------------------------------------
    // ELF ��� ������ ����ϰ� �м��� �ʿ��� ������ ����
    //--------------------------------------------------------------------------
    pstELFHeader = ( Elf64_Ehdr* ) pbFileBuffer;
    pstSectionHeader = ( Elf64_Shdr* ) ( pbFileBuffer + pstELFHeader->e_shoff );
    pstSectionNameTableHeader = pstSectionHeader + pstELFHeader->e_shstrndx;

    kPrintf( "========================= ELF Header Info =========================\n" );
    kPrintf( "Magic Number [%c%c%c] Section Header Count [%d]\n", 
            pstELFHeader->e_ident[ 1 ], pstELFHeader->e_ident[ 2 ], 
        pstELFHeader->e_ident[ 3 ], pstELFHeader->e_shnum );
    kPrintf( "File Type [%d]\n", pstELFHeader->e_type );
    kPrintf( "Section Header Offset [0x%X] Size [0x%X]\n", pstELFHeader->e_shoff, 
            pstELFHeader->e_shentsize );
    kPrintf( "Program Header Offset [0x%X] Size [0x%X]\n", pstELFHeader->e_phoff, 
            pstELFHeader->e_phentsize );
    kPrintf( "Section Name String Table Section Index [%d]\n", pstELFHeader->e_shstrndx );
    
    // ELF�� ID�� Ŭ����, ���ڵ�, �׸��� Ÿ���� Ȯ���Ͽ� �ùٸ� �������α׷����� Ȯ��
    if( ( pstELFHeader->e_ident[ EI_MAG0 ] != ELFMAG0 ) ||
        ( pstELFHeader->e_ident[ EI_MAG1 ] != ELFMAG1 ) ||
        ( pstELFHeader->e_ident[ EI_MAG2 ] != ELFMAG2 ) ||
        ( pstELFHeader->e_ident[ EI_MAG3 ] != ELFMAG3 ) ||
        ( pstELFHeader->e_ident[ EI_CLASS ] != ELFCLASS64 ) ||
        ( pstELFHeader->e_ident[ EI_DATA ] != ELFDATA2LSB ) ||
        ( pstELFHeader->e_type != ET_REL ) )
    {
        return FALSE;
    }

    //--------------------------------------------------------------------------
    // ��� ���� ����� �ε��� �޸� ��巹���� Ȯ���Ͽ� ���� �������� �ִ� ������ ã��
    // ������ ������ ���� ǥ��
    //--------------------------------------------------------------------------
    qwLastSectionAddress = 0;
    qwLastSectionSize = 0;
    for( i = 0 ; i < pstELFHeader->e_shnum ; i++ )
    {
        // ���� ������ �������� Ȯ��, �� ������ ���α׷��� ����� ��ü �޸� ũ�⸦
        // �� �� ����
        if( ( pstSectionHeader[ i ].sh_flags & SHF_ALLOC ) &&                
            ( pstSectionHeader[ i ].sh_addr >= qwLastSectionAddress ) )
        {
            qwLastSectionAddress = pstSectionHeader[ i ].sh_addr;
            qwLastSectionSize = pstSectionHeader[ i ].sh_size;
        }
    }
    
    kPrintf( "\n========================= Load & Relocaion ========================\n" );
    // ������ Section�� ��ġ �� ũ�⸦ ǥ��
    kPrintf( "Last Section Address [0x%q] Size [0x%q]\n", qwLastSectionAddress,
            qwLastSectionSize );

    // ������ ������ ��ġ�� �ִ� �޸� ���� ���, 4Kbyte ������ ����
    qwMemorySize = ( qwLastSectionAddress + qwLastSectionSize + 0x1000 - 1 ) & 
        0xfffffffffffff000;
    kPrintf( "Aligned Memory Size [0x%q]\n", qwMemorySize );
    
    // �������α׷����� ����� �޸𸮸� �Ҵ�
    pbLoadedAddress = ( char* ) kAllocateMemory( qwMemorySize );
    if( pbLoadedAddress == NULL )
    {
        kPrintf( "Memory allocate fail\n" );
        return FALSE;
    }
    else
    {
        kPrintf( "Loaded Address [0x%q]\n", pbLoadedAddress ); 
    }
    
    //--------------------------------------------------------------------------
    // ���Ͽ� �ִ� ������ �޸𸮿� ����(�ε�)
    //--------------------------------------------------------------------------
    for( i = 1 ; i < pstELFHeader->e_shnum ; i++ )
    {
        // �޸𸮿� �ø� �ʿ䰡 ���� �����̰ų� Size�� 0�� Section�̸� ������ �ʿ� ����
        if( !( pstSectionHeader[ i ].sh_flags & SHF_ALLOC ) ||
            ( pstSectionHeader[ i ].sh_size == 0 ) )
        {
            continue;
        }
        
        // ���� ����� �ε��� ��巹���� ����
        pstSectionHeader[ i ].sh_addr += ( Elf64_Addr ) pbLoadedAddress;        
        
        // .bss�� ���� SHT_NOBITS�� ������ ������ ���Ͽ� �����Ͱ� �����Ƿ� 0���� �ʱ�ȭ
        if( pstSectionHeader[ i ].sh_type == SHT_NOBITS)
        {
            // �������α׷����� �Ҵ�� �޸𸮸� 0���� ����
            kMemSet( pstSectionHeader[ i ].sh_addr, 0, pstSectionHeader[ i ].sh_size );
        }
        else
        {
            // ���� ������ ������ �������α׷����� �Ҵ�� �޸𸮷� ����
            kMemCpy( pstSectionHeader[ i ].sh_addr, 
                    pbFileBuffer + pstSectionHeader[ i ].sh_offset,
                    pstSectionHeader[ i ].sh_size );
        }        
        kPrintf( "Section [%x] Virtual Address [%q] File Address [%q] Size [%q]\n",
                i,
                pstSectionHeader[ i ].sh_addr, 
                pbFileBuffer + pstSectionHeader[ i ].sh_offset,
                pstSectionHeader[ i ].sh_size );
    }    
    kPrintf( "Program load success\n" );
    
    //--------------------------------------------------------------------------
    // ���ġ�� ����
    //--------------------------------------------------------------------------
    if( kRelocation( pbFileBuffer ) == FALSE )
    {
        kPrintf( "Relocation fail\n" );
        return FALSE;
    }
    else
    {
        kPrintf( "Relocation success\n" );
    }
    
    // �������α׷��� ��巹���� ��Ʈ�� ����Ʈ�� ��巹���� ��ȯ
    *pqwApplicationMemoryAddress = ( QWORD ) pbLoadedAddress;
    *pqwApplicationMemorySize = qwMemorySize;
    *pqwEntryPointAddress = pstELFHeader->e_entry + ( QWORD ) pbLoadedAddress;

    return TRUE;
}
    

/**
 *  ���ġ�� ����
 *      ���� ������� �޸� ��巹���� �Ҵ�Ǿ� �־�� ��
*/
static BOOL kRelocation( BYTE* pbFileBuffer )
{
    Elf64_Ehdr* pstELFHeader;
    Elf64_Shdr* pstSectionHeader;
    int i;
    int j;
    int iSymbolTableIndex;
    int iSectionIndexInSymbol;
    int iSectionIndexToRelocation;
    Elf64_Addr ulOffset;
    Elf64_Xword ulInfo;
    Elf64_Sxword lAddend;
    Elf64_Sxword lResult;
    int iNumberOfBytes;
    Elf64_Rel* pstRel;
    Elf64_Rela* pstRela;
    Elf64_Sym* pstSymbolTable;
    
    // ELF ����� ���� ��� ���̺��� ù ��° ����� ã��
    pstELFHeader = ( Elf64_Ehdr* ) pbFileBuffer;
    pstSectionHeader = ( Elf64_Shdr* ) ( pbFileBuffer + pstELFHeader->e_shoff );

    //--------------------------------------------------------------------------
    // ��� ���� ����� �˻��Ͽ� SHT_REL �Ǵ� SHT_RELA Ÿ���� ������ ������ ã�� 
    // ���ġ�� ����
    //--------------------------------------------------------------------------
    for( i = 1 ; i < pstELFHeader->e_shnum ; i++ )
    {
        if( ( pstSectionHeader[ i ].sh_type != SHT_RELA ) && 
            ( pstSectionHeader[ i ].sh_type != SHT_REL ) )
        {
            continue;
        }

        // sh_info �ʵ忡 ���ġ�� �����ؾ� �� ���� ����� �ε����� ����Ǿ� ����
        iSectionIndexToRelocation = pstSectionHeader[ i ].sh_info;
        
        // sh_link���� �����ϴ� �ɺ� ���̺� ���� ����� �ε����� ����Ǿ� ����
        iSymbolTableIndex = pstSectionHeader[ i ].sh_link;
        
        // �ɺ� ���̺� ������ ù ��° ��Ʈ���� ����
        pstSymbolTable = ( Elf64_Sym* ) 
            ( pbFileBuffer + pstSectionHeader[ iSymbolTableIndex ].sh_offset );

        //----------------------------------------------------------------------
        // ���ġ ������ ��Ʈ���� ��� ã�� ���ġ�� ���� 
        //----------------------------------------------------------------------
        for( j = 0 ; j < pstSectionHeader[ i ].sh_size ; )
        {
            // SHT_REL Ÿ��
            if( pstSectionHeader[ i ].sh_type == SHT_REL )
            {
                // SHT_REL Ÿ���� ���ؾ��ϴ� ��(Addend)�� �����Ƿ� 0���� ����
                pstRel = ( Elf64_Rel* ) 
                    ( pbFileBuffer + pstSectionHeader[ i ].sh_offset + j );
                ulOffset = pstRel->r_offset;
                ulInfo = pstRel->r_info;
                lAddend = 0;

                // SHT_REL �ڷᱸ���� ũ�⸸ŭ �̵�
                j += sizeof( Elf64_Rel );
            }
            // SHT_RELA Ÿ��
            else
            {
                pstRela = ( Elf64_Rela* ) 
                    ( pbFileBuffer + pstSectionHeader[ i ].sh_offset + j );
                ulOffset = pstRela->r_offset;
                ulInfo = pstRela->r_info;
                lAddend = pstRela->r_addend;

                // SHT_RELA �ڷᱸ���� ũ�⸸ŭ �̵�
                j += sizeof( Elf64_Rela );
            }

            // ���� ��巹�� Ÿ��(Absolute Type)�� ���� ���ġ�� �ʿ� ����
            if( pstSymbolTable[ RELOCATION_UPPER32( ulInfo ) ].st_shndx == SHN_ABS )
            {
                continue;
            }
            // ���� Ÿ�� �ɺ�(Common Type)�� ���� �������� �����Ƿ� ������ ǥ���ϰ� ����
            else if( pstSymbolTable[ RELOCATION_UPPER32( ulInfo ) ].st_shndx == 
                SHN_COMMON )
            {
                kPrintf( "Common symbol is not supported\n" );
                return FALSE;
            }

            //------------------------------------------------------------------
            // ���ġ Ÿ���� ���Ͽ� ���ġ�� ������ ���� ���
            //------------------------------------------------------------------
            switch( RELOCATION_LOWER32( ulInfo ) )
            {
                // S(st_value) + A(r_addend)�� ����ϴ� Ÿ��
            case R_X86_64_64:
            case R_X86_64_32:
            case R_X86_64_32S:
            case R_X86_64_16:
            case R_X86_64_8:
                // �ɺ��� �����ϴ� ���� ����� �ε���
                iSectionIndexInSymbol = 
                    pstSymbolTable[ RELOCATION_UPPER32( ulInfo ) ].st_shndx;
                
                lResult = ( pstSymbolTable[ RELOCATION_UPPER32( ulInfo ) ].st_value + 
                    pstSectionHeader[ iSectionIndexInSymbol ].sh_addr ) + lAddend;
                break;

                // S(st_value) + A(r_addend) - P(r_offset)�� ����ϴ� Ÿ��
            case R_X86_64_PC32:
            case R_X86_64_PC16:
            case R_X86_64_PC8:
            case R_X86_64_PC64:
                // �ɺ��� �����ϴ� ���� ����� �ε���
                iSectionIndexInSymbol = 
                    pstSymbolTable[ RELOCATION_UPPER32( ulInfo ) ].st_shndx;
                
                lResult = ( pstSymbolTable[ RELOCATION_UPPER32( ulInfo ) ].st_value + 
                    pstSectionHeader[ iSectionIndexInSymbol ].sh_addr ) + lAddend - 
                    ( ulOffset + pstSectionHeader[ iSectionIndexToRelocation ].sh_addr );
                break;

                // B(sh_addr) + A(r_ddend)�� ����ϴ� Ÿ��
            case R_X86_64_RELATIVE:
                lResult = pstSectionHeader[ i ].sh_addr + lAddend;
                break;
                
                // Z(st_size) + A(r_addend)�� ����ϴ� Ÿ��
            case R_X86_64_SIZE32:
            case R_X86_64_SIZE64:
                lResult = pstSymbolTable[ RELOCATION_UPPER32( ulInfo ) ].st_size +
                    lAddend;
                break;

                // �� ���� ���� �������� �����Ƿ� ������ ǥ���ϰ� ����
            default:
                kPrintf( "Unsupported relocation type [%X]\n", 
                         RELOCATION_LOWER32( ulInfo ) );
                return FALSE;
                break;
            }

            //------------------------------------------------------------------
            // ���ġ Ÿ������ ������ ������ ���
            //------------------------------------------------------------------
            switch( RELOCATION_LOWER32( ulInfo ) )
            {
                // 64��Ʈ ũ��
            case R_X86_64_64:
            case R_X86_64_PC64:
            case R_X86_64_SIZE64:
                iNumberOfBytes = 8;
                break;

                // 32��Ʈ ũ��
            case R_X86_64_PC32:
            case R_X86_64_32:
            case R_X86_64_32S:
            case R_X86_64_SIZE32:
                iNumberOfBytes = 4;
                break;

                // 16��Ʈ ũ��
            case R_X86_64_16:
            case R_X86_64_PC16:
                iNumberOfBytes = 2;
                break;

                // 8��Ʈ ũ��
            case R_X86_64_8:
            case R_X86_64_PC8:
                iNumberOfBytes = 1;
                break;

                // ��Ÿ Ÿ���� ������ ǥ���ϰ� ���� 
            default:
                kPrintf( "Unsupported relocation type [%X]\n", 
                         RELOCATION_LOWER32( ulInfo ) );
                return FALSE;
                break;
            }

            //------------------------------------------------------------------
            // ��� ����� ������ ������ �������Ƿ� �ش� ���ǿ� ����
            //------------------------------------------------------------------
            switch( iNumberOfBytes )
            {
            case 8:
                *( ( Elf64_Sxword* ) 
                   ( pstSectionHeader[ iSectionIndexToRelocation ].sh_addr + 
                     ulOffset ) ) += lResult;
                break;

            case 4:
                *( ( int* ) 
                   ( pstSectionHeader[ iSectionIndexToRelocation ].sh_addr + 
                     ulOffset ) ) += ( int ) lResult;
                break;

            case 2:
                *( ( short* ) 
                   ( pstSectionHeader[ iSectionIndexToRelocation ].sh_addr + 
                     ulOffset ) ) += ( short ) lResult;
                break;
            
            case 1:
                *( ( char* ) 
                   ( pstSectionHeader[ iSectionIndexToRelocation ].sh_addr + 
                     ulOffset ) ) += ( char ) lResult;
                break;

                // �� ���� ũ��� �������� �����Ƿ� ������ ǥ���ϰ� ����
            default:
                kPrintf( "Relocation error. Relocation byte size is [%d]byte\n", 
                         iNumberOfBytes );
                return FALSE;
                break;
            }
        }
    }
    return TRUE;
}

/**
 *  �½�ũ�� ���� ���ڿ� ����
 */
static void kAddArgumentStringToTask( TCB* pstTask, const char* pcArgumentString )
{
    int iLength;
    int iAlignedLength;
    QWORD qwNewRSPAddress;
    
    // ���� ���ڿ��� ���� ���
    if( pcArgumentString == NULL )
    {
        iLength = 0;
    }
    else
    {
        // ���� ���ڿ��� �ִ� 1Kbyte���� ����
        iLength = kStrLen( pcArgumentString );

        if( iLength > 1023 )
        {
            iLength = 1023;
        }
    }
    
    // ���� ���ڿ��� ���̸� 8����Ʈ�� ����
    iAlignedLength = ( iLength + 7 ) & 0xFFFFFFF8;
    
    // ���ο� RSP ���������� ���� ����ϰ� ���ÿ� ���� ����Ʈ�� ����
    qwNewRSPAddress = pstTask->stContext.vqRegister[ TASK_RSPOFFSET ] - 
        ( QWORD ) iAlignedLength;
    kMemCpy( ( void* ) qwNewRSPAddress, pcArgumentString, iLength );
    *( ( BYTE* ) qwNewRSPAddress + iLength ) = '\0';
    
    // RSP �������Ϳ� RBP ���������� ���� ���ο� ���� ��巹���� ����
    pstTask->stContext.vqRegister[ TASK_RSPOFFSET ] = qwNewRSPAddress;
    pstTask->stContext.vqRegister[ TASK_RBPOFFSET ] = qwNewRSPAddress;
    
    // ù ��° �Ķ���ͷ� ���Ǵ� RDI �������͸� �Ķ���Ͱ� ����� ������ ��巹���� ����
    pstTask->stContext.vqRegister[ TASK_RDIOFFSET ] = qwNewRSPAddress;
}

/**
 *  �������α׷����� �����ϴ� �����带 ����
 */
QWORD kCreateThread( QWORD qwEntryPoint, QWORD qwArgument, BYTE bAffinity, 
        QWORD qwExitFunction )
{
    TCB* pstTask;
    
    // ���� ���� �������α׷� �½�ũ ����
    pstTask = kCreateTask( TASK_FLAGS_USERLEVEL | TASK_FLAGS_THREAD, NULL, 0, 
                           qwEntryPoint, bAffinity );
    if( pstTask == NULL )
    {
        return TASK_INVALIDID;
    }

    // ����� �� ȣ��Ǵ� kEndTask() �Լ��� ���޹��� �Լ��� ��ü
    // ����� �� ȣ��Ǵ� �Լ��� ���� RSP �������Ͱ� ����Ű�� ����
    *( ( QWORD* ) pstTask->stContext.vqRegister[ TASK_RSPOFFSET ] ) = 
        qwExitFunction;
    
    // ù ��° �Ķ���ͷ� ���Ǵ� RDI �������Ϳ� ���ڸ� ����
    pstTask->stContext.vqRegister[ TASK_RDIOFFSET ] = qwArgument;
    
    return pstTask->stLink.qwID;
}
