#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <vector>

struct GCObject;
class GCHeap;
class Tracer;

template <typename T>
concept GCManaged = std::is_base_of_v<GCObject, T>;

/**
 * @brief A wrapper around a raw pointer
 * @tparam T The type (has to be inherited from GCObject)
 */
template <GCManaged T>
using GCRef = T*;

enum class GCTag : uint8_t { STRING, ARRAY, DICT, CLOSURE, UPVALUE, FUNCTION };

struct GCObject {
  bool marked = false;
  GCTag tag;
  GCObject* next = nullptr;

  explicit GCObject(GCTag t) : tag(t) {
  }

  virtual ~GCObject() = default;

  virtual void trace(Tracer& tracer) = 0;
};

class RootHandle {
public:
  RootHandle(GCHeap& heap, GCObject*& ref);
  ~RootHandle();

  RootHandle(const RootHandle&) = delete;
  RootHandle& operator=(const RootHandle&) = delete;

private:
  GCHeap& heap_;
  GCObject*& ref_;
};

class GCHeap {
public:
  GCHeap();
  ~GCHeap();

  template <typename T, typename... Args>
  GCRef<T> allocate(Args&&... args) {
    static_assert(std::is_base_of_v<GCObject, T>, "pebbli: Fatal: T in GCHeap::allocate must be a GCObject or derived from a GCObject");
    
    T* obj = new T(std::forward<Args>(args)...);
    obj->next = objects_;
    objects_ = obj;
    object_count_++;
    
    if (object_count_ >= next_gc_) {
      collect();
    }
    
    return obj;
  }

  void add_root(GCObject** ref);
  void remove_root(GCObject** ref);

  void collect();

private:
  GCObject* objects_;
  size_t object_count_;
  size_t next_gc_;

  std::vector<GCObject**> roots_;

  void mark();

  void sweep();

  friend class RootHandle;
};

class Tracer {
public:
  explicit Tracer(GCHeap& heap) : heap_(heap) {}

  void mark(GCObject* obj);

private:
  GCHeap& heap_;
  std::vector<GCObject*> worklist_;
};