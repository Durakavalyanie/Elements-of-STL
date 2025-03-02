#include "rec_dir_it.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace stdlike {

directory_entry::directory_entry(const std::string& path) : path_(path) {
    lstat(path_.c_str(), &stat_);
}

recursive_directory_iterator::recursive_directory_iterator(const char* path,
                                                           directory_options dir_opt)
    : dir_opt_(dir_opt), dirs_path_(new std::stack<DIR*>()), cur_path_(path) {
    DIR* dir = opendir(path);
    dirs_path_->emplace(dir);
    this->operator++();
}

recursive_directory_iterator& recursive_directory_iterator::operator++() {
    if (dirs_path_->empty()) {
        *this = end(*this);
        return *this;
    }

    if (!entry_.path_.empty()) {

        struct stat stat_for_dir;
        stat(entry_.path(), &stat_for_dir);
        bool is_dir = S_ISDIR(stat_for_dir.st_mode);

        if (is_dir) {
            bool is_denied = (access(entry_.path(), R_OK) != 0);
            if (!(entry_.is_symlink() && dir_opt_ != directory_options::follow_directory_symlink) &&
                !(is_denied && dir_opt_ == directory_options::skip_permission_denied)) {
                DIR* dir = opendir(entry_.path());
                dirs_path_->emplace(dir);
                cur_path_ = entry_.path_;
            }
        }
    }

    auto next_file = readdir(dirs_path_->top());

    while (next_file == nullptr || next_file->d_name[0] == '.') {
        if (next_file == nullptr) {
            pop();
        }
        if (dirs_path_->empty()) {
            *this = end(*this);
            return *this;
        }
        next_file = readdir(dirs_path_->top());
    }

    entry_ = directory_entry(cur_path_ + '/' + static_cast<std::string>(next_file->d_name));
    return *this;
}

void recursive_directory_iterator::pop() {
    int last_slash = cur_path_.length() - 1;
    for (; last_slash >= 0; --last_slash) {
        if (cur_path_[last_slash] == '/') {
            break;
        }
    }
    cur_path_ = cur_path_.substr(0, last_slash);
    closedir(dirs_path_->top());
    dirs_path_->pop();
    entry_ = directory_entry(cur_path_);
}

recursive_directory_iterator::~recursive_directory_iterator() {
    if (dirs_path_.use_count() != 1) {
        return;
    }
    while (!dirs_path_->empty()) {
        closedir(dirs_path_->top());
        dirs_path_->pop();
    }
}

recursive_directory_iterator begin(recursive_directory_iterator iter) {
    return iter;
}
recursive_directory_iterator end(recursive_directory_iterator) noexcept {
    return {};
}

}  // namespace stdlike