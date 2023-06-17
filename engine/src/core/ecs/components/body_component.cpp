#include <core/ecs/components/body_component.h>

namespace Sunset
{
    void set_body_position(BodyComponent* body_comp, const glm::vec3& position)
    {
        assert(body_comp != nullptr && "Cannot set body position on a null body component!");
        if (body_comp->body_data.body >= 0)
        {
            body_comp->body_data.position = position;
            body_comp->body_data.dirty_flags |= PhysicsBodyDirtyFlags::POSITION;
        }
    }

    void set_body_rotation(BodyComponent* body_comp, const glm::quat& rotation)
    {
        assert(body_comp != nullptr && "Cannot set body rotation on a null body component!");
        if (body_comp->body_data.body >= 0)
        {
            body_comp->body_data.rotation = rotation;
            body_comp->body_data.dirty_flags |= PhysicsBodyDirtyFlags::ROTATION;
        }
    }

    void set_body_type(BodyComponent* body_comp, PhysicsBodyType body_type)
    {
        assert(body_comp != nullptr && "Cannot set body type on a null body component!");
        if (body_comp->body_data.body >= 0)
        {
            body_comp->body_data.body_type = body_type;
            body_comp->body_data.dirty_flags |= PhysicsBodyDirtyFlags::BODY_TYPE;
        }
    }

    void set_body_position(Scene* scene, EntityID entity, const glm::vec3& position)
    {
        if (BodyComponent* const body_comp = scene->get_component<BodyComponent>(entity))
        {
            set_body_position(body_comp, position);
        }
    }

    void set_body_rotation(Scene* scene, EntityID entity, const glm::quat& rotation)
    {
        if (BodyComponent* const body_comp = scene->get_component<BodyComponent>(entity))
        {
            set_body_rotation(body_comp, rotation);
        }
    }

    void set_body_type(Scene* scene, EntityID entity, PhysicsBodyType body_type)
    {
        if (BodyComponent* const body_comp = scene->get_component<BodyComponent>(entity))
        {
            set_body_type(body_comp, body_type);
        }
    }
}
