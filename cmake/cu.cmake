
if(IS_INDEPENDENT)
  set(MODE_INCLUDE "${CU_STD_DIR}/include/cu/mode/")
endif()

if(DEBUG)
  add_definitions(-DDEBUG)
endif()

if(VERBOSE)
	add_definitions(-DVERBOSE)
endif()

if(UNIX)

  set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
  set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

  find_package(PkgConfig REQUIRED)
  pkg_check_modules(DBUS REQUIRED dbus-1)
  pkg_check_modules(WAYLAND REQUIRED wayland-client)

  if(DEBUG)

    # set(CLANG_DEBUG "-fsanitize=address,memory,undefined,safe-stack,thread")
    set(CLANG_DEBUG "") # break here __asan::ReportGenericError

    set(
      STRICT_FLAGS
      "-Werror -Wall -Wextra -pedantic -Wcast-align  -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self  -Wmissing-include-dirs   -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-promo  -Wstrict-overflow=5 -Wundef -Wno-unused -Wno-variadic-macros -Wno-parentheses -Wunused-function -Wunused-label -Wunused-value -Wunused-variable -fdiagnostics-show-option -Wno-missing-field-initializers -Wno-missing-braces -Wimplicit-fallthrough -Wdouble-promotion -Wnull-dereference -Wswitch -Wuninitialized -Wunknown-pragmas -Warray-bounds -Wtautological-compare -Wfloat-equal -Wabsolute-value -Wconversion -Wdangling-else -Waddress -Wpacked -Wvla -Wstack-protector"
      )

  else()

    set(OPT_FLAGS "-O3")
    set(STRICT_FLAGS)

  endif()

  set(PLATFORM_INCLUDES
      "${CU_STD_DIR}/include/cu/linux"
      ${DBUS_INCLUDE_DIRS}
      #"/usr/include/dbus-1.0/"
      #"/usr/lib/x86_64-linux-gnu/dbus-1.0/include/" #ubuntu dbus-arch-deps.h location
      #"/usr/lib/dbus-1.0/include/" #fedora dbus-arch-deps.h location
      #"/usr/lib64/dbus-1.0/include/"
     )

     message(STATUS "DBUS_INCLUDE_DIRS = ${DBUS_INCLUDE_DIRS}")

  enable_language(ASM)

  file(GLOB LINUX_ASM ${CU_STD_DIR}/src/cu_std/linux/linux_asm.s)
  set(PLATFORM_SRC ${LINUX_ASM})


  #this adds LTO support
  include(CheckIPOSupported)
  check_ipo_supported(RESULT LTO_SUPPORTED)

  if(LTO_SUPPORTED)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()

else(UNIX)

  if(DEBUG)

    set(STRICT_FLAGS "/W3 /WX")
    set(OPT_FLAGS "/Od")

  else()

    set(STRICT_FLAGS "/W3 /WX")
    set(OPT_FLAGS "/O2")

  endif()

  set(PLATFORM_INCLUDES "${CU_STD_DIR}/include/cu/win32")
  set(PLATFORM_SRC)

  # allow debug info here cos windows doesn't embed it into the dll/exe
  set(FLAGS "/EHsc /Zi ${OPT_FLAGS} ${STRICT_FLAGS}")

endif(UNIX)

function(GenerateWaylandExtensionFiles)

  # generate wayland extension files if they exist
  if(UNIX)

    find_file(FOUND_WAYLAND_GEN
              xdg-shell.h
              ${CMAKE_SOURCE_DIR}/include/generated/
              NO_DEFAULT_PATH)

    if(NOT FOUND_WAYLAND_GEN)

      find_file(FOUND_WAYLAND_EXT_XML xdg-shell.xml
                PATHS /usr/share/wayland-protocols/stable/xdg-shell/
                NO_DEFAULT_PATH)

      find_program(FOUND_WAYLAND_SCANNER wayland-scanner)

      if(FOUND_WAYLAND_EXT_XML AND FOUND_WAYLAND_SCANNER)

        execute_process(
          COMMAND echo GENERATING WAYLAND EXTENSIONS
                  # generate wayland files
          COMMAND
            bash -c
            "wayland-scanner code 		</usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml 		>${CMAKE_SOURCE_DIR}/include/generated/xdg-shell.c"
          COMMAND
            bash -c
            "wayland-scanner client-header </usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml >${CMAKE_SOURCE_DIR}/include/generated/xdg-shell.h"
          )

      else()

        if(NOT FOUND_WAYLAND_SCANNER)
          message(STATUS "WARNING: wayland-scanner not found!\n")
        endif()

        if(NOT FOUND_WAYLAND_EXT_XML)
          message(STATUS "WARNING: wayland extension xml not found!\n")
        endif()

        message(STATUS "WARNING: Building without wayland extensions\n")
        add_definitions(-DNO_WAYLAND_EXTENSIONS)

      endif()

    endif()

  endif(UNIX)
endfunction()
