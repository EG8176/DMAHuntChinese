#include "Pch.h"
#include "globals.h"
#include "ESPRenderer.h"
#include "ConfigInstance.h"
#include "ConfigUtilities.h"

void DrawSpectators()
{
	if (EnvironmentInstance == nullptr) return;

	// อ่านจาก LocalPlayer entity โดยตรง (entity+0x70+0x08) — ง่ายและแม่นกว่า pointer chain เดิม
	int spectatorCount = 0;
	for (auto& ent : EnvironmentInstance->GetRenderPlayerList())
	{
		if (!ent) continue;
		if (ent->Render.Type == EntityType::LocalPlayer)
		{
			spectatorCount = ent->SpectatorCount;
			break;
		}
	}

	if (spectatorCount <= 0 || spectatorCount > 11) return;

	// Get screen dimensions
	float screenWidth = ESPRenderer::GetScreenWidth();
	float screenHeight = ESPRenderer::GetScreenHeight();

    // Draw red frame around the screen
    int lineWidth = 50;
    ESPRenderer::DrawRect(
        ImVec2(0, 0),
        ImVec2(screenWidth, screenHeight),
        ImVec4(1.0f, 0.0f, 0.0f, Configs.General.OverlayMode ? 0.7f : 1.0f),
        static_cast<float>(lineWidth)
    );

    // Draw spectator count
    ESPRenderer::DrawText(
        ImVec2(0, 55),
        std::to_string(spectatorCount),
        ImVec4(0.0f, 1.0f, 0.0f, Configs.General.OverlayMode ? 0.7f : 1.0f),
        40
    );
}