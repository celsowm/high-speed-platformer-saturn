#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

/* This repository is an ordinary consumer of an installed LibSaturn package. This test reads its
 * sources and build files and fails on anything that would tie it to a LibSaturn checkout:
 *
 *   - an include of a header that the installed prefix does not ship (a private "src/..." header,
 *     a header only an example sees, or one that does not exist);
 *   - the examples' shared helper (example_util.h, sat_example_must);
 *   - a build file that adds a sibling directory or names a LibSaturn source path;
 *   - a script that reaches into a checkout.
 *
 *   test_boundary <repository root> <installed LibSaturn include directory> */

namespace fs = std::filesystem;

static int failures = 0;

static void fail(const std::string& file, const std::string& what) {
    std::fprintf(stderr, "FAIL %s: %s\n", file.c_str(), what.c_str());
    ++failures;
}

static std::string slurp(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static bool contains(const std::string& text, const char* needle) {
    return text.find(needle) != std::string::npos;
}

static void check_source(const fs::path& path, const fs::path& public_include) {
    const std::string name = path.generic_string();
    const std::string text = slurp(path);
    std::istringstream lines(text);
    std::string line;
    while (std::getline(lines, line)) {
        const size_t hash = line.find_first_not_of(" \t");
        if (hash == std::string::npos || line.compare(hash, 8, "#include") != 0) continue;
        const size_t open = line.find_first_of("\"<", hash);
        if (open == std::string::npos) continue;
        const char closer = line[open] == '"' ? '"' : '>';
        const size_t close = line.find(closer, open + 1);
        if (close == std::string::npos) continue;
        const std::string inc = line.substr(open + 1, close - open - 1);
        if (inc.rfind("saturn/", 0) == 0) {
            if (!fs::exists(public_include / inc)) fail(name, "includes a header the installed package does not ship: " + inc);
        } else if (inc.find("..") != std::string::npos || contains(inc, "example_util") || inc.rfind("common/", 0) == 0 ||
                   inc.rfind("src/", 0) == 0) {
            fail(name, "forbidden include: " + inc);
        } else if (inc.find('/') != std::string::npos) {
            fail(name, "includes something outside this repository's own flat sources: " + inc);
        }
    }
    if (contains(text, "sat_example_must")) fail(name, "uses the shared example macro; define a local one");
}

static void check_build_file(const fs::path& path) {
    const std::string name = path.generic_string();
    std::istringstream lines(slurp(path));
    std::string line;
    while (std::getline(lines, line)) {
        const size_t first = line.find_first_not_of(" \t");
        if (first != std::string::npos && line[first] == '#') continue;
        for (const char* banned : {"add_subdirectory", "FetchContent", "examples/", "libsaturn-1", "../libsaturn", "LibSaturn_SOURCE_DIR"}) {
            if (contains(line, banned)) fail(name, std::string("reaches outside the installed package (") + banned + "): " + line);
        }
    }
}

static void check_script(const fs::path& path) {
    const std::string text = slurp(path);
    for (const char* banned : {"examples/", "libsaturn-1", "parents[3]"}) {
        if (contains(text, banned)) fail(path.generic_string(), std::string("reaches into a LibSaturn checkout (") + banned + ")");
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::fprintf(stderr, "usage: test_boundary <repository root> <installed LibSaturn include directory>\n");
        return 2;
    }
    const fs::path root = argv[1];
    const fs::path public_include = argv[2];
    if (!fs::is_directory(public_include / "saturn")) {
        fail(public_include.generic_string(), "no saturn/ directory: is this the installed package's include directory?");
        return 1;
    }

    size_t sources = 0;
    for (const auto& entry : fs::recursive_directory_iterator(root / "src")) {
        if (!entry.is_regular_file()) continue;
        const std::string ext = entry.path().extension().string();
        if (ext == ".c" || ext == ".h") {
            check_source(entry.path(), public_include);
            ++sources;
        }
    }
    for (const auto& entry : fs::recursive_directory_iterator(root / "tests" / "host")) {
        const std::string name = entry.path().filename().string();
        if (entry.is_regular_file() && name != "test_boundary.cpp" && entry.path().extension() == ".cpp") {
            check_source(entry.path(), public_include);
        }
    }
    for (const char* build : {"CMakeLists.txt", "CMakePresets.json", "conanfile.py"}) check_build_file(root / build);
    for (const auto& entry : fs::directory_iterator(root / "cmake")) check_build_file(entry.path());
    for (const auto& entry : fs::directory_iterator(root / "tools")) {
        if (entry.path().extension() == ".py") check_script(entry.path());
    }

    if (sources < 6) fail(root.generic_string(), "expected the game's sources under src/, found fewer than six");
    for (const char* needed : {"CMakeLists.txt", "conanfile.py", "README.md", "requirements.txt", "tools/gen_stage.py", "src/main.c",
                               "src/game.c"}) {
        if (!fs::exists(root / needed)) fail((root / needed).generic_string(), "missing");
    }
    if (failures != 0) return 1;
    std::printf("boundary: %zu sources include only the installed package's public headers\n", sources);
    return 0;
}
