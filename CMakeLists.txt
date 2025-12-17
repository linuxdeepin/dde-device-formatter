cmake_minimum_required(VERSION 3.13)

# 设置项目名称和版本
project(dde-device-formatter VERSION 1.0.0)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含 GNUInstallDirs，用于获取标准安装路径
include(GNUInstallDirs)

# Find Qt version
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
message("   >>> Found Qt version: ${QT_VERSION_MAJOR}")

if (QT_VERSION_MAJOR MATCHES 6)
    set(DTK_VERSION_MAJOR 6)
else()
    set(DTK_VERSION_MAJOR "")
endif()

# 查找必要的包
set(QT_COMPONENTS Core Gui Widgets Concurrent Network DBus LinguistTools)
if (QT_VERSION_MAJOR MATCHES 5)
    list(APPEND QT_COMPONENTS X11Extras)
endif()
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${QT_COMPONENTS} REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR} COMPONENTS Widget Gui REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(UDISKS2QT REQUIRED IMPORTED_TARGET udisks2-qt${QT_VERSION_MAJOR})
pkg_check_modules(PKGS REQUIRED
    x11
)

# 设置源文件
set(SOURCES
    main.cpp
    view/mainwindow.cpp
    view/mainpage.cpp
    view/warnpage.cpp
    view/formatingpage.cpp
    view/finishpage.cpp
    view/errorpage.cpp
    app/cmdmanager.cpp
    dialogs/messagedialog.cpp
    app/singletonapp.cpp
    utils/udisksutils.cpp
    utils/fsutils.cpp
)

set(HEADERS
    view/mainwindow.h
    view/mainpage.h
    view/warnpage.h
    view/formatingpage.h
    view/finishpage.h
    view/errorpage.h
    app/cmdmanager.h
    dialogs/messagedialog.h
    app/singletonapp.h
    utils/udisksutils.h
    utils/fsutils.h
)

# 设置翻译文件
file(GLOB TS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/translations/${PROJECT_NAME}*.ts")

# 根据 Qt 版本使用不同的翻译工具
if (QT_VERSION_MAJOR MATCHES 6)
    # Qt6 使用 qt_add_translations
    qt_add_translations(${PROJECT_NAME}
        # 指定翻译源文件
        TS_FILES ${TS_FILES}
        # 指定 QM 文件输出变量
        QM_FILES_OUTPUT_VARIABLE QM_FILES
        # 指定源文件，用于更新翻译
        SOURCES ${SOURCES} ${HEADERS}
        # 指定 LRELEASE 选项
        LRELEASE_OPTIONS
            -compress
            -nounfinished
            -removeidentical
    )
else()
    # Qt5 手动生成 QM 文件
    set(QM_FILES)
    foreach(TS_FILE ${TS_FILES})
        get_filename_component(TS_FILE_NAME ${TS_FILE} NAME_WE)
        set(QM_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TS_FILE_NAME}.qm")
        add_custom_command(
            OUTPUT ${QM_FILE}
            COMMAND lrelease -compress -nounfinished -removeidentical ${TS_FILE} -qm ${QM_FILE}
            DEPENDS ${TS_FILE}
            COMMENT "Generating ${QM_FILE} from ${TS_FILE}"
        )
        list(APPEND QM_FILES ${QM_FILE})
    endforeach()
    add_custom_target(generate_qm ALL DEPENDS ${QM_FILES})
endif()

# 在文件开头部分添加自动 moc 处理
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 添加可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# 设置包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PKGS_INCLUDE_DIRS}
    ${UDISKS2QT_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::GuiPrivate
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::DBus
    Dtk${DTK_VERSION_MAJOR}::Widget
    Dtk${DTK_VERSION_MAJOR}::Gui
    PkgConfig::UDISKS2QT
    ${PKGS_LIBRARIES}
)

# Qt5 需要链接 X11Extras
if (QT_VERSION_MAJOR MATCHES 5)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::X11Extras
    )
endif()

# 添加编译选项
target_compile_options(${PROJECT_NAME} PRIVATE ${PKGS_CFLAGS_OTHER})

# 设置翻译文件路径
set(TRANSLATIONS_INSTALL_DIR "${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}/translations")

# 添加编译定义，传递安装路径到代码中
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "TRANSLATIONS_DIR=\"${TRANSLATIONS_INSTALL_DIR}\""
)

# 安装目标
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 安装翻译文件
install(FILES ${QM_FILES}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/translations
)

# 安装桌面文件
install(FILES dde-device-formatter.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)
