#ifndef _AGNOSTIC_RESOURCE_MANAGER_H_
#define _AGNOSTIC_RESOURCE_MANAGER_H_

#include <map>
#include "logger.h"

// Resource Manager
// Generalized resource managed with a map storing pointers to the resources.
// Note:	Managed resource must have a constructor taking const KeyType& to allow Load to construct correctly.

namespace agn
{
	template<class KeyType, class ResourceType>
	class ResourceManager
	{
	private:

		std::map<KeyType, ResourceType*> resource_cache_;

	public:

		~ResourceManager()
		{
			for (auto resource : resource_cache_)
			{
				delete resource.second;
			}
		}

		// IsLoaded
		// IN:		Key of resource to check
		// OUT:		Boolean indicating whether or not the resource is present in the cache
		bool IsLoaded(const KeyType& _key) const
		{
			if (resource_cache_.count(_key) == 0)
				return false;
			else
				return true;
		}

		// Load
		// IN:		Key of resource to load
		// OUT:		Pointer to resource
		ResourceType* Load(const KeyType& _key)
		{
			if (IsLoaded(_key))	// Already loaded, just return a pointer
			{
				return resource_cache_[_key];
			}
			else
			{
				// Construct resource using key and insert into cache then return pointer
				auto resource = new ResourceType(_key);
				resource_cache_.insert(std::pair<KeyType, ResourceType*>(_key, resource));
				return resource_cache_[_key];
			}
		}

		// Unload
		// IN:	Key of resource to unload
		void Unload(const KeyType& _key)
		{
			if (!IsLoaded(_key))
			{
				std::string warning;
				warning.append("Key <");
				warning.append(_key);
				warning.append("> is not in the cache - failed to unload.");
				Log::Warning(warning);
			}
			else
			{
				delete resource_cache_[_key];
				resource_cache_.erase(_key);
			}
		}

		// Clear
		// Clears the cache
		void Clear()
		{
			resource_cache_.clear();
		}
	};
}

#endif // _AGNOSTIC_RESOURCE_MANAGER_H_