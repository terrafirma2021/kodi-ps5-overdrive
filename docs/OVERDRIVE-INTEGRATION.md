# Overdrive in PS5MC

PS5MC is the application. Kodi owns the window system, media playback, settings,
skins, and update path. Overdrive becomes a built-in PS5MC subsystem rather than
another executable or a Python add-on.

The intended integration has three native parts. Only the first part and its
initial Kodi window are implemented in this preview:

1. **Implemented:** a catalog model reads versioned JSON with title metadata.
   The importer accepts the recovered Overdrive catalog as input; catalog data
   is separate from Kodi's source and can be refreshed without rebasing Kodi.
2. **Partly implemented:** a Kodi media window presents Games and Homebrew lists
   and a title-detail dialog using Kodi's controller, focus, skin, and artwork
   handling. Search, Downloads, and Library views are still to be built.
3. **Not implemented:** a PS5 download and install service for persistent jobs,
   transfers, archive extraction, verification, and ShadowMount registration.

The first code slice adds the native window and catalog browsing. Download and
install operations will only be exposed after the service has persistent jobs,
verified downloads, and a PS5 hardware test. This keeps the store visible while
avoiding controls that appear to install content but do nothing.

The recovered Overdrive sources are reference implementations for behaviour and
data format. PS5MC's existing Kodi/Kodi 22 code and PS5 platform code remain the
source of truth for rendering and playback.
