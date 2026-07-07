// netprop_dump.h (финальная улучшенная версия, без внешних зависимостей)
#ifndef NETPROP_DUMP_H
#define NETPROP_DUMP_H

#include "tier0/platform.h"
#include "filesystem.h"
#include "convar.h"
#include "dt_common.h"
#include "dt_send.h"
#include "dt_recv.h"
#include "checksum_crc.h"

#if defined(CLIENT_DLL)
#include "client_class.h"
#elif defined(GAME_DLL)
#include "server_class.h"
#endif

// --------------------------------------------------------------
// Вспомогательные функции
// --------------------------------------------------------------
static const char* GetDTTypeName(int type)
{
    switch (type)
    {
    case DPT_Int:       return "integer";
    case DPT_Float:     return "float";
    case DPT_Vector:    return "vector";
    case DPT_String:    return "string";
    case DPT_Array:     return "array";
    case DPT_DataTable: return "datatable";
    default:            return "unknown";
    }
}

static const char* SendFlagsToString(int flags)
{
    static char str[1024];
    str[0] = '\0';
    if (flags & SPROP_UNSIGNED)           Q_strncat(str, "Unsigned|", sizeof(str));
    if (flags & SPROP_COORD)              Q_strncat(str, "Coord|", sizeof(str));
    if (flags & SPROP_NOSCALE)            Q_strncat(str, "NoScale|", sizeof(str));
    if (flags & SPROP_ROUNDDOWN)          Q_strncat(str, "RoundDown|", sizeof(str));
    if (flags & SPROP_ROUNDUP)            Q_strncat(str, "RoundUp|", sizeof(str));
    if (flags & SPROP_NORMAL)             Q_strncat(str, "Normal|", sizeof(str));
    if (flags & SPROP_EXCLUDE)            Q_strncat(str, "Exclude|", sizeof(str));
    if (flags & SPROP_XYZE)               Q_strncat(str, "XYZE|", sizeof(str));
    if (flags & SPROP_INSIDEARRAY)        Q_strncat(str, "InsideArray|", sizeof(str));
    if (flags & SPROP_PROXY_ALWAYS_YES)   Q_strncat(str, "ProxyAlwaysYes|", sizeof(str));
    if (flags & SPROP_CHANGES_OFTEN)      Q_strncat(str, "ChangesOften|", sizeof(str));
    if (flags & SPROP_IS_A_VECTOR_ELEM)   Q_strncat(str, "VectorElem|", sizeof(str));
    if (flags & SPROP_COLLAPSIBLE)        Q_strncat(str, "Collapsible|", sizeof(str));
    int len = Q_strlen(str);
    if (len > 0 && str[len - 1] == '|')
        str[len - 1] = '\0';
    return str;
}

// --------------------------------------------------------------
// Запись таблиц в файл
// --------------------------------------------------------------
static void WriteSendTable(FileHandle_t hFile, SendTable* pTable, int level)
{
    char indent[64];
    for (int i = 0; i < level; ++i) indent[i] = ' ';
    indent[level] = '\0';

    for (int i = 0; i < pTable->GetNumProps(); ++i)
    {
        SendProp* pProp = pTable->GetProp(i);
        if (!pProp) continue;

        if (pProp->GetType() == DPT_DataTable)
        {
            SendTable* pInner = pProp->GetDataTable();
            if (pInner)
            {
                char buf[256];
                Q_snprintf(buf, sizeof(buf), "%sTable: %s (offset %d) (type %s)\n",
                    indent, pProp->GetName(), pProp->GetOffset(), pInner->GetName());
                g_pFullFileSystem->Write(buf, Q_strlen(buf), hFile);
                WriteSendTable(hFile, pInner, level + 1);
            }
        }
        else
        {
            char buf[256];
            Q_snprintf(buf, sizeof(buf), "%sMember: %s (offset %d) (type %s) (bits %d) (%s)\n",
                indent, pProp->GetName(), pProp->GetOffset(), GetDTTypeName(pProp->GetType()),
                pProp->m_nBits, SendFlagsToString(pProp->GetFlags()));
            g_pFullFileSystem->Write(buf, Q_strlen(buf), hFile);
        }
    }
}

static void WriteRecvTable(FileHandle_t hFile, RecvTable* pTable, int level)
{
    char indent[64];
    for (int i = 0; i < level; ++i) indent[i] = ' ';
    indent[level] = '\0';

    for (int i = 0; i < pTable->GetNumProps(); ++i)
    {
        RecvProp* pProp = pTable->GetProp(i);
        if (!pProp) continue;

        if (pProp->GetType() == DPT_DataTable)
        {
            RecvTable* pInner = pProp->GetDataTable();
            if (pInner)
            {
                char buf[256];
                Q_snprintf(buf, sizeof(buf), "%sTable: %s (offset %d) (type %s)\n",
                    indent, pProp->GetName(), pProp->GetOffset(), pInner->GetName());
                g_pFullFileSystem->Write(buf, Q_strlen(buf), hFile);
                WriteRecvTable(hFile, pInner, level + 1);
            }
        }
        else
        {
            char buf[256];
            Q_snprintf(buf, sizeof(buf), "%sMember: %s (offset %d) (type %s) (bits N/A) (%s)\n",
                indent, pProp->GetName(), pProp->GetOffset(), GetDTTypeName(pProp->GetType()),
                SendFlagsToString(pProp->GetFlags()));
            g_pFullFileSystem->Write(buf, Q_strlen(buf), hFile);
        }
    }
}

// --------------------------------------------------------------
// Функции CRC (без учёта offset)
// --------------------------------------------------------------
static CRC32_t SendTable_CRCTable_Local(CRC32_t& crc, SendTable* pTable)
{
    if (!pTable) return crc;
    CRC32_ProcessBuffer(&crc, (void*)pTable->GetName(), Q_strlen(pTable->GetName()));
    int nProps = pTable->GetNumProps();
    CRC32_ProcessBuffer(&crc, (void*)&nProps, sizeof(nProps));

    for (int i = 0; i < nProps; ++i)
    {
        SendProp* pProp = pTable->GetProp(i);
        if (!pProp) continue;

        int type = pProp->GetType();
        CRC32_ProcessBuffer(&crc, (void*)&type, sizeof(type));
        CRC32_ProcessBuffer(&crc, (void*)pProp->GetName(), Q_strlen(pProp->GetName()));
        int flags = pProp->GetFlags();
        CRC32_ProcessBuffer(&crc, (void*)&flags, sizeof(flags));

        if (type == DPT_DataTable)
        {
            SendTable* child = pProp->GetDataTable();
            if (child)
                CRC32_ProcessBuffer(&crc, (void*)child->GetName(), Q_strlen(child->GetName()));
        }
        else if (pProp->IsExcludeProp())
        {
            CRC32_ProcessBuffer(&crc, (void*)pProp->GetExcludeDTName(), Q_strlen(pProp->GetExcludeDTName()));
        }
        else if (type == DPT_Array)
        {
            int elems = pProp->GetNumElements();
            CRC32_ProcessBuffer(&crc, (void*)&elems, sizeof(elems));
        }
        else
        {
            float low = pProp->m_fLowValue, high = pProp->m_fHighValue;
            int bits = pProp->m_nBits;
            CRC32_ProcessBuffer(&crc, (void*)&low, sizeof(low));
            CRC32_ProcessBuffer(&crc, (void*)&high, sizeof(high));
            CRC32_ProcessBuffer(&crc, (void*)&bits, sizeof(bits));
        }
    }
    return crc;
}

static CRC32_t RecvTable_CRCTable_Local(CRC32_t& crc, RecvTable* pTable)
{
    if (!pTable) return crc;
    CRC32_ProcessBuffer(&crc, (void*)pTable->GetName(), Q_strlen(pTable->GetName()));
    int nProps = pTable->GetNumProps();
    CRC32_ProcessBuffer(&crc, (void*)&nProps, sizeof(nProps));

    for (int i = 0; i < nProps; ++i)
    {
        RecvProp* pProp = pTable->GetProp(i);
        if (!pProp) continue;

        int type = pProp->GetType();
        CRC32_ProcessBuffer(&crc, (void*)&type, sizeof(type));
        CRC32_ProcessBuffer(&crc, (void*)pProp->GetName(), Q_strlen(pProp->GetName()));
        int flags = pProp->GetFlags();
        CRC32_ProcessBuffer(&crc, (void*)&flags, sizeof(flags));

        if (type == DPT_DataTable)
        {
            RecvTable* child = pProp->GetDataTable();
            if (child)
                CRC32_ProcessBuffer(&crc, (void*)child->GetName(), Q_strlen(child->GetName()));
        }
        else if (type == DPT_Array)
        {
            int elems = pProp->GetNumElements();
            CRC32_ProcessBuffer(&crc, (void*)&elems, sizeof(elems));
        }
    }
    return crc;
}

// --------------------------------------------------------------
// Поиск свойств по подстроке (рекурсивный обход с выводом в консоль)
// --------------------------------------------------------------
static void FindSendProps_R(SendTable* pTable, const char* pathPrefix, const char* substr)
{
    if (!pTable) return;
    for (int i = 0; i < pTable->GetNumProps(); ++i)
    {
        SendProp* pProp = pTable->GetProp(i);
        if (!pProp) continue;
        const char* propName = pProp->GetName();
        char fullPath[512];
        Q_snprintf(fullPath, sizeof(fullPath), "%s/%s", pathPrefix, propName);

        if (pProp->GetType() == DPT_DataTable)
        {
            FindSendProps_R(pProp->GetDataTable(), fullPath, substr);
        }
        else if (V_stristr(propName, substr) || V_stristr(fullPath, substr))
        {
            Msg("[Send] %s : %s (type %s, bits %d, flags %s)\n",
                pTable->GetName(), fullPath, GetDTTypeName(pProp->GetType()),
                pProp->m_nBits, SendFlagsToString(pProp->GetFlags()));
        }
    }
}

static void FindRecvProps_R(RecvTable* pTable, const char* pathPrefix, const char* substr)
{
    if (!pTable) return;
    for (int i = 0; i < pTable->GetNumProps(); ++i)
    {
        RecvProp* pProp = pTable->GetProp(i);
        if (!pProp) continue;
        const char* propName = pProp->GetName();
        char fullPath[512];
        Q_snprintf(fullPath, sizeof(fullPath), "%s/%s", pathPrefix, propName);

        if (pProp->GetType() == DPT_DataTable)
        {
            FindRecvProps_R(pProp->GetDataTable(), fullPath, substr);
        }
        else if (V_stristr(propName, substr) || V_stristr(fullPath, substr))
        {
            Msg("[Recv] %s : %s (type %s, flags %s)\n",
                pTable->GetName(), fullPath, GetDTTypeName(pProp->GetType()),
                SendFlagsToString(pProp->GetFlags()));
        }
    }
}

// --------------------------------------------------------------
// КОНСОЛЬНЫЕ КОМАНДЫ
// --------------------------------------------------------------
#if defined(CLIENT_DLL)

CON_COMMAND(dump_client_props, "Dump all client RecvTables to client_props_dump.txt")
{
    FileHandle_t hFile = g_pFullFileSystem->Open("client_props_dump.txt", "w", "DEFAULT_WRITE_PATH");
    if (hFile == FILESYSTEM_INVALID_HANDLE) { Warning("Failed to create file\n"); return; }
    for (ClientClass* pClass = g_pClientClassHead; pClass; pClass = pClass->m_pNext)
    {
        if (!pClass->m_pRecvTable) continue;
        char header[256];
        Q_snprintf(header, sizeof(header), "%s (type %s)\n", pClass->GetName(), pClass->m_pRecvTable->GetName());
        g_pFullFileSystem->Write(header, Q_strlen(header), hFile);
        WriteRecvTable(hFile, pClass->m_pRecvTable, 1);
    }
    g_pFullFileSystem->Close(hFile);
    Msg("Client props dumped to client_props_dump.txt\n");
}

CON_COMMAND(dump_client_crc, "Calculate CRC of all client RecvTables")
{
    CRC32_t crc; CRC32_Init(&crc);
    for (ClientClass* pClass = g_pClientClassHead; pClass; pClass = pClass->m_pNext)
        RecvTable_CRCTable_Local(crc, pClass->m_pRecvTable);
    CRC32_Final(&crc);
    Msg("Client RecvTable CRC: 0x%08X\n", crc);
}

CON_COMMAND(dump_client_table, "Dump a specific client RecvTable by name to single_table_dump.txt")
{
    if (args.ArgC() < 2) { Msg("Usage: dump_client_table <name>\n"); return; }
    const char* name = args[1];
    for (ClientClass* pClass = g_pClientClassHead; pClass; pClass = pClass->m_pNext)
    {
        if (!Q_stricmp(pClass->m_pRecvTable->GetName(), name))
        {
            FileHandle_t hFile = g_pFullFileSystem->Open("single_table_dump.txt", "w", "DEFAULT_WRITE_PATH");
            WriteRecvTable(hFile, pClass->m_pRecvTable, 1);
            g_pFullFileSystem->Close(hFile);
            Msg("Dumped %s to single_table_dump.txt\n", name);
            return;
        }
    }
    Msg("Table '%s' not found.\n", name);
}

CON_COMMAND(find_prop_client, "Find client props containing substring. Usage: find_prop_client <substr>")
{
    if (args.ArgC() < 2) { Msg("Usage: find_prop_client <substr>\n"); return; }
    const char* substr = args[1];
    for (ClientClass* pClass = g_pClientClassHead; pClass; pClass = pClass->m_pNext)
        FindRecvProps_R(pClass->m_pRecvTable, pClass->m_pRecvTable->GetName(), substr);
}

CON_COMMAND(list_client_tables, "List all client RecvTables")
{
    for (ClientClass* pClass = g_pClientClassHead; pClass; pClass = pClass->m_pNext)
        Msg("%s (type %s)\n", pClass->GetName(), pClass->m_pRecvTable->GetName());
}

#elif defined(GAME_DLL)

static CRC32_t ComputeServerCRC()
{
    CRC32_t crc; CRC32_Init(&crc);
    for (ServerClass* pClass = g_pServerClassHead; pClass; pClass = pClass->m_pNext)
        if (pClass->m_pTable)
            SendTable_CRCTable_Local(crc, pClass->m_pTable);
    CRC32_Final(&crc);
    return crc;
}

CON_COMMAND(dump_server_props, "Dump all server SendTables to server_props_dump.txt")
{
    FileHandle_t hFile = g_pFullFileSystem->Open("server_props_dump.txt", "w", "DEFAULT_WRITE_PATH");
    if (hFile == FILESYSTEM_INVALID_HANDLE) { Warning("Failed to create file\n"); return; }
    for (ServerClass* pClass = g_pServerClassHead; pClass; pClass = pClass->m_pNext)
    {
        if (!pClass->m_pTable) continue;
        char header[256];
        Q_snprintf(header, sizeof(header), "%s (type %s)\n", pClass->GetName(), pClass->m_pTable->GetName());
        g_pFullFileSystem->Write(header, Q_strlen(header), hFile);
        WriteSendTable(hFile, pClass->m_pTable, 1);
    }
    g_pFullFileSystem->Close(hFile);
    Msg("Server props dumped to server_props_dump.txt\n");
}

CON_COMMAND(dump_server_crc, "Calculate and display server SendTable CRC")
{
    Msg("Server SendTable CRC: 0x%08X\n", ComputeServerCRC());
}

CON_COMMAND(dump_server_table, "Dump a specific server SendTable by name to single_table_dump.txt")
{
    if (args.ArgC() < 2) { Msg("Usage: dump_server_table <name>\n"); return; }
    const char* name = args[1];
    for (ServerClass* pClass = g_pServerClassHead; pClass; pClass = pClass->m_pNext)
    {
        if (!Q_stricmp(pClass->m_pTable->GetName(), name))
        {
            FileHandle_t hFile = g_pFullFileSystem->Open("single_table_dump.txt", "w", "DEFAULT_WRITE_PATH");
            WriteSendTable(hFile, pClass->m_pTable, 1);
            g_pFullFileSystem->Close(hFile);
            Msg("Dumped %s to single_table_dump.txt\n", name);
            return;
        }
    }
    Msg("Table '%s' not found.\n", name);
}

CON_COMMAND(find_prop_server, "Find server props containing substring. Usage: find_prop_server <substr>")
{
    if (args.ArgC() < 2) { Msg("Usage: find_prop_server <substr>\n"); return; }
    const char* substr = args[1];
    for (ServerClass* pClass = g_pServerClassHead; pClass; pClass = pClass->m_pNext)
        FindSendProps_R(pClass->m_pTable, pClass->m_pTable->GetName(), substr);
}

CON_COMMAND(list_server_tables, "List all server SendTables")
{
    for (ServerClass* pClass = g_pServerClassHead; pClass; pClass = pClass->m_pNext)
        Msg("%s (type %s)\n", pClass->GetName(), pClass->m_pTable->GetName());
}

CON_COMMAND(compare_crc_with_file, "Compare current server CRC with CRC stored in file")
{
    if (args.ArgC() < 2) { Msg("Usage: compare_crc_with_file <filename>\n"); return; }
    FileHandle_t hFile = g_pFullFileSystem->Open(args[1], "r", "DEFAULT_WRITE_PATH");
    if (hFile == FILESYSTEM_INVALID_HANDLE) { Msg("Cannot open file\n"); return; }
    char line[64];
    g_pFullFileSystem->ReadLine(line, sizeof(line), hFile);
    g_pFullFileSystem->Close(hFile);
    uint32 fileCRC = (uint32)strtoul(line, NULL, 16);
    CRC32_t curCRC = ComputeServerCRC();
    if (fileCRC == curCRC)
        Msg("CRC matches (0x%08X)\n", curCRC);
    else
        Msg("CRC mismatch: current=0x%08X, file=0x%08X\n", curCRC, fileCRC);
}

#endif // CLIENT_DLL / GAME_DLL

#endif // NETPROP_DUMP_H