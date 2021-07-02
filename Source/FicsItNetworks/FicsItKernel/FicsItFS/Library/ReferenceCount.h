#pragma once

#include <functional>
#include <mutex>
#include <atomic>

namespace CodersFileSystem {
	template<class T>
	class Ref;
	template<class T>
	class SRef;
	template<class T>
	class WRef;

	class ReferenceCounted {
		template<class T>
		friend class Ref;
		template<class T>
		friend class SRef;
		template<class T>
		friend class WRef;

	private:
		std::atomic<int> weak_count = 0;
		std::atomic<size_t> shared_count = 0;

	public:
		virtual ~ReferenceCounted() {}
	};

	template<class T>
	class Ref {
		template<class T>
		friend class SRef;
		template<class T>
		friend class WRef;
		friend struct std::hash<CodersFileSystem::WRef<T>>;
		friend struct std::hash<CodersFileSystem::SRef<T>>;

	protected:
		ReferenceCounted* ref;
		std::mutex mutex;

		Ref(T* ref) : ref(dynamic_cast<ReferenceCounted*>(ref)) {
			static_assert(std::is_base_of<ReferenceCounted, T>::value, "T not derived from ReferenceCounted");
		}

	public:
		~Ref() {
			if (ref && ref->shared_count < 1 && ref->weak_count < 1) {
				delete ref;
			}
			ref = nullptr;
		}

		T* get() const {
			if (!ref) return nullptr;
			std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex));
			if (ref->shared_count < 1) return nullptr;
			else return dynamic_cast<T*>(ref);
		}

		T& operator*() const {
			return *get();
		}

		T* operator->() const {
			return get();
		}

		bool operator<(const Ref& other) const {
			return ref < other.ref;
		}

		operator T*() const {
			return get();
		}

		template<typename R, typename P>
		R operator[](P p) {
			return (*get())[p];
		}

		bool isValid() const {
			return (bool)get();
		}
	};

	template<class T>
	class SRef : public Ref<T> {
	public:
		SRef(T* ref = nullptr) : Ref<T>(ref) {
			static_assert(std::is_base_of<ReferenceCounted, T>::value, "T not derived from ReferenceCounted");
			if (this->ref) this->ref->shared_count++;
		}

		SRef(const SRef<T>& other) : SRef(dynamic_cast<T*>(other.get())) {}

		template<class O>
		SRef(const Ref<O>& other) : SRef(dynamic_cast<T*>(other.get())) {}

		~SRef() {
			auto r = Ref<T>::ref;
			if (r) r->shared_count--;
		}

		SRef& operator=(const SRef& newRef) {
			if (&newRef == this) return *this;
			auto r = Ref<T>::get();
			if (r) r->shared_count--;
			{
				std::lock_guard<std::mutex> lock(Ref<T>::mutex);
				Ref<T>::ref = newRef.get();
			}
			if (newRef.ref) Ref<T>::get()->shared_count++;
			return *this;
		}
	};

	template<class T>
	class WRef : public Ref<T> {
	public:
		WRef(T* ref = nullptr) : Ref<T>(ref) {
			static_assert(std::is_base_of<ReferenceCounted, T>::value, "T not derived from ReferenceCounted");
			if (ref) ref->weak_count++;
		}

		WRef(const WRef<T>& other) : WRef(dynamic_cast<T*>(other.get())) {}

		template<class O>
		WRef(const Ref<O>& other) : WRef(dynamic_cast<T*>(other.get())) {}

		WRef& operator=(const WRef& newRef) {
			if (&newRef == this) return *this;
			auto r = Ref<T>::get();
			if (r) r->weak_count--;
			{
				std::lock_guard<std::mutex> lock(Ref<T>::mutex);
				Ref<T>::ref = newRef.get();
			}
			if (Ref<T>::get()) Ref<T>::get()->weak_count++;
			return *this;
		}

		~WRef() {
			auto r = Ref<T>::ref;
			if (r) r->weak_count--;
		}
	};
}

namespace std {
	template<typename R>
	struct hash<CodersFileSystem::WRef<R>> {
		std::size_t operator()(CodersFileSystem::WRef<R> const& o) const noexcept {
			return std::hash<void*>{}(o.ref);
		}
	};

	template<typename R>
	struct hash<CodersFileSystem::SRef<R>> {
		std::size_t operator()(CodersFileSystem::WRef<R> const& o) const noexcept {
			return std::hash<void*>{}(o.ref);
		}
	};
}