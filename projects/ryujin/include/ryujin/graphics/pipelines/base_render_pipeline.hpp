#ifndef base_render_pipeline_hpp__
#define base_render_pipeline_hpp__

namespace ryujin
{
    class render_manager;

    class base_render_pipeline
    {
    public:
        virtual ~base_render_pipeline() = default;

        inline void set_render_manager(render_manager* manager)
        {
            _manager = manager;
            initialize();
        }

        inline virtual void pre_render() {};
        virtual void render() = 0;

    protected:
        virtual void initialize() = 0;
        inline render_manager* get_render_manager() noexcept
        {
            return _manager;
        }

    private:
        render_manager* _manager;
    };
}

#endif