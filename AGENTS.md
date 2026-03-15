# imjack - Agent Guidelines

## Project Overview

**imjack** is an ImGui-based JACK audio router/channel selector with a configurable number of inputs. It provides an 8x8 grid interface for selecting stereo audio input pairs and routing them to the output.

- **Language:** C++ (C++11 standard)
- **Build System:** CMake (minimum 3.16.3)
- **License:** GPL v3
- **Platform:** Linux (Ubuntu tested)

## Dependencies

- **GLFW3** - Windowing library
- **OpenGL** - Graphics rendering
- **JACK Audio Connection Kit** - Real-time audio server
- **Dear ImGui** - Immediate mode GUI (bundled)
- **jackcpp** - C++ wrapper for JACK (bundled in `jackcpp/`)

## Project Structure

```
/home/yekm/src/yekm/imjack/
├── main.cpp              # Main application entry point
├── myimgui.cpp/hpp       # GLFW/ImGui initialization and window management
├── imgui_elements.cpp/h  # Custom ImGui widgets (CPU load monitor)
├── timer.cpp/h           # High-resolution timer utility
├── jackcpp/              # C++ JACK wrapper library
│   ├── include/          # Header files
│   └── src/              # Implementation files
├── cmake/modules/        # CMake find modules
├── .github/workflows/    # CI/CD configuration
├── CMakeLists.txt        # Build configuration
└── README.md             # Project description
```

## Build Instructions

`cmake --build build -j$(nproc)`

### GUI Controls

- **8x8 Grid:** Click any cell to route that stereo input pair to the output
- **Special Input:** Additional input routing option (green button)
- **CPU Monitor:** Shows user/system CPU usage and memory consumption

## Architecture

### Main Components

1. **TestJack Class**
   - Inherits from `JackCpp::AudioIO`
   - Handles JACK audio callbacks
   - Manages input/output port routing
   - Supports 8x8 grid (64 stereo pairs) + 1 special input
   - 2 stereo output ports

2. **Audio Flow**
   - Creates 128 input ports + 2 special ports = 130 total inputs
   - Routes selected input pair (or special) to 2 output ports
   - Real-time processing in `audioCallback()`

3. **GUI Framework**
   - GLFW3 for window and input handling
   - OpenGL 3.0+ for rendering
   - Dear ImGui for UI widgets
   - 60 FPS update rate

## Key Classes and Files

### `TestJack` (main.cpp:11-68)
- `audioCallback()` - Real-time audio processing
- `set_current_in(i, j)` - Select input from grid
- `set_special(s)` - Activate special input routing
- Constructor initializes 8x8 grid + special inputs

### `JackCpp::AudioIO` (jackcpp/include/jackaudioio.hpp)
- Abstract base class for JACK clients
- Port management (reserve/add/connect)
- Real-time callback processing
- Connection management

### Custom Widgets

- `cpu_load_text_now()` - Formats CPU usage statistics
- `cpu_load_gui()` - Renders CPU load display

## Development Guidelines

### Code Style

- Use C++11 features consistently
- Prefer explicit over implicit conversions
- Use `std::vector` for dynamic arrays
- Follow existing naming conventions:
  - `camelCase` for methods and variables
  - `PascalCase` for classes
  - `m_` prefix for member variables

### Audio Programming

- **Never** allocate memory or lock in audio callback
- Use pre-allocated buffers
- Keep callback execution time minimal
- Use `reserveInPorts()`/`reserveOutPorts()` before starting client

### JACK Integration

- Always check return values from JACK functions
- Handle JACK server disconnection gracefully
- Use `jack_shutdown_callback` for cleanup

### ImGui Usage

- Call `ImGui::NewFrame()` at start of frame
- Call `ImGui::Render()` after all UI code
- Use `ImGui::PushStyleColor()`/`PopStyleColor()` for custom button colors
- Keep UI responsive - don't block main thread

## Testing

- Test with `jackd` running before starting application
- Verify audio routing with `jack_connect` command-line tool
- Monitor for xruns with `jack_cpu_load`

## Common Issues

1. **JACK server not running:** Start with `jackd -d alsa` or use QjackCtl
2. **Port connection errors:** Check JACK connections with `jack_lsp`
3. **Build failures:** Ensure all dependencies are installed

## CI/CD

GitHub Actions workflow (`.github/workflows/cmake-single-platform.yml`):
- Triggers on push/PR to master
- Installs dependencies
- Builds Release configuration
- Runs on ubuntu-latest

## Resources

- [JACK Audio](https://jackaudio.org/)
- [Dear ImGui](https://github.com/ocornut/imgui)
- [GLFW](https://www.glfw.org/)
- [jackcpp](https://github.com/x37v/jackcpp)
