#include "ConfigManager.h"
#include "../res/resource.h"
#include "../core/AlgorithmIds.h"
#include <Windows.h>
#include <shlwapi.h>
#include <sstream>

#pragma comment(lib, "shlwapi.lib")

ConfigManager::ConfigManager()
    : m_isFileMode(false)
    , m_stayOnTop(false)
    , m_havalPass3(false)
    , m_havalPass4(false)
    , m_havalPass5(false)
{
}

ConfigManager::~ConfigManager()
{
}

void ConfigManager::Initialize()
{
    m_configFilePath = GetConfigFilePath();
}

std::wstring ConfigManager::GetExecutablePath()
{
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    return std::wstring(exePath);
}

std::wstring ConfigManager::GetConfigFilePath()
{
    std::wstring exePath = GetExecutablePath();
    
    // Get directory and executable name
    wchar_t drive[_MAX_DRIVE];
    wchar_t dir[_MAX_DIR];
    wchar_t fname[_MAX_FNAME];
    wchar_t ext[_MAX_EXT];
    
    _wsplitpath_s(exePath.c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
    
    // Build config file path: same directory + exe name + .ini
    std::wstring configPath = std::wstring(drive) + std::wstring(dir) + std::wstring(fname) + L".ini";
    
    return configPath;
}

std::wstring ConfigManager::ReadIniString(const std::wstring& section, const std::wstring& key, const std::wstring& defaultValue)
{
    wchar_t buffer[1024];
    GetPrivateProfileStringW(section.c_str(), key.c_str(), defaultValue.c_str(), buffer, 1024, m_configFilePath.c_str());
    return std::wstring(buffer);
}

int ConfigManager::ReadIniInt(const std::wstring& section, const std::wstring& key, int defaultValue)
{
    return GetPrivateProfileIntW(section.c_str(), key.c_str(), defaultValue, m_configFilePath.c_str());
}

bool ConfigManager::WriteIniString(const std::wstring& section, const std::wstring& key, const std::wstring& value)
{
    return WritePrivateProfileStringW(section.c_str(), key.c_str(), value.c_str(), m_configFilePath.c_str()) != 0;
}

bool ConfigManager::WriteIniInt(const std::wstring& section, const std::wstring& key, int value)
{
    return WriteIniString(section, key, std::to_wstring(value));
}

std::wstring ConfigManager::GetAlgorithmName(int algorithmId)
{
    // Map algorithm IDs to readable names
    static std::map<int, std::wstring> nameMap = {
        // SHA algorithms
        {IDC_SHA_160, L"SHA-1"},
        {IDC_SHA_224, L"SHA-224"},
        {IDC_SHA_256, L"SHA-256"},
        {IDC_SHA_384, L"SHA-384"},
        {IDC_SHA_512, L"SHA-512"},
        
        // MD algorithms
        {IDC_MD2, L"MD2"},
        {IDC_MD4, L"MD4"},
        {IDC_MD5, L"MD5"},
        
        // MD6 algorithms
        {IDC_MD6_128, L"MD6-128"},
        {IDC_MD6_160, L"MD6-160"},
        {IDC_MD6_192, L"MD6-192"},
        {IDC_MD6_224, L"MD6-224"},
        {IDC_MD6_256, L"MD6-256"},
        {IDC_MD6_384, L"MD6-384"},
        {IDC_MD6_512, L"MD6-512"},
        
        // SHA-3 algorithms
        {IDC_SHA3_224, L"SHA3-224"},
        {IDC_SHA3_256, L"SHA3-256"},
        {IDC_SHA3_384, L"SHA3-384"},
        {IDC_SHA3_512, L"SHA3-512"},
        
        // HAVAL algorithms
        {IDC_HAVAL_128, L"HAVAL-128"},
        {IDC_HAVAL_160, L"HAVAL-160"},
        {IDC_HAVAL_192, L"HAVAL-192"},
        {IDC_HAVAL_224, L"HAVAL-224"},
        {IDC_HAVAL_256, L"HAVAL-256"},
        
        // RIPEMD algorithms
        {IDC_RIPEMD_128, L"RIPEMD-128"},
        {IDC_RIPEMD_160, L"RIPEMD-160"},
        {IDC_RIPEMD_256, L"RIPEMD-256"},
        {IDC_RIPEMD_320, L"RIPEMD-320"},
        
        // Checksums
        {IDC_CRC8, L"CRC8"},
        {IDC_CRC16, L"CRC16"},
        {IDC_CRC32, L"CRC32"},
        {IDC_CRC32C, L"CRC32C"},
        {IDC_CRC64, L"CRC64"},
        {IDC_ADLER32, L"ADLER32"},
        
        // Keccak
        {IDC_KECCAK_224, L"KECCAK-224"},
        {IDC_KECCAK_256, L"KECCAK-256"},
        {IDC_KECCAK_384, L"KECCAK-384"},
        {IDC_KECCAK_512, L"KECCAK-512"},
        
        // SHAKE
        {IDC_SHAKE_128, L"SHAKE-128"},
        {IDC_SHAKE_256, L"SHAKE-256"},
        
        // Others
        {IDC_TIGER, L"TIGER"},
        {IDC_SM3, L"SM3"},
        {IDC_WHIRLPOOL, L"WHIRLPOOL"},
        {IDC_BLAKE2B, L"BLAKE2B"},
        {IDC_BLAKE2S, L"BLAKE2S"},
        {IDC_BLAKE3, L"BLAKE3"},
        {IDC_LSH_256, L"LSH-256"},
        {IDC_LSH_512, L"LSH-512"},
        {IDC_GOST94, L"GOST-94"},
        {IDC_GOST2012_256, L"GOST-256"},
        {IDC_GOST2012_512, L"GOST-512"}
    };
    
    auto it = nameMap.find(algorithmId);
    if (it != nameMap.end()) {
        return it->second;
    }
    return L"";
}

std::wstring ConfigManager::GetAlgorithmSection(int algorithmId)
{
    // Use explicit switch-case for all algorithm IDs to avoid range issues
    switch (algorithmId) {
        // Tab 1: SHA & MD
        case IDC_SHA_160:
        case IDC_SHA_224:
        case IDC_SHA_256:
        case IDC_SHA_384:
        case IDC_SHA_512:
        case IDC_MD2:
        case IDC_MD4:
        case IDC_MD5:
        case IDC_MD6_128:
        case IDC_MD6_160:
        case IDC_MD6_192:
        case IDC_MD6_224:
        case IDC_MD6_256:
        case IDC_MD6_384:
        case IDC_MD6_512:
            return L"Algorithms.SHA_MD";
        
        // Tab 2: SHA-3 & Modern
        case IDC_SHA3_224:
        case IDC_SHA3_256:
        case IDC_SHA3_384:
        case IDC_SHA3_512:
        case IDC_KECCAK_224:
        case IDC_KECCAK_256:
        case IDC_KECCAK_384:
        case IDC_KECCAK_512:
        case IDC_SHAKE_128:
        case IDC_SHAKE_256:
        case IDC_BLAKE2B:
        case IDC_BLAKE2S:
        case IDC_BLAKE3:
            return L"Algorithms.SHA3_Modern";
        
        // Tab 3: HAVAL & RIPEMD
        case IDC_HAVAL_128:
        case IDC_HAVAL_160:
        case IDC_HAVAL_192:
        case IDC_HAVAL_224:
        case IDC_HAVAL_256:
        case IDC_RIPEMD_128:
        case IDC_RIPEMD_160:
        case IDC_RIPEMD_256:
        case IDC_RIPEMD_320:
            return L"Algorithms.HAVAL_RIPEMD";
        
        // Tab 4: Checksum & Others
        case IDC_CRC8:
        case IDC_CRC16:
        case IDC_CRC32:
        case IDC_CRC32C:
        case IDC_CRC64:
        case IDC_ADLER32:
        case IDC_TIGER:
        case IDC_WHIRLPOOL:
        case IDC_SM3:
        case IDC_LSH_256:
        case IDC_LSH_512:
        case IDC_GOST94:
        case IDC_GOST2012_256:
        case IDC_GOST2012_512:
            return L"Algorithms.Checksum_Others";
        
        default:
            return L"";
    }
}

int ConfigManager::GetAlgorithmIdFromName(const std::wstring& name)
{
    // Reverse map from name to ID
    static std::map<std::wstring, int> idMap = {
        {L"SHA-1", IDC_SHA_160},
        {L"SHA-224", IDC_SHA_224},
        {L"SHA-256", IDC_SHA_256},
        {L"SHA-384", IDC_SHA_384},
        {L"SHA-512", IDC_SHA_512},
        {L"MD2", IDC_MD2},
        {L"MD4", IDC_MD4},
        {L"MD5", IDC_MD5},
        {L"MD6-128", IDC_MD6_128},
        {L"MD6-160", IDC_MD6_160},
        {L"MD6-192", IDC_MD6_192},
        {L"MD6-224", IDC_MD6_224},
        {L"MD6-256", IDC_MD6_256},
        {L"MD6-384", IDC_MD6_384},
        {L"MD6-512", IDC_MD6_512},
        {L"SHA3-224", IDC_SHA3_224},
        {L"SHA3-256", IDC_SHA3_256},
        {L"SHA3-384", IDC_SHA3_384},
        {L"SHA3-512", IDC_SHA3_512},
        {L"HAVAL-128", IDC_HAVAL_128},
        {L"HAVAL-160", IDC_HAVAL_160},
        {L"HAVAL-192", IDC_HAVAL_192},
        {L"HAVAL-224", IDC_HAVAL_224},
        {L"HAVAL-256", IDC_HAVAL_256},
        {L"RIPEMD-128", IDC_RIPEMD_128},
        {L"RIPEMD-160", IDC_RIPEMD_160},
        {L"RIPEMD-256", IDC_RIPEMD_256},
        {L"RIPEMD-320", IDC_RIPEMD_320},
        {L"CRC8", IDC_CRC8},
        {L"CRC16", IDC_CRC16},
        {L"CRC32", IDC_CRC32},
        {L"CRC32C", IDC_CRC32C},
        {L"CRC64", IDC_CRC64},
        {L"ADLER32", IDC_ADLER32},
        {L"KECCAK-224", IDC_KECCAK_224},
        {L"KECCAK-256", IDC_KECCAK_256},
        {L"KECCAK-384", IDC_KECCAK_384},
        {L"KECCAK-512", IDC_KECCAK_512},
        {L"SHAKE-128", IDC_SHAKE_128},
        {L"SHAKE-256", IDC_SHAKE_256},
        {L"TIGER", IDC_TIGER},
        {L"SM3", IDC_SM3},
        {L"WHIRLPOOL", IDC_WHIRLPOOL},
        {L"BLAKE2B", IDC_BLAKE2B},
        {L"BLAKE2S", IDC_BLAKE2S},
        {L"BLAKE3", IDC_BLAKE3},
        {L"LSH-256", IDC_LSH_256},
        {L"LSH-512", IDC_LSH_512},
        {L"GOST-94", IDC_GOST94},
        {L"GOST-256", IDC_GOST2012_256},
        {L"GOST-512", IDC_GOST2012_512}
    };
    
    auto it = idMap.find(name);
    if (it != idMap.end()) {
        return it->second;
    }
    return 0;
}

bool ConfigManager::LoadConfig()
{
    // Check if config file exists
    if (GetFileAttributesW(m_configFilePath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        // Config file doesn't exist, use defaults
        return false;
    }

    // [General] section
    m_isFileMode = ReadIniInt(L"General", L"InputMode", 0) == 1;
    m_stayOnTop = ReadIniInt(L"General", L"StayOnTop", 0) == 1;
    
    // [HAVAL] section
    m_havalPass3 = ReadIniInt(L"HAVAL", L"Pass3", 0) == 1;
    m_havalPass4 = ReadIniInt(L"HAVAL", L"Pass4", 0) == 1;
    m_havalPass5 = ReadIniInt(L"HAVAL", L"Pass5", 0) == 1;
    
    // Load algorithm configurations from multiple sections
    m_algorithms.clear();

    // Use centralized algorithm ID list instead of hardcoding
    for (size_t i = 0; i < core::ALL_ALGORITHM_COUNT; i++) {
        int id = core::ALL_ALGORITHM_IDS[i];
        std::wstring name = GetAlgorithmName(id);
        std::wstring section = GetAlgorithmSection(id);
        if (!name.empty()) {
            bool enabled = ReadIniInt(section, name, 0) == 1;
            m_algorithms[id] = enabled;
        }
    }
    
    return true;
}

bool ConfigManager::SaveConfig()
{
    // [General] section
    WriteIniInt(L"General", L"InputMode", m_isFileMode ? 1 : 0);
    WriteIniInt(L"General", L"StayOnTop", m_stayOnTop ? 1 : 0);
    
    // Add comments (INI standard doesn't support comments in WritePrivateProfileString, 
    // but we can document the format here)
    // InputMode: 0 = Text, 1 = File
    // StayOnTop: 0 = Off, 1 = On
    
    // [HAVAL] section
    WriteIniInt(L"HAVAL", L"Pass3", m_havalPass3 ? 1 : 0);
    WriteIniInt(L"HAVAL", L"Pass4", m_havalPass4 ? 1 : 0);
    WriteIniInt(L"HAVAL", L"Pass5", m_havalPass5 ? 1 : 0);
    // Pass3/4/5: 0 = Not selected, 1 = Selected (multiple can be selected)
    
    // Save algorithms to their respective sections
    for (const auto& pair : m_algorithms) {
        std::wstring name = GetAlgorithmName(pair.first);
        std::wstring section = GetAlgorithmSection(pair.first);
        if (!name.empty() && !section.empty()) {
            WriteIniInt(section, name, pair.second ? 1 : 0);
        }
    }
    
    return true;
}

void ConfigManager::SetAlgorithmEnabled(int algorithmId, bool enabled)
{
    m_algorithms[algorithmId] = enabled;
}

bool ConfigManager::GetAlgorithmEnabled(int algorithmId) const
{
    auto it = m_algorithms.find(algorithmId);
    if (it != m_algorithms.end()) {
        return it->second;
    }
    return false;
}

void ConfigManager::SetAllAlgorithms(const std::map<int, bool>& algorithms)
{
    m_algorithms = algorithms;
}

std::map<int, bool> ConfigManager::GetAllAlgorithms() const
{
    return m_algorithms;
}

void ConfigManager::SetInputMode(bool isFileMode)
{
    m_isFileMode = isFileMode;
}

bool ConfigManager::GetInputMode() const
{
    return m_isFileMode;
}

void ConfigManager::SetStayOnTop(bool stayOnTop)
{
    m_stayOnTop = stayOnTop;
}

bool ConfigManager::GetStayOnTop() const
{
    return m_stayOnTop;
}

void ConfigManager::SetHavalPass3(bool enabled)
{
    m_havalPass3 = enabled;
}

void ConfigManager::SetHavalPass4(bool enabled)
{
    m_havalPass4 = enabled;
}

void ConfigManager::SetHavalPass5(bool enabled)
{
    m_havalPass5 = enabled;
}

bool ConfigManager::GetHavalPass3() const
{
    return m_havalPass3;
}

bool ConfigManager::GetHavalPass4() const
{
    return m_havalPass4;
}

bool ConfigManager::GetHavalPass5() const
{
    return m_havalPass5;
}
