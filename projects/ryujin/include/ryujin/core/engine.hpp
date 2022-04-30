#ifndef engine_hpp__
#define engine_hpp__

#include "assets.hpp"
#include "smart_pointers.hpp"

#include "../entities/registry.hpp"
#include "../graphics/window.hpp"
#include "../graphics/render_system.hpp"

#include <atomic>
#include <chrono>
#include <memory>
#include <semaphore>
#include <string>
#include <thread>
#include <unordered_map>

namespace ryujin
{
    class engine_context;

    /// <summary>
    /// Base class for applications.
    /// </summary>
    class base_application
    {
    public:
        /// <summary>
        /// Default destructor of applications.
        /// </summary>
        virtual ~base_application() = default;

        /// <summary>
        /// Functionality to execute before engine initialization.
        /// </summary>
        /// <param name="ctx">Reference to the engine context executing the application.</param>
        virtual void pre_init(engine_context& ctx) = 0;

        /// <summary>
        /// Functionality to execute after engine initialization, but before the main application loop.
        /// </summary>
        /// <param name="ctx">Reference to the engine context executing the application.</param>
        virtual void on_load(engine_context& ctx) = 0;

        /// <summary>
        /// Functionality to after the main application loop but before exit.
        /// </summary>
        /// <param name="ctx">Reference to the engine context executing the application.</param>
        virtual void on_exit(engine_context& ctx) = 0;

        /// <summary>
        /// Functionality to execute on each step of the main application loop.
        /// </summary>
        /// <param name="ctx">Reference to the engine context executing the application.</param>
        virtual void on_frame(engine_context& ctx) = 0;

        /// <summary>
        /// Functionality to execute after render of each frame on the render thread.
        /// </summary>
        /// <param name="ctx">Reference to the engine context executing the application.</param>
        virtual void post_render(engine_context& ctx) = 0;

        /// <summary>
        /// Functionality to execute on render of each frame on the render thread.
        /// </summary>
        /// <param name="ctx">Reference to the engine context executing the application.</param>
        virtual void on_render(engine_context& ctx) = 0;
    };


    /// <summary>
    /// Class containing engine execution logic.
    /// </summary>
    class engine_context
    {
    public:
        /// <summary>
        /// Constructs a new engine.
        /// </summary>
        engine_context();

        /// <summary>
        /// Destructs an engine
        /// </summary>
        ~engine_context();

        /// <summary>
        /// Adds a window for the engine to manage.
        /// </summary>
        /// <param name="info">Construction information for the window</param>
        /// <returns>Unique pointer containing the window if successful, else an empty unique pointer</returns>
        unique_ptr<window>& add_window(const window::create_info info);

        /// <summary>
        /// Removes a window from the engine's management.
        /// </summary>
        /// <param name="win">Name of the window to remove</param>
        void remove_window(const std::string& win);

        /// <summary>
        /// Gets a unique pointer to a window with the given name.
        /// </summary>
        /// <param name="name">Name of the window</param>
        /// <returns>Unique pointer to the window of that name</returns>
        unique_ptr<window>& get_window(const std::string& name);

        /// <summary>
        /// Gets a constant reference to the map of windows that the engine currently manages.  The key of the map
        /// is the name of the window.
        /// </summary>
        /// <returns>Managed window map</returns>
        const std::unordered_map<std::string, unique_ptr<window>>& get_windows() const noexcept;

        /// <summary>
        /// Executes an application.
        /// </summary>
        /// <param name="app">Application to execute</param>
        void execute(unique_ptr<base_application>& app);

        /// <summary>
        /// Gets a reference to the engine's entity registry.
        /// </summary>
        /// <returns>Engine entity registry</returns>
        registry& get_registry() noexcept;

        /// <summary>
        /// Gets a reference to the engine's assset manager.
        /// </summary>
        /// <returns>Engine asset manager</returns>
        asset_manager& get_assets() noexcept;

        /// <summary>
        /// Gets a reference to the engine's render system.
        /// </summary>
        /// <returns>Engine's render system</returns>
        render_system& get_render_system() noexcept;

        /// <summary>
        /// Gets the time it took the engine to execute the last frame of logic.
        /// </summary>
        /// <returns>Time in seconds</returns>
        f64 deltaTime() const noexcept;
    private:

        std::unordered_map<std::string, unique_ptr<window>> _windows;

        unique_ptr<window> _invalidWindowHandle;
        
        unique_ptr<asset_manager> _assets;
        unique_ptr<render_system> _renderer;

        registry _reg;

        std::atomic_bool _isRunning;
        std::binary_semaphore _rendererComplete;
        std::binary_semaphore _gameLogicComplete;
        std::thread _renderLogic;

        f64 _delta;
        std::chrono::time_point<std::chrono::high_resolution_clock> _lastTime;
    };
}

#endif // engine_hpp__
