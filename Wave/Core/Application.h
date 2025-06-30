#pragma once

#include <Windows.h>

namespace Wave 
{
	class IApplication
	{
	public:
		// This function can be used to initialize application state and will run after essential
		// hardware resources are allocated. Some state that does not depend on these resources
		// should still be initialized in the constructor such as pointers and flags.
		virtual void Startup() = 0;
		virtual void Cleanup() = 0;

		// 'ESC' = exit
		virtual bool IsDone();

		// The update method will be invoked once per frame. Both state updating and scene
		// rendering should be handled by this method.
		virtual void Update(float deltaT) = 0;

		// Official rendering pass
		virtual void RenderScene(void) = 0;

		// Optional UI (overlay) rendering pass. This is LDR. The buffer is already cleared.
		virtual void RenderUI(class GraphicsContext&) {};

		// Override this in applications that use DirectX Raytracing to require a DXR-capable device.
		virtual bool RequiresRaytracingSupport() const { return false; }
	};

}

namespace Wave
{
	int RunApplication(IApplication& app, const wchar_t* className, HINSTANCE hInst, int nCmdShow);
}

#define CREATE_APPLICATION(app_class) \
    int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int nCmdShow) \
    { \
		app_class app; \
        return Wave::RunApplication(app, L###app_class, hInstance, nCmdShow); \
    }
