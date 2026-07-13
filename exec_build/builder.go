package main

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
)

var (
	projectDir string
	srcDir     string
	includeDir string

	sources = []string{
		"main.cpp",
		"src/base.cpp",
		"src/server.cpp",
	}

	outputName = "teajars"
	cppStd     = "c++17"
	optLevel   = "-O3"
)

func init() {
	exePath, _ := os.Executable()
	exeDir := filepath.Dir(exePath)
	projectDir = filepath.Dir(exeDir)
	if _, err := os.Stat(filepath.Join(projectDir, "main.cpp")); err != nil {
		projectDir, _ = os.Getwd()
	}
	srcDir = filepath.Join(projectDir, "src")
	includeDir = filepath.Join(projectDir, "include")
}

func isWindows() bool {
	return runtime.GOOS == "windows"
}

func isDarwin() bool {
	return runtime.GOOS == "darwin"
}

func isLinux() bool {
	return runtime.GOOS == "linux"
}

func outputExeName() string {
	if isWindows() {
		return outputName + ".exe"
	}
	return outputName
}

func absSources() []string {
	var abs []string
	for _, s := range sources {
		abs = append(abs, filepath.Join(projectDir, s))
	}
	return abs
}

func findCompiler() string {
	if cxx := os.Getenv("CXX"); cxx != "" {
		return cxx
	}
	for _, cmd := range []string{"g++", "clang++"} {
		if p, err := exec.LookPath(cmd); err == nil {
			return p
		}
	}
	fmt.Fprintln(os.Stderr, "Error: no suitable C++ compiler found (g++ or clang++)")
	os.Exit(1)
	return ""
}

func buildGCC(compiler string) {
	output := outputExeName()
	outputPath := filepath.Join(projectDir, output)

	cmd := []string{
		compiler,
		optLevel,
		"-std=" + cppStd,
		"-I" + includeDir,
		"-I" + srcDir,
		"-flto",
	}
	cmd = append(cmd, absSources()...)
	cmd = append(cmd, "-o", outputPath)

	if isWindows() {
		cmd = append(cmd, "-lws2_32")
	} else if isLinux() {
		cmd = append(cmd, "-lpthread")
	}

	fmt.Printf("Compiling with %s...\n", compiler)
	fmt.Println(strings.Join(cmd, " "))

	result := exec.Command(cmd[0], cmd[1:]...)
	result.Dir = projectDir
	result.Stdout = os.Stdout
	result.Stderr = os.Stderr
	err := result.Run()
	if err != nil {
		fmt.Fprintln(os.Stderr, "Compilation failed!")
		os.Exit(1)
	}
	fmt.Printf("Compilation successful! Output: %s\n", outputPath)
}

func buildMSVC() {
	clPath, err := exec.LookPath("cl")
	if err != nil || clPath == "" {
		fmt.Fprintln(os.Stderr, "Error: MSVC cl.exe not found. Please run from Developer Command Prompt.")
		os.Exit(1)
	}

	output := outputName + ".exe"
	outputPath := filepath.Join(projectDir, output)

	cmd := []string{
		"cl",
		"/MP",
		"/EHsc",
		"/O2",
		"/std:" + cppStd,
		"/utf-8",
		"/D_HAS_STD_BYTE=0",
		"/I" + includeDir,
		"/I" + srcDir,
	}
	cmd = append(cmd, absSources()...)
	cmd = append(cmd, "/Fe:"+outputPath)

	fmt.Println("Compiling with MSVC cl...")
	fmt.Println(strings.Join(cmd, " "))

	result := exec.Command(cmd[0], cmd[1:]...)
	result.Dir = projectDir
	result.Stdout = os.Stdout
	result.Stderr = os.Stderr
	err = result.Run()
	if err != nil {
		fmt.Fprintln(os.Stderr, "Compilation failed!")
		os.Exit(1)
	}

	for _, src := range absSources() {
		objFile := strings.TrimSuffix(filepath.Base(src), filepath.Ext(src)) + ".obj"
		objPath := filepath.Join(projectDir, objFile)
		os.Remove(objPath)
	}

	fmt.Printf("Compilation successful! Output: %s\n", outputPath)
}

func buildCMake() {
	cmakePath, err := exec.LookPath("cmake")
	if err != nil || cmakePath == "" {
		fmt.Fprintln(os.Stderr, "Error: cmake not found.")
		os.Exit(1)
	}

	buildDir := filepath.Join(projectDir, "build")
	os.MkdirAll(buildDir, 0o755)

	cmakeLists := filepath.Join(projectDir, "CMakeLists.txt")
	if _, err := os.Stat(cmakeLists); os.IsNotExist(err) {
		fmt.Fprintln(os.Stderr, "Error: CMakeLists.txt not found. Use gcc or msvc instead.")
		os.Exit(1)
	}

	fmt.Println("Configuring with CMake...")
	configure := exec.Command(cmakePath, "-S", projectDir, "-B", buildDir)
	configure.Dir = projectDir
	configure.Stdout = os.Stdout
	configure.Stderr = os.Stderr
	if err := configure.Run(); err != nil {
		fmt.Fprintln(os.Stderr, "CMake configure failed!")
		os.Exit(1)
	}

	fmt.Println("Building with CMake...")
	build := exec.Command(cmakePath, "--build", buildDir, "--config", "Release")
	build.Dir = projectDir
	build.Stdout = os.Stdout
	build.Stderr = os.Stderr
	if err := build.Run(); err != nil {
		fmt.Fprintln(os.Stderr, "CMake build failed!")
		os.Exit(1)
	}

	fmt.Println("CMake build successful!")
}

func printUsage() {
	fmt.Println("Usage: builder <option>")
	fmt.Println()
	fmt.Println("Options:")
	fmt.Println("  gcc     Build with g++ (MinGW on Windows)")
	fmt.Println("  clang   Build with clang++")
	fmt.Println("  msvc    Build with MSVC cl.exe (Windows only)")
	fmt.Println("  auto    Auto-detect compiler (default g++/clang++)")
	fmt.Println("  cmake   Build with CMake")
	fmt.Println()
	if isWindows() {
		fmt.Println("Recommended: builder gcc")
	} else if isDarwin() {
		fmt.Println("Recommended: builder clang")
	} else {
		fmt.Println("Recommended: builder auto")
	}
}

func main() {
	if len(os.Args) < 2 {
		printUsage()
		os.Exit(0)
	}

	option := strings.ToLower(os.Args[1])

	switch option {
	case "gcc":
		buildGCC("g++")
	case "clang":
		buildGCC("clang++")
	case "msvc":
		if !isWindows() {
			fmt.Fprintln(os.Stderr, "Error: MSVC is only available on Windows.")
			os.Exit(1)
		}
		buildMSVC()
	case "auto":
		compiler := findCompiler()
		buildGCC(compiler)
	case "cmake":
		buildCMake()
	default:
		fmt.Printf("Unknown option: %s\n", option)
		os.Exit(1)
	}
}
