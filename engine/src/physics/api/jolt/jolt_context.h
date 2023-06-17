#pragma once

#include <minimal.h>

#include <physics/physics_types.h>
#include <physics/api/jolt/jolt_types.h>
#include <physics/api/jolt/jolt_job_system.h>

#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

namespace Sunset
{
	// Class that determines if two object layers can collide
	class CollisionPairFilter : public JPH::ObjectLayerPairFilter
	{
	public:
		virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override;
	};

	// BroadPhaseLayerInterface implementation
	// This defines a mapping between object and broadphase layers.
	class BroadPhaseLayerMapping final : public JPH::BroadPhaseLayerInterface
	{
	public:
		BroadPhaseLayerMapping()
		{
			// Create a mapping table from object to broad phase layer
			mObjectToBroadPhase[CollisionLayers::STATIC] = BroadPhaseLayers::STATIC;
			mObjectToBroadPhase[CollisionLayers::DYNAMIC] = BroadPhaseLayers::DYNAMIC;
		}
		virtual ~BroadPhaseLayerMapping() = default;

		virtual uint32_t GetNumBroadPhaseLayers() const override
		{
			return BroadPhaseLayers::NUM_LAYERS;
		}

		virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
		{
			assert(inLayer < CollisionLayers::NUM_LAYERS);
			return mObjectToBroadPhase[inLayer];
		}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
		virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
		{
			switch ((JPH::BroadPhaseLayer::Type)inLayer)
			{
			case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::STATIC: return "STATIC";
			case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::DYNAMIC:	return "DYNAMIC";
			default: assert(false); return "INVALID";
			}
		}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

	private:
		JPH::BroadPhaseLayer mObjectToBroadPhase[CollisionLayers::NUM_LAYERS];
	};

	// Class that determines if an object layer can collide with a broadphase layer
	class BroadPhaseObjectLayerFilter : public JPH::ObjectVsBroadPhaseLayerFilter
	{
	public:
		virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override;
	};

	// An example contact listener
	class JoltBodyContactListener : public JPH::ContactListener
	{
	public:
		// See: ContactListener
		virtual JPH::ValidateResult	OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) override;
		virtual void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;
		virtual void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;
		virtual void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override;
	};

	// An example activation listener
	class JoltBodyActivationListener : public JPH::BodyActivationListener
	{
	public:
		virtual void OnBodyActivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override;
		virtual void OnBodyDeactivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override;
	};

	class JoltContext
	{
		struct Hooks
		{
			CollisionPairFilter object_layer_filter;
			BroadPhaseLayerMapping broad_phase_object_layer_mapping;
			BroadPhaseObjectLayerFilter broad_phase_layer_filter;
			JoltBodyContactListener contact_listener;
			JoltBodyActivationListener activation_listener;
		};

		struct SystemData
		{
			JoltJobSystem job_system;
			JPH::PhysicsSystem physics_system;
			JPH::TempAllocatorImpl temp_allocator{ 10 * 1024 * 1024 };
		};

	public:
		JoltContext() = default;

		void initialize();
		void destroy();
		void step_simulation();

		BodyHandle create_body(const SphereShapeDescription& shape_desc, const glm::vec3& position, const glm::quat& rotation, PhysicsBodyType body_type);
		BodyHandle create_body(const BoxShapeDescription& shape_desc, const glm::vec3& position, const glm::quat& rotation, PhysicsBodyType body_type);
		BodyHandle create_body(const CapsuleShapeDescription& shape_desc, const glm::vec3& position, const glm::quat& rotation, PhysicsBodyType body_type);
		BodyHandle create_body(const CylinderShapeDescription& shape_desc, const glm::vec3& position, const glm::quat& rotation, PhysicsBodyType body_type);

		void set_global_gravity(const glm::vec3& g);

		void destroy_body(BodyHandle body);
		void add_body_to_simulation(BodyHandle body);
		void remove_body_from_simulation(BodyHandle body);
		void set_body_position(BodyHandle body, const glm::vec3& position);
		void set_body_rotation(BodyHandle body, const glm::quat& rotation);
		void set_body_type(BodyHandle body, PhysicsBodyType body_type);
		void set_body_gravity_scale(BodyHandle body, float gravity_scale);
		void set_body_active(BodyHandle body);
		void set_body_inactive(BodyHandle body);
		glm::vec3 get_body_position(BodyHandle body);
		glm::quat get_body_rotation(BodyHandle body);

	protected:
		BodyHandle create_body_internal(JPH::ShapeSettings* shape_settings, const glm::vec3& position, const glm::quat& rotation, PhysicsBodyType body_type);

	protected:
		Hooks hooks;
		std::unique_ptr<SystemData> system_data;
	};
}
