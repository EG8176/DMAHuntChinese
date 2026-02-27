#include "ConfigInstance.h"
#include "ConfigUtilities.h"
#include "ESPRenderer.h"
#include "ImGuiMenu.h"
#include "Init.h"
#include "InputManager.h"
#include "Localization/Localization.h"
#include "Pch.h"
#include "PlayerEsp.h"
#include "globals.h"
#include <algorithm>
#include <functional>
#include <unordered_map>

static void DrawFPS() {
  ESPRenderer::DrawText(ImVec2(25, 25),
                        "Overlay FPS: " + std::to_string(FrameRate()),
                        Configs.Overlay.FpsColor, Configs.Overlay.FpsFontSize);
}

static void DrawObjectCount() {
  if (EnvironmentInstance == nullptr) return;
  ESPRenderer::DrawText(
      ImVec2(25, 25 + Configs.Overlay.FpsFontSize),
      "Objects: " + std::to_string(EnvironmentInstance->GetObjectCount()),
      Configs.Overlay.ObjectCountColor, Configs.Overlay.ObjectCountFontSize);
}

static void DrawPlayerList() {
  if (EnvironmentInstance == nullptr) return;
  int y = ESPRenderer::GetScreenHeight() / 2;

  std::vector<std::shared_ptr<WorldEntity>> templist;
  {
    std::lock_guard<std::mutex> lock(EntityMutex);
    templist = EnvironmentInstance->GetRenderPlayerList();
  }

  if (templist.empty())
    return;

  // ── Collect valid player info entries ────────────────────────────────────
  struct PlayerEntry {
    int distance;
    int teamId;   // -1=unassigned, 0=solo, 1..N=team
    bool isDead;
    std::string info;
  };
  std::vector<PlayerEntry> entries;

  for (std::shared_ptr<WorldEntity> ent : templist) {
    if (!ent || ent->Render.Type == EntityType::FriendlyPlayer)
      continue;
    if (ent->Render.Type == EntityType::LocalPlayer || !ent->Render.Valid ||
        ent->Render.Hidden)
      continue;
    if (strstr(ent->GetEntityClassName().name, "Hunter_Loot"))
      continue;

    auto health = ent->Render.Health;
    bool isDead = (ent->Render.Type == EntityType::DeadPlayer);
    if (strstr(ent->GetEntityClassName().name, "HunterBasic")) {
      if (health.current_hp == 0 && IsValidHP(health.current_max_hp))
        isDead = true;
    }
    if (!isDead && !IsValidHP(health.current_max_hp))
      continue;

    int distance = (int)Vector3::Distance(ent->Render.Position,
                                          CameraInstance->GetRenderPosition());

    std::string wname =
        Configs.Player.Name ? LOC("entity", ent->GetTypeAsString()) : "";
    std::string wdistance =
        Configs.Player.Distance ? "[" + std::to_string(distance) + "m]" : "";

    unsigned int displayHp  = health.current_hp;
    unsigned int displayMax = health.current_max_hp;
    unsigned int displayReg = health.regenerable_max_hp;
    if (isDead && !IsValidHP(health.current_max_hp)) {
      displayHp = displayMax = displayReg = 0;
    }
    std::string whealth = std::to_string(displayHp) + "/" +
                          std::to_string(displayMax) + "[" +
                          std::to_string(displayReg) + "]";

    entries.push_back({distance, ent->Render.TeamId, isDead,
                       wname + " " + whealth + " " + wdistance});
  }

  if (entries.empty())
    return;

  // ── Build display string ──────────────────────────────────────────────────
  std::stringstream result;

  if (!Configs.Player.ShowTeam) {
    // ── Flat list sorted by distance (original behaviour) ──────────────────
    std::sort(entries.begin(), entries.end(),
              [](const PlayerEntry& a, const PlayerEntry& b) {
                return a.distance < b.distance;
              });
    result << "[" << entries.size() << "]\n";
    for (size_t i = 0; i < entries.size(); ++i) {
      result << entries[i].info;
      if (i != entries.size() - 1) result << "\n";
    }
  } else {
    // ── Grouped by team ────────────────────────────────────────────────────
    // Sort all entries: alive before dead, then by distance
    std::sort(entries.begin(), entries.end(),
              [](const PlayerEntry& a, const PlayerEntry& b) {
                if (a.isDead != b.isDead) return !a.isDead; // alive first
                return a.distance < b.distance;
              });

    // Gather unique teamIds in order of first (closest) appearance
    std::vector<int> teamOrder;
    std::unordered_map<int, std::vector<const PlayerEntry*>> groups;
    for (const auto& e : entries) {
      int tid = e.teamId;
      if (groups.find(tid) == groups.end())
        teamOrder.push_back(tid);
      groups[tid].push_back(&e);
    }

    // Sort teamOrder: put solo (0) and unassigned (-1) last,
    // teams sorted by their closest (alive) member distance
    auto teamMinDist = [&](int tid) -> int {
      int best = 99999;
      for (auto* e : groups[tid])
        if (!e->isDead && e->distance < best) best = e->distance;
      return best;
    };
    std::sort(teamOrder.begin(), teamOrder.end(),
              [&](int a, int b) {
                bool aSpecial = (a <= 0), bSpecial = (b <= 0);
                if (aSpecial != bSpecial) return !aSpecial; // real teams first
                if (aSpecial && bSpecial) return a > b;     // solo(0) before unassigned(-1)
                return teamMinDist(a) < teamMinDist(b);
              });

    int totalPlayers = (int)entries.size();
    result << "[" << totalPlayers << "]\n";

    bool firstGroup = true;
    for (int tid : teamOrder) {
      if (!firstGroup) result << "\n";
      firstGroup = false;

      // ── Team header ──────────────────────────────────────────────────────
      if (tid > 0)
        result << "-- Team " << tid << " (" << groups[tid].size() << ") --";
      else if (tid == 0)
        result << "-- Solo --";
      else
        result << "-- ? --";  // unassigned (shouldn't normally appear)

      for (auto* e : groups[tid]) {
        result << "\n" << e->info;
      }
    }
  }

  ESPRenderer::DrawText(ImVec2(25, y), result.str(),
                        Configs.Player.PlayerListColor,
                        Configs.Player.PlayerListFontSize, MiddleLeft);
}

static ImVec2 GetCrosshairPosition() {
  if (Configs.General.CrosshairLowerPosition)
    return ImVec2(ESPRenderer::GetScreenWidth() * 0.5f,
                  ESPRenderer::GetScreenHeight() * 0.6f);
  else
    return ImVec2(ESPRenderer::GetScreenWidth() * 0.5f,
                  ESPRenderer::GetScreenHeight() * 0.5f);
}

static void DrawCrosshair() {
  ImVec2 center = GetCrosshairPosition();

  if (Configs.Overlay.CrosshairType == 1) {
    ESPRenderer::DrawCircle(center, Configs.Overlay.CrosshairSize,
                            Configs.Overlay.CrosshairColor, 1, true);
  }
  if (Configs.Overlay.CrosshairType == 2) {
    ESPRenderer::DrawCircle(center, Configs.Overlay.CrosshairSize,
                            Configs.Overlay.CrosshairColor, 1, false);
  }
  if (Configs.Overlay.CrosshairType == 3) {
    ESPRenderer::DrawRect(
        ImVec2(center.x - (Configs.Overlay.CrosshairSize * 0.5f),
               center.y - (Configs.Overlay.CrosshairSize * 0.5f)),
        ImVec2(center.x + (Configs.Overlay.CrosshairSize * 0.5f),
               center.y + (Configs.Overlay.CrosshairSize * 0.5f)),
        Configs.Overlay.CrosshairColor, 1, true);
  }
  if (Configs.Overlay.CrosshairType == 4) {
    ESPRenderer::DrawRect(
        ImVec2(center.x - (Configs.Overlay.CrosshairSize / 2),
               center.y - (Configs.Overlay.CrosshairSize / 2)),
        ImVec2(center.x + (Configs.Overlay.CrosshairSize / 2),
               center.y + (Configs.Overlay.CrosshairSize / 2)),
        Configs.Overlay.CrosshairColor, 1, false);
  }
}

static void DrawAimbotFOV() {
  ImVec2 center = GetCrosshairPosition();

  ESPRenderer::DrawCircle(center, Configs.Aimbot.FOV, Configs.Aimbot.FOVColor,
                          2, false);
}

void DrawRadar() {
  if (EnvironmentInstance == nullptr) return;
  // RYANS RADAR
  if (!Configs.Overlay.DrawRadar)
    return;
  if (!Keyboard::IsKeyDown(Configs.Overlay.RadarKey))
    return;

  std::vector<std::shared_ptr<WorldEntity>> tempPlayerList;
  {
    std::lock_guard<std::mutex> lock(EntityMutex);
    tempPlayerList = EnvironmentInstance->GetRenderPlayerList();
  }
  std::shared_ptr<WorldEntity> LocalPlayer = nullptr;

  for (const auto &ent : tempPlayerList) {
    if (ent && ent->Render.Type == EntityType::LocalPlayer) {
      LocalPlayer = ent;
      break;
    }
  }

  // Screen dimensions
  float screenWidth;
  float screenHeight;

  if (Configs.General.OverrideResolution) {
    screenWidth = static_cast<float>(Configs.General.Width);
    screenHeight = static_cast<float>(Configs.General.Height);
  } else {
    screenWidth = static_cast<float>(ESPRenderer::GetScreenWidth());
    screenHeight = static_cast<float>(ESPRenderer::GetScreenHeight());
  }

  // Radar settings
  float MapSize = Configs.Overlay.RadarScale * screenHeight;

  float mapCenterX = screenWidth / 2.0f;
  float mapCenterY = screenHeight / 2.0f;

  // Offset for map alignment
  float horizontaloffset = screenWidth * Configs.Overlay.RadarX;
  float verticaloffset = screenHeight * Configs.Overlay.RadarY;

  // Hardcoded map bounds
  float worldMinX = 512.0f, worldMaxX = 1535.0f;
  float worldMinY = 512.0f, worldMaxY = 1535.0f;

  // Draw Map Border
  ESPRenderer::DrawRect(ImVec2(mapCenterX - MapSize / 2 + horizontaloffset,
                               mapCenterY - MapSize / 2 + verticaloffset),
                        ImVec2(mapCenterX + MapSize / 2 + horizontaloffset,
                               mapCenterY + MapSize / 2 + verticaloffset),
                        ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f,
                        false // Outline
  );

  if (LocalPlayer) {
    // Draw Local Player
    if (Configs.Overlay.RadarDrawSelf) {
      float playerMapX = ((LocalPlayer->Render.Position.x - worldMinX) /
                          (worldMaxX - worldMinX)) *
                         MapSize;
      float playerMapY = ((worldMaxY - LocalPlayer->Render.Position.y) /
                          (worldMaxY - worldMinY)) *
                         MapSize;

      float playerScreenX =
          mapCenterX - (MapSize / 2) + playerMapX + horizontaloffset;
      float playerScreenY =
          mapCenterY - (MapSize / 2) + playerMapY + verticaloffset;

      ESPRenderer::DrawCircle(ImVec2(playerScreenX, playerScreenY), 6.0f,
                              Configs.Overlay.PlayerRadarColor, 1.0f, true);
    }
  }

  // Draw Enemies
  for (const auto &ent : tempPlayerList) {
    if (!ent || ent->Render.Type == EntityType::LocalPlayer ||
        ent->Render.Type == EntityType::FriendlyPlayer)
      continue;

    if (!ent->Render.Valid || ent->Render.Hidden)
      continue;

    ImVec4 dotColor;
    if (ent->Render.Type == EntityType::DeadPlayer) {
      dotColor = Configs.Overlay.DeadRadarColor;
    } else {
      dotColor = Configs.Overlay.EnemyRadarColor;
    }

    float enemyMapX =
        ((ent->Render.Position.x - worldMinX) / (worldMaxX - worldMinX)) *
        MapSize;
    float enemyMapY =
        ((worldMaxY - ent->Render.Position.y) / (worldMaxY - worldMinY)) *
        MapSize;

    float enemyScreenX =
        mapCenterX - (MapSize / 2) + enemyMapX + horizontaloffset;
    float enemyScreenY =
        mapCenterY - (MapSize / 2) + enemyMapY + verticaloffset;

    // Skip if out of bounds
    if (enemyScreenX < mapCenterX - MapSize / 2 + horizontaloffset ||
        enemyScreenX > mapCenterX + MapSize / 2 + horizontaloffset ||
        enemyScreenY < mapCenterY - MapSize / 2 + verticaloffset ||
        enemyScreenY > mapCenterY + MapSize / 2 + verticaloffset) {
      continue;
    }

    ESPRenderer::DrawCircle(ImVec2(enemyScreenX, enemyScreenY), 5.0f,
                            dotColor, 1.0f, true);
  }
}

void DrawOverlay() {
  if (Configs.Overlay.ShowObjectCount)
    DrawObjectCount();

  if (Configs.Overlay.ShowFPS)
    DrawFPS();

  if (Configs.Player.ShowPlayerList)
    DrawPlayerList();

  if (enableAimBot && Configs.Aimbot.DrawFOV)
    DrawAimbotFOV();

  if (Configs.Overlay.CrosshairType !=
      0) // Lastly draw crosshair to draw it on top of everything
    DrawCrosshair();
}