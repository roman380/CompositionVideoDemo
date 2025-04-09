#include "pch.h"
#include "MainWindow.h"

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Numerics;
    using namespace Windows::UI;
    using namespace Windows::UI::Composition;
    using namespace Windows::Media::Core;
    using namespace Windows::Media::Playback;
    using namespace Windows::Storage;
    using namespace Windows::Storage::Streams;
}

namespace util
{
    using namespace robmikh::common::desktop;
}

int __stdcall WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
    // Parse our command line args
    int argc = 0;
    auto argv = winrt::check_pointer(CommandLineToArgvW(GetCommandLineW(), &argc));
    std::vector<std::wstring> args(argv + 1, argv + argc);
    if (args.empty())
    {
        MessageBoxW(
            nullptr,
            L"Expected video path command line argument!",
            L"CompositionVideoDemo",
            MB_ICONERROR);
        return 1;
    }
    auto videoPath = args[0];

    // Initialize COM
    winrt::init_apartment(winrt::apartment_type::single_threaded);

    // Create the DispatcherQueue that the compositor needs to run
    auto controller = util::CreateDispatcherQueueControllerForCurrentThread();

    // Create our window and visual tree
    auto window = MainWindow(L"CompositionVideoDemo", 800, 600);
    auto compositor = winrt::Compositor();
    auto target = window.CreateWindowTarget(compositor);
    auto root = compositor.CreateSpriteVisual();
    root.RelativeSizeAdjustment({ 1.0f, 1.0f });
    root.Brush(compositor.CreateColorBrush(winrt::Colors::Black()));
    target.Root(root);

    // Setup our MediaPlayer
    auto player = winrt::MediaPlayer();
    auto surface = player.GetSurface(compositor);
    auto contentVisual = compositor.CreateSpriteVisual();
    contentVisual.RelativeSizeAdjustment({ 1.0f, 1.0f });
    contentVisual.Brush(compositor.CreateSurfaceBrush(surface.CompositionSurface()));
    root.Children().InsertAtTop(contentVisual);

    // Open our video as a IRandomAccessStream
    winrt::IRandomAccessStream stream{ nullptr };
    winrt::check_hresult(CreateRandomAccessStreamOnFile(videoPath.c_str(), static_cast<DWORD>(winrt::FileAccessMode::Read), winrt::guid_of<decltype(stream)>(), winrt::put_abi(stream)));

    // Hookup the video to our MediaPlayer
    auto source = winrt::MediaSource::CreateFromStream(stream, L"");
    auto mediaItem = winrt::MediaPlaybackItem(source);
    player.Source(mediaItem);
    player.Play();

    // Message pump
    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return util::ShutdownDispatcherQueueControllerAndWait(controller, static_cast<int>(msg.wParam));
}
