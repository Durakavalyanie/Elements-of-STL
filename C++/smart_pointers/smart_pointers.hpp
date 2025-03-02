#include "iostream"
#include "memory"

template <typename T>
class WeakPtr;

struct BaseControlBlock {
  size_t shared_count = 0;
  size_t weak_count = 0;

  BaseControlBlock(size_t in_shared_count, size_t in_weak_count)
      : shared_count(in_shared_count), weak_count(in_weak_count) {}

  virtual ~BaseControlBlock() = default;
  virtual void free_pointer() = 0;
  virtual void free_block() = 0;
};

template <typename T, typename Deleter = std::default_delete<T>,
          typename Allocator = std::allocator<T>>
struct ControlBlockRegular : public BaseControlBlock {
  using BlockAllocator = typename std::allocator_traits<
      Allocator>::template rebind_alloc<ControlBlockRegular>;

  [[no_unique_address]] Deleter deleter;
  [[no_unique_address]] BlockAllocator allocator;
  T* ptr;

  ControlBlockRegular(size_t shared_count, size_t weak_count,
                      Deleter& in_deleter, Allocator in_allocator, T* in_ptr)
      : BaseControlBlock(shared_count, weak_count),
        deleter(in_deleter),
        allocator(in_allocator),
        ptr(in_ptr) {}

  virtual void free_pointer() override { deleter(ptr); }

  virtual void free_block() override {
    std::allocator_traits<BlockAllocator>::destroy(allocator, this);
    std::allocator_traits<BlockAllocator>::deallocate(allocator, this, 1);
  }
};

template <typename T>
struct PointerControlBlock : public BaseControlBlock {
  T* ptr;

  PointerControlBlock(size_t shared_count, size_t weak_count, T* in_ptr)
      : BaseControlBlock(shared_count, weak_count), ptr(in_ptr) {}

  virtual void free_pointer() override { delete ptr; }

  virtual void free_block() override;
};

template <typename T>
void PointerControlBlock<T>::free_block() {
  using BlockAllocator = std::allocator<PointerControlBlock<T>>;
  BlockAllocator allocator;
  std::allocator_traits<BlockAllocator>::destroy(allocator, this);
  std::allocator_traits<BlockAllocator>::deallocate(allocator, this, 1);
}

template <typename T, typename Allocator = std::allocator<T>>
struct ControlBlockMakeShared : public BaseControlBlock {
  T value;
  [[no_unique_address]] Allocator allocator;

  template <typename... Args>
  ControlBlockMakeShared(size_t shared_count, size_t weak_count,
                         Allocator in_allocator, Args&&... args)
      : BaseControlBlock(shared_count, weak_count),
        value(std::forward<Args>(args)...),
        allocator(in_allocator) {}

  void free_pointer() override {
    std::allocator_traits<Allocator>::destroy(allocator, &value);
  }

  void free_block() override;
};

template <typename T, typename Allocator>
void ControlBlockMakeShared<T, Allocator>::free_block() {
  using BlockAllocator = typename std::allocator_traits<
      Allocator>::template rebind_alloc<ControlBlockMakeShared>;
  BlockAllocator block_allocator = allocator;
  std::allocator_traits<BlockAllocator>::deallocate(block_allocator, this, 1);
}

template <typename T>
class SharedPtr {
 public:
  SharedPtr() = default;

  SharedPtr(std::nullptr_t) : SharedPtr(){};

  template <typename U>
  SharedPtr(U* ptr)
      : ptr_(ptr), control_block_(new PointerControlBlock(1, 0, ptr)) {}

  SharedPtr(const SharedPtr<T>& other)
      : ptr_(other.ptr_), control_block_(other.control_block_) {
    ++control_block_->shared_count;
  }

  template <typename U>
  SharedPtr(const SharedPtr<U>& other)
      : ptr_(dynamic_cast<T*>(other.ptr_)),
        control_block_(other.control_block_) {
    ++control_block_->shared_count;
  }

  SharedPtr(SharedPtr<T>&& other)
      : ptr_(std::move(other.ptr_)),
        control_block_(std::move(other.control_block_)) {
    other.ptr_ = nullptr;
    other.control_block_ = nullptr;
  }

  template <typename U, typename Deleter>
  SharedPtr(U* ptr, Deleter deleter)
      : ptr_(ptr),
        control_block_(new ControlBlockRegular<T, Deleter, std::allocator<T>>(
            1, 0, deleter, std::allocator<T>(), ptr)) {}

  template <typename U, typename Deleter, typename Allocator>
  SharedPtr(U* ptr, Deleter deleter, Allocator allocator);

  SharedPtr<T>& operator=(const SharedPtr<T>& other);

  template <typename U>
  SharedPtr<T>& operator=(const SharedPtr<U>& other);

  SharedPtr<T>& operator=(SharedPtr&& other);

  size_t use_count() const {
    return control_block_ == nullptr ? 0 : control_block_->shared_count;
  }

  T* get() const { return ptr_; }

  T& operator*() const { return *ptr_; }

  T* operator->() const { return ptr_; }

  void reset();

  ~SharedPtr() { reset(); }

 private:
  template <typename U>
  friend class SharedPtr;

  friend class WeakPtr<T>;

  template <typename U, typename Allocator, typename... Args>
  friend SharedPtr<U> AllocateShared(const Allocator& allocator,
                                     Args&&... args);

  template <typename U, typename... Args>
  friend SharedPtr<U> MakeShared(Args&&... args);

  SharedPtr(T* ptr, BaseControlBlock* control_block)
      : ptr_(ptr), control_block_(control_block) {}

  T* ptr_ = nullptr;
  BaseControlBlock* control_block_ = nullptr;
};

template <typename T>
template <typename U, typename Deleter, typename Allocator>
SharedPtr<T>::SharedPtr(U* ptr, Deleter deleter, Allocator allocator)
    : ptr_(ptr) {
  using BlockAllocator =
      typename std::allocator_traits<Allocator>::template rebind_alloc<
          ControlBlockRegular<U, Deleter, Allocator>>;
  BlockAllocator block_allocator = allocator;
  auto* block =
      std::allocator_traits<BlockAllocator>::allocate(block_allocator, 1);
  std::allocator_traits<BlockAllocator>::construct(block_allocator, block, 1, 0,
                                                   deleter, allocator, ptr);
  control_block_ = block;
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr<T>& other) {
  if (this == &other) {
    return *this;
  }
  reset();
  ptr_ = other.ptr_;
  control_block_ = other.control_block_;
  ++control_block_->shared_count;
  return *this;
}

template <typename T>
template <typename U>
SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr<U>& other) {
  reset();
  ptr_ = dynamic_cast<T*>(other.ptr_);
  control_block_ = other.control_block_;
  ++control_block_->shared_count;
  return *this;
}

template <typename T>
void SharedPtr<T>::reset() {
  if (ptr_ == nullptr) {
    return;
  }
  --control_block_->shared_count;
  if (control_block_->shared_count == 0) {
    control_block_->free_pointer();
  }
  if (control_block_->shared_count == 0 && control_block_->weak_count == 0) {
    control_block_->free_block();
  }
  ptr_ = nullptr;
  control_block_ = nullptr;
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr&& other) {
  reset();
  ptr_ = other.ptr_;
  control_block_ = other.control_block_;
  other.ptr_ = nullptr;
  other.control_block_ = nullptr;
  return *this;
}

template <typename U, typename Allocator, typename... Args>
SharedPtr<U> AllocateShared(const Allocator& allocator, Args&&... args) {
  using BlockAllocator = typename std::allocator_traits<
      Allocator>::template rebind_alloc<ControlBlockMakeShared<U, Allocator>>;
  BlockAllocator block_allocator = allocator;
  auto* ptr =
      std::allocator_traits<BlockAllocator>::allocate(block_allocator, 1);
  std::allocator_traits<BlockAllocator>::construct(
      block_allocator, ptr, 1, 0, allocator, std::forward<Args>(args)...);

  return SharedPtr<U>(&(ptr->value), static_cast<BaseControlBlock*>(ptr));
}

template <typename U, typename... Args>
SharedPtr<U> MakeShared(Args&&... args) {
  using BlockAllocator =
      std::allocator<ControlBlockMakeShared<U, std::allocator<U>>>;
  BlockAllocator block_allocator;
  auto* ptr =
      std::allocator_traits<BlockAllocator>::allocate(block_allocator, 1);
  std::allocator_traits<BlockAllocator>::construct(block_allocator, ptr, 1, 0,
                                                   std::allocator<U>(),
                                                   std::forward<Args>(args)...);

  return SharedPtr<U>(&(ptr->value), static_cast<BaseControlBlock*>(ptr));
}

template <typename T>
class WeakPtr {
 public:
  WeakPtr() = default;

  WeakPtr(std::nullptr_t) : WeakPtr(){};

  WeakPtr(const WeakPtr& other)
      : control_block_(other.control_block_), ptr_(other.ptr_) {
    ++control_block_->weak_count;
  }

  WeakPtr(const SharedPtr<T>& other) : control_block_(other.control_block_) {
    ++control_block_->weak_count;
    ptr_ = other.ptr_;
  }

  WeakPtr(WeakPtr&& other)
      : control_block_(other.control_block_), ptr_(other.ptr_) {
    other.control_block_ = other.ptr_ = nullptr;
  }

  WeakPtr<T>& operator=(const WeakPtr& other);

  WeakPtr<T>& operator=(WeakPtr&& other);

  ~WeakPtr();

  bool expired() {
    return (control_block_ == nullptr || control_block_->shared_count == 0);
  }

  SharedPtr<T> lock() const {
    return expired() ? SharedPtr<T>() : SharedPtr<T>(ptr_, control_block_);
  }

 private:
  BaseControlBlock* control_block_ = nullptr;
  T* ptr_ = nullptr;
};

template <typename T>
WeakPtr<T>& WeakPtr<T>::operator=(const WeakPtr& other) {
  --control_block_->weak_count;
  ptr_ = other.ptr_;
  control_block_ = other.control_block_;
  ++control_block_->weak_count;
  return *this;
}

template <typename T>
WeakPtr<T>& WeakPtr<T>::operator=(WeakPtr&& other) {
  --control_block_->weak_count;
  ptr_ = other.ptr_;
  control_block_ = other.control_block_;
  other.control_block_ = nullptr;
  ptr_ = nullptr;
  return *this;
}

template <typename T>
WeakPtr<T>::~WeakPtr() {
  if (ptr_ == nullptr) {
    return;
  }
  --control_block_->weak_count;
  if (control_block_->shared_count == 0 && control_block_->weak_count == 0) {
    control_block_->free_block();
  }
}