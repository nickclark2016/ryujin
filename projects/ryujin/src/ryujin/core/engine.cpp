#include <ryujin/core/engine.hpp>

#include <ryujin/input/input.hpp>

#undef APIENTRY
#include <spdlog/spdlog.h>

#include <atomic>
#include <barrier>
#include <chrono>
#include <memory>
#include <mutex>
#include <semaphore>
#include <string>
#include <thread>
#include <unordered_map>

namespace ryujin
{
    struct engine_context::impl
    {
        std::unordered_map<std::string, unique_ptr<window>> _windows;

        unique_ptr<window> _invalidWindowHandle;

        unique_ptr<asset_manager> _assets;
        unique_ptr<render_system> _renderer;

        registry _reg;

        std::atomic_bool _isRunning;
        std::binary_semaphore _rendererComplete = std::binary_semaphore(0);
        std::binary_semaphore _gameLogicComplete = std::binary_semaphore(1);
        std::thread _renderLogic;

        f64 _delta;
        std::chrono::time_point<std::chrono::high_resolution_clock> _lastTime;
    };

    engine_context::engine_context()
        : _impl(new impl())
    {
#ifdef _DEBUG
        spdlog::set_level(spdlog::level::debug);
#else
        spdlog::set_level(spdlog::level::err);
#endif
        _impl->_assets = make_unique<asset_manager>();
    }

    engine_context::~engine_context()
    {
        _impl->_windows.clear();
    }

    unique_ptr<window>& engine_context::add_window(const window::create_info info)
    {
        auto result = window::create(info);
        if (result)
        {
            _impl->_windows[info.name] = result.success();
            auto& win = _impl->_windows[info.name];
            input::register_window(win);

            win->after_close([info, this]() {
                    this->remove_window(info.name);
                });

            return win;
        }

        return _impl->_invalidWindowHandle;
    }
    
    void engine_context::remove_window(const std::string& win)
    {
        _impl->_windows.erase(win);
    }
    
    unique_ptr<window>& engine_context::get_window(const std::string& name)
    {
        const auto& it = _impl->_windows.find(name);
        if (it == _impl->_windows.end())
        {
            return _impl->_invalidWindowHandle;
        }
        return it->second;
    }

    const std::unordered_map<std::string, unique_ptr<window>>& engine_context::get_windows() const noexcept
    {
        return _impl->_windows;
    }
    
    void engine_context::execute(unique_ptr<base_application>& app)
    {
        using std::chrono::high_resolution_clock;

        _impl->_isRunning.store(true);
        _impl->_renderer = make_unique<render_system>();

        std::barrier initSync(2);

        // invoke pre-system initialization logic
        app->pre_init(*this);

        _impl->_renderLogic = std::thread([this, &app, &initSync]() {
                // initialize systems
            _impl->_renderer->on_init(*this);
                auto illegalTextureAsset = get_assets().load_texture("data/textures/invalid_texture.png");
                _impl->_renderer->get_render_manager(0)->renderables().load_texture("internal_illegal_texture", *illegalTextureAsset);

                initSync.arrive_and_wait();

                while (_impl->_isRunning.load())
                {
                    _impl->_gameLogicComplete.acquire();
                    _impl->_renderer->render_prework(*this);
                    _impl->_rendererComplete.release();
                    _impl->_renderer->on_pre_render(*this);
                    app->on_render(*this);
                    _impl->_renderer->on_render(*this);
                    app->post_render(*this);
                    _impl->_renderer->on_post_render(*this);
                }
            });

        initSync.arrive_and_wait();
        // initialization complete

        app->on_load(*this);

        while (!_impl->_windows.empty()) {
            _impl->_rendererComplete.acquire();
            input::poll();

            auto currentTime = high_resolution_clock::now();
            auto delta = std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - _impl->_lastTime);
            _impl->_delta = delta.count();

            app->on_frame(*this);

            _impl->_lastTime = currentTime;

            _impl->_gameLogicComplete.release();
        }

        _impl->_isRunning.store(false);

        _impl->_renderLogic.join();

        app->on_exit(*this);

        _impl->_renderer = nullptr;
        glfwTerminate();
    }

    registry& engine_context::get_registry() noexcept
    {
        return _impl->_reg;
    }

    asset_manager& engine_context::get_assets() noexcept
    {
        return *_impl->_assets;
    }
    
    render_system& engine_context::get_render_system() noexcept
    {
        return *_impl->_renderer;
    }
    
    f64 engine_context::deltaTime() const noexcept
    {
        return _impl->_delta;
    }
}