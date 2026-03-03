#pragma once
#include "pch.h"
class GruntConfig
{
    std::string ConfigName;

public:
    GruntConfig(const std::string& name)
    {
        ConfigName = name;
    }

    // -- Master toggle --
    bool Enable          = true;   // master on/off for all AI enemies

    // -- Toggle per AI type --
    bool EnableGrunts    = true;   // class: grunt (ทุก variant)
    bool EnableArmored   = true;   // grunt_base + Armored
    bool EnableMeathead  = true;   // grunt_base + Humanoid
    bool EnableHive      = true;   // class: hive_template
    bool EnableWaterDevil= true;   // class: waterdevil_template
    bool EnableImmolator  = true;   // class: immolator_template (AI)
    bool EnableHellhound = true;   // class: TBD
    bool EnableLeech     = true;   // class: TBD
    bool EnableSpecial   = true;   // grunt_base + Random/unknown

    bool ShowName     = true;
    bool ShowDistance = true;
    bool ShowHealth   = false;
    int  MaxDistance  = 50;        // เมตร
    int  FontSize     = 12;

    // สีแยกตาม type
    ImVec4 ColorGrunt     = ImVec4(0.85f, 0.85f, 0.85f, 1.0f); // เทา
    ImVec4 ColorArmored   = ImVec4(1.0f,  0.45f, 0.1f,  1.0f); // ส้ม-แดง
    ImVec4 ColorMeathead  = ImVec4(0.9f,  0.2f,  0.2f,  1.0f); // แดงเข้ม
    ImVec4 ColorHive      = ImVec4(0.6f,  1.0f,  0.2f,  1.0f); // เขียว-เหลือง
    ImVec4 ColorWaterDevil= ImVec4(0.2f,  0.7f,  1.0f,  1.0f); // ฟ้า
    ImVec4 ColorImmolator  = ImVec4(1.0f,  0.3f,  0.0f,  1.0f); // แดง-ส้มไฟ
    ImVec4 ColorHellhound = ImVec4(1.0f,  0.6f,  0.0f,  1.0f); // เหลือง-ส้ม
    ImVec4 ColorLeech     = ImVec4(0.5f,  0.0f,  0.5f,  1.0f); // ม่วงเข้ม
    ImVec4 ColorSpecial   = ImVec4(0.8f,  0.3f,  1.0f,  1.0f); // ม่วง

    // -- Animal toggles --
    bool EnableAnimals   = true;   // master toggle for all animals
    bool EnableCrow      = true;
    bool EnableDyingCow  = true;
    bool EnableDyingHorse= true;
    bool EnableDuck      = true;
    bool EnableBat       = true;

    // Animal colors (subtle)
    ImVec4 ColorAnimal    = ImVec4(0.7f, 0.55f, 0.35f, 1.0f); // น้ำตาล-เบจ (default)
    ImVec4 ColorCrow      = ImVec4(0.5f, 0.5f,  0.55f, 1.0f); // เทา-ม่วง
    ImVec4 ColorDyingCow  = ImVec4(0.75f,0.6f,  0.4f,  1.0f); // น้ำตาล
    ImVec4 ColorDyingHorse= ImVec4(0.8f, 0.65f, 0.45f, 1.0f); // น้ำตาล-ส้ม
    ImVec4 ColorDuck      = ImVec4(0.4f, 0.7f,  0.5f,  1.0f); // เขียว-เทา
    ImVec4 ColorBat       = ImVec4(0.45f,0.35f, 0.5f,  1.0f); // เทา-ม่วง

    void ToJsonColor(json* j, const std::string& name, ImVec4* color)
    {
        (*j)[ConfigName][name]["r"] = color->x;
        (*j)[ConfigName][name]["g"] = color->y;
        (*j)[ConfigName][name]["b"] = color->z;
        (*j)[ConfigName][name]["a"] = color->w;
    }

    void FromJsonColor(json j, const std::string& name, ImVec4* color)
    {
        if (j[ConfigName].contains(name))
        {
            color->x = j[ConfigName][name]["r"];
            color->y = j[ConfigName][name]["g"];
            color->z = j[ConfigName][name]["b"];
            color->w = j[ConfigName][name]["a"];
        }
    }

    json ToJson()
    {
        json j;
        j[ConfigName]["Enable"]           = Enable;
        j[ConfigName]["EnableGrunts"]     = EnableGrunts;
        j[ConfigName]["EnableArmored"]    = EnableArmored;
        j[ConfigName]["EnableMeathead"]   = EnableMeathead;
        j[ConfigName]["EnableHive"]       = EnableHive;
        j[ConfigName]["EnableWaterDevil"] = EnableWaterDevil;
        j[ConfigName]["EnableImmolator"]   = EnableImmolator;
        j[ConfigName]["EnableHellhound"]  = EnableHellhound;
        j[ConfigName]["EnableLeech"]      = EnableLeech;
        j[ConfigName]["EnableSpecial"]    = EnableSpecial;
        j[ConfigName]["ShowName"]         = ShowName;
        j[ConfigName]["ShowDistance"]     = ShowDistance;
        j[ConfigName]["ShowHealth"]       = ShowHealth;
        j[ConfigName]["MaxDistance"]      = MaxDistance;
        j[ConfigName]["FontSize"]         = FontSize;
        ToJsonColor(&j, "ColorGrunt",      &ColorGrunt);
        ToJsonColor(&j, "ColorArmored",    &ColorArmored);
        ToJsonColor(&j, "ColorMeathead",   &ColorMeathead);
        ToJsonColor(&j, "ColorHive",       &ColorHive);
        ToJsonColor(&j, "ColorWaterDevil", &ColorWaterDevil);
        ToJsonColor(&j, "ColorImmolator",  &ColorImmolator);
        ToJsonColor(&j, "ColorHellhound",  &ColorHellhound);
        ToJsonColor(&j, "ColorLeech",      &ColorLeech);
        ToJsonColor(&j, "ColorSpecial",    &ColorSpecial);
        j[ConfigName]["EnableAnimals"]    = EnableAnimals;
        j[ConfigName]["EnableCrow"]       = EnableCrow;
        j[ConfigName]["EnableDyingCow"]   = EnableDyingCow;
        j[ConfigName]["EnableDyingHorse"] = EnableDyingHorse;
        j[ConfigName]["EnableDuck"]       = EnableDuck;
        j[ConfigName]["EnableBat"]        = EnableBat;
        ToJsonColor(&j, "ColorCrow",      &ColorCrow);
        ToJsonColor(&j, "ColorDyingCow",  &ColorDyingCow);
        ToJsonColor(&j, "ColorDyingHorse",&ColorDyingHorse);
        ToJsonColor(&j, "ColorDuck",      &ColorDuck);
        ToJsonColor(&j, "ColorBat",       &ColorBat);
        return j;
    }

    void FromJson(const json& j)
    {
        if (!j.contains(ConfigName))
            return;
        if (j[ConfigName].contains("Enable"))           Enable           = j[ConfigName]["Enable"];
        if (j[ConfigName].contains("EnableGrunts"))     EnableGrunts     = j[ConfigName]["EnableGrunts"];
        if (j[ConfigName].contains("EnableArmored"))    EnableArmored    = j[ConfigName]["EnableArmored"];
        if (j[ConfigName].contains("EnableMeathead"))   EnableMeathead   = j[ConfigName]["EnableMeathead"];
        if (j[ConfigName].contains("EnableHive"))       EnableHive       = j[ConfigName]["EnableHive"];
        if (j[ConfigName].contains("EnableWaterDevil")) EnableWaterDevil = j[ConfigName]["EnableWaterDevil"];
        if (j[ConfigName].contains("EnableImmolator"))  EnableImmolator  = j[ConfigName]["EnableImmolator"];
        if (j[ConfigName].contains("EnableHellhound"))  EnableHellhound  = j[ConfigName]["EnableHellhound"];
        if (j[ConfigName].contains("EnableLeech"))      EnableLeech      = j[ConfigName]["EnableLeech"];
        if (j[ConfigName].contains("EnableSpecial"))    EnableSpecial    = j[ConfigName]["EnableSpecial"];
        if (j[ConfigName].contains("ShowName"))         ShowName         = j[ConfigName]["ShowName"];
        if (j[ConfigName].contains("ShowDistance"))     ShowDistance     = j[ConfigName]["ShowDistance"];
        if (j[ConfigName].contains("ShowHealth"))       ShowHealth       = j[ConfigName]["ShowHealth"];
        if (j[ConfigName].contains("MaxDistance"))      MaxDistance      = j[ConfigName]["MaxDistance"];
        if (j[ConfigName].contains("FontSize"))         FontSize         = j[ConfigName]["FontSize"];
        FromJsonColor(j, "ColorGrunt",      &ColorGrunt);
        FromJsonColor(j, "ColorArmored",    &ColorArmored);
        FromJsonColor(j, "ColorMeathead",   &ColorMeathead);
        FromJsonColor(j, "ColorHive",       &ColorHive);
        FromJsonColor(j, "ColorWaterDevil", &ColorWaterDevil);
        FromJsonColor(j, "ColorImmolator",  &ColorImmolator);
        FromJsonColor(j, "ColorHellhound",  &ColorHellhound);
        FromJsonColor(j, "ColorLeech",      &ColorLeech);
        FromJsonColor(j, "ColorSpecial",    &ColorSpecial);
        if (j[ConfigName].contains("EnableAnimals"))    EnableAnimals    = j[ConfigName]["EnableAnimals"];
        if (j[ConfigName].contains("EnableCrow"))       EnableCrow       = j[ConfigName]["EnableCrow"];
        if (j[ConfigName].contains("EnableDyingCow"))   EnableDyingCow   = j[ConfigName]["EnableDyingCow"];
        if (j[ConfigName].contains("EnableDyingHorse")) EnableDyingHorse = j[ConfigName]["EnableDyingHorse"];
        if (j[ConfigName].contains("EnableDuck"))       EnableDuck       = j[ConfigName]["EnableDuck"];
        if (j[ConfigName].contains("EnableBat"))        EnableBat        = j[ConfigName]["EnableBat"];
        FromJsonColor(j, "ColorCrow",       &ColorCrow);
        FromJsonColor(j, "ColorDyingCow",   &ColorDyingCow);
        FromJsonColor(j, "ColorDyingHorse", &ColorDyingHorse);
        FromJsonColor(j, "ColorDuck",       &ColorDuck);
        FromJsonColor(j, "ColorBat",        &ColorBat);
    }
};
