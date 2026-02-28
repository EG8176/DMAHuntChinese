#include "pch.h"
#include "Kmbox.h"
#include "makcu.h"
#include "serialport.h"
#include <iostream>
#include <thread>

namespace kmbox
{
	// ── Global state ─────────────────────────────────────────────────────
	bool connected = false;
	std::string connectedPort = "";

	// Underlying makcu device
	static std::unique_ptr<makcu::Device> g_device;
	
	// Button state (populated from makcu callback)
	static bool key_state[256] = { false };

	// ── Lifecycle ────────────────────────────────────────────────────────
	
	void KmboxInitialize(std::string port)
	{
		// Cleanup previous
		if (g_device) {
			g_device->disconnect();
			g_device.reset();
		}
		connected = false;
		connectedPort = "";
		memset(key_state, 0, sizeof(key_state));

		g_device = std::make_unique<makcu::Device>();

		// Set up button callback to track mouse button states
		g_device->setMouseButtonCallback([](makcu::MouseButton button, bool pressed) {
			// Map makcu button enum to VK codes matching old system
			int vkMap[] = { 1, 2, 4, 5, 6 }; // LEFT->VK_LBUTTON, RIGHT->VK_RBUTTON, MIDDLE->VK_MBUTTON, X1->VK_XBUTTON1, X2->VK_XBUTTON2
			int idx = static_cast<int>(button);
			if (idx >= 0 && idx < 5) {
				key_state[vkMap[idx]] = pressed;
			}
		});

		// Enable high-performance mode for gaming
		g_device->enableHighPerformanceMode(true);

		// Connect
		LOG_INFO("[Makcu] Connecting via makcu-cpp library...");
		
		bool success = g_device->connect(port);
		
		if (success) {
			connected = true;
			auto info = g_device->getDeviceInfo();
			connectedPort = info.port;
			
			// Enable button monitoring
			g_device->enableButtonMonitoring(true);
			
			LOG_INFO("[Makcu] Connected on %s (makcu-cpp library)", connectedPort.c_str());
		}
		else {
			LOG_ERROR("[Makcu] Connection failed! Make sure your Makcu is plugged in.");
			g_device.reset();
		}
	}

	void Shutdown()
	{
		if (g_device) {
			g_device->disconnect();
			g_device.reset();
		}
		connected = false;
		connectedPort = "";
		memset(key_state, 0, sizeof(key_state));
		LOG_INFO("[Makcu] Shutdown complete.");
	}

	// ── Port helpers ─────────────────────────────────────────────────────

	std::vector<PortInfo> enumerate_ports()
	{
		std::vector<PortInfo> result;
		auto devices = makcu::Device::findDevices();
		for (const auto& dev : devices)
		{
			PortInfo info;
			info.portName = dev.port;
			info.description = dev.description;
			result.push_back(info);
		}
		
		// If no Makcu-specific ports found, fall back to listing all serial ports
		if (result.empty()) {
			auto allPorts = makcu::SerialPort::getAvailablePorts();
			for (const auto& p : allPorts) {
				PortInfo info;
				info.portName = p;
				info.description = "Serial Port";
				result.push_back(info);
			}
		}
		
		return result;
	}

	// ── Input ────────────────────────────────────────────────────────────

	bool IsDown(int key)
	{
		if (key < 0 || key >= 256) return false;
		return key_state[key];
	}

	// ── Movement ─────────────────────────────────────────────────────────

	void move(int x, int y)
	{
		if (!connected || !g_device) return;
		g_device->mouseMove(x, y);
	}

	void move_smooth(int x, int y, int segments)
	{
		if (!connected || !g_device) return;
		g_device->mouseMoveSmooth(x, y, segments);
	}

	void move_bezier(int x, int y, int segments, int ctrl_x, int ctrl_y)
	{
		if (!connected || !g_device) return;
		g_device->mouseMoveBezier(x, y, segments, ctrl_x, ctrl_y);
	}

	// ── Clicks ───────────────────────────────────────────────────────────

	void left_click()
	{
		if (!connected || !g_device) return;
		g_device->mouseDown(makcu::MouseButton::LEFT);
	}

	void left_click_release()
	{
		if (!connected || !g_device) return;
		g_device->mouseUp(makcu::MouseButton::LEFT);
	}

	// ── Test ─────────────────────────────────────────────────────────────

	void test_move()
	{
		if (!connected || !g_device) return;

		// Run asynchronously so Sleep calls don't block the UI/render thread
		std::thread([] {
			move(50, 0);   // Right
			Sleep(100);
			move(0, 50);   // Down
			Sleep(100);
			move(-50, 0);  // Left
			Sleep(100);
			move(0, -50);  // Up
		}).detach();
	}

	// ── Advanced ─────────────────────────────────────────────────────────

	makcu::Device* GetDevice()
	{
		return g_device.get();
	}
}
