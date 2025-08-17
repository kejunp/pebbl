/**
 * @file gc.cpp
 * @brief Implementation of the garbage collection system
 */

#include "gc.hpp"
#include <algorithm>

GCHeap::GCHeap() : objects_(nullptr), object_count_(0), next_gc_(8) {}

GCHeap::~GCHeap() {
  // Clean up all remaining objects
  GCObject* current = objects_;
  while (current) {
    GCObject* next = current->next;
    delete current;
    current = next;
  }
}

void GCHeap::add_root(GCObject** ref) {
  roots_.push_back(ref);
}

void GCHeap::remove_root(GCObject** ref) {
  roots_.erase(std::remove(roots_.begin(), roots_.end(), ref), roots_.end());
}

void GCHeap::collect() {
  mark();
  sweep();
  // Set next collection threshold to double the current live objects
  next_gc_ = object_count_ * 2;
}

void GCHeap::mark() {
  Tracer tracer(*this);
  
  // Mark all objects reachable from roots
  for (GCObject** root : roots_) {
    if (*root) {
      tracer.mark(*root);
    }
  }
}

void GCHeap::sweep() {
  GCObject** current = &objects_;
  size_t alive_count = 0;
  
  // Walk through the object list, removing unmarked objects
  while (*current) {
    GCObject* obj = *current;
    if (obj->marked) {
      // Object is alive, reset mark flag and continue
      obj->marked = false;
      current = &obj->next;
      alive_count++;
    } else {
      // Object is dead, remove from list and delete
      *current = obj->next;
      delete obj;
    }
  }
  
  object_count_ = alive_count;
}

void Tracer::mark(GCObject* obj) {
  if (!obj || obj->marked) {
    return;
  }
  
  // Mark the object and add to worklist
  obj->marked = true;
  worklist_.push_back(obj);
  
  // Process worklist until empty
  while (!worklist_.empty()) {
    GCObject* current = worklist_.back();
    worklist_.pop_back();
    // Let the object trace its references
    current->trace(*this);
  }
}

RootHandle::RootHandle(GCHeap& heap, GCObject*& ref) : heap_(heap), ref_(ref) {
  heap_.add_root(&ref_);
}

RootHandle::~RootHandle() {
  heap_.remove_root(&ref_);
}