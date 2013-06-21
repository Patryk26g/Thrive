#include "game.h"

#include "engine/engine.h"
#include "engine/entity_manager.h"
#include "engine/shared_data.h"
#include "engine/typedefs.h"
#include "scripting/lua_state.h"
#include "util/make_unique.h"

#include <boost/thread.hpp>
#include <type_traits>
#include <unordered_map>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>


using namespace thrive;

struct Game::Implementation {

    using Clock = std::chrono::high_resolution_clock;

    Implementation()
      : m_engine(m_entityManager, m_luaState)
    {
        m_targetFrameDuration = std::chrono::microseconds(1000000 / m_targetFrameRate);
    }
    // Lua state must be one of the last to be destroyed, so keep it at top. 
    // The reason for that is that some components keep luabind::object 
    // instances around that rely on the lua state to still exist when they
    // are destroyed. Since those components are destroyed with the entity 
    // manager, the lua state has to live longer than the manager.
    LuaState m_luaState;

    // EntityManager is required by the engine 
    // constructor, so keep it at second place
    EntityManager m_entityManager;

    Engine m_engine;

    std::chrono::microseconds m_targetFrameDuration;

    unsigned short m_targetFrameRate = 60;

    bool m_quit;

};


Game&
Game::instance() {
    static Game instance;
    return instance;
}


Game::Game()
  : m_impl(new Implementation())
{
}


Game::~Game() { }


EntityManager&
Game::entityManager() {
    return m_impl->m_entityManager;
}


Engine&
Game::engine() {
    return m_impl->m_engine;
}


void
Game::quit() {
    m_impl->m_quit = true;
}


void
Game::run() {
    unsigned int fpsCount = 0;
    int fpsTime = 0;
    auto lastUpdate = Implementation::Clock::now();
    m_impl->m_engine.init();
    // Start game loop
    m_impl->m_quit = false;
    while (not m_impl->m_quit) {
        auto now = Implementation::Clock::now();
        auto delta = now - lastUpdate;
        int milliSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(delta).count();
        lastUpdate = now;
        m_impl->m_engine.update(milliSeconds);
        m_impl->m_entityManager.update();
        auto frameDuration = Implementation::Clock::now() - now;
        auto sleepDuration = m_impl->m_targetFrameDuration - frameDuration;
        if (sleepDuration.count() > 0) {
            auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(sleepDuration).count();
            boost::chrono::microseconds boostDuration = boost::chrono::microseconds(microseconds);
            boost::this_thread::sleep_for(boostDuration);
        }
        fpsCount += 1;
        fpsTime += std::chrono::duration_cast<std::chrono::milliseconds>(frameDuration).count();
        if (fpsTime >= 1000) {
            float fps = 1000 * float(fpsCount) / float(fpsTime);
            std::cout << "FPS: " << fps << std::endl;
            fpsCount = 0;
            fpsTime = 0;
        }
    }
    m_impl->m_engine.shutdown();
}


std::chrono::microseconds
Game::targetFrameDuration() const {
    return m_impl->m_targetFrameDuration;
}


unsigned short
Game::targetFrameRate() const {
    return m_impl->m_targetFrameRate;
}


