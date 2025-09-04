#include "../../include/builtins/ls.h"
#include <utils/colors.h>
#include <iostream>
#include <filesystem>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace olsh::Builtins {

int Ls::execute(const std::vector<std::string>& args) {
    std::string path = ".";
    bool showHidden = false;
    bool longFormat = false;

    // parse args
    for (const auto& arg : args) {
        if (!arg.empty() && arg[0] == '-' && arg.size() > 1) {
            for (size_t i = 1; i < arg.size(); i++) {
                switch(arg[i]) {
                    case 'a': showHidden = true; break;
                    case 'l': longFormat = true; break;
                    default:
                        std::cerr << RED << "ls: invalid option: -" << arg[i] << RESET << "\n";
                        std::cerr << "Usage: ls [-a] [-l] <path>\n";
                        return 1;
                }
            }
        }
        else if (arg == "--all") showHidden = true;
        else if (arg == "--long") longFormat = true;
        else { path = arg; }
    }

    // get terminal width
    auto termWidth = []() -> int {
    #ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
            return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    #else
        struct winsize w{};
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0)
            return w.ws_col;
    #endif
        return 80;
    };

    // get width of a utf 8 string (ignoring ansi colors)
    auto dispWidth = [](const std::string& s) -> size_t {
        auto isCombining = [](uint32_t cp) {
            return (cp >= 0x0300 && cp <= 0x036F) ||
                   (cp >= 0x1AB0 && cp <= 0x1AFF) ||
                   (cp >= 0x1DC0 && cp <= 0x1DFF) ||
                   (cp >= 0x20D0 && cp <= 0x20FF) ||
                   (cp >= 0xFE20 && cp <= 0xFE2F) ||
                   (cp >= 0xFE00 && cp <= 0xFE0F) || // variation selectors
                   (cp == 0x200D) || // ZWJ
                   (cp == 0x200B);   // ZWSP
        };
        auto decodeAt = [&](size_t& i, uint32_t& cp){
            unsigned char c = (unsigned char)s[i];
            if (c < 0x80) { cp = c; i += 1; return; }
            if ((c >> 5) == 0x6 && i + 1 < s.size()) {
                cp = ((c & 0x1F) << 6) | ((unsigned char)s[i+1] & 0x3F); i += 2; return;
            }
            if ((c >> 4) == 0xE && i + 2 < s.size()) {
                cp = ((c & 0x0F) << 12) | (((unsigned char)s[i+1] & 0x3F) << 6) | ((unsigned char)s[i+2] & 0x3F); i += 3; return;
            }
            if ((c >> 3) == 0x1E && i + 3 < s.size()) {
                cp = ((c & 0x07) << 18) | (((unsigned char)s[i+1] & 0x3F) << 12) | (((unsigned char)s[i+2] & 0x3F) << 6) | ((unsigned char)s[i+3] & 0x3F); i += 4; return;
            }
            cp = c; i += 1; // fallback
        };
        size_t w = 0;
        for (size_t i = 0; i < s.size();) {
            unsigned char c = (unsigned char)s[i];
            if (c == 0x1B && i + 1 < s.size() && s[i+1] == '[') {
                i += 2; while (i < s.size() && s[i] != 'm') ++i; if (i < s.size()) ++i; continue;
            }
            uint32_t cp = 0; size_t j = i; decodeAt(j, cp);
            i = j;
            if (!isCombining(cp)) w += 1; // count combining marks as width 0
        }
        return w;
    };
    auto padRight = [&](const std::string& s, size_t width) -> std::string {
        size_t v = dispWidth(s);
        if (v >= width) return s;
        return s + std::string(width - v, ' ');
    };
    auto padLeft = [&](const std::string& s, size_t width) -> std::string {
        size_t v = dispWidth(s);
        if (v >= width) return s;
        return std::string(width - v, ' ') + s;
    };
    auto ellipsize = [&](const std::string& s, size_t maxW) -> std::string {
        auto isCombining = [](uint32_t cp) {
            return (cp >= 0x0300 && cp <= 0x036F) ||
                   (cp >= 0x1AB0 && cp <= 0x1AFF) ||
                   (cp >= 0x1DC0 && cp <= 0x1DFF) ||
                   (cp >= 0x20D0 && cp <= 0x20FF) ||
                   (cp >= 0xFE20 && cp <= 0xFE2F) ||
                   (cp >= 0xFE00 && cp <= 0xFE0F) ||
                   (cp == 0x200D) || (cp == 0x200B);
        };
        auto decodeAt = [&](const std::string& s, size_t& i, uint32_t& cp, size_t& step){
            unsigned char c = (unsigned char)s[i];
            if (c < 0x80) { cp = c; step = 1; return; }
            if ((c >> 5) == 0x6 && i + 1 < s.size()) { cp = ((c & 0x1F) << 6) | ((unsigned char)s[i+1] & 0x3F); step = 2; return; }
            if ((c >> 4) == 0xE && i + 2 < s.size()) { cp = ((c & 0x0F) << 12) | (((unsigned char)s[i+1] & 0x3F) << 6) | ((unsigned char)s[i+2] & 0x3F); step = 3; return; }
            if ((c >> 3) == 0x1E && i + 3 < s.size()) { cp = ((c & 0x07) << 18) | (((unsigned char)s[i+1] & 0x3F) << 12) | (((unsigned char)s[i+2] & 0x3F) << 6) | ((unsigned char)s[i+3] & 0x3F); step = 4; return; }
            cp = c; step = 1; return;
        };
        if (dispWidth(s) <= maxW) return s;
        std::string out; out.reserve(s.size());
        size_t cur = 0;
        for (size_t i = 0; i < s.size();) {
            unsigned char c = (unsigned char)s[i];
            if (c == 0x1B && i + 1 < s.size() && s[i+1] == '[') {
                size_t j = i + 2; while (j < s.size() && s[j] != 'm') ++j; if (j < s.size()) ++j;
                out.append(s, i, j - i); i = j; continue;
            }
            uint32_t cp = 0; size_t step = 1; decodeAt(s, i, cp, step);
            if (!isCombining(cp)) {
                if (cur + 1 >= maxW) break;
                cur += 1;
            }
            out.append(s, i, step); i += step;
        }
        return out + "…";
    };

    auto colorName = [&](const std::string& filename, bool isDir, bool isExec) -> std::string {
        if (isDir) return std::string(BOLD_BLUE) + filename + "/" + RESET;
        if (isExec) return std::string(BOLD_GREEN) + filename + RESET;
        // keep others default to reduce noise
        return filename;
    };

    // perms 
    auto permsStr = [&](std::filesystem::perms p, bool isDir) -> std::string {
        std::string bits;
        bits += isDir ? 'd' : '-';
        bits += (p & std::filesystem::perms::owner_read)  != std::filesystem::perms::none ? 'r' : '-';
        bits += (p & std::filesystem::perms::owner_write) != std::filesystem::perms::none ? 'w' : '-';
        bits += (p & std::filesystem::perms::owner_exec)  != std::filesystem::perms::none ? 'x' : '-';
        bits += (p & std::filesystem::perms::group_read)  != std::filesystem::perms::none ? 'r' : '-';
        bits += (p & std::filesystem::perms::group_write) != std::filesystem::perms::none ? 'w' : '-';
        bits += (p & std::filesystem::perms::group_exec)  != std::filesystem::perms::none ? 'x' : '-';
        bits += (p & std::filesystem::perms::others_read) != std::filesystem::perms::none ? 'r' : '-';
        bits += (p & std::filesystem::perms::others_write)!= std::filesystem::perms::none ? 'w' : '-';
        bits += (p & std::filesystem::perms::others_exec) != std::filesystem::perms::none ? 'x' : '-';
        return std::string(BOLD_MAGENTA) + bits + RESET;
    };

    auto humanSize = [](std::uintmax_t size) -> std::string {
        const char* units[] = {"B","K","M","G","T"};
        double v = static_cast<double>(size); int u = 0;
        while (v >= 1024.0 && u < 4) { v /= 1024.0; ++u; }
        std::ostringstream oss; if (u == 0) oss << (std::uintmax_t)size << 'B'; else oss << std::fixed << std::setprecision(1) << v << units[u];
        return oss.str();
    };

    auto modTime = [](const std::filesystem::file_time_type& ftime) -> std::string {
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
        std::time_t t = std::chrono::system_clock::to_time_t(sctp);
        std::ostringstream oss; oss << std::put_time(std::localtime(&t), "%b %d %H:%M");
        return oss.str();
    };

    auto border = [](const std::vector<size_t>& widths) -> std::string {
        std::ostringstream oss; oss << '+'; for (size_t i=0;i<widths.size();++i){ oss << std::string(widths[i]+2,'-') << '+'; } return oss.str();
    };

    struct Row { std::string name, colored, sizeStr, timeStr; bool isDir, isExec; std::filesystem::perms perms; std::uintmax_t size; };

    try {
        if (std::filesystem::is_directory(path)) {
            std::vector<Row> rows; rows.reserve(128);
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                std::string filename = entry.path().filename().string();
                if (!showHidden && !filename.empty() && filename[0] == '.') continue;
                auto p = entry.status().permissions();
                bool isDir = entry.is_directory();
                bool isExec = (p & std::filesystem::perms::owner_exec) != std::filesystem::perms::none;
                std::uintmax_t sz = entry.is_regular_file() ? std::filesystem::file_size(entry) : 0;
                rows.push_back(Row{filename, colorName(filename, isDir, isExec), humanSize(sz), modTime(entry.last_write_time()), isDir, isExec, p, sz});
            }

            if (longFormat) {
                // compute widths
                std::string hPerm = "Perms", hSize = "Size", hTime = "Modified", hName = "Name";
                size_t wPerm = std::max<size_t>(10, dispWidth(hPerm));
                size_t wSize = dispWidth(hSize), wTime = dispWidth(hTime), wName = dispWidth(hName);
                for (const auto& r : rows) { wSize = std::max(wSize, dispWidth(r.sizeStr)); wTime = std::max(wTime, dispWidth(r.timeStr)); wName = std::max(wName, dispWidth(r.colored)); }
                int tw = termWidth();
                std::vector<size_t> widths = {wPerm, wSize, wTime, wName};
                auto totalW = [&](const std::vector<size_t>& ws){ return 1 + (int)ws.size() + (int)std::accumulate(ws.begin(), ws.end(), size_t{0}) + 2*(int)ws.size(); };
                while (totalW(widths) > tw && widths[3] > 8) widths[3]--;

                std::cout << border(widths) << "\n";
                std::ostringstream hdr;
                hdr << '|' << ' ' << padRight(std::string(BOLD_CYAN)+hPerm+RESET, widths[0])
                    << ' ' << '|' << ' ' << padRight(std::string(BOLD_CYAN)+hSize+RESET, widths[1])
                    << ' ' << '|' << ' ' << padRight(std::string(BOLD_CYAN)+hTime+RESET, widths[2])
                    << ' ' << '|' << ' ' << padRight(std::string(BOLD_CYAN)+hName+RESET, widths[3])
                    << ' ' << '|';
                std::cout << hdr.str() << "\n";
                std::cout << border(widths) << "\n";

                for (const auto& r : rows) {
                    std::string n = r.colored; if (dispWidth(n) > widths[3]) n = ellipsize(n, widths[3]);
                    std::ostringstream ln;
                    ln << '|' << ' ' << padRight(permsStr(r.perms, r.isDir), widths[0])
                       << ' ' << '|' << ' ' << padLeft(r.sizeStr, widths[1])
                       << ' ' << '|' << ' ' << padRight(r.timeStr, widths[2])
                       << ' ' << '|' << ' ' << padRight(n, widths[3])
                       << ' ' << '|';
                    std::cout << ln.str() << "\n";
                }
                std::cout << border(widths) << std::endl;
            } else {
                if (rows.empty()) { std::cout << "(empty)\n"; return 0; }
                int tw = termWidth();
                size_t maxName = 0; for (const auto& r : rows) maxName = std::max(maxName, dispWidth(r.colored));
                size_t colW = std::min<size_t>(std::max<size_t>(maxName, 1), 40);
                std::vector<size_t> widths; // determine columns that fit
                int ncols = std::max(1, (tw - 1) / (int(colW) + 3)); // crude fit
                widths.assign(ncols, colW);
                std::cout << border(widths) << "\n";
                int nrows = (int(rows.size()) + ncols - 1) / ncols;
                for (int r = 0; r < nrows; ++r) {
                    std::ostringstream ln; ln << '|';
                    for (int c = 0; c < ncols; ++c) {
                        int idx = r + c * nrows; // column-major to fill down columns
                        std::string cell;
                        if (idx < (int)rows.size()) { cell = rows[idx].colored; if (dispWidth(cell) > widths[c]) cell = ellipsize(cell, widths[c]); cell = padRight(cell, widths[c]); }
                        else { cell = std::string(widths[c], ' '); }
                        ln << ' ' << cell << ' ' << '|';
                    }
                    std::cout << ln.str() << "\n";
                    std::cout << border(widths) << "\n";
                }
            }
        } else {
            std::cout << path << std::endl;
        }
        return 0;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << RED << "ls: " << e.what() << RESET << std::endl;
        return 1;
    }
}

} // namespace olsh::Builtins

