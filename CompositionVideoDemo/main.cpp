#include "pch.h"

#include "MainWindow.h"

struct DispatcherQueueController
{
	DispatcherQueueController()
	{
		DispatcherQueueOptions ControllerOptions { sizeof ControllerOptions, DQTYPE_THREAD_CURRENT, DQTAT_COM_NONE };
		THROW_IF_FAILED(CreateDispatcherQueueController(ControllerOptions, reinterpret_cast<PDISPATCHERQUEUECONTROLLER*>(winrt::put_abi(Controller))));
		WI_ASSERT(Controller);
	}
	~DispatcherQueueController()
	{
		WI_ASSERT(!Controller);
	}

	winrt::fire_and_forget Shutdown()
	{
		WI_ASSERT(Controller);
		auto Queue = Controller.DispatcherQueue();
		co_await Controller.ShutdownQueueAsync();
		co_await Queue;
		Controller = winrt::Windows::System::DispatcherQueueController { nullptr };
		co_return;
	}

	winrt::Windows::System::DispatcherQueueController Controller { nullptr };
};

int WINAPI wWinMain([[maybe_unused]] HINSTANCE Instance, [[maybe_unused]] HINSTANCE PreviousInstance, [[maybe_unused]] LPWSTR CommandLine, [[maybe_unused]] int ShowCommand)
{
	try
	{
		int argc = 0;
		auto argv = winrt::check_pointer(CommandLineToArgvW(GetCommandLineW(), &argc));
		std::vector<std::wstring> args(argv + 1, argv + argc);
		THROW_HR_IF_MSG(E_INVALIDARG, args.empty(), "Expected command line argument was not found");

		winrt::init_apartment(winrt::apartment_type::single_threaded);

		auto Path = args[0];
		winrt::Windows::Storage::Streams::IRandomAccessStream Stream { nullptr };
		THROW_IF_FAILED(CreateRandomAccessStreamOnFile(Path.c_str(), static_cast<DWORD>(winrt::Windows::Storage::FileAccessMode::Read), winrt::guid_of<decltype(Stream)>(), winrt::put_abi(Stream)));

		DispatcherQueueController DispatcherQueueController;

		MainWindow Window(L"CompositionVideoDemo", std::make_pair(800, 600));
		auto Compositor = winrt::Windows::UI::Composition::Compositor();
		auto WindowTarget = Window.CreateWindowTarget(Compositor);
		auto RootVisual = Compositor.CreateSpriteVisual();
		RootVisual.RelativeSizeAdjustment({ 1.0f, 1.0f });
		RootVisual.Brush(Compositor.CreateColorBrush(winrt::Windows::UI::Colors::Black()));
		WindowTarget.Root(RootVisual);

		auto MediaPlayer = winrt::Windows::Media::Playback::MediaPlayer();
		auto MediaPlayerSurface = MediaPlayer.GetSurface(Compositor);
		auto ContentVisual = Compositor.CreateSpriteVisual();
		ContentVisual.RelativeSizeAdjustment({ 1.0f, 1.0f });
		ContentVisual.Brush(Compositor.CreateSurfaceBrush(MediaPlayerSurface.CompositionSurface()));
		RootVisual.Children().InsertAtTop(ContentVisual);

		auto MediaSource = winrt::Windows::Media::Core::MediaSource::CreateFromStream(Stream, L"");
		auto MediaPlaybackItem = winrt::Windows::Media::Playback::MediaPlaybackItem(MediaSource);
		MediaPlayer.Source(MediaPlaybackItem);
		MediaPlayer.Play();

		for(;;)
		{
			MSG Message;
			if(!GetMessageW(&Message, WM_NULL, 0u, 0u))
				break;
			TranslateMessage(&Message);
			DispatchMessageW(&Message);
		}
		DispatcherQueueController.Shutdown();

		// WARN: We cannot co_await/get the call above here in the STA, so just ignore the disgraceful termination (we would need to poll for completion otherwise)
		DispatcherQueueController.Controller = winrt::Windows::System::DispatcherQueueController { nullptr };

		return 1;
	}
	catch(...)
	{
		LOG_CAUGHT_EXCEPTION();
		return wil::ResultFromCaughtException();
	}
}
