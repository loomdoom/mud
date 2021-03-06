//  Copyright (c) 2018 Hugo Amiard hugo.amiard@laposte.net
//  This software is provided 'as-is' under the zlib License, see the LICENSE.txt file.
//  This notice and the license may not be removed or altered from any source distribution.

#pragma once

// @todo: robust handle-based pool implementation + switch tree to use this pool + solve case of multiple types in tree (maybe just by simplifiying it out)

#if 0

#include <infra/NonCopy.h>
#include <infra/Vector.h>
#include <type/Unique.h>
#include <type/TypeUtils.h>

namespace mud
{
	template<class T>
	class VecPool;

	export_ template <class T>
	struct Handle
	{
		static VecPool<T>* s_pool = nullptr;
		uint32_t m_index;
		uint8_t m_version;
		explicit operator bool() { return s_pool->m_versions[m_index] == m_version; }
		T& operator*() const { return (T*)s_pool->m_chunk.data() + m_index; }
	};

	export_ template<class T>
	class VecPool : public NonCopy
	{
	public:
		//using Handle = T*;
		using Handle = mud::Handle<T>;

		VecPool(uint32_t size = 256, uint32_t offset = 0)
			: m_size(size)
			, m_offset(offset)
			, m_last(offset + size)
			, m_chunk(size * sizeof(T))
			, m_versions(size, 0)
		{
			m_available.reserve(size);
			m_objects.reserve(size);

			for(uint32_t i = 0; i < size; ++i)
				m_available.push_back({ i, 0 });
		}

		~VecPool()
		{
			for(Handle object : m_objects)
				any_destruct(*object);
		}

		Handle alloc()
		{
			if(m_available.empty() && !m_next)
				m_next = make_unique<VecPool<T>>(m_size * 2, m_last);
			if(m_available.empty())
				return m_next->alloc();

			Handle object = m_available.back();
			m_available.pop_back();
			m_objects.push_back(object);
			return object;
		}

		void destroy(Handle object)
		{
			any_destruct(*object);
			this->free(object);
		}

		void free(Handle object)
		{
			if(object.m_index > m_last)
				return m_next->free(object);

			m_available.push_back({ object, object.m_version + 1 });
			vector_remove(m_objects, object);
		}

	public:
		uint32_t m_size;
		uint32_t m_offset;
		uint32_t m_last;

		std::vector<uint8_t> m_chunk;
		std::vector<uint8_t> m_versions;

		std::vector<Handle> m_available;
		std::vector<Handle> m_objects;

		unique_ptr<VecPool<T>> m_next;

		static int s_count;
	};
	
	template<class T>
	int VecPool<T>::s_count = 0;
}
#endif
