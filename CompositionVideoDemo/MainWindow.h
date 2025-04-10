#pragma once

#include <shlobj.h>

#include <windows.ui.composition.interop.h>

#include <winrt\windows.ui.composition.h>
#include <winrt\windows.ui.composition.desktop.h>

template <typename T>
struct DesktopWindow
{
	using base_type = DesktopWindow<T>;
	HWND m_window = nullptr;

	static T* GetThisFromHandle(HWND const window) noexcept
	{
		return reinterpret_cast<T*>(GetWindowLongPtr(window, GWLP_USERDATA));
	}

	static LRESULT __stdcall WndProc(HWND const window, UINT const message, WPARAM const wparam, LPARAM const lparam) noexcept
	{
		WINRT_ASSERT(window);

		if(WM_NCCREATE == message)
		{
			auto cs = reinterpret_cast<CREATESTRUCT*>(lparam);
			T* that = static_cast<T*>(cs->lpCreateParams);
			WINRT_ASSERT(that);
			WINRT_ASSERT(!that->m_window);
			that->m_window = window;
			SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(that));
		} else if(T* that = GetThisFromHandle(window))
		{
			return that->MessageHandler(message, wparam, lparam);
		}

		return DefWindowProc(window, message, wparam, lparam);
	}

	LRESULT MessageHandler(UINT const message, WPARAM const wparam, LPARAM const lparam) noexcept
	{
		switch(message)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_DPICHANGED:
			{
				auto rect = reinterpret_cast<const RECT*>(lparam);
				SetWindowPos(m_window,
							 nullptr,
							 rect->left,
							 rect->top,
							 rect->right - rect->left,
							 rect->bottom - rect->top,
							 SWP_NOZORDER | SWP_NOACTIVATE);
			}
			return 0;
		default:
			return DefWindowProcW(m_window, message, wparam, lparam);
		}
	}

	inline auto CreateDesktopWindowTarget(winrt::Windows::UI::Composition::Compositor const& compositor, HWND window, bool isTopMost)
	{
		namespace abi = ABI::Windows::UI::Composition::Desktop;

		auto interop = compositor.as<abi::ICompositorDesktopInterop>();
		winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget target { nullptr };
		winrt::check_hresult(interop->CreateDesktopWindowTarget(window, isTopMost, reinterpret_cast<abi::IDesktopWindowTarget**>(winrt::put_abi(target))));
		return target;
	}

	winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget CreateWindowTarget(winrt::Windows::UI::Composition::Compositor const& compositor)
	{
		return CreateDesktopWindowTarget(compositor, m_window, true);
	}

	void InitializeObjectWithWindowHandle(winrt::Windows::Foundation::IUnknown const& object)
	{
		auto initializer = object.as<IInitializeWithWindow>();
		winrt::check_hresult(initializer->Initialize(m_window));
	}
};

struct MainWindow : DesktopWindow<MainWindow>
{
	static const std::wstring ClassName;
	MainWindow(std::wstring const& titleString, int width, int height);
	LRESULT MessageHandler(UINT const message, WPARAM const wparam, LPARAM const lparam);

private:
	static void RegisterWindowClass();
};