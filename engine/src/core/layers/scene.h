#pragma once

#include <type_traits>

#include <minimal.h>
#include <core/simulation_layer.h>
#include <core/subsystem.h>
#include <core/ecs/entity.h>
#include <memory/allocators/pool_allocator.h>

namespace Sunset
{
	struct alignas(16) SceneLightingData
	{
		glm::vec4 fog_color;
		glm::vec4 sunlight_direction;
		glm::vec4 sunlight_color;
		glm::vec4 csm_plane_distances;
		float fog_distance;
		float sunlight_intensity;
		float sunlight_angular_radius;
		float atmospheric_turbidity;
		float atmospheric_rayleigh;
		float mie_coefficient;
		float mie_directional_g;
		float num_lights{ 0 };
		int32_t irradiance_map;
		int32_t sky_box;
		int32_t prefilter_map;
		int32_t brdf_lut;
		glm::mat4 light_space_matrices[MAX_SHADOW_CASCADES];
	};

	struct SceneData
	{
		SceneLightingData lighting[MAX_BUFFERED_FRAMES];
		ImageID sky_box;
		ImageID irradiance_map;
		ImageID prefilter_map;
		ImageID brdf_lut;
		uint32_t cam_data_buffer_start{ 0 };
		uint32_t lighting_data_buffer_start{ 0 };
		BufferID buffer{ 0 };
	};

	void set_scene_fog_color(class Scene* scene, glm::vec4 fog_color);
	void set_scene_fog_distance(class Scene* scene, float fog_distance);
	void set_scene_sunlight_direction(class Scene* scene, glm::vec3 sunlight_direction);
	void set_scene_sunlight_color(class Scene* scene, glm::vec4 sunlight_color);
	void set_scene_sunlight_intensity(class Scene* scene, float sunlight_intensity);
	void set_scene_sunlight_angular_radius(class Scene* scene, float angular_radius);
	void set_scene_atmospheric_turbidity(class Scene* scene, float turbidity);
	void set_scene_atmospheric_rayleigh(class Scene* scene, float rayleigh);
	void set_scene_mie_coefficient(class Scene* scene, float coeff);
	void set_scene_mie_directional_g(class Scene* scene, float g);

	void set_scene_sky_box(class Scene* scene, const char* sky_box_path);
	void set_scene_sky_irradiance(class Scene* scene, const char* irradiance_map_path);
	void set_scene_prefilter_map(class Scene* scene, const char* prefilter_map_path);
	void set_scene_brdf_lut(class Scene* scene, const char* brdf_lut_path);

	class Scene : public SimulationLayer
	{
		public:
			Scene();
			Scene(const Scene&) = delete;
			Scene& operator=(const Scene&) = delete;
			~Scene() = default;

			virtual void initialize() override;
			virtual void destroy() override;
			virtual void update(double delta_time) override;

			template<class T>
			T* add_subsystem()
			{
				assert((std::is_base_of<Subsystem, T>::value));
				if (T* const subsystem = get_subsystem<T>())
				{
					return subsystem;
				}

				std::unique_ptr<T> new_subsystem = std::make_unique<T>();
				new_subsystem->initialize(this);

				T* new_subsystem_ptr = new_subsystem.get();

				subsystems.push_back(std::move(new_subsystem));

				return new_subsystem_ptr;
			}

			template<class T>
			T* get_subsystem()
			{
				assert((std::is_base_of<Subsystem, T>::value));
				auto found = std::find_if(
					subsystems.cbegin(),
					subsystems.cend(),
					[](const std::unique_ptr<Subsystem>& subsystem)
					{ 
						return typeid(*subsystem.get()) == typeid(T);
					}
				);
				return found != subsystems.cend() ? static_cast<T*>((*found).get()) : nullptr;
			}

			template<class T>
			void remove_subsystem()
			{
				assert((std::is_base_of<Subsystem, T>::value));
				std::erase(std::remove_if(subsystems.begin(), subsystems.end(), [](const std::unique_ptr<Subsystem>& subsystem)
					{
						return typeid(*subsystem.get()) == typeid(T);
					}), subsystems.end());
			}

			template<typename T>
			T* assign_component(EntityID entity_id)
			{
				assert(get_entity_index(entity_id) >= 0 && get_entity_index(entity_id) < entities.size());
				if (entities[get_entity_index(entity_id)].id != entity_id)
				{
					return nullptr;
				}

				int component_id = get_component_id<T>();

				if (component_pools.size() <= component_id)
				{
					component_pools.resize(component_id + 1);
				}
				if (component_pools[component_id] == nullptr)
				{
					component_pools[component_id] = std::make_unique<StaticBytePoolAllocator>(sizeof(T), MIN_ENTITIES);
				}

				T* component = new (component_pools[component_id]->get(get_entity_index(entity_id))) T();

				entities[get_entity_index(entity_id)].components.set(component_id);

				return component;
			}

			template<typename T>
			T* get_component(EntityID entity_id)
			{
				assert(get_entity_index(entity_id) >= 0 && get_entity_index(entity_id) < entities.size());
				if (entities[get_entity_index(entity_id)].id != entity_id)
				{
					return nullptr;
				}

				int component_id = get_component_id<T>();
				if (!entities[get_entity_index(entity_id)].components.test(component_id))
				{
					return nullptr;
				}

				T* component = static_cast<T*>(component_pools[component_id]->get(get_entity_index(entity_id)));

				return component;
			}

			template<typename T>
			void unassign_component(EntityID entity_id)
			{
				assert(get_entity_index(entity_id) >= 0 && get_entity_index(entity_id) < entities.size());
				if (entities[get_entity_index(entity_id)].id != entity_id)
				{
					return;
				}

				int component_id = get_component_id<T>();
				entities[get_entity_index(entity_id)].components.reset(component_id);
			}

			EntityID make_entity();
			void destroy_entity(EntityID entity_id);

		protected:
			void add_default_camera();
			void setup_subsystems();
			void setup_renderer_data();

		public:
			std::vector<std::unique_ptr<Subsystem>> subsystems;
			std::vector<std::unique_ptr<StaticBytePoolAllocator>> component_pools;
			std::vector<Entity> entities;
			std::vector<EntityIndex> free_entities;
			EntityID active_camera{ 0 };
			SceneData scene_data;
	};

	template<typename... ComponentTypes>
	struct SceneView
	{
		SceneView(Scene& scene)
			: scene(&scene)
		{
			b_all = sizeof...(ComponentTypes) == 0;
			if (!b_all)
			{
				int component_ids[] = { 0, get_component_id<ComponentTypes>()... };
				for (int i = 1; i < (sizeof...(ComponentTypes) + 1); ++i)
				{
					components.set(component_ids[i]);
				}
			}
		}

		struct Iterator
		{
			Iterator(Scene* scene, EntityIndex index, ComponentMask components, bool all)
				: scene(scene), index(index), components(components), b_all(all)
				{ }

			EntityID operator*() const
			{
				return scene->entities[index].id;
			}

			bool operator==(const Iterator& other) const
			{
				return index == other.index || index == scene->entities.size();
			}

			bool operator!=(const Iterator& other) const
			{
				return index != other.index && index != scene->entities.size();
			}

			Iterator& operator++()
			{
				do
				{
					++index;
				} while (index < scene->entities.size() && !valid_index());
				return *this;
			}

			bool valid_index()
			{
				return is_valid_entity(scene->entities[index].id)
					&& (b_all || components == (components & scene->entities[index].components));
			}

			Scene* scene;
			EntityIndex index;
			ComponentMask components;
			bool b_all{ false };
		};

		const Iterator begin() const
		{
			int first_index = 0;
			while (first_index < scene->entities.size() && (components != (components & scene->entities[first_index].components) || !is_valid_entity(scene->entities[first_index].id)))
			{
				++first_index;
			}
			return Iterator(scene, first_index, components, b_all);
		}

		const Iterator end() const
		{
			return Iterator(scene, EntityIndex(scene->entities.size()), components, b_all);
		}

		Scene* scene{ nullptr };
		ComponentMask components;
		bool b_all{ false };
	};
}
