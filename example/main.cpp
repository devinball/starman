#include <entt/entt.hpp>

#include "core/application.hpp"

int main() {
    printf("\n-----STARTING-----\n\n");

    auto app = Application{};

    app.run();

    printf("\n-----ENDING-----\n\n");

    return 0;
}


/*

// -- SYSTEMS --



// a prefab is a pre-made entity for easy instantiating, can be instantiated into a scene via the prefab api
struct Prefab {}

// abstract away all the rendering stuff into it's own little corner, completely library/api agnostic
// render will take in either a scene or a series of scenes
struct Renderer {
    void render();
}

struct EntityManager {
    public:
        EntityManager() {
            for (Entity entity = 0; entity < MAX_ENTITIES, ++entity) {
                availableEntites.push(entity)
            }
        }

        Entity CreateEntity() {
            assert(livingEntityCount < MAX_ENTITIES && "Too many entities!");

            Entity id = availableEntites.front();
            availableEntites.pop();
            ++livingEntityCount;

            return id;
        }

        void DestroyEntity();
        void SetSignature();
        Signature GetSignature();
    private:
        std::queue<Entity> availableEntites{};
        std::array<Signature, MAX_ENTITIES> signatures{};
        uint32_t livingEntityCount{};
}

// lua - component system

return {
    component = {
        position; // can i do any sort of typing?
        velocity;
    },
    system = {
        init = function(entity, component)

        end
        loop = function(entity, component)
            component.position = component.position + component.velocity
        end  
    },
}

*/