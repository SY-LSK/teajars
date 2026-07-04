import subprocess
import sys
import os
import platform
import shutil

PROJECT_DIR = os.path.dirname(os.path.abspath(__file__))
SRC_DIR = os.path.join(PROJECT_DIR, "src")
INCLUDE_DIR = os.path.join(PROJECT_DIR, "include")

SOURCES = [
    os.path.join(PROJECT_DIR, "main.cpp"),
    os.path.join(SRC_DIR, "base.cpp"),
    os.path.join(SRC_DIR, "server.cpp"),
]

OUTPUT_NAME = "teajars"

CPPSTD = "c++17"
OPT_LEVEL = "-O3"


def is_windows():
    return platform.system() == "Windows"


def is_macos():
    return platform.system() == "Darwin"


def is_linux():
    return platform.system() == "Linux"


def find_compiler():
    if os.environ.get("CXX"):
        return os.environ["CXX"]
    for cmd in ["g++", "clang++"]:
        if shutil.which(cmd):
            return cmd
    print("Error: no suitable C++ compiler found (g++ or clang++)")
    sys.exit(1)


def build_gcc(compiler):
    output = OUTPUT_NAME + (".exe" if is_windows() else "")
    output_path = os.path.join(PROJECT_DIR, output)

    cmd = [
        compiler,
        OPT_LEVEL,
        f"-std={CPPSTD}",
        f"-I{INCLUDE_DIR}",
        f"-I{SRC_DIR}",
        "-flto",
    ]

    cmd += SOURCES
    cmd += ["-o", output_path]

    if is_windows():
        cmd.append("-lws2_32")
    elif is_linux():
        cmd.append("-lpthread")

    print(f"Compiling with {compiler}...")
    print(" ".join(cmd))
    result = subprocess.run(cmd, cwd=PROJECT_DIR)
    if result.returncode == 0:
        print(f"Compilation successful! Output: {output_path}")
    else:
        print("Compilation failed!")
        sys.exit(1)


def build_msvc():
    cl_path = shutil.which("cl")
    if not cl_path:
        print("Error: MSVC cl.exe not found. Please run from Developer Command Prompt.")
        sys.exit(1)

    output = OUTPUT_NAME + ".exe"
    output_path = os.path.join(PROJECT_DIR, output)

    cmd = [
        "cl",
        "/MP",
        "/EHsc",
        "/O2",
        f"/std:{CPPSTD}",
        "/utf-8",
        "/D_HAS_STD_BYTE=0",
        f"/I{INCLUDE_DIR}",
        f"/I{SRC_DIR}",
    ] + SOURCES + [f"/Fe:{output_path}"]

    print("Compiling with MSVC cl...")
    print(" ".join(cmd))
    result = subprocess.run(cmd, cwd=PROJECT_DIR)
    if result.returncode == 0:
        for ext in [".obj"]:
            for src in SOURCES:
                obj_file = os.path.splitext(os.path.basename(src))[0] + ext
                obj_path = os.path.join(PROJECT_DIR, obj_file)
                if os.path.exists(obj_path):
                    os.remove(obj_path)
        print(f"Compilation successful! Output: {output_path}")
    else:
        print("Compilation failed!")
        sys.exit(1)


def build_cmake():
    cmake_path = shutil.which("cmake")
    if not cmake_path:
        print("Error: cmake not found.")
        sys.exit(1)

    build_dir = os.path.join(PROJECT_DIR, "build")
    os.makedirs(build_dir, exist_ok=True)

    cmake_lists = os.path.join(PROJECT_DIR, "CMakeLists.txt")
    if not os.path.exists(cmake_lists):
        print("Error: CMakeLists.txt not found. Use --gcc or --msvc instead.")
        sys.exit(1)

    print("Configuring with CMake...")
    result = subprocess.run(
        ["cmake", "-S", PROJECT_DIR, "-B", build_dir],
        cwd=PROJECT_DIR,
    )
    if result.returncode != 0:
        print("CMake configure failed!")
        sys.exit(1)

    print("Building with CMake...")
    result = subprocess.run(
        ["cmake", "--build", build_dir, "--config", "Release"],
        cwd=PROJECT_DIR,
    )
    if result.returncode == 0:
        print("CMake build successful!")
    else:
        print("CMake build failed!")
        sys.exit(1)


def main():
    if len(sys.argv) < 2:
        print("Usage: python build.py <option>")
        print()
        print("Options:")
        print("  gcc     Build with g++ (MinGW on Windows)")
        print("  clang   Build with clang++")
        print("  msvc    Build with MSVC cl.exe (Windows only)")
        print("  auto    Auto-detect compiler (default g++/clang++)")
        print("  cmake   Build with CMake")
        print()
        if is_windows():
            print("Recommended: python build.py gcc")
        elif is_macos():
            print("Recommended: python build.py clang")
        else:
            print("Recommended: python build.py auto")
        sys.exit(0)

    option = sys.argv[1].lower()

    if option == "gcc":
        build_gcc("g++")
    elif option == "clang":
        build_gcc("clang++")
    elif option == "msvc":
        if not is_windows():
            print("Error: MSVC is only available on Windows.")
            sys.exit(1)
        build_msvc()
    elif option == "auto":
        compiler = find_compiler()
        build_gcc(compiler)
    elif option == "cmake":
        build_cmake()
    else:
        print(f"Unknown option: {option}")
        sys.exit(1)


if __name__ == "__main__":
    main()