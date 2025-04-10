#pragma once

#include <windows.h>

#include <unknwn.h>
#include <winrt\base.h>

#include <wil\resource.h>
#include <wil\com.h>
#include <wil\cppwinrt.h>

#include <winrt\Windows.Foundation.h>
#include <winrt\Windows.Foundation.Collections.h>
#include <winrt\Windows.Foundation.Numerics.h>
#include <winrt\Windows.System.h>
#include <winrt\Windows.UI.h>
#include <winrt\Windows.UI.Composition.h>
#include <winrt\Windows.UI.Composition.Desktop.h>
#include <winrt\Windows.UI.Popups.h>
#include <winrt\Windows.Graphics.Capture.h>
#include <winrt\Windows.Graphics.DirectX.h>
#include <winrt\Windows.Graphics.DirectX.Direct3d11.h>
#include <winrt\Windows.Media.Core.h>
#include <winrt\Windows.Media.Playback.h>
#include <winrt\Windows.Storage.h>
#include <winrt\Windows.Storage.Streams.h>

#include <dxgi1_6.h>
#include <d3d11_4.h>
#include <d2d1_3.h>
#include <wincodec.h>

#include <shellapi.h>
#include <shcore.h>
#include <shlwapi.h>

#include <vector>
#include <string>
#include <atomic>
#include <memory>
#include <algorithm>
#include <mutex>

#pragma comment(lib, "windowsapp.lib")
