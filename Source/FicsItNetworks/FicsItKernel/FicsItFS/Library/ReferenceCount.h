#pragma once

#include <functional>

namespace FileSystem {
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
		int weak_count = 0;
		size_t shared_count = 0;

	public:
		virtual ~ReferenceCounted() {}
	};

	template<class T>
	class Ref {
		template<class T>
		friend class SRef;
		template<class T>
		friend class WRef;
		friend struct std::hash<FileSystem::WRef<T>>;
		friend struct std::hash<FileSystem::SRef<T>>;

	protected:
		ReferenceCounted* ref;

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
			if (!ref || ref->shared_count < 1) return nullptr;
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
			return get();
		}
	};

	template<class T>
	class SRef : public Ref<T> {
	public:
		SRef(T* ref = nullptr) : Ref<T>(ref) {
			static_assert(std::is_base_of<ReferenceCounted, T>::value, "T not derived from ReferenceCounted");
			if (this->ref) this->ref->shared_count++;
		}

		SRef(const SRef<T>& other) : SRef(dynamic_cast<T*>(other.ref)) {}

		template<class O>
		SRef(const Ref<O>& other) : SRef(dynamic_cast<T*>(other.ref)) {}

		~SRef() {
			auto r = Ref<T>::ref;
			if (r) r->shared_count--;
		}

		SRef& operator=(const SRef& newRef) {
			auto r = Ref<T>::ref;
			if (r) r->shared_count--;
			Ref<T>::ref = newRef.ref;
			if (newRef.ref) newRef.ref->shared_count++;
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

		WRef(const WRef<T>& other) : WRef(dynamic_cast<T*>(other.ref)) {}

		template<class O>
		WRef(const Ref<O>& other) : WRef(dynamic_cast<T*>(other.ref)) {}

		WRef& operator=(const WRef& newRef) {
			auto r = Ref<T>::ref;
			if (r) r->weak_count--;
			Ref<T>::ref = newRef.ref;
			if (newRef.ref) newRef.ref->weak_count++;
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
	struct hash<FileSystem::WRef<R>> {
		std::size_t operator()(FileSystem::WRef<R> const& o) const noexcept {
			return std::hash<void*>{}(o.ref);
		}
	};

	template<typename R>
	struct hash<FileSystem::SRef<R>> {
		std::size_t operator()(FileSystem::WRef<R> const& o) const noexcept {
			return std::hash<void*>{}(o.ref);
		}
	};
}