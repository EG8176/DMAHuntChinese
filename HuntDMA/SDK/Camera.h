#pragma once
class Camera
{
private:
	uint64_t CameraBaseOffset = 0x840;
	uint64_t ViewMatrixOffset = 0x230;
	uint64_t CameraPosOffset = 0x2F0;
	uint64_t ProjectionMatrixOffset = 0x270;

	Vector3 Position;
	ViewMatrix RenderMatrix;
	ViewMatrix ProjectionMatrix;

	// ── Render-safe copies (double buffer) ───────────────────────────────
	Vector3    R_Position;
	ViewMatrix R_RenderMatrix;
	ViewMatrix R_ProjectionMatrix;

public:
	void UpdateCamera(VMMDLL_SCATTER_HANDLE handle);
	Vector3 GetPosition() { return Position;}
	Vector2 WorldToScreen(Vector3 pos, bool clamp = true);

	// ── Render-thread safe methods ───────────────────────────────────────
	void CommitRenderData();                                  // copy live → render copies
	Vector3 GetRenderPosition() { return R_Position; }
	Vector2 RenderWorldToScreen(Vector3 pos, bool clamp = true);

};