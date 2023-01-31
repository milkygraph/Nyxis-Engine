#pragma once

#include <unordered_map>

#include "ve.hpp"
#include "model.hpp"

namespace ve
{
	class ResourceManager
	{
	public:
		ResourceManager() = default;
		~ResourceManager() = default;

		template <typename T>
		static T* get(const std::string& name)
		{
			return pInstance->getImpl<T>(name);
		}

		template <typename T>
		static void add(const std::string& name, T* resource)
		{
			pInstance->addImpl<T>(name, resource);
		}

		template <typename T>
		static void remove(const std::string& name)
		{
			pInstance->removeImpl<T>(name);
		}
		static ResourceManager* pInstance;

	protected:

		std::unordered_map<std::string, void*> resources;

		template <typename T>
		void removeImpl(const std::string& name)
		{
			delete resources[name];
			resources.erase(name);
		}
		template <typename T>
		T* getImpl(const std::string& name)
		{
			return static_cast<T*>(resources[name]);
		}
		template <typename T>
		void addImpl(const std::string& name, T* resource)
		{
			resources[name] = resource;
		}
	};
}