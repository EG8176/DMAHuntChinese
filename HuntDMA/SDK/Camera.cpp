#include "Pch.h"
#include "Camera.h"
#include "Globals.h"
#include "ConfigUtilities.h"
#include "ESPRenderer.h"

void Camera::UpdateCamera(VMMDLL_SCATTER_HANDLE handle)
{
    uint64_t CameraBase = TargetProcess.Read<uint64_t>(EnvironmentInstance->GetpSystem() + CameraBaseOffset);
    TargetProcess.AddScatterReadRequest(handle, CameraBase + CameraPosOffset, &Position, sizeof(Vector3));
    TargetProcess.AddScatterReadRequest(handle, CameraBase + ViewMatrixOffset, &RenderMatrix, sizeof(ViewMatrix));
    TargetProcess.AddScatterReadRequest(handle, CameraBase + ProjectionMatrixOffset, &ProjectionMatrix, sizeof(ViewMatrix));
}

Vector2 Camera::WorldToScreen(Vector3 pos, bool clamp)
{
    if (Position == Vector3::Zero())
        return Vector2::Zero();

    // Precompute matrix operations
    Vector4 transformed = {
        pos.x * RenderMatrix.matrix[0][0] + pos.y * RenderMatrix.matrix[1][0] + pos.z * RenderMatrix.matrix[2][0] + RenderMatrix.matrix[3][0],
        pos.x * RenderMatrix.matrix[0][1] + pos.y * RenderMatrix.matrix[1][1] + pos.z * RenderMatrix.matrix[2][1] + RenderMatrix.matrix[3][1],
        pos.x * RenderMatrix.matrix[0][2] + pos.y * RenderMatrix.matrix[1][2] + pos.z * RenderMatrix.matrix[2][2] + RenderMatrix.matrix[3][2],
        pos.x * RenderMatrix.matrix[0][3] + pos.y * RenderMatrix.matrix[1][3] + pos.z * RenderMatrix.matrix[2][3] + RenderMatrix.matrix[3][3]
    };

    // Check visibility
    if (transformed.z >= 0.0f)
        return Vector2::Zero();

    // Projection
    Vector4 projected = {
        transformed.x * ProjectionMatrix.matrix[0][0] + transformed.y * ProjectionMatrix.matrix[1][0] + transformed.z * ProjectionMatrix.matrix[2][0] + transformed.w * ProjectionMatrix.matrix[3][0],
        transformed.x * ProjectionMatrix.matrix[0][1] + transformed.y * ProjectionMatrix.matrix[1][1] + transformed.z * ProjectionMatrix.matrix[2][1] + transformed.w * ProjectionMatrix.matrix[3][1],
        transformed.x * ProjectionMatrix.matrix[0][2] + transformed.y * ProjectionMatrix.matrix[1][2] + transformed.z * ProjectionMatrix.matrix[2][2] + transformed.w * ProjectionMatrix.matrix[3][2],
        transformed.x * ProjectionMatrix.matrix[0][3] + transformed.y * ProjectionMatrix.matrix[1][3] + transformed.z * ProjectionMatrix.matrix[2][3] + transformed.w * ProjectionMatrix.matrix[3][3]
    };

    if (projected.w == 0.0f)
        return Vector2::Zero();

    // Divide by w
    projected.x /= projected.w;
    projected.y /= projected.w;

    if (abs(projected.x) > (clamp ? 1.0f : 1.5f) ||
        abs(projected.y) > (clamp ? 1.0f : 1.5f))
        return Vector2::Zero();

    // Get screen coordinates
    int width = ESPRenderer::GetScreenWidth();
    int height = ESPRenderer::GetScreenHeight();

    return Vector2{
        (1.0f + projected.x) * width * 0.5f,
        (1.0f - projected.y) * height * 0.5f
    };
}

// ── Double-buffer: copy live DMA data → render-safe copies ───────────────
void Camera::CommitRenderData()
{
    R_Position         = Position;
    R_RenderMatrix     = RenderMatrix;
    R_ProjectionMatrix = ProjectionMatrix;
}

// Identical to WorldToScreen but reads from render-safe copies
Vector2 Camera::RenderWorldToScreen(Vector3 pos, bool clamp)
{
    if (R_Position == Vector3::Zero())
        return Vector2::Zero();

    Vector4 transformed = {
        pos.x * R_RenderMatrix.matrix[0][0] + pos.y * R_RenderMatrix.matrix[1][0] + pos.z * R_RenderMatrix.matrix[2][0] + R_RenderMatrix.matrix[3][0],
        pos.x * R_RenderMatrix.matrix[0][1] + pos.y * R_RenderMatrix.matrix[1][1] + pos.z * R_RenderMatrix.matrix[2][1] + R_RenderMatrix.matrix[3][1],
        pos.x * R_RenderMatrix.matrix[0][2] + pos.y * R_RenderMatrix.matrix[1][2] + pos.z * R_RenderMatrix.matrix[2][2] + R_RenderMatrix.matrix[3][2],
        pos.x * R_RenderMatrix.matrix[0][3] + pos.y * R_RenderMatrix.matrix[1][3] + pos.z * R_RenderMatrix.matrix[2][3] + R_RenderMatrix.matrix[3][3]
    };

    if (transformed.z >= 0.0f)
        return Vector2::Zero();

    Vector4 projected = {
        transformed.x * R_ProjectionMatrix.matrix[0][0] + transformed.y * R_ProjectionMatrix.matrix[1][0] + transformed.z * R_ProjectionMatrix.matrix[2][0] + transformed.w * R_ProjectionMatrix.matrix[3][0],
        transformed.x * R_ProjectionMatrix.matrix[0][1] + transformed.y * R_ProjectionMatrix.matrix[1][1] + transformed.z * R_ProjectionMatrix.matrix[2][1] + transformed.w * R_ProjectionMatrix.matrix[3][1],
        transformed.x * R_ProjectionMatrix.matrix[0][2] + transformed.y * R_ProjectionMatrix.matrix[1][2] + transformed.z * R_ProjectionMatrix.matrix[2][2] + transformed.w * R_ProjectionMatrix.matrix[3][2],
        transformed.x * R_ProjectionMatrix.matrix[0][3] + transformed.y * R_ProjectionMatrix.matrix[1][3] + transformed.z * R_ProjectionMatrix.matrix[2][3] + transformed.w * R_ProjectionMatrix.matrix[3][3]
    };

    if (projected.w == 0.0f)
        return Vector2::Zero();

    projected.x /= projected.w;
    projected.y /= projected.w;

    if (abs(projected.x) > (clamp ? 1.0f : 1.5f) ||
        abs(projected.y) > (clamp ? 1.0f : 1.5f))
        return Vector2::Zero();

    int width = ESPRenderer::GetScreenWidth();
    int height = ESPRenderer::GetScreenHeight();

    return Vector2{
        (1.0f + projected.x) * width * 0.5f,
        (1.0f - projected.y) * height * 0.5f
    };
}
