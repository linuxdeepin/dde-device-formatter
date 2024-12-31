cmake_minimum_required(VERSION 3.13)

# 设置项目名称和版本
project(dde-device-formatter VERSION 1.0.0)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含 GNUInstallDirs，用于获取标准安装路径
include(GNUInstallDirs)

# 查找必要的包
find_package(Qt6 COMPONENTS Core Gui Widgets Concurrent Network DBus LinguistTools REQUIRED)
find_package(Dtk6 COMPONENTS Widget Gui REQUIRED)
find_package(udisks2-qt6 REQUIRED)
find_package(PkgConfig REQUIRED)
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

# 使用 Qt 提供的翻译工具
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
    Qt6::Core
    Qt6::Gui
    Qt6::GuiPrivate
    Qt6::Widgets
    Qt6::Concurrent
    Qt6::Network
    Qt6::DBus
    Dtk6::Widget
    Dtk6::Gui
    udisks2-qt6::udisks2-qt6
    ${PKGS_LIBRARIES}
)

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
