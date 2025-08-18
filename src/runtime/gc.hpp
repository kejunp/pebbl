/**
 * @file gc.hpp
 * @brief Garbage collection system for the PEBBL runtime
 */

#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <vector>

struct GCObject;
class GCHeap;
class Tracer;

/**
 * @brief Concept to ensure types are garbage-collectible
 * @tparam T The type to check
 */
template <typename T>
concept GCManaged = std::is_base_of_v<GCObject, T>;

/**
 * @brief A wrapper around a raw pointer for garbage-collected objects
 * @tparam T The type (must be inherited from GCObject)
 */
template <GCManaged T>
using GCRef = T*;

/**
 * @brief Tags identifying different types of garbage-collected objects
 */
enum class GCTag : uint8_t { 
  STRING,   ///< String object
  ARRAY,    ///< Array object  
  DICT,     ///< Dictionary object
  CLOSURE,  ///< Closure object
  UPVALUE,  ///< Upvalue object
  FUNCTION,  ///< Function object
  BUILTIN_FUNCTION ///< Native function that can't be written in pure PEBBL
};

/**
 * @brief Base class for all garbage-collected objects
 * 
 * All objects that need to be managed by the garbage collector must inherit
 * from this class. The GC uses a mark-and-sweep algorithm with a linked list
 * of all allocated objects.
 */
struct GCObject {
  bool marked = false;        ///< Mark flag for garbage collection
  GCTag tag;                  ///< Type tag for this object
  GCObject* next = nullptr;   ///< Next object in the allocation list

  /**
   * @brief Constructor that sets the object type tag
   * @param t The type tag for this object
   */
  explicit GCObject(GCTag t) : tag(t) {
  }

  /**
   * @brief Virtual destructor for proper cleanup
   */
  virtual ~GCObject() = default;

  /**
   * @brief Trace method for marking reachable objects
   * @param tracer The tracer object to use for marking
   * 
   * Each GCObject subclass must implement this method to mark any
   * other GCObjects that this object references.
   */
  virtual void trace(Tracer& tracer) = 0;
};

/**
 * @brief RAII handle for managing GC roots
 * 
 * RootHandle provides automatic registration and deregistration of GC roots.
 * When created, it registers a reference as a root with the GC heap.
 * When destroyed, it automatically removes the root registration.
 */
class RootHandle {
public:
  /**
   * @brief Constructor that registers a GC root
   * @param heap The GC heap to register with
   * @param ref Reference to the GC pointer to register as a root
   */
  RootHandle(GCHeap& heap, GCObject*& ref);

  /**
   * @brief Destructor that unregisters the GC root
   */
  ~RootHandle();

  /**
   * @brief Deleted copy constructor
   */
  RootHandle(const RootHandle&) = delete;

  /**
   * @brief Deleted copy assignment operator
   */
  RootHandle& operator=(const RootHandle&) = delete;

private:
  GCHeap& heap_;        ///< Reference to the GC heap
  GCObject*& ref_;      ///< Reference to the managed pointer
};

/**
 * @brief Garbage collection heap manager
 * 
 * GCHeap manages allocation and collection of garbage-collected objects.
 * It uses a mark-and-sweep algorithm triggered when allocation thresholds
 * are reached. Objects are tracked in a linked list for efficient traversal.
 */
class GCHeap {
public:
  /**
   * @brief Constructor initializes empty heap
   */
  GCHeap();

  /**
   * @brief Destructor cleans up all remaining objects
   */
  ~GCHeap();

  /**
   * @brief Allocate a new garbage-collected object
   * @tparam T The type to allocate (must derive from GCObject)
   * @tparam Args Constructor argument types
   * @param args Arguments to forward to the constructor
   * @return A pointer to the newly allocated object
   * 
   * This method allocates a new object of type T and adds it to the
   * GC-managed object list. If the allocation threshold is reached,
   * a garbage collection cycle is triggered.
   */
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

  /**
   * @brief Add a root reference to the GC system
   * @param ref Pointer to a GCObject pointer to register as a root
   */
  void add_root(GCObject** ref);

  /**
   * @brief Remove a root reference from the GC system
   * @param ref Pointer to a GCObject pointer to unregister as a root
   */
  void remove_root(GCObject** ref);

  /**
   * @brief Trigger a garbage collection cycle
   * 
   * Performs a full mark-and-sweep garbage collection, freeing all
   * unreachable objects and updating collection thresholds.
   */
  void collect();

private:
  GCObject* objects_;         ///< Linked list of all allocated objects
  size_t object_count_;       ///< Current number of allocated objects
  size_t next_gc_;           ///< Threshold for triggering next collection

  std::vector<GCObject**> roots_;  ///< List of registered root references

  /**
   * @brief Mark phase of garbage collection
   * 
   * Marks all reachable objects starting from registered roots.
   */
  void mark();

  /**
   * @brief Sweep phase of garbage collection
   * 
   * Deallocates all unmarked objects and resets mark flags.
   */
  void sweep();

  friend class RootHandle;
};

/**
 * @brief Tracer for marking reachable objects during garbage collection
 * 
 * The Tracer class manages the marking phase of garbage collection,
 * using a worklist algorithm to traverse the object graph and mark
 * all reachable objects.
 */
class Tracer {
public:
  /**
   * @brief Constructor
   * @param heap The GC heap this tracer belongs to
   */
  explicit Tracer(GCHeap& heap) : heap_(heap) {}

  /**
   * @brief Mark an object as reachable
   * @param obj The object to mark (can be nullptr)
   * 
   * If the object is not null and not already marked, it will be
   * marked and added to the worklist for further tracing.
   */
  void mark(GCObject* obj);

private:
  GCHeap& heap_;                      ///< Reference to the owning heap
  std::vector<GCObject*> worklist_;   ///< Worklist of objects to trace
};