#pragma once

#include <windows.h>
#include <endpointvolume.h>
#include <mmdeviceapi.h>
#include <wrl/client.h>

namespace throwback::shell {

// The speakers' volume: the default playback device's master level and
// mute, through Windows' Core Audio API. Changes made anywhere (keyboard
// keys, other apps, the device itself) are posted to notify as message;
// wParam is 1 when the default device changed, so Reattach() follows it.
class Volume {
 public:
  Volume(HWND notify, UINT message);
  ~Volume();
  Volume(const Volume&) = delete;
  Volume& operator=(const Volume&) = delete;

  // False without a playback device.
  bool Available() const { return endpoint_ != nullptr; }
  float Level() const;  // 0 to 1
  bool Muted() const;
  // Moving the level up also unmutes, as Windows' own slider does.
  void SetLevel(float level);
  void SetMuted(bool muted);
  // Follows a new default device.
  void Reattach();

 private:
  class Listener;

  Microsoft::WRL::ComPtr<IMMDeviceEnumerator> enumerator_;
  Microsoft::WRL::ComPtr<IAudioEndpointVolume> endpoint_;
  Listener* listener_ = nullptr;
};

}  // namespace throwback::shell
