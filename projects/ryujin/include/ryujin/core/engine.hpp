#ifndef engine_hpp__
#define engine_hpp__

#include "assets.hpp"

#include "../entities/registry.hpp"
#include "../graphics/window.hpp"
#include "../graphics/render_system.hpp"

#include <atomic>
#include <memory>
#include <mutex>
#include <semaphore>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>

namespace ryujin
{
    class engine_context;

    class base_application
    {
    public:
        virtual ~base_application() = default;
        virtual void on_load(engine_context& ctx) = 0;
        virtual void on_exit(engine_context& ctx) = 0;
        virtual void on_frame(engine_context& ctx) = 0;
    };

    class engine_context
    {
    public:
        engine_context();
        ~engine_context();

        std::unique_ptr<window>& add_window(const window::create_info info);
        void remove_window(const std::string& win);
        std::unique_ptr<window>& get_window(const std::string& name);

        const std::unordered_map<std::string, std::unique_ptr<window>>& get_windows() const noexcept;

        void execute(std::unique_ptr<base_application>& app);

        registry& get_registry() noexcept;
        asset_manager& get_assets() noexcept;
        render_system& get_render_system() noexcept;
    private:

        std::unordered_map<std::string, std::unique_ptr<window>> _windows;

        std::unique_ptr<window> _invalidWindowHandle;
        
        std::unique_ptr<asset_manager> _assets;
        std::unique_ptr<render_system> _renderer;

        registry _reg;

        std::atomic_bool _isRunning;
        std::binary_semaphore _rendererComplete;
        std::binary_semaphore _gameLogicComplete;
        std::thread _gameLogic;
    };
}

#endif // engine_hpp__
