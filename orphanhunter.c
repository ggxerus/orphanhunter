#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <sys/stat.h>
#include <ctime>
#include <algorithm>
#include <cstring>
#include <limits>

const int DAYS_THRESHOLD = 30;

struct OrphanPackage {
    std::string name;
    bool recently_used;
    std::string used_binary;
    time_t last_access;
};

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

bool check_file_access(const std::string& path, time_t& out_atime) {
    struct stat result;
    if (stat(path.c_str(), &result) == 0) {
        time_t now = time(nullptr);
        double seconds_diff = difftime(now, result.st_atime);
        double days_diff = seconds_diff / (60 * 60 * 24);

        if (days_diff < DAYS_THRESHOLD) {
            out_atime = result.st_atime;
            return true;
        }
    }
    return false;
}

int main() {
    std::cout << "[*] Scanning for orphan packages..." << std::endl;

    std::string orphans_raw;
    try {
        orphans_raw = exec("pacman -Qdtq");
    } catch (...) {
        std::cerr << "Error running pacman." << std::endl;
        return 1;
    }

    if (orphans_raw.empty()) {
        std::cout << "[*] No orphan packages found." << std::endl;
        return 0;
    }

    std::stringstream ss(orphans_raw);
    std::string pkg_name;
    std::vector<OrphanPackage> processed_orphans;
    std::vector<std::string> to_delete;

    while (std::getline(ss, pkg_name, '\n')) {
        if (pkg_name.empty()) continue;

        OrphanPackage op;
        op.name = pkg_name;
        op.recently_used = false;
        op.last_access = 0;

        std::string file_cmd = "pacman -Qlq " + pkg_name;
        std::string files_raw = exec(file_cmd.c_str());
        std::stringstream fss(files_raw);
        std::string file_path;

        while (std::getline(fss, file_path, '\n')) {
            if (file_path.find("/bin/") != std::string::npos && file_path.back() != '/') {
                time_t atime;
                if (check_file_access(file_path, atime)) {
                    op.recently_used = true;
                    op.used_binary = file_path;
                    op.last_access = atime;
                    break;
                }
            }
        }

        processed_orphans.push_back(op);
        if (!op.recently_used) {
            to_delete.push_back(op.name);
        }
    }

    int kept_count = 0;
    for (const auto& op : processed_orphans) {
        if (op.recently_used) {
            char time_buf[80];
            struct tm* timeinfo = localtime(&op.last_access);
            strftime(time_buf, 80, "%Y-%m-%d", timeinfo);

            std::cout << "KEEP: \033[1;32m" << op.name << "\033[0m (Used: " << op.used_binary << " on " << time_buf << ")\n";
            kept_count++;
        }
    }

    if (kept_count > 0) std::cout << "------------------------------------------\n";

    if (to_delete.empty()) {
        std::cout << "No orphans eligible for deletion." << std::endl;
        return 0;
    }

    std::cout << "CANDIDATES FOR REMOVAL (Not accessed in " << DAYS_THRESHOLD << " days):\n";
    for (const auto& name : to_delete) {
        std::cout << " - \033[1;31m" << name << "\033[0m\n";
    }

    std::cout << "\nFound " << to_delete.size() << " packages to remove.\n";
    std::cout << "[?] Proceed with removal? [y/N]: ";

    char response;
    std::cin >> response;

    if (response == 'y' || response == 'Y') {
        std::string remove_cmd = "sudo pacman -Rns";
        for (const auto& name : to_delete) {
            remove_cmd += " " + name;
        }
        int ret = system(remove_cmd.c_str());
        if (ret == 0) std::cout << "Cleanup complete.\n";
        else std::cout << "Cleanup failed.\n";
    } else {
        std::cout << "Aborted.\n";
    }

    return 0;
}
