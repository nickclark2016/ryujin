#include <ryujin/core/engine.hpp>

#include <ryujin/input/input.hpp>

#undef APIENTRY
#include <spdlog/spdlog.h>

#include <barrier>
#include <mutex>

namespace ryujin
{
    engine_context::engine_context()
        : _rendererComplete(0), _gameLogicComplete(1)
    {
#ifdef _DEBUG
        spdlog::set_level(spdlog::level::debug);
#else
        spdlog::set_level(spdlog::level::err);
#endif
        _assets = std::make_unique<asset_manager>();
    }

    engine_context::~engine_context()
    {
        _windows.clear();
    }

    std::unique_ptr<window>& engine_context::add_window(const window::create_info info)
    {
        auto result = window::create(info);
        if (result)
        {
            _windows[info.name] = result.success();
            auto& win = _windows[info.name];
            input::register_window(win);

            win->on_close([info, this]() {
                    this->remove_window(info.name);
                });

            return win;
        }

        return _invalidWindowHandle;
    }
    
    void engine_context::remove_window(const std::string& win)
    {
        _windows.erase(win);
    }
    
    std::unique_ptr<window>& engine_context::get_window(const std::string& name)
    {
        const auto& it = _windows.find(name);
        if (it == _windows.end())
        {
            return _invalidWindowHandle;
        }
        return it->second;
    }

    const std::unordered_map<std::string, std::unique_ptr<window>>& engine_context::get_windows() const noexcept
    {
        return _windows;
    }
    
    void engine_context::execute(std::unique_ptr<base_application>& app)
    {
        _isRunning.store(true);
        _renderer = std::make_unique<render_system>();

        std::barrier initSync(2);

        // invoke pre-system initialization logic
        app->pre_init(*this);

        _renderLogic = std::thread([this, &app, &initSync]() {
                // initialize systems
                _renderer->on_init(*this);
                auto illegalTextureAsset = get_assets().load_texture("data/textures/invalid_texture.png");
                _renderer->get_render_manager(0)->renderables().load_texture("internal_illegal_texture", *illegalTextureAsset);

                initSync.arrive_and_wait();

                while (_isRunning.load())
                {
                    _gameLogicComplete.acquire();
                    _renderer->render_prework(*this);
                    _rendererComplete.release();
                    _renderer->on_pre_render(*this);
                    app->on_render(*this);
                    _renderer->on_render(*this);
                    app->post_render(*this);
                    _renderer->on_post_render(*this);
                }
            });

        initSync.arrive_and_wait();
        // initialization complete

        app->on_load(*this);

        while (!_windows.empty()) {
            _rendererComplete.acquire();
            input::poll();

            app->on_frame(*this);
            _gameLogicComplete.release();
        }

        _isRunning.store(false);

        _renderLogic.join();

        app->on_exit(*this);
    }

    registry& engine_context::get_registry() noexcept
    {
        return _reg;
    }

    asset_manager& engine_context::get_assets() noexcept
    {
        return *_assets;
    }
    
    render_system& engine_context::get_render_system() noexcept
    {
        return *_renderer;
    }
}