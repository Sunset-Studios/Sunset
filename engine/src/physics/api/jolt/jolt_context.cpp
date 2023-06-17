#include <physics/api/jolt/jolt_context.h>
#include <utility/cvar.h>

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/Memory.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/TaperedCapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>

#include <physics/physics_types.h>

#include <iostream>

namespace Sunset
{
	AutoCVar_Int cvar_max_physics_jobs("phys.max_physics_jobs", "The maximum number of physics jobs that are allowed to be created and queued up at any one time.", 1024);
	AutoCVar_Int cvar_max_physics_barriers("phys.max_physics_barriers", "Jolt uses barriers to batch parallel work and wait on all the batched jobs to complete.", 8);
	AutoCVar_Int cvar_max_physics_bodies("phys.max_physics_bodies", "Maximum number of bodies the system is allowed to create and keep track of.", MAX_PHYSICS_BODIES);
	AutoCVar_Int cvar_max_physics_body_pairs("phys.max_physics_body_pairs", "Maximum number of body pairs that can be queued at any given time during broadphase update.", 65536);
	AutoCVar_Int cvar_max_physics_contact_constraints("phys.max_physics_contact_constraints", "Maximum size of the contact constraint buffer used for contact resolution between bodies.", 10240);
	AutoCVar_Int cvar_num_collision_steps("phys.num_collision_steps", "Number of collision steps to do when updating the physics simulation", 1);
	AutoCVar_Int cvar_num_integration_substeps("phys.num_integration_substeps", "Number of substeps to do during integration when updating the physics simulation", 1);

	void JoltContext::initialize()
	{
		// Register allocation hook
		JPH::RegisterDefaultAllocator();

		// Create a factory
		JPH::Factory::sInstance = new JPH::Factory();

		// Register all Jolt physics types
		JPH::RegisterTypes();

		// Allocate our physics system data
		system_data = std::make_unique<SystemData>();

		// Create our custom job system to use for queuing up Jolt jobs on multiple threads 
		system_data->job_system.Init(cvar_max_physics_jobs.get(), cvar_max_physics_barriers.get());

		// Initialize our actual physics system
		const int32_t num_body_mutexes = 0;
		system_data->physics_system.Init(
			cvar_max_physics_bodies.get(),
			num_body_mutexes,
			cvar_max_physics_body_pairs.get(),
			cvar_max_physics_contact_constraints.get(),
			hooks.broad_phase_object_layer_mapping,
			hooks.broad_phase_layer_filter,
			hooks.object_layer_filter
		);

		// A body activation listener gets notified when bodies activate and go to sleep
		// Note that this is called from a job so whatever you do here needs to be thread safe.
		// Registering one is entirely optional.
		system_data->physics_system.SetBodyActivationListener(&hooks.activation_listener);

		// A contact listener gets notified when bodies (are about to) collide, and when they separate again.
		// Note that this is called from a job so whatever you do here needs to be thread safe.
		// Registering one is entirely optional.
		system_data->physics_system.SetContactListener(&hooks.contact_listener);
	}

	void JoltContext::destroy()
	{
		// Destroy our system data
		system_data.reset();

		// Unregisters all types with the factory and cleans up the default material
		JPH::UnregisterTypes();

		// Destroy the factory
		delete JPH::Factory::sInstance;
		JPH::Factory::sInstance = nullptr;
	}

	void JoltContext::step_simulation()
	{
		static const uint32_t fixed_delta_time = 1.0f / 60.0f;
		system_data->physics_system.Update(fixed_delta_time, cvar_num_collision_steps.get(), cvar_num_integration_substeps.get(), &system_data->temp_allocator, &system_data->job_system);
	}

	BodyHandle JoltContext::create_body(const SphereShapeDescription& shape_desc, const glm::vec3& position, const glm::quat& rotation, PhysicsBodyType body_type)
	{
		JPH::SphereShapeSettings sphere_shape_settings(shape_desc.radius);

		return create_body_internal(&sphere_shape_settings, position, rotation, body_type);
	}

	BodyHandle JoltContext::create_body(const BoxShapeDescription& shape_desc, const glm::vec3& position, const glm::quat& rotation, PhysicsBodyType body_type)
	{
		JPH::BoxShapeSettings box_shape_settings(JPH::Vec3Arg(shape_desc.half_extent.x, shape_desc.half_extent.y, shape_desc.half_extent.z), shape_desc.convex_radius);

		return create_body_internal(&box_shape_settings, position, rotation, body_type);
	}

	BodyHandle JoltContext::create_body(const CapsuleShapeDescription& shape_desc, const glm::vec3& position, const glm::quat& rotation, PhysicsBodyType body_type)
	{
		if (shape_desc.bottom_radius == shape_desc.top_radius)
		{
			JPH::CapsuleShapeSettings capsule_shape_settings(shape_desc.half_height, shape_desc.top_radius);
			return create_body_internal(&capsule_shape_settings, position, rotation, body_type);
		}
		else
		{
			JPH::TaperedCapsuleShapeSettings capsule_shape_settings(shape_desc.half_height, shape_desc.top_radius, shape_desc.bottom_radius);
			return create_body_internal(&capsule_shape_settings, position, rotation, body_type);
		}
	}

	BodyHandle JoltContext::create_body(const CylinderShapeDescription& shape_desc, const glm::vec3& position, const glm::quat& rotation, PhysicsBodyType body_type)
	{
		JPH::CylinderShapeSettings cylinder_shape_settings(shape_desc.half_height, shape_desc.radius, shape_desc.convex_radius);

		return create_body_internal(&cylinder_shape_settings, position, rotation, body_type);
	}

	void JoltContext::set_global_gravity(const glm::vec3& g)
	{
		system_data->physics_system.SetGravity(JPH::RVec3Arg(g.x, g.y, g.z));
	}

	void JoltContext::destroy_body(BodyHandle body)
	{
		if (body >= 0)
		{
			JPH::BodyInterface& body_interface = system_data->physics_system.GetBodyInterface();

			const JPH::BodyID jolt_body(body);
			body_interface.RemoveBody(jolt_body);
			body_interface.DestroyBody(jolt_body);
		}
	}

	void JoltContext::add_body_to_simulation(BodyHandle body)
	{
		if (body >= 0)
		{
			JPH::BodyInterface& body_interface = system_data->physics_system.GetBodyInterface();

			const JPH::BodyID jolt_body(body);
			body_interface.AddBody(jolt_body, JPH::EActivation::DontActivate);
		}
	}

	void JoltContext::remove_body_from_simulation(BodyHandle body)
	{
		if (body >= 0)
		{
			JPH::BodyInterface& body_interface = system_data->physics_system.GetBodyInterface();

			const JPH::BodyID jolt_body(body);
			body_interface.RemoveBody(jolt_body);
		}
	}

	void JoltContext::set_body_position(BodyHandle body, const glm::vec3& position)
	{
		if (body >= 0)
		{
			JPH::BodyInterface& body_interface = system_data->physics_system.GetBodyInterface();

			const JPH::BodyID jolt_body(body);
			body_interface.SetPosition(jolt_body, JPH::RVec3Arg(position.x, position.y, position.z), JPH::EActivation::DontActivate);
		}
	}

	void JoltContext::set_body_rotation(BodyHandle body, const glm::quat& rotation)
	{
		if (body >= 0)
		{
			JPH::BodyInterface& body_interface = system_data->physics_system.GetBodyInterface();

			const JPH::BodyID jolt_body(body);
			body_interface.SetRotation(jolt_body, JPH::QuatArg(rotation.x, rotation.y, rotation.z, rotation.w), JPH::EActivation::DontActivate);
		}
	}

	void JoltContext::set_body_type(BodyHandle body, PhysicsBodyType body_type)
	{
		if (body >= 0)
		{
			JPH::BodyInterface& body_interface = system_data->physics_system.GetBodyInterface();

			const JPH::BodyID jolt_body(body);
			body_interface.SetObjectLayer(jolt_body, JOLT_FROM_SUNSET_OBJECT_LAYER(body_type));
			body_interface.SetMotionType(jolt_body, JOLT_FROM_SUNSET_MOTION_TYPE(body_type), JPH::EActivation::DontActivate);
		}
	}

	void JoltContext::set_body_active(BodyHandle body)
	{
		if (body >= 0)
		{
			JPH::BodyInterface& body_interface = system_data->physics_system.GetBodyInterface();

			const JPH::BodyID jolt_body(body);
			body_interface.ActivateBody(jolt_body);
		}
	}

	void JoltContext::set_body_inactive(BodyHandle body)
	{
		if (body >= 0)
		{
			JPH::BodyInterface& body_interface = system_data->physics_system.GetBodyInterface();

			const JPH::BodyID jolt_body(body);
			body_interface.DeactivateBody(jolt_body);
		}
	}

	glm::vec3 JoltContext::get_body_position(BodyHandle body)
	{
		if (body >= 0)
		{
			JPH::BodyInterface& body_interface = system_data->physics_system.GetBodyInterface();

			const JPH::BodyID jolt_body(body);
			const JPH::RVec3 position = body_interface.GetCenterOfMassPosition(jolt_body);
			return glm::vec3(position.GetX(), position.GetY(), position.GetZ());
		}
		return glm::vec3();
	}

	glm::quat JoltContext::get_body_rotation(BodyHandle body)
	{
		if (body >= 0)
		{
			JPH::BodyInterface& body_interface = system_data->physics_system.GetBodyInterface();

			const JPH::BodyID jolt_body(body);
			const JPH::Quat rotation = body_interface.GetRotation(jolt_body);
			return glm::quat(rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ());
		}
		return glm::quat();
	}

	BodyHandle JoltContext::create_body_internal(JPH::ShapeSettings* shape_settings, const glm::vec3& position, const glm::quat& rotation, PhysicsBodyType body_type)
	{
		JPH::BodyInterface& body_interface = system_data->physics_system.GetBodyInterface();

		JPH::ShapeSettings::ShapeResult shape_result = shape_settings->Create();
		JPH::ShapeRefC shape = shape_result.Get();

		const JPH::EMotionType motion_type = JOLT_FROM_SUNSET_MOTION_TYPE(body_type);
		const JPH::ObjectLayer object_layer = JOLT_FROM_SUNSET_OBJECT_LAYER(body_type);
		JPH::BodyCreationSettings body_settings(shape, JPH::RVec3(position.x, position.y, position.z), JPH::Quat(rotation.x, rotation.y, rotation.z, rotation.w), motion_type, object_layer);

		// TODO: May be better to opt into this. If the majority of bodies in the scene are static, not setting this would give us some space savings since we aren't creating a MotionProperties object per body.
		body_settings.mAllowDynamicOrKinematic = true;

		JPH::BodyID body = body_interface.CreateAndAddBody(body_settings, JPH::EActivation::Activate);

		return body.GetIndexAndSequenceNumber();
	}

	bool CollisionPairFilter::ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const
	{
		switch (inObject1)
		{
			case CollisionLayers::STATIC:
				return inObject2 == CollisionLayers::DYNAMIC;
			case CollisionLayers::DYNAMIC:
				return true;
			default:
				assert(false);
				return false;
		}
	}

	bool BroadPhaseObjectLayerFilter::ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const
	{
		switch (inLayer1)
		{
		case CollisionLayers::STATIC:
			return inLayer2 == BroadPhaseLayers::STATIC;
		case CollisionLayers::DYNAMIC:
			return true;
		default:
			assert(false);
			return false;
		}
	}

	JPH::ValidateResult JoltBodyContactListener::OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult)
	{
		// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
		return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
	}

	void JoltBodyContactListener::OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
	{
	}

	void JoltBodyContactListener::OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
	{
	}

	void JoltBodyContactListener::OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair)
	{
	}

	void JoltBodyActivationListener::OnBodyActivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData)
	{
		std::cout << "Body Activated: " << inBodyID.GetIndexAndSequenceNumber() << std::endl;
	}

	void JoltBodyActivationListener::OnBodyDeactivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData)
	{
		std::cout << "Body Deactivated: " << inBodyID.GetIndexAndSequenceNumber() << std::endl;
	}
}

