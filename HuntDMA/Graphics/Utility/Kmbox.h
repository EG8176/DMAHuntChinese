#pragma once
#include <string>
#include <vector>
#include <atomic>
#include <memory>

// Forward declare makcu::Device to avoid pulling the full header everywhere
namespace makcu { class Device; struct DeviceInfo; }

namespace kmbox
{
	// Info about an available COM port
	struct PortInfo {
		std::string portName;    // e.g. "COM3"
		std::string description; // e.g. "USB-SERIAL CH340 (COM3)"
	};

	// ── State ────────────────────────────────────────────────────────────
	extern bool connected;
	extern std::string connectedPort;

	// ── Lifecycle ────────────────────────────────────────────────────────
	// Initialize Makcu device. port="" means auto-detect.
	extern void KmboxInitialize(std::string port = "");
	extern void Shutdown();

	// ── Port helpers ─────────────────────────────────────────────────────
	extern std::vector<PortInfo> enumerate_ports();

	// ── Input ────────────────────────────────────────────────────────────
	extern bool IsDown(int key);  // mouse button state from device

	// ── Movement ─────────────────────────────────────────────────────────
	extern void move(int x, int y);
	extern void move_smooth(int x, int y, int segments = 10);
	extern void move_bezier(int x, int y, int segments = 20, int ctrl_x = 0, int ctrl_y = 0);

	// ── Clicks ───────────────────────────────────────────────────────────
	extern void left_click();
	extern void left_click_release();

	// ── Test ─────────────────────────────────────────────────────────────
	extern void test_move();

	// ── Direct access to underlying device (advanced) ────────────────────
	extern makcu::Device* GetDevice();
}