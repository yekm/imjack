# TODO - Project Improvements

- [ ] **Add error handling for JACK connection failures**
  - Currently no graceful handling if JACK server is not available
  - Should show user-friendly error message instead of crashing

- [ ] **Make grid size configurable**
  - Currently hardcoded to 8x8 in both code and UI
  - Add command-line options for rows and columns
  - Update port creation logic to use configurable size

- [ ] **Add configuration file support**
  - Support JSON or YAML for saving/loading settings
  - Store default grid size, window position, last used input
  - Save/load routing presets

- [ ] **Add audio level meters**
  - Display input level for each cell
  - Show clipping indicators
  - Configurable meter ballistics

- [ ] **Optimize audio callback**
  - Profile CPU usage in callback
  - Consider using SIMD for audio copying
  - Minimize branch predictions in hot path

- [ ] **Reduce GUI overhead**
  - Only update CPU text every N frames
  - Use ImGui performance flags
  - Consider reducing frame rate when idle

- [ ] **Add gain control per input**
  - Volume faders for each input pair
  - Mute/solo functionality
  - Smooth gain ramping to avoid clicks

- [ ] **Add metering/statistics**
  - Peak and RMS levels
  - Long-term statistics
  - Export to file

- [ ] **Network control interface**
  - OSC (Open Sound Control) support
  - Web interface for remote control
  - REST API
