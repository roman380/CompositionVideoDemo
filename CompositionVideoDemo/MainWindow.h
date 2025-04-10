#pragma once

#include <shlobj.h>

#include <Windows.UI.Composition.Interop.h>

#include <winrt\Windows.UI.Composition.h>
#include <winrt\Windows.UI.Composition.Desktop.h>

struct MainWindow
{
	MainWindow(std::wstring const& Text, std::pair<LONG, LONG> Size)
	{
		static wchar_t const* const g_ClassName = L"CompositionVideoDemo.MainWindow";
		std::once_flag g_RegisterClassOnce;
		std::call_once(g_RegisterClassOnce, []() {
			WNDCLASSEXW WindowClass { sizeof WindowClass };
			WindowClass.lpfnWndProc = WndProc;
			WindowClass.hInstance = nullptr;
			WindowClass.hIcon = LoadIconW(WindowClass.hInstance, IDI_APPLICATION);
			WindowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
			WindowClass.lpszClassName = g_ClassName;
			WindowClass.hIconSm = LoadIconW(WindowClass.hInstance, IDI_APPLICATION);
			winrt::check_bool(RegisterClassExW(&WindowClass));
		});
		RECT Position { 0, 0, Size.first, Size.second };
		DWORD Style = WS_OVERLAPPEDWINDOW;
		DWORD ExStyle = WS_EX_NOREDIRECTIONBITMAP;
		winrt::check_bool(AdjustWindowRectEx(&Position, Style, false, ExStyle));
		winrt::check_bool(CreateWindowExW(ExStyle, g_ClassName, Text.c_str(), Style, CW_USEDEFAULT, CW_USEDEFAULT, Position.right - Position.left, Position.bottom - Position.top, nullptr, nullptr, nullptr, this));
		WI_ASSERT(m_Handle);
		ShowWindow(m_Handle, SW_SHOW);
		UpdateWindow(m_Handle);
	}

	auto CreateDesktopWindowTarget(winrt::Windows::UI::Composition::Compositor const& compositor, HWND window, bool isTopMost)
	{
		namespace abi = ABI::Windows::UI::Composition::Desktop;

		auto interop = compositor.as<abi::ICompositorDesktopInterop>();
		winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget target { nullptr };
		winrt::check_hresult(interop->CreateDesktopWindowTarget(window, isTopMost, reinterpret_cast<abi::IDesktopWindowTarget**>(winrt::put_abi(target))));
		return target;
	}

	winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget CreateWindowTarget(winrt::Windows::UI::Composition::Compositor const& Compositor)
	{
		return CreateDesktopWindowTarget(Compositor, m_Handle, true);
	}

private:
	static MainWindow* FromHandle(HWND const Handle)
	{
		return reinterpret_cast<MainWindow*>(GetWindowLongPtrW(Handle, GWLP_USERDATA));
	}
	LRESULT HandleMessage(UINT Message, WPARAM wParam, LPARAM lParam)
	{
		switch(Message)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_DPICHANGED:
			{
				auto const& Position = *reinterpret_cast<RECT const*>(lParam);
				SetWindowPos(m_Handle, nullptr, Position.left, Position.top, Position.right - Position.left, Position.bottom - Position.top, SWP_NOZORDER | SWP_NOACTIVATE);
			}
			return 0;
		default:
			return DefWindowProcW(m_Handle, Message, wParam, lParam);
		}
	}
	static LRESULT CALLBACK WndProc(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam)
	{
		WI_ASSERT(Window);
		if(Message == WM_NCCREATE)
		{
			auto const CreateStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
			auto const That = static_cast<MainWindow*>(CreateStruct->lpCreateParams);
			WI_ASSERT(That);
			WI_ASSERT(!That->m_Handle);
			That->m_Handle = Window;
			SetWindowLongPtrW(Window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(That));
			return DefWindowProcW(Window, Message, wParam, lParam);
		}
		auto const That = FromHandle(Window);
		if(That)
			return That->HandleMessage(Message, wParam, lParam);
		return DefWindowProcW(Window, Message, wParam, lParam);
	}

	HWND m_Handle = nullptr;
};
