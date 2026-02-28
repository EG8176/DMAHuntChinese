#pragma once
#include "pch.h"
#include "WorldEntity.h"
#include "LocalizationData.h"
#include "en.h"
#include "ru.h"
#include "zh.h"

class Localization {
private:
    static inline std::unordered_map<std::string, std::unordered_map<std::string, std::string>> translations;
    static inline std::string localizationPath = "lang/";

    static void LoadLanguageFile(const std::string& language);
    static LocalizationData GetDefaultLocalization();

public:
    static inline std::map<std::string, LocalizationData> Languages = {
        {"en", Localizations::GetLocalization_EN()},
        {"ru", Localizations::GetLocalization_RU()},
        {"zh", Localizations::GetLocalization_ZH()}
    };

    static void Initialize();
    static std::string GetLocalizedString(const std::string& category, const std::string& key);
    static void CreateDefaultLocalizationFiles();

#define LOC(category, key) Localization::GetLocalizedString(category, key)
};