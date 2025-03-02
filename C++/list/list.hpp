#include <memory>
#include <initializer_list>

template <typename T, typename Allocator = std::allocator<T>>
class List {
 public:
  template <bool IsConst>
  class Iterator;

  using iterator = List::Iterator<false>;
  using const_iterator = List::Iterator<true>;
  using reverse_iterator = std::reverse_iterator<List::Iterator<false>>;
  using const_reverse_iterator = std::reverse_iterator<List::Iterator<true>>;

  using value_type = T;
  using allocator_type = Allocator;

  iterator begin() const {
    return iterator(static_cast<Node*>(fake_node_.next));
  }

  iterator end() const {
    return iterator(static_cast<Node*>(fake_node_.prev->next));
  }

  const_iterator cbegin() const { return const_iterator(begin()); }

  const_iterator cend() const { return const_iterator(end()); }

  std::reverse_iterator<iterator> rbegin() const {
    return reverse_iterator(end());
  }

  std::reverse_iterator<iterator> rend() const {
    return reverse_iterator(begin());
  }

  std::reverse_iterator<const_iterator> crbegin() const {
    return const_reverse_iterator(end());
  }

  std::reverse_iterator<const_iterator> crend() const {
    return const_reverse_iterator(begin());
  }

  List() : fake_node_(&fake_node_, &fake_node_) {}

  List(size_t count, const T& value, const Allocator& alloc = Allocator());

  explicit List(size_t count, const Allocator& alloc = Allocator());

  List(const List& other);

  List(std::initializer_list<T> init, const Allocator& alloc = Allocator());

  T& front() { return static_cast<Node*>(fake_node_.next)->value; }

  const T& front() const { return static_cast<Node*>(fake_node_.next)->value; }

  T& back() { return static_cast<Node*>(fake_node_.prev)->value; }

  const T& back() const { return static_cast<Node*>(fake_node_.prev)->value; }

  bool empty() const { return (size_ == 0); }

  size_t size() const { return size_; }

  void push_back(const T& value);

  void push_front(const T& value);

  void pop_back();

  void pop_front();

  ~List();

  allocator_type get_allocator() const { return allocator_; }

  List& operator=(const List& other);

 private:
  struct BaseNode;

  struct Node;

  using alloc_traits = std::allocator_traits<Allocator>;
  using node_alloc = typename alloc_traits::template rebind_alloc<Node>;
  using node_alloc_traits = typename alloc_traits::template rebind_traits<Node>;

  BaseNode fake_node_;
  size_t size_ = 0;
  node_alloc allocator_;
};

template <typename T, typename Allocator>
List<T, Allocator>::List(size_t count, const T& value, const Allocator& alloc)
    : fake_node_(&fake_node_, &fake_node_), allocator_(alloc) {
  try {
    for (; count > 0; --count) {
      push_back(value);
    }
  } catch (...) {
    this->~List();
    throw;
  }
}

template <typename T, typename Allocator>
List<T, Allocator>::List(size_t count, const Allocator& alloc)
    : fake_node_(&fake_node_, &fake_node_), allocator_(alloc) {
  Node* innovation_node;
  try {
    innovation_node = node_alloc_traits::allocate(allocator_, 1);
    node_alloc_traits::construct(allocator_, innovation_node);
    innovation_node->prev = &fake_node_;
    innovation_node->next = &fake_node_;
    fake_node_.prev = fake_node_.next = innovation_node;
    ++size_;
    for (; count - 1 > 0; --count) {
      innovation_node = node_alloc_traits::allocate(allocator_, 1);
      node_alloc_traits::construct(allocator_, innovation_node);
      innovation_node->next = &fake_node_;
      fake_node_.prev->next = innovation_node;
      innovation_node->prev = fake_node_.prev;
      fake_node_.prev = innovation_node;
      ++size_;
    }
  } catch (...) {
    node_alloc_traits::deallocate(allocator_, innovation_node, 1);
    this->~List();
    throw;
  }
}

template <typename T, typename Allocator>
List<T, Allocator>::List(const List& other)
    : allocator_(node_alloc_traits::select_on_container_copy_construction(
          other.allocator_)) {
  try {
    for (const auto& elem : other) {
      push_back(elem);
    }
  } catch (...) {
    this->~List();
    throw;
  }
}

template <typename T, typename Allocator>
List<T, Allocator>::List(std::initializer_list<T> init, const Allocator& alloc)
    : allocator_(alloc) {
  try {
    for (const auto& elem : init) {
      push_back(elem);
    }
  } catch (...) {
    this->~List();
    throw;
  }
}

template <typename T, typename Allocator>
void List<T, Allocator>::push_back(const T& value) {
  Node* innovation_node = node_alloc_traits::allocate(allocator_, 1);
  try {
    node_alloc_traits::construct(allocator_, innovation_node, value);
  } catch (...) {
    node_alloc_traits::deallocate(allocator_, innovation_node, 1);
    --size_;
    throw;
  }
  if (size_ == 0) {
    innovation_node->prev = &fake_node_;
    innovation_node->next = &fake_node_;
    fake_node_.prev = fake_node_.next = innovation_node;
  } else {
    innovation_node->next = &fake_node_;
    fake_node_.prev->next = innovation_node;
    innovation_node->prev = fake_node_.prev;
    fake_node_.prev = innovation_node;
  }
  ++size_;
}

template <typename T, typename Allocator>
void List<T, Allocator>::push_front(const T& value) {
  Node* innovation_node = node_alloc_traits::allocate(allocator_, 1);
  try {
    node_alloc_traits::construct(allocator_, innovation_node, value);
  } catch (...) {
    node_alloc_traits::deallocate(allocator_, innovation_node, 1);
    --size_;
    throw;
  }
  if (size_ == 0) {
    innovation_node->prev = &fake_node_;
    innovation_node->next = &fake_node_;
    fake_node_.prev = fake_node_.next = innovation_node;
  } else {
    innovation_node->prev = &fake_node_;
    fake_node_.next->prev = innovation_node;
    innovation_node->next = fake_node_.next;
    fake_node_.next = innovation_node;
  }
  ++size_;
}

template <typename T, typename Allocator>
void List<T, Allocator>::pop_back() {
  Node* temp = static_cast<Node*>(fake_node_.prev);
  fake_node_.prev->prev->next = &fake_node_;
  fake_node_.prev = fake_node_.prev->prev;
  node_alloc_traits::destroy(allocator_, temp);
  node_alloc_traits::deallocate(allocator_, temp, 1);
  --size_;
}

template <typename T, typename Allocator>
void List<T, Allocator>::pop_front() {
  Node* temp = static_cast<Node*>(fake_node_.next);
  fake_node_.next->next->prev = &fake_node_;
  fake_node_.next = fake_node_.next->next;
  node_alloc_traits::destroy(allocator_, temp);
  node_alloc_traits::deallocate(allocator_, temp, 1);
  --size_;
}

template <typename T, typename Allocator>
List<T, Allocator>::~List() {
  if (size_ == 0) {
    return;
  }
  Node* begin = static_cast<Node*>(fake_node_.next);
  BaseNode* temp;
  while (begin != fake_node_.prev) {
    temp = begin->next;
    node_alloc_traits::destroy(allocator_, begin);
    node_alloc_traits::deallocate(allocator_, begin, 1);
    begin = static_cast<Node*>(temp);
  }
  node_alloc_traits::destroy(allocator_, begin);
  node_alloc_traits::deallocate(allocator_, begin, 1);
  size_ = 0;
}

template <typename T, typename Allocator>
List<T, Allocator>& List<T, Allocator>::operator=(const List& other) {
  if (this == &other) {
    return *this;
  }

  node_alloc original_alloc = allocator_;
  size_t original_size = size_;

  if (alloc_traits::propagate_on_container_copy_assignment::value) {
    allocator_ = other.allocator_;
  }

  size_t counter = 0;
  try {
    for (const auto& elem : other) {
      push_back(elem);
      ++counter;
    }
  } catch (...) {
    size_ -= counter;
    for (; counter > 0; --counter) {
      pop_back();
    }
    allocator_ = original_alloc;
    throw;
  }

  allocator_ = original_alloc;
  for (size_t i = 0; i < original_size; ++i) {
    pop_front();
  }
  if (alloc_traits::propagate_on_container_copy_assignment::value) {
    allocator_ = other.allocator_;
  }
  return *this;
}

template <typename T, typename Allocator>
struct List<T, Allocator>::BaseNode {
  BaseNode* prev = nullptr;
  BaseNode* next = nullptr;

  BaseNode() = default;
  BaseNode(BaseNode* input_prev, BaseNode* input_next) : prev(input_prev), next(input_next) {}
};

template <typename T, typename Allocator>
struct List<T, Allocator>::Node : BaseNode {
  T value;

  Node(const T& input_value) : value(input_value) {}
  Node() = default;
};

template <typename T, typename Allocator>
template <bool IsConst>
class List<T, Allocator>::Iterator {
 public:
  using iterator_category = std::bidirectional_iterator_tag;
  using difference_type = ptrdiff_t;
  using cond_type = std::conditional_t<IsConst, const T, T>;
  using value_type = cond_type;
  using pointer = cond_type*;
  using reference = cond_type&;

  Iterator(Node* node) : node_(node) {}

  Iterator(const Iterator&) = default;

  Iterator& operator=(const Iterator&) = default;

  operator Iterator<true>() const { return Iterator<true>(node_); }

  reference operator*() const { return node_->value; }

  pointer operator->() const { return &node_->value; }

  Iterator operator++(int) {
    Iterator tmp = *this;
    node_ = static_cast<Node*>(node_->next);
    return tmp;
  }

  Iterator& operator++() {
    node_ = static_cast<Node*>(node_->next);
    return *this;
  }

  Iterator operator--(int) {
    Iterator tmp = *this;
    node_ = static_cast<Node*>(node_->prev);
    return tmp;
  }

  Iterator& operator--() {
    node_ = static_cast<Node*>(node_->prev);
    return *this;
  }

  bool operator==(const Iterator& second) const {
    return node_ == second.node_;
  }

  bool operator!=(const Iterator& second) const {
    return node_ != second.node_;
  }

 private:
  Node* node_;
};