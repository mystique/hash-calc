#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <vector>
#include <map>

class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();

    // Initialize config file path based on executable name
    void Initialize();

    // Load configuration from INI file
    bool LoadConfig();

    // Save configuration to INI file
    bool SaveConfig();

    // Algorithm selection
    void SetAlgorithmEnabled(int algorithmId, bool enabled);
    bool GetAlgorithmEnabled(int algorithmId) const;
    void SetAllAlgorithms(const std::map<int, bool>& algorithms);
    std::map<int, bool> GetAllAlgorithms() const;

    // Input mode (Text/File)
    void SetInputMode(bool isFileMode);
    bool GetInputMode() const;

    // Stay on top
    void SetStayOnTop(bool stayOnTop);
    bool GetStayOnTop() const;

    // HAVAL pass selection (multiple passes can be selected)
    void SetHavalPass3(bool enabled);
    void SetHavalPass4(bool enabled);
    void SetHavalPass5(bool enabled);
    bool GetHavalPass3() const;
    bool GetHavalPass4() const;
    bool GetHavalPass5() const;

private:
    std::wstring m_configFilePath;
    std::map<int, bool> m_algorithms;
    bool m_isFileMode;
    bool m_stayOnTop;
    bool m_havalPass3;
    bool m_havalPass4;
    bool m_havalPass5;

    // Helper functions
    std::wstring GetExecutablePath();
    std::wstring GetConfigFilePath();
    std::wstring ReadIniString(const std::wstring& section, const std::wstring& key, const std::wstring& defaultValue);
    int ReadIniInt(const std::wstring& section, const std::wstring& key, int defaultValue);
    bool WriteIniString(const std::wstring& section, const std::wstring& key, const std::wstring& value);
    bool WriteIniInt(const std::wstring& section, const std::wstring& key, int value);
    
    // Convert algorithm ID to readable name
    std::wstring GetAlgorithmName(int algorithmId);
    int GetAlgorithmIdFromName(const std::wstring& name);
    
    // Get the INI section for an algorithm based on its ID
    std::wstring GetAlgorithmSection(int algorithmId);
};

#endif // CONFIG_MANAGER_H
