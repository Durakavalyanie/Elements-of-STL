#pragma once

#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <memory>
#include <stack>
#include <string>
#include <unistd.h>

namespace stdlike {

enum class directory_options {
  none = 1,
  follow_directory_symlink = 2,
  skip_permission_denied = 3
};

class directory_entry {
 public:

  directory_entry() = default;

  directory_entry(const std::string& path);

  bool operator==(const directory_entry& other) { return this->path_ == other.path_; }

  const char* path() const { return path_.c_str(); }

  bool is_directory() const { return S_ISDIR(stat_.st_mode); }

  bool is_symlink() const { return S_ISLNK(stat_.st_mode); }

  bool is_regular_file() const { return S_ISREG(stat_.st_mode); }

  bool is_block_file() const { return S_ISBLK(stat_.st_mode); }

  bool is_character_file() const { return S_ISCHR(stat_.st_mode); }

  bool is_socket() const { return S_ISSOCK(stat_.st_mode); }

  bool is_fifo() const { return S_ISFIFO(stat_.st_mode); }

  size_t file_size() const { return stat_.st_size; }

  size_t hard_link_count() const { return stat_.st_nlink; }

  auto last_write_time() const {
    return stat_.st_ctime;
  }

  auto permitions() { return stat_.st_mode; }

 private:

  struct stat stat_;
  std::string path_;

  friend class recursive_directory_iterator;
};

class recursive_directory_iterator {
 public:

  recursive_directory_iterator() = default;

  recursive_directory_iterator(const char* path, directory_options dir_opt = directory_options::none);

  const directory_entry& operator*() { return entry_; }

  const directory_entry* operator->() { return &entry_; }

  recursive_directory_iterator& operator++();

  bool operator==(const recursive_directory_iterator& other) {return (this->entry_ == other.entry_); }

  bool operator!=(const recursive_directory_iterator& other) {return !(*this == other); };

  int depth() { return static_cast<int>(dirs_path_->size()) - 1; }

  void pop();

  ~recursive_directory_iterator();

 private:
  directory_entry entry_;
  directory_options dir_opt_;
  std::shared_ptr<std::stack<DIR*>> dirs_path_;
  std::string cur_path_;
};

recursive_directory_iterator begin( recursive_directory_iterator iter );
recursive_directory_iterator end( recursive_directory_iterator ) noexcept;

}  // namespace stdlike