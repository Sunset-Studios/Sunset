#pragma once

#include <type_traits>

#include <minimal.h>
#include <core/simulation_layer.h>
#include <core/subsystem.h>
#include <core/ecs/entity.h>
#include <memory/allocators/pool_allocator.h>

namespace Sunset
{
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

		public:
			std::vector<std::unique_ptr<Subsystem>> subsystems;
			std::vector<std::unique_ptr<StaticBytePoolAllocator>> component_pools;
			std::vector<Entity> entities;
			std::vector<EntityIndex> free_entities;
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
