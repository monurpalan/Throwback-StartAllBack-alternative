#include "shell/volume.h"

namespace throwback::shell {

// Receives Core Audio's callbacks, which arrive on its own threads, and only
// posts them to the bar's window.
class Volume::Listener : public IAudioEndpointVolumeCallback, public IMMNotificationClient {
 public:
  Listener(HWND notify, UINT message) : notify_(notify), message_(message) {}

  // IUnknown
  ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&refs_); }
  ULONG STDMETHODCALLTYPE Release() override {
    const ULONG refs = InterlockedDecrement(&refs_);
    if (refs == 0) delete this;
    return refs;
  }
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** object) override {
    if (iid == __uuidof(IUnknown) || iid == __uuidof(IAudioEndpointVolumeCallback)) {
      *object = static_cast<IAudioEndpointVolumeCallback*>(this);
    } else if (iid == __uuidof(IMMNotificationClient)) {
      *object = static_cast<IMMNotificationClient*>(this);
    } else {
      *object = nullptr;
      return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
  }

  // IAudioEndpointVolumeCallback: the level or mute changed.
  HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA) override {
    PostMessageW(notify_, message_, 0, 0);
    return S_OK;
  }

  // IMMNotificationClient: another device became the default.
  HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR) override {
    if (flow == eRender && role == eMultimedia) PostMessageW(notify_, message_, 1, 0);
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR, DWORD) override {
    PostMessageW(notify_, message_, 1, 0);
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR, const PROPERTYKEY) override {
    return S_OK;
  }

 private:
  virtual ~Listener() = default;

  LONG refs_ = 1;
  HWND notify_;
  UINT message_;
};

Volume::Volume(HWND notify, UINT message) : listener_(new Listener(notify, message)) {
  if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(enumerator_.GetAddressOf())))) {
    return;
  }
  enumerator_->RegisterEndpointNotificationCallback(listener_);
  Reattach();
}

Volume::~Volume() {
  if (endpoint_) endpoint_->UnregisterControlChangeNotify(listener_);
  if (enumerator_) enumerator_->UnregisterEndpointNotificationCallback(listener_);
  endpoint_.Reset();
  enumerator_.Reset();
  listener_->Release();
}

void Volume::Reattach() {
  if (endpoint_) endpoint_->UnregisterControlChangeNotify(listener_);
  endpoint_.Reset();
  if (!enumerator_) return;
  Microsoft::WRL::ComPtr<IMMDevice> device;
  if (FAILED(enumerator_->GetDefaultAudioEndpoint(eRender, eMultimedia, device.GetAddressOf()))) {
    return;  // no speakers or headphones right now
  }
  if (SUCCEEDED(device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, nullptr,
                                 reinterpret_cast<void**>(endpoint_.GetAddressOf())))) {
    endpoint_->RegisterControlChangeNotify(listener_);
  }
}

float Volume::Level() const {
  float level = 0.0f;
  if (endpoint_) endpoint_->GetMasterVolumeLevelScalar(&level);
  return level;
}

bool Volume::Muted() const {
  BOOL muted = FALSE;
  if (endpoint_) endpoint_->GetMute(&muted);
  return muted != FALSE;
}

void Volume::SetLevel(float level) {
  if (!endpoint_) return;
  endpoint_->SetMasterVolumeLevelScalar(level, nullptr);
  if (level > 0.0f && Muted()) endpoint_->SetMute(FALSE, nullptr);
}

void Volume::SetMuted(bool muted) {
  if (endpoint_) endpoint_->SetMute(muted ? TRUE : FALSE, nullptr);
}

}  // namespace throwback::shell
