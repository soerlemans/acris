#!/usr/bin/env python3


"""
TODO: Document.
"""

# Library Imports:
from invoke import task

# Standard Imports:
import os
import sys
import json
import multiprocessing
from enum import StrEnum

# Change current working directory to location of the tasks.py dir.
ABS_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
os.chdir(ABS_SCRIPT_DIR)


# Globals:
class BuildProfile(StrEnum):
    "Enumeration containing the different build configurations."

    BUILD = "build"
    DEBUG = "debug-build"
    RELEASEDEBUG = "reldebug-build"
    MINSIZEREL = "minsizerel-build"
    TEST = "test-build"
    pass


# Functions:
def log(*args):
    "Log function makes it clear what messages come from Invoke."
    print("@Invoke ", *args)
    pass


def log_args(*args, **kwargs):
    "Log the arguments passed to a task."
    if args:
        kwargs.update({"args": args})

    log(json.dumps(kwargs))
    pass


# def run(t_context, t_cmd: str, **kwargs: Any) -> Optional[invoke.Result]:
def run(t_context, t_cmd: str, **kwargs):
    "Helper method for colored output as a default."
    return t_context.run(t_cmd, pty=True)


def cmake_parallel_arg(t_parallel: bool) -> str:
    "Get the cmake arguments for building with multiple threads."
    arg = ""
    if t_parallel:
        # max_jobs = multiprocessing.cpu_count() // 3
        # arg = f'--parallel {max_jobs}'

        arg = f"--parallel 6 "
        pass

    return arg


def cmake_mode_args(t_mode: str) -> str:
    "Get the cmake arguments for a specific build mode."
    args = ""
    match t_mode:
        case BuildProfile.BUILD:
            pass

        case BuildProfile.DEBUG:
            args += "-DCMAKE_BUILD_TYPE=Debug "
            pass

        case BuildProfile.RELEASEDEBUG:
            args += "-DCMAKE_BUILD_TYPE=RelWithDebInfo "
            pass

        case BuildProfile.MINSIZEREL:
            args += "-DCMAKE_BUILD_TYPE=MinSizeRel "
            pass

        case BuildProfile.TEST:
            args += "-DCMAKE_BUILD_TYPE=Debug "
            args += "-DACRIS_BUILD_TESTS=TRUE "
            pass

        case _:
            raise Exception(f"Unknown build configuration {t_mode}")

    # FIXME: rang and zstd have a minim policy version below 3.5.
    # CMake 3.5 breaks backwards compatbility with these really old versions.
    # So this is a temporary measure of sidestepping this, but it should be.
    # Fixed in the dependencies.
    args += "-DCMAKE_POLICY_VERSION_MINIMUM=3.5 "

    return args


def cmake(
    t_ctx,
    t_mode: str,
    t_static=False,
    t_parallel=False,
    t_platform="",
    t_arch="",
    t_lint=False,
    t_ci_build=False,
):
    "TODO: Document."
    parallel_arg = cmake_parallel_arg(t_parallel)
    build_args = cmake_mode_args(t_mode)

    # Compile project statically.
    if t_static:
        build_args += "-DACRIS_STATIC_BUILD=TRUE "

    # Build standard library for platform.
    if t_platform:
        build_args += f"-DACRIS_STDLIB_PLATFORM={t_platform} "

    # Build standard library for architecture.
    if t_arch:
        build_args += f"-DACRIS_STDLIB_ARCH={t_arch} "

    # Perform static analysis if specified.
    if t_lint:
        build_args += "-DACRIS_CLANG_TIDY=TRUE "

    # We are performing a CI build.
    if t_ci_build:
        build_args += "-DACRIS_CI_BUILD=TRUE "

    # Always print version info.
    run(t_ctx, "cmake --version")
    run(t_ctx, f"cmake -S . -B {t_mode}/ {build_args}")
    run(t_ctx, f"cmake --build {t_mode}/ {parallel_arg}")
    pass


# Tasks:
@task
def help(ctx):
    "Show a help string."
    help_str = f"""Usage inv
    """
    pass


@task
def all(ctx, parallel=True, lint=False):
    log("Building all.")
    log_args(parallel=parallel, lint=lint)

    for mode in BuildProfile:
        print(f"@Invoke: Building '{mode}'")
        build(ctx, mode, parallel)
        pass
    pass


@task
def install(ctx, mode="", static=False, parallel=True, lint=False):
    log("Building project.")
    log_args(mode=mode, parallel=parallel, lint=lint)

    enum_values = [item.value for item in BuildProfile]
    mode = mode if mode in enum_values else "build"
    cmake(ctx, mode, t_static=static, t_parallel=parallel, t_lint=lint)

    # TODO: Check if the build made it or not.
    # TODO: Copy transpiler from Mode install location to /usr/local/bin/

    print(f"\nGive password for installing to /usr/local/:")
    # Install binary to /usr/local/bin/
    ctx.run(f"sudo cp -f ./{mode}/acris /usr/local/bin/acris")

    # Install lbiacris headers.
    # TODO: Automate this.
    local_stdacris = "/usr/local/include/stdacris"
    local_lib = "/usr/local/lib"

    ctx.run(f"sudo mkdir -p {local_stdacris}")
    ctx.run(f"sudo cp -rf ./src/stdacris/* {local_stdacris}")
    ctx.run(f"sudo cp -f ./{mode}/libstdacris.a {local_lib}")

    # TODO: Have CMake generate stdacris.a.
    # TODO: Install shared stdacris.a.
    # ctx.run(f'sudo mkdir -p /usr/local/lib/acris/')
    # ctx.run(f'sudo cp -f ./{mode}/stdlibacrislib.a /usr/local/lib/acris/')
    pass


def uninstall(ctx):
    """Uninstall acris from /usr/local/"""
    ctx.run("sudo rm -f /usr/local/bin/acris")
    ctx.run("sudo rm -rf /usr/local/include/stdacris")
    ctx.run("sudo rm -f /usr/local/lib/libstdacris.a")
    pass


# TODO: Shorten help string, possibly use a global?
@task(
    help={
        "mode": "",
        "parallel": "Flag indicating concurrent builds.",
        "lint": "Perform static analysis on source code using clang-tidy",
    }
)
def build(
    ctx,
    mode="build",
    static=False,
    parallel=True,
    platform="",
    arch="",
    lint=False,
    ci_build=False,
):
    "Build the project."
    log("Building project.")

    enum_values = [item.value for item in BuildProfile]
    mode = mode if mode in enum_values else "build"

    log_args(
        mode=mode,
        static=static,
        parallel=parallel,
        platform=platform,
        arch=arch,
        lint=lint,
        ci_build=ci_build,
    )

    cmake(ctx, mode, static, parallel, platform, arch, lint, ci_build)
    pass


def release(
    ctx,
    static=False,
    parallel=True,
    platform="",
    arch="",
    ci_build=False,
):
    "Build a release of the project."
    log("Building release version.")

    enum_values = [item.value for item in BuildProfile]
    mode = mode if mode in enum_values else "build"

    log_args(
        mode="build",
        static=static,
        parallel=parallel,
        platform=platform,
        arch=arch,
        ci_build=ci_build,
    )

    cmake(ctx, "build", static, parallel, platform, arch, False, ci_build)
    pass


@task
def clean(ctx, keep_deps=False):
    "Cleans the build files."
    for directory in BuildProfile:
        path = directory
        path += "/CMakeFiles" if keep_deps else ""
        path += "/*"

        print(f"@Invoke: Removing: {path}")
        ctx.run(f"rm -rf {path}/*")
        pass

    # Remove any generated binaries.
    ctx.run(f"rm -rf *.out")
    pass


@task
def format(ctx):
    "Clang-format all Ctx++ sources and headers."
    ctx.run(
        r"find src/ -name '*.[ch]pp' -or -name '*.[ch]' -exec clang-format -i '{}' \;"
    )
    pass


@task
def header_guard(ctx):
    "Regenerate all heade guards to be unique."
    ctx.run(
        r"PROJECT_NAME=ACRIS find src/ -name '*.hpp' -name '*.h' -exec ./tools/header_guard.awk {} \;"
    )
    pass


@task(help={"pdf": "Generate PDF Doxygen output or not (requires make)."})
def docs(ctx, pdf=False):
    "Generate Doxygen documentation."
    ctx.run(f"doxygen .doxyfile")

    if pdf:
        ctx.run("cd doxygen/latex && make")
        pass
    pass


@task
def setup(ctx):
    "Setup dependencies."
    match sys.platform:
        case "linux":
            ctx.run("bash tools/setup/linux_setup.sh")
            pass

        case "win32":
            print("Error: Currently the setup script does not support Windows.")
            pass

        case "darwin":
            ctx.run("bash tools/setup/macos_setup.sh")
            pass

        case _:
            print(f"Unsupported platform: {sys.platform}")
            pass
    pass


@task
def profile(ctx):
    "Profile application using valgrind and perf."
    print("TODO: Implement.")
    pass
