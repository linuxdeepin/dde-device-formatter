cmake_minimum_required(VERSION 3.13)

# 设置项目名称和版本
project(dde-device-formatter VERSION 1.0.0)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置安装路径
set(CMAKE_INSTALL_PREFIX /usr)

# 查找必要的包
find_package(Qt5 COMPONENTS Core Gui Widgets Concurrent Network X11Extras DBus REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PKGS REQUIRED
    x11
    udisks2-qt5
    dtkwidget
    dtkgui
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
set(TRANSLATIONS
    translations/${PROJECT_NAME}.ts
    translations/${PROJECT_NAME}_zh_CN.ts
)

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
)

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Concurrent
    Qt5::Network
    Qt5::X11Extras
    Qt5::DBus
    ${PKGS_LIBRARIES}
)

# 添加编译选项
target_compile_options(${PROJECT_NAME} PRIVATE ${PKGS_CFLAGS_OTHER})

# 处理翻译文件
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    find_package(Qt5 COMPONENTS LinguistTools REQUIRED)
    
    # 生成 .qm 文件
    qt5_add_translation(QM_FILES ${TRANSLATIONS})
    
    # 添加自定义命令执行脚本
    add_custom_target(translations ALL DEPENDS ${QM_FILES})
    add_custom_command(TARGET translations POST_BUILD
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/translate_ts2desktop.sh
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

# 安装目标
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}/translations)
install(FILES dde-device-formatter.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications) 